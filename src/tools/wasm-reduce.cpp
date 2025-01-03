/*
 * Copyright 2017 WebAssembly Community Group participants
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
// Tries to reduce the input wasm into the smallest possible wasm
// that still generates the same result on a given command. This is
// useful to reduce bug testcases, for example, if a file crashes
// the optimizer, you can reduce it to find the smallest file that
// also crashes it (which generally will show the same bug, in a
// much more debuggable manner).
//

#include <cstdio>
#include <cstdlib>
#include <memory>

#include "ir/branch-utils.h"
#include "ir/iteration.h"
#include "ir/literal-utils.h"
#include "ir/properties.h"
#include "ir/utils.h"
#include "pass.h"
#include "support/colors.h"
#include "support/command-line.h"
#include "support/file.h"
#include "support/hash.h"
#include "support/path.h"
#include "support/timing.h"
#include "tool-options.h"
#include "wasm-builder.h"
#include "wasm-io.h"
#include "wasm-validator.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// Create a string with last error message
std::string GetLastErrorStdStr() {
  DWORD error = GetLastError();
  if (error) {
    LPVOID lpMsgBuf;
    DWORD bufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                   FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,
                                 error,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPTSTR)&lpMsgBuf,
                                 0,
                                 NULL);
    if (bufLen) {
      LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
      std::string result(lpMsgStr, lpMsgStr + bufLen);
      LocalFree(lpMsgBuf);
      return result;
    }
  }
  return std::string();
}
#endif

using namespace wasm;

// A timeout on every execution of the command.
static size_t timeout = 2;

// A string of feature flags and other things to pass while reducing. The
// default of enabling all features should work in most cases.
static std::string extraFlags = "-all";

// Whether to save all intermediate working files as we go.
static bool saveAllWorkingFiles = false;

struct ProgramResult {
  int code;
  std::string output;
  double time;

  ProgramResult() = default;
  ProgramResult(std::string command) { getFromExecution(command); }

#ifdef _WIN32
  void getFromExecution(std::string command) {
    Timer timer;
    timer.start();
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hChildStd_OUT_Rd;
    HANDLE hChildStd_OUT_Wr;

    if (
      // Create a pipe for the child process's STDOUT.
      !CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0) ||
      // Ensure the read handle to the pipe for STDOUT is not inherited.
      !SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
      Fatal() << "CreatePipe \"" << command
              << "\" failed: " << GetLastErrorStdStr() << ".\n";
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hChildStd_OUT_Wr;
    si.hStdOutput = hChildStd_OUT_Wr;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process.
    if (!CreateProcess(NULL, // No module name (use command line)
                       (LPSTR)command.c_str(), // Command line
                       NULL,                   // Process handle not inheritable
                       NULL,                   // Thread handle not inheritable
                       TRUE,                   // Set handle inheritance to TRUE
                       0,                      // No creation flags
                       NULL,                   // Use parent's environment block
                       NULL, // Use parent's starting directory
                       &si,  // Pointer to STARTUPINFO structure
                       &pi)  // Pointer to PROCESS_INFORMATION structure
    ) {
      Fatal() << "CreateProcess \"" << command
              << "\" failed: " << GetLastErrorStdStr() << ".\n";
    }

    // Wait until child process exits.
    DWORD retVal = WaitForSingleObject(pi.hProcess, timeout * 1000);
    if (retVal == WAIT_TIMEOUT) {
      printf("Command timeout: %s", command.c_str());
      TerminateProcess(pi.hProcess, -1);
    }
    DWORD dwordExitCode;
    if (!GetExitCodeProcess(pi.hProcess, &dwordExitCode)) {
      Fatal() << "GetExitCodeProcess failed: " << GetLastErrorStdStr() << ".\n";
    }
    code = (int)dwordExitCode;

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Read output from the child process's pipe for STDOUT
    // Stop when there is no more data.
    {
      const int BUFSIZE = 4096;
      DWORD dwRead, dwTotal, dwTotalRead = 0;
      CHAR chBuf[BUFSIZE];
      BOOL bSuccess = FALSE;

      PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &dwTotal, NULL);
      while (dwTotalRead < dwTotal) {
        bSuccess =
          ReadFile(hChildStd_OUT_Rd, chBuf, BUFSIZE - 1, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
          break;
        chBuf[dwRead] = 0;
        dwTotalRead += dwRead;
        output.append(chBuf);
      }
    }
    timer.stop();
    time = timer.getTotal();
  }
#else  // POSIX
  // runs the command and notes the output
  // TODO: also stderr, not just stdout?
  void getFromExecution(std::string command) {
    Timer timer;
    timer.start();
    // do this using just core stdio.h and stdlib.h, for portability
    // sadly this requires two invokes
    code = system(("timeout " + std::to_string(timeout) + "s " + command +
                   " > /dev/null 2> /dev/null")
                    .c_str());
    const int MAX_BUFFER = 1024;
    char buffer[MAX_BUFFER];
    FILE* stream = popen(
      ("timeout " + std::to_string(timeout) + "s " + command + " 2> /dev/null")
        .c_str(),
      "r");
    while (fgets(buffer, MAX_BUFFER, stream) != NULL) {
      output.append(buffer);
    }
    pclose(stream);
    timer.stop();
    time = timer.getTotal() / 2;
  }
#endif // _WIN32

  bool operator==(ProgramResult& other) {
    return code == other.code && output == other.output;
  }
  bool operator!=(ProgramResult& other) { return !(*this == other); }

  bool failed() { return code != 0; }

  void dump(std::ostream& o) {
    o << "[ProgramResult] code: " << code << " stdout: \n"
      << output << "[====]\nin " << time << " seconds\n[/ProgramResult]\n";
  }
};

namespace std {

inline std::ostream& operator<<(std::ostream& o, ProgramResult& result) {
  result.dump(o);
  return o;
}

} // namespace std

ProgramResult expected;

// Removing functions is extremely beneficial and efficient. We aggressively
// try to remove functions, unless we've seen they can't be removed, in which
// case we may try again but much later.
static std::unordered_set<Name> functionsWeTriedToRemove;

// The index of the working file we save, when saveAllWorkingFiles. We must
// store this globally so that the difference instances of Reducer do not
// overlap.
static size_t workingFileIndex = 0;

struct Reducer
  : public WalkerPass<PostWalker<Reducer, UnifiedExpressionVisitor<Reducer>>> {
  std::string command, test, working;
  bool binary, deNan, verbose, debugInfo;
  ToolOptions& toolOptions;

  // test is the file we write to that the command will operate on
  // working is the current temporary state, the reduction so far
  Reducer(std::string command,
          std::string test,
          std::string working,
          bool binary,
          bool deNan,
          bool verbose,
          bool debugInfo,
          ToolOptions& toolOptions)
    : command(command), test(test), working(working), binary(binary),
      deNan(deNan), verbose(verbose), debugInfo(debugInfo),
      toolOptions(toolOptions) {}

  // runs passes in order to reduce, until we can't reduce any more
  // the criterion here is wasm binary size
  void reduceUsingPasses() {
    // run optimization passes until we can't shrink it any more
    std::vector<std::string> passes = {
      // Optimization modes.
      "-Oz",
      "-Os",
      "-O1",
      "-O2",
      "-O3",
      "-O4",
      // Optimization modes + passes that work well with them.
      "--flatten -Os",
      "--flatten -O3",
      "--flatten --simplify-locals-notee-nostructure --local-cse -Os",
      "--type-ssa -Os --type-merging",
      "--gufa -O1",
      // Individual passes or combinations of them.
      "--coalesce-locals --vacuum",
      "--dae",
      "--dae-optimizing",
      "--dce",
      "--duplicate-function-elimination",
      "--enclose-world",
      "--gto",
      "--inlining",
      "--inlining-optimizing",
      "--optimize-level=3 --inlining-optimizing",
      "--local-cse",
      "--memory-packing",
      "--remove-unused-names --merge-blocks --vacuum",
      "--optimize-instructions",
      "--precompute",
      "--remove-imports",
      "--remove-memory",
      "--remove-unused-names --remove-unused-brs",
      "--remove-unused-module-elements",
      "--remove-unused-nonfunction-module-elements",
      "--reorder-functions",
      "--reorder-locals",
      // TODO: signature* passes
      "--simplify-globals",
      "--simplify-locals --vacuum",
      "--strip",
      "--remove-unused-types --closed-world",
      "--vacuum"};
    auto oldSize = file_size(working);
    bool more = true;
    while (more) {
      // std::cerr << "|    starting passes loop iteration\n";
      more = false;
      // try both combining with a generic shrink (so minor pass overhead is
      // compensated for), and without
      for (auto pass : passes) {
        std::string currCommand = Path::getBinaryenBinaryTool("wasm-opt") + " ";
        currCommand += working + " -o " + test + " " + pass + " " + extraFlags;
        if (!binary) {
          currCommand += " -S ";
        }
        if (verbose) {
          std::cerr << "|    trying pass command: " << currCommand << "\n";
        }
        if (!ProgramResult(currCommand).failed()) {
          auto newSize = file_size(test);
          if (newSize < oldSize) {
            // the pass didn't fail, and the size looks smaller, so promising
            // see if it is still has the property we are preserving
            if (ProgramResult(command) == expected) {
              std::cerr << "|    command \"" << currCommand
                        << "\" succeeded, reduced size to " << newSize << '\n';
              applyTestToWorking();
              more = true;
              oldSize = newSize;
            }
          }
        }
      }
    }
    if (verbose) {
      std::cerr << "|    done with passes for now\n";
    }
  }

  // Apply the test file to the working file, after we saw that it successfully
  // reduced the testcase.
  void applyTestToWorking() {
    copy_file(test, working);

    if (saveAllWorkingFiles) {
      copy_file(working, working + '.' + std::to_string(workingFileIndex++));
    }
  }

  // does one pass of slow and destructive reduction. returns whether it
  // succeeded or not
  // the criterion here is a logical change in the program. this may actually
  // increase wasm size in some cases, but it should allow more reduction later.
  // @param factor how much to ignore. starting with a high factor skips through
  //               most of the file, which is often faster than going one by one
  //               from the start
  size_t reduceDestructively(int factor_) {
    factor = factor_;
    // prepare
    loadWorking();
    reduced = 0;
    funcsSeen = 0;
    // Before we do any changes, it should be valid to write out the module:
    // size should be as expected, and output should be as expected.
    ProgramResult result;
    if (!writeAndTestReduction(result)) {
      std::cerr << "\n|! WARNING: writing before destructive reduction fails, "
                   "very unlikely reduction can work\n"
                << result << '\n';
    }
    // destroy!
    walkModule(getModule());
    return reduced;
  }

  void loadWorking() {
    module = std::make_unique<Module>();

    toolOptions.applyOptionsBeforeParse(*module);

    ModuleReader reader;
    try {
      reader.read(working, *module);
    } catch (ParseException& p) {
      p.dump(std::cerr);
      std::cerr << '\n';
      Fatal() << "error in parsing working wasm binary";
    }

    toolOptions.applyOptionsAfterParse(*module);

    // If there is no features section, assume we may need them all (without
    // this, a module with no features section but that uses e.g. atomics and
    // bulk memory would not work).
    if (!module->hasFeaturesSection) {
      module->features = FeatureSet::All;
    }
    builder = std::make_unique<Builder>(*module);
    setModule(module.get());
  }

  // destructive reduction state

  size_t reduced;
  Expression* beforeReduction;
  std::unique_ptr<Module> module;
  std::unique_ptr<Builder> builder;
  Index funcsSeen;
  int factor;

  // write the module and see if the command still fails on it as expected
  bool writeAndTestReduction() {
    ProgramResult result;
    return writeAndTestReduction(result);
  }

  bool writeAndTestReduction(ProgramResult& out) {
    // write the module out
    ModuleWriter writer(toolOptions.passOptions);
    writer.setBinary(binary);
    writer.setDebugInfo(debugInfo);
    writer.write(*getModule(), test);
    // note that it is ok for the destructively-reduced module to be bigger
    // than the previous - each destructive reduction removes logical code,
    // and so is strictly better, even if the wasm binary format happens to
    // encode things slightly less efficiently.
    // test it
    out.getFromExecution(command);
    return out == expected;
  }

  size_t decisionCounter = 0;

  bool shouldTryToReduce(size_t bonus = 1) {
    assert(bonus > 0);
    // Increment to avoid returning the same result each time.
    decisionCounter += bonus;
    return (decisionCounter % factor) <= bonus;
  }

  // Returns a random number in the range [0, max). This is deterministic given
  // all the previous work done in the reducer.
  size_t deterministicRandom(size_t max) {
    assert(max > 0);
    hash_combine(decisionCounter, max);
    return decisionCounter % max;
  }

  bool isOkReplacement(Expression* with) {
    if (deNan) {
      if (auto* c = with->dynCast<Const>()) {
        if (c->value.isNaN()) {
          return false;
        }
      }
    }
    return true;
  }

  // tests a reduction on the current traversal node, and undos if it failed
  bool tryToReplaceCurrent(Expression* with) {
    if (!isOkReplacement(with)) {
      return false;
    }
    auto* curr = getCurrent();
    // std::cerr << "try " << curr << " => " << with << '\n';
    if (curr->type != with->type) {
      return false;
    }
    if (!shouldTryToReduce()) {
      return false;
    }
    replaceCurrent(with);
    if (!writeAndTestReduction()) {
      replaceCurrent(curr);
      return false;
    }
    std::cerr << "|      tryToReplaceCurrent succeeded (in " << getLocation()
              << ")\n";
    noteReduction();
    return true;
  }

  void noteReduction(size_t amount = 1) {
    reduced += amount;
    applyTestToWorking();
  }

  // tests a reduction on an arbitrary child
  bool tryToReplaceChild(Expression*& child, Expression* with) {
    if (!isOkReplacement(with)) {
      return false;
    }
    if (child->type != with->type) {
      return false;
    }
    if (!shouldTryToReduce()) {
      return false;
    }
    auto* before = child;
    child = with;
    if (!writeAndTestReduction()) {
      child = before;
      return false;
    }
    std::cerr << "|      tryToReplaceChild succeeded (in " << getLocation()
              << ")\n";
    // std::cerr << "|      " << before << " => " << with << '\n';
    noteReduction();
    return true;
  }

  std::string getLocation() {
    if (getFunction()) {
      return getFunction()->name.toString();
    }
    return "(non-function context)";
  }

  // visitors. in each we try to remove code in a destructive and nontrivial
  // way. "nontrivial" means something that optimization passes can't achieve,
  // since we don't need to duplicate work that they do

  void visitExpression(Expression* curr) {
    // type-based reductions
    if (curr->type == Type::none) {
      if (tryToReduceCurrentToNop()) {
        return;
      }
    } else if (curr->type.isConcrete()) {
      if (tryToReduceCurrentToConst()) {
        return;
      }
    } else {
      assert(curr->type == Type::unreachable);
      if (tryToReduceCurrentToUnreachable()) {
        return;
      }
    }
    // specific reductions
    if (auto* iff = curr->dynCast<If>()) {
      if (iff->type == Type::none) {
        // perhaps we need just the condition?
        if (tryToReplaceCurrent(builder->makeDrop(iff->condition))) {
          return;
        }
      }
      handleCondition(iff->condition);
    } else if (auto* br = curr->dynCast<Break>()) {
      handleCondition(br->condition);
    } else if (auto* select = curr->dynCast<Select>()) {
      handleCondition(select->condition);
    } else if (auto* sw = curr->dynCast<Switch>()) {
      handleCondition(sw->condition);
      // Try to replace switch targets with the default
      for (auto& target : sw->targets) {
        if (target != sw->default_) {
          auto old = target;
          target = sw->default_;
          if (!tryToReplaceCurrent(curr)) {
            target = old;
          }
        }
      }
      // Try to shorten the list of targets.
      while (sw->targets.size() > 1) {
        auto last = sw->targets.back();
        sw->targets.pop_back();
        if (!tryToReplaceCurrent(curr)) {
          sw->targets.push_back(last);
          break;
        }
      }
    } else if (auto* block = curr->dynCast<Block>()) {
      if (!shouldTryToReduce()) {
        return;
      }
      // replace a singleton
      auto& list = block->list;
      if (list.size() == 1 &&
          !BranchUtils::BranchSeeker::has(block, block->name)) {
        if (tryToReplaceCurrent(block->list[0])) {
          return;
        }
      }
      // try to get rid of nops
      Index i = 0;
      while (list.size() > 1 && i < list.size()) {
        auto* curr = list[i];
        if (curr->is<Nop>() && shouldTryToReduce()) {
          // try to remove it
          for (Index j = i; j < list.size() - 1; j++) {
            list[j] = list[j + 1];
          }
          list.pop_back();
          if (writeAndTestReduction()) {
            std::cerr << "|      block-nop removed\n";
            noteReduction();
            return;
          }
          list.push_back(nullptr);
          // we failed; undo
          for (Index j = list.size() - 1; j > i; j--) {
            list[j] = list[j - 1];
          }
          list[i] = curr;
        }
        i++;
      }
      return; // nothing more to do
    } else if (auto* loop = curr->dynCast<Loop>()) {
      if (shouldTryToReduce() &&
          !BranchUtils::BranchSeeker::has(loop, loop->name)) {
        tryToReplaceCurrent(loop->body);
      }
      return; // nothing more to do
    } else if (curr->is<Drop>()) {
      if (curr->type == Type::none) {
        // We can't improve this: the child has a different type than us. Return
        // here to avoid reaching the code below that tries to add a drop on
        // children (which would recreate the current state).
        return;
      }
    } else if (auto* structNew = curr->dynCast<StructNew>()) {
      // If all the fields are defaultable, try to replace this with a
      // struct.new_with_default.
      if (!structNew->isWithDefault() && structNew->type != Type::unreachable) {
        auto& fields = structNew->type.getHeapType().getStruct().fields;
        if (std::all_of(fields.begin(), fields.end(), [&](auto& field) {
              return field.type.isDefaultable();
            })) {
          ExpressionList operands(getModule()->allocator);
          operands.swap(structNew->operands);
          assert(structNew->isWithDefault());
          if (tryToReplaceCurrent(structNew)) {
            return;
          } else {
            structNew->operands.swap(operands);
            assert(!structNew->isWithDefault());
          }
        }
      }
    }
    // Finally, try to replace with a child.
    for (auto* child : ChildIterator(curr)) {
      if (child->type.isConcrete() && curr->type == Type::none) {
        if (tryToReplaceCurrent(builder->makeDrop(child))) {
          return;
        }
      } else {
        if (tryToReplaceCurrent(child)) {
          return;
        }
      }
    }
    // If that didn't work, try to replace with a child + a unary conversion,
    // but not if it's already unary
    if (curr->type.isSingle() && !curr->is<Unary>()) {
      for (auto* child : ChildIterator(curr)) {
        if (child->type == curr->type) {
          continue; // already tried
        }
        if (!child->type.isSingle()) {
          continue; // no conversion
        }
        Expression* fixed = nullptr;
        if (!curr->type.isBasic() || !child->type.isBasic()) {
          // TODO: handle compound types
          continue;
        }
        switch (curr->type.getBasic()) {
          case Type::i32: {
            TODO_SINGLE_COMPOUND(child->type);
            switch (child->type.getBasic()) {
              case Type::i32:
                WASM_UNREACHABLE("invalid type");
              case Type::i64:
                fixed = builder->makeUnary(WrapInt64, child);
                break;
              case Type::f32:
                fixed = builder->makeUnary(TruncSFloat32ToInt32, child);
                break;
              case Type::f64:
                fixed = builder->makeUnary(TruncSFloat64ToInt32, child);
                break;
              // not implemented yet
              case Type::v128:
                continue;
              case Type::none:
              case Type::unreachable:
                WASM_UNREACHABLE("unexpected type");
            }
            break;
          }
          case Type::i64: {
            TODO_SINGLE_COMPOUND(child->type);
            switch (child->type.getBasic()) {
              case Type::i32:
                fixed = builder->makeUnary(ExtendSInt32, child);
                break;
              case Type::i64:
                WASM_UNREACHABLE("invalid type");
              case Type::f32:
                fixed = builder->makeUnary(TruncSFloat32ToInt64, child);
                break;
              case Type::f64:
                fixed = builder->makeUnary(TruncSFloat64ToInt64, child);
                break;
              // not implemented yet
              case Type::v128:
                continue;
              case Type::none:
              case Type::unreachable:
                WASM_UNREACHABLE("unexpected type");
            }
            break;
          }
          case Type::f32: {
            TODO_SINGLE_COMPOUND(child->type);
            switch (child->type.getBasic()) {
              case Type::i32:
                fixed = builder->makeUnary(ConvertSInt32ToFloat32, child);
                break;
              case Type::i64:
                fixed = builder->makeUnary(ConvertSInt64ToFloat32, child);
                break;
              case Type::f32:
                WASM_UNREACHABLE("unexpected type");
              case Type::f64:
                fixed = builder->makeUnary(DemoteFloat64, child);
                break;
              // not implemented yet
              case Type::v128:
                continue;
              case Type::none:
              case Type::unreachable:
                WASM_UNREACHABLE("unexpected type");
            }
            break;
          }
          case Type::f64: {
            TODO_SINGLE_COMPOUND(child->type);
            switch (child->type.getBasic()) {
              case Type::i32:
                fixed = builder->makeUnary(ConvertSInt32ToFloat64, child);
                break;
              case Type::i64:
                fixed = builder->makeUnary(ConvertSInt64ToFloat64, child);
                break;
              case Type::f32:
                fixed = builder->makeUnary(PromoteFloat32, child);
                break;
              case Type::f64:
                WASM_UNREACHABLE("unexpected type");
              // not implemented yet
              case Type::v128:
                continue;
              case Type::none:
              case Type::unreachable:
                WASM_UNREACHABLE("unexpected type");
            }
            break;
          }
          // not implemented yet
          case Type::v128:
            continue;
          case Type::none:
          case Type::unreachable:
            WASM_UNREACHABLE("unexpected type");
        }
        assert(fixed->type == curr->type);
        if (tryToReplaceCurrent(fixed)) {
          return;
        }
      }
    }
  }

  void visitFunction(Function* curr) {
    // finish function
    funcsSeen++;
    static int last = 0;
    int percentage = (100 * funcsSeen) / getModule()->functions.size();
    if (std::abs(percentage - last) >= 5) {
      std::cerr << "|    " << percentage << "% of funcs complete\n";
      last = percentage;
    }
  }

  // TODO: bisection on segment shrinking?

  void visitDataSegment(DataSegment* curr) {
    // try to reduce to first function. first, shrink segment elements.
    // while we are shrinking successfully, keep going exponentially.
    bool shrank = false;
    shrank = shrinkByReduction(curr, 2);
    // the "opposite" of shrinking: copy a 'zero' element
    reduceByZeroing(
      curr, 0, [](char item) { return item == 0; }, 2, shrank);
  }

  template<typename T, typename U, typename C>
  void
  reduceByZeroing(T* segment, U zero, C isZero, size_t bonus, bool shrank) {
    for (auto& item : segment->data) {
      if (!shouldTryToReduce(bonus) || isZero(item)) {
        continue;
      }
      auto save = item;
      item = zero;
      if (writeAndTestReduction()) {
        std::cerr << "|      zeroed elem segment\n";
        noteReduction();
      } else {
        item = save;
      }
      if (shrank) {
        // zeroing is fairly inefficient. if we are managing to shrink
        // (which we do exponentially), just zero one per segment at most
        break;
      }
    }
  }

  template<typename T> bool shrinkByReduction(T* segment, size_t bonus) {
    // try to reduce to first function. first, shrink segment elements.
    // while we are shrinking successfully, keep going exponentially.
    bool justShrank = false;

    auto& data = segment->data;
    // when we succeed, try to shrink by more and more, similar to bisection
    size_t skip = 1;
    for (size_t i = 0; i < data.size() && !data.empty(); i++) {
      if (justShrank || shouldTryToReduce(bonus)) {
        auto save = data;
        for (size_t j = 0; j < skip; j++) {
          if (data.empty()) {
            break;
          } else {
            data.pop_back();
          }
        }
        justShrank = writeAndTestReduction();
        if (justShrank) {
          std::cerr << "|      shrank segment from " << save.size() << " => "
                    << data.size() << " (skip: " << skip << ")\n";
          noteReduction();
          skip = std::min(size_t(factor), 2 * skip);
        } else {
          data = std::move(save);
          return false;
        }
      }
    }

    return true;
  }

  void shrinkElementSegments() {
    std::cerr << "|    try to simplify elem segments\n";

    // First, shrink segment elements.
    bool shrank = false;
    for (auto& segment : module->elementSegments) {
      // Try to shrink all the segments (code in shrinkByReduction will decide
      // which to actually try to shrink, based on the current factor), and note
      // if we shrank anything at all (which we'll use later down).
      shrank = shrinkByReduction(segment.get(), 1) || shrank;
    }

    // Second, try to replace elements with a "zero".
    auto it =
      std::find_if_not(module->elementSegments.begin(),
                       module->elementSegments.end(),
                       [&](auto& segment) { return segment->data.empty(); });

    Expression* first = nullptr;
    if (it != module->elementSegments.end()) {
      first = it->get()->data[0];
    }
    if (first == nullptr) {
      // The elements are all empty, nothing left to do.
      return;
    }

    // the "opposite" of shrinking: copy a 'zero' element
    for (auto& segment : module->elementSegments) {
      reduceByZeroing(
        segment.get(),
        first,
        [&](Expression* elem) {
          if (elem->is<RefNull>()) {
            // We don't need to replace a ref.null.
            return true;
          }
          // Is the element equal to our first "zero" element?
          return ExpressionAnalyzer::equal(first, elem);
        },
        1,
        shrank);
    }
  }

  // Reduces entire functions at a time. Returns whether we did a significant
  // amount of reduction that justifies doing even more.
  bool reduceFunctions() {
    // try to remove functions
    std::vector<Name> functionNames;
    for (auto& func : module->functions) {
      functionNames.push_back(func->name);
    }
    auto numFuncs = functionNames.size();
    if (numFuncs == 0) {
      return false;
    }
    size_t skip = 1;
    size_t maxSkip = 1;
    // If we just removed some functions in the previous iteration, keep trying
    // to remove more as this is one of the most efficient ways to reduce.
    bool justReduced = true;
    // Start from a new place each time.
    size_t base = deterministicRandom(numFuncs);
    std::cerr << "|    try to remove functions (base: " << base
              << ", decisionCounter: " << decisionCounter << ", numFuncs "
              << numFuncs << ")\n";
    for (size_t x = 0; x < functionNames.size(); x++) {
      size_t i = (base + x) % numFuncs;
      if (!justReduced &&
          functionsWeTriedToRemove.count(functionNames[i]) == 1 &&
          !shouldTryToReduce(std::max((factor / 5) + 1, 20000))) {
        continue;
      }
      std::vector<Name> names;
      for (size_t j = 0; names.size() < skip && i + j < functionNames.size();
           j++) {
        auto name = functionNames[i + j];
        if (module->getFunctionOrNull(name)) {
          names.push_back(name);
          functionsWeTriedToRemove.insert(name);
        }
      }
      if (names.size() == 0) {
        continue;
      }
      std::cerr << "|     trying at i=" << i << " of size " << names.size()
                << "\n";
      // Try to remove functions and/or empty them. Note that
      // tryToRemoveFunctions() will reload the module if it fails, which means
      // function names may change - for that reason, run it second.
      justReduced = tryToEmptyFunctions(names) || tryToRemoveFunctions(names);
      if (justReduced) {
        noteReduction(names.size());
        // Subtract 1 since the loop increments us anyhow by one: we want to
        // skip over the skipped functions, and not any more.
        x += skip - 1;
        skip = std::min(size_t(factor), 2 * skip);
        maxSkip = std::max(skip, maxSkip);
      } else {
        skip = std::max(skip / 2, size_t(1)); // or 1?
        x += factor / 100;
      }
    }
    // If maxSkip is 1 then we never reduced at all. If it is 2 then we did
    // manage to reduce individual functions, but all our attempts at
    // exponential growth failed. Only suggest doing a new iteration of this
    // function if we did in fact manage to grow, which indicated there are lots
    // of opportunities here, and it is worth focusing on this.
    return maxSkip > 2;
  }

  void visitModule(Module* curr) {
    // The initial module given to us is our global object. As we continue to
    // process things here, we may replace the module, so we should never again
    // refer to curr.
    assert(curr == module.get());
    curr = nullptr;

    // Reduction of entire functions at a time is very effective, and we do it
    // with exponential growth and backoff, so keep doing it while it works.
    while (reduceFunctions()) {
    }

    shrinkElementSegments();

    // try to remove exports
    std::cerr << "|    try to remove exports (with factor " << factor << ")\n";
    std::vector<Export> exports;
    for (auto& exp : module->exports) {
      exports.push_back(*exp);
    }
    size_t skip = 1;
    for (size_t i = 0; i < exports.size(); i++) {
      if (!shouldTryToReduce(std::max((factor / 100) + 1, 1000))) {
        continue;
      }
      std::vector<Export> currExports;
      for (size_t j = 0; currExports.size() < skip && i + j < exports.size();
           j++) {
        auto exp = exports[i + j];
        if (module->getExportOrNull(exp.name)) {
          currExports.push_back(exp);
          module->removeExport(exp.name);
        }
      }
      ProgramResult result;
      if (!writeAndTestReduction(result)) {
        for (auto exp : currExports) {
          module->addExport(new Export(exp));
        }
        skip = std::max(skip / 2, size_t(1)); // or 1?
      } else {
        std::cerr << "|      removed " << currExports.size() << " exports\n";
        noteReduction(currExports.size());
        i += skip;
        skip = std::min(size_t(factor), 2 * skip);
      }
    }
    // If we are left with a single function that is not exported or used in
    // a table, that is useful as then we can change the return type.
    bool allTablesEmpty =
      std::all_of(module->elementSegments.begin(),
                  module->elementSegments.end(),
                  [&](auto& segment) { return segment->data.empty(); });

    if (module->functions.size() == 1 && module->exports.empty() &&
        allTablesEmpty) {
      auto* func = module->functions[0].get();
      // We can't remove something that might have breaks to it.
      if (!func->imported() && !Properties::isNamedControlFlow(func->body)) {
        auto funcType = func->type;
        auto* funcBody = func->body;
        for (auto* child : ChildIterator(func->body)) {
          if (!(child->type.isConcrete() || child->type == Type::none)) {
            continue; // not something a function can return
          }
          // Try to replace the body with the child, fixing up the function
          // to accept it.
          func->type = Signature(funcType.getSignature().params, child->type);
          func->body = child;
          if (writeAndTestReduction()) {
            // great, we succeeded!
            std::cerr << "|    altered function result type\n";
            noteReduction(1);
            break;
          }
          // Undo.
          func->type = funcType;
          func->body = funcBody;
        }
      }
    }
  }

  // Try to empty out the bodies of some functions.
  bool tryToEmptyFunctions(std::vector<Name> names) {
    std::vector<Expression*> oldBodies;
    size_t actuallyEmptied = 0;
    for (auto name : names) {
      auto* func = module->getFunction(name);
      auto* oldBody = func->body;
      oldBodies.push_back(oldBody);
      // Nothing to do for imported functions (body is nullptr) or for bodies
      // that have already been as reduced as we can make them.
      if (func->imported() || oldBody->is<Unreachable>() ||
          oldBody->is<Nop>()) {
        continue;
      }
      actuallyEmptied++;
      bool useUnreachable = func->getResults() != Type::none;
      if (useUnreachable) {
        func->body = builder->makeUnreachable();
      } else {
        func->body = builder->makeNop();
      }
    }
    if (actuallyEmptied > 0 && writeAndTestReduction()) {
      std::cerr << "|        emptied " << actuallyEmptied << " / "
                << names.size() << " functions\n";
      return true;
    } else {
      // Restore the bodies.
      for (size_t i = 0; i < names.size(); i++) {
        module->getFunction(names[i])->body = oldBodies[i];
      }
      return false;
    }
  }

  // Try to actually remove functions. If they are somehow referred to, we will
  // get a validation error and undo it.
  bool tryToRemoveFunctions(std::vector<Name> names) {
    for (auto name : names) {
      module->removeFunction(name);
    }

    // remove all references to them
    struct FunctionReferenceRemover
      : public PostWalker<FunctionReferenceRemover> {
      std::unordered_set<Name> names;
      std::vector<Name> exportsToRemove;

      FunctionReferenceRemover(std::vector<Name>& vec) {
        for (auto name : vec) {
          names.insert(name);
        }
      }
      void visitCall(Call* curr) {
        if (names.count(curr->target)) {
          replaceCurrent(Builder(*getModule()).replaceWithIdenticalType(curr));
        }
      }
      void visitRefFunc(RefFunc* curr) {
        if (names.count(curr->func)) {
          replaceCurrent(Builder(*getModule()).replaceWithIdenticalType(curr));
        }
      }
      void visitExport(Export* curr) {
        if (names.count(curr->value)) {
          exportsToRemove.push_back(curr->name);
        }
      }
      void doWalkModule(Module* module) {
        PostWalker<FunctionReferenceRemover>::doWalkModule(module);
        for (auto name : exportsToRemove) {
          module->removeExport(name);
        }
      }
    };
    FunctionReferenceRemover referenceRemover(names);
    referenceRemover.walkModule(module.get());

    if (WasmValidator().validate(
          *module, WasmValidator::Globally | WasmValidator::Quiet) &&
        writeAndTestReduction()) {
      std::cerr << "|        removed " << names.size() << " functions\n";
      return true;
    } else {
      loadWorking(); // restore it from orbit
      return false;
    }
  }

  // helpers

  // try to replace condition with always true and always false
  void handleCondition(Expression*& condition) {
    if (!condition) {
      return;
    }
    if (condition->is<Const>()) {
      return;
    }
    auto* c = builder->makeConst(int32_t(0));
    if (!tryToReplaceChild(condition, c)) {
      c->value = Literal(int32_t(1));
      tryToReplaceChild(condition, c);
    }
  }

  bool tryToReduceCurrentToNop() {
    auto* curr = getCurrent();
    if (curr->is<Nop>()) {
      return false;
    }
    // try to replace with a trivial value
    Nop nop;
    if (tryToReplaceCurrent(&nop)) {
      replaceCurrent(builder->makeNop());
      return true;
    }
    return false;
  }

  // Try to replace a concrete value with a trivial constant.
  bool tryToReduceCurrentToConst() {
    auto* curr = getCurrent();

    // References.
    if (curr->type.isNullable() && !curr->is<RefNull>()) {
      RefNull* n = builder->makeRefNull(curr->type.getHeapType());
      return tryToReplaceCurrent(n);
    }

    // Tuples.
    if (curr->type.isTuple() && curr->type.isDefaultable()) {
      Expression* n =
        builder->makeConstantExpression(Literal::makeZeros(curr->type));
      if (ExpressionAnalyzer::equal(n, curr)) {
        return false;
      }
      return tryToReplaceCurrent(n);
    }

    // Numbers. We try to replace them with a 0 or a 1.
    if (!curr->type.isNumber()) {
      return false;
    }
    auto* existing = curr->dynCast<Const>();
    if (existing && existing->value.isZero()) {
      // It's already a zero.
      return false;
    }
    auto* c = builder->makeConst(Literal::makeZero(curr->type));
    if (tryToReplaceCurrent(c)) {
      return true;
    }
    // It's not a zero, and can't be replaced with a zero. Try to make it a one,
    // if it isn't already.
    if (existing &&
        existing->value == Literal::makeFromInt32(1, existing->type)) {
      // It's already a one.
      return false;
    }
    c->value = Literal::makeOne(curr->type);
    return tryToReplaceCurrent(c);
  }

  bool tryToReduceCurrentToUnreachable() {
    auto* curr = getCurrent();
    if (curr->is<Unreachable>()) {
      return false;
    }
    // try to replace with a trivial value
    Unreachable un;
    if (tryToReplaceCurrent(&un)) {
      replaceCurrent(builder->makeUnreachable());
      return true;
    }
    // maybe a return? TODO
    return false;
  }
};

//
// main
//

int main(int argc, const char* argv[]) {
  std::string input, test, working, command;
  // By default, look for binaries alongside our own binary.
  std::string binDir = Path::getDirName(argv[0]);
  bool binary = true, deNan = false, verbose = false, debugInfo = false,
       force = false;

  const std::string WasmReduceOption = "wasm-reduce options";

  ToolOptions options("wasm-reduce",
                      "Reduce a wasm file to a smaller one that has the same "
                      "behavior on a given command");
  options
    .add("--command",
         "-cmd",
         "The command to run on the test, that we want to reduce while keeping "
         "the command's output identical. "
         "We look at the command's return code and stdout here (TODO: stderr), "
         "and we reduce while keeping those unchanged.",
         WasmReduceOption,
         Options::Arguments::One,
         [&](Options* o, const std::string& argument) { command = argument; })
    .add("--test",
         "-t",
         "Test file (this will be written to test, the given command should "
         "read it when we call it)",
         WasmReduceOption,
         Options::Arguments::One,
         [&](Options* o, const std::string& argument) { test = argument; })
    .add("--working",
         "-w",
         "Working file (this will contain the current good state while doing "
         "temporary computations, "
         "and will contain the final best result at the end)",
         WasmReduceOption,
         Options::Arguments::One,
         [&](Options* o, const std::string& argument) { working = argument; })
    .add("--binaries",
         "-b",
         "binaryen binaries location (bin/ directory)",
         WasmReduceOption,
         Options::Arguments::One,
         [&](Options* o, const std::string& argument) {
           // Add separator just in case
           binDir = argument + Path::getPathSeparator();
         })
    .add("--text",
         "-S",
         "Emit intermediate files as text, instead of binary (also make sure "
         "the test and working files have a .wat or .wast suffix)",
         WasmReduceOption,
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) { binary = false; })
    .add("--denan",
         "",
         "Avoid nans when reducing",
         WasmReduceOption,
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) { deNan = true; })
    .add("--verbose",
         "-v",
         "Verbose output mode",
         WasmReduceOption,
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) { verbose = true; })
    .add("--debugInfo",
         "-g",
         "Keep debug info in binaries",
         WasmReduceOption,
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) { debugInfo = true; })
    .add("--force",
         "-f",
         "Force the reduction attempt, ignoring problems that imply it is "
         "unlikely to succeed",
         WasmReduceOption,
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) { force = true; })
    .add("--timeout",
         "-to",
         "A timeout to apply to each execution of the command, in seconds "
         "(default: 2)",
         WasmReduceOption,
         Options::Arguments::One,
         [&](Options* o, const std::string& argument) {
           timeout = atoi(argument.c_str());
           std::cout << "|applying timeout: " << timeout << "\n";
         })
    .add("--extra-flags",
         "-ef",
         "Extra commandline flags to pass to wasm-opt while reducing. "
         "(default: --enable-all)",
         WasmReduceOption,
         Options::Arguments::One,
         [&](Options* o, const std::string& argument) {
           extraFlags = argument;
           std::cout << "|applying extraFlags: " << extraFlags << "\n";
         })
    .add("--save-all-working",
         "-saw",
         "Save all intermediate working files, as $WORKING.0, .1, .2 etc",
         WasmReduceOption,
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) {
           saveAllWorkingFiles = true;
           std::cout << "|saving all intermediate working files\n";
         })
    .add_positional(
      "INFILE",
      Options::Arguments::One,
      [&](Options* o, const std::string& argument) { input = argument; });
  options.parse(argc, argv);

  if (debugInfo) {
    extraFlags += " -g ";
  }

  if (test.size() == 0) {
    Fatal() << "test file not provided\n";
  }
  if (working.size() == 0) {
    Fatal() << "working file not provided\n";
  }

  if (!binary) {
    Colors::setEnabled(false);
  }

  Path::setBinaryenBinDir(binDir);

  std::cerr << "|wasm-reduce\n";
  std::cerr << "|input: " << input << '\n';
  std::cerr << "|test: " << test << '\n';
  std::cerr << "|working: " << working << '\n';
  std::cerr << "|bin dir: " << binDir << '\n';
  std::cerr << "|extra flags: " << extraFlags << '\n';

  // get the expected output
  copy_file(input, test);
  expected.getFromExecution(command);

  std::cerr << "|expected result:\n" << expected << '\n';
  std::cerr << "|!! Make sure the above is what you expect! !!\n\n";

  auto stopIfNotForced = [&](std::string message, ProgramResult& result) {
    std::cerr << "|! " << message << '\n' << result << '\n';
    if (!force) {
      Fatal() << "|! stopping, as it is very unlikely reduction can succeed "
                 "(use -f to ignore this check)";
    }
  };

  if (expected.time + 1 >= timeout) {
    stopIfNotForced("execution time is dangerously close to the timeout - you "
                    "should probably increase the timeout",
                    expected);
  }

  if (!force) {
    std::cerr << "|checking that command has different behavior on different "
                 "inputs (this "
                 "verifies that the test file is used by the command)\n";
    // Try it on an invalid input.
    {
      std::ofstream dst(test, std::ios::binary);
      dst << "waka waka\n";
    }
    ProgramResult resultOnInvalid(command);
    if (resultOnInvalid == expected) {
      // Try it on a valid input.
      Module emptyModule;
      ModuleWriter writer(options.passOptions);
      writer.setBinary(true);
      writer.write(emptyModule, test);
      ProgramResult resultOnValid(command);
      if (resultOnValid == expected) {
        Fatal()
          << "running the command on the given input gives the same result as "
             "when running it on either a trivial valid wasm or a file with "
             "nonsense in it. does the script not look at the test file (" +
               test + ")? (use -f to ignore this check)";
      }
    }
  }

  std::cerr << "|checking that command has expected behavior on canonicalized "
               "(read-written) input\n";
  {
    // read and write it
    auto cmd = Path::getBinaryenBinaryTool("wasm-opt") + " " + input + " -o " +
               test + " " + extraFlags;
    if (!binary) {
      cmd += " -S ";
    }
    ProgramResult readWrite(cmd);
    if (readWrite.failed()) {
      stopIfNotForced("failed to read and write the input", readWrite);
    } else {
      ProgramResult result(command);
      if (result != expected) {
        stopIfNotForced("running command on the canonicalized module should "
                        "give the same results",
                        result);
      }
    }
  }

  copy_file(input, working);
  auto workingSize = file_size(working);
  std::cerr << "|input size: " << workingSize << "\n";

  std::cerr << "|starting reduction!\n";

  int factor = binary ? workingSize * 2 : workingSize / 10;

  size_t lastDestructiveReductions = 0;
  size_t lastPostPassesSize = 0;

  bool stopping = false;

  while (1) {
    Reducer reducer(
      command, test, working, binary, deNan, verbose, debugInfo, options);

    // run binaryen optimization passes to reduce. passes are fast to run
    // and can often reduce large amounts of code efficiently, as opposed
    // to detructive reduction (i.e., that doesn't preserve correctness as
    // passes do) since destrucive must operate one change at a time
    std::cerr << "|  reduce using passes...\n";
    auto oldSize = file_size(working);
    reducer.reduceUsingPasses();
    auto newSize = file_size(working);
    auto passProgress = oldSize - newSize;
    std::cerr << "|  after pass reduction: " << newSize << "\n";

    // always stop after a pass reduction attempt, for final cleanup
    if (stopping) {
      break;
    }

    // check if the full cycle (destructive/passes) has helped or not
    if (lastPostPassesSize && newSize >= lastPostPassesSize) {
      std::cerr << "|  progress has stopped, skipping to the end\n";
      if (factor == 1) {
        // this is after doing work with factor 1, so after the remaining work,
        // stop
        stopping = true;
      } else {
        // decrease the factor quickly
        factor = (factor + 1) / 2; // stable on 1
      }
    }
    lastPostPassesSize = newSize;

    // If destructive reductions lead to useful proportionate pass reductions,
    // keep going at the same factor, as pass reductions are far faster.
    std::cerr << "|  pass progress: " << passProgress
              << ", last destructive: " << lastDestructiveReductions << '\n';
    if (passProgress >= 4 * lastDestructiveReductions) {
      std::cerr << "|  progress is good, do not quickly decrease factor\n";
      // While the amount of pass reductions is proportionately high, we do
      // still want to reduce the factor by some amount. If we do not then there
      // is a risk that both pass and destructive reductions are very low, and
      // we get "stuck" cycling through them. In that case we simply need to do
      // more destructive reductions to make real progress. For that reason,
      // decrease the factor by some small percentage.
      factor = std::max(1, (factor * 9) / 10);
    } else {
      if (factor > 10) {
        factor = (factor / 3) + 1;
      } else {
        factor = (factor + 1) / 2; // stable on 1
      }
    }

    // no point in a factor lorger than the size
    assert(newSize > 4); // wasm modules are >4 bytes anyhow
    factor = std::min(factor, int(newSize) / 4);

    // try to reduce destructively. if a high factor fails to find anything,
    // quickly try a lower one (no point in doing passes until we reduce
    // destructively at least a little)
    while (1) {
      std::cerr << "|  reduce destructively... (factor: " << factor << ")\n";
      lastDestructiveReductions = reducer.reduceDestructively(factor);
      if (lastDestructiveReductions > 0) {
        break;
      }
      // we failed to reduce destructively
      if (factor == 1) {
        stopping = true;
        break;
      }
      factor = std::max(
        1, factor / 4); // quickly now, try to find *something* we can reduce
    }

    std::cerr << "|  destructive reduction led to size: " << file_size(working)
              << '\n';
  }
  std::cerr << "|finished, final size: " << file_size(working) << "\n";
  copy_file(working, test); // just to avoid confusion
}
