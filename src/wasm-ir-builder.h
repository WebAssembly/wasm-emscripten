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

#ifndef wasm_wasm_ir_builder_h
#define wasm_wasm_ir_builder_h

#include <vector>

#include "ir/names.h"
#include "support/result.h"
#include "wasm-builder.h"
#include "wasm-traversal.h"
#include "wasm-type.h"
#include "wasm.h"

namespace wasm {

// A utility for constructing valid Binaryen IR from arbitrary valid sequences
// of WebAssembly instructions. The user is responsible for providing Expression
// nodes with all of their non-child fields already filled out, and IRBuilder is
// responsible for setting child fields and finalizing nodes.
//
// To use, call CHECK_ERR(visit(...)) or CHECK_ERR(makeXYZ(...)) on each
// expression in the sequence, then call build().
//
// Unlike `Builder`, `IRBuilder` requires referenced module-level items (e.g.
// globals, tables, functions, etc.) to already exist in the module.
class IRBuilder : public UnifiedExpressionVisitor<IRBuilder, Result<>> {
public:
  IRBuilder(Module& wasm, Function* func = nullptr)
    : wasm(wasm), func(func), builder(wasm) {}

  // Get the valid Binaryen IR expression representing the sequence of visited
  // instructions. The IRBuilder is reset and can be used with a fresh sequence
  // of instructions after this is called.
  [[nodiscard]] Result<Expression*> build();

  // Call visit() on an existing Expression with its non-child fields
  // initialized to initialize the child fields and refinalize it.
  [[nodiscard]] Result<> visit(Expression*);

  // Like visit, but pushes the expression onto the stack as-is without popping
  // any children or refinalization.
  void push(Expression*);

  // Set the debug location to be attached to the next visited, created, or
  // pushed instruction.
  void setDebugLocation(const Function::DebugLocation&);

  // Handle the boundaries of control flow structures. Users may choose to use
  // the corresponding `makeXYZ` function below instead of `visitXYZStart`, but
  // either way must call `visitEnd` and friends at the appropriate times.
  [[nodiscard]] Result<> visitFunctionStart(Function* func);
  [[nodiscard]] Result<> visitBlockStart(Block* block);
  [[nodiscard]] Result<> visitIfStart(If* iff, Name label = {});
  [[nodiscard]] Result<> visitElse();
  [[nodiscard]] Result<> visitLoopStart(Loop* iff);
  [[nodiscard]] Result<> visitTryStart(Try* tryy, Name label = {});
  [[nodiscard]] Result<> visitCatch(Name tag);
  [[nodiscard]] Result<> visitCatchAll();
  [[nodiscard]] Result<> visitDelegate(Index label);
  [[nodiscard]] Result<> visitTryTableStart(TryTable* trytable,
                                            Name label = {});
  [[nodiscard]] Result<> visitEnd();

  // Binaryen IR uses names to refer to branch targets, but in general there may
  // be branches to constructs that do not yet have names, so in IRBuilder we
  // use indices to refer to branch targets instead, just as the binary format
  // does. This function converts a branch target name to the correct index.
  //
  // Labels in delegates need special handling because the indexing needs to be
  // relative to the try's enclosing scope rather than the try itself.
  [[nodiscard]] Result<Index> getLabelIndex(Name label,
                                            bool inDelegate = false);

