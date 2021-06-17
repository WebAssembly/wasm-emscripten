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

#ifndef wasm_ir_effects_h
#define wasm_ir_effects_h

#include "pass.h"
#include "wasm-traversal.h"

namespace wasm {

// Analyze various possible effects.

class EffectAnalyzer {
public:
  EffectAnalyzer(const PassOptions& passOptions,
                 FeatureSet features,
                 Expression* ast = nullptr)
    : ignoreImplicitTraps(passOptions.ignoreImplicitTraps),
      debugInfo(passOptions.debugInfo), features(features) {
    if (ast) {
      walk(ast);
    }
  }

  bool ignoreImplicitTraps;
  bool debugInfo;
  FeatureSet features;

  // Walk an expression and all its children.
  void walk(Expression* ast) {
    pre();
    InternalAnalyzer(*this).walk(ast);
    post();
  }

  // Visit an expression, without any children.
  void visit(Expression* ast) {
    pre();
    InternalAnalyzer(*this).visit(ast);
    post();
  }

  // Core effect tracking

  // Definitely branches out of this expression, or does a return, etc.
  // breakTargets tracks individual targets, which we may eventually see are
  // internal, while this is set when we see something that will definitely
  // not be internal, or is otherwise special like an infinite loop (which
  // does not technically branch "out", but it does break the normal assumption
  // of control flow proceeding normally).
  bool branchesOut = false;
  bool calls = false;
  std::set<Index> localsRead;
  std::set<Index> localsWritten;
  std::set<Name> globalsRead;
  std::set<Name> globalsWritten;
  std::set<Name> tablesRead;
  std::set<Name> tablesWritten;
  std::set<Name> elementSegmentsRead;
  std::set<Name> elementSegmentsDropped;
  // TODO: add dataSegmentsRead & dataSegmentsDropped for bulk memory operations
  bool readsMemory = false;
  bool writesMemory = false;
  // TODO: Type-based alias analysis. For example, writes to Arrays never
  // interfere with reads from Structs.
  bool readsHeap = false;
  bool writesHeap = false;
  // A trap, either from an unreachable instruction, or from an implicit trap
  // that we do not ignore (see below).
  //
  // Note that we ignore trap differences, so it is ok to reorder traps with
  // each other, but it is not ok to remove them or reorder them with other
  // effects in a noticeable way.
  //
  // Note also that we ignore runtime-dependent traps, such as hitting a
  // recursion limit or running out of memory. Such traps are not part of wasm's
  // official semantics, and they can occur anywhere: *any* instruction could in
  // theory be implemented by a VM call (as will be the case when running in an
  // interpreter), and such a call could run out of stack or memory in
  // principle. To put it another way, an i32 division by zero is the program
  // doing something bad that causes a trap, but the VM running out of memory is
  // the VM doing something bad - and therefore the VM behaving in a way that is
  // not according to the wasm semantics - and we do not model such things. Note
  // that as a result we do *not* mark things like GC allocation instructions as
  // having side effects, which has the nice benefit of making it possible to
  // eliminate an allocation whose result is not captured.
  bool trap = false;
  // A trap from an instruction like a load or div/rem, which may trap on corner
  // cases. If we do not ignore implicit traps then these are counted as a trap.
  bool implicitTrap = false;
  // An atomic load/store/RMW/Cmpxchg or an operator that has a defined ordering
  // wrt atomics (e.g. memory.grow)
  bool isAtomic = false;
  bool throws = false;
  // The nested depth of try-catch_all. If an instruction that may throw is
  // inside an inner try-catch_all, we don't mark it as 'throws', because it
  // will be caught by an inner catch_all. We only count 'try's with a
  // 'catch_all' because instructions within a 'try' without a 'catch_all' can
  // still throw outside of the try.
  size_t tryDepth = 0;
  // The nested depth of catch. This is necessary to track danglng pops.
  size_t catchDepth = 0;
  // If this expression contains 'pop's that are not enclosed in 'catch' body.
  // For example, (drop (pop i32)) should set this to true.
  bool danglingPop = false;

