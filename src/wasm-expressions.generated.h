/*
 * Copyright 2020 WebAssembly Community Group participants
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

//=============================================================================
// This is an AUTOGENERATED file, even though it looks human-readable! Do not
// edit it by hand, instead edit what it is generated from. You can and should
// treat it like human-written code in all other ways, though, like reviewing
// it in a PR, etc.
//=============================================================================
class Nop : public SpecificExpression<Expression::NopId> {
  Nop() {}
  Nop(MixedArena& allocator) : Nop() {}
  void finalize();
};
class Block : public SpecificExpression<Expression::BlockId> {
  Block(MixedArena& allocator) : list(allocator) {}
  Name name;
  ExpressionList list;
  void finalize();
  void finalize(Type type_);
  void finalize(Type type_, bool hasBreak);
};
class If : public SpecificExpression<Expression::IfId> {
  If() {}
  If(MixedArena& allocator) : If() {}
  Expression* condition;
  Expression* ifTrue;
  Expression* ifFalse = nullptr;
  void finalize(Type type_);
  void finalize();
};
class Loop : public SpecificExpression<Expression::LoopId> {
  Loop() {}
  Loop(MixedArena& allocator) : Loop() {}
  Name name;
  Expression* body;
  void finalize(Type type_);
  void finalize();
};
class Break : public SpecificExpression<Expression::BreakId> {
  Break() { type = Type::unreachable; }
  Break(MixedArena& allocator) : Break() {}
  Name name;
  Expression* value = nullptr;
  Expression* condition = nullptr;
  void finalize();
};
class Switch : public SpecificExpression<Expression::SwitchId> {
  Switch(MixedArena& allocator) : targets(allocator) { type = Type::unreachable; }
  ArenaVector<Name> targets;
  Name default_;
  Expression* condition;
  Expression* value = nullptr;
  void finalize();
};
class Call : public SpecificExpression<Expression::CallId> {
  Call(MixedArena& allocator) : operands(allocator) {}
  ExpressionList operands;
  Name target;
  bool isReturn;
  void finalize();
};
class CallIndirect : public SpecificExpression<Expression::CallIndirectId> {
  CallIndirect(MixedArena& allocator) : operands(allocator) {}
  Signature sig;
  ExpressionList operands;
  Expression* target;
  bool isReturn;
  void finalize();
};
class LocalGet : public SpecificExpression<Expression::LocalGetId> {
  LocalGet() {}
  LocalGet(MixedArena& allocator) : LocalGet() {}
  Index index;
  void finalize();
};
class LocalSet : public SpecificExpression<Expression::LocalSetId> {
  LocalSet() {}
  LocalSet(MixedArena& allocator) : LocalSet() {}
  Index index;
  Expression* value;
  bool isTee() const;
  void makeTee(Type type);
  void makeSet();
  void finalize();
};
class GlobalGet : public SpecificExpression<Expression::GlobalGetId> {
  GlobalGet() {}
  GlobalGet(MixedArena& allocator) : GlobalGet() {}
  Name name;
  void finalize();
};
class GlobalSet : public SpecificExpression<Expression::GlobalSetId> {
  GlobalSet() {}
  GlobalSet(MixedArena& allocator) : GlobalSet() {}
  Name name;
  Expression* value;
  void finalize();
};
class Load : public SpecificExpression<Expression::LoadId> {
  Load() {}
  Load(MixedArena& allocator) : Load() {}
  uint8_t bytes;
  bool signed_;
  Address offset;
  Address align;
  bool isAtomic;
  Expression* ptr;
  void finalize();
};
class Store : public SpecificExpression<Expression::StoreId> {
  Store() {}
  Store(MixedArena& allocator) : Store() {}
  uint8_t bytes;
  Address offset;
  Address align;
  bool isAtomic;
  Expression* ptr;
  Expression* value;
  Type valueType;
  void finalize();
};
class AtomicRMW : public SpecificExpression<Expression::AtomicRMWId> {
  AtomicRMW() {}
  AtomicRMW(MixedArena& allocator) : AtomicRMW() {}
  AtomicRMWOp op;
  uint8_t bytes;
  Address offset;
  Expression* ptr;
  Expression* value;
  void finalize();
};
class AtomicCmpxchg : public SpecificExpression<Expression::AtomicCmpxchgId> {
  AtomicCmpxchg() {}
  AtomicCmpxchg(MixedArena& allocator) : AtomicCmpxchg() {}
  uint8_t bytes;
  Address offset;
  Expression* ptr;
  Expression* expected;
  Expression* replacement;
  void finalize();
};
class AtomicWait : public SpecificExpression<Expression::AtomicWaitId> {
  AtomicWait() {}
  AtomicWait(MixedArena& allocator) : AtomicWait() {}
  Address offset;
  Expression* ptr;
  Expression* expected;
  Expression* wait;
  Type expectedType;
  void finalize();
};
class AtomicNotify : public SpecificExpression<Expression::AtomicNotifyId> {
  AtomicNotify() {}
  AtomicNotify(MixedArena& allocator) : AtomicNotify() {}
  Address offset;
  Expression* ptr;
  Expression* notifyCount;
  void finalize();
};
class AtomicFence : public SpecificExpression<Expression::AtomicFenceId> {
  AtomicFence() {}
  AtomicFence(MixedArena& allocator) : AtomicFence() {}
  uint8_t order;
  void finalize();
};
class SIMDExtract : public SpecificExpression<Expression::SIMDExtractId> {
  SIMDExtract() {}
  SIMDExtract(MixedArena& allocator) : SIMDExtract() {}
  Expression* vec;
  uint8_t index;
  void finalize();
};
class SIMDReplace : public SpecificExpression<Expression::SIMDReplaceId> {
  SIMDReplace() {}
  SIMDReplace(MixedArena& allocator) : SIMDReplace() {}
  Expression* vec;
  uint8_t index;
  Expression* value;
  void finalize();
};
class SIMDShuffle : public SpecificExpression<Expression::SIMDShuffleId> {
  SIMDShuffle() {}
  SIMDShuffle(MixedArena& allocator) : SIMDShuffle() {}
  Expression* left;
  Expression* right;
  void finalize();
};
class SIMDTernary : public SpecificExpression<Expression::SIMDTernaryId> {
  SIMDTernary() {}
  SIMDTernary(MixedArena& allocator) : SIMDTernary() {}
  Expression* a;
  Expression* b;
  Expression* c;
  void finalize();
};
class SIMDShift : public SpecificExpression<Expression::SIMDShiftId> {
  SIMDShift() {}
  SIMDShift(MixedArena& allocator) : SIMDShift() {}
  Expression* vec;
  Expression* shift;
  void finalize();
};
class SIMDLoad : public SpecificExpression<Expression::SIMDLoadId> {
  SIMDLoad() {}
  SIMDLoad(MixedArena& allocator) : SIMDLoad() {}
  Address offset;
  Address align;
  Expression* ptr;
  Index getMemBytes();
  void finalize();
};
class MemoryInit : public SpecificExpression<Expression::MemoryInitId> {
  MemoryInit() {}
  MemoryInit(MixedArena& allocator) : MemoryInit() {}
  Index segment;
  Expression* dest;
  Expression* offset;
  Expression* size;
  void finalize();
};
class DataDrop : public SpecificExpression<Expression::DataDropId> {
  DataDrop() {}
  DataDrop(MixedArena& allocator) : DataDrop() {}
  Index segment;
  void finalize();
};
class MemoryCopy : public SpecificExpression<Expression::MemoryCopyId> {
  MemoryCopy() {}
  MemoryCopy(MixedArena& allocator) : MemoryCopy() {}
  Expression* dest;
  Expression* source;
  Expression* size;
  void finalize();
};
class MemoryFill : public SpecificExpression<Expression::MemoryFillId> {
  MemoryFill() {}
  MemoryFill(MixedArena& allocator) : MemoryFill() {}
  Expression* dest;
  Expression* value;
  Expression* size;
  void finalize();
};
class Const : public SpecificExpression<Expression::ConstId> {
  Const() {}
  Const(MixedArena& allocator) : Const() {}
  Const* set(Literal value_);
  void finalize();
};
class Unary : public SpecificExpression<Expression::UnaryId> {
  Unary() {}
  Unary(MixedArena& allocator) : Unary() {}
  Expression* value;
  bool isRelational();
  void finalize();
};
class Binary : public SpecificExpression<Expression::BinaryId> {
  Binary() {}
  Binary(MixedArena& allocator) : Binary() {}
  Expression* left;
  Expression* right;
  bool isRelational();
  void finalize();
};
class Select : public SpecificExpression<Expression::SelectId> {
  Select() {}
  Select(MixedArena& allocator) : Select() {}
  Expression* ifTrue;
  Expression* ifFalse;
  Expression* condition;
  void finalize();
  void finalize(Type type_);
};
class Drop : public SpecificExpression<Expression::DropId> {
  Drop() {}
  Drop(MixedArena& allocator) : Drop() {}
  Expression* value;
  void finalize();
};
class Return : public SpecificExpression<Expression::ReturnId> {
  Return() {}
  Return(MixedArena& allocator) : Return() {}
  Expression* value = nullptr;
  void finalize();
};
class MemorySize : public SpecificExpression<Expression::MemorySizeId> {
  MemorySize() { type = Type::i32; }
  MemorySize(MixedArena& allocator) : MemorySize() {}
  Type ptrType = Type::i32;
  void make64();
  void finalize();
};
class MemoryGrow : public SpecificExpression<Expression::MemoryGrowId> {
  MemoryGrow() {}
  MemoryGrow(MixedArena& allocator) : MemoryGrow() {}
  Expression* delta = nullptr;
  Type ptrType = Type::i32;
  void make64();
  void finalize();
};
class Unreachable : public SpecificExpression<Expression::UnreachableId> {
  Unreachable() { type = Type::unreachable }
  Unreachable(MixedArena& allocator) : Unreachable() {}
  void finalize();
};
class Pop : public SpecificExpression<Expression::PopId> {
  Pop() {}
  Pop(MixedArena& allocator) : Pop() {}
  void finalize();
};
class RefNull : public SpecificExpression<Expression::RefNullId> {
  RefNull() {}
  RefNull(MixedArena& allocator) : RefNull() {}
  void finalize();
  void finalize(HeapType heapType);
  void finalize(Type type);
};
class RefIsNull : public SpecificExpression<Expression::RefIsNullId> {
  RefIsNull() {}
  RefIsNull(MixedArena& allocator) : RefIsNull() {}
  Expression* value;
  void finalize();
};
class RefFunc : public SpecificExpression<Expression::RefFuncId> {
  RefFunc() {}
  RefFunc(MixedArena& allocator) : RefFunc() {}
  Name func;
  void finalize();
};
class RefEq : public SpecificExpression<Expression::RefEqId> {
  RefEq() {}
  RefEq(MixedArena& allocator) : RefEq() {}
  Expression* left;
  Expression* right;
  void finalize();
};
class Try : public SpecificExpression<Expression::TryId> {
  Try() {}
  Try(MixedArena& allocator) : Try() {}
  Expression* body;
  Expression* catchBody;
  void finalize();
  void finalize(Type type_);
};
class Throw : public SpecificExpression<Expression::ThrowId> {
  Throw(MixedArena& allocator) : operands(allocator) {}
  Name event;
  ExpressionList operands;
  void finalize();
};
class Rethrow : public SpecificExpression<Expression::RethrowId> {
  Rethrow() {}
  Rethrow(MixedArena& allocator) : Rethrow() {}
  Expression* exnref;
  void finalize();
};
class BrOnExn : public SpecificExpression<Expression::BrOnExnId> {
  BrOnExn() { type = Type::unreachable; }
  BrOnExn(MixedArena& allocator) : BrOnExn() {}
  Name name;
  Name event;
  Expression* exnref;
  Type send;
  void finalize();
};
class TupleMake : public SpecificExpression<Expression::TupleMakeId> {
  TupleMake(MixedArena& allocator) : operands(allocator) {}
  ExpressionList operands;
  void finalize();
};
class TupleExtract : public SpecificExpression<Expression::TupleExtractId> {
  TupleExtract() {}
  TupleExtract(MixedArena& allocator) : TupleExtract() {}
  Expression* tuple;
  Index index;
  void finalize();
};
class I31New : public SpecificExpression<Expression::I31NewId> {
  I31New() {}
  I31New(MixedArena& allocator) : I31New() {}
  Expression* value;
  void finalize();
};
class I31Get : public SpecificExpression<Expression::I31GetId> {
  I31Get() {}
  I31Get(MixedArena& allocator) : I31Get() {}
  Expression* i31;
  bool signed_;
  void finalize();
};
class RefTest : public SpecificExpression<Expression::RefTestId> {
  RefTest() {}
  RefTest(MixedArena& allocator) : RefTest() {}
  void finalize();
};
class RefCast : public SpecificExpression<Expression::RefCastId> {
  RefCast() {}
  RefCast(MixedArena& allocator) : RefCast() {}
  void finalize();
};
class BrOnCast : public SpecificExpression<Expression::BrOnCastId> {
  BrOnCast() {}
  BrOnCast(MixedArena& allocator) : BrOnCast() {}
  void finalize();
};
class RttCanon : public SpecificExpression<Expression::RttCanonId> {
  RttCanon() {}
  RttCanon(MixedArena& allocator) : RttCanon() {}
  void finalize();
};
class RttSub : public SpecificExpression<Expression::RttSubId> {
  RttSub() {}
  RttSub(MixedArena& allocator) : RttSub() {}
  void finalize();
};
class StructNew : public SpecificExpression<Expression::StructNewId> {
  StructNew() {}
  StructNew(MixedArena& allocator) : StructNew() {}
  void finalize();
};
class StructGet : public SpecificExpression<Expression::StructGetId> {
  StructGet() {}
  StructGet(MixedArena& allocator) : StructGet() {}
  void finalize();
};
class StructSet : public SpecificExpression<Expression::StructSetId> {
  StructSet() {}
  StructSet(MixedArena& allocator) : StructSet() {}
  void finalize();
};
class ArrayNew : public SpecificExpression<Expression::ArrayNewId> {
  ArrayNew() {}
  ArrayNew(MixedArena& allocator) : ArrayNew() {}
  void finalize();
};
class ArrayGet : public SpecificExpression<Expression::ArrayGetId> {
  ArrayGet() {}
  ArrayGet(MixedArena& allocator) : ArrayGet() {}
  void finalize();
};
class ArraySet : public SpecificExpression<Expression::ArraySetId> {
  ArraySet() {}
  ArraySet(MixedArena& allocator) : ArraySet() {}
  void finalize();
};
class ArrayLen : public SpecificExpression<Expression::ArrayLenId> {
  ArrayLen() {}
  ArrayLen(MixedArena& allocator) : ArrayLen() {}
  void finalize();
};
