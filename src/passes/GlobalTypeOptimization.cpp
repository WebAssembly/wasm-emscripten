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
// Optimize types at the global level, altering fields etc. on the set of heap
// types defined in the module.
//
//  * Immutability: If a field has no struct.set, it can become immutable.
//  * Fields that are never read from can be removed entirely.
//

#include "ir/effects.h"
#include "ir/localize.h"
#include "ir/ordering.h"
#include "ir/struct-utils.h"
#include "ir/subtypes.h"
#include "ir/type-updating.h"
#include "ir/utils.h"
#include "pass.h"
#include "wasm-builder.h"
#include "wasm-type.h"
#include "wasm-type-ordering.h"
#include "wasm.h"

namespace wasm {

namespace {

// Information about usage of a field.
struct FieldInfo {
  bool hasWrite = false;
  bool hasRead = false;

  void noteWrite() { hasWrite = true; }
  void noteRead() { hasRead = true; }

  bool combine(const FieldInfo& other) {
    bool changed = false;
    if (!hasWrite && other.hasWrite) {
      hasWrite = true;
      changed = true;
    }
    if (!hasRead && other.hasRead) {
      hasRead = true;
      changed = true;
    }
    return changed;
  }
};

struct FieldInfoScanner
  : public StructUtils::StructScanner<FieldInfo, FieldInfoScanner> {
  std::unique_ptr<Pass> create() override {
    return std::make_unique<FieldInfoScanner>(functionNewInfos,
                                              functionSetGetInfos);
  }

  FieldInfoScanner(
    StructUtils::FunctionStructValuesMap<FieldInfo>& functionNewInfos,
    StructUtils::FunctionStructValuesMap<FieldInfo>& functionSetGetInfos)
    : StructUtils::StructScanner<FieldInfo, FieldInfoScanner>(
        functionNewInfos, functionSetGetInfos) {}

  void noteExpression(Expression* expr,
                      HeapType type,
                      Index index,
                      FieldInfo& info) {
    info.noteWrite();
  }

  void
  noteDefault(Type fieldType, HeapType type, Index index, FieldInfo& info) {
    info.noteWrite();
  }

  void noteCopy(HeapType type, Index index, FieldInfo& info) {
    info.noteWrite();
  }

  void noteRead(HeapType type, Index index, FieldInfo& info) {
    info.noteRead();
  }
};

struct GlobalTypeOptimization : public Pass {
  StructUtils::StructValuesMap<FieldInfo> combinedSetGetInfos;

  // Maps types to a vector of booleans that indicate whether a field can
  // become immutable. To avoid eager allocation of memory, the vectors are
  // only resized when we actually have a true to place in them (which is
  // rare).
  std::unordered_map<HeapType, std::vector<bool>> canBecomeImmutable;

  // Maps each field to its new index after field removals. That is, this
  // takes into account that fields before this one may have been removed,
  // which would then reduce this field's index. If a field itself is removed,
  // it has the special value |RemovedField|. This is always of the full size
  // of the number of fields, unlike canBecomeImmutable which is lazily
  // allocated, as if we remove one field that affects the indexes of all the
  // others anyhow.
  static const Index RemovedField = Index(-1);
  std::unordered_map<HeapType, std::vector<Index>> indexesAfterRemovals;

