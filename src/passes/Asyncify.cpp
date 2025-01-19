/*
 * Copyright 2019 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Asyncify: async/await style code transformation, that allows pausing
// and resuming. This lets a language support synchronous operations in
// an async manner, for example, you can do a blocking wait, and that will
// be turned into code that unwinds the stack at the "blocking" operation,
// then is able to rewind it back up when the actual async operation
// completes, so the code appears to have been running synchronously
// all the while. Use cases for this include coroutines, python generators,
// etc.
//
// The approach here is a third-generation design after Emscripten's original
// Asyncify and then Emterpreter-Async approaches:
//
//  * Old Asyncify rewrote control flow in LLVM IR. A problem is that this
//    needs to save all SSA registers as part of the local state, which can be
//    very costly. A further increase can happen because of phis that are
//    added because of control flow transformations. As a result we saw
//    pathological cases where the code size increase was unacceptable.
//  * Emterpreter-Async avoids any risk of code size increase by running it
//    in a little bytecode VM, which also makes pausing/resuming trivial.
//    Speed is the main downside here; however, the approach did prove that by
//    *selectively* emterpretifying only certain code, many projects can run
//    at full speed - often just the "main loop" must be emterpreted, while
//    high-speed code that it calls, and in which cannot be an async operation,
//    remain at full speed.
//
// New Asyncify's design learns from both of those:
//
//  * The code rewrite is done in Binaryen, that is, at the wasm level. At
//    this level we will only need to save wasm locals, which is a much smaller
//    number than LLVM's SSA registers.
//  * We aim for low but non-zero overhead. Low overhead is very important
//    for obvious reasons, while Emterpreter-Async proved it is tolerable to
//    have *some* overhead, if the transform can be applied selectively.
//
// This new Asyncify transformation implemented here is simpler than old
// Asyncify but has low overhead when properly optimized. Old Asyncify worked at
// the CFG level and added branches there; new Asyncify on the other hand works
// on the structured control flow of wasm and simply "skips over" code when
// rewinding the stack, and jumps out when unwinding. The transformed code looks
// conceptually like this:
// (We have three phases: normal, unwinding, and rewinding)
//
//   void foo(int x) {
//     // new prelude
//     if (rewinding) {
//       loadLocals();
//     }
//     // main body starts here
//     if (normal) {
//       // some code we must skip while rewinding
//       x = x + 1;
//       x = x / 2;
//     }
//     // If rewinding, do this call only if it is the right one.
//     if (normal or check_call_index(0)) { // 0 is index of this bar() call
//       bar(x);
//       if (unwinding) {
//         noteUnWound(0);
//         saveLocals();
//         return;
//       }
//     }
//     if (normal) {
//       // more code we must skip while rewinding
//       while (x & 7) {
//         x = x + 1;
//       }
//     }
//     [..]
//
// The general idea is that while rewinding we just "keep going forward",
// skipping code we should not run. That is, instead of jumping directly
// to the right place, we have ifs that skip along instead. The ifs for
// rewinding and unwinding should be well-predicted, so the overhead should
// not be very high. However, we do need to reach the right location via
// such skipping, which means that in a very large function the rewind
// takes some extra time. On the other hand, though, code that cannot
// unwind or rewind (like that loop near the end) can run at full speed.
// Overall, this should allow good performance with small overhead that is
// mostly noticed at rewind time.
//
// After this pass is run a new i32 global "__asyncify_state" is added, which
// has the following values:
//
//   0: normal execution
//   1: unwinding the stack
//   2: rewinding the stack
//
// There is also "__asyncify_data" which when rewinding and unwinding
// contains a pointer to a data structure with the info needed to rewind
// and unwind:
//
//   {                                           // offsets
//     i32  - current asyncify stack location    //  0
//     i32  - asyncify stack end                 //  4
//   }
//
// Or for wasm64:
//
//   {                                           // offsets
//     i64  - current asyncify stack location    //  0
//     i64  - asyncify stack end                 //  8
//   }
//
// The asyncify stack is a representation of the call frame, as a list of
// indexes of calls. In the example above, we saw index "0" for calling "bar"
// from "foo". When unwinding, the indexes are added to the stack; when
// rewinding, they are popped off; the current asyncify stack location is
// updated while doing both operations. The asyncify stack is also used to
// save locals. Note that the stack end location is provided, which is for
// error detection.
//
// Note: all pointers are assumed to be 4-byte (wasm32) / 8-byte (wasm64)
// aligned.
//
// When you start an unwinding operation, you must set the initial fields
// of the data structure, that is, set the current stack location to the
// proper place, and the end to the proper end based on how much memory
// you reserved. Note that asyncify will grow the stack up.
//
// The pass will also create five functions that let you control unwinding
// and rewinding:
//
//  * asyncify_start_unwind(data : iPTR): call this to start unwinding the
//    stack from the current location. "data" must point to a data
//    structure as described above (with fields containing valid data).
//
//  * asyncify_stop_unwind(): call this to note that unwinding has
//    concluded. If no other code will run before you start to rewind,
//    this is not strictly necessary, however, if you swap between
//    coroutines, or even just want to run some normal code during a
//    "sleep", then you must call this at the proper time. Otherwise,
//    the code will think it is still unwinding when it should not be,
//    which means it will keep unwinding in a meaningless way.
//
//  * asyncify_start_rewind(data : iPTR): call this to start rewinding the
//    stack back up to the location stored in the provided data. This prepares
//    for the rewind; to start it, you must call the first function in the
//    call stack to be unwound.
//
//  * asyncify_stop_rewind(): call this to note that rewinding has
//    concluded, and normal execution can resume.
//
//  * asyncify_get_state(): call this to get the current value of the
//    internal "__asyncify_state" variable as described above.
//    It can be used to distinguish between unwinding/rewinding and normal
//    calls, so that you know when to start an asynchronous operation and
//    when to propagate results back.
//
// These five functions are exported so that you can call them from the
// outside. If you want to manage things from inside the wasm, then you
// couldn't have called them before they were created by this pass. To work
// around that, you can create imports to asyncify.start_unwind,
// asyncify.stop_unwind, asyncify.start_rewind, asyncify.stop_rewind, and
// asyncify.get_state; if those exist when this pass runs then it will turn
// those into direct calls to the functions that it creates. Note that when
// doing everything in wasm like this, Asyncify must not instrument your
// "runtime" support code, that is, the code that initiates unwinds and rewinds
// and stops them. If it did, the unwind would not stop until you left the wasm
// module entirely, etc. Therefore we do not instrument a function if it has a
// call to the four asyncify_* methods. Note that you may need to disable
// inlining if that would cause code that does need to be instrumented show up
// in that runtime code.
//
// To use this API, call asyncify_start_unwind when you want to. The call
// stack will then be unwound, and so execution will resume in the JS or
// other host environment on the outside that called into wasm. When you
// return there after unwinding, call asyncify_stop_unwind. Then when
// you want to rewind, call asyncify_start_rewind, and then call the same
// initial function you called before, so that rewinding can begin. The
// rewinding will reach the same function from which you started, since
// we are recreating the call stack. At that point you should call
// asyncify_stop_rewind and then execution can resume normally.
//
// Note that the asyncify_* API calls will verify that the data structure
// is valid, checking if the current location is past the end. If so, they
// will throw a wasm unreachable. No check is done during unwinding or
// rewinding, to keep things fast - in principle, from when unwinding begins
// and until it ends there should be no memory accesses aside from the
// asyncify code itself (and likewise when rewinding), so there should be
// no chance of memory corruption causing odd errors. However, the low
// overhead of this approach does cause an error only to be shown when
// unwinding/rewinding finishes, and not at the specific spot where the
// stack end was exceeded.
//
// By default this pass is very careful: it assumes that any import and
// any indirect call may start an unwind/rewind operation. If you know more
// specific information you can inform asyncify about that, which can reduce
// a great deal of overhead, as it can instrument less code. The relevant
// options to wasm-opt etc. are:
//
//   --pass-arg=asyncify-imports@module1.base1,module2.base2,module3.base3
//
//      Each module.base in that comma-separated list will be considered to
//      be an import that can unwind/rewind, and all others are assumed not to
//      (aside from the asyncify.* imports, which are always assumed to).
//      Each entry can end in a '*' in which case it is matched as a prefix.
//
//      The list of imports can be a response file (which is convenient if it
//      is long, or you don't want to bother escaping it on the commandline
//      etc.), e.g. --pass-arg=asyncify-imports@@file.txt will load the
//      contents of file.txt (note the double "@@" - the first is the
//      separator for --pass-arg, and the second is the usual convention for
//      indicating a response file).
//
//   --pass-arg=asyncify-ignore-imports
//
//      Ignore all imports (except for asyncify.*), that is, assume none of them
//      can start an unwind/rewind. (This is effectively the same as providing
//      asyncify-imports with a list of non-existent imports.)
//
//   --pass-arg=asyncify-ignore-indirect
//
//      Ignore all indirect calls. This implies that you know the call stack
//      will never need to be unwound with an indirect call somewhere in it.
//      If that is true for your codebase, then this can be extremely useful
//      as otherwise it looks like any indirect call can go to a lot of places.
//
//   --pass-arg=asyncify-asserts
//
//      This enables extra asserts in the output, like checking if we put in
//      an unwind/rewind in an invalid place (this can be helpful for manual
//      tweaking of the only-list / remove-list, see later).
//
//   --pass-arg=asyncify-verbose
//
//      Logs out instrumentation decisions to the console. This can help figure
//      out why a certain function was instrumented.
//
//   --pass-arg=asyncify-memory@memory
//      Picks which exported memory of the module to store and load data from
//      and to (useful if the module contains multiple memories).
//
// For manual fine-tuning of the list of instrumented functions, there are lists
// that you can set. These must be used carefully, as misuse can break your
// application - for example, if a function is called that should be
// instrumented but isn't because of these options, then bad things can happen.
// Note that even if you test this locally then users running your code in
// production may reach other code paths. Use these carefully!
//
// Each of the lists can be used with a response file (@filename, which is then
// loaded from the file). You can also use '*' wildcard matching in the lists.
//
//   --pass-arg=asyncify-removelist@name1,name2,name3
//
//      If the "remove-list" is provided, then the functions in it will be
//      *removed* from the list of instrumented functions. That is, they won't
//      be instrumented even if it looks like they need to be. This can be
//      useful if you know things the safe whole-program analysis doesn't, e.g.
//      that certain code paths will not be taken, where certain indirect calls
//      will go, etc.
//
//   --pass-arg=asyncify-addlist@name1,name2,name3
//
//      If the "add-list" is provided, then the functions in the list will be
//      *added* to the list of instrumented functions, that is, they will be
//      instrumented even if otherwise we think they don't need to be. As by
//      default everything will be instrumented in the safest way possible,
//      this is only useful if you use ignore-indirect and use this to fix up
//      some indirect calls that *do* need to be instrumented, or if you will
//      do some later transform of the code that adds more call paths, etc.
//
//   --pass-arg=asyncify-propagate-addlist
//
//      The default behaviour of the addlist does not propagate instrumentation
//      status. If this option is set then functions which call a function in
//      the addlist will also be instrumented, and those that call them and so
//      on.
//
//   --pass-arg=asyncify-onlylist@name1,name2,name3
//
//      If the "only-list" is provided, then *only* the functions in the list
//      will be instrumented, and nothing else.
//
// Note that there are two types of instrumentation that happen for each
// function: if foo() can be part of a pause/resume operation, then we need to
// instrument code inside it to support pausing and resuming, but also, we need
// callers of the function to instrument calls to it. Normally this is already
// taken care of as the callers need to be instrumented as well anyhow. However,
// the ignore-indirect option makes things more complicated, since we can't tell
// where all the calls to foo() are - all we see are indirect calls that do not
// refer to foo() by name. To make it possible for you to use the add-list or
// only-list with ignore-indirect, those lists affect *both* kinds of
// instrumentation. That is, if parent() calls foo() indirectly, and you add
// parent() to the add-list, then the indirect calls in parent() will be
// instrumented to support pausing/resuming, even if ignore-indirect is set.
// Typically you don't need to think about this, and just add all the functions
// that can be on the stack while pausing - what this means is that when you do
// so, indirect calls will just work. (The cost is that an instrumented function
// will check for pausing at all indirect calls, which may be unnecessary in
// some cases; but this is an instrumented function anyhow, and indirect calls
// are slower anyhow, so this simple model seems good enough - a more complex
// model where you can specify "instrument, but not indirect calls from me"
// would likely have little benefit.)
//
// TODO When wasm has GC, extending the live ranges of locals can keep things
//      alive unnecessarily. We may want to set locals to null at the end
//      of their original range.
//

#include "asmjs/shared-constants.h"
#include "cfg/liveness-traversal.h"
#include "ir/effects.h"
#include "ir/find_all.h"
#include "ir/linear-execution.h"
#include "ir/literal-utils.h"
#include "ir/memory-utils.h"
#include "ir/module-utils.h"
#include "ir/names.h"
#include "ir/utils.h"
#include "pass.h"
#include "passes/pass-utils.h"
#include "support/file.h"
#include "support/string.h"
#include "wasm-builder.h"
#include "wasm.h"

namespace wasm {

namespace {

static const Name ASYNCIFY_STATE = "__asyncify_state";
static const Name ASYNCIFY_GET_STATE = "asyncify_get_state";
static const Name ASYNCIFY_DATA = "__asyncify_data";
static const Name ASYNCIFY_START_UNWIND = "asyncify_start_unwind";
static const Name ASYNCIFY_STOP_UNWIND = "asyncify_stop_unwind";
static const Name ASYNCIFY_START_REWIND = "asyncify_start_rewind";
static const Name ASYNCIFY_STOP_REWIND = "asyncify_stop_rewind";
static const Name ASYNCIFY_UNWIND = "__asyncify_unwind";
static const Name ASYNCIFY = "asyncify";
static const Name START_UNWIND = "start_unwind";
static const Name STOP_UNWIND = "stop_unwind";
static const Name START_REWIND = "start_rewind";
static const Name STOP_REWIND = "stop_rewind";
static const Name ASYNCIFY_GET_CALL_INDEX = "__asyncify_get_call_index";
static const Name ASYNCIFY_CHECK_CALL_INDEX = "__asyncify_check_call_index";

// TODO: having just normal/unwind_or_rewind would decrease code
//       size, but make debugging harder
enum class State { Normal = 0, Unwinding = 1, Rewinding = 2 };

enum class DataOffset { BStackPos = 0, BStackEnd = 4, BStackEnd64 = 8 };

const auto STACK_ALIGN = 4;

// A helper class for managing fake global names. Creates the globals and
// provides mappings for using them.
// Fake globals are used to stash and then use return values from calls. We need
// to store them somewhere that is valid Binaryen IR, but also will be ignored
// by the Asyncify instrumentation, so we don't want to use a local. What we do
// is replace the local used to receive a call's result with a fake global.set
// to stash it, then do a fake global.get to receive it afterwards. (We do it in
// two steps so that if we are async, we only do the first and not the second,
// i.e., we don't store to the target local if not running normally).
class FakeGlobalHelper {
  Module& module;

public:
  FakeGlobalHelper(Module& module) : module(module) {
    Builder builder(module);
    std::string prefix = "asyncify_fake_call_global_";
    for (auto type : collectTypes()) {
      auto global = prefix + Type(type).toString();
      map[type] = global;
      rev[global] = type;
      module.addGlobal(builder.makeGlobal(
        global, type, LiteralUtils::makeZero(type, module), Builder::Mutable));
    }
  }

  ~FakeGlobalHelper() {
    for (auto& pair : map) {
      auto name = pair.second;
      module.removeGlobal(name);
    }
  }

  Name getName(Type type) { return map.at(type); }

  Type getTypeOrNone(Name name) {
    auto iter = rev.find(name);
    if (iter != rev.end()) {
      return iter->second;
    }
    return Type::none;
  }

private:
  std::unordered_map<Type, Name> map;
  std::unordered_map<Name, Type> rev;

  // Collect the types returned from all calls for which call support globals
  // may need to be generated.
  using Types = std::unordered_set<Type>;
  Types collectTypes() {
    ModuleUtils::ParallelFunctionAnalysis<Types> analysis(
      module, [&](Function* func, Types& types) {
        if (!func->body) {
          return;
        }
        struct TypeCollector : PostWalker<TypeCollector> {
          Types& types;
          TypeCollector(Types& types) : types(types) {}
          void visitCall(Call* curr) {
            if (curr->type.isConcrete()) {
              types.insert(curr->type);
            }
          }
          void visitCallIndirect(CallIndirect* curr) {
            if (curr->type.isConcrete()) {
              types.insert(curr->type);
            }
          }
        };
        TypeCollector(types).walk(func->body);
      });
    Types types;
    for (auto& pair : analysis.map) {
      Types& functionTypes = pair.second;
      for (auto t : functionTypes) {
        types.insert(t);
      }
    }
    return types;
  }
};

class PatternMatcher {
public:
  std::string designation;
  std::set<Name> names;
  std::set<std::string> patterns;
  std::set<std::string> patternsMatched;
  std::map<std::string, std::string> unescaped;

  PatternMatcher(std::string designation,
                 Module& module,
                 const String::Split& list)
    : designation(designation) {
    // The lists contain human-readable strings. Turn them into the
    // internal escaped names for later comparisons
    for (auto& name : list) {
      auto escaped = WasmBinaryReader::escape(name);
      unescaped[escaped.toString()] = name;
      if (name.find('*') != std::string::npos) {
        patterns.insert(escaped.toString());
      } else {
        auto* func = module.getFunctionOrNull(escaped);
        if (!func) {
          std::cerr << "warning: Asyncify " << designation
                    << "list contained a non-existing function name: " << name
                    << " (" << escaped << ")\n";
        } else if (func->imported()) {
          Fatal() << "Asyncify " << designation
                  << "list contained an imported function name (use the import "
                     "list for imports): "
                  << name << '\n';
        }
        names.insert(escaped.str);
      }
    }
  }

  bool match(Name funcName) {
    if (names.count(funcName) > 0) {
      return true;
    } else {
      for (auto& pattern : patterns) {
        if (String::wildcardMatch(pattern, funcName.toString())) {
          patternsMatched.insert(pattern);
          return true;
        }
      }
    }
    return false;
  }

  void checkPatternsMatches() {
    for (auto& pattern : patterns) {
      if (patternsMatched.count(pattern) == 0) {
        std::cerr << "warning: Asyncify " << designation
                  << "list contained a non-matching pattern: "
                  << unescaped[pattern] << " (" << pattern << ")\n";
      }
    }
  }
};

// Analyze the entire module to see which calls may change the state, that
// is, start an unwind or rewind), either in itself or in something called
// by it.
// Handles global module management, needed from the various parts of this
// transformation.
class ModuleAnalyzer {
  Module& module;
  bool canIndirectChangeState;

  struct Info
    : public ModuleUtils::CallGraphPropertyAnalysis<Info>::FunctionInfo {
    // The function name.
    Name name;
    // If this function can start an unwind/rewind. We only set this in cases
    // where we need to know that fact and also that we need to instrument code
    // to handle it (as a result, we do not set it for the bottommost runtime,
    // which needs no instrumentation).
    bool canChangeState = false;
    // If this function is part of the runtime that receives an unwinding
    // and starts a rewinding. If so, we do not instrument it, see above.
    // This is only relevant when handling things entirely inside wasm,
    // as opposed to imports.
    bool isBottomMostRuntime = false;
    // If this function is part of the runtime that starts an unwinding
    // and stops a rewinding. If so, we do not instrument it, see above.
    // The difference between the top-most and bottom-most runtime is that
    // the top-most part is still marked as changing the state; so things
    // that call it are instrumented. This is not done for the bottom.
    bool isTopMostRuntime = false;
    bool inRemoveList = false;
    bool addedFromList = false;
  };

  using Map = std::map<Function*, Info>;
  Map map;

public:
  ModuleAnalyzer(Module& module,
                 std::function<bool(Name, Name)> canImportChangeState,
                 bool canIndirectChangeState,
                 const String::Split& removeListInput,
                 const String::Split& addListInput,
                 bool propagateAddList,
                 const String::Split& onlyListInput,
                 bool verbose)
    : module(module), canIndirectChangeState(canIndirectChangeState),
      fakeGlobals(module), verbose(verbose) {

    PatternMatcher removeList("remove", module, removeListInput);
    PatternMatcher addList("add", module, addListInput);
    PatternMatcher onlyList("only", module, onlyListInput);

    // Rename the asyncify imports so their internal name matches the
    // convention. This makes replacing them with the implementations
    // later easier.
    std::map<Name, Name> renamings;
    for (auto& func : module.functions) {
      if (func->module == ASYNCIFY) {
        if (func->base == START_UNWIND) {
          renamings[func->name] = ASYNCIFY_START_UNWIND;
        } else if (func->base == STOP_UNWIND) {
          renamings[func->name] = ASYNCIFY_STOP_UNWIND;
        } else if (func->base == START_REWIND) {
          renamings[func->name] = ASYNCIFY_START_REWIND;
        } else if (func->base == STOP_REWIND) {
          renamings[func->name] = ASYNCIFY_STOP_REWIND;
        } else {
          Fatal() << "call to unidenfied asyncify import: " << func->base;
        }
      }
    }
    ModuleUtils::renameFunctions(module, renamings);

    // Scan to see which functions can directly change the state.
    // Also handle the asyncify imports, removing them (as we will implement
    // them later), and replace calls to them with calls to the later proper
    // name.
    ModuleUtils::CallGraphPropertyAnalysis<Info> scanner(
      module, [&](Function* func, Info& info) {
        info.name = func->name;
        if (func->imported()) {
          // The relevant asyncify imports can definitely change the state.
          if (func->module == ASYNCIFY &&
              (func->base == START_UNWIND || func->base == STOP_REWIND)) {
            info.canChangeState = true;
          } else {
            info.canChangeState =
              canImportChangeState(func->module, func->base);
            if (verbose && info.canChangeState) {
              std::cout << "[asyncify] " << func->name
                        << " is an import that can change the state\n";
            }
          }
          return;
        }
        struct Walker : PostWalker<Walker> {
          Info& info;
          Module& module;
          bool canIndirectChangeState;

          Walker(Info& info, Module& module, bool canIndirectChangeState)
            : info(info), module(module),
              canIndirectChangeState(canIndirectChangeState) {}

          void visitCall(Call* curr) {
            if (curr->isReturn) {
              Fatal() << "tail calls not yet supported in asyncify";
            }
            auto* target = module.getFunction(curr->target);
            if (target->imported() && target->module == ASYNCIFY) {
              // Redirect the imports to the functions we'll add later.
              if (target->base == START_UNWIND) {
                info.canChangeState = true;
                info.isTopMostRuntime = true;
              } else if (target->base == STOP_UNWIND) {
                info.isBottomMostRuntime = true;
              } else if (target->base == START_REWIND) {
                info.isBottomMostRuntime = true;
              } else if (target->base == STOP_REWIND) {
                info.canChangeState = true;
                info.isTopMostRuntime = true;
              } else {
                WASM_UNREACHABLE("call to unidenfied asyncify import");
              }
            }
          }
          void visitCallIndirect(CallIndirect* curr) {
            if (curr->isReturn) {
              Fatal() << "tail calls not yet supported in asyncify";
            }
            if (canIndirectChangeState) {
              info.canChangeState = true;
            }
            // TODO optimize the other case, at least by type
          }
        };
        Walker walker(info, module, canIndirectChangeState);
        walker.walk(func->body);

        if (info.isBottomMostRuntime) {
          info.canChangeState = false;
          // TODO: issue warnings on suspicious things, like a function in
          //       the bottom-most runtime also doing top-most runtime stuff
          //       like starting and unwinding.
        }
        if (verbose && info.canChangeState) {
          std::cout << "[asyncify] " << func->name
                    << " can change the state due to initial scan\n";
        }
      });

    // Functions in the remove-list are assumed to not change the state.
    for (auto& [func, info] : scanner.map) {
      if (removeList.match(func->name)) {
        info.inRemoveList = true;
        if (verbose && info.canChangeState) {
          std::cout << "[asyncify] " << func->name
                    << " is in the remove-list, ignore\n";
        }
        info.canChangeState = false;
      }
    }

    // Remove the asyncify imports, if any, and any calls to them.
    std::vector<Name> funcsToDelete;
    for (auto& [func, info] : scanner.map) {
      auto& callsTo = info.callsTo;
      if (func->imported() && func->module == ASYNCIFY) {
        funcsToDelete.push_back(func->name);
      }
      std::vector<Function*> callersToDelete;
      for (auto* target : callsTo) {
        if (target->imported() && target->module == ASYNCIFY) {
          callersToDelete.push_back(target);
        }
      }
      for (auto* target : callersToDelete) {
        callsTo.erase(target);
      }
    }
    for (auto name : funcsToDelete) {
      module.removeFunction(name);
    }

    auto handleAddList = [&](ModuleAnalyzer::Map& map) {
      if (!addListInput.empty()) {
        for (auto& func : module.functions) {
          if (addList.match(func->name) && removeList.match(func->name)) {
            Fatal() << func->name
                    << " is found in the add-list and in the remove-list";
          }

          if (!func->imported() && addList.match(func->name)) {
            auto& info = map[func.get()];
            if (verbose && !info.canChangeState) {
              std::cout << "[asyncify] " << func->name
                        << " is in the add-list, add\n";
            }
            info.canChangeState = true;
            info.addedFromList = true;
          }
        }
      }
    };

    // When propagateAddList is enabled, we should check a add-list before
    // scannerpropagateBack so that callers of functions in add-list should also
    // be instrumented.
    if (propagateAddList) {
      handleAddList(scanner.map);
    }

    // The order of propagation in |propagateBack| is non-deterministic, so sort
    // the loggings we intend to do.
    std::vector<std::string> loggings;

    scanner.propagateBack([](const Info& info) { return info.canChangeState; },
                          [](const Info& info) {
                            return !info.isBottomMostRuntime &&
                                   !info.inRemoveList;
                          },
                          [](Info& info) { info.canChangeState = true; },
                          [&](const Info& info, Function* reason) {
                            if (verbose) {
                              std::stringstream str;
                              str << "[asyncify] " << info.name
                                  << " can change the state due to "
                                  << reason->name << "\n";
                              loggings.push_back(str.str());
                            }
                          },
                          scanner.IgnoreNonDirectCalls);

    if (!loggings.empty()) {
      std::sort(loggings.begin(), loggings.end());
      for (auto& logging : loggings) {
        std::cout << logging;
      }
    }

    map.swap(scanner.map);

    if (!onlyListInput.empty()) {
      // Only the functions in the only-list can change the state.
      for (auto& func : module.functions) {
        if (!func->imported()) {
          auto& info = map[func.get()];
          bool matched = onlyList.match(func->name);
          info.canChangeState = matched;
          if (matched) {
            info.addedFromList = true;
          }
          if (verbose) {
            std::cout << "[asyncify] " << func->name
                      << "'s state is set based on the only-list to " << matched
                      << '\n';
          }
        }
      }
    }

    // When propagateAddList is disabled, which is default behavior,
    // functions in add-list are just prepended to instrumented functions.
    if (!propagateAddList) {
      handleAddList(map);
    }

    removeList.checkPatternsMatches();
    addList.checkPatternsMatches();
    onlyList.checkPatternsMatches();
  }

  bool needsInstrumentation(Function* func) {
    auto& info = map[func];
    return info.canChangeState && !info.isTopMostRuntime;
  }

  bool canChangeState(Expression* curr, Function* func) {
    // Look inside to see if we call any of the things we know can change the
    // state.
    // TODO: caching, this is O(N^2)
    struct Walker : PostWalker<Walker> {
      void visitCall(Call* curr) {
        // We only implement these at the very end, but we know that they
        // definitely change the state.
        if (curr->target == ASYNCIFY_START_UNWIND ||
            curr->target == ASYNCIFY_STOP_REWIND ||
            curr->target == ASYNCIFY_GET_CALL_INDEX ||
            curr->target == ASYNCIFY_CHECK_CALL_INDEX) {
          canChangeState = true;
          return;
        }
        if (curr->target == ASYNCIFY_STOP_UNWIND ||
            curr->target == ASYNCIFY_START_REWIND) {
          isBottomMostRuntime = true;
          return;
        }
        // The target may not exist if it is one of our temporary intrinsics.
        auto* target = module->getFunctionOrNull(curr->target);
        if (target && (*map)[target].canChangeState) {
          canChangeState = true;
        }
      }
      void visitCallIndirect(CallIndirect* curr) { hasIndirectCall = true; }
      Module* module;
      ModuleAnalyzer* analyzer;
      Map* map;
      bool hasIndirectCall = false;
      bool canChangeState = false;
      bool isBottomMostRuntime = false;
    };
    Walker walker;
    walker.module = &module;
    walker.analyzer = this;
    walker.map = &map;
    walker.walk(curr);
    // An indirect call is normally ignored if we are ignoring indirect calls.
    // However, see the docs at the top: if the function we are inside was
    // specifically added by the user (in the only-list or the add-list) then we
    // instrument indirect calls from it (this allows specifically allowing some
    // indirect calls but not others).
    if (walker.hasIndirectCall &&
        (canIndirectChangeState || map[func].addedFromList)) {
      walker.canChangeState = true;
    }
    // The bottom-most runtime can never change the state.
    return walker.canChangeState && !walker.isBottomMostRuntime;
  }

  FakeGlobalHelper fakeGlobals;
  bool verbose;
};

// Checks if something performs a call: either a direct or indirect call,
// and perhaps it is dropped or assigned to a local. This captures all the
// cases of a call in flat IR.
static bool doesCall(Expression* curr) {
  if (auto* set = curr->dynCast<LocalSet>()) {
    curr = set->value;
  } else if (auto* drop = curr->dynCast<Drop>()) {
    curr = drop->value;
  }
  return curr->is<Call>() || curr->is<CallIndirect>();
}

class AsyncifyBuilder : public Builder {
public:
  Module& wasm;
  Type pointerType;
  Name asyncifyMemory;

  AsyncifyBuilder(Module& wasm, Type pointerType, Name asyncifyMemory)
    : Builder(wasm), wasm(wasm), pointerType(pointerType),
      asyncifyMemory(asyncifyMemory) {}

  Expression* makeGetStackPos() {
    return makeLoad(pointerType.getByteSize(),
                    false,
                    int(DataOffset::BStackPos),
                    pointerType.getByteSize(),
                    makeGlobalGet(ASYNCIFY_DATA, pointerType),
                    pointerType,
                    asyncifyMemory);
  }

  Expression* makeIncStackPos(int32_t by) {
    if (by == 0) {
      return makeNop();
    }
    auto literal = Literal::makeFromInt64(by, pointerType);
    return makeStore(pointerType.getByteSize(),
                     int(DataOffset::BStackPos),
                     pointerType.getByteSize(),
                     makeGlobalGet(ASYNCIFY_DATA, pointerType),
                     makeBinary(Abstract::getBinary(pointerType, Abstract::Add),
                                makeGetStackPos(),
                                makeConst(literal)),
                     pointerType,
                     asyncifyMemory);
  }

  Expression* makeStateCheck(State value) {
    return makeBinary(EqInt32,
                      makeGlobalGet(ASYNCIFY_STATE, Type::i32),
                      makeConst(Literal(int32_t(value))));
  }
};

// Instrument control flow, around calls and adding skips for rewinding.
struct AsyncifyFlow : public Pass {
  bool isFunctionParallel() override { return true; }

  ModuleAnalyzer* analyzer;
  Type pointerType;
  Name asyncifyMemory;

  std::unique_ptr<Pass> create() override {
    return std::make_unique<AsyncifyFlow>(
      analyzer, pointerType, asyncifyMemory);
  }

  AsyncifyFlow(ModuleAnalyzer* analyzer, Type pointerType, Name asyncifyMemory)
    : analyzer(analyzer), pointerType(pointerType),
      asyncifyMemory(asyncifyMemory) {}

  void runOnFunction(Module* module_, Function* func_) override {
    module = module_;
    func = func_;
    builder =
      std::make_unique<AsyncifyBuilder>(*module, pointerType, asyncifyMemory);
    // If the function cannot change our state, we have nothing to do -
    // we will never unwind or rewind the stack here.
    if (!analyzer->needsInstrumentation(func)) {
      return;
    }
    // Rewrite the function body.
    // Each function we enter will pop one from the stack, which is the index
    // of the next call to make.
    auto* block = builder->makeBlock(
      {builder->makeIf(builder->makeStateCheck(
                         State::Rewinding), // TODO: such checks can be !normal
                       makeCallIndexPop()),
       process(func->body)});
    if (func->getResults() != Type::none) {
      // Rewriting control flow may alter things; make sure the function ends in
      // something valid (which the optimizer can remove later).
      block->list.push_back(builder->makeUnreachable());
    }
    block->finalize();
    func->body = block;
    // Making things like returns conditional may alter types.
    ReFinalize().walkFunctionInModule(func, module);
  }

private:
  std::unique_ptr<AsyncifyBuilder> builder;
  Module* module;
  Function* func;

  // Each call in the function has an index, noted during unwind and checked
  // during rewind.
  Index callIndex = 0;

  Expression* process(Expression* curr) {
    // The IR is in flat form, which makes this much simpler: there are no
    // unnecessarily nested side effects or control flow, so we can add
    // skips for rewinding in an easy manner, putting a single if around
    // whole chunks of code. Also calls are separated out so that it is easy
    // to add the necessary checks for them for unwinding/rewinding support.
    //
    // Aside from that, we must "linearize" all control flow so that we can
    // reach the right part when rewinding, which is done by always skipping
    // forward. For example, for an if we do this:
    //
    //    if (cond) { // cond is either a const or a local.get in a flat IR
    //      side1();
    //    } else {
    //      side2();
    //    }
    // =>
    //    if (rewinding || cond) {
    //      new_side1();
    //    }
    //    if (rewinding || !cond) {
    //      new_side2();
    //    }
    // where new_sideN is
    // if (normal || check_call_index(callIndex)) {
    //   sideN();
    // }
    // when sideN can change the state, and
    // if (normal) {
    //   sideN();
    // }
    // when it does not. (See makeCallSupport() for details.)
    //
    // This way we will linearly get through all the code in the function,
    // if we are rewinding. In a similar way we skip over breaks, etc.; just
    // "keep on truckin'".
    //
    // Note that temp locals added in this way are just normal locals, and in
    // particular they are saved and loaded. That way if we resume from the
    // first if arm we will avoid the second.

    // To avoid recursion, we use stacks here. We process the work for each
    // node in two phases as follows:
    //
    //  1. The "Scan" phase finds children we need to process (ones that may
    //     change the state), adds Scan tasks for them to the work stack, and
    //     process call instructions.
    //  2. The "Finish" phase runs after all children have been Scanned and
    //     Finished. It pops the children's results from the results stack (if
    //     there were relevant children), and then it pushes its own result.
    //
    struct Work {
      Expression* curr;
      enum { Scan, Finish } phase;
    };

    std::vector<Work> work;
    std::vector<Expression*> results;
    std::unordered_set<Expression*> processed;

    work.push_back(Work{curr, Work::Scan});

    while (!work.empty()) {
      auto item = work.back();
      work.pop_back();
      processed.insert(item.curr);
      auto* curr = item.curr;
      auto phase = item.phase;

      if (phase == Work::Scan && !analyzer->canChangeState(curr, func)) {
        results.push_back(makeMaybeSkip(curr));
        continue;
      }

      if (auto* block = curr->dynCast<Block>()) {
        auto& list = block->list;

        // Find the children we need to process. They will be Scanned and
        // Finished before we reach our own Finish phase.
        if (phase == Work::Scan) {
          work.push_back(Work{curr, Work::Finish});
          // Add Scan tasks in reverse order, so that we process them in
          // execution order.
          for (size_t i = list.size(); i > 0; i--) {
            auto* child = list[i - 1];
            if (analyzer->canChangeState(child, func)) {
              work.push_back(Work{child, Work::Scan});
            }
          }
          continue;
        }
        Index i = list.size() - 1;
        // At least one of our children may change the state. Clump them as
        // necessary.
        while (1) {
          if (processed.count(list[i])) {
            list[i] = results.back();
            results.pop_back();
          } else {
            Index begin = i;
            while (begin > 0 && !processed.count(list[begin - 1])) {
              begin--;
            }
            // We have a range of [begin, i] in which the state cannot change,
            // so all we need to do is skip it if rewinding.
            if (begin == i) {
              list[i] = makeMaybeSkip(list[i]);
            } else {
              auto* block = builder->makeBlock();
              for (auto j = begin; j <= i; j++) {
                block->list.push_back(list[j]);
              }
              block->finalize();
              list[begin] = makeMaybeSkip(block);
              for (auto j = begin + 1; j <= i; j++) {
                list[j] = builder->makeNop();
              }
            }
            i = begin;
          }
          if (i == 0) {
            break;
          } else {
            i--;
          }
        }
        results.push_back(block);
        continue;
      } else if (auto* iff = curr->dynCast<If>()) {
        // The state change cannot be in the condition due to flat form, so it
        // must be in one of the children.
        assert(!analyzer->canChangeState(iff->condition, func));
        if (item.phase == Work::Scan) {
          work.push_back(Work{curr, Work::Finish});
          // Add ifTrue later so that we process it first.
          if (iff->ifFalse) {
            work.push_back(Work{iff->ifFalse, Work::Scan});
          }
          work.push_back(Work{iff->ifTrue, Work::Scan});
          continue;
        }
        // We must linearize this, which means we pass through both arms if we
        // are rewinding.
        if (!iff->ifFalse) {
          iff->condition = builder->makeBinary(
            OrInt32, iff->condition, builder->makeStateCheck(State::Rewinding));
          iff->ifTrue = results.back();
          results.pop_back();
          iff->finalize();
          results.push_back(iff);
          continue;
        }
        auto* newIfFalse = results.back();
        results.pop_back();
        auto* newIfTrue = results.back();
        results.pop_back();
        auto conditionTemp = builder->addVar(func, Type::i32);
        // TODO: can avoid pre if the condition is a get or a const
        auto* pre =
          makeMaybeSkip(builder->makeLocalSet(conditionTemp, iff->condition));
        iff->condition = builder->makeLocalGet(conditionTemp, Type::i32);
        iff->condition = builder->makeBinary(
          OrInt32, iff->condition, builder->makeStateCheck(State::Rewinding));
        iff->ifTrue = newIfTrue;
        iff->ifFalse = nullptr;
        iff->finalize();
        // Add support for the second arm as well.
        auto* otherIf = builder->makeIf(
          builder->makeBinary(
            OrInt32,
            builder->makeUnary(EqZInt32,
                               builder->makeLocalGet(conditionTemp, Type::i32)),
            builder->makeStateCheck(State::Rewinding)),
          newIfFalse);
        otherIf->finalize();
        results.push_back(builder->makeBlock({pre, iff, otherIf}));
        continue;
      } else if (auto* loop = curr->dynCast<Loop>()) {
        if (item.phase == Work::Scan) {
          work.push_back(Work{curr, Work::Finish});
          work.push_back(Work{loop->body, Work::Scan});
          continue;
        }
        loop->body = results.back();
        results.pop_back();
        results.push_back(loop);
        continue;
      } else if (doesCall(curr)) {
        // We reach here only in Scan phase, but we in effect "Finish" calls
        // here as well.
        results.push_back(makeCallSupport(curr));
        continue;
      }
      // We must handle all control flow above, and all things that can change
      // the state, so there should be nothing that can reach here - add it
      // earlier as necessary.
      // std::cout << *curr << '\n';
      WASM_UNREACHABLE("unexpected expression type");
    }
    assert(results.size() == 1);
    return results.back();
  }

  // Possibly skip some code, if rewinding.
  Expression* makeMaybeSkip(Expression* curr) {
    return builder->makeIf(builder->makeStateCheck(State::Normal), curr);
  }

  Expression* makeCallSupport(Expression* curr) {
    // TODO: stop doing this after code can no longer reach a call that may
    //       change the state
    assert(doesCall(curr));
    assert(curr->type == Type::none);
    // The case of a set is tricky: we must *not* execute the set when
    // unwinding, since at that point we have a fake value for the return,
    // and if we applied it to the local, it would end up saved and then
    // potentially used later - and with coalescing, that may interfere
    // with other things. Note also that at this point in the process we
    // have not yet written the load saving/loading code, so the optimizer
    // doesn't see that locals will have another use at the beginning and
    // end of the function. We don't do that yet because we don't want it
    // to force the final number of locals to be too high, but we also
    // must be careful to never touch a local unnecessarily to avoid bugs.
    // To implement this, write to a fake global; we'll fix it up after
    // AsyncifyLocals locals adds local saving/restoring.
    auto* set = curr->dynCast<LocalSet>();
    if (set) {
      auto name = analyzer->fakeGlobals.getName(set->value->type);
      curr = builder->makeGlobalSet(name, set->value);
      set->value = builder->makeGlobalGet(name, set->value->type);
    }
    // Instrument the call itself (or, if a drop, the drop+call).
    auto index = callIndex++;
    // Execute the call, if either normal execution, or if rewinding and this
    // is the right call to go into.
    // TODO: we can read the next call index once in each function (but should
    //       avoid saving/restoring that local later)
    curr = builder->makeIf(
      builder->makeBinary(OrInt32,
                          builder->makeStateCheck(State::Normal),
                          makeCallIndexPeek(index)),
      builder->makeSequence(curr, makePossibleUnwind(index, set)));
    return curr;
  }

  Expression* makePossibleUnwind(Index index, Expression* ifNotUnwinding) {
    // In this pass we emit an "unwind" as a call to a fake intrinsic. We
    // will implement it in the later pass. (We can't create the unwind block
    // target here, as the optimizer would remove it later; we can only add
    // it when we add its contents, later.)
    return builder->makeIf(
      builder->makeStateCheck(State::Unwinding),
      builder->makeCall(
        ASYNCIFY_UNWIND, {builder->makeConst(int32_t(index))}, Type::none),
      ifNotUnwinding);
  }

  Expression* makeCallIndexPeek(Index index) {
    // Emit an intrinsic for this, as we store the index into a local, and
    // don't want it to be seen by asyncify itself.
    return builder->makeCall(ASYNCIFY_CHECK_CALL_INDEX,
                             {builder->makeConst(int32_t(index))},
                             Type::i32);
  }

  Expression* makeCallIndexPop() {
    // Emit an intrinsic for this, as we store the index into a local, and
    // don't want it to be seen by asyncify itself.
    return builder->makeCall(ASYNCIFY_GET_CALL_INDEX, {}, Type::none);
  }
};

// Add asserts in non-instrumented code.
struct AsyncifyAssertInNonInstrumented : public Pass {
  bool isFunctionParallel() override { return true; }

  ModuleAnalyzer* analyzer;
  Type pointerType;
  Name asyncifyMemory;

  std::unique_ptr<Pass> create() override {
    return std::make_unique<AsyncifyAssertInNonInstrumented>(
      analyzer, pointerType, asyncifyMemory);
  }

  AsyncifyAssertInNonInstrumented(ModuleAnalyzer* analyzer,
                                  Type pointerType,
                                  Name asyncifyMemory)
    : analyzer(analyzer), pointerType(pointerType),
      asyncifyMemory(asyncifyMemory) {}

  void runOnFunction(Module* module_, Function* func) override {
    // FIXME: This looks like it was never right, as it should ignore the top-
    //        most runtime, but it will actually instrument it (as it needs no
    //        instrumentation, like random code - but the top-most runtime is
    //        actually a place that needs neither instrumentation *nor*
    //        assertions, as the assertions will error when it changes the
    //        state).
    if (!analyzer->needsInstrumentation(func)) {
      module = module_;
      builder =
        std::make_unique<AsyncifyBuilder>(*module, pointerType, asyncifyMemory);
      addAssertsInNonInstrumented(func);
    }
  }

  // Given a function that is not instrumented - because we proved it doesn't
  // need it, or depending on the only-list / remove-list - add assertions that
  // verify that property at runtime.
  // Note that it is ok to run code while sleeping (if you are careful not
  // to break assumptions in the program!) - so what is actually
  // checked here is if the state *changes* in an uninstrumented function.
  // That is, if in an uninstrumented function, a sleep should not begin
  // from any call.
  void addAssertsInNonInstrumented(Function* func) {
    auto oldState = builder->addVar(func, Type::i32);
    // Add a check at the function entry.
    func->body = builder->makeSequence(
      builder->makeLocalSet(oldState,
                            builder->makeGlobalGet(ASYNCIFY_STATE, Type::i32)),
      func->body);
    // Add a check around every call.
    struct Walker : PostWalker<Walker> {
      void visitCall(Call* curr) {
        // Tail calls will need another type of check, as they wouldn't reach
        // this assertion.
        assert(!curr->isReturn);
        handleCall(curr);
      }
      void visitCallIndirect(CallIndirect* curr) {
        // Tail calls will need another type of check, as they wouldn't reach
        // this assertion.
        assert(!curr->isReturn);
        handleCall(curr);
      }
      void handleCall(Expression* call) {
        auto* check = builder->makeIf(
          builder->makeBinary(NeInt32,
                              builder->makeGlobalGet(ASYNCIFY_STATE, Type::i32),
                              builder->makeLocalGet(oldState, Type::i32)),
          builder->makeUnreachable());
        Expression* rep;
        if (call->type.isConcrete()) {
          auto temp = builder->addVar(func, call->type);
          rep = builder->makeBlock({
            builder->makeLocalSet(temp, call),
            check,
            builder->makeLocalGet(temp, call->type),
          });
        } else {
          rep = builder->makeSequence(call, check);
        }
        replaceCurrent(rep);
      }
      Function* func;
      AsyncifyBuilder* builder;
      Index oldState;
    };
    Walker walker;
    walker.func = func;
    walker.builder = builder.get();
    walker.oldState = oldState;
    walker.walk(func->body);
  }

private:
  std::unique_ptr<AsyncifyBuilder> builder;
  Module* module;
};

// Instrument local saving/restoring.
struct AsyncifyLocals : public WalkerPass<PostWalker<AsyncifyLocals>> {
  bool isFunctionParallel() override { return true; }

  ModuleAnalyzer* analyzer;
  Type pointerType;
  Name asyncifyMemory;

  std::unique_ptr<Pass> create() override {
    return std::make_unique<AsyncifyLocals>(
      analyzer, pointerType, asyncifyMemory);
  }

  AsyncifyLocals(ModuleAnalyzer* analyzer,
                 Type pointerType,
                 Name asyncifyMemory)
    : analyzer(analyzer), pointerType(pointerType),
      asyncifyMemory(asyncifyMemory) {}

  void visitCall(Call* curr) {
    // Replace calls to the fake intrinsics.
    if (curr->target == ASYNCIFY_UNWIND) {
      replaceCurrent(builder->makeBreak(ASYNCIFY_UNWIND, curr->operands[0]));
    } else if (curr->target == ASYNCIFY_GET_CALL_INDEX) {
      replaceCurrent(builder->makeSequence(
        builder->makeIncStackPos(-4),
        builder->makeLocalSet(rewindIndex,
                              builder->makeLoad(4,
                                                false,
                                                0,
                                                4,
                                                builder->makeGetStackPos(),
                                                Type::i32,
                                                asyncifyMemory))));
    } else if (curr->target == ASYNCIFY_CHECK_CALL_INDEX) {
      replaceCurrent(
        builder->makeBinary(EqInt32,
                            builder->makeLocalGet(rewindIndex, Type::i32),
                            curr->operands[0]));
    }
  }

  void visitGlobalSet(GlobalSet* curr) {
    auto type = analyzer->fakeGlobals.getTypeOrNone(curr->name);
    if (type != Type::none) {
      replaceCurrent(
        builder->makeLocalSet(getFakeCallLocal(type), curr->value));
    }
  }

  void visitGlobalGet(GlobalGet* curr) {
    auto type = analyzer->fakeGlobals.getTypeOrNone(curr->name);
    if (type != Type::none) {
      replaceCurrent(builder->makeLocalGet(getFakeCallLocal(type), type));
    }
  }

  Index getFakeCallLocal(Type type) {
    auto iter = fakeCallLocals.find(type);
    if (iter != fakeCallLocals.end()) {
      return iter->second;
    }
    return fakeCallLocals[type] = builder->addVar(getFunction(), type);
  }

  void doWalkFunction(Function* func) {
    // If the function cannot change our state, we have nothing to do -
    // we will never unwind or rewind the stack here.
    if (!analyzer->needsInstrumentation(func)) {
      return;
    }
    // Find the locals that we actually need to load and save: any local that is
    // alive at a relevant call site must be handled, but others can be ignored.
    findRelevantLiveLocals(func);
    // The new function body has a prelude to load locals if rewinding,
    // then the actual main body, which does all its unwindings by breaking
    // to the unwind block, which then handles pushing the call index, as
    // well as saving the locals.
    // An index is needed for getting the unwinding and rewinding call indexes
    // around TODO: can this be the same index?
    auto unwindIndex = builder->addVar(func, Type::i32);
    rewindIndex = builder->addVar(func, Type::i32);
    // Rewrite the function body.
    builder = std::make_unique<AsyncifyBuilder>(
      *getModule(), pointerType, asyncifyMemory);
    walk(func->body);
    // After the normal function body, emit a barrier before the postamble.
    Expression* barrier;
    if (func->getResults() == Type::none) {
      // The function may have ended without a return; ensure one.
      barrier = builder->makeReturn();
    } else {
      // The function must have returned or hit an unreachable, but emit one
      // to make possible bugs easier to figure out (as this should never be
      // reached). The optimizer can remove this anyhow.
      barrier = builder->makeUnreachable();
    }
    auto* newBody = builder->makeBlock(
      {builder->makeIf(builder->makeStateCheck(State::Rewinding),
                       makeLocalLoading()),
       builder->makeLocalSet(
         unwindIndex,
         builder->makeBlock(ASYNCIFY_UNWIND,
                            builder->makeSequence(func->body, barrier))),
       makeCallIndexPush(unwindIndex),
       makeLocalSaving()});
    if (func->getResults() != Type::none) {
      // If we unwind, we must still "return" a value, even if it will be
      // ignored on the outside.
      newBody->list.push_back(
        LiteralUtils::makeZero(func->getResults(), *getModule()));
      newBody->finalize(func->getResults());
    }
    func->body = newBody;
    // Making things like returns conditional may alter types.
    ReFinalize().walkFunctionInModule(func, getModule());
  }

private:
  std::unique_ptr<AsyncifyBuilder> builder;

  Index rewindIndex;
  std::unordered_map<Type, Index> fakeCallLocals;
  std::set<Index> relevantLiveLocals;

  void findRelevantLiveLocals(Function* func) {
    struct RelevantLiveLocalsWalker
      : public LivenessWalker<RelevantLiveLocalsWalker,
                              Visitor<RelevantLiveLocalsWalker>> {
      // Basic blocks that have a possible unwind/rewind in them.
      std::set<BasicBlock*> relevantBasicBlocks;

      void visitCall(Call* curr) {
        if (!currBasicBlock) {
          return;
        }
        // Note blocks where we might unwind/rewind, all of which have a
        // possible call to ASYNCIFY_CHECK_CALL_INDEX emitted right before the
        // actual call.
        // Note that each relevant original call was turned into a sequence of
        // instructions, one of which is an if and then a call to this special
        // intrinsic. We rely on the fact that if a local was live at the
        // original call, it also would be in all that sequence of instructions,
        // and in particular at the call we look for here (which is right before
        // the call, and so anything that has its final use at the call is still
        // live here).
        if (curr->target == ASYNCIFY_CHECK_CALL_INDEX) {
          relevantBasicBlocks.insert(currBasicBlock);
        }
      }
    };

    RelevantLiveLocalsWalker walker;
    walker.setFunction(func);
    walker.walkFunctionInModule(func, getModule());
    // The relevant live locals are ones that are alive at an unwind/rewind
    // location. TODO look more precisely inside basic blocks, as one might stop
    // being live in the middle
    for (auto* block : walker.liveBlocks) {
      if (walker.relevantBasicBlocks.count(block)) {
        for (auto local : block->contents.start) {
          relevantLiveLocals.insert(local);
        }
      }
    }
  }

  Expression* makeLocalLoading() {
    if (relevantLiveLocals.empty()) {
      return builder->makeNop();
    }
    auto* func = getFunction();
    auto numLocals = func->getNumLocals();
    Index total = 0;
    for (Index i = 0; i < numLocals; i++) {
      if (!relevantLiveLocals.count(i)) {
        continue;
      }
      total += getByteSize(func->getLocalType(i));
    }
    auto* block = builder->makeBlock();
    block->list.push_back(builder->makeIncStackPos(-total));
    auto tempIndex = builder->addVar(func, builder->pointerType);
    block->list.push_back(
      builder->makeLocalSet(tempIndex, builder->makeGetStackPos()));
    Index offset = 0;
    for (Index i = 0; i < numLocals; i++) {
      if (!relevantLiveLocals.count(i)) {
        continue;
      }
      auto localType = func->getLocalType(i);
      SmallVector<Expression*, 1> loads;
      for (const auto& type : localType) {
        auto size = getByteSize(type);
        assert(size % STACK_ALIGN == 0);
        // TODO: higher alignment?
        loads.push_back(builder->makeLoad(
          size,
          true,
          offset,
          STACK_ALIGN,
          builder->makeLocalGet(tempIndex, builder->pointerType),
          type,
          asyncifyMemory));
        offset += size;
      }
      Expression* load;
      if (loads.size() == 1) {
        load = loads[0];
      } else if (localType.size() > 1) {
        load = builder->makeTupleMake(std::move(loads));
      } else {
        WASM_UNREACHABLE("Unexpected empty type");
      }
      block->list.push_back(builder->makeLocalSet(i, load));
    }
    block->finalize();
    return block;
  }

  Expression* makeLocalSaving() {
    if (relevantLiveLocals.empty()) {
      return builder->makeNop();
    }
    auto* func = getFunction();
    auto numLocals = func->getNumLocals();
    auto* block = builder->makeBlock();
    auto tempIndex = builder->addVar(func, builder->pointerType);
    block->list.push_back(
      builder->makeLocalSet(tempIndex, builder->makeGetStackPos()));
    Index offset = 0;
    for (Index i = 0; i < numLocals; i++) {
      if (!relevantLiveLocals.count(i)) {
        continue;
      }
      auto localType = func->getLocalType(i);
      size_t j = 0;
      for (const auto& type : localType) {
        auto size = getByteSize(type);
        Expression* localGet = builder->makeLocalGet(i, localType);
        if (localType.size() > 1) {
          localGet = builder->makeTupleExtract(localGet, j);
        }
        assert(size % STACK_ALIGN == 0);
        // TODO: higher alignment?
        block->list.push_back(builder->makeStore(
          size,
          offset,
          STACK_ALIGN,
          builder->makeLocalGet(tempIndex, builder->pointerType),
          localGet,
          type,
          asyncifyMemory));
        offset += size;
        ++j;
      }
    }
    block->list.push_back(builder->makeIncStackPos(offset));
    block->finalize();
    return block;
  }

  Expression* makeCallIndexPush(Index tempIndex) {
    // TODO: add a check against the stack end here
    return builder->makeSequence(
      builder->makeStore(4,
                         0,
                         4,
                         builder->makeGetStackPos(),
                         builder->makeLocalGet(tempIndex, Type::i32),
                         Type::i32,
                         asyncifyMemory),
      builder->makeIncStackPos(4));
  }

  unsigned getByteSize(Type type) {
    if (!type.hasByteSize()) {
      Fatal() << "Asyncify does not yet support non-number types, like "
                 "references (see "
                 "https://github.com/WebAssembly/binaryen/issues/3739)";
    }
    return type.getByteSize();
  }
};

} // anonymous namespace

static std::string getFullImportName(Name module, Name base) {
  return std::string(module.str) + '.' + base.toString();
}

struct Asyncify : public Pass {
  // Adds calls.
  bool addsEffects() override { return true; }

  void run(Module* module) override {
    bool optimize = getPassOptions().optimizeLevel > 0;

    // Find which things can change the state.
    auto stateChangingImports = String::trim(read_possible_response_file(
      getArgumentOrDefault("asyncify-imports", "")));
    auto ignoreImports = getArgumentOrDefault("asyncify-ignore-imports", "");
    bool allImportsCanChangeState =
      stateChangingImports == "" && ignoreImports == "";
    String::Split listedImports(stateChangingImports,
                                String::Split::NewLineOr(","));
    // canIndirectChangeState is the default.  asyncify-ignore-indirect sets it
    // to false.
    auto canIndirectChangeState = !hasArgument("asyncify-ignore-indirect");
    std::string removeListInput =
      getArgumentOrDefault("asyncify-removelist", "");
    if (removeListInput.empty()) {
      // Support old name for now to avoid immediate breakage TODO remove
      removeListInput = getArgumentOrDefault("asyncify-blacklist", "");
    }
    String::Split removeList(
      String::trim(read_possible_response_file(removeListInput)),
      String::Split::NewLineOr(","));
    String::Split addList(String::trim(read_possible_response_file(
                            getArgumentOrDefault("asyncify-addlist", ""))),
                          String::Split::NewLineOr(","));
    std::string onlyListInput = getArgumentOrDefault("asyncify-onlylist", "");
    if (onlyListInput.empty()) {
      // Support old name for now to avoid immediate breakage TODO remove
      onlyListInput = getArgumentOrDefault("asyncify-whitelist", "");
    }
    String::Split onlyList(
      String::trim(read_possible_response_file(onlyListInput)),
      String::Split::NewLineOr(","));
    auto asserts = hasArgument("asyncify-asserts");
    auto verbose = hasArgument("asyncify-verbose");
    auto relocatable = hasArgument("asyncify-relocatable");
    auto secondaryMemory = hasArgument("asyncify-in-secondary-memory");
    auto propagateAddList = hasArgument("asyncify-propagate-addlist");

    // Ensure there is a memory, as we need it.

    if (secondaryMemory) {
      auto secondaryMemorySizeString =
        getArgumentOrDefault("asyncify-secondary-memory-size", "1");
      Address secondaryMemorySize = std::stoi(secondaryMemorySizeString);
      asyncifyMemory = createSecondaryMemory(module, secondaryMemorySize);
    } else {
      if (module->memories.size() <= 1) {
        MemoryUtils::ensureExists(module);
        asyncifyMemory = module->memories[0]->name;
      } else {
        auto asyncifyMemoryValue =
          getArgumentOrDefault("asyncify-memory", "memory");
        for (auto& theExport : module->exports) {
          if (theExport->kind == ExternalKind::Memory &&
              theExport->name == asyncifyMemoryValue) {
            asyncifyMemory = theExport->value;
            break;
          }
        }
        if (!asyncifyMemory) {
          Fatal() << "Please specify which of the multiple memories to use, "
                     "with --pass-arg=asyncify-memory@memory";
        }
      }
    }
    pointerType =
      module->getMemory(asyncifyMemory)->is64() ? Type::i64 : Type::i32;

    removeList = handleBracketingOperators(removeList);
    addList = handleBracketingOperators(addList);
    onlyList = handleBracketingOperators(onlyList);

    if (!onlyList.empty() && (!removeList.empty() || !addList.empty())) {
      Fatal() << "It makes no sense to use both an asyncify only-list together "
                 "with another list.";
    }

    auto canImportChangeState = [&](Name module, Name base) {
      if (allImportsCanChangeState) {
        return true;
      }
      auto full = getFullImportName(module, base);
      for (auto& listedImport : listedImports) {
        if (String::wildcardMatch(listedImport, full)) {
          return true;
        }
      }
      return false;
    };

    // Scan the module.
    ModuleAnalyzer analyzer(*module,
                            canImportChangeState,
                            canIndirectChangeState,
                            removeList,
                            addList,
                            propagateAddList,
                            onlyList,
                            verbose);

    // Add necessary globals before we emit code to use them.
    addGlobals(module, relocatable);

    // Compute the set of functions we will instrument. All of the passes we run
    // below only need to run there.
    PassUtils::FuncSet instrumentedFuncs;
    for (auto& func : module->functions) {
      if (analyzer.needsInstrumentation(func.get())) {
        instrumentedFuncs.insert(func.get());
      }
    }

    // Instrument the flow of code, adding code instrumentation and
    // skips for when rewinding. We do this on flat IR so that it is
    // practical to add code around each call, without affecting
    // anything else.
    {
      PassUtils::FilteredPassRunner runner(module, instrumentedFuncs);
      runner.add("flatten");
      // Dce is useful here, since AsyncifyFlow makes control flow conditional,
      // which may make unreachable code look reachable. It also lets us ignore
      // unreachable code here.
      runner.add("dce");
      if (optimize) {
        // Optimizing before AsyncifyFlow is crucial, especially coalescing,
        // because the flow changes add many branches, break up if-elses, etc.,
        // all of which extend the live ranges of locals. In other words, it is
        // not possible to coalesce well afterwards.
        runner.add("remove-unused-names");
        runner.add("simplify-locals-nonesting");
        runner.add("reorder-locals");
        runner.add("coalesce-locals");
        runner.add("simplify-locals-nonesting");
        runner.add("reorder-locals");
        runner.add("merge-blocks");
      }
      runner.add(
        std::make_unique<AsyncifyFlow>(&analyzer, pointerType, asyncifyMemory));
      runner.setIsNested(true);
      runner.setValidateGlobally(false);
      runner.run();
    }
    if (asserts) {
      // Add asserts in non-instrumented code. Note we do not use an
      // instrumented pass runner here as we do want to run on all functions.
      PassRunner runner(module);
      runner.add(std::make_unique<AsyncifyAssertInNonInstrumented>(
        &analyzer, pointerType, asyncifyMemory));
      runner.setIsNested(true);
      runner.setValidateGlobally(false);
      runner.run();
    }
    // Next, add local saving/restoring logic. We optimize before doing this,
    // to undo the extra code generated by flattening, and to arrive at the
    // minimal amount of locals (which is important as we must save and
    // restore those locals). We also and optimize after as well to simplify
    // the code as much as possible.
    {
      PassUtils::FilteredPassRunner runner(module, instrumentedFuncs);
      if (optimize) {
        runner.addDefaultFunctionOptimizationPasses();
      }
      runner.add(std::make_unique<AsyncifyLocals>(
        &analyzer, pointerType, asyncifyMemory));
      if (optimize) {
        runner.addDefaultFunctionOptimizationPasses();
      }
      runner.setIsNested(true);
      runner.setValidateGlobally(false);
      runner.run();
    }
    // Finally, add function support (that should not have been seen by
    // the previous passes).
    addFunctions(module);
  }

private:
  void addGlobals(Module* module, bool imported) {
    Builder builder(*module);

    auto asyncifyState = builder.makeGlobal(ASYNCIFY_STATE,
                                            Type::i32,
                                            builder.makeConst(int32_t(0)),
                                            Builder::Mutable);
    if (imported) {
      asyncifyState->module = ENV;
      asyncifyState->base = ASYNCIFY_STATE;
    }
    module->addGlobal(std::move(asyncifyState));

    auto asyncifyData = builder.makeGlobal(ASYNCIFY_DATA,
                                           pointerType,
                                           builder.makeConst(pointerType),
                                           Builder::Mutable);
    if (imported) {
      asyncifyData->module = ENV;
      asyncifyData->base = ASYNCIFY_DATA;
    }
    module->addGlobal(std::move(asyncifyData));
  }

  void addFunctions(Module* module) {
    Builder builder(*module);
    auto makeFunction = [&](Name name, bool setData, State state) {
      std::vector<Type> params;
      if (setData) {
        params.push_back(pointerType);
      }
      auto* body = builder.makeBlock();
      body->list.push_back(builder.makeGlobalSet(
        ASYNCIFY_STATE, builder.makeConst(int32_t(state))));
      if (setData) {
        body->list.push_back(builder.makeGlobalSet(
          ASYNCIFY_DATA, builder.makeLocalGet(0, pointerType)));
      }
      // Verify the data is valid.
      auto* stackPos =
        builder.makeLoad(pointerType.getByteSize(),
                         false,
                         int(DataOffset::BStackPos),
                         pointerType.getByteSize(),
                         builder.makeGlobalGet(ASYNCIFY_DATA, pointerType),
                         pointerType,
                         asyncifyMemory);
      auto* stackEnd =
        builder.makeLoad(pointerType.getByteSize(),
                         false,
                         int(pointerType == Type::i64 ? DataOffset::BStackEnd64
                                                      : DataOffset::BStackEnd),
                         pointerType.getByteSize(),
                         builder.makeGlobalGet(ASYNCIFY_DATA, pointerType),
                         pointerType,
                         asyncifyMemory);
      body->list.push_back(builder.makeIf(
        builder.makeBinary(
          Abstract::getBinary(pointerType, Abstract::GtU), stackPos, stackEnd),
        builder.makeUnreachable()));
      body->finalize();
      auto func = builder.makeFunction(
        name, Signature(Type(params), Type::none), {}, body);
      module->addFunction(std::move(func));
      module->addExport(builder.makeExport(name, name, ExternalKind::Function));
    };

    makeFunction(ASYNCIFY_START_UNWIND, true, State::Unwinding);
    makeFunction(ASYNCIFY_STOP_UNWIND, false, State::Normal);
    makeFunction(ASYNCIFY_START_REWIND, true, State::Rewinding);
    makeFunction(ASYNCIFY_STOP_REWIND, false, State::Normal);

    module->addFunction(
      builder.makeFunction(ASYNCIFY_GET_STATE,
                           Signature(Type::none, Type::i32),
                           {},
                           builder.makeGlobalGet(ASYNCIFY_STATE, Type::i32)));
    module->addExport(builder.makeExport(
      ASYNCIFY_GET_STATE, ASYNCIFY_GET_STATE, ExternalKind::Function));
  }

  Name createSecondaryMemory(Module* module, Address secondaryMemorySize) {
    Name name = Names::getValidMemoryName(*module, "asyncify_memory");
    auto secondaryMemory =
      Builder::makeMemory(name, secondaryMemorySize, secondaryMemorySize);
    module->addMemory(std::move(secondaryMemory));
    return name;
  }

  Type pointerType;
  Name asyncifyMemory;
};

Pass* createAsyncifyPass() { return new Asyncify(); }

// Helper passes that can be run after Asyncify.

template<bool neverRewind, bool neverUnwind, bool importsAlwaysUnwind>
struct ModAsyncify
  : public WalkerPass<LinearExecutionWalker<
      ModAsyncify<neverRewind, neverUnwind, importsAlwaysUnwind>>> {
  bool isFunctionParallel() override { return true; }

  std::unique_ptr<Pass> create() override {
    return std::make_unique<
      ModAsyncify<neverRewind, neverUnwind, importsAlwaysUnwind>>();
  }

  void doWalkFunction(Function* func) {
    // Find the asyncify state name.
    auto* unwind = this->getModule()->getExport(ASYNCIFY_STOP_UNWIND);
    auto* unwindFunc = this->getModule()->getFunction(unwind->value);
    FindAll<GlobalSet> sets(unwindFunc->body);
    assert(sets.list.size() == 1);
    asyncifyStateName = sets.list[0]->name;
    // Walk and optimize.
    this->walk(func->body);
  }

  // Note that we don't just implement GlobalGet as we may know the value is
  // *not* 0, 1, or 2, but not know the actual value. So what we can say depends
  // on the comparison being done on it, and so we implement Binary and
  // Select.

  void visitBinary(Binary* curr) {
    // Check if this is a comparison of the asyncify state to a specific
    // constant, which we may know is impossible.
    bool flip = false;
    if (curr->op == NeInt32) {
      flip = true;
    } else if (curr->op != EqInt32) {
      return;
    }
    auto* c = curr->right->dynCast<Const>();
    if (!c) {
      return;
    }
    auto* get = curr->left->dynCast<GlobalGet>();
    if (!get || get->name != asyncifyStateName) {
      return;
    }
    // This is a comparison of the state to a constant, check if we know the
    // value.
    int32_t value;
    auto checkedValue = c->value.geti32();
    if ((checkedValue == int(State::Unwinding) && neverUnwind) ||
        (checkedValue == int(State::Rewinding) && neverRewind)) {
      // We know the state is checked against an impossible value.
      value = 0;
    } else if (checkedValue == int(State::Unwinding) && this->unwinding) {
      // We know we are in fact unwinding right now.
      value = 1;
      unsetUnwinding();
    } else {
      return;
    }
    if (flip) {
      value = 1 - value;
    }
    Builder builder(*this->getModule());
    this->replaceCurrent(builder.makeConst(int32_t(value)));
  }

  void visitSelect(Select* curr) {
    auto* get = curr->condition->dynCast<GlobalGet>();
    if (!get || get->name != asyncifyStateName) {
      return;
    }
    // This is a comparison of the normal state, which means we are checking
    // "if running normally, run this code, but if rewinding, ignore it". If
    // we know we'll never rewind, we can optimize this.
    if (neverRewind) {
      Builder builder(*this->getModule());
      curr->condition = builder.makeConst(int32_t(0));
    }
  }

  void visitUnary(Unary* curr) {
    if (curr->op != EqZInt32) {
      return;
    }
    auto* get = curr->value->dynCast<GlobalGet>();
    if (!get || get->name != asyncifyStateName) {
      return;
    }
    // This is a comparison of the state to zero, which means we are checking
    // "if running normally, run this code, but if rewinding, ignore it". If
    // we know we'll never rewind, we can optimize this.
    if (neverRewind) {
      Builder builder(*this->getModule());
      // The whole expression will be 1 because it is (i32.eqz (i32.const 0))
      this->replaceCurrent(builder.makeConst(int32_t(1)));
    }
  }

  void visitCall(Call* curr) {
    unsetUnwinding();
    if (!importsAlwaysUnwind) {
      return;
    }
    auto* target = this->getModule()->getFunction(curr->target);
    if (!target->imported()) {
      return;
    }
    // This is an import that definitely unwinds. Await the next check of
    // the state in this linear execution trace, which we can turn into a
    // constant.
    this->unwinding = true;
  }

  void visitCallIndirect(CallIndirect* curr) { unsetUnwinding(); }

  static void doNoteNonLinear(
    ModAsyncify<neverRewind, neverUnwind, importsAlwaysUnwind>* self,
    Expression**) {
    // When control flow branches, stop tracking an unwinding.
    self->unsetUnwinding();
  }

  void visitGlobalSet(GlobalSet* set) {
    // TODO: this could be more precise
    unsetUnwinding();
  }

private:
  Name asyncifyStateName;

  // Whether we just did a call to an import that indicates we are unwinding.
  bool unwinding = false;

  void unsetUnwinding() { this->unwinding = false; }
};

//
// Assume imports that may unwind will always unwind, and that rewinding never
// happens.
//

Pass* createModAsyncifyAlwaysOnlyUnwindPass() {
  return new ModAsyncify<true, false, true>();
}

//
// Assume that we never unwind, but may still rewind.
//
struct ModAsyncifyNeverUnwind : public Pass {
  void run(Module* module) override {}
};

Pass* createModAsyncifyNeverUnwindPass() {
  return new ModAsyncify<false, true, false>();
}

} // namespace wasm
