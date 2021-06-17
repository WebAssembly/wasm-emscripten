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

// Implements a switch on an expression class ID, and has a case for each id
// in which it runs delegates on the fields and immediates. You should include
// this file after defining the relevant DELEGATE_* macros.
//
// All defines used here are undefed automatically at the end for you.
//
// Most of the defines are necessary, and you will get an error if you forget
// them, but some are optional and some imply others, see below.
//
// The defines are as follows:
//
// DELEGATE_START(id) - called at the start of a case for an expression class.
//
// DELEGATE_END(id) - called at the end of a case.
//
// DELEGATE_GET_FIELD(id, name) - called to get a field by its name. This must
//    know the object on which to get it, so it is just useful for the case
//    where you operate on a single such object, but in that case it is nice
//    because then other things can be defined automatically for you, see later.
//
// DELEGATE_FIELD_CHILD(id, name) - called for each child field (note: children
//    are visited in reverse order, which is convenient for walking by pushing
//    them to a stack first).
//
// DELEGATE_FIELD_OPTIONAL_CHILD(id, name) - called for a child that may not be
//    present (like a Return's value). If you do not define this then
//    DELEGATE_FIELD_CHILD is called.
//
// DELEGATE_FIELD_CHILD_VECTOR(id, name) - called for a variable-sized vector of
//    child pointers. If this is not defined, and DELEGATE_GET_FIELD is, then
//    DELEGATE_FIELD_CHILD is called on them.
//
// DELEGATE_FIELD_INT(id, name) - called for an integer field (bool, enum,
//    Index, int32, or int64).
//
// DELEGATE_FIELD_INT_ARRAY(id, name) - called for a std::array of fixed size of
//    integer values (like a SIMD mask). If this is not defined, and
//    DELEGATE_GET_FIELD is, then DELEGATE_FIELD_INT is called on them.
//
// DELEGATE_FIELD_LITERAL(id, name) - called for a Literal.
//
// DELEGATE_FIELD_NAME(id, name) - called for a Name.
//
// DELEGATE_FIELD_NAME_VECTOR(id, name) - called for a variable-sized vector of
//    names (like try's catch event names). If this is not defined, and
//    DELEGATE_GET_FIELD is, then DELEGATE_FIELD_CHILD is called on them.
//
// DELEGATE_FIELD_SCOPE_NAME_DEF(id, name) - called for a scope name definition
//    (like a block's name).
//
// DELEGATE_FIELD_SCOPE_NAME_USE(id, name) - called for a scope name use (like
//    a break's target).
//
// DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR(id, name) - called for a variable-sized
//    vector of scope names (like a switch's targets). If this is not defined,
//    and DELEGATE_GET_FIELD is, then DELEGATE_FIELD_SCOPE_NAME_USE is called on
//    them.
//
// DELEGATE_FIELD_SIGNATURE(id, name) - called for a Signature.
//
// DELEGATE_FIELD_TYPE(id, name) - called for a Type.
//
// DELEGATE_FIELD_ADDRESS(id, name) - called for an Address.

#ifndef DELEGATE_START
#define DELEGATE_START(id)
#endif

#ifndef DELEGATE_END
#define DELEGATE_END(id)
#endif

#ifndef DELEGATE_FIELD_CHILD
#error please define DELEGATE_FIELD_CHILD(id, name)
#endif

#ifndef DELEGATE_FIELD_OPTIONAL_CHILD
#define DELEGATE_FIELD_OPTIONAL_CHILD(id, name) DELEGATE_FIELD_CHILD(id, name)
#endif

#ifndef DELEGATE_FIELD_CHILD_VECTOR
#ifdef DELEGATE_GET_FIELD
#define DELEGATE_FIELD_CHILD_VECTOR(id, name)                                  \
  for (int i = int((DELEGATE_GET_FIELD(id, name)).size()) - 1; i >= 0; i--) {  \
    DELEGATE_FIELD_CHILD(id, name[i]);                                         \
  }
#else
#error please define DELEGATE_FIELD_CHILD_VECTOR(id, name)
#endif
#endif

