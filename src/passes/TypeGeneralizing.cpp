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

#include "analysis/cfg.h"
#include "analysis/lattice.h"
#include "analysis/lattices/inverted.h"
#include "analysis/lattices/shared.h"
#include "analysis/lattices/stack.h"
#include "analysis/lattices/tuple.h"
#include "analysis/lattices/valtype.h"
#include "analysis/lattices/vector.h"
#include "analysis/monotone-analyzer.h"
#include "ir/utils.h"
#include "pass.h"
#include "wasm-traversal.h"
#include "wasm.h"

#define TYPE_GENERALIZING_DEBUG 0

#if TYPE_GENERALIZING_DEBUG
#define DBG(statement) statement
#else
#define DBG(statement)
#endif

// Generalize the types of program locations as much as possible, both to
// eliminate unnecessarily refined types from the type section and (TODO) to
// weaken casts that cast to unnecessarily refined types. If the casts are
// weakened enough, they will be able to be removed by OptimizeInstructions.
//
// Perform a backward analysis tracking requirements on the types of program
// locations (currently just locals and stack values) to discover how much the
// type of each location can be generalized without breaking validation or
// changing program behavior.

namespace wasm {

namespace {

using namespace analysis;

// We will learn stricter and stricter requirements as we perform the analysis,
// so more specific types need to be higher up the lattice.
using TypeRequirement = Inverted<ValType>;

// Record a type requirement for each local variable. Shared the requirements
// across basic blocks.
using LocalTypeRequirements = Shared<Vector<TypeRequirement>>;

// The type requirements for each reference-typed value on the stack at a
// particular location.
using ValueStackTypeRequirements = Stack<TypeRequirement>;

// The full lattice used for the analysis.
using StateLattice =
  analysis::Tuple<LocalTypeRequirements, ValueStackTypeRequirements>;

// Equip the state lattice with helpful accessors.
struct State : StateLattice {
  using Element = StateLattice::Element;

  static constexpr int LocalsIndex = 0;
  static constexpr int StackIndex = 1;

  State(Function* func) : StateLattice{Shared{initLocals(func)}, initStack()} {}

  void push(Element& elem, Type type) const noexcept {
    stackLattice().push(stack(elem), std::move(type));
  }

  Type pop(Element& elem) const noexcept {
    return stackLattice().pop(stack(elem));
  }

  void clearStack(Element& elem) const noexcept {
    stack(elem) = stackLattice().getBottom();
  }

  const std::vector<Type>& getLocals(Element& elem) const noexcept {
    return *locals(elem);
  }

  const std::vector<Type>& getLocals() const noexcept {
    return *locals(getBottom());
  }

  Type getLocal(Element& elem, Index i) const noexcept {
    return getLocals(elem)[i];
  }

  bool updateLocal(Element& elem, Index i, Type type) const noexcept {
    return localsLattice().join(
      locals(elem),
      Vector<TypeRequirement>::SingletonElement(i, std::move(type)));
  }

private:
  static LocalTypeRequirements initLocals(Function* func) noexcept {
    return Shared{Vector{Inverted{ValType{}}, func->getNumLocals()}};
  }

  static ValueStackTypeRequirements initStack() noexcept {
    return Stack{Inverted{ValType{}}};
  }

  const LocalTypeRequirements& localsLattice() const noexcept {
    return std::get<LocalsIndex>(lattices);
  }

  const ValueStackTypeRequirements& stackLattice() const noexcept {
    return std::get<StackIndex>(lattices);
  }

  typename LocalTypeRequirements::Element&
  locals(Element& elem) const noexcept {
    return std::get<LocalsIndex>(elem);
  }

  const typename LocalTypeRequirements::Element&
  locals(const Element& elem) const noexcept {
    return std::get<LocalsIndex>(elem);
  }

  typename ValueStackTypeRequirements::Element&
  stack(Element& elem) const noexcept {
    return std::get<StackIndex>(elem);
  }

  const typename ValueStackTypeRequirements::Element&
  stack(const Element& elem) const noexcept {
    return std::get<StackIndex>(elem);
  }
};

struct TransferFn : OverriddenVisitor<TransferFn> {
  Module& wasm;
  Function* func;
  State lattice;
  typename State::Element* state = nullptr;

  // For each local, the set of blocks we may need to re-analyze when we update
  // the constraint on the local.
  std::vector<std::vector<const BasicBlock*>> localDependents;