  // Helper functions to check for various effect types

  bool accessesLocal() const {
    return localsRead.size() + localsWritten.size() > 0;
  }
  bool accessesGlobal() const {
    return globalsRead.size() + globalsWritten.size() > 0;
  }
  bool accessesTable() const {
    return calls || tablesRead.size() || tablesWritten.size();
  }
  bool accessesMemory() const { return calls || readsMemory || writesMemory; }
  bool accessesHeap() const { return calls || readsHeap || writesHeap; }
  // Check whether this may transfer control flow to somewhere outside of this
  // expression (aside from just flowing out normally). That includes a break
  // or a throw (if the throw is not known to be caught inside this expression;
  // note that if the throw is not caught in this expression then it might be
  // caught in this function but outside of this expression, or it might not be
  // caught in the function at all, which would mean control flow cannot be
  // transferred inside the function, but this expression does not know that).
  bool transfersControlFlow() const {
    return branchesOut || throws || hasExternalBreakTargets();
  }

  // Changes something in globally-stored state.
  bool writesGlobalState() const {
    return globalsWritten.size() || tablesWritten.size() ||
           elementSegmentsDropped.size() || writesMemory || writesHeap ||
           isAtomic || calls;
  }
  bool readsGlobalState() const {
    return globalsRead.size() || tablesRead.size() ||
           elementSegmentsRead.size() || readsMemory || readsHeap || isAtomic ||
           calls;
  }

  bool hasSideEffects() const {
    return localsWritten.size() > 0 || danglingPop || writesGlobalState() ||
           trap || throws || transfersControlFlow();
  }
  bool hasAnything() const {
    return hasSideEffects() || accessesLocal() || accessesTable() ||
           elementSegmentsDropped.size() || readsMemory || accessesGlobal();
  }

  // check if we break to anything external from ourselves
  bool hasExternalBreakTargets() const { return !breakTargets.empty(); }

  // checks if these effects would invalidate another set (e.g., if we write, we
  // invalidate someone that reads, they can't be moved past us)
  bool invalidates(const EffectAnalyzer& other) {
    if ((transfersControlFlow() && other.hasSideEffects()) ||
        (other.transfersControlFlow() && hasSideEffects()) ||
        ((writesMemory || calls) && other.accessesMemory()) ||
        ((other.writesMemory || other.calls) && accessesMemory()) ||
        ((writesHeap || calls) && other.accessesHeap()) ||
        ((other.writesHeap || other.calls) && accessesHeap()) ||
        (danglingPop || other.danglingPop)) {
      return true;
    }
    // All atomics are sequentially consistent for now, and ordered wrt other
    // memory references.
    if ((isAtomic && other.accessesMemory()) ||
        (other.isAtomic && accessesMemory())) {
      return true;
    }
    for (auto local : localsWritten) {
      if (other.localsRead.count(local) || other.localsWritten.count(local)) {
        return true;
      }
    }
    for (auto local : localsRead) {
      if (other.localsWritten.count(local)) {
        return true;
      }
    }
    if ((other.calls && accessesGlobal()) ||
        (calls && other.accessesGlobal())) {
      return true;
    }
    for (auto global : globalsWritten) {
      if (other.globalsRead.count(global) ||
          other.globalsWritten.count(global)) {
        return true;
      }
    }
    for (auto table : tablesWritten) {
      if (other.tablesRead.count(table) || other.tablesWritten.count(table)) {
        return true;
      }
    }
    for (auto elem : elementSegmentsDropped) {
      if (other.elementSegmentsRead.count(elem)) {
        return true;
      }
    }
    for (auto global : globalsRead) {
      if (other.globalsWritten.count(global)) {
        return true;
      }
    }
    for (auto table : tablesRead) {
      if (other.tablesWritten.count(table)) {
        return true;
      }
    }
    for (auto elem : elementSegmentsRead) {
      if (other.elementSegmentsDropped.count(elem)) {
        return true;
      }
    }
    // We are ok to reorder implicit traps, but not conditionalize them.
    if ((trap && other.transfersControlFlow()) ||
        (other.trap && transfersControlFlow())) {
      return true;
    }
    // Note that the above includes disallowing the reordering of a trap with an
    // exception (as an exception can transfer control flow inside the current
    // function, so transfersControlFlow would be true) - while we allow the
    // reordering of traps with each other, we do not reorder exceptions with
    // anything.
    assert(!((trap && other.throws) || (throws && other.trap)));
    // We can't reorder an implicit trap in a way that could alter what global
    // state is modified.
    if ((trap && other.writesGlobalState()) ||
        (other.trap && writesGlobalState())) {
      return true;
    }
    return false;
  }