  // Instead of calling visit, call makeXYZ to have the IRBuilder allocate the
  // nodes. This is generally safer than calling `visit` because the function
  // signatures ensure that there are no missing fields.
  [[nodiscard]] Result<> makeNop();
  [[nodiscard]] Result<> makeBlock(Name label, Type type);
  [[nodiscard]] Result<> makeIf(Name label, Type type);
  [[nodiscard]] Result<> makeLoop(Name label, Type type);
  [[nodiscard]] Result<> makeBreak(Index label, bool isConditional);
  [[nodiscard]] Result<> makeSwitch(const std::vector<Index>& labels,
                                    Index defaultLabel);
  // Unlike Builder::makeCall, this assumes the function already exists.
  [[nodiscard]] Result<> makeCall(Name func, bool isReturn);
  [[nodiscard]] Result<>
  makeCallIndirect(Name table, HeapType type, bool isReturn);
  [[nodiscard]] Result<> makeLocalGet(Index local);
  [[nodiscard]] Result<> makeLocalSet(Index local);
  [[nodiscard]] Result<> makeLocalTee(Index local);
  [[nodiscard]] Result<> makeGlobalGet(Name global);
  [[nodiscard]] Result<> makeGlobalSet(Name global);
  [[nodiscard]] Result<> makeLoad(unsigned bytes,
                                  bool signed_,
                                  Address offset,
                                  unsigned align,
                                  Type type,
                                  Name mem);
  [[nodiscard]] Result<> makeStore(
    unsigned bytes, Address offset, unsigned align, Type type, Name mem);
  [[nodiscard]] Result<>
  makeAtomicLoad(unsigned bytes, Address offset, Type type, Name mem);
  [[nodiscard]] Result<>
  makeAtomicStore(unsigned bytes, Address offset, Type type, Name mem);
  [[nodiscard]] Result<> makeAtomicRMW(
    AtomicRMWOp op, unsigned bytes, Address offset, Type type, Name mem);
  [[nodiscard]] Result<>
  makeAtomicCmpxchg(unsigned bytes, Address offset, Type type, Name mem);
  [[nodiscard]] Result<> makeAtomicWait(Type type, Address offset, Name mem);
  [[nodiscard]] Result<> makeAtomicNotify(Address offset, Name mem);
  [[nodiscard]] Result<> makeAtomicFence();
  [[nodiscard]] Result<> makeSIMDExtract(SIMDExtractOp op, uint8_t lane);
  [[nodiscard]] Result<> makeSIMDReplace(SIMDReplaceOp op, uint8_t lane);
  [[nodiscard]] Result<> makeSIMDShuffle(const std::array<uint8_t, 16>& lanes);
  [[nodiscard]] Result<> makeSIMDTernary(SIMDTernaryOp op);
  [[nodiscard]] Result<> makeSIMDShift(SIMDShiftOp op);
  [[nodiscard]] Result<>
  makeSIMDLoad(SIMDLoadOp op, Address offset, unsigned align, Name mem);
  [[nodiscard]] Result<> makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp op,
                                               Address offset,
                                               unsigned align,
                                               uint8_t lane,
                                               Name mem);
  [[nodiscard]] Result<> makeMemoryInit(Name data, Name mem);
  [[nodiscard]] Result<> makeDataDrop(Name data);
  [[nodiscard]] Result<> makeMemoryCopy(Name destMem, Name srcMem);
  [[nodiscard]] Result<> makeMemoryFill(Name mem);
  [[nodiscard]] Result<> makeConst(Literal val);
  [[nodiscard]] Result<> makeUnary(UnaryOp op);
  [[nodiscard]] Result<> makeBinary(BinaryOp op);
  [[nodiscard]] Result<> makeSelect(std::optional<Type> type = std::nullopt);
  [[nodiscard]] Result<> makeDrop();
  [[nodiscard]] Result<> makeReturn();
  [[nodiscard]] Result<> makeMemorySize(Name mem);
  [[nodiscard]] Result<> makeMemoryGrow(Name mem);
  [[nodiscard]] Result<> makeUnreachable();
  [[nodiscard]] Result<> makePop(Type type);
  [[nodiscard]] Result<> makeRefNull(HeapType type);
  [[nodiscard]] Result<> makeRefIsNull();
  [[nodiscard]] Result<> makeRefFunc(Name func);
  [[nodiscard]] Result<> makeRefEq();
  [[nodiscard]] Result<> makeTableGet(Name table);
  [[nodiscard]] Result<> makeTableSet(Name table);
  [[nodiscard]] Result<> makeTableSize(Name table);
  [[nodiscard]] Result<> makeTableGrow(Name table);
  [[nodiscard]] Result<> makeTableFill(Name table);
  [[nodiscard]] Result<> makeTableCopy(Name destTable, Name srcTable);
  [[nodiscard]] Result<> makeTry(Name label, Type type);
  [[nodiscard]] Result<> makeTryTable(Name label,
                                      Type type,
                                      const std::vector<Name>& tags,
                                      const std::vector<Index>& labels,
                                      const std::vector<bool>& isRefs);
  [[nodiscard]] Result<> makeThrow(Name tag);
  [[nodiscard]] Result<> makeRethrow(Index label);
  [[nodiscard]] Result<> makeThrowRef();
  [[nodiscard]] Result<> makeTupleMake(uint32_t arity);
  [[nodiscard]] Result<> makeTupleExtract(uint32_t arity, uint32_t index);
  [[nodiscard]] Result<> makeTupleDrop(uint32_t arity);
  [[nodiscard]] Result<> makeRefI31();
  [[nodiscard]] Result<> makeI31Get(bool signed_);
  [[nodiscard]] Result<> makeCallRef(HeapType type, bool isReturn);
  [[nodiscard]] Result<> makeRefTest(Type type);
  [[nodiscard]] Result<> makeRefCast(Type type);
  [[nodiscard]] Result<>
  makeBrOn(Index label, BrOnOp op, Type in = Type::none, Type out = Type::none);
  [[nodiscard]] Result<> makeStructNew(HeapType type);
  [[nodiscard]] Result<> makeStructNewDefault(HeapType type);
  [[nodiscard]] Result<>
  makeStructGet(HeapType type, Index field, bool signed_);
  [[nodiscard]] Result<> makeStructSet(HeapType type, Index field);
  [[nodiscard]] Result<> makeArrayNew(HeapType type);
  [[nodiscard]] Result<> makeArrayNewDefault(HeapType type);
  [[nodiscard]] Result<> makeArrayNewData(HeapType type, Name data);
  [[nodiscard]] Result<> makeArrayNewElem(HeapType type, Name elem);
  [[nodiscard]] Result<> makeArrayNewFixed(HeapType type, uint32_t arity);
  [[nodiscard]] Result<> makeArrayGet(HeapType type, bool signed_);
  [[nodiscard]] Result<> makeArraySet(HeapType type);
  [[nodiscard]] Result<> makeArrayLen();
  [[nodiscard]] Result<> makeArrayCopy(HeapType destType, HeapType srcType);
  [[nodiscard]] Result<> makeArrayFill(HeapType type);
  [[nodiscard]] Result<> makeArrayInitData(HeapType type, Name data);
  [[nodiscard]] Result<> makeArrayInitElem(HeapType type, Name elem);
  [[nodiscard]] Result<> makeRefAs(RefAsOp op);
  [[nodiscard]] Result<> makeStringNew(StringNewOp op, bool try_, Name mem);
  [[nodiscard]] Result<> makeStringConst(Name string);
  [[nodiscard]] Result<> makeStringMeasure(StringMeasureOp op);
  [[nodiscard]] Result<> makeStringEncode(StringEncodeOp op, Name mem);
  [[nodiscard]] Result<> makeStringConcat();
  [[nodiscard]] Result<> makeStringEq(StringEqOp op);
  [[nodiscard]] Result<> makeStringAs(StringAsOp op);
  [[nodiscard]] Result<> makeStringWTF8Advance();
  [[nodiscard]] Result<> makeStringWTF16Get();
  [[nodiscard]] Result<> makeStringIterNext();
  [[nodiscard]] Result<> makeStringIterMove(StringIterMoveOp op);
  [[nodiscard]] Result<> makeStringSliceWTF(StringSliceWTFOp op);
  [[nodiscard]] Result<> makeStringSliceIter();
  [[nodiscard]] Result<> makeContBind(HeapType contTypeBefore,
                                      HeapType contTypeAfter);
  [[nodiscard]] Result<> makeContNew(HeapType ct);
  [[nodiscard]] Result<> makeResume(HeapType ct,
                                    const std::vector<Name>& tags,
                                    const std::vector<Index>& labels);
  [[nodiscard]] Result<> makeSuspend(Name tag);

