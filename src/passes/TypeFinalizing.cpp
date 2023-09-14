/*
 * Copyright 2023 WebAssembly Community Group participants
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
// Make all types final or unfinal (open). A typical workflow might be to un-
// finalize all types, optimize a lot, and then finalize at the end.
//

#include "ir/module-utils.h"
#include "ir/type-updating.h"
#include "pass.h"
#include "wasm-type.h"
#include "wasm.h"

namespace wasm {

namespace {

struct TypeFinalizing : public Pass {
  // Whether we should finalize or unfinalize types.
  bool finalize;

  // We will only modify types that we are allowed to (which are private ones).
  std::unordered_set<HeapType> modifiableTypes;

  TypeFinalizing(bool finalize) : finalize(finalize) {}

  void run(Module* module) override {
    if (!module->features.hasGC()) {
      return;
    }

    auto privateTypes = getPublicHeapTypes(*module);
    modifiableTypes.insert(privateTypes.begin(), privateTypes.end());

    class TypeRewriter : public GlobalTypeRewriter {
      TypeFinalizing& parent;

    public:
      TypeRewriter(Module& wasm, TypeFinalizing& parent)
        : GlobalTypeRewriter(wasm), parent(parent) {}

      void modifyTypeBuilderEntry(TypeBuilder& typeBuilder, Index i, HeapType oldType) override {
        if (parent.modifiableTypes.count(oldStructType)) {
          typeBuilder[i].setOpen(!parent.finalize);
        }
      }
    };

    TypeRewriter(wasm, *this).update();
  }
};

} // anonymous namespace

Pass* createTypeFinalizingPass() {
  return new TypeFinalizing(true);
}

Pass* createTypeUnFinalizingPass() {
  return new TypeFinalizing(false);
}

} // namespace wasm
