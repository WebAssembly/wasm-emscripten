/*
 * Copyright 2022 WebAssembly Community Group participants
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

#include "ir/iteration.h"
#include "ir/local-structural-dominance.h"
#include "support/small_set.h"
#include "support/small_vector.h"

namespace wasm {

LocalStructuralDominance::LocalStructuralDominance(Function* func,
                                                   Module& wasm,
                                                   Mode mode) {
  if (!wasm.features.hasReferenceTypes()) {
    // No references, so nothing to look at.
    return;
  }

  auto num = func->getNumLocals();

  bool hasRefVar = false;
  for (Index i = func->getNumParams(); i < num; i++) {
    if (func->getLocalType(i).isRef()) {
      hasRefVar = true;
      break;
    }
  }
  if (!hasRefVar) {
    return;
  }

  if (mode == IgnoreNullable) {
    bool hasNonNullableVar = false;
    for (auto var : func->vars) {
      // Check if we have any non-nullable vars at all.
      if (var.isNonNullable()) {
        hasNonNullableVar = true;
        break;
      }
    }
    if (!hasNonNullableVar) {
      return;
    }
  }








  struct Scanner : public PostWalker<Scanner> {
    std::set<Index>& nonDominatingIndexes;

    Scanner(Function* func, Mode mode, std::set<Index>& nonDominatingIndexes) : nonDominatingIndexes(nonDominatingIndexes) {
      auto num = func->getNumLocals();
      localsSet.resize(num);

      // Parameters always dominate.
      for (Index i = 0; i < func->getNumParams(); i++) {
        localsSet[i] = true;
      }

      for (Index i = func->getNumParams(); i < func->getNumLocals(); i++) {
        auto type = func->getLocalType(i);
        // Mark locals we don't need to care about as "set". We never do any
        // work for such a local.
        if (!type.isRef() || (mode == IgnoreNullable && type.isNullable())) {
          localsSet[i] = true;
        }
      }

      // We begin with a new scope for the function, and then we start on the
      // body. (Note that we don't need to exit that scope, that work would not
      // do anything useful.)
      doBeginScope(this, nullptr);

      walk(func->body);
    }

    // The locals that have been set, and so at the current time, they
    // structurally dominate.
    std::vector<bool> localsSet;

    using Locals = SmallVector<Index, 5>;

    // When we exit a control flow structure, we must undo the locals that it set.
    std::vector<Locals> cleanupStack;

    static void doBeginScope(Scanner* self, Expression** currp) {
      // TODO: could push one only when first needed. Set a pointer to know.
      self->cleanupStack.emplace_back();
    }

    static void doEndScope(Scanner* self, Expression** currp) {
      assert(!self->cleanupStack.empty());
      for (auto index : self->cleanupStack.back()) {
        assert(self->localsSet[index]);
        self->localsSet[index] = false;
      }
      self->cleanupStack.pop_back();
    }

    static void scan(Scanner* self, Expression** currp) {
      Expression* curr = *currp;

      switch (curr->_id) {
        case Expression::Id::InvalidId:
          WASM_UNREACHABLE("bad id");

        // Local operations
        case Expression::Id::LocalGetId: {
          auto index = curr->cast<LocalGet>()->index;
          if (!self->localsSet[index]) {
            self->nonDominatingIndexes.insert(index);
          }
          break;
        }
        case Expression::Id::LocalSetId: {
          auto index = curr->cast<LocalSet>()->index;
          if (!self->localsSet[index]) {
            // This local is now set until the end of this scope.
            self->localsSet[index] = true;
            self->cleanupStack.back().push_back(index);
          }
          break;
        }

        // Control flow structures.
        case Expression::Id::BlockId: {
          self->pushTask(Scanner::doEndScope, currp);
          auto& list = curr->cast<Block>()->list;
          for (int i = int(list.size()) - 1; i >= 0; i--) {
            self->pushTask(Scanner::scan, &list[i]);
          }
          self->pushTask(Scanner::doBeginScope, currp);
          break;
        }
        case Expression::Id::IfId: {
          if (curr->cast<If>()->ifFalse) {
            self->pushTask(Scanner::doEndScope, currp);
            self->maybePushTask(Scanner::scan, &curr->cast<If>()->ifFalse);
            self->pushTask(Scanner::doBeginScope, currp);
          }
          self->pushTask(Scanner::doEndScope, currp);
          self->pushTask(Scanner::scan, &curr->cast<If>()->ifTrue);
          self->pushTask(Scanner::doBeginScope, currp);
          self->pushTask(Scanner::scan, &curr->cast<If>()->condition);
          break;
        }
        case Expression::Id::LoopId: {
          self->pushTask(Scanner::doEndScope, currp);
          self->pushTask(Scanner::scan, &curr->cast<Loop>()->body);
          self->pushTask(Scanner::doBeginScope, currp);
          break;
        }
        case Expression::Id::TryId: {
          auto& list = curr->cast<Try>()->catchBodies;
          for (int i = int(list.size()) - 1; i >= 0; i--) {
            self->pushTask(Scanner::doEndScope, currp);
            self->pushTask(Scanner::scan, &list[i]);
            self->pushTask(Scanner::doBeginScope, currp);
          }
          self->pushTask(Scanner::doEndScope, currp);
          self->pushTask(Scanner::scan, &curr->cast<Try>()->body);
          self->pushTask(Scanner::doBeginScope, currp);
          break;
        }

        default: {
          // Control flow structures have been handled. This is an expression,
          // which we scan normally.
          assert(!Properties::isControlFlowStructure(curr));
          PostWalker<Scanner>::scan(self, currp);
        }
      }
    }
  };

  Scanner(func, mode, nonDominatingIndexes);
























#if 0


  // Our main work stack.
  struct WorkItem {
    enum {
      // When we first see an expression we scan it and add work items for it
      // and its children.
      Scan,
      // Visit a specific instruction. This is only ever called on a LocalSet
      // due to the optimizations below.
      Visit,
      // Enter or exit a scope
      EnterScope,
      ExitScope
    } op;

    Expression* curr;
  };
  SmallVector<WorkItem, 5> workStack;

  // The stack begins with a new scope for the function, and then we start on
  // the body. (Note that we don't need to exit that scope, that work would not
  // do anything useful.)
  workStack.push_back(WorkItem{WorkItem::Scan, func->body});
  workStack.push_back(WorkItem{WorkItem::EnterScope, nullptr});

  while (!workStack.empty()) {
    auto item = workStack.back();
    workStack.pop_back();

    if (item.op == WorkItem::Scan) {
      if (!Properties::isControlFlowStructure(item.curr)) {
#if 0
        auto childIterator = ChildIterator(item.curr);
        auto& children = childIterator.children;
        if (children.empty()) {
          // No children, so just visit here right now.
          //
          // The only such instruction we care about is a (relevant) local.get.
          if (auto* get = item.curr->dynCast<LocalGet>()) {
            auto index = get->index;
            if (!localsSet[index]) {
              nonDominatingIndexes.insert(index);
            }
          }

          continue;
        }

        // Otherwise, prepare to visit here after our children.
        //
        // The only such instruction we need to visit is a (relevant) local.set.
        if (auto* set = item.curr->dynCast<LocalSet>()) {
          auto index = set->index;
          if (!localsSet[index]) {
            workStack.push_back(WorkItem{WorkItem::Visit, set});
          }
        }
        for (auto* child : children) {
          workStack.push_back(WorkItem{WorkItem::Scan, *child});
        }

#else

        auto* curr = item.curr; // TODO move up

#define DELEGATE_ID curr->_id

#define DELEGATE_START(id)                                                     \
  auto* cast = curr->cast<id>();                                               \
  WASM_UNUSED(cast);                                                           \
  if (DELEGATE_ID == Expression::LocalSetId) { /* type check here? */          \
    auto* set = cast->cast<LocalSet>();                                        \
    auto index = set->index;                                                   \
    if (!localsSet[index]) {                                                   \
      workStack.push_back(WorkItem{WorkItem::Visit, set});                     \
    }                                                                          \
  } else if (DELEGATE_ID == Expression::LocalGetId) { /* type check here? */   \
    /* no children, so just visit it right now */                              \
    auto* get = cast->cast<LocalGet>();                                        \
    auto index = get->index;                                                   \
    if (!localsSet[index]) {                                                   \
      nonDominatingIndexes.insert(index);                                      \
    }                                                                          \
  }