  // Private functions that must be public for technical reasons.
  [[nodiscard]] Result<> visitExpression(Expression*);
  [[nodiscard]] Result<>
  visitDrop(Drop*, std::optional<uint32_t> arity = std::nullopt);
  [[nodiscard]] Result<> visitIf(If*);
  [[nodiscard]] Result<> visitReturn(Return*);
  [[nodiscard]] Result<> visitStructNew(StructNew*);
  [[nodiscard]] Result<> visitArrayNew(ArrayNew*);
  [[nodiscard]] Result<> visitArrayNewFixed(ArrayNewFixed*);
  // Used to visit break exprs when traversing the module in the fully nested
  // format. Break label destinations are assumed to have already been visited,
  // with a corresponding push onto the scope stack. As a result, an error will
  // return if a corresponding scope is not found for the break.
  [[nodiscard]] Result<> visitBreak(Break*,
                                    std::optional<Index> label = std::nullopt);
  // Used to visit break nodes when traversing a single block without its
  // context. The type indicates how many values the break carries to its
  // destination.
  [[nodiscard]] Result<> visitBreakWithType(Break*, Type);
  [[nodiscard]] Result<>
  // Used to visit switch exprs when traversing the module in the fully nested
  // format. Switch label destinations are assumed to have already been visited,
  // with a corresponding push onto the scope stack. As a result, an error will
  // return if a corresponding scope is not found for the switch.
  visitSwitch(Switch*, std::optional<Index> defaultLabel = std::nullopt);
  // Used to visit switch nodes when traversing a single block without its
  // context. The type indicates how many values the switch carries to its
  // destination.
  [[nodiscard]] Result<> visitSwitchWithType(Switch*, Type);
  [[nodiscard]] Result<> visitCall(Call*);
  [[nodiscard]] Result<> visitCallIndirect(CallIndirect*);
  [[nodiscard]] Result<> visitCallRef(CallRef*);
  [[nodiscard]] Result<> visitLocalSet(LocalSet*);
  [[nodiscard]] Result<> visitGlobalSet(GlobalSet*);
  [[nodiscard]] Result<> visitThrow(Throw*);
  [[nodiscard]] Result<> visitStringNew(StringNew*);
  [[nodiscard]] Result<> visitStringEncode(StringEncode*);
  [[nodiscard]] Result<> visitContBind(ContBind*);
  [[nodiscard]] Result<> visitResume(Resume*);
  [[nodiscard]] Result<> visitSuspend(Suspend*);
  [[nodiscard]] Result<> visitTupleMake(TupleMake*);
  [[nodiscard]] Result<>
  visitTupleExtract(TupleExtract*,
                    std::optional<uint32_t> arity = std::nullopt);
  [[nodiscard]] Result<> visitPop(Pop*);

private:
  Module& wasm;
  Function* func;
  Builder builder;
  std::optional<Function::DebugLocation> debugLoc;