  void mergeIn(EffectAnalyzer& other) {
    branchesOut = branchesOut || other.branchesOut;
    calls = calls || other.calls;
    readsMemory = readsMemory || other.readsMemory;
    writesMemory = writesMemory || other.writesMemory;
    readsHeap = readsHeap || other.readsHeap;
    writesHeap = writesHeap || other.writesHeap;
    trap = trap || other.trap;
    implicitTrap = implicitTrap || other.implicitTrap;
    isAtomic = isAtomic || other.isAtomic;
    throws = throws || other.throws;
    danglingPop = danglingPop || other.danglingPop;
    for (auto i : other.localsRead) {
      localsRead.insert(i);
    }
    for (auto i : other.localsWritten) {
      localsWritten.insert(i);
    }
    for (auto i : other.globalsRead) {
      globalsRead.insert(i);
    }
    for (auto i : other.globalsWritten) {
      globalsWritten.insert(i);
    }
    for (auto i : other.tablesRead) {
      tablesRead.insert(i);
    }
    for (auto i : other.tablesWritten) {
      tablesWritten.insert(i);
    }
    for (auto i : other.elementSegmentsRead) {
      elementSegmentsRead.insert(i);
    }
    for (auto i : other.elementSegmentsDropped) {
      elementSegmentsDropped.insert(i);
    }
    for (auto i : other.breakTargets) {
      breakTargets.insert(i);
    }
  }

  // the checks above happen after the node's children were processed, in the
  // order of execution we must also check for control flow that happens before
  // the children, i.e., loops
  bool checkPre(Expression* curr) {
    if (curr->is<Loop>()) {
      branchesOut = true;
      return true;
    }
    return false;
  }

  bool checkPost(Expression* curr) {
    visit(curr);
    if (curr->is<Loop>()) {
      branchesOut = true;
    }
    return hasAnything();
  }