  // The set of basic blocks that may depend on the result of the current
  // transfer.
  std::unordered_set<const BasicBlock*> currDependents;

  TransferFn(Module& wasm, Function* func, CFG& cfg)
    : wasm(wasm), func(func), lattice(func),
      localDependents(func->getNumLocals()) {
    // Initialize `localDependents`. Any block containing a `local.set l` may
    // need to be re-analyzed whenever the constraint on `l` is updated.
    auto numLocals = func->getNumLocals();
    std::vector<std::unordered_set<const BasicBlock*>> dependentSets(numLocals);
    for (const auto& bb : cfg) {
      for (const auto* inst : bb) {
        if (auto set = inst->dynCast<LocalSet>()) {
          dependentSets[set->index].insert(&bb);
        }
      }
    }
    for (size_t i = 0, n = dependentSets.size(); i < n; ++i) {
      localDependents[i] = std::vector<const BasicBlock*>(
        dependentSets[i].begin(), dependentSets[i].end());
    }
  }

  Type pop() noexcept { return lattice.pop(*state); }
  void push(Type type) noexcept { lattice.push(*state, type); }
  void clearStack() noexcept { lattice.clearStack(*state); }
  Type getLocal(Index i) noexcept { return lattice.getLocal(*state, i); }
  void updateLocal(Index i, Type type) noexcept {
    if (lattice.updateLocal(*state, i, type)) {
      currDependents.insert(localDependents[i].begin(),
                            localDependents[i].end());
    }
  }

  void dumpState() {
#if TYPE_GENERALIZING_DEBUG
    std::cerr << "locals: ";
    for (size_t i = 0, n = lattice.getLocals(*state).size(); i < n; ++i) {
      if (i != 0) {
        std::cerr << ", ";
      }
      std::cerr << getLocal(i);
    }
    std::cerr << "\nstack: ";
    auto& stack = std::get<1>(*state);
    for (size_t i = 0, n = stack.size(); i < n; ++i) {
      if (i != 0) {
        std::cerr << ", ";
      }
      std::cerr << stack[i];
    }
    std::cerr << "\n";
#endif // TYPE_GENERALIZING_DEBUG
  }

  std::unordered_set<const BasicBlock*>
  transfer(const BasicBlock& bb, typename State::Element& elem) noexcept {
    DBG(std::cerr << "transferring bb " << bb.getIndex() << "\n");
    state = &elem;

    // This is a backward analysis: The constraints on a type depend on how it
    // will be used in the future. Traverse the basic block in reverse and
    // return the predecessors as the dependent blocks.
    assert(currDependents.empty());
    const auto& preds = bb.preds();
    currDependents.insert(preds.begin(), preds.end());

    dumpState();
    if (bb.isExit()) {
      DBG(std::cerr << "visiting exit\n");
      visitFunctionExit();
      dumpState();
    }
    for (auto it = bb.rbegin(); it != bb.rend(); ++it) {
      DBG(std::cerr << "visiting " << ShallowExpression{*it} << "\n");
      visit(*it);
      dumpState();
    }
    if (bb.isEntry()) {
      DBG(std::cerr << "visiting entry\n");
      visitFunctionEntry();
      dumpState();
    }
    DBG(std::cerr << "\n");

    state = nullptr;

    // Return the blocks that may need to be re-analyzed.
    return std::move(currDependents);
  }

  void visitFunctionExit() {
    // We cannot change the types of results. Push a requirement that the stack
    // end up with the correct type.
    if (auto result = func->getResults(); result.isRef()) {
      push(result);
    }
  }

  void visitFunctionEntry() {
    // We cannot change the types of parameters, so require that they have their
    // original types.
    Index i = 0;
    Index numParams = func->getNumParams();
    Index numLocals = func->getNumLocals();
    for (; i < numParams; ++i) {
      updateLocal(i, func->getLocalType(i));
    }
    // We also cannot change the types of any other non-ref locals. For
    // reference-typed locals, we cannot generalize beyond their top type.
    for (Index i = numParams; i < numLocals; ++i) {
      auto type = func->getLocalType(i);
      // TODO: Support optimizing tuple locals.
      if (type.isRef()) {
        updateLocal(i, Type(type.getHeapType().getTop(), Nullable));
      } else {
        updateLocal(i, type);
      }
    }
  }

  void visitNop(Nop* curr) {}
  void visitBlock(Block* curr) {}
  void visitIf(If* curr) {}
  void visitLoop(Loop* curr) {}