  void applyDebugLoc(Expression* expr);

  // The context for a single block scope, including the instructions parsed
  // inside that scope so far and the ultimate result type we expect this block
  // to have.
  struct ScopeCtx {
    struct NoScope {};
    struct FuncScope {
      Function* func;
    };
    struct BlockScope {
      Block* block;
    };
    struct IfScope {
      If* iff;
      Name originalLabel;
    };
    struct ElseScope {
      If* iff;
      Name originalLabel;
    };
    struct LoopScope {
      Loop* loop;
    };
    struct TryScope {
      Try* tryy;
      Name originalLabel;
    };
    struct CatchScope {
      Try* tryy;
      Name originalLabel;
      Index index;
    };
    struct CatchAllScope {
      Try* tryy;
      Name originalLabel;
    };
    struct TryTableScope {
      TryTable* trytable;
      Name originalLabel;
    };
    using Scope = std::variant<NoScope,
                               FuncScope,
                               BlockScope,
                               IfScope,
                               ElseScope,
                               LoopScope,
                               TryScope,
                               CatchScope,
                               CatchAllScope,
                               TryTableScope>;

    // The control flow structure we are building expressions for.
    Scope scope;

    // The branch label name for this scope. Always fresh, never shadowed.
    Name label;
    bool labelUsed = false;

    std::vector<Expression*> exprStack;
    // Whether we have seen an unreachable instruction and are in
    // stack-polymorphic unreachable mode.
    bool unreachable = false;

