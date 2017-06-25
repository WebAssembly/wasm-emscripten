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
// Folds duplicate code together, saving space
//
// We fold tails of code where they merge and moving the code
// to the merge point is helpful. There are two cases here:
//  * blocks, we merge the fallthrough + the breaks
//  * if-else, we merge the arms
//

#include "wasm.h"
#include "pass.h"
#include "wasm-builder.h"
#include "ast_utils.h"
#include "ast/label-utils.h"

namespace wasm {

static const Index WORTH_ADDING_BLOCK_TO_REMOVE_THIS_MUCH = 3;

struct ExpressionMarker : public PostWalker<ExpressionMarker, UnifiedExpressionVisitor<ExpressionMarker>> {
  std::set<Expression*>& marked;

  ExpressionMarker(std::set<Expression*>& marked, Expression* expr) : marked(marked) {
    walk(expr);
  }

  void visitExpression(Expression* expr) {
    marked.insert(expr);
  }
};

struct CodeFolding : public WalkerPass<ControlFlowWalker<CodeFolding>> {
  bool isFunctionParallel() override { return true; }

  Pass* create() override { return new CodeFolding; }

  // information about a "tail" - code that reaches a point that we can
  // merge (e.g., a branch and some code leading up to it)
  struct Tail {
    Expression* expr; // nullptr if this is a fallthrough
    Block* block; // the enclosing block of code we hope to merge at its tail
    Expression** pointer; // for an expr with no parent block, the location it is at, so we can replace it

    // For a fallthrough
    Tail(Block* block) : expr(nullptr), block(block), pointer(nullptr) {}
    // For a break
    Tail(Expression* expr, Block* block) : expr(expr), block(block), pointer(nullptr) {
      validate();
    }
    Tail(Expression* expr, Expression** pointer) : expr(expr), block(nullptr), pointer(pointer) {}

    bool isFallthrough() const { return expr == nullptr; }

    void validate() const {
      if (expr && block) {
        assert(block->list.back() == expr);
      }
    }
  };

  // state

  bool anotherPass;

  // pass state

  std::map<Name, std::vector<Tail>> breakTails; // break target name => tails that reach it
  std::vector<Tail> unreachableTails; // tails leading to (unreachable)
  std::vector<Tail> returnTails; // tails leading to (return)
  std::set<Name> unoptimizables; // break target names that we can't handle
  std::set<Expression*> modifieds; // modified code should not be processed again, wait for next pass

  // walking

  void visitBreak(Break* curr) {
    if (curr->condition || curr->value) {
      unoptimizables.insert(curr->name);
    } else {
      // we can only optimize if we are at the end of the parent block
      Block* parent = controlFlowStack.back()->dynCast<Block>();
      if (parent && curr == parent->list.back()) {
        breakTails[curr->name].push_back(Tail(curr, parent));
      } else {
        unoptimizables.insert(curr->name);
      }
    }
  }

  void visitSwitch(Switch* curr) {
    for (auto target : curr->targets) {
      unoptimizables.insert(target);
    }
    unoptimizables.insert(curr->default_);
  }

  void visitUnreachable(Unreachable* curr) {
    // we can only optimize if we are at the end of the parent block
    if (!controlFlowStack.empty()) {
      Block* parent = controlFlowStack.back()->dynCast<Block>();
      if (parent && curr == parent->list.back()) {
        unreachableTails.push_back(Tail(curr, parent));
      }
    }
  }

  void visitReturn(Return* curr) {
    if (!controlFlowStack.empty()) {
      // we can easily optimize if we are at the end of the parent block
      Block* parent = controlFlowStack.back()->dynCast<Block>();
      if (parent && curr == parent->list.back()) {
        returnTails.push_back(Tail(curr, parent));
        return;
      }
    }
    // otherwise, if we have a large value, it might be worth optimizing us as well
    returnTails.push_back(Tail(curr, getCurrentPointer()));
  }

  void visitBlock(Block* curr) {
    if (!curr->name.is()) return;
    if (unoptimizables.count(curr->name) > 0) return;
    auto iter = breakTails.find(curr->name);
    if (iter == breakTails.end()) return;
    // looks promising
    auto& tails = iter->second;
    // see if there is a fallthrough
    bool hasFallthrough = true;
    for (auto* child : curr->list) {
      if (child->type == unreachable) {
        hasFallthrough = false;
      }
    }
    if (hasFallthrough) {
      tails.push_back({ Tail(curr) });
    }
    optimizeExpressionTails(tails, curr);
  }