  void visitBreak(Break* curr) {
    if (curr->condition) {
      // `br_if` pops everything but the sent value off the stack if the branch
      // is taken, but if the branch is not taken, it only pops the condition.
      // We must therefore propagate all requirements from the fallthrough
      // successor but only the requirements for the sent value, if any, from
      // the branch successor. We don't have any way to differentiate between
      // requirements received from the two successors, however, so we cannot
      // yet do anything correct here!
      //
      // Here is a sample program that would break if we tried to conservatively
      // preserve the join of the requirements from both successors:
      //
      // (module
      //  (func $func_any (param funcref anyref)
      //   (unreachable)
      //  )
      //
      //  (func $extern_any-any (param externref anyref) (result anyref)
      //   (unreachable)
      //  )
      //
      //  (func $br-if-bad
      //   (local $bang externref)
      //   (call $func_any ;; 2. Requires [func, any]
      //    (ref.null nofunc)
      //    (block $l (result anyref)
      //     (call $extern_any-any ;; 1. Requires [extern, any]
      //      (local.get $bang)
      //      (br_if $l ;; 3. After join, requires [unreachable, any]
      //       (ref.null none)
      //       (i32.const 0)
      //      )
      //     )
      //    )
      //   )
      //  )
      // )
      //
      // To fix this, we need to insert an extra basic block encompassing the
      // liminal space between where the br_if determines it should take the
      // branch and when control arrives at the branch target. This is when the
      // extra values are popped off the stack.
      WASM_UNREACHABLE("TODO");
    } else {
      // `br` pops everything but the sent value off the stack, so do not
      // require anything of values on the stack except for that sent value, if
      // it exists.
      if (curr->value && curr->value->type.isRef()) {
        auto type = pop();
        clearStack();
        push(type);
      } else {
        // No sent value. Do not require anything.
        clearStack();
      }
    }
  }

  void visitSwitch(Switch* curr) {
    // Just like `br`, do not require anything of the values on the stack except
    // for the sent value, if it exists.
    if (curr->value && curr->value->type.isRef()) {
      auto type = pop();
      clearStack();
      push(type);
    } else {
      clearStack();
    }
  }

  template<typename T> void handleCall(T* curr, Type params) {
    if (curr->type.isRef()) {
      pop();
    }
    for (auto param : params) {
      // Cannot generalize beyond param types without interprocedural analysis.
      if (param.isRef()) {
        push(param);
      }
    }
  }

  void visitCall(Call* curr) {
    handleCall(curr, wasm.getFunction(curr->target)->getParams());
  }

  void visitCallIndirect(CallIndirect* curr) {
    handleCall(curr, curr->heapType.getSignature().params);
  }

  void visitLocalGet(LocalGet* curr) {
    if (!curr->type.isRef()) {
      return;
    }
    // Propagate the requirement on the local.get output to the local.
    updateLocal(curr->index, pop());
  }

  void visitLocalSet(LocalSet* curr) {
    if (!curr->value->type.isRef()) {
      return;
    }
    if (curr->isTee()) {
      // Same as the local.get.
      updateLocal(curr->index, pop());
    }
    // Propagate the requirement on the local to our input value.
    push(getLocal(curr->index));
  }

  void visitGlobalGet(GlobalGet* curr) {
    if (curr->type.isRef()) {
      // Cannot generalize globals without interprocedural analysis.
      pop();
    }
  }

  void visitGlobalSet(GlobalSet* curr) {
    if (auto type = wasm.getGlobal(curr->name)->type; type.isRef()) {
      // Cannot generalize globals without interprocedural analysis.
      push(type);
    }
  }

  void visitLoad(Load* curr) {}
  void visitStore(Store* curr) {}
  void visitAtomicRMW(AtomicRMW* curr) {}
  void visitAtomicCmpxchg(AtomicCmpxchg* curr) {}
  void visitAtomicWait(AtomicWait* curr) {}
  void visitAtomicNotify(AtomicNotify* curr) {}
  void visitAtomicFence(AtomicFence* curr) {}
  void visitSIMDExtract(SIMDExtract* curr) {}
  void visitSIMDReplace(SIMDReplace* curr) {}
  void visitSIMDShuffle(SIMDShuffle* curr) {}
  void visitSIMDTernary(SIMDTernary* curr) {}
  void visitSIMDShift(SIMDShift* curr) {}
  void visitSIMDLoad(SIMDLoad* curr) {}
  void visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {}
  void visitMemoryInit(MemoryInit* curr) {}
  void visitDataDrop(DataDrop* curr) {}
  void visitMemoryCopy(MemoryCopy* curr) {}
  void visitMemoryFill(MemoryFill* curr) {}
  void visitConst(Const* curr) {}
  void visitUnary(Unary* curr) {}
  void visitBinary(Binary* curr) {}