    ScopeCtx() : scope(NoScope{}) {}
    ScopeCtx(Scope scope) : scope(scope) {}
    ScopeCtx(Scope scope, Name label) : scope(scope), label(label) {}

    static ScopeCtx makeFunc(Function* func) {
      return ScopeCtx(FuncScope{func});
    }
    static ScopeCtx makeBlock(Block* block) {
      return ScopeCtx(BlockScope{block});
    }
    static ScopeCtx makeIf(If* iff, Name originalLabel = {}) {
      return ScopeCtx(IfScope{iff, originalLabel});
    }
    static ScopeCtx makeElse(If* iff, Name originalLabel, Name label) {
      return ScopeCtx(ElseScope{iff, originalLabel}, label);
    }
    static ScopeCtx makeLoop(Loop* loop) { return ScopeCtx(LoopScope{loop}); }
    static ScopeCtx makeTry(Try* tryy, Name originalLabel = {}) {
      return ScopeCtx(TryScope{tryy, originalLabel});
    }
    static ScopeCtx
    makeCatch(Try* tryy, Name originalLabel, Index index, Name label) {
      return ScopeCtx(CatchScope{tryy, originalLabel, index}, label);
    }
    static ScopeCtx makeCatchAll(Try* tryy, Name originalLabel, Name label) {
      return ScopeCtx(CatchAllScope{tryy, originalLabel}, label);
    }
    static ScopeCtx makeTryTable(TryTable* trytable, Name originalLabel = {}) {
      return ScopeCtx(TryTableScope{trytable, originalLabel});
    }

    bool isNone() { return std::get_if<NoScope>(&scope); }
    Function* getFunction() {
      if (auto* funcScope = std::get_if<FuncScope>(&scope)) {
        return funcScope->func;
      }
      return nullptr;
    }
    Block* getBlock() {
      if (auto* blockScope = std::get_if<BlockScope>(&scope)) {
        return blockScope->block;
      }
      return nullptr;
    }
    If* getIf() {
      if (auto* ifScope = std::get_if<IfScope>(&scope)) {
        return ifScope->iff;
      }
      return nullptr;
    }
    If* getElse() {
      if (auto* elseScope = std::get_if<ElseScope>(&scope)) {
        return elseScope->iff;
      }
      return nullptr;
    }
    Loop* getLoop() {
      if (auto* loopScope = std::get_if<LoopScope>(&scope)) {
        return loopScope->loop;
      }
      return nullptr;
    }
    Try* getTry() {
      if (auto* tryScope = std::get_if<TryScope>(&scope)) {
        return tryScope->tryy;
      }
      return nullptr;
    }
    Try* getCatch() {
      if (auto* catchScope = std::get_if<CatchScope>(&scope)) {
        return catchScope->tryy;
      }
      return nullptr;
    }
    Try* getCatchAll() {
      if (auto* catchAllScope = std::get_if<CatchAllScope>(&scope)) {
        return catchAllScope->tryy;
      }
      return nullptr;
    }
    TryTable* getTryTable() {
      if (auto* tryTableScope = std::get_if<TryTableScope>(&scope)) {
        return tryTableScope->trytable;
      }
      return nullptr;
    }
    Type getResultType() {
      if (auto* func = getFunction()) {
        return func->type.getSignature().results;
      }
      if (auto* block = getBlock()) {
        return block->type;
      }
      if (auto* iff = getIf()) {
        return iff->type;
      }
      if (auto* iff = getElse()) {
        return iff->type;
      }
      if (auto* loop = getLoop()) {
        return loop->type;
      }
      if (auto* tryy = getTry()) {
        return tryy->type;
      }
      if (auto* tryy = getCatch()) {
        return tryy->type;
      }
      if (auto* tryy = getCatchAll()) {
        return tryy->type;
      }
      if (auto* trytable = getTryTable()) {
        return trytable->type;
      }
      WASM_UNREACHABLE("unexpected scope kind");
    }
    Name getOriginalLabel() {
      if (std::get_if<NoScope>(&scope) || getFunction()) {
        return Name{};
      }
      if (auto* block = getBlock()) {
        return block->name;
      }
      if (auto* ifScope = std::get_if<IfScope>(&scope)) {
        return ifScope->originalLabel;
      }
      if (auto* elseScope = std::get_if<ElseScope>(&scope)) {
        return elseScope->originalLabel;
      }
      if (auto* loop = getLoop()) {
        return loop->name;
      }
      if (auto* tryScope = std::get_if<TryScope>(&scope)) {
        return tryScope->originalLabel;
      }
      if (auto* catchScope = std::get_if<CatchScope>(&scope)) {
        return catchScope->originalLabel;
      }
      if (auto* catchAllScope = std::get_if<CatchAllScope>(&scope)) {
        return catchAllScope->originalLabel;
      }
      if (auto* tryTableScope = std::get_if<TryTableScope>(&scope)) {
        return tryTableScope->originalLabel;
      }
      WASM_UNREACHABLE("unexpected scope kind");
    }
    Index getIndex() {
      if (auto* catchScope = std::get_if<CatchScope>(&scope)) {
        return catchScope->index;
      }
      WASM_UNREACHABLE("unexpected scope kind");
    }
  };