  void run(Module* module) override {
    if (!module->features.hasGC()) {
      return;
    }

    if (!getPassOptions().closedWorld) {
      Fatal() << "GTO requires --closed-world";
    }

    // Find and analyze struct operations inside each function.
    StructUtils::FunctionStructValuesMap<FieldInfo> functionNewInfos(*module),
      functionSetGetInfos(*module);
    FieldInfoScanner scanner(functionNewInfos, functionSetGetInfos);
    scanner.run(getPassRunner(), module);
    scanner.runOnModuleCode(getPassRunner(), module);

    // Combine the data from the functions.
    functionSetGetInfos.combineInto(combinedSetGetInfos);
    // TODO: combine newInfos as well, once we have a need for that (we will
    //       when we do things like subtyping).

    // Propagate information to super and subtypes on set/get infos:
    //
    //  * For removing unread fields, we can only remove a field if it is never
    //    read in any sub or supertype, as such a read may alias any of those
    //    types (where the field is present).
    //
    //    Note that we *can* propagate reads only to supertypes, but we are
    //    limited in what we optimize. If type A has fields {a, b}, and its
    //    subtype B has the same fields, and if field a is only used in reads of
    //    type B, then we still cannot remove it. If we removed it then A would
    //    have fields {b}, that is, field b would be at index 0, while type B
    //    would still be {a, b} which has field b at index 1, which is not
    //    compatible. The only case in which we can optimize is to remove a
    //    field from the end, that is, we could remove field b from A.
    //    Otherwise, as mentioned before we can only remove a field if we also
    //    remove it from all sub- and super-types.
    //
    //  * For immutability, this is necessary because we cannot have a
    //    supertype's field be immutable while a subtype's is not - they must
    //    match for us to preserve subtyping.
    //
    //    Note that we do not need to care about types here: If the fields were
    //    mutable before, then they must have had identical types for them to be
    //    subtypes (as wasm only allows the type to differ if the fields are
    //    immutable). Note that by making more things immutable we therefore
    //    make it possible to apply more specific subtypes in subtype fields.
    StructUtils::TypeHierarchyPropagator<FieldInfo> propagator(*module);
    auto subSupers = combinedSetGetInfos;
    propagator.propagateToSuperAndSubTypes(subSupers);
    auto subs = std::move(combinedSetGetInfos);
    propagator.propagateToSubTypes(subs);

    // Process the propagated info. We look at supertypes first, as the order of
    // fields in a supertype is a constraint on what subtypes can do. That is,
    // we decide for each supertype what the optimal order is, and consider that
    // fixed, and then subtypes can decide how to sort fields that they append.
    for (auto type : SupertypesFirst().sort(propagator.subTypes)) {
      if (!type.isStruct()) {
        continue;
      }
      auto& fields = type.getStruct().fields;
      auto& subSuper = subSupers[type];
      auto& sub = subs[type];

      // Process immutability.
      for (Index i = 0; i < fields.size(); i++) {
        if (fields[i].mutable_ == Immutable) {
          // Already immutable; nothing to do.
          continue;
        }

        if (subSuper[i].hasWrite) {
          // A set exists.
          continue;
        }

        // No set exists. Mark it as something we can make immutable.
        auto& vec = canBecomeImmutable[type];
        vec.resize(i + 1);
        vec[i] = true;
      }

      // Process removability.
      std::set<Index> removableIndexes;
      for (Index i = 0; i < fields.size(); i++) {
        // If there is no read whatsoever, in either subs or supers, then we can
        // remove the field. That is so even if there are writes (it would be a
        // pointless"write-only-field").
        auto hasNoReadsAnywhere = !subSuper[i].hasRead;

        // Check for reads or writes in ourselves and our supers. If there are
        // none, then operations only happen in our strict subtypes, and those
        // subtypes can define the field there, and we don't need it here.
        auto hasNoReadsOrWritesInSupers = !sub[i].hasRead && !sub[i].hasWrite;

        if (hasNoReadsAnywhere || hasNoReadsOrWritesInSupers) {
          removableIndexes.insert(i);
        }
      }
      if (!removableIndexes.empty()) {
        // We are removing fields. Reorder them to allow that, as in the general
        // case we can only remove fields from the end, so that if our subtypes
        // still need the fields they can append them. For example:
        //
        //  type A     = { x: i32, y: f64 };
        //  type B : A = { x: 132, y: f64, z: v128 };
        //
        // If field x is used in B but never in A then we want to remove it, but
        // we cannot end up with this:
        //
        //  type A     = { y: f64 };
        //  type B : A = { x: 132, y: f64, z: v128 };
        //
        // Here B no longer extends A's fields. Instead, we reorder A, which
        // then imposes the same order on B's fields:
        //
        //  type A     = { y: f64, x: i32 };
        //  type B : A = { y: f64, x: i32, z: v128 };
        //
        // And after that, it is safe to remove x in A: B will then append it,
        // just like it appends z, leading to this:
        //
        //  type A     = { y: f64 };
        //  type B : A = { y: f64, x: i32, z: v128 };
        //
        auto& indexesAfterRemoval = indexesAfterRemovals[type];
        assert(indexesAfterRemoval.empty());

        // How many fields were skipped over (removed). This is also the amount
        // we must skip when assigning new indexes.
        Index skip = 0;

        // If we have a super, then we extend it, and must match its fields.
        // That is, we can only append fields: we cannot reorder or remove any
        // field that is in the super.
        if (auto super = type.getSuperType()) {
          // We must have visited the super before.
          assert(indexesAfterRemovals.count(*super));
          auto& superIndexes = indexesAfterRemovals[*super];
          indexesAfterRemoval = superIndexes;

          // Update |skip| to reflect any removed fields.
          if (!indexesAfterRemoval.empty()) {
            // The maximum new index indicates how many fields remained.
            auto remaining = std::max(indexesAfterRemoval) + 1; // Remvoedfield is -1!11
            skip += indexesAfterRemoval.size() - remaining;
          }
        }
XXX
        // Add any additional fields on top of the super, as needed.
        auto numSuperFields = indexesAfterRemoval.size();
        indexesAfterRemoval.resize(fields.size());
        for (Index i = numSuperFields; i < fields.size(); i++) {
          if (!removableIndexes.count(i)) {
            indexesAfterRemoval[i] = i - skip;
          } else {
            indexesAfterRemoval[i] = RemovedField;
            skip++;
          }
        }
      }
    }

    // If we found fields that can be removed, remove them from instructions.
    // (Note that we must do this first, while we still have the old heap types
    // that we can identify, and only after this should we update all the types
    // throughout the module.)
    if (!indexesAfterRemovals.empty()) {
      removeFieldsInInstructions(*module);
    }

    // Update the types in the entire module.
    if (!indexesAfterRemovals.empty() || !canBecomeImmutable.empty()) {
      updateTypes(*module);
    }
  }