  void visitSelect(Select* curr) {
    if (curr->type.isRef()) {
      // The inputs may be as general as the output.
      auto type = pop();
      push(type);
      push(type);
    }
  }

  void visitDrop(Drop* curr) {
    if (curr->type.isRef()) {
      pop();
    }
  }

  // This is handled by propagating the stack backward from the exit block.
  void visitReturn(Return* curr) {}

  void visitMemorySize(MemorySize* curr) {}
  void visitMemoryGrow(MemoryGrow* curr) {}

  void visitUnreachable(Unreachable* curr) {
    // Require nothing about values flowing into an unreachable.
    clearStack();
  }

  void visitPop(Pop* curr) { WASM_UNREACHABLE("TODO"); }

  void visitRefNull(RefNull* curr) { pop(); }

  void visitRefIsNull(RefIsNull* curr) {
    // ref.is_null works on any reference type, so do not impose any
    // constraints. We still need to push something, so push bottom.
    push(Type::none);
  }

  void visitRefFunc(RefFunc* curr) { pop(); }

  void visitRefEq(RefEq* curr) {
    // Both operands must be eqref.
    auto eqref = Type(HeapType::eq, Nullable);
    push(eqref);
    push(eqref);
  }

  void visitTableGet(TableGet* curr) {
    // Cannot generalize table types yet.
    pop();
  }

  void visitTableSet(TableSet* curr) {
    // Cannot generalize table types yet.
    push(wasm.getTable(curr->table)->type);
  }

  void visitTableSize(TableSize* curr) {}
  void visitTableGrow(TableGrow* curr) {}

  void visitTableFill(TableFill* curr) {
    // Cannot generalize table types yet.
    push(wasm.getTable(curr->table)->type);
  }

  void visitTableCopy(TableCopy* curr) {
    // Cannot generalize table types yet.
  }

  void visitTry(Try* curr) { WASM_UNREACHABLE("TODO"); }
  void visitThrow(Throw* curr) { WASM_UNREACHABLE("TODO"); }
  void visitRethrow(Rethrow* curr) { WASM_UNREACHABLE("TODO"); }
  void visitTupleMake(TupleMake* curr) { WASM_UNREACHABLE("TODO"); }
  void visitTupleExtract(TupleExtract* curr) { WASM_UNREACHABLE("TODO"); }

  void visitRefI31(RefI31* curr) { pop(); }
  void visitI31Get(I31Get* curr) { push(Type(HeapType::i31, Nullable)); }

  void visitCallRef(CallRef* curr) {
    auto sigType = curr->target->type.getHeapType();
    if (sigType.isBottom()) {
      // This will be emitted as an unreachable, so impose no requirements on
      // the arguments, but do require that the target continue to have bottom
      // type.
      clearStack();
      push(Type(HeapType::nofunc, Nullable));
      return;
    }

    auto sig = sigType.getSignature();
    auto numParams = sig.params.size();
    std::optional<Type> resultReq;
    if (sig.results.isRef()) {
      resultReq = pop();
    }

    // We have a choice here: We can either try to generalize the type of the
    // incoming function reference or the type of the incoming function
    // arguments. Because function parameters are contravariant, generalizing
    // the function type inhibits generalizing the arguments and vice versa.
    // Attempt to split the difference by generalizing the function type only as
    // much as we can without imposing stronger requirements on the arguments.
    auto targetReq = sigType;
    while (true) {
      auto candidateReq = targetReq.getDeclaredSuperType();
      if (!candidateReq) {
        // There is no more general type we can require.
        break;
      }

      auto candidateSig = candidateReq->getSignature();

      if (resultReq && *resultReq != candidateSig.results &&
          Type::isSubType(*resultReq, candidateSig.results)) {
        // Generalizing further would violate the requirement on the result
        // type.
        break;
      }

      for (size_t i = 0; i < numParams; ++i) {
        if (candidateSig.params[i] != sig.params[i]) {
          // Generalizing further would restrict how much we could generalize
          // this argument, so we choose not to generalize futher.
          // TODO: Experiment with making the opposite choice.
          goto done;
        }
      }

      // We can generalize.
      targetReq = *candidateReq;
    }
  done:

    // Push the new requirements for the parameters.
    auto targetSig = targetReq.getSignature();
    for (auto param : targetSig.params) {
      if (param.isRef()) {
        push(param);
      }
    }
    // The new requirement for the call target.
    push(Type(targetReq, Nullable));
  }