  // The stack of block contexts currently being parsed.
  std::vector<ScopeCtx> scopeStack;

  // Map label names to stacks of label depths at which they appear. The
  // relative index of a label name is the current depth minus the top depth on
  // its stack.
  std::unordered_map<Name, std::vector<Index>> labelDepths;

  Name makeFresh(Name label) {
    return Names::getValidName(label, [&](Name candidate) {
      return labelDepths.insert({candidate, {}}).second;
    });
  }

  void pushScope(ScopeCtx scope) {
    if (auto label = scope.getOriginalLabel()) {
      // Assign a fresh label to the scope, if necessary.
      if (!scope.label) {
        scope.label = makeFresh(label);
      }
      // Record the original label to handle references to it correctly.
      labelDepths[label].push_back(scopeStack.size() + 1);
    }
    scopeStack.push_back(scope);
  }

  ScopeCtx& getScope() {
    if (scopeStack.empty()) {
      // We are not in a block context, so push a dummy scope.
      scopeStack.push_back({});
    }
    return scopeStack.back();
  }

  Result<ScopeCtx*> getScope(Index label) {
    Index numLabels = scopeStack.size();
    if (!scopeStack.empty() && scopeStack[0].isNone()) {
      --numLabels;
    }
    if (label >= numLabels) {
      return Err{"label index out of bounds"};
    }
    return &scopeStack[scopeStack.size() - 1 - label];
  }

  // Collect the current scope into a single expression. If it has multiple
  // top-level expressions, this requires collecting them into a block. If we
  // are in a block context, we can collect them directly into the destination
  // `block`, but otherwise we will have to allocate a new block.
  Result<Expression*> finishScope(Block* block = nullptr);

  [[nodiscard]] Result<Name> getLabelName(Index label);
  [[nodiscard]] Result<Name> getDelegateLabelName(Index label);
  [[nodiscard]] Result<Index> addScratchLocal(Type);
  [[nodiscard]] Result<Expression*> pop(size_t size = 1);

  struct HoistedVal {
    // The index in the stack of the original value-producing expression.
    Index valIndex;
    // The local.get placed on the stack, if any.
    LocalGet* get;
  };

  // Find the last value-producing expression, if any, and hoist its value to
  // the top of the stack using a scratch local if necessary.
  [[nodiscard]] MaybeResult<HoistedVal> hoistLastValue();
  // Transform the stack as necessary such that the original producer of the
  // hoisted value will be popped along with the final expression that produces
  // the value, if they are different. May only be called directly after
  // hoistLastValue(). `sizeHint` is the size of the type we ultimately want to
  // consume, so if the hoisted value has `sizeHint` elements, it is left intact
  // even if it is a tuple. Otherwise, hoisted tuple values will be broken into
  // pieces.
  [[nodiscard]] Result<> packageHoistedValue(const HoistedVal&,
                                             size_t sizeHint = 1);

  [[nodiscard]] Result<Expression*>
  getBranchValue(Expression* curr, Name labelName, std::optional<Index> label);

  void dump();
};

} // namespace wasm

#endif // wasm_wasm_ir_builder_h
