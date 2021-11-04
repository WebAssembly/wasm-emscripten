/*
 * Copyright 2021 WebAssembly Community Group participants
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
// Apply more specific subtypes to type fields where possible, where all the
// writes to that field in the entire program allow doing so.
//

#include "ir/lubs.h"
#include "ir/struct-utils.h"
#include "ir/type-updating.h"
#include "ir/utils.h"
#include "pass.h"
#include "support/unique_deferring_queue.h"
#include "wasm-type.h"
#include "wasm.h"

using namespace std;

namespace wasm {

namespace {

// We use a LUBFinder to track field info. A LUBFinder keeps track of the best
// possible LUB so far. The only extra functionality we need here is whether
// there is a default null value (which would force us to keep the
// type nullable).
struct FieldInfo : public LUBFinder {
  bool nullDefault = false;

  void noteNullDefault() { nullDefault = true; }

  bool hasNullDefault() { return nullDefault; }

  bool noted() { return LUBFinder::noted() || nullDefault; }

  Type get() {
    auto ret = LUBFinder::get();
    if (nullDefault && !ret.isNullable()) {
      ret = Type(ret.getHeapType(), Nullable);
    }
    return ret;
  }

  bool combine(const FieldInfo& other) {
    auto old = nullDefault;
    if (other.nullDefault) {
      noteNullDefault();
    }
    return LUBFinder::combine(other) || nullDefault != old;
  }

  // Force the lub to a particular type.
  void set(Type type) { lub = type; }

  void dump(std::ostream& o) {
    std::cout << "FieldInfo(" << lub << ", " << nullDefault << ")";
  }
};

struct FieldInfoScanner : public Scanner<FieldInfo, FieldInfoScanner> {
  Pass* create() override {
    return new FieldInfoScanner(functionNewInfos, functionSetGetInfos);
  }

  FieldInfoScanner(FunctionStructValuesMap<FieldInfo>& functionNewInfos,
                   FunctionStructValuesMap<FieldInfo>& functionSetGetInfos)
    : Scanner<FieldInfo, FieldInfoScanner>(functionNewInfos,
                                           functionSetGetInfos) {}

  void noteExpression(Expression* expr,
                      HeapType type,
                      Index index,
                      FieldInfo& info) {
    info.note(expr);
  }

  void
  noteDefault(Type fieldType, HeapType type, Index index, FieldInfo& info) {
    // Default values do not affect what the heap type of a field can be turned
    // into. Note them, however, as they force us to keep the type nullable.
    if (fieldType.isRef()) {
      info.noteNullDefault();
    }
  }

  void noteCopy(HeapType type, Index index, FieldInfo& info) {
    // Copies do not add any type requirements at all: the type will always be
    // read and written to a place with the same type.
  }

  void noteRead(HeapType type, Index index, FieldInfo& info) {
    // Nothing to do for a read, we just care about written values.
  }
};

struct GlobalSubtyping : public Pass {
  StructValuesMap<FieldInfo> finalInfos;

  void run(PassRunner* runner, Module* module) override {
    if (getTypeSystem() != TypeSystem::Nominal) {
      Fatal() << "GlobalSubtyping requires nominal typing";
    }

    // Find and analyze struct operations inside each function.
    FunctionStructValuesMap<FieldInfo> functionNewInfos(*module),
      functionSetGetInfos(*module);
    FieldInfoScanner scanner(functionNewInfos, functionSetGetInfos);
    scanner.run(runner, module);
    scanner.runOnModuleCode(runner, module);

    // Combine the data from the functions.
    StructValuesMap<FieldInfo> combinedNewInfos;
    StructValuesMap<FieldInfo> combinedSetGetInfos;
    functionNewInfos.combineInto(combinedNewInfos);
    functionSetGetInfos.combineInto(combinedSetGetInfos);

    // Propagate things written during new to supertypes, as they must also be
    // able to contain that type. Propagate things written using set to subtypes
    // as well, as the reference might be to a supertype if the field is present
    // there.
    TypeHierarchyPropagator<FieldInfo> propagator(*module);
    propagator.propagateToSuperTypes(combinedNewInfos);
    propagator.propagateToSuperAndSubTypes(combinedSetGetInfos);

    // Combine everything together.
    combinedNewInfos.combineInto(finalInfos);
    combinedSetGetInfos.combineInto(finalInfos);

    // While we do the following work, see if we have anything to optimize, so
    // that we can avoid wasteful work later if not.
    bool canOptimize = false;

    // We have combined all the information we have about writes to the fields,
    // but we still need to make sure that the new types makes sense. In
    // particular, subtyping cares about things like mutability, and we also
    // need to handle the case where we have no writes to a type but do have
    // them to subtypes or supertypes; in all these cases, we must preserve
    // that a field is always a subtype of the parent field. To do so, we go
    // through all the types downward from supertypes to subtypes, ensuring the
    // subtypes are suitable.
    auto& subTypes = propagator.subTypes;
    UniqueDeferredQueue<HeapType> work;
    for (auto type : subTypes.types) {
      if (type.isStruct() && !type.getSuperType()) {
        work.push(type);
      }
    }
    while (!work.empty()) {
      auto type = work.pop();

      auto& fields = type.getStruct().fields;
      for (Index i = 0; i < fields.size(); i++) {
        auto oldType = fields[i].type;
        auto& info = finalInfos[type][i];
        auto newType = info.get();
        if (newType == Type::unreachable) {
          // We have no info on this field, so use the old type.
          info.set(oldType);
        } else {
          // We saw writes to this field, which must have been of subtypes of
          // the old type.
          assert(Type::isSubType(newType, oldType));
        }
      }

      if (auto super = type.getSuperType()) {
        auto& superFields = super->getStruct().fields;
        for (Index i = 0; i < superFields.size(); i++) {
          auto newSuperType = finalInfos[*super][i].get();
          auto& info = finalInfos[type][i];
          auto newType = info.get();
          if (!Type::isSubType(newType, newSuperType)) {
            // To ensure we are a subtype of the super's field, simply copy that
            // value, which is more specific than us.
            info.set(newSuperType);
          } else if (fields[i].mutable_ == Mutable) {
            // Mutable fields must have identical types, so we cannot
            // specialize.
            info.set(newSuperType);
          }
        }
      }

      // After all those decisions, see if we found anything to optimize.
      if (!canOptimize) {
        for (Index i = 0; i < fields.size(); i++) {
          auto oldType = fields[i].type;
          auto newType = finalInfos[type][i].get();
          if (newType != oldType) {
            canOptimize = true;
            break;
          }
        }
      }

      for (auto subType : subTypes.getSubTypes(type)) {
        work.push(subType);
      }
    }

    if (canOptimize) {
      updateTypes(*module, runner);
    }
  }

  void updateTypes(Module& wasm, PassRunner* runner) {
    class TypeRewriter : public GlobalTypeRewriter {
      GlobalSubtyping& parent;

    public:
      TypeRewriter(Module& wasm, GlobalSubtyping& parent)
        : GlobalTypeRewriter(wasm), parent(parent) {}

      virtual void modifyStruct(HeapType oldStructType, Struct& struct_) {
        auto& oldFields = oldStructType.getStruct().fields;
        auto& newFields = struct_.fields;

        for (Index i = 0; i < newFields.size(); i++) {
          auto oldType = oldFields[i].type;
          if (!oldType.isRef()) {
            continue;
          }
          auto newType = parent.finalInfos[oldStructType][i].get();
          newFields[i].type = getTempType(newType);
        }
      }
    };

    TypeRewriter(wasm, *this).update();

    ReFinalize().run(runner, &wasm);
  }
};

} // anonymous namespace

Pass* createGlobalSubtypingPass() { return new GlobalSubtyping(); }

} // namespace wasm