#define DELEGATE_GET_FIELD(id, field) cast->field

#define DELEGATE_FIELD_CHILD(id, field)                                        \
  workStack.push_back(WorkItem{WorkItem::Scan, cast->field});

#define DELEGATE_FIELD_OPTIONAL_CHILD(id, field)                               \
  if (cast->field) {                                                           \
    workStack.push_back(WorkItem{WorkItem::Scan, cast->field});                \
  }

#define DELEGATE_FIELD_INT(id, field)
#define DELEGATE_FIELD_INT_ARRAY(id, field)
#define DELEGATE_FIELD_LITERAL(id, field)
#define DELEGATE_FIELD_NAME(id, field)
#define DELEGATE_FIELD_NAME_VECTOR(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_DEF(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_USE(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR(id, field)
#define DELEGATE_FIELD_TYPE(id, field)
#define DELEGATE_FIELD_HEAPTYPE(id, field)
#define DELEGATE_FIELD_ADDRESS(id, field)

#include "wasm-delegations-fields.def"

#endif

        continue;
      }

      // This is a control flow structure.

      // First, go through the structure children. Blocks are special in that
      // all their children go in a single scope.
      if (item.curr->is<Block>()) {
        workStack.push_back(WorkItem{WorkItem::ExitScope, nullptr});
        for (auto* child : StructuralChildIterator(item.curr).children) {
          workStack.push_back(WorkItem{WorkItem::Scan, *child});
        }
        workStack.push_back(WorkItem{WorkItem::EnterScope, nullptr});
      } else {
        for (auto* child : StructuralChildIterator(item.curr).children) {
          workStack.push_back(WorkItem{WorkItem::ExitScope, nullptr});
          workStack.push_back(WorkItem{WorkItem::Scan, *child});
          workStack.push_back(WorkItem{WorkItem::EnterScope, nullptr});
        }
      }

      // Next handle value children, which are not involved in structuring (like
      // the If condition).
      for (auto* child : ValueChildIterator(item.curr).children) {
        workStack.push_back(WorkItem{WorkItem::Scan, *child});
      }
    } else if (item.op == WorkItem::Visit) {
      auto* set = item.curr->cast<LocalSet>();
      auto index = set->index;
      if (!localsSet[index]) {
        // This local is now set until the end of this scope.
        localsSet[index] = true;
        cleanupStack.back().push_back(index);
      }
    } else if (item.op == WorkItem::EnterScope) {
      cleanupStack.emplace_back();
    } else if (item.op == WorkItem::ExitScope) {
      assert(!cleanupStack.empty());
      for (auto index : cleanupStack.back()) {
        assert(localsSet[index]);
        localsSet[index] = false;
      }
      cleanupStack.pop_back();
    } else {
      WASM_UNREACHABLE("bad op");
    }
  }
#endif
}

} // namespace wasm
