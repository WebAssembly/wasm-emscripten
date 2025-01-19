/*
 * Copyright 2016 WebAssembly Community Group participants
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

#include "Relooper.h"

#include <stdlib.h>
#include <string.h>

#include <list>
#include <stack>
#include <string>

#include "ir/branch-utils.h"
#include "ir/utils.h"
#include "parsing.h"

namespace CFG {

template<class T, class U>
static bool contains(const T& container, const U& contained) {
  return !!container.count(contained);
}

#ifdef RELOOPER_DEBUG
static void PrintDebug(const char* Format, ...);
#define DebugDump(x, ...) Debugging::Dump(x, __VA_ARGS__)
#else
#define PrintDebug(x, ...)
#define DebugDump(x, ...)
#endif

// Rendering utilities

static wasm::Expression* HandleFollowupMultiples(wasm::Expression* Ret,
                                                 Shape* Parent,
                                                 RelooperBuilder& Builder,
                                                 bool InLoop) {
  if (!Parent->Next) {
    return Ret;
  }

  auto* Curr = Ret->dynCast<wasm::Block>();
  if (!Curr || Curr->name.is()) {
    Curr = Builder.makeBlock(Ret);
  }
  // for each multiple after us, we create a block target for breaks to reach
  while (Parent->Next) {
    auto* Multiple = Shape::IsMultiple(Parent->Next);
    if (!Multiple) {
      break;
    }
    for (auto& [Id, Body] : Multiple->InnerMap) {
      Curr->name = Builder.getBlockBreakName(Id);
      Curr->finalize(); // it may now be reachable, via a break
      auto* Outer = Builder.makeBlock(Curr);
      Outer->list.push_back(Body->Render(Builder, InLoop));
      Outer->finalize(); // TODO: not really necessary
      Curr = Outer;
    }
    Parent->Next = Parent->Next->Next;
  }
  // after the multiples is a simple or a loop, in both cases we must hit an
  // entry block, and so this is the last one we need to take into account now
  // (this is why we require that loops hit an entry).
  if (Parent->Next) {
    auto* Simple = Shape::IsSimple(Parent->Next);
    if (Simple) {
      // breaking on the next block's id takes us out, where we
      // will reach its rendering
      Curr->name = Builder.getBlockBreakName(Simple->Inner->Id);
    } else {
      // add one break target per entry for the loop
      auto* Loop = Shape::IsLoop(Parent->Next);
      assert(Loop);
      assert(Loop->Entries.size() > 0);
      if (Loop->Entries.size() == 1) {
        Curr->name = Builder.getBlockBreakName((*Loop->Entries.begin())->Id);
      } else {
        for (auto* Entry : Loop->Entries) {
          Curr->name = Builder.getBlockBreakName(Entry->Id);
          Curr->finalize();
          auto* Outer = Builder.makeBlock(Curr);
          Outer->finalize(); // TODO: not really necessary
          Curr = Outer;
        }
      }
    }
  }
  Curr->finalize();
  return Curr;
}

// Branch

Branch::Branch(wasm::Expression* ConditionInit, wasm::Expression* CodeInit)
  : Condition(ConditionInit), Code(CodeInit) {}

Branch::Branch(std::vector<wasm::Index>&& ValuesInit,
               wasm::Expression* CodeInit)
  : Condition(nullptr), Code(CodeInit) {
  if (ValuesInit.size() > 0) {
    SwitchValues = std::make_unique<std::vector<wasm::Index>>(ValuesInit);
  }
  // otherwise, it is the default
}

wasm::Expression*
Branch::Render(RelooperBuilder& Builder, Block* Target, bool SetLabel) {
  auto* Ret = Builder.makeBlock();
  if (Code) {
    Ret->list.push_back(Code);
  }
  if (SetLabel) {
    Ret->list.push_back(Builder.makeSetLabel(Target->Id));
  }
  if (Type == Break) {
    Ret->list.push_back(Builder.makeBlockBreak(Target->Id));
  } else if (Type == Continue) {
    assert(Ancestor);
    Ret->list.push_back(Builder.makeShapeContinue(Ancestor->Id));
  }
  Ret->finalize();
  return Ret;
}

// Block

Block::Block(Relooper* relooper,
             wasm::Expression* CodeInit,
             wasm::Expression* SwitchConditionInit)
  : relooper(relooper), Code(CodeInit), SwitchCondition(SwitchConditionInit),
    IsCheckedMultipleEntry(false) {}

void Block::AddBranchTo(Block* Target,
                        wasm::Expression* Condition,
                        wasm::Expression* Code) {
  // cannot add more than one branch to the same target
  assert(!contains(BranchesOut, Target));
  BranchesOut[Target] = relooper->AddBranch(Condition, Code);
}

void Block::AddSwitchBranchTo(Block* Target,
                              std::vector<wasm::Index>&& Values,
                              wasm::Expression* Code) {
  // cannot add more than one branch to the same target
  assert(!contains(BranchesOut, Target));
  BranchesOut[Target] = relooper->AddBranch(std::move(Values), Code);
}

wasm::Expression* Block::Render(RelooperBuilder& Builder, bool InLoop) {
  auto* Ret = Builder.makeBlock();
  if (IsCheckedMultipleEntry && InLoop) {
    Ret->list.push_back(Builder.makeSetLabel(0));
  }
  if (Code) {
    Ret->list.push_back(Code);
  }

  if (!ProcessedBranchesOut.size()) {
    Ret->finalize();
    return Ret;
  }

  // in some cases it is clear we can avoid setting label, see later
  bool SetLabel = true;

  // A setting of the label variable (label = x) is necessary if it can
  // cause an impact. The main case is where we set label to x, then elsewhere
  // we check if label is equal to that value, i.e., that label is an entry
  // in a multiple block. We also need to reset the label when we enter
  // that block, so that each setting is a one-time action: consider
  //
  //    while (1) {
  //      if (check) label = 1;
  //      if (label == 1) { label = 0 }
  //    }
  //
  // (Note that this case is impossible due to fusing, but that is not
  // material here.) So setting to 0 is important just to clear the 1 for
  // future iterations.
  // TODO: When inside a loop, if necessary clear the label variable
  //       once on the top, and never do settings that are in effect clears

  // Fusing: If the next is a Multiple, we can fuse it with this block. Note
  // that we must be the Inner of a Simple, so fusing means joining a Simple
  // to a Multiple. What happens there is that all options in the Multiple
  // *must* appear in the Simple (the Simple is the only one reaching the
  // Multiple), so we can remove the Multiple and add its independent groups
  // into the Simple's branches.
  MultipleShape* Fused = Shape::IsMultiple(Parent->Next);
  if (Fused) {
    PrintDebug("Fusing Multiple to Simple\n", 0);
    Parent->Next = Parent->Next->Next;
    // When the Multiple has the same number of groups as we have branches,
    // they will all be fused, so it is safe to not set the label at all.
    // If a switch, then we can have multiple branches to the same target
    // (in different table indexes), and so this check is not sufficient
    // TODO: optimize
    if (SetLabel && Fused->InnerMap.size() == ProcessedBranchesOut.size() &&
        !SwitchCondition) {
      SetLabel = false;
    }
  }

  // The block we branch to without checking the condition, if none of the other
  // conditions held.
  Block* DefaultTarget = nullptr;

  // Find the default target, the one without a condition
  for (auto& [Target, Details] : ProcessedBranchesOut) {
    if ((!SwitchCondition && !Details->Condition) ||
        (SwitchCondition && !Details->SwitchValues)) {
      assert(!DefaultTarget &&
             "block has branches without a default (nullptr for the "
             "condition)"); // Must be exactly one default // nullptr
      DefaultTarget = Target;
    }
  }
  // Since each Block* must* branch somewhere, this must be set
  assert(DefaultTarget);

  // root of the main part, that we are about to emit
  wasm::Expression* Root = nullptr;

  if (!SwitchCondition) {
    // We'll emit a chain of if-elses
    wasm::If* CurrIf = nullptr;

    // we build an if, then add a child, then add a child to that, etc., so we
    // must finalize them in reverse order
    std::vector<wasm::If*> finalizeStack;

    wasm::Expression* RemainingConditions = nullptr;

    for (auto iter = ProcessedBranchesOut.begin();; iter++) {
      Block* Target;
      Branch* Details;
      if (iter != ProcessedBranchesOut.end()) {
        Target = iter->first;
        if (Target == DefaultTarget) {
          continue; // done at the end
        }
        Details = iter->second;
        // must have a condition if this is not the default target
        assert(Details->Condition);
      } else {
        Target = DefaultTarget;
        Details = ProcessedBranchesOut[DefaultTarget];
      }
      bool SetCurrLabel = SetLabel && Target->IsCheckedMultipleEntry;
      bool HasFusedContent = Fused && contains(Fused->InnerMap, Target->Id);
      if (HasFusedContent) {
        assert(Details->Type == Branch::Break);
        Details->Type = Branch::Direct;
      }
      wasm::Expression* CurrContent = nullptr;
      bool IsDefault = iter == ProcessedBranchesOut.end();
      if (SetCurrLabel || Details->Type != Branch::Direct || HasFusedContent ||
          Details->Code) {
        CurrContent = Details->Render(Builder, Target, SetCurrLabel);
        if (HasFusedContent) {
          CurrContent = Builder.blockify(
            CurrContent,
            Fused->InnerMap.find(Target->Id)->second->Render(Builder, InLoop));
        }
      }
      // If there is nothing to show in this branch, omit the condition
      if (CurrContent) {
        if (IsDefault) {
          wasm::Expression* Now;
          if (RemainingConditions) {
            Now = Builder.makeIf(RemainingConditions, CurrContent);
            finalizeStack.push_back(Now->cast<wasm::If>());
          } else {
            Now = CurrContent;
          }
          if (!CurrIf) {
            assert(!Root);
            Root = Now;
          } else {
            CurrIf->ifFalse = Now;
            CurrIf->finalize();
          }
        } else {
          auto* Now = Builder.makeIf(Details->Condition, CurrContent);
          finalizeStack.push_back(Now);
          if (!CurrIf) {
            assert(!Root);
            Root = CurrIf = Now;
          } else {
            CurrIf->ifFalse = Now;
            CurrIf->finalize();
            CurrIf = Now;
          }
        }
      } else {
        auto* Now = Builder.makeUnary(wasm::EqZInt32, Details->Condition);
        if (RemainingConditions) {
          RemainingConditions =
            Builder.makeBinary(wasm::AndInt32, RemainingConditions, Now);
        } else {
          RemainingConditions = Now;
        }
      }
      if (IsDefault) {
        break;
      }
    }

    // finalize the if-chains
    while (finalizeStack.size() > 0) {
      wasm::If* curr = finalizeStack.back();
      finalizeStack.pop_back();
      curr->finalize();
    }

  } else {
    // Emit a switch
    auto Base = std::string("switch$") + std::to_string(Id);
    auto SwitchDefault = wasm::Name(Base + "$default");
    auto SwitchLeave = wasm::Name(Base + "$leave");
    std::map<Block*, wasm::Name> BlockNameMap;
    auto* Outer = Builder.makeBlock();
    auto* Inner = Outer;
    std::vector<wasm::Name> Table;
    for (auto& [Target, Details] : ProcessedBranchesOut) {
      wasm::Name CurrName;
      if (Details->SwitchValues) {
        CurrName = wasm::Name(Base + "$case$" + std::to_string(Target->Id));
      } else {
        CurrName = SwitchDefault;
      }
      // generate the content for this block
      bool SetCurrLabel = SetLabel && Target->IsCheckedMultipleEntry;
      bool HasFusedContent = Fused && contains(Fused->InnerMap, Target->Id);
      if (HasFusedContent) {
        assert(Details->Type == Branch::Break);
        Details->Type = Branch::Direct;
      }
      wasm::Expression* CurrContent = nullptr;
      if (SetCurrLabel || Details->Type != Branch::Direct || HasFusedContent ||
          Details->Code) {
        CurrContent = Details->Render(Builder, Target, SetCurrLabel);
        if (HasFusedContent) {
          CurrContent = Builder.blockify(
            CurrContent,
            Fused->InnerMap.find(Target->Id)->second->Render(Builder, InLoop));
        }
      }
      // generate a block to branch to, if we have content
      if (CurrContent) {
        auto* NextOuter = Builder.makeBlock();
        NextOuter->list.push_back(Outer);
        // breaking on Outer leads to the content in NextOuter
        Outer->name = CurrName;
        NextOuter->list.push_back(CurrContent);
        // if this is not a dead end, also need to break to the outside this is
        // both an optimization, and avoids incorrectness as adding a break in
        // unreachable code can make a place look reachable that isn't
        if (CurrContent->type != wasm::Type::unreachable) {
          NextOuter->list.push_back(Builder.makeBreak(SwitchLeave));
        }
        // prepare for more nesting
        Outer = NextOuter;
      } else {
        CurrName = SwitchLeave; // just go out straight from the table
        if (!Details->SwitchValues) {
          // this is the default, and it has no content. So make the default be
          // the leave
          for (auto& Value : Table) {
            if (Value == SwitchDefault) {
              Value = SwitchLeave;
            }
          }
          SwitchDefault = SwitchLeave;
        }
      }
      if (Details->SwitchValues) {
        for (auto Value : *Details->SwitchValues) {
          while (Table.size() <= Value) {
            Table.push_back(SwitchDefault);
          }
          Table[Value] = CurrName;
        }
      }
    }
    // finish up the whole pattern
    Outer->name = SwitchLeave;
    Inner->list.push_back(
      Builder.makeSwitch(Table, SwitchDefault, SwitchCondition));
    Root = Outer;
  }

  if (Root) {
    Ret->list.push_back(Root);
  }
  Ret->finalize();

  return Ret;
}

// SimpleShape

wasm::Expression* SimpleShape::Render(RelooperBuilder& Builder, bool InLoop) {
  auto* Ret = Inner->Render(Builder, InLoop);
  Ret = HandleFollowupMultiples(Ret, this, Builder, InLoop);
  if (Next) {
    Ret = Builder.makeSequence(Ret, Next->Render(Builder, InLoop));
  }
  return Ret;
}

// MultipleShape

wasm::Expression* MultipleShape::Render(RelooperBuilder& Builder, bool InLoop) {
  // TODO: consider switch
  // emit an if-else chain
  wasm::If* FirstIf = nullptr;
  wasm::If* CurrIf = nullptr;
  std::vector<wasm::If*> finalizeStack;
  for (auto& [Id, Body] : InnerMap) {
    auto* Now =
      Builder.makeIf(Builder.makeCheckLabel(Id), Body->Render(Builder, InLoop));
    finalizeStack.push_back(Now);
    if (!CurrIf) {
      FirstIf = CurrIf = Now;
    } else {
      CurrIf->ifFalse = Now;
      CurrIf->finalize();
      CurrIf = Now;
    }
  }
  while (finalizeStack.size() > 0) {
    wasm::If* curr = finalizeStack.back();
    finalizeStack.pop_back();
    curr->finalize();
  }
  wasm::Expression* Ret = Builder.makeBlock(FirstIf);
  Ret = HandleFollowupMultiples(Ret, this, Builder, InLoop);
  if (Next) {
    Ret = Builder.makeSequence(Ret, Next->Render(Builder, InLoop));
  }
  return Ret;
}

// LoopShape

wasm::Expression* LoopShape::Render(RelooperBuilder& Builder, bool InLoop) {
  wasm::Expression* Ret = Builder.makeLoop(Builder.getShapeContinueName(Id),
                                           Inner->Render(Builder, true));
  Ret = HandleFollowupMultiples(Ret, this, Builder, InLoop);
  if (Next) {
    Ret = Builder.makeSequence(Ret, Next->Render(Builder, InLoop));
  }
  return Ret;
}

// Relooper

Relooper::Relooper(wasm::Module* ModuleInit)
  : Module(ModuleInit), Root(nullptr), MinSize(false), BlockIdCounter(1),
    ShapeIdCounter(0) { // block ID 0 is reserved for clearings
}

Block* Relooper::AddBlock(wasm::Expression* CodeInit,
                          wasm::Expression* SwitchConditionInit) {

  auto block = std::make_unique<Block>(this, CodeInit, SwitchConditionInit);
  block->Id = BlockIdCounter++;
  auto* blockPtr = block.get();
  Blocks.push_back(std::move(block));
  return blockPtr;
}

Branch* Relooper::AddBranch(wasm::Expression* ConditionInit,
                            wasm::Expression* CodeInit) {
  auto branch = std::make_unique<Branch>(ConditionInit, CodeInit);
  auto* branchPtr = branch.get();
  Branches.push_back(std::move(branch));
  return branchPtr;
}
Branch* Relooper::AddBranch(std::vector<wasm::Index>&& ValuesInit,
                            wasm::Expression* CodeInit) {
  auto branch = std::make_unique<Branch>(std::move(ValuesInit), CodeInit);
  auto* branchPtr = branch.get();
  Branches.push_back(std::move(branch));
  return branchPtr;
}

SimpleShape* Relooper::AddSimpleShape() {
  auto shape = std::make_unique<SimpleShape>();
  shape->Id = ShapeIdCounter++;
  auto* shapePtr = shape.get();
  Shapes.push_back(std::move(shape));
  return shapePtr;
}

MultipleShape* Relooper::AddMultipleShape() {
  auto shape = std::make_unique<MultipleShape>();
  shape->Id = ShapeIdCounter++;
  auto* shapePtr = shape.get();
  Shapes.push_back(std::move(shape));
  return shapePtr;
}

LoopShape* Relooper::AddLoopShape() {
  auto shape = std::make_unique<LoopShape>();
  shape->Id = ShapeIdCounter++;
  auto* shapePtr = shape.get();
  Shapes.push_back(std::move(shape));
  return shapePtr;
}

namespace {

using BlockList = std::list<Block*>;

struct RelooperRecursor {
  Relooper* Parent;
  RelooperRecursor(Relooper* ParentInit) : Parent(ParentInit) {}
};

struct Liveness : public RelooperRecursor {
  Liveness(Relooper* Parent) : RelooperRecursor(Parent) {}
  BlockSet Live;

  void FindLive(Block* Root) {
    BlockList ToInvestigate;
    ToInvestigate.push_back(Root);
    while (ToInvestigate.size() > 0) {
      Block* Curr = ToInvestigate.front();
      ToInvestigate.pop_front();
      if (contains(Live, Curr)) {
        continue;
      }
      Live.insert(Curr);
      for (auto& iter : Curr->BranchesOut) {
        ToInvestigate.push_back(iter.first);
      }
    }
  }
};

using BranchBlock = std::pair<Branch*, Block*>;

struct Optimizer : public RelooperRecursor {
  Block* Entry;

  Optimizer(Relooper* Parent, Block* EntryInit)
    : RelooperRecursor(Parent), Entry(EntryInit) {
    // TODO: there are likely some rare but possible O(N^2) cases with this
    // looping
    bool More = true;
#if RELOOPER_OPTIMIZER_DEBUG
    std::cout << "pre-optimize\n";
    for (auto* Block : Parent->Blocks) {
      DebugDump(Block, "pre-block");
    }
#endif

    // First, run one-time preparatory passes.
    CanonicalizeCode();

    // Loop over passes that allow further reduction.
    while (More) {
      More = false;
      More = SkipEmptyBlocks() || More;
      More = MergeEquivalentBranches() || More;
      More = UnSwitch() || More;
      More = MergeConsecutiveBlocks() || More;
      // TODO: Merge identical blocks. This would avoid taking into account
      // their position / how they are reached, which means that the merging may
      // add overhead, so we do it carefully:
      //  * Merging a large-enough block is good for size, and we do it
      //    in we are in MinSize mode, which means we can tolerate slightly
      //    slower throughput.
      // TODO: Fuse a non-empty block with a single successor.
    }

    // Finally, run one-time final passes.
    // TODO

#if RELOOPER_OPTIMIZER_DEBUG
    std::cout << "post-optimize\n";
    for (auto* Block : Parent->Blocks) {
      DebugDump(Block, "post-block");
    }
#endif
  }

  // We will be performing code comparisons, so do some basic canonicalization
  // to avoid things being unequal for silly reasons.
  void CanonicalizeCode() {
    for (auto& Block : Parent->Blocks) {
      Block->Code = Canonicalize(Block->Code);
      for (auto& [_, Branch] : Block->BranchesOut) {
        if (Branch->Code) {
          Branch->Code = Canonicalize(Branch->Code);
        }
      }
    }
  }

  // If a branch goes to an empty block which has one target,
  // and there is no phi or switch to worry us, just skip through.
  bool SkipEmptyBlocks() {
    bool Worked = false;
    for (auto& CurrBlock : Parent->Blocks) {
      // Generate a new set of branches out TODO optimize
      BlockBranchMap NewBranchesOut;
      for (auto& iter : CurrBlock->BranchesOut) {
        auto* Next = iter.first;
        auto* NextBranch = iter.second;
        auto* First = Next;
        auto* Replacement = First;
#if RELOOPER_OPTIMIZER_DEBUG
        std::cout << " maybeskip from " << Block->Id << " to next=" << Next->Id
                  << '\n';
#endif
        std::unordered_set<decltype(Replacement)> Seen;
        while (1) {
          if (IsEmpty(Next) && Next->BranchesOut.size() == 1) {
            auto iter = Next->BranchesOut.begin();
            Block* NextNext = iter->first;
            Branch* NextNextBranch = iter->second;
            assert(!NextNextBranch->Condition && !NextNextBranch->SwitchValues);
            if (!NextNextBranch->Code) { // TODO: handle extra code too
              // We can skip through!
              Next = Replacement = NextNext;
              // If we've already seen this, stop - it's an infinite loop of
              // empty blocks we can skip through.
              if (!Seen.emplace(Replacement).second) {
                // Stop here. Note that if we started from X and ended up with X
                // once more, then Replacement == First and so lower down we
                // will not report that we did any work, avoiding an infinite
                // loop due to always thinking there is more work to do.
                break;
              } else {
                // Otherwise, keep going.
                continue;
              }
            }
          }
          break;
        }
        if (Replacement != First) {
#if RELOOPER_OPTIMIZER_DEBUG
          std::cout << "  skip to replacement! " << CurrBlock->Id << " -> "
                    << First->Id << " -> " << Replacement->Id << '\n';
#endif
          Worked = true;
        }
        // Add a branch to the target (which may be the unchanged original) in
        // the set of new branches. If it's a replacement, it may collide, and
        // we need to merge.
        if (NewBranchesOut.count(Replacement)) {
#if RELOOPER_OPTIMIZER_DEBUG
          std::cout << "  merge\n";
#endif
          MergeBranchInto(NextBranch, NewBranchesOut[Replacement]);
        } else {
          NewBranchesOut[Replacement] = NextBranch;
        }
      }
      CurrBlock->BranchesOut.swap(NewBranchesOut);
    }
    return Worked;
  }

  // Our IR has one Branch from each block to one of its targets, so there
  // is nothing to reduce there, but different targets may in fact be
  // equivalent in their *contents*.
  bool MergeEquivalentBranches() {
    bool Worked = false;
    for (auto& ParentBlock : Parent->Blocks) {
#if RELOOPER_OPTIMIZER_DEBUG
      std::cout << "at parent " << ParentBlock->Id << '\n';
#endif
      if (ParentBlock->BranchesOut.size() >= 2) {
        std::unordered_map<size_t, std::vector<BranchBlock>> HashedBranchesOut;
        std::vector<Block*> BlocksToErase;
        for (auto& [CurrBlock, CurrBranch] : ParentBlock->BranchesOut) {
#if RELOOPER_OPTIMIZER_DEBUG
          std::cout << "  consider child " << CurrBlock->Id << '\n';
#endif
          if (CurrBranch->Code) {
            // We can't merge code; ignore
            continue;
          }
          auto HashValue = Hash(CurrBlock);
          auto& HashedSiblings = HashedBranchesOut[HashValue];
          // Check if we are equivalent to any of them - if so, merge us.
          bool Merged = false;
          for (auto& [SiblingBranch, SiblingBlock] : HashedSiblings) {
            if (HaveEquivalentContents(CurrBlock, SiblingBlock)) {
#if RELOOPER_OPTIMIZER_DEBUG
              std::cout << "    equiv! to " << SiblingBlock->Id << '\n';
#endif
              MergeBranchInto(CurrBranch, SiblingBranch);
              BlocksToErase.push_back(CurrBlock);
              Merged = true;
              Worked = true;
            }
#if RELOOPER_OPTIMIZER_DEBUG
            else {
              std::cout << "    same hash, but not equiv to "
                        << SiblingBlock->Id << '\n';
            }
#endif
          }
          if (!Merged) {
            HashedSiblings.emplace_back(CurrBranch, CurrBlock);
          }
        }
        for (auto* Curr : BlocksToErase) {
          ParentBlock->BranchesOut.erase(Curr);
        }
      }
    }
    return Worked;
  }

  // Merge consecutive blocks, that is, A -> B where no other branches go to B.
  // In that case we are guaranteed to not increase code size.
  bool MergeConsecutiveBlocks() {
    bool Worked = false;
    // First, count predecessors.
    std::map<Block*, size_t> NumPredecessors;
    for (auto& CurrBlock : Parent->Blocks) {
      for (auto& iter : CurrBlock->BranchesOut) {
        auto* NextBlock = iter.first;
        NumPredecessors[NextBlock]++;
      }
    }
    NumPredecessors[Entry]++;
    for (auto& CurrBlock : Parent->Blocks) {
      if (CurrBlock->BranchesOut.size() == 1) {
        auto iter = CurrBlock->BranchesOut.begin();
        auto* NextBlock = iter->first;
        auto* NextBranch = iter->second;
        assert(NumPredecessors[NextBlock] > 0);
        if (NextBlock != CurrBlock.get() && NumPredecessors[NextBlock] == 1) {
          // Good to merge!
          wasm::Builder Builder(*Parent->Module);
          // Merge in code on the branch as well, if any.
          if (NextBranch->Code) {
            CurrBlock->Code =
              Builder.makeSequence(CurrBlock->Code, NextBranch->Code);
          }
          CurrBlock->Code =
            Builder.makeSequence(CurrBlock->Code, NextBlock->Code);
          // Use the next block's branching behavior
          CurrBlock->BranchesOut.swap(NextBlock->BranchesOut);
          NextBlock->BranchesOut.clear();
          CurrBlock->SwitchCondition = NextBlock->SwitchCondition;
          // The next block now has no predecessors.
          NumPredecessors[NextBlock] = 0;
          Worked = true;
        }
      }
    }
    return Worked;
  }

  // Removes unneeded switches - if only one branch is left, the default, then
  // no switch is needed.
  bool UnSwitch() {
    bool Worked = false;
    for (auto& ParentBlock : Parent->Blocks) {
#if RELOOPER_OPTIMIZER_DEBUG
      std::cout << "un-switching at " << ParentBlock->Id << ' '
                << !!ParentBlock->SwitchCondition << ' '
                << ParentBlock->BranchesOut.size() << '\n';
#endif
      if (ParentBlock->SwitchCondition) {
        if (ParentBlock->BranchesOut.size() <= 1) {
#if RELOOPER_OPTIMIZER_DEBUG
          std::cout << "  un-switching!: " << ParentBlock->Id << '\n';
#endif
          ParentBlock->SwitchCondition = nullptr;
          if (!ParentBlock->BranchesOut.empty()) {
            assert(!ParentBlock->BranchesOut.begin()->second->SwitchValues);
          }
          Worked = true;
        }
      } else {
        // If the block has no switch, the branches must not as well.
#ifndef NDEBUG
        for (auto& [_, CurrBranch] : ParentBlock->BranchesOut) {
          assert(!CurrBranch->SwitchValues);
        }
#endif
      }
    }
    return Worked;
  }

private:
  wasm::Expression* Canonicalize(wasm::Expression* Curr) {
    wasm::Builder Builder(*Parent->Module);
    // Our preferred form is a block with no name and a flat list
    // with Nops removed, and extra Unreachables removed as well.
    // If the block would contain one item, return just the item.
    wasm::Block* Outer = Curr->dynCast<wasm::Block>();
    if (!Outer) {
      Outer = Builder.makeBlock(Curr);
    } else if (Outer->name.is()) {
      // Perhaps the name can be removed.
      if (!wasm::BranchUtils::BranchSeeker::has(Outer, Outer->name)) {
        Outer->name = wasm::Name();
      } else {
        Outer = Builder.makeBlock(Curr);
      }
    }
    Flatten(Outer);
    if (Outer->list.size() == 1) {
      return Outer->list[0];
    } else {
      return Outer;
    }
  }

  void Flatten(wasm::Block* Outer) {
    wasm::ExpressionList NewList(Parent->Module->allocator);
    bool SeenUnreachableType = false;
    auto Add = [&](wasm::Expression* Curr) {
      if (Curr->is<wasm::Nop>()) {
        // Do nothing with it.
        return;
      } else if (Curr->is<wasm::Unreachable>()) {
        // If we already saw an unreachable-typed item, emit no
        // Unreachable nodes after it.
        if (SeenUnreachableType) {
          return;
        }
      }
      NewList.push_back(Curr);
      if (Curr->type == wasm::Type::unreachable) {
        SeenUnreachableType = true;
      }
    };
    std::function<void(wasm::Block*)> FlattenIntoNewList =
      [&](wasm::Block* Curr) {
        assert(!Curr->name.is());
        for (auto* Item : Curr->list) {
          if (auto* Block = Item->dynCast<wasm::Block>()) {
            if (Block->name.is()) {
              // Leave it whole, it's not a trivial block.
              Add(Block);
            } else {
              FlattenIntoNewList(Block);
            }
          } else {
            // A random item.
            Add(Item);
          }
        }
        // All the items have been moved out.
        Curr->list.clear();
      };
    FlattenIntoNewList(Outer);
    assert(Outer->list.empty());
    Outer->list.swap(NewList);
  }

  bool IsEmpty(Block* Curr) {
    if (Curr->SwitchCondition) {
      // This is non-trivial, so treat it as a non-empty block.
      return false;
    }
    return IsEmpty(Curr->Code);
  }

  bool IsEmpty(wasm::Expression* Code) {
    if (Code->is<wasm::Nop>()) {
      return true; // a nop
    }
    if (auto* WasmBlock = Code->dynCast<wasm::Block>()) {
      for (auto* Item : WasmBlock->list) {
        if (!IsEmpty(Item)) {
          return false;
        }
      }
      return true; // block with no non-empty contents
    }
    return false;
  }

  // Checks functional equivalence, namely: the Code and SwitchCondition.
  // We also check the branches out, *non-recursively*: that is, we check
  // that they are literally identical, not that they can be computed to
  // be equivalent.
  bool HaveEquivalentContents(Block* A, Block* B) {
    if (!IsPossibleCodeEquivalent(A->SwitchCondition, B->SwitchCondition)) {
      return false;
    }
    if (!IsCodeEquivalent(A->Code, B->Code)) {
      return false;
    }
    if (A->BranchesOut.size() != B->BranchesOut.size()) {
      return false;
    }
    for (auto& [ABlock, ABranch] : A->BranchesOut) {
      if (B->BranchesOut.count(ABlock) == 0) {
        return false;
      }
      auto* BBranch = B->BranchesOut[ABlock];
      if (!IsPossibleCodeEquivalent(ABranch->Condition, BBranch->Condition)) {
        return false;
      }
      if (!IsPossibleUniquePtrEquivalent(ABranch->SwitchValues,
                                         BBranch->SwitchValues)) {
        return false;
      }
      if (!IsPossibleCodeEquivalent(ABranch->Code, BBranch->Code)) {
        return false;
      }
    }
    return true;
  }

  // Checks if values referred to by pointers are identical, allowing the code
  // to also be nullptr
  template<typename T>
  static bool IsPossibleUniquePtrEquivalent(std::unique_ptr<T>& A,
                                            std::unique_ptr<T>& B) {
    if (A == B) {
      return true;
    }
    if (!A || !B) {
      return false;
    }
    return *A == *B;
  }

  // Checks if code is equivalent, allowing the code to also be nullptr
  static bool IsPossibleCodeEquivalent(wasm::Expression* A,
                                       wasm::Expression* B) {
    if (A == B) {
      return true;
    }
    if (!A || !B) {
      return false;
    }
    return IsCodeEquivalent(A, B);
  }

  static bool IsCodeEquivalent(wasm::Expression* A, wasm::Expression* B) {
    return wasm::ExpressionAnalyzer::equal(A, B);
  }

  // Merges one branch into another. Valid under the assumption that the
  // blocks they reach are identical, and so one branch is enough for both
  // with a unified condition.
  // Only one is allowed to have code, as the code may have side effects,
  // and we don't have a way to order or resolve those, unless the code
  // is equivalent.
  void MergeBranchInto(Branch* Curr, Branch* Into) {
    assert(Curr != Into);
    if (Curr->SwitchValues) {
      if (!Into->SwitchValues) {
        assert(!Into->Condition);
        // Merging into the already-default, nothing to do.
      } else {
        Into->SwitchValues->insert(Into->SwitchValues->end(),
                                   Curr->SwitchValues->begin(),
                                   Curr->SwitchValues->end());
      }
    } else {
      if (!Curr->Condition) {
        // This is now the new default. Whether Into has a condition
        // or switch values, remove them all to make us the default.
        Into->Condition = nullptr;
        Into->SwitchValues.reset();
      } else if (!Into->Condition) {
        // Nothing to do, already the default.
      } else {
        assert(!Into->SwitchValues);
        // Merge them, checking both.
        Into->Condition =
          wasm::Builder(*Parent->Module)
            .makeBinary(wasm::OrInt32, Into->Condition, Curr->Condition);
      }
    }
    if (!Curr->Code) {
      // No code to merge in.
    } else if (!Into->Code) {
      // Just use the code being merged in.
      Into->Code = Curr->Code;
    } else {
      assert(IsCodeEquivalent(Into->Code, Curr->Code));
      // Keep the code already there, either is fine.
    }
  }

  // Hashes the direct block contents, but not Relooper internals
  // (like Shapes). Only partially hashes the branches out, no
  // recursion: hashes the branch infos, looks at raw pointers
  // for the blocks.
  size_t Hash(Block* Curr) {
    auto digest = wasm::ExpressionAnalyzer::hash(Curr->Code);
    wasm::rehash(digest, uint8_t(1));
    if (Curr->SwitchCondition) {
      wasm::hash_combine(digest,
                         wasm::ExpressionAnalyzer::hash(Curr->SwitchCondition));
    }
    wasm::rehash(digest, uint8_t(2));
    for (auto& [CurrBlock, CurrBranch] : Curr->BranchesOut) {
      // Hash the Block* as a pointer TODO: full hash?
      wasm::rehash(digest, reinterpret_cast<size_t>(CurrBlock));
      // Hash the Branch info properly
      wasm::hash_combine(digest, Hash(CurrBranch));
    }
    return digest;
  }

  // Hashes the direct block contents, but not Relooper internals
  // (like Shapes).
  size_t Hash(Branch* Curr) {
    auto digest = wasm::hash(0);
    if (Curr->SwitchValues) {
      for (auto i : *Curr->SwitchValues) {
        wasm::rehash(digest, i); // TODO hash i
      }
    } else {
      if (Curr->Condition) {
        wasm::hash_combine(digest,
                           wasm::ExpressionAnalyzer::hash(Curr->Condition));
      }
    }
    wasm::rehash(digest, uint8_t(1));
    if (Curr->Code) {
      wasm::hash_combine(digest, wasm::ExpressionAnalyzer::hash(Curr->Code));
    }
    return digest;
  }
};

} // namespace

void Relooper::Calculate(Block* Entry) {
  // Optimize.
  Optimizer(this, Entry);

  // Find live blocks.
  Liveness Live(this);
  Live.FindLive(Entry);

  // Add incoming branches from live blocks, ignoring dead code
  for (unsigned i = 0; i < Blocks.size(); i++) {
    Block* Curr = Blocks[i].get();
    if (!contains(Live.Live, Curr)) {
      continue;
    }
    for (auto& [CurrBlock, _] : Curr->BranchesOut) {
      CurrBlock->BranchesIn.insert(Curr);
    }
  }

  // Recursively process the graph

  struct Analyzer : public RelooperRecursor {
    Analyzer(Relooper* Parent) : RelooperRecursor(Parent) {}

    // Create a list of entries from a block. If LimitTo is provided, only
    // results in that set will appear
    void GetBlocksOut(Block* Source,
                      BlockSet& Entries,
                      BlockSet* LimitTo = nullptr) {
      for (auto& [CurrBlock, _] : Source->BranchesOut) {
        if (!LimitTo || contains(*LimitTo, CurrBlock)) {
          Entries.insert(CurrBlock);
        }
      }
    }

    // Converts/processes all branchings to a specific target
    void Solipsize(Block* Target,
                   Branch::FlowType Type,
                   Shape* Ancestor,
                   BlockSet& From) {
      PrintDebug("Solipsizing branches into %d\n", Target->Id);
      DebugDump(From, "  relevant to solipsize: ");
      for (auto iter = Target->BranchesIn.begin();
           iter != Target->BranchesIn.end();) {
        Block* Prior = *iter;
        if (!contains(From, Prior)) {
          iter++;
          continue;
        }
        Branch* PriorOut = Prior->BranchesOut[Target];
        PriorOut->Ancestor = Ancestor;
        PriorOut->Type = Type;
        iter++; // carefully increment iter before erasing
        Target->BranchesIn.erase(Prior);
        Target->ProcessedBranchesIn.insert(Prior);
        Prior->BranchesOut.erase(Target);
        Prior->ProcessedBranchesOut[Target] = PriorOut;
        PrintDebug("  eliminated branch from %d\n", Prior->Id);
           }
    }

    Shape* MakeSimple(BlockSet& Blocks, Block* Inner, BlockSet& NextEntries) {
      PrintDebug("creating simple block with block #%d\n", Inner->Id);
      SimpleShape* Simple = Parent->AddSimpleShape();
      Simple->Inner = Inner;
      Inner->Parent = Simple;
      if (Blocks.size() > 1) {
        Blocks.erase(Inner);
        GetBlocksOut(Inner, NextEntries, &Blocks);
        BlockSet JustInner;
        JustInner.insert(Inner);
        for (auto* Next : NextEntries) {
          Solipsize(Next, Branch::Break, Simple, JustInner);
        }
      }
      return Simple;
    }

    Shape*
    MakeLoop(BlockSet& Blocks, BlockSet& Entries, BlockSet& NextEntries) {
      // Find the inner blocks in this loop. Proceed backwards from the entries
      // until you reach a seen block, collecting as you go.
      BlockSet InnerBlocks;
      BlockSet Queue = Entries;
      while (Queue.size() > 0) {
        Block* Curr = *(Queue.begin());
        Queue.erase(Queue.begin());
        if (!contains(InnerBlocks, Curr)) {
          // This element is new, mark it as inner and remove from outer
          InnerBlocks.insert(Curr);
          Blocks.erase(Curr);
          // Add the elements prior to it
          for (auto* Prev : Curr->BranchesIn) {
            Queue.insert(Prev);
          }
#if 0
          // Add elements it leads to, if they are dead ends. There is no reason not to hoist dead ends
          // into loops, as it can avoid multiple entries after the loop
          for (auto iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
            Block* Target = iter->first;
            if (Target->BranchesIn.size() <= 1 && Target->BranchesOut.size() == 0) {
              Queue.insert(Target);
            }
          }
#endif
        }
      }
      assert(InnerBlocks.size() > 0);

      for (auto* Curr : InnerBlocks) {
        for (auto& [Possible, _] : Curr->BranchesOut) {
          if (!contains(InnerBlocks, Possible)) {
            NextEntries.insert(Possible);
          }
        }
      }

#if 0
      // We can avoid multiple next entries by hoisting them into the loop.
      if (NextEntries.size() > 1) {
        BlockBlockSetMap IndependentGroups;
        FindIndependentGroups(NextEntries, IndependentGroups, &InnerBlocks);

        while (IndependentGroups.size() > 0 && NextEntries.size() > 1) {
          Block* Min = nullptr;
          int MinSize = 0;
          for (auto iter = IndependentGroups.begin(); iter != IndependentGroups.end(); iter++) {
            Block* Entry = iter->first;
            BlockSet &Blocks = iter->second;
            if (!Min || Blocks.size() < MinSize) { // TODO: code size, not # of blocks
              Min = Entry;
              MinSize = Blocks.size();
            }
          }
          // check how many new entries this would cause
          BlockSet &Hoisted = IndependentGroups[Min];
          bool abort = false;
          for (auto iter = Hoisted.begin(); iter != Hoisted.end() && !abort; iter++) {
            Block* Curr = *iter;
            for (auto iter = Curr->BranchesOut.begin(); iter != Curr->BranchesOut.end(); iter++) {
              Block* Target = iter->first;
              if (!contains(Hoisted, Target) && !contains(NextEntries, Target)) {
                // abort this hoisting
                abort = true;
                break;
              }
            }
          }
          if (abort) {
            IndependentGroups.erase(Min);
            continue;
          }
          // hoist this entry
          PrintDebug("hoisting %d into loop\n", Min->Id);
          NextEntries.erase(Min);
          for (auto iter = Hoisted.begin(); iter != Hoisted.end(); iter++) {
            Block* Curr = *iter;
            InnerBlocks.insert(Curr);
            Blocks.erase(Curr);
          }
          IndependentGroups.erase(Min);
        }
      }
#endif

      PrintDebug("creating loop block:\n", 0);
      DebugDump(InnerBlocks, "  inner blocks:");
      DebugDump(Entries, "  inner entries:");
      DebugDump(Blocks, "  outer blocks:");
      DebugDump(NextEntries, "  outer entries:");

      LoopShape* Loop = Parent->AddLoopShape();

      // Solipsize the loop, replacing with break/continue and marking branches
      // as Processed (will not affect later calculations) A. Branches to the
      // loop entries become a continue to this shape
      for (auto* Entry : Entries) {
        Solipsize(Entry, Branch::Continue, Loop, InnerBlocks);
      }
      // B. Branches to outside the loop (a next entry) become breaks on this
      // shape
      for (auto* Next : NextEntries) {
        Solipsize(Next, Branch::Break, Loop, InnerBlocks);
      }
      // Finish up
      Shape* Inner = Process(InnerBlocks, Entries);
      Loop->Inner = Inner;
      Loop->Entries = Entries;
      return Loop;
    }

    // For each entry, find the independent group reachable by it. The
    // independent group is the entry itself, plus all the blocks it can reach
    // that cannot be directly reached by another entry. Note that we ignore
    // directly reaching the entry itself by another entry.
    //   @param Ignore - previous blocks that are irrelevant
    void FindIndependentGroups(BlockSet& Entries,
                               BlockBlockSetMap& IndependentGroups,
                               BlockSet* Ignore = nullptr) {
      using BlockBlockMap = std::map<Block*, Block*>;

      struct HelperClass {
        BlockBlockSetMap& IndependentGroups;
        // For each block, which entry it belongs to. We have reached it from
        // there.
        BlockBlockMap Ownership;

        HelperClass(BlockBlockSetMap& IndependentGroupsInit)
          : IndependentGroups(IndependentGroupsInit) {}
        void InvalidateWithChildren(Block* New) { // TODO: rename New
          // Being in the list means you need to be invalidated
          BlockList ToInvalidate;
          ToInvalidate.push_back(New);
          while (ToInvalidate.size() > 0) {
            Block* Invalidatee = ToInvalidate.front();
            ToInvalidate.pop_front();
            Block* Owner = Ownership[Invalidatee];
            // Owner may have been invalidated, do not add to IndependentGroups!
            if (contains(IndependentGroups, Owner)) {
              IndependentGroups[Owner].erase(Invalidatee);
            }
            // may have been seen before and invalidated already
            if (Ownership[Invalidatee]) {
              Ownership[Invalidatee] = nullptr;
              for (auto& [Target, _] : Invalidatee->BranchesOut) {
                auto Known = Ownership.find(Target);
                if (Known != Ownership.end()) {
                  Block* TargetOwner = Known->second;
                  if (TargetOwner) {
                    ToInvalidate.push_back(Target);
                  }
                }
              }
            }
          }
        }
      };
      HelperClass Helper(IndependentGroups);

      // We flow out from each of the entries, simultaneously.
      // When we reach a new block, we add it as belonging to the one we got to
      // it from. If we reach a new block that is already marked as belonging to
      // someone, it is reachable by two entries and is not valid for any of
      // them. Remove it and all it can reach that have been visited.

      // Being in the queue means we just added this item, and we need to add
      // its children
      BlockList Queue;
      for (auto* Entry : Entries) {
        Helper.Ownership[Entry] = Entry;
        IndependentGroups[Entry].insert(Entry);
        Queue.push_back(Entry);
      }
      while (Queue.size() > 0) {
        Block* Curr = Queue.front();
        Queue.pop_front();
        // Curr must be in the ownership map if we are in the queue
        Block* Owner = Helper.Ownership[Curr];
        if (!Owner) {
          // we have been invalidated meanwhile after being reached from two
          // entries
          continue;
        }
        // Add all children
        for (auto& [New, _] : Curr->BranchesOut) {
          auto Known = Helper.Ownership.find(New);
          if (Known == Helper.Ownership.end()) {
            // New node. Add it, and put it in the queue
            Helper.Ownership[New] = Owner;
            IndependentGroups[Owner].insert(New);
            Queue.push_back(New);
            continue;
          }
          Block* NewOwner = Known->second;
          if (!NewOwner) {
            continue; // We reached an invalidated node
          }
          if (NewOwner != Owner) {
            // Invalidate this and all reachable that we have seen - we reached
            // this from two locations
            Helper.InvalidateWithChildren(New);
          }
          // otherwise, we have the same owner, so do nothing
        }
      }

      // Having processed all the interesting blocks, we remain with just one
      // potential issue: If a->b, and a was invalidated, but then b was later
      // reached by someone else, we must invalidate b. To check for this, we go
      // over all elements in the independent groups, if an element has a parent
      // which does *not* have the same owner, we must remove it and all its
      // children.

      for (auto* Entry : Entries) {
        BlockSet& CurrGroup = IndependentGroups[Entry];
        BlockList ToInvalidate;
        for (auto* Child : CurrGroup) {
          for (auto* Parent : Child->BranchesIn) {
            if (Ignore && contains(*Ignore, Parent)) {
              continue;
            }
            if (Helper.Ownership[Parent] != Helper.Ownership[Child]) {
              ToInvalidate.push_back(Child);
            }
          }
        }
        while (ToInvalidate.size() > 0) {
          Block* Invalidatee = ToInvalidate.front();
          ToInvalidate.pop_front();
          Helper.InvalidateWithChildren(Invalidatee);
        }
      }

      // Remove empty groups
      for (auto* Entry : Entries) {
        if (IndependentGroups[Entry].size() == 0) {
          IndependentGroups.erase(Entry);
        }
      }

#ifdef RELOOPER_DEBUG
      PrintDebug("Investigated independent groups:\n");
      for (auto& iter : IndependentGroups) {
        DebugDump(iter.second, " group: ");
      }
#endif
    }

    Shape* MakeMultiple(BlockSet& Blocks,
                        BlockSet& Entries,
                        BlockBlockSetMap& IndependentGroups,
                        BlockSet& NextEntries,
                        bool IsCheckedMultiple) {
      PrintDebug("creating multiple block with %d inner groups\n",
                 IndependentGroups.size());
      MultipleShape* Multiple = Parent->AddMultipleShape();
      BlockSet CurrEntries;
      for (auto& [CurrEntry, CurrBlocks] : IndependentGroups) {
        PrintDebug("  multiple group with entry %d:\n", CurrEntry->Id);
        DebugDump(CurrBlocks, "    ");
        // Create inner block
        CurrEntries.clear();
        CurrEntries.insert(CurrEntry);
        for (auto* CurrInner : CurrBlocks) {
          // Remove the block from the remaining blocks
          Blocks.erase(CurrInner);
          // Find new next entries and fix branches to them
          for (auto iter = CurrInner->BranchesOut.begin();
               iter != CurrInner->BranchesOut.end();) {
            Block* CurrTarget = iter->first;
            auto Next = iter;
            Next++;
            if (!contains(CurrBlocks, CurrTarget)) {
              NextEntries.insert(CurrTarget);
              Solipsize(CurrTarget, Branch::Break, Multiple, CurrBlocks);
            }
            iter = Next; // increment carefully because Solipsize can remove us
               }
        }
        Multiple->InnerMap[CurrEntry->Id] = Process(CurrBlocks, CurrEntries);
        if (IsCheckedMultiple) {
          CurrEntry->IsCheckedMultipleEntry = true;
        }
      }
      DebugDump(Blocks, "  remaining blocks after multiple:");
      // Add entries not handled as next entries, they are deferred
      for (auto* Entry : Entries) {
        if (!contains(IndependentGroups, Entry)) {
          NextEntries.insert(Entry);
        }
      }
      return Multiple;
    }

    // Main function.
    // Process a set of blocks with specified entries, returns a shape
    // The Make* functions receive a NextEntries. If they fill it with data,
    // those are the entries for the
    //   ->Next block on them, and the blocks are what remains in Blocks (which
    //   Make* modify). In this way we avoid recursing on Next (imagine a long
    //   chain of Simples, if we recursed we could blow the stack).
    Shape* Process(BlockSet& Blocks, BlockSet& InitialEntries) {
      PrintDebug("Process() called\n", 0);
      BlockSet* Entries = &InitialEntries;
      BlockSet TempEntries[2];
      int CurrTempIndex = 0;
      BlockSet* NextEntries;
      Shape* Ret = nullptr;
      Shape* Prev = nullptr;
#define Make(call)                                                             \
  Shape* Temp = call;                                                          \
  if (Prev)                                                                    \
    Prev->Next = Temp;                                                         \
  if (!Ret)                                                                    \
    Ret = Temp;                                                                \
  if (!NextEntries->size()) {                                                  \
    PrintDebug("Process() returning\n", 0);                                    \
    return Ret;                                                                \
  }                                                                            \
  Prev = Temp;                                                                 \
  Entries = NextEntries;                                                       \
  continue;
      while (1) {
        PrintDebug("Process() running\n", 0);
        DebugDump(Blocks, "  blocks : ");
        DebugDump(*Entries, "  entries: ");

        CurrTempIndex = 1 - CurrTempIndex;
        NextEntries = &TempEntries[CurrTempIndex];
        NextEntries->clear();

        if (Entries->size() == 0) {
          return Ret;
        }
        if (Entries->size() == 1) {
          Block* Curr = *(Entries->begin());
          if (Curr->BranchesIn.size() == 0) {
            // One entry, no looping ==> Simple
            Make(MakeSimple(Blocks, Curr, *NextEntries));
          }
          // One entry, looping ==> Loop
          Make(MakeLoop(Blocks, *Entries, *NextEntries));
        }

        // More than one entry, try to eliminate through a Multiple groups of independent blocks from entries.
        BlockBlockSetMap IndependentGroups;
        FindIndependentGroups(*Entries, IndependentGroups);

        PrintDebug("Independent groups: %d\n", IndependentGroups.size());

        if (IndependentGroups.size() >= 2) {
          // Identify dead-end groups to process
          std::vector<Block*> EntriesToProcess;
          std::vector<Block*> EntriesToDefer;
          for (auto& [Entry, Group] : IndependentGroups) {
            bool DeadEnd = true;
            for (auto* Curr : Group) {
              for (auto& [Target, _] : Curr->BranchesOut) {
                if (!contains(Group, Target)) {
                  DeadEnd = false;
                  break;
                }
              }
              if (!DeadEnd) {
                break;
              }
            }
            if (DeadEnd) {
              EntriesToProcess.push_back(Entry);
            } else {
              EntriesToDefer.push_back(Entry);
            }
          }
          if (!EntriesToProcess.empty()) {
            // Remove deferred entries from IndependentGroups and add them to NextEntries
            for (auto* Entry : EntriesToDefer) {
              IndependentGroups.erase(Entry);
              NextEntries->insert(Entry);
            }
            // Proceed to make a multiple with the dead-end groups
            bool Checked = false;
            for (auto* Entry : *Entries) {
              if (InitialEntries.count(Entry)) {
                Checked = true;
                break;
              }
            }
            Make(MakeMultiple(Blocks, *Entries, IndependentGroups, *NextEntries, Checked));
            // After handling, reset Entries to NextEntries for further processing
            Entries = NextEntries;
            continue;
          }
        }

        // No independent groups or no dead-end groups to process, must be loopable ==> Loop
        Make(MakeLoop(Blocks, *Entries, *NextEntries));
      }
    }
  };


  // Main

  BlockSet AllBlocks;
  for (auto* Curr : Live.Live) {
    AllBlocks.insert(Curr);
#ifdef RELOOPER_DEBUG
    PrintDebug("Adding block %d (%s)\n", Curr->Id, Curr->Code);
#endif
  }

  BlockSet Entries;
  Entries.insert(Entry);
  Root = Analyzer(this).Process(AllBlocks, Entries);
  assert(Root);
}

wasm::Expression* Relooper::Render(RelooperBuilder& Builder) {
  assert(Root);
  auto* ret = Root->Render(Builder, false);
  // we may use the same name for more than one block in HandleFollowupMultiples
  wasm::UniqueNameMapper::uniquify(ret);
  return ret;
}

#ifdef RELOOPER_DEBUG
// Debugging

void Debugging::Dump(Block* Curr, const char* prefix) {
  if (prefix)
    std::cout << prefix << ": ";
  std::cout << Curr->Id << " [code " << *Curr->Code << "] [switch? "
            << !!Curr->SwitchCondition << "]\n";
  for (auto iter2 = Curr->BranchesOut.begin(); iter2 != Curr->BranchesOut.end();
       iter2++) {
    Block* Other = iter2->first;
    Branch* Br = iter2->second;
    std::cout << "  -> " << Other->Id << ' ';
    if (Br->Condition) {
      std::cout << "[if " << *Br->Condition << "] ";
    } else if (Br->SwitchValues) {
      std::cout << "[cases ";
      for (auto x : *Br->SwitchValues) {
        std::cout << x << ' ';
      }
      std::cout << "] ";
    } else {
      std::cout << "[default] ";
    }
    if (Br->Code)
      std::cout << "[phi " << *Br->Code << "] ";
    std::cout << '\n';
  }
  std::cout << '\n';
}

void Debugging::Dump(BlockSet& Blocks, const char* prefix) {
  if (prefix)
    std::cout << prefix << ": ";
  for (auto* Curr : Blocks) {
    Dump(Curr);
  }
}

void Debugging::Dump(Shape* S, const char* prefix) {
  if (prefix)
    std::cout << prefix << ": ";
  if (!S) {
    printf(" (null)\n");
    return;
  }
  printf(" %d ", S->Id);
  if (SimpleShape* Simple = Shape::IsSimple(S)) {
    printf("<< Simple with block %d\n", Simple->Inner->Id);
  } else if (MultipleShape* Multiple = Shape::IsMultiple(S)) {
    printf("<< Multiple\n");
    for (auto& [Entry, _] : Multiple->InnerMap) {
      printf("     with entry %d\n", Entry);
    }
  } else if (Shape::IsLoop(S)) {
    printf("<< Loop\n");
  } else {
    abort();
  }
}

static void PrintDebug(const char* Format, ...) {
  printf("// ");
  va_list Args;
  va_start(Args, Format);
  vprintf(Format, Args);
  va_end(Args);
}
#endif

} // namespace CFG