#ifndef DELEGATE_FIELD_INT
#error please define DELEGATE_FIELD_INT(id, name)
#endif

#ifndef DELEGATE_FIELD_INT_ARRAY
#ifdef DELEGATE_GET_FIELD
#define DELEGATE_FIELD_INT_ARRAY(id, name)                                     \
  for (Index i = 0; i < (DELEGATE_GET_FIELD(id, name)).size(); i++) {          \
    DELEGATE_FIELD_INT(id, name[i]);                                           \
  }
#else
#error please define DELEGATE_FIELD_INT_ARRAY(id, name)
#endif
#endif

#ifndef DELEGATE_FIELD_LITERAL
#error please define DELEGATE_FIELD_LITERAL(id, name)
#endif

#ifndef DELEGATE_FIELD_NAME
#error please define DELEGATE_FIELD_NAME(id, name)
#endif

#ifndef DELEGATE_FIELD_NAME_VECTOR
#ifdef DELEGATE_GET_FIELD
#define DELEGATE_FIELD_NAME_VECTOR(id, name)                                   \
  for (Index i = 0; i < (DELEGATE_GET_FIELD(id, name)).size(); i++) {          \
    DELEGATE_FIELD_NAME(id, name[i]);                                          \
  }
#else
#error please define DELEGATE_FIELD_NAME_VECTOR(id, name)
#endif
#endif

#ifndef DELEGATE_FIELD_SCOPE_NAME_DEF
#error please define DELEGATE_FIELD_SCOPE_NAME_DEF(id, name)
#endif

#ifndef DELEGATE_FIELD_SCOPE_NAME_USE
#error please define DELEGATE_FIELD_SCOPE_NAME_USE(id, name)
#endif

#ifndef DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR
#ifdef DELEGATE_GET_FIELD
#define DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR(id, name)                         \
  for (Index i = 0; i < (DELEGATE_GET_FIELD(id, name)).size(); i++) {          \
    DELEGATE_FIELD_SCOPE_NAME_USE(id, name[i]);                                \
  }
#else
#error please define DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR(id, name)
#endif
#endif

#ifndef DELEGATE_FIELD_SIGNATURE
#error please define DELEGATE_FIELD_SIGNATURE(id, name)
#endif

#ifndef DELEGATE_FIELD_TYPE
#error please define DELEGATE_FIELD_TYPE(id, name)
#endif

#ifndef DELEGATE_FIELD_ADDRESS
#error please define DELEGATE_FIELD_ADDRESS(id, name)
#endif

