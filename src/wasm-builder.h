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

#ifndef wasm_wasm_builder_h
#define wasm_wasm_builder_h

#include "ir/manipulation.h"
#include "parsing.h"
#include "wasm.h"

namespace wasm {

// Useful data structures

struct NameType {
  Name name;
  Type type;
  NameType() : name(nullptr), type(Type::none) {}
  NameType(Name name, Type type) : name(name), type(type) {}
};

// General AST node builder

class Builder {
  Module& wasm;

public:
  Builder(Module& wasm) : wasm(wasm) {}

  // make* functions create an expression instance.

  static std::unique_ptr<Function> makeFunction(Name name,
                                                Signature sig,
                                                std::vector<Type>&& vars,
                                                Expression* body = nullptr) {
    auto func = std::make_unique<Function>();
    func->name = name;
    func->sig = sig;
    func->body = body;
    func->vars.swap(vars);
    return func;
  }

  static std::unique_ptr<Function> makeFunction(Name name,
                                                std::vector<NameType>&& params,
                                                Type resultType,
                                                std::vector<NameType>&& vars,
                                                Expression* body = nullptr) {
    auto func = std::make_unique<Function>();
    func->name = name;
    func->body = body;
    std::vector<Type> paramVec;
    for (auto& param : params) {
      paramVec.push_back(param.type);
      Index index = func->localNames.size();
      func->localIndices[param.name] = index;
      func->localNames[index] = param.name;
    }
    func->sig = Signature(Type(paramVec), resultType);
    for (auto& var : vars) {
      func->vars.push_back(var.type);
      Index index = func->localNames.size();
      func->localIndices[var.name] = index;
      func->localNames[index] = var.name;
    }
    return func;
  }

  static std::unique_ptr<Table> makeTable(Name name,
                                          Type type = Type::funcref,
                                          Address initial = 0,
                                          Address max = Table::kMaxSize) {
    auto table = std::make_unique<Table>();
    table->name = name;
    table->type = type;
    table->initial = initial;
    table->max = max;
    return table;
  }

  static std::unique_ptr<ElementSegment>
  makeElementSegment(Name name,
                     Name table,
                     Expression* offset = nullptr,
                     Type type = Type::funcref) {
    auto seg = std::make_unique<ElementSegment>();
    seg->name = name;
    seg->table = table;
    seg->offset = offset;
    seg->type = type;
    return seg;
  }

  static std::unique_ptr<Export>
  makeExport(Name name, Name value, ExternalKind kind) {
    auto export_ = std::make_unique<Export>();
    export_->name = name;
    export_->value = value;
    export_->kind = kind;
    return export_;
  }

  enum Mutability { Mutable, Immutable };

  static std::unique_ptr<Global>
  makeGlobal(Name name, Type type, Expression* init, Mutability mutable_) {
    auto glob = std::make_unique<Global>();
    glob->name = name;
    glob->type = type;
    glob->init = init;
    glob->mutable_ = mutable_ == Mutable;
    return glob;
  }

  static std::unique_ptr<Event>
  makeEvent(Name name, uint32_t attribute, Signature sig) {
    auto event = std::make_unique<Event>();
    event->name = name;
    event->attribute = attribute;
    event->sig = sig;
    return event;
  }

  // IR nodes