  void updateTypes(Module& wasm) {
    class TypeRewriter : public GlobalTypeRewriter {
      GlobalTypeOptimization& parent;

    public:
      TypeRewriter(Module& wasm, GlobalTypeOptimization& parent)
        : GlobalTypeRewriter(wasm), parent(parent) {}

      void modifyStruct(HeapType oldStructType, Struct& struct_) override {
        auto& newFields = struct_.fields;

        // Adjust immutability.
        auto immIter = parent.canBecomeImmutable.find(oldStructType);
        if (immIter != parent.canBecomeImmutable.end()) {
          auto& immutableVec = immIter->second;
          for (Index i = 0; i < immutableVec.size(); i++) {
            if (immutableVec[i]) {
              newFields[i].mutable_ = Immutable;
            }
          }
        }

        // Remove fields where we can.
        auto remIter = parent.indexesAfterRemovals.find(oldStructType);
        if (remIter != parent.indexesAfterRemovals.end()) {
          auto& indexesAfterRemoval = remIter->second;
          Index removed = 0;
          for (Index i = 0; i < newFields.size(); i++) {
            auto newIndex = indexesAfterRemoval[i];
            if (newIndex != RemovedField) {
              newFields[newIndex] = newFields[i];
            } else {
              removed++;
            }
          }
          newFields.resize(newFields.size() - removed);

          // Update field names as well. The Type Rewriter cannot do this for
          // us, as it does not know which old fields map to which new ones (it
          // just keeps the names in sequence).
          auto iter = wasm.typeNames.find(oldStructType);
          if (iter != wasm.typeNames.end()) {
            auto& nameInfo = iter->second;

            // Make a copy of the old ones to base ourselves off of as we do so.
            auto oldFieldNames = nameInfo.fieldNames;

            // Clear the old names and write the new ones.
            nameInfo.fieldNames.clear();
            for (Index i = 0; i < oldFieldNames.size(); i++) {
              auto newIndex = indexesAfterRemoval[i];
              if (newIndex != RemovedField && oldFieldNames.count(i)) {
                assert(oldFieldNames[i].is());
                nameInfo.fieldNames[newIndex] = oldFieldNames[i];
              }
            }
          }
        }
      }
    };

    TypeRewriter(wasm, *this).update();
  }

