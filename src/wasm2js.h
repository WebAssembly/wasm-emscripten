/*
 * Copyright 2015 WebAssembly Community Group participants
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
// WebAssembly-to-JS code translator. Converts wasm functions into
// valid JavaScript (with a somewhat asm.js-ish flavor).
//

#ifndef wasm_wasm2js_h
#define wasm_wasm2js_h

#include <cmath>
#include <numeric>

#include "abi/js.h"
#include "asm_v_wasm.h"
#include "asmjs/asmangle.h"
#include "asmjs/shared-constants.h"
#include "emscripten-optimizer/optimizer.h"
#include "ir/branch-utils.h"
#include "ir/effects.h"
#include "ir/element-utils.h"
#include "ir/find_all.h"
#include "ir/import-utils.h"
#include "ir/load-utils.h"
#include "ir/module-utils.h"
#include "ir/names.h"
#include "ir/table-utils.h"
#include "ir/utils.h"
#include "mixed_arena.h"
#include "passes/passes.h"
#include "support/base64.h"
#include "support/file.h"
#include "wasm-builder.h"
#include "wasm-io.h"
#include "wasm-validator.h"
#include "wasm.h"

namespace wasm {

using namespace cashew;

// Appends extra to block, flattening out if extra is a block as well
void flattenAppend(Ref ast, Ref extra) {
  int index;
  if (ast[0] == BLOCK || ast[0] == TOPLEVEL) {
    index = 1;
  } else if (ast[0] == DEFUN) {
    index = 3;
  } else {
    abort();
  }
  if (extra->isArray() && extra[0] == BLOCK) {
    for (size_t i = 0; i < extra[1]->size(); i++) {
      ast[index]->push_back(extra[1][i]);
    }
  } else {
    ast[index]->push_back(extra);
  }
}

// Appends extra to a chain of sequence elements
void sequenceAppend(Ref& ast, Ref extra) {
  if (!ast.get()) {
    ast = extra;
    return;
  }
  ast = ValueBuilder::makeSeq(ast, extra);
}

bool isTableExported(Module& wasm) {
  if (wasm.tables.empty() || wasm.tables[0]->imported()) {
    return false;
  }
  for (auto& ex : wasm.exports) {
    if (ex->kind == ExternalKind::Table && ex->value == wasm.tables[0]->name) {
      return true;
    }
  }
  return false;
}

bool hasActiveSegments(Module& wasm) {
  for (Index i = 0; i < wasm.memory.segments.size(); i++) {
    if (!wasm.memory.segments[i].isPassive) {
      return true;
    }
  }
  return false;
}

bool needsBufferView(Module& wasm) {
  if (!wasm.memory.exists) {
    return false;
  }

  // If there are any active segments, initActiveSegments needs access
  // to bufferView.
  if (hasActiveSegments(wasm)) {
    return true;
  }

  // The special support functions are emitted as part of the JS glue, if we
  // need them.
  bool need = false;
  ModuleUtils::iterImportedFunctions(wasm, [&](Function* import) {
    if (ABI::wasm2js::isHelper(import->base)) {
      need = true;
    }
  });
  return need;
}

IString stringToIString(std::string str) { return IString(str.c_str(), false); }

// Used when taking a wasm name and generating a JS identifier. Each scope here
// is used to ensure that all names have a unique name but the same wasm name
// within a scope always resolves to the same symbol.
//
// Export: Export names
// Top: The main scope which contains functions and globals
// Local: Local variables in a function.
// Label: Label identifiers in a function
enum class NameScope {
  Export,
  Top,
  Local,
  Label,
  Max,
};

//
// Wasm2JSBuilder - converts a WebAssembly module's functions into JS
//
// In general, JS (asm.js) => wasm is very straightforward, as can
// be seen in asm2wasm.h. Just a single pass, plus a little
// state bookkeeping (breakStack, etc.), and a few after-the
// fact corrections for imports, etc. However, wasm => JS
// is tricky because wasm has statements == expressions, or in
// other words, things like `break` and `if` can show up
// in places where JS can't handle them, like inside an
// a loop's condition check. For that reason we use flat IR here.
// We do optimize it later, to allow some nesting, but we avoid
// non-JS-compatible nesting like block return values control
// flow in an if condition, etc.
//

class Wasm2JSBuilder {
public:
  struct Flags {
    // see wasm2js.cpp for details
    bool debug = false;
    bool pedantic = false;
    bool allowAsserts = false;
    bool emscripten = false;
    bool deterministic = false;
    std::string symbolsFile;
  };

  Wasm2JSBuilder(Flags f, PassOptions options_) : flags(f), options(options_) {
    // We don't try to model wasm's trapping precisely - if we did, each load
    // and store would need to do a check. Given that, we can just ignore
    // implicit traps like those when optimizing. (When not optimizing, it's
    // nice to see codegen that matches wasm more precisely.)
    if (options.optimizeLevel > 0) {
      options.ignoreImplicitTraps = true;
    }
  }

  Ref processWasm(Module* wasm, Name funcName = ASM_FUNC);
  Ref processFunction(Module* wasm, Function* func, bool standalone = false);
  Ref processStandaloneFunction(Module* wasm, Function* func) {
    return processFunction(wasm, func, true);
  }

  // The second pass on an expression: process it fully, generating
  // JS
  Ref processFunctionBody(Module* m, Function* func, bool standalone);

  // Get a temp var.
  IString getTemp(Type type, Function* func) {
    IString ret;
    TODO_SINGLE_COMPOUND(type);
    if (frees[type.getBasic()].size() > 0) {
      ret = frees[type.getBasic()].back();
      frees[type.getBasic()].pop_back();
    } else {
      size_t index = temps[type.getBasic()]++;
      ret = IString((std::string("wasm2js_") + type.toString() + "$" +
                     std::to_string(index))
                      .c_str(),
                    false);
    }
    if (func->localIndices.find(ret) == func->localIndices.end()) {
      Builder::addVar(func, ret, type);
    }
    return ret;
  }

  // Free a temp var.
  void freeTemp(Type type, IString temp) {
    TODO_SINGLE_COMPOUND(type);
    frees[type.getBasic()].push_back(temp);
  }

  // Generates a mangled name from `name` within the specified scope.
  //
  // The goal of this function is to ensure that all identifiers in JS ar
  // unique. Otherwise there can be clashes with locals and functions and cause
  // unwanted name shadowing.
  //
  // The returned string from this function is constant for a particular `name`
  // within a `scope`. Or in other words, the same `name` and `scope` pair will
  // always return the same result. If `scope` changes, however, the return
  // value may differ even if the same `name` is passed in.
  IString fromName(Name name, NameScope scope) {
    // TODO: checking names do not collide after mangling

    // First up check our cached of mangled names to avoid doing extra work
    // below
    auto& map = wasmNameToMangledName[(int)scope];
    auto it = map.find(name.c_str());
    if (it != map.end()) {
      return it->second;
    }
    // The mangled names in our scope.
    auto& scopeMangledNames = mangledNames[(int)scope];
    // In some cases (see below) we need to also check the Top scope.
    auto& topMangledNames = mangledNames[int(NameScope::Top)];

    // This is the first time we've seen the `name` and `scope` pair. Generate a
    // globally unique name based on `name` and then register that in our cache
    // and return it.
    //
    // Identifiers here generated are of the form `${name}_${n}` where `_${n}`
    // is omitted if `n==0` and otherwise `n` is just looped over to find the
    // next unused identifier.
    IString ret;
    for (int i = 0;; i++) {
      std::ostringstream out;
      out << name.c_str();
      if (i > 0) {
        out << "_" << i;
      }
      auto mangled = asmangle(out.str());
      ret = stringToIString(mangled);
      if (scopeMangledNames.count(ret)) {
        // When export names collide things may be confusing, as this is
        // observable externally by the person using the JS. Report a warning.
        if (scope == NameScope::Export) {
          std::cerr << "wasm2js: warning: export names colliding: " << mangled
                    << '\n';
        }
        continue;
      }
      // The Local scope is special: a Local name must not collide with a Top
      // name, as they are in a single namespace in JS and can conflict:
      //
      // function foo(bar) {
      //   var bar = 0;
      // }
      // function bar() { ..
      if (scope == NameScope::Local && topMangledNames.count(ret)) {
        continue;
      }
      // We found a good name, use it.
      scopeMangledNames.insert(ret);
      map[name.c_str()] = ret;
      return ret;
    }
  }

private:
  Flags flags;
  PassOptions options;

  // How many temp vars we need
  std::vector<size_t> temps; // type => num temps
  // Which are currently free to use
  std::vector<std::vector<IString>> frees; // type => list of free names

  // Mangled names cache by interned names.
  // Utilizes the usually reused underlying cstring's pointer as the key.
  std::unordered_map<const void*, IString>
    wasmNameToMangledName[(int)NameScope::Max];
  // Set of all mangled names in each scope.
  std::unordered_set<IString> mangledNames[(int)NameScope::Max];

  // If a function is callable from outside, we'll need to cast the inputs
  // and our return value. Otherwise, internally, casts are only needed
  // on operations.
  std::unordered_set<Name> functionsCallableFromOutside;

  void addBasics(Ref ast, Module* wasm);
  void addFunctionImport(Ref ast, Function* import);
  void addGlobalImport(Ref ast, Global* import);
  void addTable(Ref ast, Module* wasm);
  void addStart(Ref ast, Module* wasm);
  void addExports(Ref ast, Module* wasm);
  void addGlobal(Ref ast, Global* global);
  void addMemoryFuncs(Ref ast, Module* wasm);
  void addMemoryGrowFunc(Ref ast, Module* wasm);

  Wasm2JSBuilder() = delete;
  Wasm2JSBuilder(const Wasm2JSBuilder&) = delete;
  Wasm2JSBuilder& operator=(const Wasm2JSBuilder&) = delete;
};

Ref Wasm2JSBuilder::processWasm(Module* wasm, Name funcName) {
  // Scan the wasm for important things.
  for (auto& exp : wasm->exports) {
    if (exp->kind == ExternalKind::Function) {
      functionsCallableFromOutside.insert(exp->value);
    }
  }
  ElementUtils::iterAllElementFunctionNames(
    wasm, [&](Name name) { functionsCallableFromOutside.insert(name); });

  // Ensure the scratch memory helpers.
  // If later on they aren't needed, we'll clean them up.
  ABI::wasm2js::ensureHelpers(wasm);

  // Process the code, and optimize if relevant.
  // First, do the lowering to a JS-friendly subset.
  {
    PassRunner runner(wasm, options);
    runner.add(make_unique<AutoDrop>());
    // TODO: only legalize if necessary - emscripten would already do so, and
    //       likely other toolchains. but spec test suite needs that.
    runner.add("legalize-js-interface");
    // First up remove as many non-JS operations we can, including things like
    // 64-bit integer multiplication/division, `f32.nearest` instructions, etc.
    // This may inject intrinsics which use i64 so it needs to be run before the
    // i64-to-i32 lowering pass.
    runner.add("remove-non-js-ops");
    // Currently the i64-to-32 lowering pass requires that `flatten` be run
    // before it to produce correct code. For some more details about this see
    // #1480
    runner.add("flatten");
    runner.add("i64-to-i32-lowering");
    runner.add("alignment-lowering");
    // Next, optimize that as best we can. This should not generate
    // non-JS-friendly things.
    if (options.optimizeLevel > 0) {
      // It is especially import to propagate constants after the lowering.
      // However, this can be a slow operation, especially after flattening;
      // some local simplification helps.
      if (options.optimizeLevel >= 3 || options.shrinkLevel >= 1) {
        runner.add("simplify-locals-nonesting");
        runner.add("precompute-propagate");
        // Avoiding reinterpretation is helped by propagation. We also run
        // it later down as default optimizations help as well.
        runner.add("avoid-reinterprets");
      }
      runner.addDefaultOptimizationPasses();
      runner.add("avoid-reinterprets");
    }
    // Finally, get the code into the flat form we need for wasm2js itself, and
    // optimize that a little in a way that keeps that property.
    runner.add("flatten");
    // Regardless of optimization level, run some simple optimizations to undo
    // some of the effects of flattening.
    runner.add("simplify-locals-notee-nostructure");
    // Some operations can be very slow if we didn't run full optimizations
    // earlier, so don't run them automatically.
    if (options.optimizeLevel > 0) {
      runner.add("remove-unused-names");
      runner.add("merge-blocks");
      runner.add("reorder-locals");
      runner.add("coalesce-locals");
    }
    runner.add("reorder-locals");
    runner.add("vacuum");
    runner.add("remove-unused-module-elements");
    // DCE at the end to make sure all IR nodes have valid types for conversion
    // to JS, and not unreachable.
    runner.add("dce");
    runner.setDebug(flags.debug);
    runner.run();
  }

  if (flags.symbolsFile.size() > 0) {
    Output out(flags.symbolsFile, wasm::Flags::Text);
    Index i = 0;
    for (auto& func : wasm->functions) {
      out.getStream() << i++ << ':' << func->name.str << '\n';
    }
  }

#ifndef NDEBUG
  if (!WasmValidator().validate(*wasm)) {
    std::cout << *wasm << '\n';
    Fatal() << "error in validating wasm2js output";
  }
#endif

  Ref ret = ValueBuilder::makeToplevel();
  Ref asmFunc = ValueBuilder::makeFunction(funcName);
  ret[1]->push_back(asmFunc);
  ValueBuilder::appendArgumentToFunction(asmFunc, ENV);

  // add memory import
  if (wasm->memory.exists) {
    if (wasm->memory.imported()) {
      // find memory and buffer in imports
      Ref theVar = ValueBuilder::makeVar();
      asmFunc[3]->push_back(theVar);
      ValueBuilder::appendToVar(
        theVar,
        "memory",
        ValueBuilder::makeDot(ValueBuilder::makeName(ENV),
                              ValueBuilder::makeName(wasm->memory.base)));

      // Assign `buffer = memory.buffer`
      Ref buf = ValueBuilder::makeVar();
      asmFunc[3]->push_back(buf);
      ValueBuilder::appendToVar(
        buf,
        BUFFER,
        ValueBuilder::makeDot(ValueBuilder::makeName("memory"),
                              ValueBuilder::makeName("buffer")));

      // If memory is growable, override the imported memory's grow method to
      // ensure so that when grow is called from the output it works as expected
      if (wasm->memory.max > wasm->memory.initial) {
        asmFunc[3]->push_back(
          ValueBuilder::makeStatement(ValueBuilder::makeBinary(
            ValueBuilder::makeDot(ValueBuilder::makeName("memory"),
                                  ValueBuilder::makeName("grow")),
            SET,
            ValueBuilder::makeName(WASM_MEMORY_GROW))));
      }
    } else {
      Ref theVar = ValueBuilder::makeVar();
      asmFunc[3]->push_back(theVar);
      ValueBuilder::appendToVar(
        theVar,
        BUFFER,
        ValueBuilder::makeNew(ValueBuilder::makeCall(
          ValueBuilder::makeName("ArrayBuffer"),
          ValueBuilder::makeInt(Address::address32_t(wasm->memory.initial.addr *
                                                     Memory::kPageSize)))));
    }
  }

  // add imported tables
  ModuleUtils::iterImportedTables(*wasm, [&](Table* table) {
    Ref theVar = ValueBuilder::makeVar();
    asmFunc[3]->push_back(theVar);
    ValueBuilder::appendToVar(
      theVar,
      FUNCTION_TABLE,
      ValueBuilder::makeDot(ValueBuilder::makeName(ENV), table->base));
  });

  // create heaps, etc
  addBasics(asmFunc[3], wasm);
  ModuleUtils::iterImportedFunctions(
    *wasm, [&](Function* import) { addFunctionImport(asmFunc[3], import); });
  ModuleUtils::iterImportedGlobals(
    *wasm, [&](Global* import) { addGlobalImport(asmFunc[3], import); });

  // Note the names of functions. We need to do this here as when generating
  // mangled local names we need them not to conflict with these (see fromName)
  // so we can't wait until we parse each function to note its name.
  for (auto& f : wasm->functions) {
    fromName(f->name, NameScope::Top);
  }

  // globals
  bool generateFetchHighBits = false;
  ModuleUtils::iterDefinedGlobals(*wasm, [&](Global* global) {
    addGlobal(asmFunc[3], global);
    if (flags.allowAsserts && global->name == INT64_TO_32_HIGH_BITS) {
      generateFetchHighBits = true;
    }
  });
  if (flags.emscripten) {
    asmFunc[3]->push_back(
      ValueBuilder::makeName("// EMSCRIPTEN_START_FUNCS\n"));
  }
  // functions
  ModuleUtils::iterDefinedFunctions(*wasm, [&](Function* func) {
    asmFunc[3]->push_back(processFunction(wasm, func));
  });
  if (generateFetchHighBits) {
    Builder builder(*wasm);
    asmFunc[3]->push_back(
      processFunction(wasm,
                      wasm->addFunction(builder.makeFunction(
                        WASM_FETCH_HIGH_BITS,
                        Signature(Type::none, Type::i32),
                        {},
                        builder.makeReturn(builder.makeGlobalGet(
                          INT64_TO_32_HIGH_BITS, Type::i32))))));
    auto e = new Export();
    e->name = WASM_FETCH_HIGH_BITS;
    e->value = WASM_FETCH_HIGH_BITS;
    e->kind = ExternalKind::Function;
    wasm->addExport(e);
  }
  if (flags.emscripten) {
    asmFunc[3]->push_back(ValueBuilder::makeName("// EMSCRIPTEN_END_FUNCS\n"));
  }

  if (needsBufferView(*wasm)) {
    asmFunc[3]->push_back(
      ValueBuilder::makeBinary(ValueBuilder::makeName("bufferView"),
                               SET,
                               ValueBuilder::makeName(HEAPU8)));
  }
  if (hasActiveSegments(*wasm)) {
    asmFunc[3]->push_back(
      ValueBuilder::makeCall(ValueBuilder::makeName("initActiveSegments"),
                             ValueBuilder::makeName(ENV)));
  }

  addTable(asmFunc[3], wasm);
  addStart(asmFunc[3], wasm);
  addExports(asmFunc[3], wasm);
  return ret;
}

void Wasm2JSBuilder::addBasics(Ref ast, Module* wasm) {
  if (wasm->memory.exists) {
    // heaps, var HEAP8 = new global.Int8Array(buffer); etc
    auto addHeap = [&](IString name, IString view) {
      Ref theVar = ValueBuilder::makeVar();
      ast->push_back(theVar);
      ValueBuilder::appendToVar(theVar,
                                name,
                                ValueBuilder::makeNew(ValueBuilder::makeCall(
                                  view, ValueBuilder::makeName(BUFFER))));
    };
    addHeap(HEAP8, INT8ARRAY);
    addHeap(HEAP16, INT16ARRAY);
    addHeap(HEAP32, INT32ARRAY);
    addHeap(HEAPU8, UINT8ARRAY);
    addHeap(HEAPU16, UINT16ARRAY);
    addHeap(HEAPU32, UINT32ARRAY);
    addHeap(HEAPF32, FLOAT32ARRAY);
    addHeap(HEAPF64, FLOAT64ARRAY);
  }
  // core asm.js imports
  auto addMath = [&](IString name, IString base) {
    Ref theVar = ValueBuilder::makeVar();
    ast->push_back(theVar);
    ValueBuilder::appendToVar(
      theVar, name, ValueBuilder::makeDot(ValueBuilder::makeName(MATH), base));
  };
  addMath(MATH_IMUL, IMUL);
  addMath(MATH_FROUND, FROUND);
  addMath(MATH_ABS, ABS);
  addMath(MATH_CLZ32, CLZ32);
  addMath(MATH_MIN, MIN);
  addMath(MATH_MAX, MAX);
  addMath(MATH_FLOOR, FLOOR);
  addMath(MATH_CEIL, CEIL);
  addMath(MATH_TRUNC, TRUNC);
  addMath(MATH_SQRT, SQRT);
  // abort function
  Ref abortVar = ValueBuilder::makeVar();
  ast->push_back(abortVar);
  ValueBuilder::appendToVar(
    abortVar,
    "abort",
    ValueBuilder::makeDot(ValueBuilder::makeName(ENV), ABORT_FUNC));
  // TODO: this shouldn't be needed once we stop generating literal asm.js code
  // NaN and Infinity variables
  Ref nanVar = ValueBuilder::makeVar();
  ast->push_back(nanVar);
  ValueBuilder::appendToVar(nanVar, "nan", ValueBuilder::makeName("NaN"));
  Ref infinityVar = ValueBuilder::makeVar();
  ast->push_back(infinityVar);
  ValueBuilder::appendToVar(
    infinityVar, "infinity", ValueBuilder::makeName("Infinity"));
}

void Wasm2JSBuilder::addFunctionImport(Ref ast, Function* import) {
  // The scratch memory helpers are emitted in the glue, see code and comments
  // below.
  if (ABI::wasm2js::isHelper(import->base)) {
    return;
  }
  Ref theVar = ValueBuilder::makeVar();
  ast->push_back(theVar);
  // TODO: handle nested module imports
  Ref module = ValueBuilder::makeName(ENV);
  ValueBuilder::appendToVar(
    theVar,
    fromName(import->name, NameScope::Top),
    ValueBuilder::makeDot(module, fromName(import->base, NameScope::Top)));
}

void Wasm2JSBuilder::addGlobalImport(Ref ast, Global* import) {
  Ref theVar = ValueBuilder::makeVar();
  ast->push_back(theVar);
  // TODO: handle nested module imports
  Ref module = ValueBuilder::makeName(ENV);
  Ref value =
    ValueBuilder::makeDot(module, fromName(import->base, NameScope::Top));
  if (import->type == Type::i32) {
    value = makeAsmCoercion(value, ASM_INT);
  }
  ValueBuilder::appendToVar(
    theVar, fromName(import->name, NameScope::Top), value);
}

void Wasm2JSBuilder::addTable(Ref ast, Module* wasm) {
  if (wasm->tables.size() == 0) {
    return;
  }

  bool perElementInit = false;

  // Emit a simple flat table as a JS array literal. Otherwise,
  // emit assignments separately for each index.
  Ref theArray = ValueBuilder::makeArray();
  for (auto& table : wasm->tables) {
    if (!table->type.isFunction()) {
      Fatal() << "wasm2js doesn't support non-function tables\n";
    }

    if (!table->imported()) {
      TableUtils::FlatTable flat(*wasm, *table);
      if (flat.valid) {
        Name null("null");
        for (auto& name : flat.names) {
          if (name.is()) {
            name = fromName(name, NameScope::Top);
          } else {
            name = null;
          }
          ValueBuilder::appendToArray(theArray, ValueBuilder::makeName(name));
        }
      } else {
        perElementInit = true;
        Ref initial =
          ValueBuilder::makeInt(Address::address32_t(table->initial.addr));
        theArray = ValueBuilder::makeNew(
          ValueBuilder::makeCall(IString("Array"), initial));
      }
    } else {
      perElementInit = true;
    }

    if (isTableExported(*wasm)) {
      // If the table is exported use a fake WebAssembly.Table object
      // We don't handle the case where a table is both imported and exported.
      if (table->imported()) {
        Fatal() << "wasm2js doesn't support a table that is both imported and "
                   "exported\n";
      }
      Ref theVar = ValueBuilder::makeVar();
      ast->push_back(theVar);

      Ref table = ValueBuilder::makeCall(IString("Table"), theArray);
      ValueBuilder::appendToVar(theVar, FUNCTION_TABLE, table);
    } else if (!table->imported()) {
      // Otherwise if the table is internal (neither imported not exported).
      // Just use a plain array in this case, avoiding the Table.
      Ref theVar = ValueBuilder::makeVar();
      ast->push_back(theVar);
      ValueBuilder::appendToVar(theVar, FUNCTION_TABLE, theArray);
    }

    if (perElementInit) {
      // TODO: optimize for size
      ModuleUtils::iterTableSegments(
        *wasm, table->name, [&](ElementSegment* segment) {
          auto offset = segment->offset;
          ElementUtils::iterElementSegmentFunctionNames(
            segment, [&](Name entry, Index i) {
              Ref index;
              if (auto* c = offset->dynCast<Const>()) {
                index = ValueBuilder::makeInt(c->value.geti32() + i);
              } else if (auto* get = offset->dynCast<GlobalGet>()) {
                index = ValueBuilder::makeBinary(
                  ValueBuilder::makeName(
                    stringToIString(asmangle(get->name.str))),
                  PLUS,
                  ValueBuilder::makeNum(i));
              } else {
                WASM_UNREACHABLE("unexpected expr type");
              }
              ast->push_back(
                ValueBuilder::makeStatement(ValueBuilder::makeBinary(
                  ValueBuilder::makeSub(ValueBuilder::makeName(FUNCTION_TABLE),
                                        index),
                  SET,
                  ValueBuilder::makeName(fromName(entry, NameScope::Top)))));
            });
        });
    }
  }
}

void Wasm2JSBuilder::addStart(Ref ast, Module* wasm) {
  if (wasm->start.is()) {
    ast->push_back(
      ValueBuilder::makeCall(fromName(wasm->start, NameScope::Top)));
  }
}

void Wasm2JSBuilder::addExports(Ref ast, Module* wasm) {
  Ref exports = ValueBuilder::makeObject();
  for (auto& export_ : wasm->exports) {
    switch (export_->kind) {
      case ExternalKind::Function: {
        ValueBuilder::appendToObjectWithQuotes(
          exports,
          fromName(export_->name, NameScope::Export),
          ValueBuilder::makeName(fromName(export_->value, NameScope::Top)));
        break;
      }
      case ExternalKind::Memory: {
        Ref descs = ValueBuilder::makeObject();
        Ref growDesc = ValueBuilder::makeObject();
        ValueBuilder::appendToObjectWithQuotes(
          descs, IString("grow"), growDesc);
        if (wasm->memory.max > wasm->memory.initial) {
          ValueBuilder::appendToObjectWithQuotes(
            growDesc,
            IString("value"),
            ValueBuilder::makeName(WASM_MEMORY_GROW));
        }
        Ref bufferDesc = ValueBuilder::makeObject();
        Ref bufferGetter = ValueBuilder::makeFunction(IString(""));
        bufferGetter[3]->push_back(
          ValueBuilder::makeReturn(ValueBuilder::makeName(BUFFER)));
        ValueBuilder::appendToObjectWithQuotes(
          bufferDesc, IString("get"), bufferGetter);
        ValueBuilder::appendToObjectWithQuotes(
          descs, IString("buffer"), bufferDesc);
        Ref memory = ValueBuilder::makeCall(
          ValueBuilder::makeDot(ValueBuilder::makeName(IString("Object")),
                                IString("create")),
          ValueBuilder::makeDot(ValueBuilder::makeName(IString("Object")),
                                IString("prototype")));
        ValueBuilder::appendToCall(memory, descs);
        ValueBuilder::appendToObjectWithQuotes(
          exports, fromName(export_->name, NameScope::Export), memory);
        break;
      }
      case ExternalKind::Table: {
        ValueBuilder::appendToObjectWithQuotes(
          exports,
          fromName(export_->name, NameScope::Export),
          ValueBuilder::makeName(FUNCTION_TABLE));
        break;
      }
      case ExternalKind::Global: {
        ValueBuilder::appendToObjectWithQuotes(
          exports,
          fromName(export_->name, NameScope::Export),
          ValueBuilder::makeName(fromName(export_->value, NameScope::Top)));
        break;
      }
      case ExternalKind::Event:
      case ExternalKind::Invalid:
        Fatal() << "unsupported export type: " << export_->name << "\n";
    }
  }
  if (wasm->memory.exists) {
    addMemoryFuncs(ast, wasm);
  }
  ast->push_back(
    ValueBuilder::makeStatement(ValueBuilder::makeReturn(exports)));
}

void Wasm2JSBuilder::addGlobal(Ref ast, Global* global) {
  if (auto* const_ = global->init->dynCast<Const>()) {
    Ref theValue;
    TODO_SINGLE_COMPOUND(const_->type);
    switch (const_->type.getBasic()) {
      case Type::i32: {
        theValue = ValueBuilder::makeInt(const_->value.geti32());
        break;
      }
      case Type::f32: {
        theValue = ValueBuilder::makeCall(
          MATH_FROUND,
          makeAsmCoercion(ValueBuilder::makeDouble(const_->value.getf32()),
                          ASM_DOUBLE));
        break;
      }
      case Type::f64: {
        theValue = makeAsmCoercion(
          ValueBuilder::makeDouble(const_->value.getf64()), ASM_DOUBLE);
        break;
      }
      default: {
        assert(false && "Top const type not supported");
      }
    }
    Ref theVar = ValueBuilder::makeVar();
    ast->push_back(theVar);
    ValueBuilder::appendToVar(
      theVar, fromName(global->name, NameScope::Top), theValue);
  } else if (auto* get = global->init->dynCast<GlobalGet>()) {
    Ref theVar = ValueBuilder::makeVar();
    ast->push_back(theVar);
    ValueBuilder::appendToVar(
      theVar,
      fromName(global->name, NameScope::Top),
      ValueBuilder::makeName(fromName(get->name, NameScope::Top)));
  } else {
    assert(false && "Top init type not supported");
  }
}

Ref Wasm2JSBuilder::processFunction(Module* m,
                                    Function* func,
                                    bool standaloneFunction) {
  if (standaloneFunction) {
    // We are only printing a function, not a whole module. Prepare it for
    // translation now (if there were a module, we'd have done this for all
    // functions in parallel, earlier).
    PassRunner runner(m);
    // We only run a subset of all passes here. TODO: create a full valid module
    // for each assertion body.
    runner.add("flatten");
    runner.add("simplify-locals-notee-nostructure");
    runner.add("reorder-locals");
    runner.add("remove-unused-names");
    runner.add("vacuum");
    runner.runOnFunction(func);
  }

  // We will be symbolically referring to all variables in the function, so make
  // sure that everything has a name and it's unique.
  Names::ensureNames(func);
  Ref ret = ValueBuilder::makeFunction(fromName(func->name, NameScope::Top));
  frees.clear();
  frees.resize(std::max(Type::i32, std::max(Type::f32, Type::f64)) + 1);
  temps.clear();
  temps.resize(std::max(Type::i32, std::max(Type::f32, Type::f64)) + 1);
  temps[Type::i32] = temps[Type::f32] = temps[Type::f64] = 0;
  // arguments
  bool needCoercions = options.optimizeLevel == 0 || standaloneFunction ||
                       functionsCallableFromOutside.count(func->name);
  for (Index i = 0; i < func->getNumParams(); i++) {
    IString name = fromName(func->getLocalNameOrGeneric(i), NameScope::Local);
    ValueBuilder::appendArgumentToFunction(ret, name);
    if (needCoercions) {
      ret[3]->push_back(ValueBuilder::makeStatement(ValueBuilder::makeBinary(
        ValueBuilder::makeName(name),
        SET,
        makeAsmCoercion(ValueBuilder::makeName(name),
                        wasmToAsmType(func->getLocalType(i))))));
    }
  }
  Ref theVar = ValueBuilder::makeVar();
  size_t theVarIndex = ret[3]->size();
  ret[3]->push_back(theVar);
  // body
  flattenAppend(ret, processFunctionBody(m, func, standaloneFunction));
  // vars, including new temp vars
  for (Index i = func->getVarIndexBase(); i < func->getNumLocals(); i++) {
    ValueBuilder::appendToVar(
      theVar,
      fromName(func->getLocalNameOrGeneric(i), NameScope::Local),
      makeAsmCoercedZero(wasmToAsmType(func->getLocalType(i))));
  }
  if (theVar[1]->size() == 0) {
    ret[3]->splice(theVarIndex, 1);
  }
  // checks: all temp vars should be free at the end
  assert(frees[Type::i32].size() == temps[Type::i32]);
  assert(frees[Type::f32].size() == temps[Type::f32]);
  assert(frees[Type::f64].size() == temps[Type::f64]);
  return ret;
}

Ref Wasm2JSBuilder::processFunctionBody(Module* m,
                                        Function* func,
                                        bool standaloneFunction) {
  // Switches are tricky to handle - in wasm they often come with
  // massively-nested "towers" of blocks, which if naively translated
  // to JS may exceed parse recursion limits of VMs. Therefore even when
  // not optimizing we work hard to emit minimal and minimally-nested
  // switches.
  // We do so by pre-scanning for br_tables and noting which of their
  // targets can be hoisted up into them, e.g.
  //
  // (block $a
  //  (block $b
  //   (block $c
  //    (block $d
  //     (block $e
  //      (br_table $a $b $c $d $e (..))
  //     )
  //     ;; code X (for block $e)
  //     ;; implicit fallthrough - can be done in the switch too
  //    )
  //    ;; code Y
  //    (br $c) ;; branch which is identical to a fallthrough
  //   )
  //   ;; code Z
  //   (br $a) ;; skip some blocks - can't do this in a switch!
  //  )
  //  ;; code W
  // )
  //
  // Every branch we see is a potential hazard - all targets must not
  // be optimized into the switch, since they must be reached normally,
  // unless they happen to be right after us, in which case it's just
  // a fallthrough anyhow.
  struct SwitchProcessor : public ExpressionStackWalker<SwitchProcessor> {
    // A list of expressions we don't need to emit, as we are handling them
    // in another way.
    std::set<Expression*> unneededExpressions;

    struct SwitchCase {
      Name target;
      std::vector<Expression*> code;
      SwitchCase(Name target) : target(target) {}
    };

    // The switch cases we found that we can hoist up.
    std::map<Switch*, std::vector<SwitchCase>> hoistedSwitchCases;

    void visitSwitch(Switch* brTable) {
      Index i = expressionStack.size() - 1;
      assert(expressionStack[i] == brTable);
      // A set of names we must stop at, since we've seen branches to them.
      std::set<Name> namesBranchedTo;
      while (1) {
        // Stop if we are at the top level.
        if (i == 0) {
          break;
        }
        i--;
        auto* child = expressionStack[i + 1];
        auto* curr = expressionStack[i];
        // Stop if the current node is not a block with the child in the
        // first position, i.e., the classic switch pattern.
        auto* block = curr->dynCast<Block>();
        if (!block || block->list[0] != child) {
          break;
        }
        // Ignore the case of a name-less block for simplicity (merge-blocks
        // would have removed it).
        if (!block->name.is()) {
          break;
        }
        // If we have already seen this block, stop here.
        if (unneededExpressions.count(block)) {
          // XXX FIXME we should probably abort the entire optimization
          break;
        }
        auto& list = block->list;
        if (child == brTable) {
          // Nothing more to do here (we can in fact skip any code til
          // the parent block).
          continue;
        }
        // Ok, we are a block and our child in the first position is a
        // block, and the neither is branched to - unless maybe the child
        // branches to the parent, check that. Note how we treat the
        // final element which may be a break that is a fallthrough.
        Expression* unneededBr = nullptr;
        for (Index j = 1; j < list.size(); j++) {
          auto* item = list[j];
          auto newBranches = BranchUtils::getExitingBranches(item);
          if (auto* br = item->dynCast<Break>()) {
            if (j == list.size() - 1) {
              if (!br->condition && br->name == block->name) {
                // This is a natural, unnecessary-to-emit fallthrough.
                unneededBr = br;
                break;
              }
            }
          }
          namesBranchedTo.insert(newBranches.begin(), newBranches.end());
        }
        if (namesBranchedTo.count(block->name)) {
          break;
        }
        // We can move code after the child (reached by branching on the
        // child) into the switch.
        auto* childBlock = child->cast<Block>();
        hoistedSwitchCases[brTable].emplace_back(childBlock->name);
        SwitchCase& case_ = hoistedSwitchCases[brTable].back();
        for (Index j = 1; j < list.size(); j++) {
          auto* item = list[j];
          if (item != unneededBr) {
            case_.code.push_back(item);
          }
        }
        list.resize(1);
        // Finally, mark the block as unneeded outside the switch.
        unneededExpressions.insert(childBlock);
      }
    }
  };

  struct ExpressionProcessor
    : public OverriddenVisitor<ExpressionProcessor, Ref> {
    Wasm2JSBuilder* parent;
    IString result; // TODO: remove
    Function* func;
    Module* module;
    bool standaloneFunction;

    SwitchProcessor switchProcessor;

    ExpressionProcessor(Wasm2JSBuilder* parent,
                        Module* m,
                        Function* func,
                        bool standaloneFunction)
      : parent(parent), func(func), module(m),
        standaloneFunction(standaloneFunction) {}

    Ref process() {
      switchProcessor.walk(func->body);
      return visit(func->body, NO_RESULT);
    }

    // A scoped temporary variable.
    struct ScopedTemp {
      Wasm2JSBuilder* parent;
      Type type;
      IString temp; // TODO: switch to indexes; avoid names
      bool needFree;
      // @param possible if provided, this is a variable we can use as our temp.
      //                 it has already been allocated in a higher scope, and we
      //                 can just assign to it as our result is going there
      //                 anyhow.
      ScopedTemp(Type type,
                 Wasm2JSBuilder* parent,
                 Function* func,
                 IString possible = NO_RESULT)
        : parent(parent), type(type) {
        assert(possible != EXPRESSION_RESULT);
        if (possible == NO_RESULT) {
          temp = parent->getTemp(type, func);
          needFree = true;
        } else {
          temp = possible;
          needFree = false;
        }
      }
      ~ScopedTemp() {
        if (needFree) {
          parent->freeTemp(type, temp);
        }
      }

      IString getName() { return temp; }
      Ref getAstName() { return ValueBuilder::makeName(temp); }
    };

    Ref visit(Expression* curr, IString nextResult) {
      IString old = result;
      result = nextResult;
      Ref ret = OverriddenVisitor::visit(curr);
      // keep it consistent for the rest of this frame, which may call visit on
      // multiple children
      result = old;
      return ret;
    }

    Ref visit(Expression* curr, ScopedTemp& temp) {
      return visit(curr, temp.temp);
    }

    Ref visitAndAssign(Expression* curr, IString result) {
      assert(result != NO_RESULT);
      Ref ret = visit(curr, result);
      return ValueBuilder::makeStatement(
        ValueBuilder::makeBinary(ValueBuilder::makeName(result), SET, ret));
    }

    Ref visitAndAssign(Expression* curr, ScopedTemp& temp) {
      return visitAndAssign(curr, temp.getName());
    }

    // Expressions with control flow turn into a block, which we must
    // then handle, even if we are an expression.
    bool isBlock(Ref ast) { return !!ast && ast->isArray() && ast[0] == BLOCK; }

    Ref blockify(Ref ast) {
      if (isBlock(ast)) {
        return ast;
      }
      Ref ret = ValueBuilder::makeBlock();
      ret[1]->push_back(ValueBuilder::makeStatement(ast));
      return ret;
    }

    // Breaks to the top of a loop should be emitted as continues, to that
    // loop's main label
    std::unordered_set<Name> continueLabels;

    IString fromName(Name name, NameScope scope) {
      return parent->fromName(name, scope);
    }

    // Visitors

    Ref visitBlock(Block* curr) {
      if (switchProcessor.unneededExpressions.count(curr)) {
        // We have had our tail hoisted into a switch that is nested in our
        // first position, so we don't need to emit that code again, or
        // ourselves in fact.
        return visit(curr->list[0], NO_RESULT);
      }
      Ref ret = ValueBuilder::makeBlock();
      size_t size = curr->list.size();
      for (size_t i = 0; i < size; i++) {
        flattenAppend(
          ret, ValueBuilder::makeStatement(visit(curr->list[i], NO_RESULT)));
      }
      if (curr->name.is()) {
        ret =
          ValueBuilder::makeLabel(fromName(curr->name, NameScope::Label), ret);
      }
      return ret;
    }

    Ref visitIf(If* curr) {
      Ref condition = visit(curr->condition, EXPRESSION_RESULT);
      Ref ifTrue = visit(curr->ifTrue, NO_RESULT);
      Ref ifFalse;
      if (curr->ifFalse) {
        ifFalse = visit(curr->ifFalse, NO_RESULT);
      }
      return ValueBuilder::makeIf(condition, ifTrue, ifFalse); // simple if
    }

    Ref visitLoop(Loop* curr) {
      Name asmLabel = curr->name;
      continueLabels.insert(asmLabel);
      Ref body = visit(curr->body, result);
      // if we can reach the end of the block, we must leave the while (1) loop
      if (curr->body->type != Type::unreachable) {
        assert(curr->body->type == Type::none); // flat IR
        body = blockify(body);
        flattenAppend(
          body, ValueBuilder::makeBreak(fromName(asmLabel, NameScope::Label)));
      }
      Ref ret = ValueBuilder::makeWhile(ValueBuilder::makeInt(1), body);
      return ValueBuilder::makeLabel(fromName(asmLabel, NameScope::Label), ret);
    }

    Ref makeBreakOrContinue(Name name) {
      if (continueLabels.count(name)) {
        return ValueBuilder::makeContinue(fromName(name, NameScope::Label));
      } else {
        return ValueBuilder::makeBreak(fromName(name, NameScope::Label));
      }
    }

    Ref visitBreak(Break* curr) {
      if (curr->condition) {
        // we need an equivalent to an if here, so use that code
        Break fakeBreak = *curr;
        fakeBreak.condition = nullptr;
        If fakeIf;
        fakeIf.condition = curr->condition;
        fakeIf.ifTrue = &fakeBreak;
        return visit(&fakeIf, result);
      }
      return makeBreakOrContinue(curr->name);
    }

    Expression* defaultBody = nullptr; // default must be last in asm.js

    Ref visitSwitch(Switch* curr) {
      // Even without optimizations, we work hard here to emit minimal and
      // especially minimally-nested code, since otherwise we may get block
      // nesting of a size that JS engines can't handle.
      Ref condition = visit(curr->condition, EXPRESSION_RESULT);
      Ref theSwitch =
        ValueBuilder::makeSwitch(makeAsmCoercion(condition, ASM_INT));
      // First, group the switch targets.
      std::map<Name, std::vector<Index>> targetIndexes;
      for (size_t i = 0; i < curr->targets.size(); i++) {
        targetIndexes[curr->targets[i]].push_back(i);
      }
      // Emit first any hoisted groups.
      auto& hoistedCases = switchProcessor.hoistedSwitchCases[curr];
      std::set<Name> emittedTargets;
      bool hoistedEndsWithUnreachable = false;
      for (auto& case_ : hoistedCases) {
        auto target = case_.target;
        auto& code = case_.code;
        emittedTargets.insert(target);
        if (target != curr->default_) {
          auto& indexes = targetIndexes[target];
          for (auto i : indexes) {
            ValueBuilder::appendCaseToSwitch(theSwitch,
                                             ValueBuilder::makeNum(i));
          }
        } else {
          ValueBuilder::appendDefaultToSwitch(theSwitch);
        }
        for (auto* c : code) {
          ValueBuilder::appendCodeToSwitch(
            theSwitch, blockify(visit(c, NO_RESULT)), false);
          hoistedEndsWithUnreachable = c->type == Type::unreachable;
        }
      }
      // After the hoisted cases, if any remain we must make sure not to
      // fall through into them. If no code was hoisted, this is unnecessary,
      // and if the hoisted code ended with an unreachable it also is not
      // necessary.
      bool stoppedFurtherFallthrough = false;
      auto stopFurtherFallthrough = [&]() {
        if (!stoppedFurtherFallthrough && !hoistedCases.empty() &&
            !hoistedEndsWithUnreachable) {
          stoppedFurtherFallthrough = true;
          ValueBuilder::appendCodeToSwitch(
            theSwitch, blockify(ValueBuilder::makeBreak(IString())), false);
        }
      };

      // Emit any remaining groups by just emitting branches to their code,
      // which will appear outside the switch.
      for (auto& pair : targetIndexes) {
        auto target = pair.first;
        auto& indexes = pair.second;
        if (emittedTargets.count(target)) {
          continue;
        }
        stopFurtherFallthrough();
        if (target != curr->default_) {
          for (auto i : indexes) {
            ValueBuilder::appendCaseToSwitch(theSwitch,
                                             ValueBuilder::makeNum(i));
          }
          ValueBuilder::appendCodeToSwitch(
            theSwitch, blockify(makeBreakOrContinue(target)), false);
        } else {
          // For the group going to the same place as the default, we can just
          // emit the default itself, which we do at the end.
        }
      }
      // TODO: if the group the default is in is not the largest, we can turn
      // the largest into
      //       the default by using a local and a check on the range
      if (!emittedTargets.count(curr->default_)) {
        stopFurtherFallthrough();
        ValueBuilder::appendDefaultToSwitch(theSwitch);
        ValueBuilder::appendCodeToSwitch(
          theSwitch, blockify(makeBreakOrContinue(curr->default_)), false);
      }
      return theSwitch;
    }

    Ref visitCall(Call* curr) {
      if (curr->isReturn) {
        Fatal() << "tail calls not yet supported in wasm2js";
      }
      Ref theCall =
        ValueBuilder::makeCall(fromName(curr->target, NameScope::Top));
      // For wasm => wasm calls, we don't need coercions. TODO: even imports
      // might be safe?
      bool needCoercions = parent->options.optimizeLevel == 0 ||
                           standaloneFunction ||
                           module->getFunction(curr->target)->imported();
      for (auto operand : curr->operands) {
        auto value = visit(operand, EXPRESSION_RESULT);
        if (needCoercions) {
          value = makeAsmCoercion(value, wasmToAsmType(operand->type));
        }
        theCall[2]->push_back(value);
      }
      if (needCoercions) {
        theCall = makeAsmCoercion(theCall, wasmToAsmType(curr->type));
      }
      return theCall;
    }

    Ref visitCallIndirect(CallIndirect* curr) {
      if (curr->isReturn) {
        Fatal() << "tail calls not yet supported in wasm2js";
      }
      // If the target has effects that interact with the operands, we must
      // reorder it to the start.
      bool mustReorder = false;
      EffectAnalyzer targetEffects(
        parent->options, module->features, curr->target);
      if (targetEffects.hasAnything()) {
        for (auto* operand : curr->operands) {
          if (targetEffects.invalidates(
                EffectAnalyzer(parent->options, module->features, operand))) {
            mustReorder = true;
            break;
          }
        }
      }
      // Ensure the function pointer is a number. In general in wasm2js we are
      // ok with true/false being present, as they are immediately cast to a
      // number anyhow on their use. However, FUNCTION_TABLE[true] is *not* the
      // same as FUNCTION_TABLE[1], so we must cast. This is a rare exception
      // because FUNCTION_TABLE is just a normal JS object, not a typed array
      // or a mathematical operation (all of which coerce to a number for us).
      auto target = visit(curr->target, EXPRESSION_RESULT);
      target = makeAsmCoercion(target, ASM_INT);
      if (mustReorder) {
        Ref ret;
        ScopedTemp idx(Type::i32, parent, func);
        std::vector<ScopedTemp*> temps; // TODO: utility class, with destructor?
        for (auto* operand : curr->operands) {
          temps.push_back(new ScopedTemp(operand->type, parent, func));
          IString temp = temps.back()->temp;
          sequenceAppend(ret, visitAndAssign(operand, temp));
        }
        sequenceAppend(ret,
                       ValueBuilder::makeBinary(
                         ValueBuilder::makeName(idx.getName()), SET, target));
        Ref theCall = ValueBuilder::makeCall(ValueBuilder::makeSub(
          ValueBuilder::makeName(FUNCTION_TABLE), idx.getAstName()));
        for (size_t i = 0; i < temps.size(); i++) {
          IString temp = temps[i]->temp;
          auto& operand = curr->operands[i];
          theCall[2]->push_back(makeAsmCoercion(ValueBuilder::makeName(temp),
                                                wasmToAsmType(operand->type)));
        }
        theCall = makeAsmCoercion(theCall, wasmToAsmType(curr->type));
        sequenceAppend(ret, theCall);
        for (auto temp : temps) {
          delete temp;
        }
        return ret;
      } else {
        // Target has no side effects, emit simple code
        Ref theCall = ValueBuilder::makeCall(ValueBuilder::makeSub(
          ValueBuilder::makeName(FUNCTION_TABLE), target));
        for (auto* operand : curr->operands) {
          theCall[2]->push_back(visit(operand, EXPRESSION_RESULT));
        }
        theCall = makeAsmCoercion(theCall, wasmToAsmType(curr->type));
        return theCall;
      }
    }

    // TODO: remove
    Ref makeSetVar(Expression* curr,
                   Expression* value,
                   Name name,
                   NameScope scope) {
      return ValueBuilder::makeBinary(
        ValueBuilder::makeName(fromName(name, scope)),
        SET,
        visit(value, EXPRESSION_RESULT));
    }

    Ref visitLocalGet(LocalGet* curr) {
      return ValueBuilder::makeName(
        fromName(func->getLocalNameOrGeneric(curr->index), NameScope::Local));
    }

    Ref visitLocalSet(LocalSet* curr) {
      return makeSetVar(curr,
                        curr->value,
                        func->getLocalNameOrGeneric(curr->index),
                        NameScope::Local);
    }

    Ref visitGlobalGet(GlobalGet* curr) {
      return ValueBuilder::makeName(fromName(curr->name, NameScope::Top));
    }

    Ref visitGlobalSet(GlobalSet* curr) {
      return makeSetVar(curr, curr->value, curr->name, NameScope::Top);
    }

    Ref visitLoad(Load* curr) {
      // Unaligned loads and stores must have been fixed up already.
      assert(curr->align == 0 || curr->align == curr->bytes);
      // normal load
      Ref ptr = makePointer(curr->ptr, curr->offset);
      Ref ret;
      switch (curr->type.getBasic()) {
        case Type::i32: {
          switch (curr->bytes) {
            case 1:
              ret = ValueBuilder::makeSub(
                ValueBuilder::makeName(
                  LoadUtils::isSignRelevant(curr) && curr->signed_ ? HEAP8
                                                                   : HEAPU8),
                ValueBuilder::makePtrShift(ptr, 0));
              break;
            case 2:
              ret = ValueBuilder::makeSub(
                ValueBuilder::makeName(
                  LoadUtils::isSignRelevant(curr) && curr->signed_ ? HEAP16
                                                                   : HEAPU16),
                ValueBuilder::makePtrShift(ptr, 1));
              break;
            case 4:
              ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAP32),
                                          ValueBuilder::makePtrShift(ptr, 2));
              break;
            default: {
              Fatal() << "Unhandled number of bytes in i32 load: "
                      << curr->bytes;
            }
          }
          break;
        }
        case Type::f32:
          ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAPF32),
                                      ValueBuilder::makePtrShift(ptr, 2));
          break;
        case Type::f64:
          ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAPF64),
                                      ValueBuilder::makePtrShift(ptr, 3));
          break;
        default: {
          Fatal() << "Unhandled type in load: " << curr->type;
        }
      }
      if (curr->isAtomic) {
        Ref call = ValueBuilder::makeCall(
          ValueBuilder::makeDot(ValueBuilder::makeName(ATOMICS), LOAD));
        ValueBuilder::appendToCall(call, ret[1]);
        ValueBuilder::appendToCall(call, ret[2]);
        ret = call;
      }
      // Coercions are not actually needed, as if the user reads beyond valid
      // memory, it's undefined behavior anyhow, and so we don't care much about
      // slowness of undefined values etc.
      bool needCoercions =
        parent->options.optimizeLevel == 0 || standaloneFunction;
      if (needCoercions) {
        ret = makeAsmCoercion(ret, wasmToAsmType(curr->type));
      }
      return ret;
    }

    Ref visitStore(Store* curr) {
      if (module->memory.initial < module->memory.max &&
          curr->type != Type::unreachable) {
        // In JS, if memory grows then it is dangerous to write
        //  HEAP[f()] = ..
        // or
        //  HEAP[..] = f()
        // since if the call swaps HEAP (in a growth operation) then
        // we will not actually write to the new version (since the
        // semantics of JS mean we already looked at HEAP and have
        // decided where to assign to).
        if (!FindAll<Call>(curr->ptr).list.empty() ||
            !FindAll<Call>(curr->value).list.empty() ||
            !FindAll<CallIndirect>(curr->ptr).list.empty() ||
            !FindAll<CallIndirect>(curr->value).list.empty() ||
            !FindAll<MemoryGrow>(curr->ptr).list.empty() ||
            !FindAll<MemoryGrow>(curr->value).list.empty()) {
          Ref ret;
          ScopedTemp ptr(Type::i32, parent, func);
          sequenceAppend(ret, visitAndAssign(curr->ptr, ptr));
          ScopedTemp value(curr->value->type, parent, func);
          sequenceAppend(ret, visitAndAssign(curr->value, value));
          LocalGet getPtr;
          getPtr.index = func->getLocalIndex(ptr.getName());
          getPtr.type = Type::i32;
          LocalGet getValue;
          getValue.index = func->getLocalIndex(value.getName());
          getValue.type = curr->value->type;
          Store fakeStore = *curr;
          fakeStore.ptr = &getPtr;
          fakeStore.value = &getValue;
          sequenceAppend(ret, visitStore(&fakeStore));
          return ret;
        }
      }
      // FIXME if memory growth, store ptr cannot contain a function call
      //       also other stores to memory, check them, all makeSub's
      // Unaligned loads and stores must have been fixed up already.
      assert(curr->align == 0 || curr->align == curr->bytes);
      // normal store
      Ref ptr = makePointer(curr->ptr, curr->offset);
      Ref value = visit(curr->value, EXPRESSION_RESULT);
      Ref ret;
      switch (curr->valueType.getBasic()) {
        case Type::i32: {
          switch (curr->bytes) {
            case 1:
              ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAP8),
                                          ValueBuilder::makePtrShift(ptr, 0));
              break;
            case 2:
              ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAP16),
                                          ValueBuilder::makePtrShift(ptr, 1));
              break;
            case 4:
              ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAP32),
                                          ValueBuilder::makePtrShift(ptr, 2));
              break;
            default:
              abort();
          }
          break;
        }
        case Type::f32:
          ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAPF32),
                                      ValueBuilder::makePtrShift(ptr, 2));
          break;
        case Type::f64:
          ret = ValueBuilder::makeSub(ValueBuilder::makeName(HEAPF64),
                                      ValueBuilder::makePtrShift(ptr, 3));
          break;
        default: {
          Fatal() << "Unhandled type in store: " << curr->valueType;
        }
      }
      if (curr->isAtomic) {
        Ref call = ValueBuilder::makeCall(
          ValueBuilder::makeDot(ValueBuilder::makeName(ATOMICS), STORE));
        ValueBuilder::appendToCall(call, ret[1]);
        ValueBuilder::appendToCall(call, ret[2]);
        ValueBuilder::appendToCall(call, value);
        return call;
      }
      return ValueBuilder::makeBinary(ret, SET, value);
    }

    Ref visitDrop(Drop* curr) { return visit(curr->value, NO_RESULT); }

    Ref visitConst(Const* curr) {
      switch (curr->type.getBasic()) {
        case Type::i32:
          return ValueBuilder::makeInt(curr->value.geti32());
        // An i64 argument translates to two actual arguments to asm.js
        // functions, so we do a bit of a hack here to get our one `Ref` to look
        // like two function arguments.
        case Type::i64: {
          auto lo = (unsigned)curr->value.geti64();
          auto hi = (unsigned)(curr->value.geti64() >> 32);
          std::ostringstream out;
          out << lo << "," << hi;
          std::string os = out.str();
          IString name(os.c_str(), false);
          return ValueBuilder::makeName(name);
        }
        case Type::f32: {
          Ref ret = ValueBuilder::makeCall(MATH_FROUND);
          Const fake;
          fake.value = Literal(double(curr->value.getf32()));
          fake.type = Type::f64;
          ret[2]->push_back(visitConst(&fake));
          return ret;
        }
        case Type::f64: {
          double d = curr->value.getf64();
          if (d == 0 && std::signbit(d)) { // negative zero
            return ValueBuilder::makeUnary(
              PLUS,
              ValueBuilder::makeUnary(MINUS, ValueBuilder::makeDouble(0)));
          }
          return ValueBuilder::makeUnary(
            PLUS, ValueBuilder::makeDouble(curr->value.getf64()));
        }
        default:
          Fatal() << "unknown const type";
      }
    }

    Ref visitUnary(Unary* curr) {
      // normal unary
      switch (curr->type.getBasic()) {
        case Type::i32: {
          switch (curr->op) {
            case ClzInt32: {
              return ValueBuilder::makeCall(
                MATH_CLZ32, visit(curr->value, EXPRESSION_RESULT));
            }
            case CtzInt32:
            case PopcntInt32: {
              WASM_UNREACHABLE("i32 unary should have been removed");
            }
            case EqZInt32: {
              // XXX !x does change the type to bool, which is correct, but may
              // be slower?
              return ValueBuilder::makeUnary(
                L_NOT, visit(curr->value, EXPRESSION_RESULT));
            }
            case ReinterpretFloat32: {
              ABI::wasm2js::ensureHelpers(module,
                                          ABI::wasm2js::SCRATCH_STORE_F32);
              ABI::wasm2js::ensureHelpers(module,
                                          ABI::wasm2js::SCRATCH_LOAD_I32);

              Ref store =
                ValueBuilder::makeCall(ABI::wasm2js::SCRATCH_STORE_F32,
                                       visit(curr->value, EXPRESSION_RESULT));
              // 32-bit scratch memory uses index 2, so that it does not
              // conflict with indexes 0, 1 which are used for 64-bit, see
              // comment where |scratchBuffer| is defined.
              Ref load = ValueBuilder::makeCall(ABI::wasm2js::SCRATCH_LOAD_I32,
                                                ValueBuilder::makeInt(2));
              return ValueBuilder::makeSeq(store, load);
            }
            // generate (~~expr), what Emscripten does
            case TruncSFloat32ToInt32:
            case TruncSFloat64ToInt32:
            case TruncSatSFloat32ToInt32:
            case TruncSatSFloat64ToInt32: {
              return ValueBuilder::makeUnary(
                B_NOT,
                ValueBuilder::makeUnary(B_NOT,
                                        visit(curr->value, EXPRESSION_RESULT)));
            }
            // generate (~~expr >>> 0), what Emscripten does
            case TruncUFloat32ToInt32:
            case TruncUFloat64ToInt32:
            case TruncSatUFloat32ToInt32:
            case TruncSatUFloat64ToInt32: {
              return ValueBuilder::makeBinary(
                ValueBuilder::makeUnary(
                  B_NOT,
                  ValueBuilder::makeUnary(
                    B_NOT, visit(curr->value, EXPRESSION_RESULT))),
                TRSHIFT,
                ValueBuilder::makeNum(0));
            }
            case ExtendS8Int32: {
              return ValueBuilder::makeBinary(
                ValueBuilder::makeBinary(visit(curr->value, EXPRESSION_RESULT),
                                         LSHIFT,
                                         ValueBuilder::makeNum(24)),
                RSHIFT,
                ValueBuilder::makeNum(24));
            }
            case ExtendS16Int32: {
              return ValueBuilder::makeBinary(
                ValueBuilder::makeBinary(visit(curr->value, EXPRESSION_RESULT),
                                         LSHIFT,
                                         ValueBuilder::makeNum(16)),
                RSHIFT,
                ValueBuilder::makeNum(16));
            }
            default:
              WASM_UNREACHABLE("unhandled unary");
          }
        }
        case Type::f32:
        case Type::f64: {
          Ref ret;
          switch (curr->op) {
            case NegFloat32:
            case NegFloat64:
              ret = ValueBuilder::makeUnary(
                MINUS, visit(curr->value, EXPRESSION_RESULT));
              break;
            case AbsFloat32:
            case AbsFloat64:
              ret = ValueBuilder::makeCall(
                MATH_ABS, visit(curr->value, EXPRESSION_RESULT));
              break;
            case CeilFloat32:
            case CeilFloat64:
              ret = ValueBuilder::makeCall(
                MATH_CEIL, visit(curr->value, EXPRESSION_RESULT));
              break;
            case FloorFloat32:
            case FloorFloat64:
              ret = ValueBuilder::makeCall(
                MATH_FLOOR, visit(curr->value, EXPRESSION_RESULT));
              break;
            case TruncFloat32:
            case TruncFloat64:
              ret = ValueBuilder::makeCall(
                MATH_TRUNC, visit(curr->value, EXPRESSION_RESULT));
              break;
            case SqrtFloat32:
            case SqrtFloat64:
              ret = ValueBuilder::makeCall(
                MATH_SQRT, visit(curr->value, EXPRESSION_RESULT));
              break;
            case PromoteFloat32:
              return makeAsmCoercion(visit(curr->value, EXPRESSION_RESULT),
                                     ASM_DOUBLE);
            case DemoteFloat64:
              return makeAsmCoercion(visit(curr->value, EXPRESSION_RESULT),
                                     ASM_FLOAT);
            case ReinterpretInt32: {
              ABI::wasm2js::ensureHelpers(module,
                                          ABI::wasm2js::SCRATCH_STORE_I32);
              ABI::wasm2js::ensureHelpers(module,
                                          ABI::wasm2js::SCRATCH_LOAD_F32);

              // 32-bit scratch memory uses index 2, so that it does not
              // conflict with indexes 0, 1 which are used for 64-bit, see
              // comment where |scratchBuffer| is defined.
              Ref store =
                ValueBuilder::makeCall(ABI::wasm2js::SCRATCH_STORE_I32,
                                       ValueBuilder::makeNum(2),
                                       visit(curr->value, EXPRESSION_RESULT));
              Ref load = ValueBuilder::makeCall(ABI::wasm2js::SCRATCH_LOAD_F32);
              return ValueBuilder::makeSeq(store, load);
            }
            // Coerce the integer to a float as emscripten does
            case ConvertSInt32ToFloat32:
              return makeAsmCoercion(
                makeAsmCoercion(visit(curr->value, EXPRESSION_RESULT), ASM_INT),
                ASM_FLOAT);
            case ConvertSInt32ToFloat64:
              return makeAsmCoercion(
                makeAsmCoercion(visit(curr->value, EXPRESSION_RESULT), ASM_INT),
                ASM_DOUBLE);

            // Generate (expr >>> 0), followed by a coercion
            case ConvertUInt32ToFloat32:
              return makeAsmCoercion(
                ValueBuilder::makeBinary(visit(curr->value, EXPRESSION_RESULT),
                                         TRSHIFT,
                                         ValueBuilder::makeInt(0)),
                ASM_FLOAT);
            case ConvertUInt32ToFloat64:
              return makeAsmCoercion(
                ValueBuilder::makeBinary(visit(curr->value, EXPRESSION_RESULT),
                                         TRSHIFT,
                                         ValueBuilder::makeInt(0)),
                ASM_DOUBLE);
            // TODO: more complex unary conversions
            case NearestFloat32:
            case NearestFloat64:
              WASM_UNREACHABLE(
                "operation should have been removed in previous passes");

            default:
              WASM_UNREACHABLE("unhandled unary float operator");
          }
          if (curr->type == Type::f32) { // doubles need much less coercing
            return makeAsmCoercion(ret, ASM_FLOAT);
          }
          return ret;
        }
        default: {
          Fatal() << "Unhandled type in unary: " << curr;
        }
      }
    }

    Ref visitBinary(Binary* curr) {
      // normal binary
      Ref left = visit(curr->left, EXPRESSION_RESULT);
      Ref right = visit(curr->right, EXPRESSION_RESULT);
      Ref ret;
      switch (curr->type.getBasic()) {
        case Type::i32: {
          switch (curr->op) {
            case AddInt32:
              ret = ValueBuilder::makeBinary(left, PLUS, right);
              break;
            case SubInt32:
              ret = ValueBuilder::makeBinary(left, MINUS, right);
              break;
            case MulInt32: {
              if (curr->type == Type::i32) {
                // TODO: when one operand is a small int, emit a multiply
                return ValueBuilder::makeCall(MATH_IMUL, left, right);
              } else {
                return ValueBuilder::makeBinary(left, MUL, right);
              }
            }
            case DivSInt32:
              ret = ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                             DIV,
                                             makeSigning(right, ASM_SIGNED));
              break;
            case DivUInt32:
              ret = ValueBuilder::makeBinary(makeSigning(left, ASM_UNSIGNED),
                                             DIV,
                                             makeSigning(right, ASM_UNSIGNED));
              break;
            case RemSInt32:
              ret = ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                             MOD,
                                             makeSigning(right, ASM_SIGNED));
              break;
            case RemUInt32:
              ret = ValueBuilder::makeBinary(makeSigning(left, ASM_UNSIGNED),
                                             MOD,
                                             makeSigning(right, ASM_UNSIGNED));
              break;
            case AndInt32:
              ret = ValueBuilder::makeBinary(left, AND, right);
              break;
            case OrInt32:
              ret = ValueBuilder::makeBinary(left, OR, right);
              break;
            case XorInt32:
              ret = ValueBuilder::makeBinary(left, XOR, right);
              break;
            case ShlInt32:
              ret = ValueBuilder::makeBinary(left, LSHIFT, right);
              break;
            case ShrUInt32:
              ret = ValueBuilder::makeBinary(left, TRSHIFT, right);
              break;
            case ShrSInt32:
              ret = ValueBuilder::makeBinary(left, RSHIFT, right);
              break;
            case EqInt32: {
              return ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                              EQ,
                                              makeSigning(right, ASM_SIGNED));
            }
            case NeInt32: {
              return ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                              NE,
                                              makeSigning(right, ASM_SIGNED));
            }
            case LtSInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                              LT,
                                              makeSigning(right, ASM_SIGNED));
            case LtUInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_UNSIGNED),
                                              LT,
                                              makeSigning(right, ASM_UNSIGNED));
            case LeSInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                              LE,
                                              makeSigning(right, ASM_SIGNED));
            case LeUInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_UNSIGNED),
                                              LE,
                                              makeSigning(right, ASM_UNSIGNED));
            case GtSInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                              GT,
                                              makeSigning(right, ASM_SIGNED));
            case GtUInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_UNSIGNED),
                                              GT,
                                              makeSigning(right, ASM_UNSIGNED));
            case GeSInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_SIGNED),
                                              GE,
                                              makeSigning(right, ASM_SIGNED));
            case GeUInt32:
              return ValueBuilder::makeBinary(makeSigning(left, ASM_UNSIGNED),
                                              GE,
                                              makeSigning(right, ASM_UNSIGNED));
            case EqFloat32:
            case EqFloat64:
              return ValueBuilder::makeBinary(left, EQ, right);
            case NeFloat32:
            case NeFloat64:
              return ValueBuilder::makeBinary(left, NE, right);
            case GeFloat32:
            case GeFloat64:
              return ValueBuilder::makeBinary(left, GE, right);
            case GtFloat32:
            case GtFloat64:
              return ValueBuilder::makeBinary(left, GT, right);
            case LeFloat32:
            case LeFloat64:
              return ValueBuilder::makeBinary(left, LE, right);
            case LtFloat32:
            case LtFloat64:
              return ValueBuilder::makeBinary(left, LT, right);
            case RotLInt32:
            case RotRInt32:
              WASM_UNREACHABLE("should be removed already");
            default:
              WASM_UNREACHABLE("unhandled i32 binary operator");
          }
          break;
        }
        case Type::f32:
        case Type::f64:
          switch (curr->op) {
            case AddFloat32:
            case AddFloat64:
              ret = ValueBuilder::makeBinary(left, PLUS, right);
              break;
            case SubFloat32:
            case SubFloat64:
              ret = ValueBuilder::makeBinary(left, MINUS, right);
              break;
            case MulFloat32:
            case MulFloat64:
              ret = ValueBuilder::makeBinary(left, MUL, right);
              break;
            case DivFloat32:
            case DivFloat64:
              ret = ValueBuilder::makeBinary(left, DIV, right);
              break;
            case MinFloat32:
            case MinFloat64:
              ret = ValueBuilder::makeCall(MATH_MIN, left, right);
              break;
            case MaxFloat32:
            case MaxFloat64:
              ret = ValueBuilder::makeCall(MATH_MAX, left, right);
              break;
            case CopySignFloat32:
            case CopySignFloat64:
            default:
              Fatal() << "Unhandled binary float operator: ";
          }
          if (curr->type == Type::f32) {
            return makeAsmCoercion(ret, ASM_FLOAT);
          }
          return ret;
        default:
          Fatal() << "Unhandled type in binary: " << curr;
      }
      return makeAsmCoercion(ret, wasmToAsmType(curr->type));
    }

    Ref visitSelect(Select* curr) {
      // If the condition has effects that interact with the operands, we must
      // reorder it to the start. We must also use locals if the values have
      // side effects, as a JS conditional does not visit both sides.
      bool useLocals = false;
      EffectAnalyzer conditionEffects(
        parent->options, module->features, curr->condition);
      EffectAnalyzer ifTrueEffects(
        parent->options, module->features, curr->ifTrue);
      EffectAnalyzer ifFalseEffects(
        parent->options, module->features, curr->ifFalse);
      if (conditionEffects.invalidates(ifTrueEffects) ||
          conditionEffects.invalidates(ifFalseEffects) ||
          ifTrueEffects.hasSideEffects() || ifFalseEffects.hasSideEffects()) {
        useLocals = true;
      }
      if (useLocals) {
        ScopedTemp tempIfTrue(curr->type, parent, func),
          tempIfFalse(curr->type, parent, func),
          tempCondition(Type::i32, parent, func);
        Ref ifTrue = visit(curr->ifTrue, EXPRESSION_RESULT);
        Ref ifFalse = visit(curr->ifFalse, EXPRESSION_RESULT);
        Ref condition = visit(curr->condition, EXPRESSION_RESULT);
        return ValueBuilder::makeSeq(
          ValueBuilder::makeBinary(tempIfTrue.getAstName(), SET, ifTrue),
          ValueBuilder::makeSeq(
            ValueBuilder::makeBinary(tempIfFalse.getAstName(), SET, ifFalse),
            ValueBuilder::makeSeq(
              ValueBuilder::makeBinary(
                tempCondition.getAstName(), SET, condition),
              ValueBuilder::makeConditional(tempCondition.getAstName(),
                                            tempIfTrue.getAstName(),
                                            tempIfFalse.getAstName()))));
      } else {
        // Simple case without reordering.
        return ValueBuilder::makeConditional(
          visit(curr->condition, EXPRESSION_RESULT),
          visit(curr->ifTrue, EXPRESSION_RESULT),
          visit(curr->ifFalse, EXPRESSION_RESULT));
      }
    }

    Ref visitReturn(Return* curr) {
      if (!curr->value) {
        return ValueBuilder::makeReturn(Ref());
      }
      Ref val = visit(curr->value, EXPRESSION_RESULT);
      bool needCoercion =
        parent->options.optimizeLevel == 0 || standaloneFunction ||
        parent->functionsCallableFromOutside.count(func->name);
      if (needCoercion) {
        val = makeAsmCoercion(val, wasmToAsmType(curr->value->type));
      }
      return ValueBuilder::makeReturn(val);
    }

    Ref visitMemorySize(MemorySize* curr) {
      return ValueBuilder::makeCall(WASM_MEMORY_SIZE);
    }

    Ref visitMemoryGrow(MemoryGrow* curr) {
      if (module->memory.exists &&
          module->memory.max > module->memory.initial) {
        return ValueBuilder::makeCall(
          WASM_MEMORY_GROW,
          makeAsmCoercion(visit(curr->delta, EXPRESSION_RESULT),
                          wasmToAsmType(curr->delta->type)));
      } else {
        return ValueBuilder::makeCall(ABORT_FUNC);
      }
    }

    Ref visitNop(Nop* curr) { return ValueBuilder::makeToplevel(); }
    Ref visitUnreachable(Unreachable* curr) {
      return ValueBuilder::makeCall(ABORT_FUNC);
    }

    // Atomics

    struct HeapAndPointer {
      Ref heap;
      Ref ptr;
    };

    HeapAndPointer
    getHeapAndAdjustedPointer(Index bytes, Expression* ptr, Index offset) {
      IString heap;
      Ref adjustedPtr = makePointer(ptr, offset);
      switch (bytes) {
        case 1:
          heap = HEAP8;
          break;
        case 2:
          heap = HEAP16;
          adjustedPtr = ValueBuilder::makePtrShift(adjustedPtr, 1);
          break;
        case 4:
          heap = HEAP32;
          adjustedPtr = ValueBuilder::makePtrShift(adjustedPtr, 2);
          break;
        default: {
          WASM_UNREACHABLE("unimp");
        }
      }
      return {ValueBuilder::makeName(heap), adjustedPtr};
    }

    Ref visitAtomicRMW(AtomicRMW* curr) {
      auto hap =
        getHeapAndAdjustedPointer(curr->bytes, curr->ptr, curr->offset);
      IString target;
      switch (curr->op) {
        case RMWAdd:
          target = IString("add");
          break;
        case RMWSub:
          target = IString("sub");
          break;
        case RMWAnd:
          target = IString("and");
          break;
        case RMWOr:
          target = IString("or");
          break;
        case RMWXor:
          target = IString("xor");
          break;
        case RMWXchg:
          target = IString("exchange");
          break;
        default:
          WASM_UNREACHABLE("unimp");
      }
      Ref call = ValueBuilder::makeCall(
        ValueBuilder::makeDot(ValueBuilder::makeName(ATOMICS), target));
      ValueBuilder::appendToCall(call, hap.heap);
      ValueBuilder::appendToCall(call, hap.ptr);
      ValueBuilder::appendToCall(call, visit(curr->value, EXPRESSION_RESULT));
      return call;
    }

    Ref visitAtomicCmpxchg(AtomicCmpxchg* curr) {
      auto hap =
        getHeapAndAdjustedPointer(curr->bytes, curr->ptr, curr->offset);
      Ref expected = visit(curr->expected, EXPRESSION_RESULT);
      Ref replacement = visit(curr->replacement, EXPRESSION_RESULT);
      Ref call = ValueBuilder::makeCall(ValueBuilder::makeDot(
        ValueBuilder::makeName(ATOMICS), COMPARE_EXCHANGE));
      ValueBuilder::appendToCall(call, hap.heap);
      ValueBuilder::appendToCall(call, hap.ptr);
      ValueBuilder::appendToCall(call, expected);
      ValueBuilder::appendToCall(call, replacement);
      return makeAsmCoercion(call, wasmToAsmType(curr->type));
    }

    Ref visitAtomicWait(AtomicWait* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }

    Ref visitAtomicNotify(AtomicNotify* curr) {
      Ref call = ValueBuilder::makeCall(ValueBuilder::makeDot(
        ValueBuilder::makeName(ATOMICS), IString("notify")));
      ValueBuilder::appendToCall(call, ValueBuilder::makeName(HEAP32));
      ValueBuilder::appendToCall(
        call,
        ValueBuilder::makePtrShift(makePointer(curr->ptr, curr->offset), 2));
      ValueBuilder::appendToCall(call,
                                 visit(curr->notifyCount, EXPRESSION_RESULT));
      return call;
    }

    Ref visitAtomicFence(AtomicFence* curr) {
      // Sequentially consistent fences can be lowered to no operation
      return ValueBuilder::makeToplevel();
    }

    // TODOs

    Ref visitSIMDExtract(SIMDExtract* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitSIMDReplace(SIMDReplace* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitSIMDShuffle(SIMDShuffle* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitSIMDTernary(SIMDTernary* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitSIMDShift(SIMDShift* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitSIMDLoad(SIMDLoad* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitMemoryInit(MemoryInit* curr) {
      ABI::wasm2js::ensureHelpers(module, ABI::wasm2js::MEMORY_INIT);
      return ValueBuilder::makeCall(ABI::wasm2js::MEMORY_INIT,
                                    ValueBuilder::makeNum(curr->segment),
                                    visit(curr->dest, EXPRESSION_RESULT),
                                    visit(curr->offset, EXPRESSION_RESULT),
                                    visit(curr->size, EXPRESSION_RESULT));
    }
    Ref visitDataDrop(DataDrop* curr) {
      ABI::wasm2js::ensureHelpers(module, ABI::wasm2js::DATA_DROP);
      return ValueBuilder::makeCall(ABI::wasm2js::DATA_DROP,
                                    ValueBuilder::makeNum(curr->segment));
    }
    Ref visitMemoryCopy(MemoryCopy* curr) {
      ABI::wasm2js::ensureHelpers(module, ABI::wasm2js::MEMORY_COPY);
      return ValueBuilder::makeCall(ABI::wasm2js::MEMORY_COPY,
                                    visit(curr->dest, EXPRESSION_RESULT),
                                    visit(curr->source, EXPRESSION_RESULT),
                                    visit(curr->size, EXPRESSION_RESULT));
    }
    Ref visitMemoryFill(MemoryFill* curr) {
      ABI::wasm2js::ensureHelpers(module, ABI::wasm2js::MEMORY_FILL);
      return ValueBuilder::makeCall(ABI::wasm2js::MEMORY_FILL,
                                    visit(curr->dest, EXPRESSION_RESULT),
                                    visit(curr->value, EXPRESSION_RESULT),
                                    visit(curr->size, EXPRESSION_RESULT));
    }
    Ref visitTableGet(TableGet* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTableSet(TableSet* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTableSize(TableSize* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTableGrow(TableGrow* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTableFill(TableFill* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTableCopy(TableCopy* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTableInit(TableInit* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitElemDrop(ElemDrop* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefNull(RefNull* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefIs(RefIs* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefFunc(RefFunc* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefEq(RefEq* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTry(Try* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitThrow(Throw* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRethrow(Rethrow* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitPop(Pop* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTupleMake(TupleMake* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitTupleExtract(TupleExtract* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitI31New(I31New* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitI31Get(I31Get* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitCallRef(CallRef* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefTest(RefTest* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefCast(RefCast* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitBrOn(BrOn* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRttCanon(RttCanon* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRttSub(RttSub* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitStructNew(StructNew* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitStructGet(StructGet* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitStructSet(StructSet* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitArrayNew(ArrayNew* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitArrayGet(ArrayGet* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitArraySet(ArraySet* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitArrayLen(ArrayLen* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitArrayCopy(ArrayCopy* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }
    Ref visitRefAs(RefAs* curr) {
      unimplemented(curr);
      WASM_UNREACHABLE("unimp");
    }

  private:
    Ref makePointer(Expression* ptr, Address offset) {
      auto ret = visit(ptr, EXPRESSION_RESULT);
      if (offset) {
        ret = makeAsmCoercion(
          ValueBuilder::makeBinary(ret, PLUS, ValueBuilder::makeNum(offset)),
          ASM_INT);
      }
      return ret;
    }

    void unimplemented(Expression* curr) {
      Fatal() << "wasm2js cannot convert " << *curr;
    }
  };

  return ExpressionProcessor(this, m, func, standaloneFunction).process();
}

void Wasm2JSBuilder::addMemoryFuncs(Ref ast, Module* wasm) {
  Ref memorySizeFunc = ValueBuilder::makeFunction(WASM_MEMORY_SIZE);
  memorySizeFunc[3]->push_back(ValueBuilder::makeReturn(
    makeAsmCoercion(ValueBuilder::makeBinary(
                      ValueBuilder::makeDot(ValueBuilder::makeName(BUFFER),
                                            IString("byteLength")),
                      DIV,
                      ValueBuilder::makeInt(Memory::kPageSize)),
                    AsmType::ASM_INT)));
  ast->push_back(memorySizeFunc);

  if (wasm->memory.max > wasm->memory.initial) {
    addMemoryGrowFunc(ast, wasm);
  }
}

void Wasm2JSBuilder::addMemoryGrowFunc(Ref ast, Module* wasm) {
  Ref memoryGrowFunc = ValueBuilder::makeFunction(WASM_MEMORY_GROW);
  ValueBuilder::appendArgumentToFunction(memoryGrowFunc, IString("pagesToAdd"));

  memoryGrowFunc[3]->push_back(
    ValueBuilder::makeStatement(ValueBuilder::makeBinary(
      ValueBuilder::makeName(IString("pagesToAdd")),
      SET,
      makeAsmCoercion(ValueBuilder::makeName(IString("pagesToAdd")),
                      AsmType::ASM_INT))));

  Ref oldPages = ValueBuilder::makeVar();
  memoryGrowFunc[3]->push_back(oldPages);
  ValueBuilder::appendToVar(
    oldPages,
    IString("oldPages"),
    makeAsmCoercion(ValueBuilder::makeCall(WASM_MEMORY_SIZE),
                    AsmType::ASM_INT));

  Ref newPages = ValueBuilder::makeVar();
  memoryGrowFunc[3]->push_back(newPages);
  ValueBuilder::appendToVar(
    newPages,
    IString("newPages"),
    makeAsmCoercion(
      ValueBuilder::makeBinary(ValueBuilder::makeName(IString("oldPages")),
                               PLUS,
                               ValueBuilder::makeName(IString("pagesToAdd"))),
      AsmType::ASM_INT));

  Ref block = ValueBuilder::makeBlock();
  memoryGrowFunc[3]->push_back(ValueBuilder::makeIf(
    ValueBuilder::makeBinary(
      ValueBuilder::makeBinary(ValueBuilder::makeName(IString("oldPages")),
                               LT,
                               ValueBuilder::makeName(IString("newPages"))),
      IString("&&"),
      ValueBuilder::makeBinary(ValueBuilder::makeName(IString("newPages")),
                               LT,
                               ValueBuilder::makeInt(Memory::kMaxSize32))),
    block,
    NULL));

  Ref newBuffer = ValueBuilder::makeVar();
  ValueBuilder::appendToBlock(block, newBuffer);
  ValueBuilder::appendToVar(
    newBuffer,
    IString("newBuffer"),
    ValueBuilder::makeNew(ValueBuilder::makeCall(
      ARRAY_BUFFER,
      ValueBuilder::makeCall(MATH_IMUL,
                             ValueBuilder::makeName(IString("newPages")),
                             ValueBuilder::makeInt(Memory::kPageSize)))));

  Ref newHEAP8 = ValueBuilder::makeVar();
  ValueBuilder::appendToBlock(block, newHEAP8);
  ValueBuilder::appendToVar(newHEAP8,
                            IString("newHEAP8"),
                            ValueBuilder::makeNew(ValueBuilder::makeCall(
                              ValueBuilder::makeName(INT8ARRAY),
                              ValueBuilder::makeName(IString("newBuffer")))));

  ValueBuilder::appendToBlock(
    block,
    ValueBuilder::makeCall(
      ValueBuilder::makeDot(ValueBuilder::makeName(IString("newHEAP8")),
                            IString("set")),
      ValueBuilder::makeName(HEAP8)));

  auto setHeap = [&](IString name, IString view) {
    ValueBuilder::appendToBlock(
      block,
      ValueBuilder::makeBinary(
        ValueBuilder::makeName(name),
        SET,
        ValueBuilder::makeNew(ValueBuilder::makeCall(
          ValueBuilder::makeName(view),
          ValueBuilder::makeName(IString("newBuffer"))))));
  };

  setHeap(HEAP8, INT8ARRAY);
  setHeap(HEAP16, INT16ARRAY);
  setHeap(HEAP32, INT32ARRAY);
  setHeap(HEAPU8, UINT8ARRAY);
  setHeap(HEAPU16, UINT16ARRAY);
  setHeap(HEAPU32, UINT32ARRAY);
  setHeap(HEAPF32, FLOAT32ARRAY);
  setHeap(HEAPF64, FLOAT64ARRAY);

  ValueBuilder::appendToBlock(
    block,
    ValueBuilder::makeBinary(ValueBuilder::makeName(BUFFER),
                             SET,
                             ValueBuilder::makeName(IString("newBuffer"))));

  // apply the changes to the memory import
  if (wasm->memory.imported()) {
    ValueBuilder::appendToBlock(
      block,
      ValueBuilder::makeBinary(
        ValueBuilder::makeDot(ValueBuilder::makeName("memory"),
                              ValueBuilder::makeName(BUFFER)),
        SET,
        ValueBuilder::makeName(BUFFER)));
  }

  if (needsBufferView(*wasm)) {
    ValueBuilder::appendToBlock(
      block,
      ValueBuilder::makeBinary(ValueBuilder::makeName("bufferView"),
                               SET,
                               ValueBuilder::makeName(HEAPU8)));
  }

  memoryGrowFunc[3]->push_back(
    ValueBuilder::makeReturn(ValueBuilder::makeName(IString("oldPages"))));

  ast->push_back(memoryGrowFunc);
}

// Wasm2JSGlue emits the core of the module - the functions etc. that would
// be the asm.js function in an asm.js world. This class emits the rest of the
// "glue" around that.
class Wasm2JSGlue {
public:
  Wasm2JSGlue(Module& wasm,
              Output& out,
              Wasm2JSBuilder::Flags flags,
              Name moduleName)
    : wasm(wasm), out(out), flags(flags), moduleName(moduleName) {}

  void emitPre();
  void emitPost();

private:
  Module& wasm;
  Output& out;
  Wasm2JSBuilder::Flags flags;
  Name moduleName;

  void emitPreEmscripten();
  void emitPreES6();
  void emitPostEmscripten();
  void emitPostES6();

  void emitMemory();
  void emitSpecialSupport();
};

void Wasm2JSGlue::emitPre() {
  if (flags.emscripten) {
    emitPreEmscripten();
  } else {
    emitPreES6();
  }

  if (isTableExported(wasm)) {
    out << "function Table(ret) {\n";
    if (wasm.tables[0]->initial == wasm.tables[0]->max) {
      out << "  // grow method not included; table is not growable\n";
    } else {
      out << "  ret.grow = function(by) {\n"
          << "    var old = this.length;\n"
          << "    this.length = this.length + by;\n"
          << "    return old;\n"
          << "  };\n";
    }
    out << "  ret.set = function(i, func) {\n"
        << "    this[i] = func;\n"
        << "  };\n"
        << "  ret.get = function(i) {\n"
        << "    return this[i];\n"
        << "  };\n"
        << "  return ret;\n"
        << "}\n\n";
  }

  emitMemory();
  emitSpecialSupport();
}

void Wasm2JSGlue::emitPreEmscripten() {
  out << "function instantiate(asmLibraryArg) {\n";
}

void Wasm2JSGlue::emitPreES6() {
  std::unordered_map<Name, Name> baseModuleMap;

  auto noteImport = [&](Name module, Name base) {
    // Right now codegen requires a flat namespace going into the module,
    // meaning we don't support importing the same name from multiple namespaces
    // yet.
    if (baseModuleMap.count(base) && baseModuleMap[base] != module) {
      Fatal() << "the name " << base << " cannot be imported from "
              << "two different modules yet";
    }
    baseModuleMap[base] = module;

    out << "import { " << asmangle(base.str) << " } from '" << module.str
        << "';\n";
  };

  ImportInfo imports(wasm);

  ModuleUtils::iterImportedGlobals(
    wasm, [&](Global* import) { noteImport(import->module, import->base); });
  ModuleUtils::iterImportedTables(
    wasm, [&](Table* import) { noteImport(import->module, import->base); });
  ModuleUtils::iterImportedFunctions(wasm, [&](Function* import) {
    // The special helpers are emitted in the glue, see code and comments
    // below.
    if (ABI::wasm2js::isHelper(import->base)) {
      return;
    }
    noteImport(import->module, import->base);
  });

  out << '\n';
}

void Wasm2JSGlue::emitPost() {
  if (flags.emscripten) {
    emitPostEmscripten();
  } else {
    emitPostES6();
  }
}

void Wasm2JSGlue::emitPostEmscripten() {
  out << "  return asmFunc(asmLibraryArg);\n}\n";
}

void Wasm2JSGlue::emitPostES6() {
  // Create an initial `ArrayBuffer` and populate it with static data.
  // Currently we use base64 encoding to encode static data and we decode it at
  // instantiation time.
  //
  // Note that the translation here expects that the lower values of this memory
  // can be used for conversions, so make sure there's at least one page.
  if (wasm.memory.exists && wasm.memory.imported()) {
    out << "var mem" << moduleName.str << " = new ArrayBuffer("
        << wasm.memory.initial.addr * Memory::kPageSize << ");\n";
  }

  // Actually invoke the `asmFunc` generated function, passing in all global
  // values followed by all imports
  out << "var ret" << moduleName.str << " = " << moduleName.str << "(";
  out << "  { abort: function() { throw new Error('abort'); }";

  ModuleUtils::iterImportedFunctions(wasm, [&](Function* import) {
    // The special helpers are emitted in the glue, see code and comments
    // below.
    if (ABI::wasm2js::isHelper(import->base)) {
      return;
    }
    out << ",\n    " << asmangle(import->base.str);
  });

  ModuleUtils::iterImportedMemories(wasm, [&](Memory* import) {
    // The special helpers are emitted in the glue, see code and comments
    // below.
    if (ABI::wasm2js::isHelper(import->base)) {
      return;
    }
    out << ",\n    " << asmangle(import->base.str) << ": { buffer : mem"
        << moduleName.str << " }";
  });

  ModuleUtils::iterImportedTables(wasm, [&](Table* import) {
    // The special helpers are emitted in the glue, see code and comments
    // below.
    if (ABI::wasm2js::isHelper(import->base)) {
      return;
    }
    out << ",\n    " << asmangle(import->base.str);
  });

  out << "\n  });\n";

  if (flags.allowAsserts) {
    return;
  }

  // And now that we have our returned instance, export all our functions
  // that are hanging off it.
  for (auto& exp : wasm.exports) {
    switch (exp->kind) {
      case ExternalKind::Function:
      case ExternalKind::Memory:
        break;

      // Exported globals and function tables aren't supported yet
      default:
        continue;
    }
    std::ostringstream export_name;
    for (auto* ptr = exp->name.str; *ptr; ptr++) {
      if (*ptr == '-') {
        export_name << '_';
      } else {
        export_name << *ptr;
      }
    }
    out << "export var " << asmangle(exp->name.str) << " = ret"
        << moduleName.str << "." << asmangle(exp->name.str) << ";\n";
  }
}

void Wasm2JSGlue::emitMemory() {
  if (needsBufferView(wasm)) {
    // Create a helper bufferView to access the buffer if we need one. We use it
    // for creating memory segments if we have any (we may not if the segments
    // are shipped in a side .mem file, for example), and also in bulk memory
    // operations.
    // This will get assigned during `asmFunc` (and potentially re-assigned
    // during __wasm_memory_grow).
    // TODO: We should probably just share a single HEAPU8 var.
    out << "  var bufferView;\n";
  }

  // If there are no memory segments, we don't need to emit any support code for
  // segment creation.
  if ((!wasm.memory.exists) || wasm.memory.segments.empty()) {
    return;
  }

  // If we have passive memory segments, we need to store those.
  for (auto& seg : wasm.memory.segments) {
    if (seg.isPassive) {
      out << "  var memorySegments = {};\n";
      break;
    }
  }

  out <<
    R"(  var base64ReverseLookup = new Uint8Array(123/*'z'+1*/);
  for (var i = 25; i >= 0; --i) {
    base64ReverseLookup[48+i] = 52+i; // '0-9'
    base64ReverseLookup[65+i] = i; // 'A-Z'
    base64ReverseLookup[97+i] = 26+i; // 'a-z'
  }
  base64ReverseLookup[43] = 62; // '+'
  base64ReverseLookup[47] = 63; // '/'
  /** @noinline Inlining this function would mean expanding the base64 string 4x times in the source code, which Closure seems to be happy to do. */
  function base64DecodeToExistingUint8Array(uint8Array, offset, b64) {
    var b1, b2, i = 0, j = offset, bLength = b64.length, end = offset + (bLength*3>>2) - (b64[bLength-2] == '=') - (b64[bLength-1] == '=');
    for (; i < bLength; i += 4) {
      b1 = base64ReverseLookup[b64.charCodeAt(i+1)];
      b2 = base64ReverseLookup[b64.charCodeAt(i+2)];
      uint8Array[j++] = base64ReverseLookup[b64.charCodeAt(i)] << 2 | b1 >> 4;
      if (j < end) uint8Array[j++] = b1 << 4 | b2 >> 2;
      if (j < end) uint8Array[j++] = b2 << 6 | base64ReverseLookup[b64.charCodeAt(i+3)];
    })";
  if (wasm.features.hasBulkMemory()) {
    // Passive segments in bulk memory are initialized into new arrays that are
    // passed into here, and we need to return them.
    out << R"(
    return uint8Array;)";
  }
  out << R"(
  }
)";

  for (Index i = 0; i < wasm.memory.segments.size(); i++) {
    auto& seg = wasm.memory.segments[i];
    if (seg.isPassive) {
      // Fancy passive segments are decoded into typed arrays on the side, for
      // later copying.
      out << "memorySegments[" << i
          << "] = base64DecodeToExistingUint8Array(new Uint8Array("
          << seg.data.size() << ")"
          << ", 0, \"" << base64Encode(seg.data) << "\");\n";
    }
  }

  if (hasActiveSegments(wasm)) {
    auto globalOffset = [&](const Memory::Segment& segment) {
      if (auto* c = segment.offset->dynCast<Const>()) {
        return std::to_string(c->value.getInteger());
      }
      if (auto* get = segment.offset->dynCast<GlobalGet>()) {
        auto internalName = get->name;
        auto importedName = wasm.getGlobal(internalName)->base;
        return std::string("imports[") + asmangle(importedName.str) + "]";
      }
      Fatal() << "non-constant offsets aren't supported yet\n";
    };

    out << "function initActiveSegments(imports) {\n";
    for (Index i = 0; i < wasm.memory.segments.size(); i++) {
      auto& seg = wasm.memory.segments[i];
      if (!seg.isPassive) {
        // Plain active segments are decoded directly into the main memory.
        out << "  base64DecodeToExistingUint8Array(bufferView, "
            << globalOffset(seg) << ", \"" << base64Encode(seg.data)
            << "\");\n";
      }
    }
    out << "}\n";
  }
}