  Nop* makeNop() { return wasm.allocator.alloc<Nop>(); }
  Block* makeBlock(Expression* first = nullptr) {
    auto* ret = wasm.allocator.alloc<Block>();
    if (first) {
      ret->list.push_back(first);
      ret->finalize();
    }
    return ret;
  }
  Block* makeBlock(Name name, Expression* first = nullptr) {
    auto* ret = makeBlock(first);
    ret->name = name;
    ret->finalize();
    return ret;
  }
  Block* makeBlock(const std::vector<Expression*>& items) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->list.set(items);
    ret->finalize();
    return ret;
  }
  Block* makeBlock(const std::vector<Expression*>& items, Type type) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->list.set(items);
    ret->finalize(type);
    return ret;
  }
  Block* makeBlock(const ExpressionList& items) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->list.set(items);
    ret->finalize();
    return ret;
  }
  Block* makeBlock(const ExpressionList& items, Type type) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->list.set(items);
    ret->finalize(type);
    return ret;
  }
  Block* makeBlock(Name name, const ExpressionList& items) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->name = name;
    ret->list.set(items);
    ret->finalize();
    return ret;
  }
  Block* makeBlock(Name name, const ExpressionList& items, Type type) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->name = name;
    ret->list.set(items);
    ret->finalize(type);
    return ret;
  }
  If* makeIf(Expression* condition,
             Expression* ifTrue,
             Expression* ifFalse = nullptr) {
    auto* ret = wasm.allocator.alloc<If>();
    ret->condition = condition;
    ret->ifTrue = ifTrue;
    ret->ifFalse = ifFalse;
    ret->finalize();
    return ret;
  }
  If* makeIf(Expression* condition,
             Expression* ifTrue,
             Expression* ifFalse,
             Type type) {
    auto* ret = wasm.allocator.alloc<If>();
    ret->condition = condition;
    ret->ifTrue = ifTrue;
    ret->ifFalse = ifFalse;
    ret->finalize(type);
    return ret;
  }
  Loop* makeLoop(Name name, Expression* body) {
    auto* ret = wasm.allocator.alloc<Loop>();
    ret->name = name;
    ret->body = body;
    ret->finalize();
    return ret;
  }
  Loop* makeLoop(Name name, Expression* body, Type type) {
    auto* ret = wasm.allocator.alloc<Loop>();
    ret->name = name;
    ret->body = body;
    ret->finalize(type);
    return ret;
  }
  Break* makeBreak(Name name,
                   Expression* value = nullptr,
                   Expression* condition = nullptr) {
    auto* ret = wasm.allocator.alloc<Break>();
    ret->name = name;
    ret->value = value;
    ret->condition = condition;
    ret->finalize();
    return ret;
  }
  template<typename T>
  Switch* makeSwitch(T& list,
                     Name default_,
                     Expression* condition,
                     Expression* value = nullptr) {
    auto* ret = wasm.allocator.alloc<Switch>();
    ret->targets.set(list);
    ret->default_ = default_;
    ret->value = value;
    ret->condition = condition;
    return ret;
  }
  Call* makeCall(Name target,
                 const std::vector<Expression*>& args,
                 Type type,
                 bool isReturn = false) {
    auto* call = wasm.allocator.alloc<Call>();
    // not all functions may exist yet, so type must be provided
    call->type = type;
    call->target = target;
    call->operands.set(args);
    call->isReturn = isReturn;
    return call;
  }
  template<typename T>
  Call* makeCall(Name target, const T& args, Type type, bool isReturn = false) {
    auto* call = wasm.allocator.alloc<Call>();
    // not all functions may exist yet, so type must be provided
    call->type = type;
    call->target = target;
    call->operands.set(args);
    call->isReturn = isReturn;
    call->finalize();
    return call;
  }
  template<typename T>
  CallIndirect* makeCallIndirect(const Name table,
                                 Expression* target,
                                 const T& args,
                                 Signature sig,
                                 bool isReturn = false) {
    auto* call = wasm.allocator.alloc<CallIndirect>();
    call->table = table;
    call->sig = sig;
    call->type = sig.results;
    call->target = target;
    call->operands.set(args);
    call->isReturn = isReturn;
    call->finalize();
    return call;
  }
  template<typename T>
  CallRef* makeCallRef(Expression* target,
                       const T& args,
                       Type type,
                       bool isReturn = false) {
    auto* call = wasm.allocator.alloc<CallRef>();
    call->type = type;
    call->target = target;
    call->operands.set(args);
    call->isReturn = isReturn;
    call->finalize();
    return call;
  }
  LocalGet* makeLocalGet(Index index, Type type) {
    auto* ret = wasm.allocator.alloc<LocalGet>();
    ret->index = index;
    ret->type = type;
    return ret;
  }
  LocalSet* makeLocalSet(Index index, Expression* value) {
    auto* ret = wasm.allocator.alloc<LocalSet>();
    ret->index = index;
    ret->value = value;
    ret->makeSet();
    ret->finalize();
    return ret;
  }
  LocalSet* makeLocalTee(Index index, Expression* value, Type type) {
    auto* ret = wasm.allocator.alloc<LocalSet>();
    ret->index = index;
    ret->value = value;
    ret->makeTee(type);
    return ret;
  }
  GlobalGet* makeGlobalGet(Name name, Type type) {
    auto* ret = wasm.allocator.alloc<GlobalGet>();
    ret->name = name;
    ret->type = type;
    return ret;
  }
  GlobalSet* makeGlobalSet(Name name, Expression* value) {
    auto* ret = wasm.allocator.alloc<GlobalSet>();
    ret->name = name;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  Load* makeLoad(unsigned bytes,
                 bool signed_,
                 uint32_t offset,
                 unsigned align,
                 Expression* ptr,
                 Type type) {
    auto* ret = wasm.allocator.alloc<Load>();
    ret->isAtomic = false;
    ret->bytes = bytes;
    ret->signed_ = signed_;
    ret->offset = offset;
    ret->align = align;
    ret->ptr = ptr;
    ret->type = type;
    return ret;
  }
  Load*
  makeAtomicLoad(unsigned bytes, uint32_t offset, Expression* ptr, Type type) {
    Load* load = makeLoad(bytes, false, offset, bytes, ptr, type);
    load->isAtomic = true;
    return load;
  }
  AtomicWait* makeAtomicWait(Expression* ptr,
                             Expression* expected,
                             Expression* timeout,
                             Type expectedType,
                             Address offset) {
    auto* wait = wasm.allocator.alloc<AtomicWait>();
    wait->offset = offset;
    wait->ptr = ptr;
    wait->expected = expected;
    wait->timeout = timeout;
    wait->expectedType = expectedType;
    wait->finalize();
    return wait;
  }
  AtomicNotify*
  makeAtomicNotify(Expression* ptr, Expression* notifyCount, Address offset) {
    auto* notify = wasm.allocator.alloc<AtomicNotify>();
    notify->offset = offset;
    notify->ptr = ptr;
    notify->notifyCount = notifyCount;
    notify->finalize();
    return notify;
  }
  AtomicFence* makeAtomicFence() { return wasm.allocator.alloc<AtomicFence>(); }
  Store* makeStore(unsigned bytes,
                   uint32_t offset,
                   unsigned align,
                   Expression* ptr,
                   Expression* value,
                   Type type) {
    auto* ret = wasm.allocator.alloc<Store>();
    ret->isAtomic = false;
    ret->bytes = bytes;
    ret->offset = offset;
    ret->align = align;
    ret->ptr = ptr;
    ret->value = value;
    ret->valueType = type;
    ret->finalize();
    assert(ret->value->type.isConcrete() ? ret->value->type == type : true);
    return ret;
  }
  Store* makeAtomicStore(unsigned bytes,
                         uint32_t offset,
                         Expression* ptr,
                         Expression* value,
                         Type type) {
    Store* store = makeStore(bytes, offset, bytes, ptr, value, type);
    store->isAtomic = true;
    return store;
  }
  AtomicRMW* makeAtomicRMW(AtomicRMWOp op,
                           unsigned bytes,
                           uint32_t offset,
                           Expression* ptr,
                           Expression* value,
                           Type type) {
    auto* ret = wasm.allocator.alloc<AtomicRMW>();
    ret->op = op;
    ret->bytes = bytes;
    ret->offset = offset;
    ret->ptr = ptr;
    ret->value = value;
    ret->type = type;
    ret->finalize();
    return ret;
  }
  AtomicCmpxchg* makeAtomicCmpxchg(unsigned bytes,
                                   uint32_t offset,
                                   Expression* ptr,
                                   Expression* expected,
                                   Expression* replacement,
                                   Type type) {
    auto* ret = wasm.allocator.alloc<AtomicCmpxchg>();
    ret->bytes = bytes;
    ret->offset = offset;
    ret->ptr = ptr;
    ret->expected = expected;
    ret->replacement = replacement;
    ret->type = type;
    ret->finalize();
    return ret;
  }
  SIMDExtract*
  makeSIMDExtract(SIMDExtractOp op, Expression* vec, uint8_t index) {
    auto* ret = wasm.allocator.alloc<SIMDExtract>();
    ret->op = op;
    ret->vec = vec;
    ret->index = index;
    ret->finalize();
    return ret;
  }
  SIMDReplace* makeSIMDReplace(SIMDReplaceOp op,
                               Expression* vec,
                               uint8_t index,
                               Expression* value) {
    auto* ret = wasm.allocator.alloc<SIMDReplace>();
    ret->op = op;
    ret->vec = vec;
    ret->index = index;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  SIMDShuffle* makeSIMDShuffle(Expression* left,
                               Expression* right,
                               const std::array<uint8_t, 16>& mask) {
    auto* ret = wasm.allocator.alloc<SIMDShuffle>();
    ret->left = left;
    ret->right = right;
    ret->mask = mask;
    ret->finalize();
    return ret;
  }
  SIMDTernary* makeSIMDTernary(SIMDTernaryOp op,
                               Expression* a,
                               Expression* b,
                               Expression* c) {
    auto* ret = wasm.allocator.alloc<SIMDTernary>();
    ret->op = op;
    ret->a = a;
    ret->b = b;
    ret->c = c;
    ret->finalize();
    return ret;
  }
  SIMDShift* makeSIMDShift(SIMDShiftOp op, Expression* vec, Expression* shift) {
    auto* ret = wasm.allocator.alloc<SIMDShift>();
    ret->op = op;
    ret->vec = vec;
    ret->shift = shift;
    ret->finalize();
    return ret;
  }
  SIMDLoad*
  makeSIMDLoad(SIMDLoadOp op, Address offset, Address align, Expression* ptr) {
    auto* ret = wasm.allocator.alloc<SIMDLoad>();
    ret->op = op;
    ret->offset = offset;
    ret->align = align;
    ret->ptr = ptr;
    ret->finalize();
    return ret;
  }
  SIMDLoadStoreLane* makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp op,
                                           Address offset,
                                           Address align,
                                           uint8_t index,
                                           Expression* ptr,
                                           Expression* vec) {
    auto* ret = wasm.allocator.alloc<SIMDLoadStoreLane>();
    ret->op = op;
    ret->offset = offset;
    ret->align = align;
    ret->index = index;
    ret->ptr = ptr;
    ret->vec = vec;
    ret->finalize();
    return ret;
  }
  MemoryInit* makeMemoryInit(uint32_t segment,
                             Expression* dest,
                             Expression* offset,
                             Expression* size) {
    auto* ret = wasm.allocator.alloc<MemoryInit>();
    ret->segment = segment;
    ret->dest = dest;
    ret->offset = offset;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  DataDrop* makeDataDrop(uint32_t segment) {
    auto* ret = wasm.allocator.alloc<DataDrop>();
    ret->segment = segment;
    ret->finalize();
    return ret;
  }
  MemoryCopy*
  makeMemoryCopy(Expression* dest, Expression* source, Expression* size) {
    auto* ret = wasm.allocator.alloc<MemoryCopy>();
    ret->dest = dest;
    ret->source = source;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  MemoryFill*
  makeMemoryFill(Expression* dest, Expression* value, Expression* size) {
    auto* ret = wasm.allocator.alloc<MemoryFill>();
    ret->dest = dest;
    ret->value = value;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  Const* makeConst(Literal value) {
    assert(value.type.isNumber());
    auto* ret = wasm.allocator.alloc<Const>();
    ret->value = value;
    ret->type = value.type;
    return ret;
  }
  template<typename T> Const* makeConst(T x) { return makeConst(Literal(x)); }
  Unary* makeUnary(UnaryOp op, Expression* value) {
    auto* ret = wasm.allocator.alloc<Unary>();
    ret->op = op;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  Const* makeConstPtr(uint64_t val) {
    return makeConst(Literal::makeFromInt64(val, wasm.memory.indexType));
  }
  Binary* makeBinary(BinaryOp op, Expression* left, Expression* right) {
    auto* ret = wasm.allocator.alloc<Binary>();
    ret->op = op;
    ret->left = left;
    ret->right = right;
    ret->finalize();
    return ret;
  }
  Select*
  makeSelect(Expression* condition, Expression* ifTrue, Expression* ifFalse) {
    auto* ret = wasm.allocator.alloc<Select>();
    ret->condition = condition;
    ret->ifTrue = ifTrue;
    ret->ifFalse = ifFalse;
    ret->finalize();
    return ret;
  }
  Select* makeSelect(Expression* condition,
                     Expression* ifTrue,
                     Expression* ifFalse,
                     Type type) {
    auto* ret = wasm.allocator.alloc<Select>();
    ret->condition = condition;
    ret->ifTrue = ifTrue;
    ret->ifFalse = ifFalse;
    ret->finalize(type);
    return ret;
  }
  Return* makeReturn(Expression* value = nullptr) {
    auto* ret = wasm.allocator.alloc<Return>();
    ret->value = value;
    return ret;
  }
  MemorySize* makeMemorySize() {
    auto* ret = wasm.allocator.alloc<MemorySize>();
    if (wasm.memory.is64()) {
      ret->make64();
    }
    ret->finalize();
    return ret;
  }
  MemoryGrow* makeMemoryGrow(Expression* delta) {
    auto* ret = wasm.allocator.alloc<MemoryGrow>();
    if (wasm.memory.is64()) {
      ret->make64();
    }
    ret->delta = delta;
    ret->finalize();
    return ret;
  }
  TableGet* makeTableGet(Name table, Expression* offset, Type type) {
    auto* ret = wasm.allocator.alloc<TableGet>();
    ret->table = table;
    ret->offset = offset;
    ret->finalize(type);
    return ret;
  }
  TableSet* makeTableSet(Name table, Expression* value, Expression* offset) {
    auto* ret = wasm.allocator.alloc<TableSet>();
    ret->table = table;
    ret->value = value;
    ret->offset = offset;
    ret->finalize();
    return ret;
  }
  TableSize* makeTableSize(Name table) {
    auto* ret = wasm.allocator.alloc<TableSize>();
    ret->table = table;
    return ret;
  }
  TableGrow*
  makeTableGrow(Name table, Expression* delta, Expression* initialValue) {
    auto* ret = wasm.allocator.alloc<TableGrow>();
    ret->table = table;
    ret->delta = delta;
    ret->initialValue = initialValue;
    return ret;
  }
  TableFill* makeTableFill(Name table,
                           Expression* size,
                           Expression* value,
                           Expression* dest) {
    auto* ret = wasm.allocator.alloc<TableFill>();
    ret->table = table;
    ret->size = size;
    ret->value = value;
    ret->dest = dest;
    ret->finalize();
    return ret;
  }
  TableCopy* makeTableCopy(Name srcTable,
                           Name destTable,
                           Expression* size,
                           Expression* srcOffset,
                           Expression* destOffset) {
    auto* ret = wasm.allocator.alloc<TableCopy>();
    ret->srcTable = srcTable;
    ret->destTable = destTable;
    ret->size = size;
    ret->srcOffset = srcOffset;
    ret->destOffset = destOffset;
    ret->finalize();
    return ret;
  }
  TableInit* makeTableInit(Name table,
                           Name segment,
                           Expression* size,
                           Expression* srcOffset,
                           Expression* destOffset) {
    auto* ret = wasm.allocator.alloc<TableInit>();
    ret->table = table;
    ret->segment = segment;
    ret->srcOffset = srcOffset;
    ret->destOffset = destOffset;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  ElemDrop* makeElemDrop(Name segment) {
    auto* ret = wasm.allocator.alloc<ElemDrop>();
    ret->segment = segment;
    return ret;
  }
  RefNull* makeRefNull(Type type) {
    auto* ret = wasm.allocator.alloc<RefNull>();
    ret->finalize(type);
    return ret;
  }
  RefIs* makeRefIs(RefIsOp op, Expression* value) {
    auto* ret = wasm.allocator.alloc<RefIs>();
    ret->op = op;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  RefFunc* makeRefFunc(Name func, HeapType heapType) {
    auto* ret = wasm.allocator.alloc<RefFunc>();
    ret->func = func;
    ret->finalize(Type(heapType, NonNullable));
    return ret;
  }
  RefEq* makeRefEq(Expression* left, Expression* right) {
    auto* ret = wasm.allocator.alloc<RefEq>();
    ret->left = left;
    ret->right = right;
    ret->finalize();
    return ret;
  }

private:
  Try* makeTry(Name name,
               Expression* body,
               const std::vector<Name>& catchEvents,
               const std::vector<Expression*>& catchBodies,
               Name delegateTarget,
               Type type,
               bool hasType) { // differentiate whether a type was passed in
    auto* ret = wasm.allocator.alloc<Try>();
    ret->name = name;
    ret->body = body;
    ret->catchEvents.set(catchEvents);
    ret->catchBodies.set(catchBodies);
    if (hasType) {
      ret->finalize(type);
    } else {
      ret->finalize();
    }
    return ret;
  }

public:
  Try* makeTry(Expression* body,
               const std::vector<Name>& catchEvents,
               const std::vector<Expression*>& catchBodies) {
    return makeTry(
      Name(), body, catchEvents, catchBodies, Name(), Type::none, false);
  }
  Try* makeTry(Expression* body,
               const std::vector<Name>& catchEvents,
               const std::vector<Expression*>& catchBodies,
               Type type) {
    return makeTry(Name(), body, catchEvents, catchBodies, Name(), type, true);
  }
  Try* makeTry(Name name,
               Expression* body,
               const std::vector<Name>& catchEvents,
               const std::vector<Expression*>& catchBodies) {
    return makeTry(
      name, body, catchEvents, catchBodies, Name(), Type::none, false);
  }
  Try* makeTry(Name name,
               Expression* body,
               const std::vector<Name>& catchEvents,
               const std::vector<Expression*>& catchBodies,
               Type type) {
    return makeTry(name, body, catchEvents, catchBodies, Name(), type, true);
  }
  Try* makeTry(Expression* body, Name delegateTarget) {
    return makeTry(Name(), body, {}, {}, delegateTarget, Type::none, false);
  }
  Try* makeTry(Expression* body, Name delegateTarget, Type type) {
    return makeTry(Name(), body, {}, {}, delegateTarget, type, true);
  }
  Try* makeTry(Name name, Expression* body, Name delegateTarget) {
    return makeTry(name, body, {}, {}, delegateTarget, Type::none, false);
  }
  Try* makeTry(Name name, Expression* body, Name delegateTarget, Type type) {
    return makeTry(name, body, {}, {}, delegateTarget, type, true);
  }
  Throw* makeThrow(Event* event, const std::vector<Expression*>& args) {
    return makeThrow(event->name, args);
  }
  Throw* makeThrow(Name event, const std::vector<Expression*>& args) {
    auto* ret = wasm.allocator.alloc<Throw>();
    ret->event = event;
    ret->operands.set(args);
    ret->finalize();
    return ret;
  }
  Rethrow* makeRethrow(Name target) {
    auto* ret = wasm.allocator.alloc<Rethrow>();
    ret->target = target;
    ret->finalize();
    return ret;
  }
  Unreachable* makeUnreachable() { return wasm.allocator.alloc<Unreachable>(); }
  Pop* makePop(Type type) {
    auto* ret = wasm.allocator.alloc<Pop>();
    ret->type = type;
    ret->finalize();
    return ret;
  }
  template<typename ListType> TupleMake* makeTupleMake(ListType&& operands) {
    auto* ret = wasm.allocator.alloc<TupleMake>();
    ret->operands.set(operands);
    ret->finalize();
    return ret;
  }
  TupleExtract* makeTupleExtract(Expression* tuple, Index index) {
    auto* ret = wasm.allocator.alloc<TupleExtract>();
    ret->tuple = tuple;
    ret->index = index;
    ret->finalize();
    return ret;
  }
  I31New* makeI31New(Expression* value) {
    auto* ret = wasm.allocator.alloc<I31New>();
    ret->value = value;
    ret->finalize();
    return ret;
  }
  I31Get* makeI31Get(Expression* i31, bool signed_) {
    auto* ret = wasm.allocator.alloc<I31Get>();
    ret->i31 = i31;
    ret->signed_ = signed_;
    ret->finalize();
    return ret;
  }
  RefTest* makeRefTest(Expression* ref, Expression* rtt) {
    auto* ret = wasm.allocator.alloc<RefTest>();
    ret->ref = ref;
    ret->rtt = rtt;
    ret->finalize();
    return ret;
  }
  RefCast* makeRefCast(Expression* ref, Expression* rtt) {
    auto* ret = wasm.allocator.alloc<RefCast>();
    ret->ref = ref;
    ret->rtt = rtt;
    ret->finalize();
    return ret;
  }
  BrOn*
  makeBrOn(BrOnOp op, Name name, Expression* ref, Expression* rtt = nullptr) {
    auto* ret = wasm.allocator.alloc<BrOn>();
    ret->op = op;
    ret->name = name;
    ret->ref = ref;
    ret->rtt = rtt;
    ret->finalize();
    return ret;
  }
  RttCanon* makeRttCanon(HeapType heapType) {
    auto* ret = wasm.allocator.alloc<RttCanon>();
    ret->type = Type(Rtt(0, heapType));
    ret->finalize();
    return ret;
  }
  RttSub* makeRttSub(HeapType heapType, Expression* parent) {
    auto* ret = wasm.allocator.alloc<RttSub>();
    ret->parent = parent;
    auto parentRtt = parent->type.getRtt();
    if (parentRtt.hasDepth()) {
      ret->type = Type(Rtt(parentRtt.depth + 1, heapType));
    } else {
      ret->type = Type(Rtt(heapType));
    }
    ret->finalize();
    return ret;
  }
  RttSub* makeRttFreshSub(HeapType heapType, Expression* parent) {
    auto* ret = makeRttSub(heapType, parent);
    ret->fresh = true;
    return ret;
  }
  template<typename T>
  StructNew* makeStructNew(Expression* rtt, const T& args) {
    auto* ret = wasm.allocator.alloc<StructNew>();
    ret->rtt = rtt;
    ret->operands.set(args);
    ret->finalize();
    return ret;
  }
  StructGet*
  makeStructGet(Index index, Expression* ref, Type type, bool signed_ = false) {
    auto* ret = wasm.allocator.alloc<StructGet>();
    ret->index = index;
    ret->ref = ref;
    ret->type = type;
    ret->signed_ = signed_;
    ret->finalize();
    return ret;
  }
  StructSet* makeStructSet(Index index, Expression* ref, Expression* value) {
    auto* ret = wasm.allocator.alloc<StructSet>();
    ret->index = index;
    ret->ref = ref;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  ArrayNew*
  makeArrayNew(Expression* rtt, Expression* size, Expression* init = nullptr) {
    auto* ret = wasm.allocator.alloc<ArrayNew>();
    ret->rtt = rtt;
    ret->size = size;
    ret->init = init;
    ret->finalize();
    return ret;
  }
  ArrayGet*
  makeArrayGet(Expression* ref, Expression* index, bool signed_ = false) {
    auto* ret = wasm.allocator.alloc<ArrayGet>();
    ret->ref = ref;
    ret->index = index;
    ret->signed_ = signed_;
    ret->finalize();
    return ret;
  }
  ArraySet*
  makeArraySet(Expression* ref, Expression* index, Expression* value) {
    auto* ret = wasm.allocator.alloc<ArraySet>();
    ret->ref = ref;
    ret->index = index;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  ArrayLen* makeArrayLen(Expression* ref) {
    auto* ret = wasm.allocator.alloc<ArrayLen>();
    ret->ref = ref;
    ret->finalize();
    return ret;
  }
  ArrayCopy* makeArrayCopy(Expression* destRef,
                           Expression* destIndex,
                           Expression* srcRef,
                           Expression* srcIndex,
                           Expression* length) {
    auto* ret = wasm.allocator.alloc<ArrayCopy>();
    ret->destRef = destRef;
    ret->destIndex = destIndex;
    ret->srcRef = srcRef;
    ret->srcIndex = srcIndex;
    ret->length = length;
    ret->finalize();
    return ret;
  }
  RefAs* makeRefAs(RefAsOp op, Expression* value) {
    auto* ret = wasm.allocator.alloc<RefAs>();
    ret->op = op;
    ret->value = value;
    ret->finalize();
    return ret;
  }

  // Additional helpers

  Drop* makeDrop(Expression* value) {
    auto* ret = wasm.allocator.alloc<Drop>();
    ret->value = value;
    ret->finalize();
    return ret;
  }

  // Make a constant expression. This might be a wasm Const, or something
  // else of constant value like ref.null.
  Expression* makeConstantExpression(Literal value) {
    auto type = value.type;
    if (type.isNumber()) {
      return makeConst(value);
    }
    if (value.isNull()) {
      return makeRefNull(type);
    }
    if (type.isFunction()) {
      return makeRefFunc(value.getFunc(), type.getHeapType());
    }
    if (type.isRtt()) {
      return makeRtt(value.type);
    }
    TODO_SINGLE_COMPOUND(type);
    switch (type.getBasic()) {
      case Type::externref:
      case Type::anyref:
      case Type::eqref:
        assert(value.isNull() && "unexpected non-null reference type literal");
        return makeRefNull(type);
      case Type::i31ref:
        return makeI31New(makeConst(value.geti31()));
      default:
        WASM_UNREACHABLE("invalid constant expression");
    }
  }

  Expression* makeConstantExpression(Literals values) {
    assert(values.size() > 0);
    if (values.size() == 1) {
      return makeConstantExpression(values[0]);
    } else {
      std::vector<Expression*> consts;
      for (auto value : values) {
        consts.push_back(makeConstantExpression(value));
      }
      return makeTupleMake(consts);
    }
  }

  // Given a type, creates an RTT expression of that type, using a combination
  // of rtt.canon and rtt.subs.
  Expression* makeRtt(Type type) {
    Expression* ret = makeRttCanon(type.getHeapType());
    if (type.getRtt().hasDepth()) {
      for (Index i = 0; i < type.getRtt().depth; i++) {
        ret = makeRttSub(type.getHeapType(), ret);
      }
    }
    return ret;
  }

  // Additional utility functions for building on top of nodes
  // Convenient to have these on Builder, as it has allocation built in

  static Index addParam(Function* func, Name name, Type type) {
    // only ok to add a param if no vars, otherwise indices are invalidated
    assert(func->localIndices.size() == func->sig.params.size());
    assert(name.is());
    std::vector<Type> params(func->sig.params.begin(), func->sig.params.end());
    params.push_back(type);
    func->sig.params = Type(params);
    Index index = func->localNames.size();
    func->localIndices[name] = index;
    func->localNames[index] = name;
    return index;
  }

  static Index addVar(Function* func, Name name, Type type) {
    // always ok to add a var, it does not affect other indices
    assert(type.isConcrete());
    Index index = func->getNumLocals();
    if (name.is()) {
      func->localIndices[name] = index;
      func->localNames[index] = name;
    }
    func->vars.emplace_back(type);
    return index;
  }

  static Index addVar(Function* func, Type type) {
    return addVar(func, Name(), type);
  }

  static void clearLocalNames(Function* func) {
    func->localNames.clear();
    func->localIndices.clear();
  }

  static void clearLocals(Function* func) {
    func->sig.params = Type::none;
    func->vars.clear();
    clearLocalNames(func);
  }

  // ensure a node is a block, if it isn't already, and optionally append to the
  // block
  Block* blockify(Expression* any, Expression* append = nullptr) {
    Block* block = nullptr;
    if (any) {
      block = any->dynCast<Block>();
    }
    if (!block) {
      block = makeBlock(any);
    }
    if (append) {
      block->list.push_back(append);
      block->finalize();
    }
    return block;
  }

  template<typename... Ts>
  Block* blockify(Expression* any, Expression* append, Ts... args) {
    return blockify(blockify(any, append), args...);
  }

  // ensure a node is a block, if it isn't already, and optionally append to the
  // block this variant sets a name for the block, so it will not reuse a block
  // already named
  Block*
  blockifyWithName(Expression* any, Name name, Expression* append = nullptr) {
    Block* block = nullptr;
    if (any) {
      block = any->dynCast<Block>();
    }
    if (!block || block->name.is()) {
      block = makeBlock(any);
    }
    block->name = name;
    if (append) {
      block->list.push_back(append);
      block->finalize();
    }
    return block;
  }

  // a helper for the common pattern of a sequence of two expressions. Similar
  // to blockify, but does *not* reuse a block if the first is one.
  Block* makeSequence(Expression* left, Expression* right) {
    auto* block = makeBlock(left);
    block->list.push_back(right);
    block->finalize();
    return block;
  }

  Block* makeSequence(Expression* left, Expression* right, Type type) {
    auto* block = makeBlock(left);
    block->list.push_back(right);
    block->finalize(type);
    return block;
  }

  // Drop an expression if it has a concrete type
  Expression* dropIfConcretelyTyped(Expression* curr) {
    if (!curr->type.isConcrete()) {
      return curr;
    }
    return makeDrop(curr);
  }

  void flip(If* iff) {
    std::swap(iff->ifTrue, iff->ifFalse);
    iff->condition = makeUnary(EqZInt32, iff->condition);
  }

  // Returns a replacement with the precise same type, and with minimal contents
  // as best we can. As a replacement, this may reuse the input node.
  template<typename T> Expression* replaceWithIdenticalType(T* curr) {
    if (curr->type.isTuple()) {
      return makeConstantExpression(Literal::makeZeros(curr->type));
    }
    if (curr->type.isNullable()) {
      return ExpressionManipulator::refNull(curr, curr->type);
    }
    if (curr->type.isFunction() || !curr->type.isBasic()) {
      // We can't do any better, keep the original.
      return curr;
    }
    Literal value;
    // TODO: reuse node conditionally when possible for literals
    switch (curr->type.getBasic()) {
      case Type::i32:
        value = Literal(int32_t(0));
        break;
      case Type::i64:
        value = Literal(int64_t(0));
        break;
      case Type::f32:
        value = Literal(float(0));
        break;
      case Type::f64:
        value = Literal(double(0));
        break;
      case Type::v128: {
        std::array<uint8_t, 16> bytes;
        bytes.fill(0);
        value = Literal(bytes.data());
        break;
      }
      case Type::funcref:
        WASM_UNREACHABLE("handled above");
      case Type::externref:
      case Type::anyref:
      case Type::eqref:
        return ExpressionManipulator::refNull(curr, curr->type);
      case Type::i31ref:
        return makeI31New(makeConst(0));
      case Type::dataref:
        return curr;
      case Type::none:
        return ExpressionManipulator::nop(curr);
      case Type::unreachable:
        return ExpressionManipulator::unreachable(curr);
    }
    return makeConst(value);
  }
};

// This class adds methods that first inspect the input. They may not have fully
// comprehensive error checking, when that can be left to the validator; the
// benefit of the validate* methods is that they can share code between the
// text and binary format parsers, for handling certain situations in the
// input which preclude even creating valid IR, which the validator depends
// on.
class ValidatingBuilder : public Builder {
  size_t line = -1, col = -1;

public:
  ValidatingBuilder(Module& wasm, size_t line) : Builder(wasm), line(line) {}
  ValidatingBuilder(Module& wasm, size_t line, size_t col)
    : Builder(wasm), line(line), col(col) {}

  Expression* validateAndMakeBrOn(BrOnOp op,
                                  Name name,
                                  Expression* ref,
                                  Expression* rtt = nullptr) {
    if (op == BrOnCast) {
      if (rtt->type == Type::unreachable) {
        // An unreachable rtt is not supported: the text and binary formats do
        // not provide the type, so if it's unreachable we should not even
        // create a br_on_cast in such a case, as we'd have no idea what it
        // casts to.
        return makeSequence(makeDrop(ref), rtt);
      }
    }
    if (op == BrOnNull) {
      if (!ref->type.isRef() && ref->type != Type::unreachable) {
        throw ParseException("Invalid ref for br_on_null", line, col);
      }
    }
    return makeBrOn(op, name, ref, rtt);
  }

  template<typename T>
  Expression* validateAndMakeCallRef(Expression* target,
                                     const T& args,
                                     bool isReturn = false) {
    if (!target->type.isRef()) {
      if (target->type == Type::unreachable) {
        // An unreachable target is not supported. Similiar to br_on_cast, just
        // emit an unreachable sequence, since we don't have enough information
        // to create a full call_ref.
        auto* block = makeBlock(args);
        block->list.push_back(target);
        block->finalize(Type::unreachable);
        return block;
      }
      throw ParseException("Non-reference type for a call_ref", line, col);
    }
    auto heapType = target->type.getHeapType();
    if (!heapType.isSignature()) {
      throw ParseException("Invalid reference type for a call_ref", line, col);
    }
    return makeCallRef(target, args, heapType.getSignature().results, isReturn);
  }
};

} // namespace wasm

#endif // wasm_wasm_builder_h