  // After updating the types to remove certain fields, we must also remove
  // them from struct instructions.
  void removeFieldsInInstructions(Module& wasm) {
    struct FieldRemover : public WalkerPass<PostWalker<FieldRemover>> {
      bool isFunctionParallel() override { return true; }

      GlobalTypeOptimization& parent;

      FieldRemover(GlobalTypeOptimization& parent) : parent(parent) {}

      std::unique_ptr<Pass> create() override {
        return std::make_unique<FieldRemover>(parent);
      }

      void visitStructNew(StructNew* curr) {
        if (curr->type == Type::unreachable) {
          return;
        }
        if (curr->isWithDefault()) {
          // Nothing to do, a default was written and will no longer be.
          return;
        }

        auto iter = parent.indexesAfterRemovals.find(curr->type.getHeapType());
        if (iter == parent.indexesAfterRemovals.end()) {
          return;
        }
        auto& indexesAfterRemoval = iter->second;

        auto& operands = curr->operands;
        assert(indexesAfterRemoval.size() == operands.size());

        // Check for side effects in removed fields. If there are any, we must
        // use locals to save the values (while keeping them in order).
        bool useLocals = false;
        for (Index i = 0; i < operands.size(); i++) {
          auto newIndex = indexesAfterRemoval[i];
          if (newIndex == RemovedField &&
              EffectAnalyzer(getPassOptions(), *getModule(), operands[i])
                .hasUnremovableSideEffects()) {
            useLocals = true;
            break;
          }
        }
        if (useLocals) {
          auto* func = getFunction();
          if (!func) {
            Fatal() << "TODO: side effects in removed fields in globals\n";
          }
          ChildLocalizer localizer(curr, func, *getModule(), getPassOptions());
          replaceCurrent(localizer.getReplacement());
        }

        // Remove the unneeded operands.
        Index removed = 0;
        for (Index i = 0; i < operands.size(); i++) {
          auto newIndex = indexesAfterRemoval[i];
          if (newIndex != RemovedField) {
            assert(newIndex < operands.size());
            operands[newIndex] = operands[i];
          } else {
            removed++;
          }
        }
        operands.resize(operands.size() - removed);
      }

      void visitStructSet(StructSet* curr) {
        if (curr->ref->type == Type::unreachable) {
          return;
        }

        auto newIndex = getNewIndex(curr->ref->type.getHeapType(), curr->index);
        if (newIndex != RemovedField) {
          // Map to the new index.
          curr->index = newIndex;
        } else {
          // This field was removed, so just emit drops of our children, plus a
          // trap if the ref is null. Note that we must preserve the order of
          // operations here: the trap on a null ref happens after the value,
          // which might have side effects.
          Builder builder(*getModule());
          auto flipped = getResultOfFirst(curr->ref,
                                          builder.makeDrop(curr->value),
                                          getFunction(),
                                          getModule(),
                                          getPassOptions());
          replaceCurrent(
            builder.makeDrop(builder.makeRefAs(RefAsNonNull, flipped)));
        }
      }

      void visitStructGet(StructGet* curr) {
        if (curr->ref->type == Type::unreachable) {
          return;
        }

        auto newIndex = getNewIndex(curr->ref->type.getHeapType(), curr->index);
        // We must not remove a field that is read from.
        assert(newIndex != RemovedField);
        curr->index = newIndex;
      }

    private:
      Index getNewIndex(HeapType type, Index index) {
        auto iter = parent.indexesAfterRemovals.find(type);
        if (iter == parent.indexesAfterRemovals.end()) {
          return index;
        }
        auto& indexesAfterRemoval = iter->second;
        auto newIndex = indexesAfterRemoval[index];
        assert(newIndex < indexesAfterRemoval.size() ||
               newIndex == RemovedField);
        return newIndex;
      }
    };

    FieldRemover remover(*this);
    remover.run(getPassRunner(), &wasm);
    remover.runOnModuleCode(getPassRunner(), &wasm);
  }
};

} // anonymous namespace

Pass* createGlobalTypeOptimizationPass() {
  return new GlobalTypeOptimization();
}

} // namespace wasm