  void visitIf(If* curr) {
    if (!curr->ifFalse) return;
    // if both sides are identical, this is easy to fold
    // (except if the condition is unreachable and we return a value, then we can't just replace
    // outselves with a drop
    if (ExpressionAnalyzer::equal(curr->ifTrue, curr->ifFalse)) {
      Builder builder(*getModule());
      // remove if (4 bytes), remove one arm, add drop (1), add block (3),
      // so this must be a net savings
      markAsModified(curr);
      auto* ret = builder.makeSequence(
        builder.makeDrop(curr->condition),
        curr->ifTrue
      );
      // we must ensure we present the same type as the if had
      ret->finalize(curr->type);
      replaceCurrent(ret);
    } else {
      // if both are blocks, look for a tail we can merge
      auto* left = curr->ifTrue->dynCast<Block>();
      auto* right = curr->ifFalse->dynCast<Block>();
      // we need nameless blocks, as if there is a name, someone might branch
      // to the end, skipping the code we want to merge
      if (left && right &&
          !left->name.is() && !right->name.is()) {
        std::vector<Tail> tails = { Tail(left), Tail(right) };
        optimizeExpressionTails(tails, curr);
      }
    }
  }

  void doWalkFunction(Function* func) {
    anotherPass = true;
    while (anotherPass) {
      anotherPass = false;
      WalkerPass<ControlFlowWalker<CodeFolding>>::doWalkFunction(func);
      optimizeTerminatingTails(unreachableTails);
      // optimize returns at the end, so we can benefit from a fallthrough if there is a value TODO: separate passes for them?
      optimizeTerminatingTails(returnTails);
      // TODO add fallthrough for returns
      // TODO optimzier returns not in blocks, a big return value can be worth it
      // clean up
      breakTails.clear();
      unreachableTails.clear();
      returnTails.clear();
      unoptimizables.clear();
      modifieds.clear();
    }
  }

private:
  // optimize tails that reach the outside of an expression. code that is identical in all
  // paths leading to the block exit can be merged.
  template<typename T>
  void optimizeExpressionTails(std::vector<Tail>& tails, T* curr) {
    if (tails.size() < 2) return;
    // see if anything is untoward, and we should not do this
    for (auto& tail : tails) {
      if (tail.expr && modifieds.count(tail.expr) > 0) return;
      if (modifieds.count(tail.block) > 0) return;
      // if we were not modified, then we should be valid for processing
      tail.validate();
    }
    // we can ignore the final br in a tail
    auto effectiveSize = [&](const Tail& tail) {
      auto ret = tail.block->list.size();
      if (!tail.isFallthrough()) {
        ret--;
      }
      return ret;
    };
    // the mergeable items do not include the final br in a tail
    auto getMergeable = [&](const Tail& tail, Index num) {
      return tail.block->list[effectiveSize(tail) - num - 1];
    };
    // we are going to remove duplicate elements and add a block.
    // so for this to make sense, we need the size of the duplicate
    // elements to be worth that extra block (although, there is
    // some chance the block would get merged higher up, see later)
    std::vector<Expression*> mergeable; // the elements we can merge
    Index num = 0; // how many elements back from the tail to look at
    Index saved = 0; // how much we can save
    while (1) {
      // check if this num is still relevant
      bool stop = false;
      for (auto& tail : tails) {
        assert(tail.block);
        if (num >= effectiveSize(tail)) {
          // one of the lists is too short
          stop = true;
          break;
        }
      }
      if (stop) break;
      auto* item = getMergeable(tails[0], num);
      for (auto& tail : tails) {
        if (!ExpressionAnalyzer::equal(item, getMergeable(tail, num))) {
          // one of the lists has a different item
          stop = true;
          break;
        }
      }
      if (stop) break;
      // we found another one we can merge
      mergeable.push_back(item);
      num++;
      saved += Measurer::measure(item);
    }
    if (saved == 0) return;
    // we may be able to save enough.
    if (saved < WORTH_ADDING_BLOCK_TO_REMOVE_THIS_MUCH) {
      // it's not obvious we can save enough. see if we get rid
      // of a block, that would justify this
      bool willEmptyBlock = false;
      for (auto& tail : tails) {
        // it is enough to zero out the block, or leave just one
        // element, as then the block can be replaced with that
        if (num >= tail.block->list.size() - 1) {
          willEmptyBlock = true;
          break;
        }
      }
      if (!willEmptyBlock) {
        // last chance, if our parent is a block, then it should be
        // fine to create a new block here, it will be merged up
        assert(curr == controlFlowStack.back()); // we are an if or a block, at the top
        if (controlFlowStack.size() <= 1) {
          return; // no parent at all
                  // TODO: if we are the toplevel in the function, then in the binary format
                  //       we might avoid emitting a block, so the same logic applies here?
        }
        auto* parent = controlFlowStack[controlFlowStack.size() - 2]->dynCast<Block>();
        if (!parent) {
          return; // parent is not a block
        }
        bool isChild = false;
        for (auto* child : parent->list) {
          if (child == curr) {
            isChild = true;
            break;
          }
        }
        if (!isChild) {
          return; // not a child, something in between
        }
      }
    }
    // this is worth doing, do it!
    for (auto& tail : tails) {
      // remove the items we are merging / moving
      // first, mark them as modified, so we don't try to handle them
      // again in this pass, which might be buggy
      markAsModified(tail.block);
      // we must preserve the br if there is one
      Expression* last = nullptr;
      if (!tail.isFallthrough()) {
        last = tail.block->list.back();
        tail.block->list.pop_back();
      }
      for (Index i = 0; i < mergeable.size(); i++) {
        tail.block->list.pop_back();
      }
      if (!tail.isFallthrough()) {
        tail.block->list.push_back(last);
      }
      // the blocks lose their endings, so any values are gone, and the blocks
      // are now either none or unreachable
      tail.block->finalize();
    }
    // since we managed a merge, then it might open up more opportunities later
    anotherPass = true;
    // make a block with curr + the merged code
    Builder builder(*getModule());
    auto* block = builder.makeBlock();
    block->list.push_back(curr);
    while (!mergeable.empty()) {
      block->list.push_back(mergeable.back());
      mergeable.pop_back();
    }
    auto oldType = curr->type;
    // NB: we template-specialize so that this calls the proper finalizer for
    //     the type
    curr->finalize();
    // ensure the replacement has the same type, so the outside is not surprised
    block->finalize(oldType);
    replaceCurrent(block);
  }

