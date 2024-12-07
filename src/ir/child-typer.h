/*
 * Copyright 2024 WebAssembly Community Group participants
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
#ifndef wasm_ir_child_typer_h
#define wasm_ir_child_typer_h

#include "wasm-traversal.h"
#include "wasm.h"

namespace wasm {

// CRTP visitor for determining constraints on the types of expression children.
// For each child of the visited expression, calls a callback with a pointer to
// the child and information on how the child is constrained. The possible
// callbacks are:
//
//   noteSubtype(Expression** childp, Type type) - The child must be a subtype
//   of `type`, which may be a tuple type. For children that must not produce
//   values, this may be `Type::none`. This accounts for most type constraints.
//
//   noteAnyType(Expression** childp) - The child may have any non-tuple type.
//   Used for the children of polymorphic instructions like `drop` and `select`.
//
//   noteAnyReference(Expression** childp) - The child may have any reference
//   type. Used for the children of polymorphic reference instructions like
//   `ref.is_null`.
//
//   noteAnyTupleType(Expression** childp, size_t arity) - The child may have
//   any tuple type with the given arity. Used for the children of polymorphic
//   tuple instructions like `tuple.drop` and `tuple.extract`.
//
// Subclasses must additionally implement a callback for getting the type of a
// branch target. This callback will only be used when the label type is not
// passed directly as an argument to the branch visitor method (see below).
//
//   Type getLabelType(Name label)
//
// Children with type `unreachable` satisfy all constraints.
//
// Constraints are determined using information that would be present in the
// binary, e.g. type annotation immediates. Many of the visitor methods take
// optional additional parameter for passing this information directly, and if
// those parameters are not used, it is an error to use this utility in cases
// where that information cannot be recovered from the IR.
//
// For example, it is an error to visit a `StructSet` expression whose `ref`
// field is unreachable or null without directly passing the heap type to the
// visitor because it is not possible for the utility to determine what type the
// value child should be in that case.
//
// Conversely, this utility does not use any information that would not be
// present in the binary, and in particular it generally does not introspect on
// the types of children. For example, it does not report the constraint that
// two non-reference children of `select` must have the same type because that
// would require inspecting the types of those children.
template<typename Subtype> struct ChildTyper : OverriddenVisitor<Subtype> {
  Module& wasm;
  Function* func;

  ChildTyper(Module& wasm, Function* func) : wasm(wasm), func(func) {}

  Subtype& self() { return *static_cast<Subtype*>(this); }

  void note(Expression** childp, Type type) {
    self().noteSubtype(childp, type);
  }

  void notePointer(Expression** ptrp, Name mem) {
    note(ptrp, wasm.getMemory(mem)->addressType);
  }

  void noteAny(Expression** childp) { self().noteAnyType(childp); }

  void noteAnyReference(Expression** childp) {
    self().noteAnyReferenceType(childp);
  }

  void noteAnyTuple(Expression** childp, size_t arity) {
    self().noteAnyTupleType(childp, arity);
  }

  Type getLabelType(Name label) { return self().getLabelType(label); }

  void visitNop(Nop* curr) {}

  void visitBlock(Block* curr) {
    size_t n = curr->list.size();
    if (n == 0) {
      return;
    }
    for (size_t i = 0; i < n - 1; ++i) {
      note(&curr->list[i], Type::none);
    }
    note(&curr->list.back(), curr->type);
  }

  void visitIf(If* curr) {
    note(&curr->condition, Type::i32);
    note(&curr->ifTrue, curr->type);
    if (curr->ifFalse) {
      note(&curr->ifFalse, curr->type);
    }
  }

  void visitLoop(Loop* curr) { note(&curr->body, curr->type); }

  void visitBreak(Break* curr, std::optional<Type> labelType = std::nullopt) {
    if (!labelType) {
      labelType = getLabelType(curr->name);
    }
    if (*labelType != Type::none) {
      note(&curr->value, *labelType);
    }
    if (curr->condition) {
      note(&curr->condition, Type::i32);
    }
  }

  void visitSwitch(Switch* curr, std::optional<Type> labelType = std::nullopt) {
    if (!labelType) {
      Type glb = getLabelType(curr->default_);
      for (auto label : curr->targets) {
        glb = Type::getGreatestLowerBound(glb, getLabelType(label));
      }
      labelType = glb;
    }
    if (*labelType != Type::none) {
      note(&curr->value, *labelType);
    }
    note(&curr->condition, Type::i32);
  }

  template<typename T> void handleCall(T* curr, Type params) {
    assert(params.size() == curr->operands.size());
    for (size_t i = 0; i < params.size(); ++i) {
      note(&curr->operands[i], params[i]);
    }
  }

  void visitCall(Call* curr) {
    auto params = wasm.getFunction(curr->target)->getParams();
    handleCall(curr, params);
  }

  void visitCallIndirect(CallIndirect* curr) {
    auto params = curr->heapType.getSignature().params;
    handleCall(curr, params);
    note(&curr->target, Type::i32);
  }

  void visitLocalGet(LocalGet* curr) {}

  void visitLocalSet(LocalSet* curr) {
    assert(func);
    note(&curr->value, func->getLocalType(curr->index));
  }

  void visitGlobalGet(GlobalGet* curr) {}

  void visitGlobalSet(GlobalSet* curr) {
    note(&curr->value, wasm.getGlobal(curr->name)->type);
  }

  void visitLoad(Load* curr) { notePointer(&curr->ptr, curr->memory); }

  void visitStore(Store* curr) {
    notePointer(&curr->ptr, curr->memory);
    note(&curr->value, curr->valueType);
  }

  void visitAtomicRMW(AtomicRMW* curr) {
    assert(curr->type == Type::i32 || curr->type == Type::i64);
    notePointer(&curr->ptr, curr->memory);
    note(&curr->value, curr->type);
  }

  void visitAtomicCmpxchg(AtomicCmpxchg* curr,
                          std::optional<Type> type = std::nullopt) {
    assert(!type || *type == Type::i32 || *type == Type::i64);
    notePointer(&curr->ptr, curr->memory);
    if (!type) {
      if (curr->expected->type == Type::i64 ||
          curr->replacement->type == Type::i64) {
        type = Type::i64;
      } else {
        type = Type::i32;
      }
    }
    note(&curr->expected, *type);
    note(&curr->replacement, *type);
  }

  void visitAtomicWait(AtomicWait* curr) {
    notePointer(&curr->ptr, curr->memory);
    note(&curr->expected, curr->expectedType);
    note(&curr->timeout, Type::i64);
  }

  void visitAtomicNotify(AtomicNotify* curr) {
    notePointer(&curr->ptr, curr->memory);
    note(&curr->notifyCount, Type::i32);
  }

  void visitAtomicFence(AtomicFence* curr) {}

  void visitSIMDExtract(SIMDExtract* curr) { note(&curr->vec, Type::v128); }

  void visitSIMDReplace(SIMDReplace* curr) {
    note(&curr->vec, Type::v128);
    switch (curr->op) {
      case ReplaceLaneVecI8x16:
      case ReplaceLaneVecI16x8:
      case ReplaceLaneVecI32x4:
        note(&curr->value, Type::i32);
        break;
      case ReplaceLaneVecI64x2:
        note(&curr->value, Type::i64);
        break;
      case ReplaceLaneVecF16x8:
      case ReplaceLaneVecF32x4:
        note(&curr->value, Type::f32);
        break;
      case ReplaceLaneVecF64x2:
        note(&curr->value, Type::f64);
        break;
    }
  }

  void visitSIMDShuffle(SIMDShuffle* curr) {
    note(&curr->left, Type::v128);
    note(&curr->right, Type::v128);
  }

  void visitSIMDTernary(SIMDTernary* curr) {
    note(&curr->a, Type::v128);
    note(&curr->b, Type::v128);
    note(&curr->c, Type::v128);
  }

  void visitSIMDShift(SIMDShift* curr) {
    note(&curr->vec, Type::v128);
    note(&curr->shift, Type::i32);
  }

  void visitSIMDLoad(SIMDLoad* curr) { notePointer(&curr->ptr, curr->memory); }

  void visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
    notePointer(&curr->ptr, curr->memory);
    note(&curr->vec, Type::v128);
  }

  void visitMemoryInit(MemoryInit* curr) {
    notePointer(&curr->dest, curr->memory);
    note(&curr->offset, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitDataDrop(DataDrop* curr) {}

  void visitMemoryCopy(MemoryCopy* curr) {
    assert(wasm.getMemory(curr->destMemory)->addressType ==
           wasm.getMemory(curr->sourceMemory)->addressType);
    notePointer(&curr->dest, curr->destMemory);
    notePointer(&curr->source, curr->sourceMemory);
    notePointer(&curr->size, curr->destMemory);
  }

  void visitMemoryFill(MemoryFill* curr) {
    notePointer(&curr->dest, curr->memory);
    note(&curr->value, Type::i32);
    notePointer(&curr->size, curr->memory);
  }

  void visitConst(Const* curr) {}

  void visitUnary(Unary* curr) {
    switch (curr->op) {
      case ClzInt32:
      case CtzInt32:
      case PopcntInt32:
      case EqZInt32:
      case ExtendSInt32:
      case ExtendUInt32:
      case ExtendS8Int32:
      case ExtendS16Int32:
      case ConvertUInt32ToFloat32:
      case ConvertUInt32ToFloat64:
      case ConvertSInt32ToFloat32:
      case ConvertSInt32ToFloat64:
      case ReinterpretInt32:
      case SplatVecI8x16:
      case SplatVecI16x8:
      case SplatVecI32x4:
        note(&curr->value, Type::i32);
        break;
      case ClzInt64:
      case CtzInt64:
      case PopcntInt64:
      case EqZInt64:
      case ExtendS8Int64:
      case ExtendS16Int64:
      case ExtendS32Int64:
      case WrapInt64:
      case ConvertUInt64ToFloat32:
      case ConvertUInt64ToFloat64:
      case ConvertSInt64ToFloat32:
      case ConvertSInt64ToFloat64:
      case ReinterpretInt64:
      case SplatVecI64x2:
        note(&curr->value, Type::i64);
        break;
      case NegFloat32:
      case AbsFloat32:
      case CeilFloat32:
      case FloorFloat32:
      case TruncFloat32:
      case NearestFloat32:
      case SqrtFloat32:
      case TruncSFloat32ToInt32:
      case TruncSFloat32ToInt64:
      case TruncUFloat32ToInt32:
      case TruncUFloat32ToInt64:
      case TruncSatSFloat32ToInt32:
      case TruncSatSFloat32ToInt64:
      case TruncSatUFloat32ToInt32:
      case TruncSatUFloat32ToInt64:
      case ReinterpretFloat32:
      case PromoteFloat32:
      case SplatVecF16x8:
      case SplatVecF32x4:
        note(&curr->value, Type::f32);
        break;
      case NegFloat64:
      case AbsFloat64:
      case CeilFloat64:
      case FloorFloat64:
      case TruncFloat64:
      case NearestFloat64:
      case SqrtFloat64:
      case TruncSFloat64ToInt32:
      case TruncSFloat64ToInt64:
      case TruncUFloat64ToInt32:
      case TruncUFloat64ToInt64:
      case TruncSatSFloat64ToInt32:
      case TruncSatSFloat64ToInt64:
      case TruncSatUFloat64ToInt32:
      case TruncSatUFloat64ToInt64:
      case ReinterpretFloat64:
      case DemoteFloat64:
      case SplatVecF64x2:
        note(&curr->value, Type::f64);
        break;
      case NotVec128:
      case PopcntVecI8x16:
      case AbsVecI8x16:
      case AbsVecI16x8:
      case AbsVecI32x4:
      case AbsVecI64x2:
      case NegVecI8x16:
      case NegVecI16x8:
      case NegVecI32x4:
      case NegVecI64x2:
      case AbsVecF16x8:
      case NegVecF16x8:
      case SqrtVecF16x8:
      case CeilVecF16x8:
      case FloorVecF16x8:
      case TruncVecF16x8:
      case NearestVecF16x8:
      case AbsVecF32x4:
      case NegVecF32x4:
      case SqrtVecF32x4:
      case CeilVecF32x4:
      case FloorVecF32x4:
      case TruncVecF32x4:
      case NearestVecF32x4:
      case AbsVecF64x2:
      case NegVecF64x2:
      case SqrtVecF64x2:
      case CeilVecF64x2:
      case FloorVecF64x2:
      case TruncVecF64x2:
      case NearestVecF64x2:
      case ExtAddPairwiseSVecI8x16ToI16x8:
      case ExtAddPairwiseUVecI8x16ToI16x8:
      case ExtAddPairwiseSVecI16x8ToI32x4:
      case ExtAddPairwiseUVecI16x8ToI32x4:
      case TruncSatSVecF32x4ToVecI32x4:
      case TruncSatUVecF32x4ToVecI32x4:
      case ConvertSVecI32x4ToVecF32x4:
      case ConvertUVecI32x4ToVecF32x4:
      case ExtendLowSVecI8x16ToVecI16x8:
      case ExtendHighSVecI8x16ToVecI16x8:
      case ExtendLowUVecI8x16ToVecI16x8:
      case ExtendHighUVecI8x16ToVecI16x8:
      case ExtendLowSVecI16x8ToVecI32x4:
      case ExtendHighSVecI16x8ToVecI32x4:
      case ExtendLowUVecI16x8ToVecI32x4:
      case ExtendHighUVecI16x8ToVecI32x4:
      case ExtendLowSVecI32x4ToVecI64x2:
      case ExtendHighSVecI32x4ToVecI64x2:
      case ExtendLowUVecI32x4ToVecI64x2:
      case ExtendHighUVecI32x4ToVecI64x2:
      case ConvertLowSVecI32x4ToVecF64x2:
      case ConvertLowUVecI32x4ToVecF64x2:
      case TruncSatZeroSVecF64x2ToVecI32x4:
      case TruncSatZeroUVecF64x2ToVecI32x4:
      case DemoteZeroVecF64x2ToVecF32x4:
      case PromoteLowVecF32x4ToVecF64x2:
      case RelaxedTruncSVecF32x4ToVecI32x4:
      case RelaxedTruncUVecF32x4ToVecI32x4:
      case RelaxedTruncZeroSVecF64x2ToVecI32x4:
      case RelaxedTruncZeroUVecF64x2ToVecI32x4:
      case TruncSatSVecF16x8ToVecI16x8:
      case TruncSatUVecF16x8ToVecI16x8:
      case ConvertSVecI16x8ToVecF16x8:
      case ConvertUVecI16x8ToVecF16x8:
      case AnyTrueVec128:
      case AllTrueVecI8x16:
      case AllTrueVecI16x8:
      case AllTrueVecI32x4:
      case AllTrueVecI64x2:
      case BitmaskVecI8x16:
      case BitmaskVecI16x8:
      case BitmaskVecI32x4:
      case BitmaskVecI64x2:
        note(&curr->value, Type::v128);
        break;
      case InvalidUnary:
        WASM_UNREACHABLE("invalid unary op");
    }
  }

  void visitBinary(Binary* curr) {
    switch (curr->op) {
      case AddInt32:
      case SubInt32:
      case MulInt32:
      case DivSInt32:
      case DivUInt32:
      case RemSInt32:
      case RemUInt32:
      case AndInt32:
      case OrInt32:
      case XorInt32:
      case ShlInt32:
      case ShrUInt32:
      case ShrSInt32:
      case RotLInt32:
      case RotRInt32:
      case EqInt32:
      case NeInt32:
      case LtSInt32:
      case LtUInt32:
      case LeSInt32:
      case LeUInt32:
      case GtSInt32:
      case GtUInt32:
      case GeSInt32:
      case GeUInt32:
        note(&curr->left, Type::i32);
        note(&curr->right, Type::i32);
        break;
      case AddInt64:
      case SubInt64:
      case MulInt64:
      case DivSInt64:
      case DivUInt64:
      case RemSInt64:
      case RemUInt64:
      case AndInt64:
      case OrInt64:
      case XorInt64:
      case ShlInt64:
      case ShrUInt64:
      case ShrSInt64:
      case RotLInt64:
      case RotRInt64:
      case EqInt64:
      case NeInt64:
      case LtSInt64:
      case LtUInt64:
      case LeSInt64:
      case LeUInt64:
      case GtSInt64:
      case GtUInt64:
      case GeSInt64:
      case GeUInt64:
        note(&curr->left, Type::i64);
        note(&curr->right, Type::i64);
        break;
      case AddFloat32:
      case SubFloat32:
      case MulFloat32:
      case DivFloat32:
      case CopySignFloat32:
      case MinFloat32:
      case MaxFloat32:
      case EqFloat32:
      case NeFloat32:
      case LtFloat32:
      case LeFloat32:
      case GtFloat32:
      case GeFloat32:
        note(&curr->left, Type::f32);
        note(&curr->right, Type::f32);
        break;
      case AddFloat64:
      case SubFloat64:
      case MulFloat64:
      case DivFloat64:
      case CopySignFloat64:
      case MinFloat64:
      case MaxFloat64:
      case EqFloat64:
      case NeFloat64:
      case LtFloat64:
      case LeFloat64:
      case GtFloat64:
      case GeFloat64:
        note(&curr->left, Type::f64);
        note(&curr->right, Type::f64);
        break;
      case EqVecI8x16:
      case NeVecI8x16:
      case LtSVecI8x16:
      case LtUVecI8x16:
      case LeSVecI8x16:
      case LeUVecI8x16:
      case GtSVecI8x16:
      case GtUVecI8x16:
      case GeSVecI8x16:
      case GeUVecI8x16:
      case EqVecI16x8:
      case NeVecI16x8:
      case LtSVecI16x8:
      case LtUVecI16x8:
      case LeSVecI16x8:
      case LeUVecI16x8:
      case GtSVecI16x8:
      case GtUVecI16x8:
      case GeSVecI16x8:
      case GeUVecI16x8:
      case EqVecI32x4:
      case NeVecI32x4:
      case LtSVecI32x4:
      case LtUVecI32x4:
      case LeSVecI32x4:
      case LeUVecI32x4:
      case GtSVecI32x4:
      case GtUVecI32x4:
      case GeSVecI32x4:
      case GeUVecI32x4:
      case EqVecI64x2:
      case NeVecI64x2:
      case LtSVecI64x2:
      case LeSVecI64x2:
      case GtSVecI64x2:
      case GeSVecI64x2:
      case EqVecF16x8:
      case NeVecF16x8:
      case LtVecF16x8:
      case LeVecF16x8:
      case GtVecF16x8:
      case GeVecF16x8:
      case EqVecF32x4:
      case NeVecF32x4:
      case LtVecF32x4:
      case LeVecF32x4:
      case GtVecF32x4:
      case GeVecF32x4:
      case EqVecF64x2:
      case NeVecF64x2:
      case LtVecF64x2:
      case LeVecF64x2:
      case GtVecF64x2:
      case GeVecF64x2:
      case AndVec128:
      case OrVec128:
      case XorVec128:
      case AndNotVec128:
      case AddVecI8x16:
      case AddSatSVecI8x16:
      case AddSatUVecI8x16:
      case SubVecI8x16:
      case SubSatSVecI8x16:
      case SubSatUVecI8x16:
      case MinSVecI8x16:
      case MinUVecI8x16:
      case MaxSVecI8x16:
      case MaxUVecI8x16:
      case AvgrUVecI8x16:
      case Q15MulrSatSVecI16x8:
      case ExtMulLowSVecI16x8:
      case ExtMulHighSVecI16x8:
      case ExtMulLowUVecI16x8:
      case ExtMulHighUVecI16x8:
      case AddVecI16x8:
      case AddSatSVecI16x8:
      case AddSatUVecI16x8:
      case SubVecI16x8:
      case SubSatSVecI16x8:
      case SubSatUVecI16x8:
      case MulVecI16x8:
      case MinSVecI16x8:
      case MinUVecI16x8:
      case MaxSVecI16x8:
      case MaxUVecI16x8:
      case AvgrUVecI16x8:
      case AddVecI32x4:
      case SubVecI32x4:
      case MulVecI32x4:
      case MinSVecI32x4:
      case MinUVecI32x4:
      case MaxSVecI32x4:
      case MaxUVecI32x4:
      case DotSVecI16x8ToVecI32x4:
      case ExtMulLowSVecI32x4:
      case ExtMulHighSVecI32x4:
      case ExtMulLowUVecI32x4:
      case ExtMulHighUVecI32x4:
      case AddVecI64x2:
      case SubVecI64x2:
      case MulVecI64x2:
      case ExtMulLowSVecI64x2:
      case ExtMulHighSVecI64x2:
      case ExtMulLowUVecI64x2:
      case ExtMulHighUVecI64x2:
      case AddVecF16x8:
      case SubVecF16x8:
      case MulVecF16x8:
      case DivVecF16x8:
      case MinVecF16x8:
      case MaxVecF16x8:
      case PMinVecF16x8:
      case PMaxVecF16x8:
      case AddVecF32x4:
      case SubVecF32x4:
      case MulVecF32x4:
      case DivVecF32x4:
      case MinVecF32x4:
      case MaxVecF32x4:
      case PMinVecF32x4:
      case PMaxVecF32x4:
      case RelaxedMinVecF32x4:
      case RelaxedMaxVecF32x4:
      case AddVecF64x2:
      case SubVecF64x2:
      case MulVecF64x2:
      case DivVecF64x2:
      case MinVecF64x2:
      case MaxVecF64x2:
      case PMinVecF64x2:
      case PMaxVecF64x2:
      case RelaxedMinVecF64x2:
      case RelaxedMaxVecF64x2:
      case NarrowSVecI16x8ToVecI8x16:
      case NarrowUVecI16x8ToVecI8x16:
      case NarrowSVecI32x4ToVecI16x8:
      case NarrowUVecI32x4ToVecI16x8:
      case SwizzleVecI8x16:
      case RelaxedSwizzleVecI8x16:
      case RelaxedQ15MulrSVecI16x8:
      case DotI8x16I7x16SToVecI16x8:
        note(&curr->left, Type::v128);
        note(&curr->right, Type::v128);
        break;
      case InvalidBinary:
        WASM_UNREACHABLE("invalid binary op");
    }
  }

  void visitSelect(Select* curr, std::optional<Type> type = std::nullopt) {
    if (type) {
      note(&curr->ifTrue, *type);
      note(&curr->ifFalse, *type);
    } else {
      noteAny(&curr->ifTrue);
      noteAny(&curr->ifFalse);
    }
    note(&curr->condition, Type::i32);
  }

  void visitDrop(Drop* curr, std::optional<Index> arity = std::nullopt) {
    if (!arity) {
      arity = curr->value->type.size();
    }
    if (*arity > 1) {
      noteAnyTuple(&curr->value, *arity);
    } else {
      noteAny(&curr->value);
    }
  }

  void visitReturn(Return* curr) {
    assert(func);
    auto type = func->getResults();
    if (type != Type::none) {
      note(&curr->value, type);
    }
  }

  void visitMemorySize(MemorySize* curr) {}

  void visitMemoryGrow(MemoryGrow* curr) {
    notePointer(&curr->delta, curr->memory);
  }

  void visitUnreachable(Unreachable* curr) {}

  void visitPop(Pop* curr) {}

  void visitRefNull(RefNull* curr) {}

  void visitRefIsNull(RefIsNull* curr) { noteAnyReference(&curr->value); }

  void visitRefFunc(RefFunc* curr) {}

  void visitRefEq(RefEq* curr) {
    Type eqref(HeapType::eq, Nullable);
    note(&curr->left, eqref);
    note(&curr->right, eqref);
  }

  void visitTableGet(TableGet* curr) { note(&curr->index, Type::i32); }

  void visitTableSet(TableSet* curr) {
    note(&curr->index, Type::i32);
    note(&curr->value, wasm.getTable(curr->table)->type);
  }

  void visitTableSize(TableSize* curr) {}

  void visitTableGrow(TableGrow* curr) {
    note(&curr->value, wasm.getTable(curr->table)->type);
    note(&curr->delta, Type::i32);
  }

  void visitTableFill(TableFill* curr) {
    auto type = wasm.getTable(curr->table)->type;
    note(&curr->dest, Type::i32);
    note(&curr->value, type);
    note(&curr->size, Type::i32);
  }

  void visitTableCopy(TableCopy* curr) {
    note(&curr->dest, Type::i32);
    note(&curr->source, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitTableInit(TableInit* curr) {
    note(&curr->dest, wasm.getTable(curr->table)->addressType);
    note(&curr->offset, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitTry(Try* curr) {
    note(&curr->body, curr->type);
    for (auto& expr : curr->catchBodies) {
      note(&expr, curr->type);
    }
  }

  void visitTryTable(TryTable* curr) { note(&curr->body, curr->type); }

  void visitThrow(Throw* curr) {
    auto type = wasm.getTag(curr->tag)->sig.params;
    assert(curr->operands.size() == type.size());
    for (size_t i = 0; i < type.size(); ++i) {
      note(&curr->operands[i], type[i]);
    }
  }

  void visitRethrow(Rethrow* curr) {}

  void visitThrowRef(ThrowRef* curr) {
    note(&curr->exnref, Type(HeapType::exn, Nullable));
  }

  void visitTupleMake(TupleMake* curr) {
    for (auto& expr : curr->operands) {
      noteAny(&expr);
    }
  }

  void visitTupleExtract(TupleExtract* curr,
                         std::optional<size_t> arity = std::nullopt) {
    if (!arity) {
      assert(curr->tuple->type.isTuple());
      arity = curr->tuple->type.size();
    }
    noteAnyTuple(&curr->tuple, *arity);
  }

  void visitRefI31(RefI31* curr) { note(&curr->value, Type::i32); }

  void visitI31Get(I31Get* curr) {
    note(&curr->i31, Type(HeapType::i31, Nullable));
  }

  void visitCallRef(CallRef* curr, std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->target->type.getHeapType().getSignature();
    }
    auto params = ht->getSignature().params;
    assert(curr->operands.size() == params.size());
    for (size_t i = 0; i < params.size(); ++i) {
      note(&curr->operands[i], params[i]);
    }
    note(&curr->target, Type(*ht, Nullable));
  }

  void visitRefTest(RefTest* curr) {
    auto top = curr->castType.getHeapType().getTop();
    note(&curr->ref, Type(top, Nullable));
  }

  void visitRefCast(RefCast* curr) {
    auto top = curr->type.getHeapType().getTop();
    note(&curr->ref, Type(top, Nullable));
  }

  void visitBrOn(BrOn* curr) {
    switch (curr->op) {
      case BrOnNull:
      case BrOnNonNull:
        noteAnyReference(&curr->ref);
        return;
      case BrOnCast:
      case BrOnCastFail: {
        auto top = curr->castType.getHeapType().getTop();
        note(&curr->ref, Type(top, Nullable));
        return;
      }
    }
    WASM_UNREACHABLE("unexpected op");
  }

  void visitStructNew(StructNew* curr) {
    if (curr->isWithDefault()) {
      return;
    }
    const auto& fields = curr->type.getHeapType().getStruct().fields;
    assert(fields.size() == curr->operands.size());
    for (size_t i = 0; i < fields.size(); ++i) {
      note(&curr->operands[i], fields[i].type);
    }
  }

  void visitStructGet(StructGet* curr,
                      std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    note(&curr->ref, Type(*ht, Nullable));
  }

  void visitStructSet(StructSet* curr,
                      std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    const auto& fields = ht->getStruct().fields;
    assert(curr->index < fields.size());
    note(&curr->ref, Type(*ht, Nullable));
    note(&curr->value, fields[curr->index].type);
  }

  void visitArrayNew(ArrayNew* curr) {
    if (!curr->isWithDefault()) {
      note(&curr->init, curr->type.getHeapType().getArray().element.type);
    }
    note(&curr->size, Type::i32);
  }

  void visitArrayNewData(ArrayNewData* curr) {
    note(&curr->offset, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitArrayNewElem(ArrayNewElem* curr) {
    note(&curr->offset, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitArrayNewFixed(ArrayNewFixed* curr) {
    auto type = curr->type.getHeapType().getArray().element.type;
    for (auto& expr : curr->values) {
      note(&expr, type);
    }
  }

  void visitArrayGet(ArrayGet* curr,
                     std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    note(&curr->ref, Type(*ht, Nullable));
    note(&curr->index, Type::i32);
  }

  void visitArraySet(ArraySet* curr,
                     std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    auto type = ht->getArray().element.type;
    note(&curr->ref, Type(*ht, Nullable));
    note(&curr->index, Type::i32);
    note(&curr->value, type);
  }

  void visitArrayLen(ArrayLen* curr) {
    note(&curr->ref, Type(HeapType::array, Nullable));
  }

  void visitArrayCopy(ArrayCopy* curr,
                      std::optional<HeapType> dest = std::nullopt,
                      std::optional<HeapType> src = std::nullopt) {
    if (!dest) {
      dest = curr->destRef->type.getHeapType();
    }
    if (!src) {
      src = curr->srcRef->type.getHeapType();
    }
    note(&curr->destRef, Type(*dest, Nullable));
    note(&curr->destIndex, Type::i32);
    note(&curr->srcRef, Type(*src, Nullable));
    note(&curr->srcIndex, Type::i32);
    note(&curr->length, Type::i32);
  }

  void visitArrayFill(ArrayFill* curr,
                      std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    auto type = ht->getArray().element.type;
    note(&curr->ref, Type(*ht, Nullable));
    note(&curr->index, Type::i32);
    note(&curr->value, type);
    note(&curr->size, Type::i32);
  }

  void visitArrayInitData(ArrayInitData* curr,
                          std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    note(&curr->ref, Type(*ht, Nullable));
    note(&curr->index, Type::i32);
    note(&curr->offset, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitArrayInitElem(ArrayInitElem* curr,
                          std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->ref->type.getHeapType();
    }
    note(&curr->ref, Type(*ht, Nullable));
    note(&curr->index, Type::i32);
    note(&curr->offset, Type::i32);
    note(&curr->size, Type::i32);
  }

  void visitRefAs(RefAs* curr) {
    switch (curr->op) {
      case RefAsNonNull:
        noteAnyReference(&curr->value);
        return;
      case AnyConvertExtern:
        note(&curr->value, Type(HeapType::ext, Nullable));
        return;
      case ExternConvertAny:
        note(&curr->value, Type(HeapType::any, Nullable));
        return;
    }
    WASM_UNREACHABLE("unexpected op");
  }

  void visitStringNew(StringNew* curr,
                      std::optional<HeapType> ht = std::nullopt) {
    switch (curr->op) {
      case StringNewLossyUTF8Array:
      case StringNewWTF16Array:
        if (!ht) {
          ht = curr->ref->type.getHeapType();
        }
        note(&curr->ref, Type(*ht, Nullable));
        note(&curr->start, Type::i32);
        note(&curr->end, Type::i32);
        return;
      case StringNewFromCodePoint:
        note(&curr->ref, Type::i32);
        return;
    }
    WASM_UNREACHABLE("unexpected op");
  }

  void visitStringConst(StringConst* curr) {}

  void visitStringMeasure(StringMeasure* curr) {
    note(&curr->ref, Type(HeapType::string, Nullable));
  }

  void visitStringEncode(StringEncode* curr,
                         std::optional<HeapType> ht = std::nullopt) {
    if (!ht) {
      ht = curr->array->type.getHeapType();
    }
    note(&curr->str, Type(HeapType::string, Nullable));
    note(&curr->array, Type(*ht, Nullable));
    note(&curr->start, Type::i32);
  }

  void visitStringConcat(StringConcat* curr) {
    auto stringref = Type(HeapType::string, Nullable);
    note(&curr->left, stringref);
    note(&curr->right, stringref);
  }

  void visitStringEq(StringEq* curr) {
    auto stringref = Type(HeapType::string, Nullable);
    note(&curr->left, stringref);
    note(&curr->right, stringref);
  }

  void visitStringWTF16Get(StringWTF16Get* curr) {
    note(&curr->ref, Type(HeapType::string, Nullable));
    note(&curr->pos, Type::i32);
  }

  void visitStringSliceWTF(StringSliceWTF* curr) {
    note(&curr->ref, Type(HeapType::string, Nullable));
    note(&curr->start, Type::i32);
    note(&curr->end, Type::i32);
  }

  void visitContNew(ContNew* curr) { note(&curr->func, curr->type); }

  void visitContBind(ContBind* curr,
                     std::optional<HeapType> src = std::nullopt,
                     std::optional<HeapType> dest = std::nullopt) {
    if (!src.has_value()) {
      src = curr->cont->type.getHeapType();
    }
    if (!dest.has_value()) {
      dest = curr->type.getHeapType();
    }
    auto sourceParams = src->getContinuation().type.getSignature().params;
    auto targetParams = dest->getContinuation().type.getSignature().params;
    assert(sourceParams.size() >= targetParams.size());
    auto n = sourceParams.size() - targetParams.size();
    assert(curr->operands.size() == n);
    for (size_t i = 0; i < n; ++i) {
      note(&curr->operands[i], sourceParams[i]);
    }
    note(&curr->cont, Type(*src, Nullable));
  }

  void visitSuspend(Suspend* curr) {
    auto params = wasm.getTag(curr->tag)->sig.params;
    assert(params.size() == curr->operands.size());
    for (size_t i = 0; i < params.size(); ++i) {
      note(&curr->operands[i], params[i]);
    }
  }

  void visitResume(Resume* curr, std::optional<HeapType> ct = std::nullopt) {
    if (!ct.has_value()) {
      ct = curr->cont->type.getHeapType();
    }
    assert(ct->isContinuation());
    auto params = ct->getContinuation().type.getSignature().params;
    assert(params.size() == curr->operands.size());
    for (size_t i = 0; i < params.size(); ++i) {
      note(&curr->operands[i], params[i]);
    }
    note(&curr->cont, Type(*ct, Nullable));
  }

  void visitResumeThrow(ResumeThrow* curr,
                        std::optional<HeapType> ct = std::nullopt) {
    if (!ct.has_value()) {
      ct = curr->cont->type.getHeapType();
    }
    assert(ct->isContinuation());
    auto params = wasm.getTag(curr->tag)->sig.params;
    assert(params.size() == curr->operands.size());
    for (size_t i = 0; i < params.size(); ++i) {
      note(&curr->operands[i], params[i]);
    }
    note(&curr->cont, Type(*ct, Nullable));
  }

  void visitStackSwitch(StackSwitch* curr,
                        std::optional<HeapType> ct = std::nullopt) {
    if (!ct.has_value()) {
      ct = curr->cont->type.getHeapType();
    }
    assert(ct->isContinuation());
    auto params = ct->getContinuation().type.getSignature().params;
    assert(params.size() >= 1 &&
           ((params.size() - 1) == curr->operands.size()));
    for (size_t i = 0; i < params.size() - 1; ++i) {
      note(&curr->operands[i], params[i]);
    }
    note(&curr->cont, Type(*ct, Nullable));
  }
};

} // namespace wasm

#endif // wasm_ir_child_typer_h