  void visitRefTest(RefTest* curr) {
    // Do not require anything of the input.
    push(Type::none);
  }

  void visitRefCast(RefCast* curr) {
    // In principle, we do not have to require anything of the input. However,
    // if our goal is to eliminate casts, then we want the input to be a subtype
    // of the output, so we don't want the input to be generalized too much.
    // Propagate the requirement on our output to our input if we can, and
    // otherwise keep the input type unchanged. Unfortunately we cannot fall
    // back to requiring nothing of our input because that would not be
    // monotonic.
    auto req = pop();
    if (req == Type::none) {
      push(Type::none);
    } else {
      push(Type::getLeastUpperBound(curr->ref->type, req));
    }
  }

  void visitBrOn(BrOn* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStructNew(StructNew* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStructGet(StructGet* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStructSet(StructSet* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayNew(ArrayNew* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayNewData(ArrayNewData* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayNewElem(ArrayNewElem* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayNewFixed(ArrayNewFixed* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayGet(ArrayGet* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArraySet(ArraySet* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayLen(ArrayLen* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayCopy(ArrayCopy* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayFill(ArrayFill* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayInitData(ArrayInitData* curr) { WASM_UNREACHABLE("TODO"); }
  void visitArrayInitElem(ArrayInitElem* curr) { WASM_UNREACHABLE("TODO"); }
  void visitRefAs(RefAs* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringNew(StringNew* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringConst(StringConst* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringMeasure(StringMeasure* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringEncode(StringEncode* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringConcat(StringConcat* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringEq(StringEq* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringAs(StringAs* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringWTF8Advance(StringWTF8Advance* curr) {
    WASM_UNREACHABLE("TODO");
  }
  void visitStringWTF16Get(StringWTF16Get* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringIterNext(StringIterNext* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringIterMove(StringIterMove* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringSliceWTF(StringSliceWTF* curr) { WASM_UNREACHABLE("TODO"); }
  void visitStringSliceIter(StringSliceIter* curr) { WASM_UNREACHABLE("TODO"); }
};

struct TypeGeneralizing : WalkerPass<PostWalker<TypeGeneralizing>> {
  std::vector<Type> localTypes;
  bool refinalize = false;

  bool isFunctionParallel() override { return true; }
  std::unique_ptr<Pass> create() override {
    return std::make_unique<TypeGeneralizing>();
  }

  void runOnFunction(Module* wasm, Function* func) override {
    // First, remove unreachable code. If we didn't, the unreachable code could
    // become invalid after this optimization because we do not materialize or
    // analyze unreachable blocks.
    PassRunner runner(getPassRunner());
    runner.add("dce");
    runner.runOnFunction(func);

    auto cfg = CFG::fromFunction(func);
    DBG(cfg.print(std::cerr));
    TransferFn txfn(*wasm, func, cfg);
    MonotoneCFGAnalyzer analyzer(txfn.lattice, txfn, cfg);
    analyzer.evaluate();

    // Optimize local types. TODO: Optimize casts as well.
    localTypes = txfn.lattice.getLocals();
    auto numParams = func->getNumParams();
    for (Index i = numParams; i < localTypes.size(); ++i) {
      func->vars[i - numParams] = localTypes[i];
    }

    // Update gets and sets accordingly.
    super::runOnFunction(wasm, func);

    if (refinalize) {
      ReFinalize().walkFunctionInModule(func, wasm);
    }
  }

  void visitLocalGet(LocalGet* curr) {
    if (localTypes[curr->index] != curr->type) {
      curr->type = localTypes[curr->index];
      refinalize = true;
    }
  }

  void visitLocalSet(LocalSet* curr) {
    if (curr->isTee() && localTypes[curr->index] != curr->type) {
      curr->type = localTypes[curr->index];
      refinalize = true;
    }
  }
};

} // anonymous namespace

Pass* createTypeGeneralizingPass() { return new TypeGeneralizing; }

} // namespace wasm