  // optimize tails that terminate control flow in this function, so we
  // are (1) merge just a few of them, we don't need all like with the
  // branches to a block, and (2) we do it on the function body
  void optimizeTerminatingTails(std::vector<Tail>& tails) {
    if (tails.size() < 2) return;
    // remove things that are untoward and cannot be optimized
    tails.erase(std::remove_if(tails.begin(), tails.end(), [&](Tail& tail) {
      if (tail.expr && modifieds.count(tail.expr) > 0) return true;
      if (tail.block && modifieds.count(tail.block) > 0) return true;
      // if we were not modified, then we should be valid for processing
      tail.validate();
      return false;
    }), tails.end());
    // now let's try to find subsets that are mergeable. we don't look hard
    // for the most optimal; further passes may find more
    // effectiveSize: TODO: special-case fallthrough, matters for returns
    auto effectiveSize = [&](Tail& tail) -> Index {
      if (tail.block) {
        return tail.block->list.size();
      } else {
        return 1;
      }
    };
    // getItem: returns the relevant item from the tail. this includes the
    //          final item
    //          TODO: special-case fallthrough, matters for returns
    auto getItem = [&](Tail& tail, Index num) {
      if (tail.block) {
        return tail.block->list[effectiveSize(tail) - num - 1];
      } else {
        return tail.expr;
      }
    };
    Index num = 0; // how many elements back from the tail to look at
    while (1) {
      // work on a test set of tails, see if we can optimize here. if we fail,
      // we'll go back to the previous num and set of tails
      auto test = tails;
      // remove too-short tails
      test.erase(std::remove_if(test.begin(), test.end(), [&](Tail& tail) {
        return num >= effectiveSize(tail);
      }), test.end());
      // if no tails passed the test, this num was too much
      if (test.empty()) break;
      // now we want to find a mergeable item - any item that is equal among a subset
      std::map<uint32_t, std::vector<Expression*>> hashed; // hash value => expressions with that hash
      for (auto& tail : test) {
        auto* item = getItem(tail, num);
        hashed[ExpressionAnalyzer::hash(item)].push_back(item);
      }
      std::vector<Expression*> best;
      for (auto& iter : hashed) {
        auto& items = iter.second;
        if (items.size() == 1) continue;
        assert(items.size() > 0);
        // look for an item that has another match.
        while (items.size() >= 2) {
          auto first = items[0];
          std::vector<Expression*> others;
          items.erase(std::remove_if(items.begin(), items.end(), [&](Expression* item) {
            if (item == first || // don't bother comparing the first
                ExpressionAnalyzer::equal(item, first)) {
              // equal, keep it
              return false;
            } else {
              // unequal, look at it later
              others.push_back(item);
              return true;
            }
          }), items.end());
          if (items.size() >= 2 && items.size() > best.size()) {
            best.swap(items);
          }
          items.swap(others);
        }
      }
      // if there are no mergeable items, this num was too much
      if (best.empty()) break;
      assert(best.size() >= 2);
      // we found another one we can merge. remove the irrelevant tails
      auto* correct = best[0];
      test.erase(std::remove_if(test.begin(), test.end(), [&](Tail& tail) {
        auto* item = getItem(tail, num);
        return !ExpressionAnalyzer::equal(item, correct);
      }), test.end());
      assert(test.size() >= 2);
      // carry on
      num++;
      tails.swap(test);
    }
    // if we found nothing, stop
    if (num == 0) return;
    // great, we found something! let's scan and measure it
    std::vector<Expression*> mergeable; // the elements we can merge
    Index saved = 0; // how much we can save
    for (Index i = 0; i < num; i++) {
      auto item = getItem(tails[0], i);
      mergeable.push_back(item);
      saved += Measurer::measure(item) * (tails.size() - 1);
    }
    // compure the cost: in non-fallthroughs, we are replacing the final
    // element with a br; for a fallthrough, if there is one, we must
    // add a return element (for the function body, so it doesn't reach us)
    // TODO: handle fallthroughts for return
    Index cost = tails.size();
    // we also need to add two blocks: for us to break to, and to contain
    // that block and the merged code. very possibly one of the blocks
    // can be removed, though
    cost += WORTH_ADDING_BLOCK_TO_REMOVE_THIS_MUCH;
    // is it worth it?
    // TODO: if not worth it, perhaps a smaller num or choice of options might have worked?
    if (saved <= cost) return;
    // this is worth doing, do it!
    // since we managed a merge, then it might open up more opportunities later
    anotherPass = true;
    Builder builder(*getModule());
    LabelUtils::LabelManager labels(getFunction()); // TODO: don't create one per merge, linear in function size
    Name innerName = labels.getUnique("folding-inner");
    for (auto& tail : tails) {
      // remove the items we are merging / moving, and add a break
      // also mark as modified, so we don't try to handle them
      // again in this pass, which might be buggy
      if (tail.block) {
        markAsModified(tail.block);
        for (Index i = 0; i < mergeable.size(); i++) {
          tail.block->list.pop_back();
        }
        tail.block->list.push_back(builder.makeBreak(innerName));
        tail.block->finalize(tail.block->type);
      } else {
        markAsModified(tail.expr);
        *tail.pointer = builder.makeBreak(innerName);
      }
    }
    // make a block with the old body + the merged code
    auto* old = getFunction()->body;
    auto* inner = builder.makeBlock();
    inner->name = innerName;
    if (old->type == unreachable) {
      // the old body is not flowed out of anyhow, so just put it there
      inner->list.push_back(old);
    } else {
      // otherwise, we must not flow out to the merged code
      if (old->type == none) {
        inner->list.push_back(old);
        inner->list.push_back(builder.makeReturn());
      } else {
        // looks like we must return this. but if it's a toplevel block
        // then it might be marked as having a type, but not actually
        // returning it (we marked it as such for wasm type-checking
        // rules, and now it won't be toplevel in the function, it can
        // change)
        auto* toplevel = old->dynCast<Block>();
        if (toplevel) toplevel->finalize();
        if (old->type != unreachable) {
          inner->list.push_back(builder.makeReturn(old));
        } else {
          inner->list.push_back(old);
        }
      }
    }
    inner->finalize();
    auto* outer = builder.makeBlock();
    outer->list.push_back(inner);
    while (!mergeable.empty()) {
      outer->list.push_back(mergeable.back());
      mergeable.pop_back();
    }
    outer->finalize(getFunction()->result);
    getFunction()->body = outer;
    // ensure the replacement has the same type, so the outside is not surprised
  }

  void markAsModified(Expression* curr) {
    ExpressionMarker marker(modifieds, curr);
  }
};

Pass *createCodeFoldingPass() {
  return new CodeFolding();
}

} // namespace wasm