switch (DELEGATE_ID) {
  case Expression::Id::InvalidId:
  case Expression::Id::NumExpressionIds: {
    WASM_UNREACHABLE("unexpected expression type");
  }
  case Expression::Id::BlockId: {
    DELEGATE_START(Block);
    DELEGATE_FIELD_CHILD_VECTOR(Block, list);
    DELEGATE_FIELD_SCOPE_NAME_DEF(Block, name);
    DELEGATE_END(Block);
    break;
  }
  case Expression::Id::IfId: {
    DELEGATE_START(If);
    DELEGATE_FIELD_OPTIONAL_CHILD(If, ifFalse);
    DELEGATE_FIELD_CHILD(If, ifTrue);
    DELEGATE_FIELD_CHILD(If, condition);
    DELEGATE_END(If);
    break;
  }
  case Expression::Id::LoopId: {
    DELEGATE_START(Loop);
    DELEGATE_FIELD_CHILD(Loop, body);
    DELEGATE_FIELD_SCOPE_NAME_DEF(Loop, name);
    DELEGATE_END(Loop);
    break;
  }
  case Expression::Id::BreakId: {
    DELEGATE_START(Break);
    DELEGATE_FIELD_OPTIONAL_CHILD(Break, condition);
    DELEGATE_FIELD_OPTIONAL_CHILD(Break, value);
    DELEGATE_FIELD_SCOPE_NAME_USE(Break, name);
    DELEGATE_END(Break);
    break;
  }
  case Expression::Id::SwitchId: {
    DELEGATE_START(Switch);
    DELEGATE_FIELD_CHILD(Switch, condition);
    DELEGATE_FIELD_OPTIONAL_CHILD(Switch, value);
    DELEGATE_FIELD_SCOPE_NAME_USE(Switch, default_);
    DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR(Switch, targets);
    DELEGATE_END(Switch);
    break;
  }
  case Expression::Id::CallId: {
    DELEGATE_START(Call);
    DELEGATE_FIELD_CHILD_VECTOR(Call, operands);
    DELEGATE_FIELD_NAME(Call, target);
    DELEGATE_FIELD_INT(Call, isReturn);
    DELEGATE_END(Call);
    break;
  }
  case Expression::Id::CallIndirectId: {
    DELEGATE_START(CallIndirect);
    DELEGATE_FIELD_CHILD(CallIndirect, target);
    DELEGATE_FIELD_NAME(CallIndirect, table);
    DELEGATE_FIELD_CHILD_VECTOR(CallIndirect, operands);
    DELEGATE_FIELD_SIGNATURE(CallIndirect, sig);
    DELEGATE_FIELD_INT(CallIndirect, isReturn);
    DELEGATE_END(CallIndirect);
    break;
  }
  case Expression::Id::LocalGetId: {
    DELEGATE_START(LocalGet);
    DELEGATE_FIELD_INT(LocalGet, index);
    DELEGATE_END(LocalGet);
    break;
  }
  case Expression::Id::LocalSetId: {
    DELEGATE_START(LocalSet);
    DELEGATE_FIELD_CHILD(LocalSet, value);
    DELEGATE_FIELD_INT(LocalSet, index);
    DELEGATE_END(LocalSet);
    break;
  }
  case Expression::Id::GlobalGetId: {
    DELEGATE_START(GlobalGet);
    DELEGATE_FIELD_INT(GlobalGet, name);
    DELEGATE_END(GlobalGet);
    break;
  }
  case Expression::Id::GlobalSetId: {
    DELEGATE_START(GlobalSet);
    DELEGATE_FIELD_CHILD(GlobalSet, value);
    DELEGATE_FIELD_INT(GlobalSet, name);
    DELEGATE_END(GlobalSet);
    break;
  }
  case Expression::Id::LoadId: {
    DELEGATE_START(Load);
    DELEGATE_FIELD_CHILD(Load, ptr);
    DELEGATE_FIELD_INT(Load, bytes);
    DELEGATE_FIELD_INT(Load, signed_);
    DELEGATE_FIELD_ADDRESS(Load, offset);
    DELEGATE_FIELD_ADDRESS(Load, align);
    DELEGATE_FIELD_INT(Load, isAtomic);
    DELEGATE_END(Load);
    break;
  }
  case Expression::Id::StoreId: {
    DELEGATE_START(Store);
    DELEGATE_FIELD_CHILD(Store, value);
    DELEGATE_FIELD_CHILD(Store, ptr);
    DELEGATE_FIELD_INT(Store, bytes);
    DELEGATE_FIELD_ADDRESS(Store, offset);
    DELEGATE_FIELD_ADDRESS(Store, align);
    DELEGATE_FIELD_INT(Store, isAtomic);
    DELEGATE_FIELD_TYPE(Store, valueType);
    DELEGATE_END(Store);
    break;
  }
  case Expression::Id::AtomicRMWId: {
    DELEGATE_START(AtomicRMW);
    DELEGATE_FIELD_CHILD(AtomicRMW, value);
    DELEGATE_FIELD_CHILD(AtomicRMW, ptr);
    DELEGATE_FIELD_INT(AtomicRMW, op);
    DELEGATE_FIELD_INT(AtomicRMW, bytes);
    DELEGATE_FIELD_ADDRESS(AtomicRMW, offset);
    DELEGATE_END(AtomicRMW);
    break;
  }
  case Expression::Id::AtomicCmpxchgId: {
    DELEGATE_START(AtomicCmpxchg);
    DELEGATE_FIELD_CHILD(AtomicCmpxchg, replacement);
    DELEGATE_FIELD_CHILD(AtomicCmpxchg, expected);
    DELEGATE_FIELD_CHILD(AtomicCmpxchg, ptr);
    DELEGATE_FIELD_INT(AtomicCmpxchg, bytes);
    DELEGATE_FIELD_ADDRESS(AtomicCmpxchg, offset);
    DELEGATE_END(AtomicCmpxchgId);
    break;
  }
  case Expression::Id::AtomicWaitId: {
    DELEGATE_START(AtomicWait);
    DELEGATE_FIELD_CHILD(AtomicWait, timeout);
    DELEGATE_FIELD_CHILD(AtomicWait, expected);
    DELEGATE_FIELD_CHILD(AtomicWait, ptr);
    DELEGATE_FIELD_ADDRESS(AtomicWait, offset);
    DELEGATE_FIELD_TYPE(AtomicWait, expectedType);
    DELEGATE_END(AtomicWait);
    break;
  }
  case Expression::Id::AtomicNotifyId: {
    DELEGATE_START(AtomicNotify);
    DELEGATE_FIELD_CHILD(AtomicNotify, notifyCount);
    DELEGATE_FIELD_CHILD(AtomicNotify, ptr);
    DELEGATE_FIELD_ADDRESS(AtomicNotify, offset);
    DELEGATE_END(AtomicNotify);
    break;
  }
  case Expression::Id::AtomicFenceId: {
    DELEGATE_START(AtomicFence);
    DELEGATE_FIELD_INT(AtomicFence, order);
    DELEGATE_END(AtomicFence);
    break;
  }
  case Expression::Id::SIMDExtractId: {
    DELEGATE_START(SIMDExtract);
    DELEGATE_FIELD_CHILD(SIMDExtract, vec);
    DELEGATE_FIELD_INT(SIMDExtract, op);
    DELEGATE_FIELD_INT(SIMDExtract, index);
    DELEGATE_END(SIMDExtract);
    break;
  }
  case Expression::Id::SIMDReplaceId: {
    DELEGATE_START(SIMDReplace);
    DELEGATE_FIELD_CHILD(SIMDReplace, value);
    DELEGATE_FIELD_CHILD(SIMDReplace, vec);
    DELEGATE_FIELD_INT(SIMDReplace, op);
    DELEGATE_FIELD_INT(SIMDReplace, index);
    DELEGATE_END(SIMDReplace);
    break;
  }
  case Expression::Id::SIMDShuffleId: {
    DELEGATE_START(SIMDShuffle);
    DELEGATE_FIELD_CHILD(SIMDShuffle, right);
    DELEGATE_FIELD_CHILD(SIMDShuffle, left);
    DELEGATE_FIELD_INT_ARRAY(SIMDShuffle, mask);
    DELEGATE_END(SIMDShuffle);
    break;
  }
  case Expression::Id::SIMDTernaryId: {
    DELEGATE_START(SIMDTernary);
    DELEGATE_FIELD_CHILD(SIMDTernary, c);
    DELEGATE_FIELD_CHILD(SIMDTernary, b);
    DELEGATE_FIELD_CHILD(SIMDTernary, a);
    DELEGATE_FIELD_INT(SIMDTernary, op);
    DELEGATE_END(SIMDTernary);
    break;
  }
  case Expression::Id::SIMDShiftId: {
    DELEGATE_START(SIMDShift);
    DELEGATE_FIELD_CHILD(SIMDShift, shift);
    DELEGATE_FIELD_CHILD(SIMDShift, vec);
    DELEGATE_FIELD_INT(SIMDShift, op);
    DELEGATE_END(SIMDShift);
    break;
  }
  case Expression::Id::SIMDLoadId: {
    DELEGATE_START(SIMDLoad);
    DELEGATE_FIELD_CHILD(SIMDLoad, ptr);
    DELEGATE_FIELD_INT(SIMDLoad, op);
    DELEGATE_FIELD_ADDRESS(SIMDLoad, offset);
    DELEGATE_FIELD_ADDRESS(SIMDLoad, align);
    DELEGATE_END(SIMDLoad);
    break;
  }
  case Expression::Id::SIMDLoadStoreLaneId: {
    DELEGATE_START(SIMDLoadStoreLane);
    DELEGATE_FIELD_CHILD(SIMDLoadStoreLane, vec);
    DELEGATE_FIELD_CHILD(SIMDLoadStoreLane, ptr);
    DELEGATE_FIELD_INT(SIMDLoadStoreLane, op);
    DELEGATE_FIELD_ADDRESS(SIMDLoadStoreLane, offset);
    DELEGATE_FIELD_ADDRESS(SIMDLoadStoreLane, align);
    DELEGATE_FIELD_INT(SIMDLoadStoreLane, index);
    DELEGATE_END(SIMDLoadStoreLane);
    break;
  }
  case Expression::Id::MemoryInitId: {
    DELEGATE_START(MemoryInit);
    DELEGATE_FIELD_CHILD(MemoryInit, size);
    DELEGATE_FIELD_CHILD(MemoryInit, offset);
    DELEGATE_FIELD_CHILD(MemoryInit, dest);
    DELEGATE_FIELD_INT(MemoryInit, segment);
    DELEGATE_END(MemoryInit);
    break;
  }
  case Expression::Id::DataDropId: {
    DELEGATE_START(DataDrop);
    DELEGATE_FIELD_INT(DataDrop, segment);
    DELEGATE_END(DataDrop);
    break;
  }
  case Expression::Id::MemoryCopyId: {
    DELEGATE_START(MemoryCopy);
    DELEGATE_FIELD_CHILD(MemoryCopy, size);
    DELEGATE_FIELD_CHILD(MemoryCopy, source);
    DELEGATE_FIELD_CHILD(MemoryCopy, dest);
    DELEGATE_END(MemoryCopy);
    break;
  }
  case Expression::Id::MemoryFillId: {
    DELEGATE_START(MemoryFill);
    DELEGATE_FIELD_CHILD(MemoryFill, size);
    DELEGATE_FIELD_CHILD(MemoryFill, value);
    DELEGATE_FIELD_CHILD(MemoryFill, dest);
    DELEGATE_END(MemoryFill);
    break;
  }
  case Expression::Id::ConstId: {
    DELEGATE_START(Const);
    DELEGATE_FIELD_LITERAL(Const, value);
    DELEGATE_END(Const);
    break;
  }
  case Expression::Id::UnaryId: {
    DELEGATE_START(Unary);
    DELEGATE_FIELD_CHILD(Unary, value);
    DELEGATE_FIELD_INT(Unary, op);
    DELEGATE_END(Unary);
    break;
  }
  case Expression::Id::BinaryId: {
    DELEGATE_START(Binary);
    DELEGATE_FIELD_CHILD(Binary, right);
    DELEGATE_FIELD_CHILD(Binary, left);
    DELEGATE_FIELD_INT(Binary, op);
    DELEGATE_END(Binary);
    break;
  }
  case Expression::Id::SelectId: {
    DELEGATE_START(Select);
    DELEGATE_FIELD_CHILD(Select, condition);
    DELEGATE_FIELD_CHILD(Select, ifFalse);
    DELEGATE_FIELD_CHILD(Select, ifTrue);
    DELEGATE_END(Select);
    break;
  }
  case Expression::Id::DropId: {
    DELEGATE_START(Drop);
    DELEGATE_FIELD_CHILD(Drop, value);
    DELEGATE_END(Drop);
    break;
  }
  case Expression::Id::ReturnId: {
    DELEGATE_START(Return);
    DELEGATE_FIELD_OPTIONAL_CHILD(Return, value);
    DELEGATE_END(Return);
    break;
  }
  case Expression::Id::MemorySizeId: {
    DELEGATE_START(MemorySize);
    DELEGATE_END(MemorySize);
    break;
  }
  case Expression::Id::MemoryGrowId: {
    DELEGATE_START(MemoryGrow);
    DELEGATE_FIELD_CHILD(MemoryGrow, delta);
    DELEGATE_END(MemoryGrow);
    break;
  }
  case Expression::Id::TableGetId: {
    DELEGATE_START(TableGet);
    DELEGATE_FIELD_NAME(TableGet, table);
    DELEGATE_FIELD_CHILD(TableGet, offset);
    DELEGATE_END(TableGet);
    break;
  }
  case Expression::Id::TableSetId: {
    DELEGATE_START(TableSet);
    DELEGATE_FIELD_NAME(TableSet, table);
    DELEGATE_FIELD_CHILD(TableSet, value);
    DELEGATE_FIELD_CHILD(TableSet, offset);
    DELEGATE_END(TableSet);
    break;
  }
  case Expression::Id::TableSizeId: {
    DELEGATE_START(TableSize);
    DELEGATE_FIELD_NAME(TableSize, table);
    DELEGATE_END(TableSize);
    break;
  }
  case Expression::Id::TableGrowId: {
    DELEGATE_START(TableGrow);
    DELEGATE_FIELD_NAME(TableGrow, table);
    DELEGATE_FIELD_CHILD(TableGrow, delta);
    DELEGATE_FIELD_CHILD(TableGrow, initialValue);
    DELEGATE_END(TableGrow);
    break;
  }
  case Expression::Id::TableFillId: {
    DELEGATE_START(TableFill);
    DELEGATE_FIELD_NAME(TableFill, table);
    DELEGATE_FIELD_CHILD(TableFill, size);
    DELEGATE_FIELD_CHILD(TableFill, value);
    DELEGATE_FIELD_CHILD(TableFill, dest);
    DELEGATE_END(TableFill);
    break;
  }
  case Expression::Id::TableCopyId: {
    DELEGATE_START(TableCopy);
    DELEGATE_FIELD_NAME(TableCopy, destTable);
    DELEGATE_FIELD_NAME(TableCopy, srcTable);
    DELEGATE_FIELD_CHILD(TableCopy, size);
    DELEGATE_FIELD_CHILD(TableCopy, srcOffset);
    DELEGATE_FIELD_CHILD(TableCopy, destOffset);
    DELEGATE_END(TableCopy);
    break;
  }
  case Expression::Id::TableInitId: {
    DELEGATE_START(TableInit);
    DELEGATE_FIELD_NAME(TableInit, table);
    DELEGATE_FIELD_NAME(TableInit, segment);
    DELEGATE_FIELD_CHILD(TableInit, size);
    DELEGATE_FIELD_CHILD(TableInit, srcOffset);
    DELEGATE_FIELD_CHILD(TableInit, destOffset);
    DELEGATE_END(TableInit);
    break;
  }
  case Expression::Id::ElemDropId: {
    DELEGATE_START(ElemDrop);
    DELEGATE_FIELD_NAME(ElemDrop, segment);
    DELEGATE_END(ElemDrop);
    break;
  }
  case Expression::Id::RefNullId: {
    DELEGATE_START(RefNull);
    DELEGATE_FIELD_TYPE(RefNull, type);
    DELEGATE_END(RefNull);
    break;
  }
  case Expression::Id::RefIsId: {
    DELEGATE_START(RefIs);
    DELEGATE_FIELD_INT(RefIs, op);
    DELEGATE_FIELD_CHILD(RefIs, value);
    DELEGATE_END(RefIs);
    break;
  }
  case Expression::Id::RefFuncId: {
    DELEGATE_START(RefFunc);
    DELEGATE_FIELD_NAME(RefFunc, func);
    DELEGATE_END(RefFunc);
    break;
  }
  case Expression::Id::RefEqId: {
    DELEGATE_START(RefEq);
    DELEGATE_FIELD_CHILD(RefEq, right);
    DELEGATE_FIELD_CHILD(RefEq, left);
    DELEGATE_END(RefEq);
    break;
  }
  case Expression::Id::TryId: {
    DELEGATE_START(Try);
    DELEGATE_FIELD_SCOPE_NAME_USE(Try, delegateTarget);
    DELEGATE_FIELD_CHILD_VECTOR(Try, catchBodies);
    DELEGATE_FIELD_NAME_VECTOR(Try, catchEvents);
    DELEGATE_FIELD_SCOPE_NAME_DEF(Try, name);
    DELEGATE_FIELD_CHILD(Try, body);
    DELEGATE_END(Try);
    break;
  }
  case Expression::Id::ThrowId: {
    DELEGATE_START(Throw);
    DELEGATE_FIELD_CHILD_VECTOR(Throw, operands);
    DELEGATE_FIELD_NAME(Throw, event);
    DELEGATE_END(Throw);
    break;
  }
  case Expression::Id::RethrowId: {
    DELEGATE_START(Rethrow);
    DELEGATE_FIELD_SCOPE_NAME_USE(Rethrow, target);
    DELEGATE_END(Rethrow);
    break;
  }
  case Expression::Id::NopId: {
    DELEGATE_START(Nop);
    DELEGATE_END(Nop);
    break;
  }
  case Expression::Id::UnreachableId: {
    DELEGATE_START(Unreachable);
    DELEGATE_END(Unreachable);
    break;
  }
  case Expression::Id::PopId: {
    DELEGATE_START(Pop);
    DELEGATE_END(Pop);
    break;
  }
  case Expression::Id::TupleMakeId: {
    DELEGATE_START(TupleMake);
    DELEGATE_FIELD_CHILD_VECTOR(Tuple, operands);
    DELEGATE_END(TupleMake);
    break;
  }
  case Expression::Id::TupleExtractId: {
    DELEGATE_START(TupleExtract);
    DELEGATE_FIELD_CHILD(TupleExtract, tuple);
    DELEGATE_FIELD_INT(TupleExtract, index);
    DELEGATE_END(TupleExtract);
    break;
  }
  case Expression::Id::I31NewId: {
    DELEGATE_START(I31New);
    DELEGATE_FIELD_CHILD(I31New, value);
    DELEGATE_END(I31New);
    break;
  }
  case Expression::Id::I31GetId: {
    DELEGATE_START(I31Get);
    DELEGATE_FIELD_CHILD(I31Get, i31);
    DELEGATE_FIELD_INT(I31Get, signed_);
    DELEGATE_END(I31Get);
    break;
  }
  case Expression::Id::CallRefId: {
    DELEGATE_START(CallRef);
    DELEGATE_FIELD_CHILD(CallRef, target);
    DELEGATE_FIELD_CHILD_VECTOR(CallRef, operands);
    DELEGATE_FIELD_INT(CallRef, isReturn);
    DELEGATE_END(CallRef);
    break;
  }
  case Expression::Id::RefTestId: {
    DELEGATE_START(RefTest);
    DELEGATE_FIELD_CHILD(RefTest, rtt);
    DELEGATE_FIELD_CHILD(RefTest, ref);
    DELEGATE_END(RefTest);
    break;
  }
  case Expression::Id::RefCastId: {
    DELEGATE_START(RefCast);
    DELEGATE_FIELD_CHILD(RefCast, rtt);
    DELEGATE_FIELD_CHILD(RefCast, ref);
    DELEGATE_END(RefCast);
    break;
  }
  case Expression::Id::BrOnId: {
    DELEGATE_START(BrOn);
    DELEGATE_FIELD_INT(BrOn, op);
    DELEGATE_FIELD_SCOPE_NAME_USE(BrOn, name);
    DELEGATE_FIELD_OPTIONAL_CHILD(BrOn, rtt);
    DELEGATE_FIELD_CHILD(BrOn, ref);
    DELEGATE_END(BrOn);
    break;
  }
  case Expression::Id::RttCanonId: {
    DELEGATE_START(RttCanon);
    DELEGATE_END(RttCanon);
    break;
  }
  case Expression::Id::RttSubId: {
    DELEGATE_START(RttSub);
    DELEGATE_FIELD_CHILD(RttSub, parent);
    DELEGATE_FIELD_INT(RttSub, fresh);
    DELEGATE_END(RttSub);
    break;
  }
  case Expression::Id::StructNewId: {
    DELEGATE_START(StructNew);
    DELEGATE_FIELD_CHILD(StructNew, rtt);
    DELEGATE_FIELD_CHILD_VECTOR(StructNew, operands);
    DELEGATE_END(StructNew);
    break;
  }
  case Expression::Id::StructGetId: {
    DELEGATE_START(StructGet);
    DELEGATE_FIELD_INT(StructGet, index);
    DELEGATE_FIELD_CHILD(StructGet, ref);
    DELEGATE_FIELD_INT(StructGet, signed_);
    DELEGATE_END(StructGet);
    break;
  }
  case Expression::Id::StructSetId: {
    DELEGATE_START(StructSet);
    DELEGATE_FIELD_INT(StructSet, index);
    DELEGATE_FIELD_CHILD(StructSet, value);
    DELEGATE_FIELD_CHILD(StructSet, ref);
    DELEGATE_END(StructSet);
    break;
  }
  case Expression::Id::ArrayNewId: {
    DELEGATE_START(ArrayNew);
    DELEGATE_FIELD_CHILD(ArrayNew, rtt);
    DELEGATE_FIELD_CHILD(ArrayNew, size);
    DELEGATE_FIELD_OPTIONAL_CHILD(ArrayNew, init);
    DELEGATE_END(ArrayNew);
    break;
  }
  case Expression::Id::ArrayGetId: {
    DELEGATE_START(ArrayGet);
    DELEGATE_FIELD_CHILD(ArrayGet, index);
    DELEGATE_FIELD_CHILD(ArrayGet, ref);
    DELEGATE_FIELD_INT(ArrayGet, signed_);
    DELEGATE_END(ArrayGet);
    break;
  }
  case Expression::Id::ArraySetId: {
    DELEGATE_START(ArraySet);
    DELEGATE_FIELD_CHILD(ArrayGet, value);
    DELEGATE_FIELD_CHILD(ArrayGet, index);
    DELEGATE_FIELD_CHILD(ArrayGet, ref);
    DELEGATE_END(ArraySet);
    break;
  }
  case Expression::Id::ArrayLenId: {
    DELEGATE_START(ArrayLen);
    DELEGATE_FIELD_CHILD(ArrayLen, ref);
    DELEGATE_END(ArrayLen);
    break;
  }
  case Expression::Id::ArrayCopyId: {
    DELEGATE_START(ArrayCopy);
    DELEGATE_FIELD_CHILD(ArrayCopy, length);
    DELEGATE_FIELD_CHILD(ArrayCopy, srcIndex);
    DELEGATE_FIELD_CHILD(ArrayCopy, srcRef);
    DELEGATE_FIELD_CHILD(ArrayCopy, destIndex);
    DELEGATE_FIELD_CHILD(ArrayCopy, destRef);
    DELEGATE_END(ArrayCopy);
    break;
  }
  case Expression::Id::RefAsId: {
    DELEGATE_START(RefAs);
    DELEGATE_FIELD_INT(RefAs, op);
    DELEGATE_FIELD_CHILD(RefAs, value);
    DELEGATE_END(RefAs);
    break;
  }
}

#undef DELEGATE_ID
#undef DELEGATE_START
#undef DELEGATE_END
#undef DELEGATE_FIELD_CHILD
#undef DELEGATE_FIELD_OPTIONAL_CHILD
#undef DELEGATE_FIELD_CHILD_VECTOR
#undef DELEGATE_FIELD_INT
#undef DELEGATE_FIELD_INT_ARRAY
#undef DELEGATE_FIELD_LITERAL
#undef DELEGATE_FIELD_NAME
#undef DELEGATE_FIELD_NAME_VECTOR
#undef DELEGATE_FIELD_SCOPE_NAME_DEF
#undef DELEGATE_FIELD_SCOPE_NAME_USE
#undef DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR
#undef DELEGATE_FIELD_SIGNATURE
#undef DELEGATE_FIELD_TYPE
#undef DELEGATE_FIELD_ADDRESS
#undef DELEGATE_GET_FIELD
