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

#ifndef wasm_ast_trapping_h
#define wasm_ast_trapping_h

#include "pass.h"

namespace wasm {

enum class TrapMode {
  Allow,
  Clamp,
  JS
};

inline void addTrapModePass(PassRunner& runner, TrapMode trapMode) {
  if (trapMode == TrapMode::Clamp) {
    runner.add("trap-mode-clamp");
  } else if (trapMode == TrapMode::JS) {
    runner.add("trap-mode-js");
  }
}

class GeneratedTrappingFunctions {
public:
  GeneratedTrappingFunctions(TrapMode mode, Module &wasm, bool immediate = false)
    : mode(mode),
      wasm(wasm),
      immediate(immediate) { }

  bool hasFunction(Name name) {
    return functions.find(name) != functions.end();
  }
  bool hasImport(Name name) {
    return imports.find(name) != imports.end();
  }

  void addFunction(Function* function) {
    functions[function->name] = function;
    if (immediate) {
      wasm.addFunction(function);
    }
  }
  void addImport(Import* import) {
    imports[import->name] = import;
    if (immediate) {
      wasm.addImport(import);
    }
  }

  void addToModule() {
    if (!immediate) {
      for (auto &pair : functions) {
        wasm.addFunction(pair.second);
      }
      for (auto &pair : imports) {
        wasm.addImport(pair.second);
      }
    }
    functions.clear();
    imports.clear();
  }

  TrapMode getMode() {
    return mode;
  }

  Module& getModule() {
    return wasm;
  }

private:
  std::map<Name, Function*> functions;
  std::map<Name, Import*> imports;

  TrapMode mode;
  Module& wasm;
  bool immediate;
};

Expression* makeTrappingBinary(Binary* curr, GeneratedTrappingFunctions &generated);
Expression* makeTrappingUnary(Unary* curr, GeneratedTrappingFunctions &generated);

} // wasm

#endif // wasm_ast_trapping_h