void Wasm2JSGlue::emitSpecialSupport() {
  // The special support functions are emitted as part of the JS glue, if we
  // need them.
  bool need = false;
  ModuleUtils::iterImportedFunctions(wasm, [&](Function* import) {
    if (ABI::wasm2js::isHelper(import->base)) {
      need = true;
    }
  });
  if (!need) {
    return;
  }

  // Scratch memory uses 3 indexes, each referring to 4 bytes. Indexes 0, 1 are
  // used for 64-bit operations, while 2 is for 32-bit. These operations need
  // separate indexes because we need to handle the case where the optimizer
  // reorders a 32-bit reinterpret in between a 64-bit's split-out parts.
  // That situation can occur because the 64-bit reinterpret was split up into
  // pieces early, in the 64-bit lowering pass, while the 32-bit reinterprets
  // are lowered only at the very end, and until then the optimizer sees wasm
  // reinterprets which have no side effects (but they will have the side effect
  // of altering scratch memory). That is, conceptual code like this:
  //
  //   a = reinterpret_64(b)
  //   x = reinterpret_32(y)
  //
  // turns into
  //
  //   scratch_write(b)
  //   a_low = scratch_read(0)
  //   a_high = scratch_read(1)
  //   x = reinterpret_32(y)
  //
  // (Note how the single wasm instruction for a 64-bit reinterpret turns into
  // multiple operations. We have to do such splitting, because in JS we will
  // have to have separate operations to receive each 32-bit chunk anyhow. A
  // *32*-bit reinterpret *could* be a single function, but given we have the
  // separate functions anyhow for the 64-bit case, it's more compact to reuse
  // those.)
  // At this point, the scratch_* functions look like they have side effects to
  // the optimizer (which is true, as they modify scratch memory), but the
  // reinterpret_32 is still a normal wasm instruction without side effects, so
  // the optimizer might do this:
  //
  //   scratch_write(b)
  //   a_low = scratch_read(0)
  //   x = reinterpret_32(y)    ;; this moved one line up
  //   a_high = scratch_read(1)
  //
  // When we do lower the reinterpret_32 into JS, we get:
  //
  //   scratch_write(b)
  //   a_low = scratch_read(0)
  //   scratch_write(y)
  //   x = scratch_read()
  //   a_high = scratch_read(1)
  //
  // The second write occurs before the first's values have been read, so they
  // interfere.
  //
  // There isn't a problem with reordering 32-bit reinterprets with each other
  // as each is lowered into a pair of write+read in JS (after the wasm
  // optimizer runs), so they are guaranteed to be adjacent (and a JS optimizer
  // that runs later will handle that ok since they are calls, which can always
  // have side effects).
  out << R"(
  var scratchBuffer = new ArrayBuffer(16);
  var i32ScratchView = new Int32Array(scratchBuffer);
  var f32ScratchView = new Float32Array(scratchBuffer);
  var f64ScratchView = new Float64Array(scratchBuffer);
  )";

  ModuleUtils::iterImportedFunctions(wasm, [&](Function* import) {
    if (import->base == ABI::wasm2js::SCRATCH_STORE_I32) {
      out << R"(
  function wasm2js_scratch_store_i32(index, value) {
    i32ScratchView[index] = value;
  }
      )";
    } else if (import->base == ABI::wasm2js::SCRATCH_LOAD_I32) {
      out << R"(
  function wasm2js_scratch_load_i32(index) {
    return i32ScratchView[index];
  }
      )";
    } else if (import->base == ABI::wasm2js::SCRATCH_STORE_F32) {
      out << R"(
  function wasm2js_scratch_store_f32(value) {
    f32ScratchView[2] = value;
  }
      )";
    } else if (import->base == ABI::wasm2js::SCRATCH_LOAD_F32) {
      out << R"(
  function wasm2js_scratch_load_f32() {
    return f32ScratchView[2];
  }
      )";
    } else if (import->base == ABI::wasm2js::SCRATCH_STORE_F64) {
      out << R"(
  function wasm2js_scratch_store_f64(value) {
    f64ScratchView[0] = value;
  }
      )";
    } else if (import->base == ABI::wasm2js::SCRATCH_LOAD_F64) {
      out << R"(
  function wasm2js_scratch_load_f64() {
    return f64ScratchView[0];
  }
      )";
    } else if (import->base == ABI::wasm2js::MEMORY_INIT) {
      out << R"(
  function wasm2js_memory_init(segment, dest, offset, size) {
    // TODO: traps on invalid things
    bufferView.set(memorySegments[segment].subarray(offset, offset + size), dest);
  }
      )";
    } else if (import->base == ABI::wasm2js::MEMORY_FILL) {
      out << R"(
  function wasm2js_memory_fill(dest, value, size) {
    dest = dest >>> 0;
    size = size >>> 0;
    if (dest + size > bufferView.length) throw "trap: invalid memory.fill";
    bufferView.fill(value, dest, dest + size);
  }
      )";
    } else if (import->base == ABI::wasm2js::MEMORY_COPY) {
      out << R"(
  function wasm2js_memory_copy(dest, source, size) {
    // TODO: traps on invalid things
    bufferView.copyWithin(dest, source, source + size);
  }
      )";
    } else if (import->base == ABI::wasm2js::DATA_DROP) {
      out << R"(
  function wasm2js_data_drop(segment) {
    // TODO: traps on invalid things
    memorySegments[segment] = new Uint8Array(0);
  }
      )";
    } else if (import->base == ABI::wasm2js::ATOMIC_WAIT_I32) {
      out << R"(
  function wasm2js_atomic_wait_i32(ptr, expected, timeoutLow, timeoutHigh) {
    if (timeoutLow != -1 || timeoutHigh != -1) throw 'unsupported timeout';
    var view = new Int32Array(bufferView.buffer); // TODO cache
    var result = Atomics.wait(view, ptr, expected);
    if (result == 'ok') return 0;
    if (result == 'not-equal') return 1;
    if (result == 'timed-out') return 2;
    throw 'bad result ' + result;
  }
      )";
    } else if (import->base == ABI::wasm2js::ATOMIC_RMW_I64) {
      out << R"(
  function wasm2js_atomic_rmw_i64(op, bytes, offset, ptr, valueLow, valueHigh) {
    assert(bytes == 8); // TODO: support 1, 2, 4 as well
    var view = new BigInt64Array(bufferView.buffer); // TODO cache
    ptr = (ptr + offset) >> 3;
    var value = BigInt(valueLow >>> 0) | (BigInt(valueHigh >>> 0) << BigInt(32));
    var result;
    switch (op) {
      case 0: { // Add
        result = Atomics.add(view, ptr, value);
        break;
      }
      case 1: { // Sub
        result = Atomics.sub(view, ptr, value);
        break;
      }
      case 2: { // And
        result = Atomics.and(view, ptr, value);
        break;
      }
      case 3: { // Or
        result = Atomics.or(view, ptr, value);
        break;
      }
      case 4: { // Xor
        result = Atomics.xor(view, ptr, value);
        break;
      }
      case 5: { // Xchg
        result = Atomics.exchange(view, ptr, value);
        break;
      }
      default: throw 'bad op';
    }
    var low = Number(result & BigInt(0xffffffff)) | 0;
    var high = Number((result >> BigInt(32)) & BigInt(0xffffffff)) | 0;
    stashedBits = high;
    return low;
  }
      )";
    } else if (import->base == ABI::wasm2js::GET_STASHED_BITS) {
      out << R"(
  var stashedBits = 0;

  function wasm2js_get_stashed_bits() {
    return stashedBits;
  }
      )";
    }
  });

  out << '\n';
}

} // namespace wasm

#endif // wasm_wasm2js_h