  std::set<Name> breakTargets;

private:
  struct InternalAnalyzer
    : public PostWalker<InternalAnalyzer, OverriddenVisitor<InternalAnalyzer>> {

    EffectAnalyzer& parent;

    InternalAnalyzer(EffectAnalyzer& parent) : parent(parent) {}

    static void scan(InternalAnalyzer* self, Expression** currp) {
      Expression* curr = *currp;
      // We need to decrement try depth before catch starts, so handle it
      // separately
      if (curr->is<Try>()) {
        self->pushTask(doVisitTry, currp);
        self->pushTask(doEndCatch, currp);
        auto& catchBodies = curr->cast<Try>()->catchBodies;
        for (int i = int(catchBodies.size()) - 1; i >= 0; i--) {
          self->pushTask(scan, &catchBodies[i]);
        }
        self->pushTask(doStartCatch, currp);
        self->pushTask(scan, &curr->cast<Try>()->body);
        self->pushTask(doStartTry, currp);
        return;
      }
      PostWalker<InternalAnalyzer, OverriddenVisitor<InternalAnalyzer>>::scan(
        self, currp);
    }

    static void doStartTry(InternalAnalyzer* self, Expression** currp) {
      Try* curr = (*currp)->cast<Try>();
      // We only count 'try's with a 'catch_all' because instructions within a
      // 'try' without a 'catch_all' can still throw outside of the try.
      if (curr->hasCatchAll()) {
        self->parent.tryDepth++;
      }
    }

    static void doStartCatch(InternalAnalyzer* self, Expression** currp) {
      Try* curr = (*currp)->cast<Try>();
      // We only count 'try's with a 'catch_all' because instructions within a
      // 'try' without a 'catch_all' can still throw outside of the try.
      if (curr->hasCatchAll()) {
        assert(self->parent.tryDepth > 0 && "try depth cannot be negative");
        self->parent.tryDepth--;
      }
      self->parent.catchDepth++;
    }

    static void doEndCatch(InternalAnalyzer* self, Expression** currp) {
      assert(self->parent.catchDepth > 0 && "catch depth cannot be negative");
      self->parent.catchDepth--;
    }

    void visitBlock(Block* curr) {
      if (curr->name.is()) {
        parent.breakTargets.erase(curr->name); // these were internal breaks
      }
    }
    void visitIf(If* curr) {}
    void visitLoop(Loop* curr) {
      if (curr->name.is()) {
        parent.breakTargets.erase(curr->name); // these were internal breaks
      }
      // if the loop is unreachable, then there is branching control flow:
      //  (1) if the body is unreachable because of a (return), uncaught (br)
      //      etc., then we already noted branching, so it is ok to mark it
      //      again (if we have *caught* (br)s, then they did not lead to the
      //      loop body being unreachable). (same logic applies to blocks)
      //  (2) if the loop is unreachable because it only has branches up to the
      //      loop top, but no way to get out, then it is an infinite loop, and
      //      we consider that a branching side effect (note how the same logic
      //      does not apply to blocks).
      if (curr->type == Type::unreachable) {
        parent.branchesOut = true;
      }
    }
    void visitBreak(Break* curr) { parent.breakTargets.insert(curr->name); }
    void visitSwitch(Switch* curr) {
      for (auto name : curr->targets) {
        parent.breakTargets.insert(name);
      }
      parent.breakTargets.insert(curr->default_);
    }

    void visitCall(Call* curr) {
      parent.calls = true;
      // When EH is enabled, any call can throw.
      if (parent.features.hasExceptionHandling() && parent.tryDepth == 0) {
        parent.throws = true;
      }
      if (curr->isReturn) {
        parent.branchesOut = true;
      }
      if (parent.debugInfo) {
        // debugInfo call imports must be preserved very strongly, do not
        // move code around them
        // FIXME: we could check if the call is to an import
        parent.branchesOut = true;
      }
    }
    void visitCallIndirect(CallIndirect* curr) {
      parent.calls = true;
      parent.tablesRead.insert(curr->table);
      if (parent.features.hasExceptionHandling() && parent.tryDepth == 0) {
        parent.throws = true;
      }
      if (curr->isReturn) {
        parent.branchesOut = true;
      }
    }
    void visitLocalGet(LocalGet* curr) {
      parent.localsRead.insert(curr->index);
    }
    void visitLocalSet(LocalSet* curr) {
      parent.localsWritten.insert(curr->index);
    }
    void visitGlobalGet(GlobalGet* curr) {
      parent.globalsRead.insert(curr->name);
    }
    void visitGlobalSet(GlobalSet* curr) {
      parent.globalsWritten.insert(curr->name);
    }
    void visitLoad(Load* curr) {
      parent.readsMemory = true;
      parent.isAtomic |= curr->isAtomic;
      parent.implicitTrap = true;
    }
    void visitStore(Store* curr) {
      parent.writesMemory = true;
      parent.isAtomic |= curr->isAtomic;
      parent.implicitTrap = true;
    }
    void visitAtomicRMW(AtomicRMW* curr) {
      parent.readsMemory = true;
      parent.writesMemory = true;
      parent.isAtomic = true;
      parent.implicitTrap = true;
    }
    void visitAtomicCmpxchg(AtomicCmpxchg* curr) {
      parent.readsMemory = true;
      parent.writesMemory = true;
      parent.isAtomic = true;
      parent.implicitTrap = true;
    }
    void visitAtomicWait(AtomicWait* curr) {
      parent.readsMemory = true;
      // AtomicWait doesn't strictly write memory, but it does modify the
      // waiters list associated with the specified address, which we can think
      // of as a write.
      parent.writesMemory = true;
      parent.isAtomic = true;
      parent.implicitTrap = true;
    }
    void visitAtomicNotify(AtomicNotify* curr) {
      // AtomicNotify doesn't strictly write memory, but it does modify the
      // waiters list associated with the specified address, which we can think
      // of as a write.
      parent.readsMemory = true;
      parent.writesMemory = true;
      parent.isAtomic = true;
      parent.implicitTrap = true;
    }
    void visitAtomicFence(AtomicFence* curr) {
      // AtomicFence should not be reordered with any memory operations, so we
      // set these to true.
      parent.readsMemory = true;
      parent.writesMemory = true;
      parent.isAtomic = true;
    }
    void visitSIMDExtract(SIMDExtract* curr) {}
    void visitSIMDReplace(SIMDReplace* curr) {}
    void visitSIMDShuffle(SIMDShuffle* curr) {}
    void visitSIMDTernary(SIMDTernary* curr) {}
    void visitSIMDShift(SIMDShift* curr) {}
    void visitSIMDLoad(SIMDLoad* curr) {
      parent.readsMemory = true;
      parent.implicitTrap = true;
    }
    void visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
      if (curr->isLoad()) {
        parent.readsMemory = true;
      } else {
        parent.writesMemory = true;
      }
      parent.implicitTrap = true;
    }
    void visitMemoryInit(MemoryInit* curr) {
      parent.writesMemory = true;
      parent.implicitTrap = true;
    }
    void visitDataDrop(DataDrop* curr) {
      // data.drop does not actually write memory, but it does alter the size of
      // a segment, which can be noticeable later by memory.init, so we need to
      // mark it as having a global side effect of some kind.
      parent.writesMemory = true;
      parent.implicitTrap = true;
    }
    void visitMemoryCopy(MemoryCopy* curr) {
      parent.readsMemory = true;
      parent.writesMemory = true;
      parent.implicitTrap = true;
    }
    void visitMemoryFill(MemoryFill* curr) {
      parent.writesMemory = true;
      parent.implicitTrap = true;
    }
    void visitConst(Const* curr) {}
    void visitUnary(Unary* curr) {
      switch (curr->op) {
        case TruncSFloat32ToInt32:
        case TruncSFloat32ToInt64:
        case TruncUFloat32ToInt32:
        case TruncUFloat32ToInt64:
        case TruncSFloat64ToInt32:
        case TruncSFloat64ToInt64:
        case TruncUFloat64ToInt32:
        case TruncUFloat64ToInt64: {
          parent.implicitTrap = true;
          break;
        }
        default: {
        }
      }
    }
    void visitBinary(Binary* curr) {
      switch (curr->op) {
        case DivSInt32:
        case DivUInt32:
        case RemSInt32:
        case RemUInt32:
        case DivSInt64:
        case DivUInt64:
        case RemSInt64:
        case RemUInt64: {
          // div and rem may contain implicit trap only if RHS is
          // non-constant or constant which equal zero or -1 for
          // signed divisions. Reminder traps only with zero
          // divider.
          if (auto* c = curr->right->dynCast<Const>()) {
            if (c->value.isZero()) {
              parent.implicitTrap = true;
            } else if ((curr->op == DivSInt32 || curr->op == DivSInt64) &&
                       c->value.getInteger() == -1LL) {
              parent.implicitTrap = true;
            }
          } else {
            parent.implicitTrap = true;
          }
          break;
        }
        default: {
        }
      }
    }
    void visitSelect(Select* curr) {}
    void visitDrop(Drop* curr) {}
    void visitReturn(Return* curr) { parent.branchesOut = true; }
    void visitMemorySize(MemorySize* curr) {
      // memory.size accesses the size of the memory, and thus can be modeled as
      // reading memory
      parent.readsMemory = true;
      // Atomics are sequentially consistent with memory.size.
      parent.isAtomic = true;
    }
    void visitMemoryGrow(MemoryGrow* curr) {
      parent.calls = true;
      // memory.grow technically does a read-modify-write operation on the
      // memory size in the successful case, modifying the set of valid
      // addresses, and just a read operation in the failure case
      parent.readsMemory = true;
      parent.writesMemory = true;
      // Atomics are also sequentially consistent with memory.grow.
      parent.isAtomic = true;
    }
    void visitTableGet(TableGet* curr) {
      parent.tablesRead.insert(curr->table);
      parent.implicitTrap = true;
    }
    void visitTableSet(TableSet* curr) {
      parent.tablesWritten.insert(curr->table);
      parent.implicitTrap = true;
    }
    void visitTableSize(TableSize* curr) {
      parent.tablesRead.insert(curr->table);
    }
    void visitTableGrow(TableGrow* curr) {
      parent.tablesRead.insert(curr->table);
      parent.tablesWritten.insert(curr->table);
    }
    void visitTableFill(TableFill* curr) {
      parent.tablesWritten.insert(curr->table);
      parent.implicitTrap = true;
    }
    void visitTableCopy(TableCopy* curr) {
      parent.tablesRead.insert(curr->srcTable);
      parent.tablesWritten.insert(curr->destTable);
      parent.implicitTrap = true;
    }
    void visitTableInit(TableInit* curr) {
      parent.tablesWritten.insert(curr->table);
      parent.elementSegmentsRead.insert(curr->segment);
      parent.implicitTrap = true;
    }
    void visitElemDrop(ElemDrop* curr) {
      parent.elementSegmentsDropped.insert(curr->segment);
      parent.implicitTrap = true;
    }
    void visitRefNull(RefNull* curr) {}
    void visitRefIs(RefIs* curr) {}
    void visitRefFunc(RefFunc* curr) {}
    void visitRefEq(RefEq* curr) {}
    void visitTry(Try* curr) {}
    void visitThrow(Throw* curr) {
      if (parent.tryDepth == 0) {
        parent.throws = true;
      }
    }
    void visitRethrow(Rethrow* curr) {
      if (parent.tryDepth == 0) {
        parent.throws = true;
      }
      // traps when the arg is null
      parent.implicitTrap = true;
    }
    void visitNop(Nop* curr) {}
    void visitUnreachable(Unreachable* curr) { parent.trap = true; }
    void visitPop(Pop* curr) {
      if (parent.catchDepth == 0) {
        parent.danglingPop = true;
      }
    }
    void visitTupleMake(TupleMake* curr) {}
    void visitTupleExtract(TupleExtract* curr) {}
    void visitI31New(I31New* curr) {}
    void visitI31Get(I31Get* curr) {}
    void visitCallRef(CallRef* curr) {
      parent.calls = true;
      if (parent.features.hasExceptionHandling() && parent.tryDepth == 0) {
        parent.throws = true;
      }
      if (curr->isReturn) {
        parent.branchesOut = true;
      }
      // traps when the arg is null
      parent.implicitTrap = true;
    }
    void visitRefTest(RefTest* curr) {}
    void visitRefCast(RefCast* curr) {
      // Traps if the ref is not null and it has an invalid rtt.
      parent.implicitTrap = true;
    }
    void visitBrOn(BrOn* curr) { parent.breakTargets.insert(curr->name); }
    void visitRttCanon(RttCanon* curr) {}
    void visitRttSub(RttSub* curr) {}
    void visitStructNew(StructNew* curr) {}
    void visitStructGet(StructGet* curr) {
      parent.readsHeap = true;
      // traps when the arg is null
      if (curr->ref->type.isNullable()) {
        parent.implicitTrap = true;
      }
    }
    void visitStructSet(StructSet* curr) {
      parent.writesHeap = true;
      // traps when the arg is null
      if (curr->ref->type.isNullable()) {
        parent.implicitTrap = true;
      }
    }
    void visitArrayNew(ArrayNew* curr) {}
    void visitArrayGet(ArrayGet* curr) {
      parent.readsHeap = true;
      // traps when the arg is null or the index out of bounds
      parent.implicitTrap = true;
    }
    void visitArraySet(ArraySet* curr) {
      parent.writesHeap = true;
      // traps when the arg is null or the index out of bounds
      parent.implicitTrap = true;
    }
    void visitArrayLen(ArrayLen* curr) {
      // traps when the arg is null
      if (curr->ref->type.isNullable()) {
        parent.implicitTrap = true;
      }
    }
    void visitArrayCopy(ArrayCopy* curr) {
      // traps when a ref is null, or when out of bounds.
      parent.implicitTrap = true;
    }
    void visitRefAs(RefAs* curr) {
      // traps when the arg is not valid
      if (curr->value->type.isNullable()) {
        parent.implicitTrap = true;
      }
    }
  };

public:
  // Helpers

