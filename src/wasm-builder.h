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
#include <optional>

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
                                                HeapType type,
                                                std::vector<Type>&& vars,
                                                Expression* body = nullptr) {
    assert(type.isSignature());
    auto func = std::make_unique<Function>();
    func->name = name;
    func->type = type;
    func->body = body;
    func->vars.swap(vars);
    return func;
  }

  static std::unique_ptr<Function> makeFunction(Name name,
                                                std::vector<NameType>&& params,
                                                HeapType type,
                                                std::vector<NameType>&& vars,
                                                Expression* body = nullptr) {
    assert(type.isSignature());
    auto func = std::make_unique<Function>();
    func->name = name;
    func->type = type;
    func->body = body;
    for (size_t i = 0; i < params.size(); ++i) {
      NameType& param = params[i];
      assert(func->getParams()[i] == param.type);
      Index index = func->localNames.size();
      func->localIndices[param.name] = index;
      func->localNames[index] = param.name;
    }
    for (auto& var : vars) {
      func->vars.push_back(var.type);
      Index index = func->localNames.size();
      func->localIndices[var.name] = index;
      func->localNames[index] = var.name;
    }
    return func;
  }

  static std::unique_ptr<Table> makeTable(Name name,
                                          Type type = Type(HeapType::func,
                                                           Nullable),
                                          Address initial = 0,
                                          Address max = Table::kMaxSize,
                                          Type addressType = Type::i32) {
    auto table = std::make_unique<Table>();
    table->name = name;
    table->type = type;
    table->addressType = addressType;
    table->initial = initial;
    table->max = max;
    return table;
  }

  static std::unique_ptr<ElementSegment>
  makeElementSegment(Name name,
                     Name table,
                     Expression* offset = nullptr,
                     Type type = Type(HeapType::func, Nullable)) {
    auto seg = std::make_unique<ElementSegment>();
    seg->name = name;
    seg->table = table;
    seg->offset = offset;
    seg->type = type;
    return seg;
  }

  static std::unique_ptr<Memory> makeMemory(Name name,
                                            Address initial = 0,
                                            Address max = Memory::kMaxSize32,
                                            bool shared = false,
                                            Type addressType = Type::i32) {
    auto memory = std::make_unique<Memory>();
    memory->name = name;
    memory->initial = initial;
    memory->max = max;
    memory->shared = shared;
    memory->addressType = addressType;
    return memory;
  }

  static std::unique_ptr<DataSegment>
  makeDataSegment(Name name = "",
                  Name memory = "",
                  bool isPassive = false,
                  Expression* offset = nullptr,
                  const char* init = "",
                  Address size = 0) {
    auto seg = std::make_unique<DataSegment>();
    seg->name = name;
    seg->memory = memory;
    seg->isPassive = isPassive;
    seg->offset = offset;
    seg->data.resize(size);
    std::copy_n(init, size, seg->data.begin());
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

  static std::unique_ptr<Tag> makeTag(Name name, Signature sig) {
    auto tag = std::make_unique<Tag>();
    tag->name = name;
    tag->sig = sig;
    return tag;
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

  template<typename T>
  using bool_if_not_expr_t =
    std::enable_if_t<std::negation_v<std::is_convertible<T, Expression*>>,
                     bool>;

  template<typename T, bool_if_not_expr_t<T> = true>
  Block* makeBlock(const T& items, std::optional<Type> type = std::nullopt) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->list.set(items);
    ret->finalize(type);
    return ret;
  }

  template<typename T, bool_if_not_expr_t<T> = true>
  Block* makeBlock(Name name,
                   const T& items,
                   std::optional<Type> type = std::nullopt) {
    auto* ret = wasm.allocator.alloc<Block>();
    ret->name = name;
    ret->list.set(items);
    ret->finalize(type);
    return ret;
  }
  Block* makeBlock(std::initializer_list<Expression*>&& items,
                   std::optional<Type> type = std::nullopt) {
    return makeBlock(items, type);
  }
  Block* makeBlock(Name name,
                   std::initializer_list<Expression*>&& items,
                   std::optional<Type> type = std::nullopt) {
    return makeBlock(name, items, type);
  }

  If* makeIf(Expression* condition,
             Expression* ifTrue,
             Expression* ifFalse = nullptr,
             std::optional<Type> type = std::nullopt) {
    auto* ret = wasm.allocator.alloc<If>();
    ret->condition = condition;
    ret->ifTrue = ifTrue;
    ret->ifFalse = ifFalse;
    ret->finalize(type);
    return ret;
  }
  Loop* makeLoop(Name name,
                 Expression* body,
                 std::optional<Type> type = std::nullopt) {
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
    call->finalize();
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
                                 HeapType heapType,
                                 bool isReturn = false) {
    assert(heapType.isSignature());
    auto* call = wasm.allocator.alloc<CallIndirect>();
    call->table = table;
    call->heapType = heapType;
    call->type = heapType.getSignature().results;
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
                 Address offset,
                 unsigned align,
                 Expression* ptr,
                 Type type,
                 Name memory) {
    auto* ret = wasm.allocator.alloc<Load>();
    ret->isAtomic = false;
    ret->bytes = bytes;
    ret->signed_ = signed_;
    ret->offset = offset;
    ret->align = align;
    ret->ptr = ptr;
    ret->type = type;
    ret->memory = memory;
    ret->finalize();
    return ret;
  }
  Load* makeAtomicLoad(
    unsigned bytes, Address offset, Expression* ptr, Type type, Name memory) {
    Load* load = makeLoad(bytes, false, offset, bytes, ptr, type, memory);
    load->isAtomic = true;
    return load;
  }
  AtomicWait* makeAtomicWait(Expression* ptr,
                             Expression* expected,
                             Expression* timeout,
                             Type expectedType,
                             Address offset,
                             Name memory) {
    auto* wait = wasm.allocator.alloc<AtomicWait>();
    wait->offset = offset;
    wait->ptr = ptr;
    wait->expected = expected;
    wait->timeout = timeout;
    wait->expectedType = expectedType;
    wait->finalize();
    wait->memory = memory;
    return wait;
  }
  AtomicNotify* makeAtomicNotify(Expression* ptr,
                                 Expression* notifyCount,
                                 Address offset,
                                 Name memory) {
    auto* notify = wasm.allocator.alloc<AtomicNotify>();
    notify->offset = offset;
    notify->ptr = ptr;
    notify->notifyCount = notifyCount;
    notify->finalize();
    notify->memory = memory;
    return notify;
  }
  AtomicFence* makeAtomicFence() { return wasm.allocator.alloc<AtomicFence>(); }
  Store* makeStore(unsigned bytes,
                   Address offset,
                   unsigned align,
                   Expression* ptr,
                   Expression* value,
                   Type type,
                   Name memory) {
    auto* ret = wasm.allocator.alloc<Store>();
    ret->isAtomic = false;
    ret->bytes = bytes;
    ret->offset = offset;
    ret->align = align;
    ret->ptr = ptr;
    ret->value = value;
    ret->valueType = type;
    ret->memory = memory;
    ret->finalize();
    return ret;
  }
  Store* makeAtomicStore(unsigned bytes,
                         Address offset,
                         Expression* ptr,
                         Expression* value,
                         Type type,
                         Name memory) {
    Store* store = makeStore(bytes, offset, bytes, ptr, value, type, memory);
    store->isAtomic = true;
    return store;
  }
  AtomicRMW* makeAtomicRMW(AtomicRMWOp op,
                           unsigned bytes,
                           Address offset,
                           Expression* ptr,
                           Expression* value,
                           Type type,
                           Name memory) {
    auto* ret = wasm.allocator.alloc<AtomicRMW>();
    ret->op = op;
    ret->bytes = bytes;
    ret->offset = offset;
    ret->ptr = ptr;
    ret->value = value;
    ret->type = type;
    ret->finalize();
    ret->memory = memory;
    return ret;
  }
  AtomicCmpxchg* makeAtomicCmpxchg(unsigned bytes,
                                   Address offset,
                                   Expression* ptr,
                                   Expression* expected,
                                   Expression* replacement,
                                   Type type,
                                   Name memory) {
    auto* ret = wasm.allocator.alloc<AtomicCmpxchg>();
    ret->bytes = bytes;
    ret->offset = offset;
    ret->ptr = ptr;
    ret->expected = expected;
    ret->replacement = replacement;
    ret->type = type;
    ret->finalize();
    ret->memory = memory;
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
  SIMDLoad* makeSIMDLoad(SIMDLoadOp op,
                         Address offset,
                         Address align,
                         Expression* ptr,
                         Name memory) {
    auto* ret = wasm.allocator.alloc<SIMDLoad>();
    ret->op = op;
    ret->offset = offset;
    ret->align = align;
    ret->ptr = ptr;
    ret->memory = memory;
    ret->finalize();
    return ret;
  }
  SIMDLoadStoreLane* makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp op,
                                           Address offset,
                                           Address align,
                                           uint8_t index,
                                           Expression* ptr,
                                           Expression* vec,
                                           Name memory) {
    auto* ret = wasm.allocator.alloc<SIMDLoadStoreLane>();
    ret->op = op;
    ret->offset = offset;
    ret->align = align;
    ret->index = index;
    ret->ptr = ptr;
    ret->vec = vec;
    ret->finalize();
    ret->memory = memory;
    return ret;
  }
  MemoryInit* makeMemoryInit(Name segment,
                             Expression* dest,
                             Expression* offset,
                             Expression* size,
                             Name memory) {
    auto* ret = wasm.allocator.alloc<MemoryInit>();
    ret->segment = segment;
    ret->dest = dest;
    ret->offset = offset;
    ret->size = size;
    ret->memory = memory;
    ret->finalize();
    return ret;
  }
  DataDrop* makeDataDrop(Name segment) {
    auto* ret = wasm.allocator.alloc<DataDrop>();
    ret->segment = segment;
    ret->finalize();
    return ret;
  }
  MemoryCopy* makeMemoryCopy(Expression* dest,
                             Expression* source,
                             Expression* size,
                             Name destMemory,
                             Name sourceMemory) {
    auto* ret = wasm.allocator.alloc<MemoryCopy>();
    ret->dest = dest;
    ret->source = source;
    ret->size = size;
    ret->destMemory = destMemory;
    ret->sourceMemory = sourceMemory;
    ret->finalize();
    return ret;
  }
  MemoryFill* makeMemoryFill(Expression* dest,
                             Expression* value,
                             Expression* size,
                             Name memory) {
    auto* ret = wasm.allocator.alloc<MemoryFill>();
    ret->dest = dest;
    ret->value = value;
    ret->size = size;
    ret->memory = memory;
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
  Const* makeConstPtr(uint64_t val, Type addressType) {
    return makeConst(Literal::makeFromInt64(val, addressType));
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
  Return* makeReturn(Expression* value = nullptr) {
    auto* ret = wasm.allocator.alloc<Return>();
    ret->value = value;
    return ret;
  }

  // Some APIs can be told if the memory is 32 or 64-bit. If they are not
  // informed of that, then the memory they refer to is looked up and that
  // information fetched from there.
  enum MemoryInfo { Memory32, Memory64, Unspecified };

  bool isMemory64(Name memoryName, MemoryInfo info) {
    return info == MemoryInfo::Memory64 || (info == MemoryInfo::Unspecified &&
                                            wasm.getMemory(memoryName)->is64());
  }

  bool isTable64(Name tableName) { return wasm.getTable(tableName)->is64(); }

  MemorySize* makeMemorySize(Name memoryName,
                             MemoryInfo info = MemoryInfo::Unspecified) {
    auto* ret = wasm.allocator.alloc<MemorySize>();
    if (isMemory64(memoryName, info)) {
      ret->type = Type::i64;
    }
    ret->memory = memoryName;
    ret->finalize();
    return ret;
  }
  MemoryGrow* makeMemoryGrow(Expression* delta,
                             Name memoryName,
                             MemoryInfo info = MemoryInfo::Unspecified) {
    auto* ret = wasm.allocator.alloc<MemoryGrow>();
    if (isMemory64(memoryName, info)) {
      ret->type = Type::i64;
    }
    ret->delta = delta;
    ret->memory = memoryName;
    ret->finalize();
    return ret;
  }
  RefNull* makeRefNull(HeapType type) {
    auto* ret = wasm.allocator.alloc<RefNull>();
    ret->finalize(Type(type.getBottom(), Nullable));
    return ret;
  }
  RefNull* makeRefNull(Type type) {
    assert(type.isNullable() && type.isNull());
    auto* ret = wasm.allocator.alloc<RefNull>();
    ret->finalize(type);
    return ret;
  }
  RefIsNull* makeRefIsNull(Expression* value) {
    auto* ret = wasm.allocator.alloc<RefIsNull>();
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
  TableGet* makeTableGet(Name table, Expression* index, Type type) {
    auto* ret = wasm.allocator.alloc<TableGet>();
    ret->table = table;
    ret->index = index;
    ret->type = type;
    ret->finalize();
    return ret;
  }
  TableSet* makeTableSet(Name table, Expression* index, Expression* value) {
    auto* ret = wasm.allocator.alloc<TableSet>();
    ret->table = table;
    ret->index = index;
    ret->value = value;
    ret->finalize();
    return ret;
  }
  TableSize* makeTableSize(Name table) {
    auto* ret = wasm.allocator.alloc<TableSize>();
    ret->table = table;
    if (isTable64(table)) {
      ret->type = Type::i64;
    }
    ret->finalize();
    return ret;
  }
  TableGrow* makeTableGrow(Name table, Expression* value, Expression* delta) {
    auto* ret = wasm.allocator.alloc<TableGrow>();
    ret->table = table;
    ret->value = value;
    ret->delta = delta;
    if (isTable64(table)) {
      ret->type = Type::i64;
    }
    ret->finalize();
    return ret;
  }
  TableFill* makeTableFill(Name table,
                           Expression* dest,
                           Expression* value,
                           Expression* size) {
    auto* ret = wasm.allocator.alloc<TableFill>();
    ret->table = table;
    ret->dest = dest;
    ret->value = value;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  TableCopy* makeTableCopy(Expression* dest,
                           Expression* source,
                           Expression* size,
                           Name destTable,
                           Name sourceTable) {
    auto* ret = wasm.allocator.alloc<TableCopy>();
    ret->dest = dest;
    ret->source = source;
    ret->size = size;
    ret->destTable = destTable;
    ret->sourceTable = sourceTable;
    ret->finalize();
    return ret;
  }
  TableInit* makeTableInit(Name segment,
                           Expression* dest,
                           Expression* offset,
                           Expression* size,
                           Name table) {
    auto* ret = wasm.allocator.alloc<TableInit>();
    ret->segment = segment;
    ret->dest = dest;
    ret->offset = offset;
    ret->size = size;
    ret->table = table;
    ret->finalize();
    return ret;
  }

private:
  Try* makeTry(Name name,
               Expression* body,
               const std::vector<Name>& catchTags,
               const std::vector<Expression*>& catchBodies,
               Name delegateTarget,
               std::optional<Type> type = std::nullopt) {
    auto* ret = wasm.allocator.alloc<Try>();
    ret->name = name;
    ret->body = body;
    ret->catchTags.set(catchTags);
    ret->catchBodies.set(catchBodies);
    ret->finalize(type);
    return ret;
  }

public:
  // TODO delete?
  Try* makeTry(Expression* body,
               const std::vector<Name>& catchTags,
               const std::vector<Expression*>& catchBodies,
               std::optional<Type> type = std::nullopt) {
    return makeTry(Name(), body, catchTags, catchBodies, Name(), type);
  }
  Try* makeTry(Name name,
               Expression* body,
               const std::vector<Name>& catchTags,
               const std::vector<Expression*>& catchBodies,
               std::optional<Type> type = std::nullopt) {
    return makeTry(name, body, catchTags, catchBodies, Name(), type);
  }
  Try* makeTry(Expression* body,
               Name delegateTarget,
               std::optional<Type> type = std::nullopt) {
    return makeTry(Name(), body, {}, {}, delegateTarget, type);
  }
  Try* makeTry(Name name,
               Expression* body,
               Name delegateTarget,
               std::optional<Type> type = std::nullopt) {
    return makeTry(name, body, {}, {}, delegateTarget, type);
  }
  TryTable* makeTryTable(Expression* body,
                         const std::vector<Name>& catchTags,
                         const std::vector<Name>& catchDests,
                         const std::vector<bool>& catchRefs,
                         std::optional<Type> type = std::nullopt) {
    auto* ret = wasm.allocator.alloc<TryTable>();
    ret->body = body;
    ret->catchTags.set(catchTags);
    ret->catchDests.set(catchDests);
    ret->catchRefs.set(catchRefs);
    ret->finalize(type, &wasm);
    return ret;
  }
  Throw* makeThrow(Tag* tag, const std::vector<Expression*>& args) {
    return makeThrow(tag->name, args);
  }
  template<typename T> Throw* makeThrow(Name tag, const T& args) {
    auto* ret = wasm.allocator.alloc<Throw>();
    ret->tag = tag;
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
  ThrowRef* makeThrowRef(Expression* exnref) {
    auto* ret = wasm.allocator.alloc<ThrowRef>();
    ret->exnref = exnref;
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
  RefI31* makeRefI31(Expression* value, Shareability share = Unshared) {
    auto* ret = wasm.allocator.alloc<RefI31>();
    ret->value = value;
    ret->type = Type(HeapTypes::i31.getBasic(share), NonNullable);
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
  RefTest* makeRefTest(Expression* ref, Type castType) {
    auto* ret = wasm.allocator.alloc<RefTest>();
    ret->ref = ref;
    ret->castType = castType;
    ret->finalize();
    return ret;
  }
  RefCast* makeRefCast(Expression* ref, Type type) {
    auto* ret = wasm.allocator.alloc<RefCast>();
    ret->ref = ref;
    ret->type = type;
    ret->finalize();
    return ret;
  }
  BrOn*
  makeBrOn(BrOnOp op, Name name, Expression* ref, Type castType = Type::none) {
    auto* ret = wasm.allocator.alloc<BrOn>();
    ret->op = op;
    ret->name = name;
    ret->ref = ref;
    ret->castType = castType;
    ret->finalize();
    return ret;
  }
  StructNew* makeStructNew(HeapType type,
                           std::initializer_list<Expression*> args) {
    auto* ret = wasm.allocator.alloc<StructNew>();
    ret->operands.set(args);
    ret->type = Type(type, NonNullable);
    ret->finalize();
    return ret;
  }
  StructNew* makeStructNew(HeapType type, ExpressionList&& args) {
    auto* ret = wasm.allocator.alloc<StructNew>();
    ret->operands = std::move(args);
    ret->type = Type(type, NonNullable);
    ret->finalize();
    return ret;
  }
  template<typename T> StructNew* makeStructNew(HeapType type, const T& args) {
    auto* ret = wasm.allocator.alloc<StructNew>();
    ret->operands.set(args);
    ret->type = Type(type, NonNullable);
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
  makeArrayNew(HeapType type, Expression* size, Expression* init = nullptr) {
    auto* ret = wasm.allocator.alloc<ArrayNew>();
    ret->size = size;
    ret->init = init;
    ret->type = Type(type, NonNullable);
    ret->finalize();
    return ret;
  }
  ArrayNewData* makeArrayNewData(HeapType type,
                                 Name seg,
                                 Expression* offset,
                                 Expression* size) {
    auto* ret = wasm.allocator.alloc<ArrayNewData>();
    ret->segment = seg;
    ret->offset = offset;
    ret->size = size;
    ret->type = Type(type, NonNullable);
    ret->finalize();
    return ret;
  }
  ArrayNewElem* makeArrayNewElem(HeapType type,
                                 Name seg,
                                 Expression* offset,
                                 Expression* size) {
    auto* ret = wasm.allocator.alloc<ArrayNewElem>();
    ret->segment = seg;
    ret->offset = offset;
    ret->size = size;
    ret->type = Type(type, NonNullable);
    ret->finalize();
    return ret;
  }
  template<typename T>
  ArrayNewFixed* makeArrayNewFixed(HeapType type, const T& values) {
    auto* ret = wasm.allocator.alloc<ArrayNewFixed>();
    ret->values.set(values);
    ret->type = Type(type, NonNullable);
    ret->finalize();
    return ret;
  }
  ArrayNewFixed*
  makeArrayNewFixed(HeapType type,
                    std::initializer_list<Expression*>&& values) {
    return makeArrayNewFixed(type, values);
  }
  ArrayGet* makeArrayGet(Expression* ref,
                         Expression* index,
                         Type type,
                         bool signed_ = false) {
    auto* ret = wasm.allocator.alloc<ArrayGet>();
    ret->ref = ref;
    ret->index = index;
    ret->type = type;
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
  ArrayFill* makeArrayFill(Expression* ref,
                           Expression* index,
                           Expression* value,
                           Expression* size) {
    auto* ret = wasm.allocator.alloc<ArrayFill>();
    ret->ref = ref;
    ret->index = index;
    ret->value = value;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  ArrayInitData* makeArrayInitData(Name seg,
                                   Expression* ref,
                                   Expression* index,
                                   Expression* offset,
                                   Expression* size) {
    auto* ret = wasm.allocator.alloc<ArrayInitData>();
    ret->segment = seg;
    ret->ref = ref;
    ret->index = index;
    ret->offset = offset;
    ret->size = size;
    ret->finalize();
    return ret;
  }
  ArrayInitElem* makeArrayInitElem(Name seg,
                                   Expression* ref,
                                   Expression* index,
                                   Expression* offset,
                                   Expression* size) {
    auto* ret = wasm.allocator.alloc<ArrayInitElem>();
    ret->segment = seg;
    ret->ref = ref;
    ret->index = index;
    ret->offset = offset;
    ret->size = size;
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
  StringNew* makeStringNew(StringNewOp op,
                           Expression* ref,
                           Expression* start = nullptr,
                           Expression* end = nullptr) {
    assert((start && end) != (op == StringNewFromCodePoint));
    auto* ret = wasm.allocator.alloc<StringNew>();
    ret->op = op;
    ret->ref = ref;
    ret->start = start;
    ret->end = end;
    ret->finalize();
    return ret;
  }
  StringConst* makeStringConst(Name string) {
    auto* ret = wasm.allocator.alloc<StringConst>();
    ret->string = string;
    ret->finalize();
    return ret;
  }
  StringMeasure* makeStringMeasure(StringMeasureOp op, Expression* ref) {
    auto* ret = wasm.allocator.alloc<StringMeasure>();
    ret->op = op;
    ret->ref = ref;
    ret->finalize();
    return ret;
  }
  StringEncode* makeStringEncode(StringEncodeOp op,
                                 Expression* str,
                                 Expression* array,
                                 Expression* start = nullptr) {
    auto* ret = wasm.allocator.alloc<StringEncode>();
    ret->op = op;
    ret->str = str;
    ret->array = array;
    ret->start = start;
    ret->finalize();
    return ret;
  }
  StringConcat* makeStringConcat(Expression* left, Expression* right) {
    auto* ret = wasm.allocator.alloc<StringConcat>();
    ret->left = left;
    ret->right = right;
    ret->finalize();
    return ret;
  }
  StringEq* makeStringEq(StringEqOp op, Expression* left, Expression* right) {
    auto* ret = wasm.allocator.alloc<StringEq>();
    ret->op = op;
    ret->left = left;
    ret->right = right;
    ret->finalize();
    return ret;
  }
  StringWTF16Get* makeStringWTF16Get(Expression* ref, Expression* pos) {
    auto* ret = wasm.allocator.alloc<StringWTF16Get>();
    ret->ref = ref;
    ret->pos = pos;
    ret->finalize();
    return ret;
  }
  StringSliceWTF*
  makeStringSliceWTF(Expression* ref, Expression* start, Expression* end) {
    auto* ret = wasm.allocator.alloc<StringSliceWTF>();
    ret->ref = ref;
    ret->start = start;
    ret->end = end;
    ret->finalize();
    return ret;
  }
  ContNew* makeContNew(HeapType type, Expression* func) {
    auto* ret = wasm.allocator.alloc<ContNew>();
    ret->type = Type(type, NonNullable);
    ret->func = func;
    ret->finalize();
    return ret;
  }
  ContBind* makeContBind(HeapType targetType,
                         ExpressionList&& operands,
                         Expression* cont) {
    auto* ret = wasm.allocator.alloc<ContBind>();
    ret->type = Type(targetType, NonNullable);
    ret->operands = std::move(operands);
    ret->cont = cont;
    ret->finalize();
    return ret;
  }
  Suspend* makeSuspend(Name tag, const std::vector<Expression*>& args) {
    auto* ret = wasm.allocator.alloc<Suspend>();
    ret->tag = tag;
    ret->operands.set(args);
    ret->finalize(&wasm);
    return ret;
  }
  Resume* makeResume(const std::vector<Name>& handlerTags,
                     const std::vector<Name>& handlerBlocks,
                     const std::vector<Type>& sentTypes,
                     ExpressionList&& operands,
                     Expression* cont) {
    auto* ret = wasm.allocator.alloc<Resume>();
    ret->handlerTags.set(handlerTags);
    ret->handlerBlocks.set(handlerBlocks);
    ret->sentTypes.set(sentTypes);
    ret->operands = std::move(operands);
    ret->cont = cont;
    ret->finalize();
    return ret;
  }
  ResumeThrow* makeResumeThrow(Name tag,
                               const std::vector<Name>& handlerTags,
                               const std::vector<Name>& handlerBlocks,
                               const std::vector<Type>& sentTypes,
                               ExpressionList&& operands,
                               Expression* cont) {
    auto* ret = wasm.allocator.alloc<ResumeThrow>();
    ret->tag = tag;
    ret->handlerTags.set(handlerTags);
    ret->handlerBlocks.set(handlerBlocks);
    ret->sentTypes.set(sentTypes);
    ret->operands = std::move(operands);
    ret->cont = cont;
    ret->finalize();
    return ret;
  }
  StackSwitch*
  makeStackSwitch(Name tag, ExpressionList&& operands, Expression* cont) {
    auto* ret = wasm.allocator.alloc<StackSwitch>();
    ret->tag = tag;
    ret->operands = std::move(operands);
    ret->cont = cont;
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
    if (type.isRef() && type.getHeapType().isMaybeShared(HeapType::i31)) {
      return makeRefI31(makeConst(value.geti31()),
                        type.getHeapType().getShared());
    }
    if (type.isString()) {
      // The string is already WTF-16, but we need to convert from `Literals` to
      // actual string.
      std::stringstream wtf16;
      for (auto c : value.getGCData()->values) {
        auto u = c.getInteger();
        assert(u < 0x10000);
        wtf16 << uint8_t(u & 0xFF);
        wtf16 << uint8_t(u >> 8);
      }
      // TODO: Use wtf16.view() once we have C++20.
      return makeStringConst(wtf16.str());
    }
    if (type.isRef() && type.getHeapType().isMaybeShared(HeapType::ext)) {
      return makeRefAs(ExternConvertAny,
                       makeConstantExpression(value.internalize()));
    }
    TODO_SINGLE_COMPOUND(type);
    WASM_UNREACHABLE("unsupported constant expression");
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

  // Additional utility functions for building on top of nodes
  // Convenient to have these on Builder, as it has allocation built in

  static Index addParam(Function* func, Name name, Type type) {
    // only ok to add a param if no vars, otherwise indices are invalidated
    assert(func->localIndices.size() == func->getParams().size());
    assert(name.is());
    Signature sig = func->getSig();
    std::vector<Type> params(sig.params.begin(), sig.params.end());
    params.push_back(type);
    func->type = Signature(Type(params), sig.results);
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
  Block* blockifyWithName(Expression* any,
                          Name name,
                          Expression* append = nullptr,
                          std::optional<Type> type = std::nullopt) {
    Block* block = nullptr;
    if (any) {
      block = any->dynCast<Block>();
    }
    if (!block || block->name.is()) {
      block = makeBlock(name, any);
    } else {
      block->name = name;
    }
    if (append) {
      block->list.push_back(append);
    }
    if (append || type) {
      block->finalize(type);
    }
    return block;
  }

  // a helper for the common pattern of a sequence of two expressions. Similar
  // to blockify, but does *not* reuse a block if the first is one.
  Block* makeSequence(Expression* left,
                      Expression* right,
                      std::optional<Type> type = std::nullopt) {
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
    auto type = curr->type;
    // Anything that would otherwise have a more refined type than the original
    // expression needs to be wrapped in a block with the original type.
    auto maybeWrap = [&](Expression* expr) -> Expression* {
      return expr->type == type ? expr : makeBlock({expr}, type);
    };
    if (curr->type.isTuple() && curr->type.isDefaultable()) {
      return maybeWrap(makeConstantExpression(Literal::makeZeros(curr->type)));
    }
    if (curr->type.isNullable()) {
      return maybeWrap(ExpressionManipulator::refNull(
        curr, Type(curr->type.getHeapType().getBottom(), Nullable)));
    }
    if (curr->type.isRef() &&
        curr->type.getHeapType().isMaybeShared(HeapType::i31)) {
      return maybeWrap(
        makeRefI31(makeConst(0), curr->type.getHeapType().getShared()));
    }
    if (!curr->type.isBasic()) {
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
      case Type::none:
        return ExpressionManipulator::nop(curr);
      case Type::unreachable:
        return ExpressionManipulator::unreachable(curr);
    }
    return makeConst(value);
  }
};

} // namespace wasm

#endif // wasm_wasm_builder_h