  static bool canReorder(const PassOptions& passOptions,
                         FeatureSet features,
                         Expression* a,
                         Expression* b) {
    EffectAnalyzer aEffects(passOptions, features, a);
    EffectAnalyzer bEffects(passOptions, features, b);
    return !aEffects.invalidates(bEffects);
  }

  // C-API

  enum SideEffects : uint32_t {
    None = 0,
    Branches = 1 << 0,
    Calls = 1 << 1,
    ReadsLocal = 1 << 2,
    WritesLocal = 1 << 3,
    ReadsGlobal = 1 << 4,
    WritesGlobal = 1 << 5,
    ReadsMemory = 1 << 6,
    WritesMemory = 1 << 7,
    ImplicitTrap = 1 << 8,
    IsAtomic = 1 << 9,
    Throws = 1 << 10,
    DanglingPop = 1 << 11,
    ReadsTable = 1 << 12,
    WritesTable = 1 << 13,
    ReadsElementSegment = 1 << 14,
    DropsElementSegment = 1 << 15,
    // TODO: Add ReadsDataSegment and DropsDataSegment
    Any = (1 << 16) - 1
  };
  uint32_t getSideEffects() const {
    uint32_t effects = 0;
    if (branchesOut || hasExternalBreakTargets()) {
      effects |= SideEffects::Branches;
    }
    if (calls) {
      effects |= SideEffects::Calls;
    }
    if (localsRead.size() > 0) {
      effects |= SideEffects::ReadsLocal;
    }
    if (localsWritten.size() > 0) {
      effects |= SideEffects::WritesLocal;
    }
    if (globalsRead.size() > 0) {
      effects |= SideEffects::ReadsGlobal;
    }
    if (globalsWritten.size() > 0) {
      effects |= SideEffects::WritesGlobal;
    }
    if (tablesRead.size() > 0) {
      effects |= SideEffects::ReadsTable;
    }
    if (tablesWritten.size() > 0) {
      effects |= SideEffects::WritesTable;
    }
    if (elementSegmentsRead.size() > 0) {
      effects |= SideEffects::ReadsElementSegment;
    }
    if (elementSegmentsDropped.size() > 0) {
      effects |= SideEffects::DropsElementSegment;
    }
    if (readsMemory) {
      effects |= SideEffects::ReadsMemory;
    }
    if (writesMemory) {
      effects |= SideEffects::WritesMemory;
    }
    if (implicitTrap) {
      effects |= SideEffects::ImplicitTrap;
    }
    if (isAtomic) {
      effects |= SideEffects::IsAtomic;
    }
    if (throws) {
      effects |= SideEffects::Throws;
    }
    if (danglingPop) {
      effects |= SideEffects::DanglingPop;
    }
    return effects;
  }

  void ignoreBranches() {
    branchesOut = false;
    breakTargets.clear();
  }

private:
  void pre() { breakTargets.clear(); }

  void post() {
    assert(tryDepth == 0);

    if (ignoreImplicitTraps) {
      implicitTrap = false;
    } else if (implicitTrap) {
      trap = true;
    }
  }
};

// Calculate effects only on the node itself (shallowly), and not on
// children.
class ShallowEffectAnalyzer : public EffectAnalyzer {
public:
  ShallowEffectAnalyzer(const PassOptions& passOptions,
                        FeatureSet features,
                        Expression* ast = nullptr)
    : EffectAnalyzer(passOptions, features) {
    if (ast) {
      visit(ast);
    }
  }
};

} // namespace wasm

#endif // wasm_ir_effects_h
