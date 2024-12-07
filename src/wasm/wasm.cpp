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

#include "wasm.h"
#include "ir/branch-utils.h"
#include "wasm-traversal.h"
#include "wasm-type.h"

namespace wasm {

// shared constants

Name WASM("wasm");
Name RETURN_FLOW("*return:)*");
Name RETURN_CALL_FLOW("*return-call:)*");
Name NONCONSTANT_FLOW("*nonconstant:)*");

namespace BinaryConsts {
namespace CustomSections {
const char* Name = "name";
const char* SourceMapUrl = "sourceMappingURL";
const char* Dylink = "dylink";
const char* Dylink0 = "dylink.0";
const char* Linking = "linking";
const char* Producers = "producers";
const char* BuildId = "build_id";
const char* TargetFeatures = "target_features";
const char* AtomicsFeature = "atomics";
const char* BulkMemoryFeature = "bulk-memory";
const char* ExceptionHandlingFeature = "exception-handling";
const char* MutableGlobalsFeature = "mutable-globals";
const char* TruncSatFeature = "nontrapping-fptoint";
const char* SignExtFeature = "sign-ext";
const char* SIMD128Feature = "simd128";
const char* TailCallFeature = "tail-call";
const char* ReferenceTypesFeature = "reference-types";
const char* MultivalueFeature = "multivalue";
const char* GCFeature = "gc";
const char* Memory64Feature = "memory64";
const char* RelaxedSIMDFeature = "relaxed-simd";
const char* ExtendedConstFeature = "extended-const";
const char* StringsFeature = "strings";
const char* MultiMemoryFeature = "multimemory";
const char* StackSwitchingFeature = "stack-switching";
const char* SharedEverythingFeature = "shared-everything";
const char* FP16Feature = "fp16";
const char* BulkMemoryOptFeature = "bulk-memory-opt";
const char* CallIndirectOverlongFeature = "call-indirect-overlong";
} // namespace CustomSections
} // namespace BinaryConsts

Name STACK_POINTER("__stack_pointer");
Name MODULE("module");
Name START("start");
Name GLOBAL("global");
Name FUNC("func");
Name CONT("cont");
Name PARAM("param");
Name RESULT("result");
Name MEMORY("memory");
Name DATA("data");
Name PASSIVE("passive");
Name EXPORT("export");
Name IMPORT("import");
Name TABLE("table");
Name ELEM("elem");
Name DECLARE("declare");
Name OFFSET("offset");
Name ITEM("item");
Name LOCAL("local");
Name TYPE("type");
Name REF("ref");
Name NULL_("null");
Name CALL("call");
Name CALL_INDIRECT("call_indirect");
Name BLOCK("block");
Name BR_IF("br_if");
Name THEN("then");
Name ELSE("else");
Name _NAN("NaN");
Name _INFINITY("Infinity");
Name NEG_INFINITY("-infinity");
Name NEG_NAN("-nan");
Name CASE("case");
Name BR("br");
Name FUNCREF("funcref");
Name FAKE_RETURN("__binaryen_fake_return");
Name DELEGATE_CALLER_TARGET("__binaryen_delegate_caller_target");
Name MUT("mut");
Name SPECTEST("spectest");
Name PRINT("print");
Name EXIT("exit");
Name SHARED("shared");
Name TAG("tag");
Name TUPLE("tuple");

// Expressions

void Expression::dump() { std::cout << *this << '\n'; }

const char* getExpressionName(Expression* curr) {
  switch (curr->_id) {

#define DELEGATE(CLASS_TO_VISIT)                                               \
  case Expression::Id::CLASS_TO_VISIT##Id:                                     \
    return #CLASS_TO_VISIT;

#include "wasm-delegations.def"

    default:
      WASM_UNREACHABLE("invalid id");
  }
}

Literal getLiteralFromConstExpression(Expression* curr) {
  // TODO: Do we need this function given that Properties::getLiteral
  // (currently) does the same?
  assert(Properties::isConstantExpression(curr));
  return Properties::getLiteral(curr);
}

Literals getLiteralsFromConstExpression(Expression* curr) {
  // TODO: Do we need this function given that Properties::getLiterals
  // (currently) does the same?
  if (auto* t = curr->dynCast<TupleMake>()) {
    Literals values;
    for (auto* operand : t->operands) {
      values.push_back(getLiteralFromConstExpression(operand));
    }
    return values;
  } else {
    return {getLiteralFromConstExpression(curr)};
  }
}

// a block is unreachable if one of its elements is unreachable,
// and there are no branches to it

static void handleUnreachable(Block* block, Block::Breakability breakability) {
  if (block->type == Type::unreachable) {
    return; // nothing to do
  }
  if (block->list.size() == 0) {
    return; // nothing to do
  }
  // if we are concrete, stop - even an unreachable child
  // won't change that (since we have a break with a value,
  // or the final child flows out a value)
  if (block->type.isConcrete()) {
    return;
  }
  // look for an unreachable child
  for (auto* child : block->list) {
    if (child->type == Type::unreachable) {
      // there is an unreachable child, so we are unreachable, unless we have a
      // break
      if (breakability == Block::Unknown) {
        breakability = BranchUtils::BranchSeeker::has(block, block->name)
                         ? Block::HasBreak
                         : Block::NoBreak;
      }
      if (breakability == Block::NoBreak) {
        block->type = Type::unreachable;
      }
      return;
    }
  }
}

void Block::finalize(std::optional<Type> type_, Breakability breakability) {
  if (type_) {
    type = *type_;
    if (type == Type::none && list.size() > 0) {
      handleUnreachable(this, breakability);
    }
    return;
  }

  if (list.size() == 0) {
    type = Type::none;
    return;
  }
  // The default type is what is at the end. Next we need to see if breaks
  // and/ or unreachability change that.
  type = list.back()->type;
  if (!name.is()) {
    // Nothing branches here, so this is easy.
    handleUnreachable(this, NoBreak);
    return;
  }

  // The default type is according to the value that flows out.
  BranchUtils::BranchSeeker seeker(this->name);
  Expression* temp = this;
  seeker.walk(temp);
  if (seeker.found) {
    // Calculate the LUB of the branch types and the flowed-out type.
    seeker.types.insert(type);
    type = Type::getLeastUpperBound(seeker.types);
  } else {
    // There are no branches, so this block may be unreachable.
    handleUnreachable(this, NoBreak);
  }
}

void If::finalize(std::optional<Type> type_) {
  // The If is unreachable if the condition is unreachable or both arms are
  // unreachable.
  if (condition->type == Type::unreachable ||
      (ifFalse && ifTrue->type == Type::unreachable &&
       ifFalse->type == Type::unreachable)) {
    type = Type::unreachable;
    return;
  }

  if (type_) {
    type = *type_;
  } else {
    type = ifFalse ? Type::getLeastUpperBound(ifTrue->type, ifFalse->type)
                   : Type::none;
  }
}

void Loop::finalize(std::optional<Type> type_) {
  if (type_) {
    type = *type_;
    if (type == Type::none && body->type == Type::unreachable) {
      type = Type::unreachable;
    }
  } else {
    type = body->type;
  }
}

void Break::finalize() {
  if (condition) {
    if (condition->type == Type::unreachable) {
      type = Type::unreachable;
    } else if (value) {
      // N.B. This is not correct wrt the spec, which mandates that it be the
      // type of the block we target. In practice this does not matter because
      // the br_if return value is not really used in the wild. To fix this,
      // we'd need to do something like what we do for local.tee's type, which
      // is to fix it up in a way that is aware of function-level context and
      // not just the instruction itself (which would be a pain).
      type = value->type;
    } else {
      type = Type::none;
    }
  } else {
    type = Type::unreachable;
  }
}

void Switch::finalize() { type = Type::unreachable; }

// Sets the type to unreachable if there is an unreachable operand. Returns true
// if so.
template<typename T> bool handleUnreachableOperands(T* curr) {
  for (auto* child : curr->operands) {
    if (child->type == Type::unreachable) {
      curr->type = Type::unreachable;
      return true;
    }
  }
  return false;
}

void Call::finalize() {
  handleUnreachableOperands(this);
  if (isReturn) {
    type = Type::unreachable;
  }
}

void CallIndirect::finalize() {
  type = heapType.getSignature().results;
  handleUnreachableOperands(this);
  if (isReturn) {
    type = Type::unreachable;
  }
  if (target->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

bool LocalSet::isTee() const { return type != Type::none; }

// Changes to local.tee. The type of the local should be given.
void LocalSet::makeTee(Type type_) {
  type = type_;
  finalize(); // type may need to be unreachable
}

// Changes to local.set.
void LocalSet::makeSet() {
  type = Type::none;
  finalize(); // type may need to be unreachable
}

void LocalSet::finalize() {
  if (value->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void GlobalSet::finalize() {
  if (value->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void Load::finalize() {
  if (ptr->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void Store::finalize() {
  assert(valueType != Type::none); // must be set
  if (ptr->type == Type::unreachable || value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void AtomicRMW::finalize() {
  if (ptr->type == Type::unreachable || value->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void AtomicCmpxchg::finalize() {
  if (ptr->type == Type::unreachable || expected->type == Type::unreachable ||
      replacement->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void AtomicWait::finalize() {
  type = Type::i32;
  if (ptr->type == Type::unreachable || expected->type == Type::unreachable ||
      timeout->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void AtomicNotify::finalize() {
  type = Type::i32;
  if (ptr->type == Type::unreachable ||
      notifyCount->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void AtomicFence::finalize() { type = Type::none; }

void SIMDExtract::finalize() {
  assert(vec);
  switch (op) {
    case ExtractLaneSVecI8x16:
    case ExtractLaneUVecI8x16:
    case ExtractLaneSVecI16x8:
    case ExtractLaneUVecI16x8:
    case ExtractLaneVecI32x4:
      type = Type::i32;
      break;
    case ExtractLaneVecI64x2:
      type = Type::i64;
      break;
    case ExtractLaneVecF16x8:
    case ExtractLaneVecF32x4:
      type = Type::f32;
      break;
    case ExtractLaneVecF64x2:
      type = Type::f64;
      break;
    default:
      WASM_UNREACHABLE("unexpected op");
  }
  if (vec->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void SIMDReplace::finalize() {
  assert(vec && value);
  type = Type::v128;
  if (vec->type == Type::unreachable || value->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void SIMDShuffle::finalize() {
  assert(left && right);
  type = Type::v128;
  if (left->type == Type::unreachable || right->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void SIMDTernary::finalize() {
  assert(a && b && c);
  type = Type::v128;
  if (a->type == Type::unreachable || b->type == Type::unreachable ||
      c->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void MemoryInit::finalize() {
  assert(dest && offset && size);
  type = Type::none;
  if (dest->type == Type::unreachable || offset->type == Type::unreachable ||
      size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void DataDrop::finalize() { type = Type::none; }

void MemoryCopy::finalize() {
  assert(dest && source && size);
  type = Type::none;
  if (dest->type == Type::unreachable || source->type == Type::unreachable ||
      size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void MemoryFill::finalize() {
  assert(dest && value && size);
  type = Type::none;
  if (dest->type == Type::unreachable || value->type == Type::unreachable ||
      size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void SIMDShift::finalize() {
  assert(vec && shift);
  type = Type::v128;
  if (vec->type == Type::unreachable || shift->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void SIMDLoad::finalize() {
  assert(ptr);
  type = Type::v128;
  if (ptr->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

Index SIMDLoad::getMemBytes() {
  switch (op) {
    case Load8SplatVec128:
      return 1;
    case Load16SplatVec128:
      return 2;
    case Load32SplatVec128:
    case Load32ZeroVec128:
      return 4;
    case Load64SplatVec128:
    case Load8x8SVec128:
    case Load8x8UVec128:
    case Load16x4SVec128:
    case Load16x4UVec128:
    case Load32x2SVec128:
    case Load32x2UVec128:
    case Load64ZeroVec128:
      return 8;
  }
  WASM_UNREACHABLE("unexpected op");
}

void SIMDLoadStoreLane::finalize() {
  assert(ptr && vec);
  type = isLoad() ? Type::v128 : Type::none;
  if (ptr->type == Type::unreachable || vec->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

Index SIMDLoadStoreLane::getMemBytes() {
  switch (op) {
    case Load8LaneVec128:
    case Store8LaneVec128:
      return 1;
    case Load16LaneVec128:
    case Store16LaneVec128:
      return 2;
    case Load32LaneVec128:
    case Store32LaneVec128:
      return 4;
    case Load64LaneVec128:
    case Store64LaneVec128:
      return 8;
  }
  WASM_UNREACHABLE("unexpected op");
}

bool SIMDLoadStoreLane::isStore() {
  switch (op) {
    case Store8LaneVec128:
    case Store16LaneVec128:
    case Store32LaneVec128:
    case Store64LaneVec128:
      return true;
    case Load16LaneVec128:
    case Load32LaneVec128:
    case Load64LaneVec128:
    case Load8LaneVec128:
      return false;
  }
  WASM_UNREACHABLE("unexpected op");
}

Const* Const::set(Literal value_) {
  value = value_;
  type = value.type;
  return this;
}

void Const::finalize() { type = value.type; }

bool Unary::isRelational() { return op == EqZInt32 || op == EqZInt64; }

void Unary::finalize() {
  if (value->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  switch (op) {
    case ClzInt32:
    case CtzInt32:
    case PopcntInt32:
    case NegFloat32:
    case AbsFloat32:
    case CeilFloat32:
    case FloorFloat32:
    case TruncFloat32:
    case NearestFloat32:
    case SqrtFloat32:
    case ClzInt64:
    case CtzInt64:
    case PopcntInt64:
    case NegFloat64:
    case AbsFloat64:
    case CeilFloat64:
    case FloorFloat64:
    case TruncFloat64:
    case NearestFloat64:
    case SqrtFloat64:
      type = value->type;
      break;
    case EqZInt32:
    case EqZInt64:
      type = Type::i32;
      break;
    case ExtendS8Int32:
    case ExtendS16Int32:
      type = Type::i32;
      break;
    case ExtendSInt32:
    case ExtendUInt32:
    case ExtendS8Int64:
    case ExtendS16Int64:
    case ExtendS32Int64:
      type = Type::i64;
      break;
    case WrapInt64:
      type = Type::i32;
      break;
    case PromoteFloat32:
      type = Type::f64;
      break;
    case DemoteFloat64:
      type = Type::f32;
      break;
    case TruncSFloat32ToInt32:
    case TruncUFloat32ToInt32:
    case TruncSFloat64ToInt32:
    case TruncUFloat64ToInt32:
    case TruncSatSFloat32ToInt32:
    case TruncSatUFloat32ToInt32:
    case TruncSatSFloat64ToInt32:
    case TruncSatUFloat64ToInt32:
    case ReinterpretFloat32:
      type = Type::i32;
      break;
    case TruncSFloat32ToInt64:
    case TruncUFloat32ToInt64:
    case TruncSFloat64ToInt64:
    case TruncUFloat64ToInt64:
    case TruncSatSFloat32ToInt64:
    case TruncSatUFloat32ToInt64:
    case TruncSatSFloat64ToInt64:
    case TruncSatUFloat64ToInt64:
    case ReinterpretFloat64:
      type = Type::i64;
      break;
    case ReinterpretInt32:
    case ConvertSInt32ToFloat32:
    case ConvertUInt32ToFloat32:
    case ConvertSInt64ToFloat32:
    case ConvertUInt64ToFloat32:
      type = Type::f32;
      break;
    case ReinterpretInt64:
    case ConvertSInt32ToFloat64:
    case ConvertUInt32ToFloat64:
    case ConvertSInt64ToFloat64:
    case ConvertUInt64ToFloat64:
      type = Type::f64;
      break;
    case SplatVecI8x16:
    case SplatVecI16x8:
    case SplatVecI32x4:
    case SplatVecI64x2:
    case SplatVecF16x8:
    case SplatVecF32x4:
    case SplatVecF64x2:
    case NotVec128:
    case AbsVecI8x16:
    case AbsVecI16x8:
    case AbsVecI32x4:
    case AbsVecI64x2:
    case PopcntVecI8x16:
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
      type = Type::v128;
      break;
    case AnyTrueVec128:
    case AllTrueVecI8x16:
    case AllTrueVecI16x8:
    case AllTrueVecI32x4:
    case AllTrueVecI64x2:
    case BitmaskVecI8x16:
    case BitmaskVecI16x8:
    case BitmaskVecI32x4:
    case BitmaskVecI64x2:
      type = Type::i32;
      break;

    case InvalidUnary:
      WASM_UNREACHABLE("invalid unary op");
  }
}

bool Binary::isRelational() {
  switch (op) {
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
    case EqFloat32:
    case NeFloat32:
    case LtFloat32:
    case LeFloat32:
    case GtFloat32:
    case GeFloat32:
    case EqFloat64:
    case NeFloat64:
    case LtFloat64:
    case LeFloat64:
    case GtFloat64:
    case GeFloat64:
      return true;
    default:
      return false;
  }
}

void Binary::finalize() {
  assert(left && right);
  if (left->type == Type::unreachable || right->type == Type::unreachable) {
    type = Type::unreachable;
  } else if (isRelational()) {
    type = Type::i32;
  } else {
    type = left->type;
  }
}

void Select::finalize() {
  assert(ifTrue && ifFalse);
  if (ifTrue->type == Type::unreachable || ifFalse->type == Type::unreachable ||
      condition->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::getLeastUpperBound(ifTrue->type, ifFalse->type);
  }
}

void Drop::finalize() {
  if (value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void MemorySize::finalize() {}

void MemoryGrow::finalize() {
  if (delta->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void RefNull::finalize(HeapType heapType) {
  assert(heapType.isBottom());
  type = Type(heapType, Nullable);
}

void RefNull::finalize(Type type_) { type = type_; }

void RefNull::finalize() {}

void RefIsNull::finalize() {
  if (value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void RefFunc::finalize() {
  // No-op. We assume that the full proper typed function type has been applied
  // previously.
}

void RefFunc::finalize(Type type_) { type = type_; }

void RefEq::finalize() {
  if (left->type == Type::unreachable || right->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void TableGet::finalize() {
  if (index->type == Type::unreachable) {
    type = Type::unreachable;
  }
  // Otherwise, the type should have been set already.
}

void TableSet::finalize() {
  if (index->type == Type::unreachable || value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void TableSize::finalize() {
  // Nothing to do - the type must have been set already during construction.
}

void TableGrow::finalize() {
  if (delta->type == Type::unreachable || value->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void TableFill::finalize() {
  if (dest->type == Type::unreachable || value->type == Type::unreachable ||
      size->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void TableCopy::finalize() {
  type = Type::none;
  if (dest->type == Type::unreachable || source->type == Type::unreachable ||
      size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void TableInit::finalize() {
  type = Type::none;
  if (dest->type == Type::unreachable || offset->type == Type::unreachable ||
      size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void Try::finalize(std::optional<Type> type_) {
  if (type_) {
    type = *type_;
    bool allUnreachable = body->type == Type::unreachable;
    for (auto catchBody : catchBodies) {
      allUnreachable &= catchBody->type == Type::unreachable;
    }
    if (type == Type::none && allUnreachable) {
      type = Type::unreachable;
    }

  } else {
    // Calculate the LUB of catch bodies' types.
    std::unordered_set<Type> types{body->type};
    types.reserve(catchBodies.size());
    for (auto catchBody : catchBodies) {
      types.insert(catchBody->type);
    }
    type = Type::getLeastUpperBound(types);
  }
}

void Throw::finalize() { type = Type::unreachable; }

void Rethrow::finalize() { type = Type::unreachable; }

void ThrowRef::finalize() { type = Type::unreachable; }

bool TryTable::hasCatchAll() const {
  return std::any_of(
    catchTags.begin(), catchTags.end(), [](Name t) { return !t; });
}

static void populateTryTableSentTypes(TryTable* curr, Module* wasm) {
  if (!wasm) {
    return;
  }
  curr->sentTypes.clear();
  // We always use the refined non-nullable type in our IR, which is what the
  // wasm spec defines when GC is enabled (=== non-nullable types are allowed).
  // If GC is not enabled then we emit a nullable type in the binary format in
  // WasmBinaryWriter::writeType.
  Type exnref = Type(HeapType::exn, NonNullable);
  for (Index i = 0; i < curr->catchTags.size(); i++) {
    auto tagName = curr->catchTags[i];
    std::vector<Type> sentType;
    if (tagName) {
      for (auto t : wasm->getTag(tagName)->sig.params) {
        sentType.push_back(t);
      }
    }
    if (curr->catchRefs[i]) {
      sentType.push_back(exnref);
    }
    curr->sentTypes.push_back(sentType.empty() ? Type::none : Type(sentType));
  }
}

void TryTable::finalize(std::optional<Type> type_, Module* wasm) {
  if (type_) {
    type = *type_;
    if (type == Type::none && body->type == Type::unreachable) {
      type = Type::unreachable;
    }
  } else {
    type = body->type;
  }
  populateTryTableSentTypes(this, wasm);
}

void TupleMake::finalize() {
  std::vector<Type> types;
  types.reserve(operands.size());
  for (auto* op : operands) {
    if (op->type == Type::unreachable) {
      type = Type::unreachable;
      return;
    }
    types.push_back(op->type);
  }
  type = Type(types);
}

void TupleExtract::finalize() {
  if (tuple->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    assert(index < tuple->type.size());
    type = tuple->type[index];
  }
}

void RefI31::finalize() {
  if (value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    assert(type.isRef() && type.getHeapType().isMaybeShared(HeapType::i31));
  }
}

void I31Get::finalize() {
  if (i31->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void CallRef::finalize() {
  if (handleUnreachableOperands(this)) {
    return;
  }
  if (isReturn) {
    type = Type::unreachable;
    return;
  }
  if (target->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  assert(target->type.isRef());
  if (target->type.isNull()) {
    // If this call_ref has been optimized to have a null reference, then it
    // will definitely trap. We could update the type to be unreachable, but
    // that would violate the invariant that non-branch instructions other than
    // `unreachable` can only be unreachable if they have unreachable children.
    // Make the result type as close to `unreachable` as possible without
    // actually making it unreachable. TODO: consider just making this
    // unreachable instead (and similar in other GC accessors), although this
    // would currently cause the parser to admit more invalid modules.
    if (type.isRef()) {
      type = Type(type.getHeapType().getBottom(), NonNullable);
    } else if (type.isTuple()) {
      Tuple elems;
      for (auto t : type) {
        elems.push_back(
          t.isRef() ? Type(t.getHeapType().getBottom(), NonNullable) : t);
      }
      type = Type(elems);
    }
    return;
  }
  assert(target->type.getHeapType().isSignature());
  type = target->type.getHeapType().getSignature().results;
}

void RefTest::finalize() {
  if (ref->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
    // Do not unnecessarily lose type information.
    castType = Type::getGreatestLowerBound(castType, ref->type);
  }
}

void RefCast::finalize() {
  if (ref->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }

  // We reach this before validation, so the input type might be totally wrong.
  // Return early in this case to avoid doing the wrong thing below.
  if (!ref->type.isRef()) {
    return;
  }

  // Do not unnecessarily lose type information. We could leave this for
  // optimizations (and indeed we do a more powerful version of this in
  // OptimizeInstructions), but doing it here as part of
  // finalization/refinalization ensures that type information flows through in
  // an optimal manner and can be used as soon as possible.
  type = Type::getGreatestLowerBound(type, ref->type);
}

void BrOn::finalize() {
  if (ref->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  if (op == BrOnCast || op == BrOnCastFail) {
    // The cast type must be a subtype of the input type. If we've refined the
    // input type so that this is no longer true, we can fix it by similarly
    // refining the cast type in a way that will not change the cast behavior.
    castType = Type::getGreatestLowerBound(castType, ref->type);
    assert(castType.isRef());
  }
  switch (op) {
    case BrOnNull:
      // If we do not branch, we flow out the existing value as non-null.
      type = Type(ref->type.getHeapType(), NonNullable);
      break;
    case BrOnNonNull:
      // If we do not branch, we flow out nothing (the spec could also have had
      // us flow out the null, but it does not).
      type = Type::none;
      break;
    case BrOnCast:
      if (castType.isNullable()) {
        // Nulls take the branch, so the result is non-nullable.
        type = Type(ref->type.getHeapType(), NonNullable);
      } else {
        // Nulls do not take the branch, so the result is non-nullable only if
        // the input is.
        type = ref->type;
      }
      break;
    case BrOnCastFail:
      if (castType.isNullable()) {
        // Nulls do not take the branch, so the result is non-nullable only if
        // the input is.
        type = Type(castType.getHeapType(), ref->type.getNullability());
      } else {
        // Nulls take the branch, so the result is non-nullable.
        type = castType;
      }
      break;
    default:
      WASM_UNREACHABLE("invalid br_on_*");
  }
}

Type BrOn::getSentType() {
  switch (op) {
    case BrOnNull:
      // BrOnNull does not send a value on the branch.
      return Type::none;
    case BrOnNonNull:
      // If the input is unreachable, the branch is not taken, and there is no
      // valid type we can report as being sent. Report it as unreachable.
      if (ref->type == Type::unreachable) {
        return Type::unreachable;
      }
      // BrOnNonNull sends the non-nullable type on the branch.
      return Type(ref->type.getHeapType(), NonNullable);
    case BrOnCast:
      // The same as the result type of br_on_cast_fail.
      if (castType.isNullable()) {
        return Type(castType.getHeapType(), ref->type.getNullability());
      } else {
        return castType;
      }
    case BrOnCastFail:
      // The same as the result type of br_on_cast (if reachable).
      if (ref->type == Type::unreachable) {
        return Type::unreachable;
      }
      if (castType.isNullable()) {
        return Type(ref->type.getHeapType(), NonNullable);
      } else {
        return ref->type;
      }
    default:
      WASM_UNREACHABLE("invalid br_on_*");
  }
}

void StructNew::finalize() {
  if (handleUnreachableOperands(this)) {
    return;
  }
}

void StructGet::finalize() {
  if (ref->type == Type::unreachable) {
    type = Type::unreachable;
  } else if (ref->type.isNull()) {
    // See comment on CallRef for explanation.
    if (type.isRef()) {
      type = Type(type.getHeapType().getBottom(), NonNullable);
    }
  } else {
    type = ref->type.getHeapType().getStruct().fields[index].type;
  }
}

void StructSet::finalize() {
  if (ref->type == Type::unreachable || value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void ArrayNew::finalize() {
  if (size->type == Type::unreachable ||
      (init && init->type == Type::unreachable)) {
    type = Type::unreachable;
  }
}

void ArrayNewData::finalize() {
  if (offset->type == Type::unreachable || size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void ArrayNewElem::finalize() {
  if (offset->type == Type::unreachable || size->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void ArrayNewFixed::finalize() {
  for (auto* value : values) {
    if (value->type == Type::unreachable) {
      type = Type::unreachable;
      return;
    }
  }
}

void ArrayGet::finalize() {
  if (ref->type == Type::unreachable || index->type == Type::unreachable) {
    type = Type::unreachable;
  } else if (ref->type.isNull()) {
    // See comment on CallRef for explanation.
    if (type.isRef()) {
      type = Type(type.getHeapType().getBottom(), NonNullable);
    }
  } else {
    type = ref->type.getHeapType().getArray().element.type;
  }
}

void ArraySet::finalize() {
  if (ref->type == Type::unreachable || index->type == Type::unreachable ||
      value->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void ArrayLen::finalize() {
  if (ref->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void ArrayCopy::finalize() {
  if (srcRef->type == Type::unreachable ||
      srcIndex->type == Type::unreachable ||
      destRef->type == Type::unreachable ||
      destIndex->type == Type::unreachable ||
      length->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void ArrayFill::finalize() {
  if (ref->type == Type::unreachable || index->type == Type::unreachable ||
      value->type == Type::unreachable || size->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void ArrayInitData::finalize() {
  if (ref->type == Type::unreachable || index->type == Type::unreachable ||
      offset->type == Type::unreachable || size->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void ArrayInitElem::finalize() {
  if (ref->type == Type::unreachable || index->type == Type::unreachable ||
      offset->type == Type::unreachable || size->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::none;
  }
}

void RefAs::finalize() {
  // An unreachable child means we are unreachable. Also set ourselves to
  // unreachable when the child is invalid (say, it is an i32 or some other non-
  // reference), which avoids getHeapType() erroring right after us (in this
  // situation, the validator will report an error later).
  // TODO: Remove that part when we solve the validation issue more generally,
  //       see https://github.com/WebAssembly/binaryen/issues/6781
  if (!value->type.isRef()) {
    type = Type::unreachable;
    return;
  }
  auto valHeapType = value->type.getHeapType();
  switch (op) {
    case RefAsNonNull:
      type = Type(valHeapType, NonNullable);
      break;
    case AnyConvertExtern:
      type = Type(HeapTypes::any.getBasic(valHeapType.getShared()),
                  value->type.getNullability());
      break;
    case ExternConvertAny:
      type = Type(HeapTypes::ext.getBasic(valHeapType.getShared()),
                  value->type.getNullability());
      break;
    default:
      WASM_UNREACHABLE("invalid ref.as_*");
  }
}

void StringNew::finalize() {
  if (ref->type == Type::unreachable ||
      (start && start->type == Type::unreachable) ||
      (end && end->type == Type::unreachable)) {
    type = Type::unreachable;
  } else {
    type = Type(HeapType::string, NonNullable);
  }
}

void StringConst::finalize() { type = Type(HeapType::string, NonNullable); }

void StringMeasure::finalize() {
  if (ref->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void StringEncode::finalize() {
  if (str->type == Type::unreachable || array->type == Type::unreachable ||
      start->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void StringConcat::finalize() {
  if (left->type == Type::unreachable || right->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type(HeapType::string, NonNullable);
  }
}

void StringEq::finalize() {
  if (left->type == Type::unreachable || right->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void StringWTF16Get::finalize() {
  if (ref->type == Type::unreachable || pos->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type::i32;
  }
}

void StringSliceWTF::finalize() {
  if (ref->type == Type::unreachable || start->type == Type::unreachable ||
      end->type == Type::unreachable) {
    type = Type::unreachable;
  } else {
    type = Type(HeapType::string, NonNullable);
  }
}

void ContNew::finalize() {
  if (func->type == Type::unreachable) {
    type = Type::unreachable;
  }
}

void ContBind::finalize() {
  if (cont->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  if (handleUnreachableOperands(this)) {
    return;
  }
}

void Suspend::finalize(Module* wasm) {
  if (!handleUnreachableOperands(this) && wasm) {
    auto tag = wasm->getTag(this->tag);
    type = tag->sig.results;
  }
}

void Resume::finalize() {
  if (cont->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  if (handleUnreachableOperands(this)) {
    return;
  }

  assert(this->cont->type.isContinuation());
  const Signature& contSig =
    this->cont->type.getHeapType().getContinuation().type.getSignature();
  type = contSig.results;
}

void ResumeThrow::finalize() {
  if (cont->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  if (handleUnreachableOperands(this)) {
    return;
  }

  assert(this->cont->type.isContinuation());
  const Signature& contSig =
    this->cont->type.getHeapType().getContinuation().type.getSignature();
  type = contSig.results;
}

void StackSwitch::finalize() {
  if (cont->type == Type::unreachable) {
    type = Type::unreachable;
    return;
  }
  if (handleUnreachableOperands(this)) {
    return;
  }

  assert(this->cont->type.isContinuation());
  type =
    this->cont->type.getHeapType().getContinuation().type.getSignature().params;
}

size_t Function::getNumParams() { return getParams().size(); }

size_t Function::getNumVars() { return vars.size(); }

size_t Function::getNumLocals() { return getParams().size() + vars.size(); }

bool Function::isParam(Index index) {
  size_t size = getParams().size();
  assert(index < size + vars.size());
  return index < size;
}

bool Function::isVar(Index index) {
  auto base = getVarIndexBase();
  assert(index < base + vars.size());
  return index >= base;
}

bool Function::hasLocalName(Index index) const {
  return localNames.find(index) != localNames.end();
}

Name Function::getLocalName(Index index) { return localNames.at(index); }

void Function::setLocalName(Index index, Name name) {
  assert(index < getNumLocals());
  localNames[index] = name;
  localIndices[name] = index;
}

Name Function::getLocalNameOrDefault(Index index) {
  auto nameIt = localNames.find(index);
  if (nameIt != localNames.end()) {
    return nameIt->second;
  }
  // this is an unnamed local
  return Name();
}

Name Function::getLocalNameOrGeneric(Index index) {
  auto nameIt = localNames.find(index);
  if (nameIt != localNames.end()) {
    return nameIt->second;
  }
  return Name::fromInt(index);
}

bool Function::hasLocalIndex(Name name) const {
  return localIndices.find(name) != localIndices.end();
}

Index Function::getLocalIndex(Name name) {
  auto iter = localIndices.find(name);
  if (iter == localIndices.end()) {
    Fatal() << "Function::getLocalIndex: " << name << " does not exist";
  }
  return iter->second;
}

Index Function::getVarIndexBase() { return getParams().size(); }

Type Function::getLocalType(Index index) {
  auto numParams = getParams().size();
  if (index < numParams) {
    return getParams()[index];
  } else if (isVar(index)) {
    return vars[index - numParams];
  } else {
    WASM_UNREACHABLE("invalid local index");
  }
}

void Function::clearNames() { localNames.clear(); }

void Function::clearDebugInfo() {
  localIndices.clear();
  debugLocations.clear();
  prologLocation.reset();
  epilogLocation.reset();
}

template<typename Map>
typename Map::mapped_type&
getModuleElement(Map& m, Name name, std::string_view funcName) {
  auto iter = m.find(name);
  if (iter == m.end()) {
    Fatal() << "Module::" << funcName << ": " << name << " does not exist";
  }
  return iter->second;
}

Export* Module::getExport(Name name) {
  return getModuleElement(exportsMap, name, "getExport");
}

Function* Module::getFunction(Name name) {
  return getModuleElement(functionsMap, name, "getFunction");
}

Table* Module::getTable(Name name) {
  return getModuleElement(tablesMap, name, "getTable");
}

ElementSegment* Module::getElementSegment(Name name) {
  return getModuleElement(elementSegmentsMap, name, "getElementSegment");
}

Memory* Module::getMemory(Name name) {
  return getModuleElement(memoriesMap, name, "getMemory");
}

DataSegment* Module::getDataSegment(Name name) {
  return getModuleElement(dataSegmentsMap, name, "getDataSegment");
}

Global* Module::getGlobal(Name name) {
  return getModuleElement(globalsMap, name, "getGlobal");
}

Tag* Module::getTag(Name name) {
  return getModuleElement(tagsMap, name, "getTag");
}

template<typename Map>
typename Map::mapped_type getModuleElementOrNull(Map& m, Name name) {
  auto iter = m.find(name);
  if (iter == m.end()) {
    return nullptr;
  }
  return iter->second;
}

Export* Module::getExportOrNull(Name name) {
  return getModuleElementOrNull(exportsMap, name);
}

Function* Module::getFunctionOrNull(Name name) {
  return getModuleElementOrNull(functionsMap, name);
}

Table* Module::getTableOrNull(Name name) {
  return getModuleElementOrNull(tablesMap, name);
}

ElementSegment* Module::getElementSegmentOrNull(Name name) {
  return getModuleElementOrNull(elementSegmentsMap, name);
}

Memory* Module::getMemoryOrNull(Name name) {
  return getModuleElementOrNull(memoriesMap, name);
}

DataSegment* Module::getDataSegmentOrNull(Name name) {
  return getModuleElementOrNull(dataSegmentsMap, name);
}

Global* Module::getGlobalOrNull(Name name) {
  return getModuleElementOrNull(globalsMap, name);
}

Tag* Module::getTagOrNull(Name name) {
  return getModuleElementOrNull(tagsMap, name);
}

Importable* Module::getImport(ModuleItemKind kind, Name name) {
  switch (kind) {
    case ModuleItemKind::Function:
      return getFunction(name);
    case ModuleItemKind::Table:
      return getTable(name);
    case ModuleItemKind::Memory:
      return getMemory(name);
    case ModuleItemKind::Global:
      return getGlobal(name);
    case ModuleItemKind::Tag:
      return getTag(name);
    case ModuleItemKind::DataSegment:
    case ModuleItemKind::ElementSegment:
    case ModuleItemKind::Invalid:
      WASM_UNREACHABLE("invalid kind");
  }

  WASM_UNREACHABLE("unexpected kind");
}

Importable* Module::getImportOrNull(ModuleItemKind kind, Name name) {
  auto doReturn = [](Importable* importable) {
    return importable ? importable->imported() ? importable : nullptr : nullptr;
  };

  switch (kind) {
    case ModuleItemKind::Function:
      return doReturn(getFunctionOrNull(name));
    case ModuleItemKind::Table:
      return doReturn(getTableOrNull(name));
    case ModuleItemKind::Memory:
      return doReturn(getMemoryOrNull(name));
    case ModuleItemKind::Global:
      return doReturn(getGlobalOrNull(name));
    case ModuleItemKind::Tag:
      return doReturn(getTagOrNull(name));
    case ModuleItemKind::DataSegment:
    case ModuleItemKind::ElementSegment:
      return nullptr;
    case ModuleItemKind::Invalid:
      WASM_UNREACHABLE("invalid kind");
  }

  WASM_UNREACHABLE("unexpected kind");
}

// TODO(@warchant): refactor all usages to use variant with unique_ptr
template<typename Vector, typename Map, typename Elem>
Elem* addModuleElement(Vector& v, Map& m, Elem* curr, std::string funcName) {
  if (!curr->name.is()) {
    Fatal() << "Module::" << funcName << ": empty name";
  }
  if (getModuleElementOrNull(m, curr->name)) {
    Fatal() << "Module::" << funcName << ": " << curr->name
            << " already exists";
  }
  v.push_back(std::unique_ptr<Elem>(curr));
  m[curr->name] = curr;
  return curr;
}

template<typename Vector, typename Map, typename Elem>
Elem* addModuleElement(Vector& v,
                       Map& m,
                       std::unique_ptr<Elem> curr,
                       std::string funcName) {
  if (!curr->name.is()) {
    Fatal() << "Module::" << funcName << ": empty name";
  }
  if (getModuleElementOrNull(m, curr->name)) {
    Fatal() << "Module::" << funcName << ": " << curr->name
            << " already exists";
  }
  auto* ret = m[curr->name] = curr.get();
  v.push_back(std::move(curr));
  return ret;
}

Export* Module::addExport(Export* curr) {
  return addModuleElement(exports, exportsMap, curr, "addExport");
}

Function* Module::addFunction(Function* curr) {
  return addModuleElement(functions, functionsMap, curr, "addFunction");
}

Global* Module::addGlobal(Global* curr) {
  return addModuleElement(globals, globalsMap, curr, "addGlobal");
}

Tag* Module::addTag(Tag* curr) {
  return addModuleElement(tags, tagsMap, curr, "addTag");
}

Export* Module::addExport(std::unique_ptr<Export>&& curr) {
  return addModuleElement(exports, exportsMap, std::move(curr), "addExport");
}

Function* Module::addFunction(std::unique_ptr<Function>&& curr) {
  return addModuleElement(
    functions, functionsMap, std::move(curr), "addFunction");
}

Table* Module::addTable(std::unique_ptr<Table>&& curr) {
  return addModuleElement(tables, tablesMap, std::move(curr), "addTable");
}

ElementSegment*
Module::addElementSegment(std::unique_ptr<ElementSegment>&& curr) {
  return addModuleElement(
    elementSegments, elementSegmentsMap, std::move(curr), "addElementSegment");
}

Memory* Module::addMemory(std::unique_ptr<Memory>&& curr) {
  return addModuleElement(memories, memoriesMap, std::move(curr), "addMemory");
}

DataSegment* Module::addDataSegment(std::unique_ptr<DataSegment>&& curr) {
  return addModuleElement(
    dataSegments, dataSegmentsMap, std::move(curr), "addDataSegment");
}

Global* Module::addGlobal(std::unique_ptr<Global>&& curr) {
  return addModuleElement(globals, globalsMap, std::move(curr), "addGlobal");
}

Tag* Module::addTag(std::unique_ptr<Tag>&& curr) {
  return addModuleElement(tags, tagsMap, std::move(curr), "addTag");
}

void Module::addStart(const Name& s) { start = s; }

template<typename Vector, typename Map>
void removeModuleElement(Vector& v, Map& m, Name name) {
  m.erase(name);
  for (size_t i = 0; i < v.size(); i++) {
    if (v[i]->name == name) {
      v.erase(v.begin() + i);
      break;
    }
  }
}

void Module::removeExport(Name name) {
  removeModuleElement(exports, exportsMap, name);
}
void Module::removeFunction(Name name) {
  removeModuleElement(functions, functionsMap, name);
}
void Module::removeTable(Name name) {
  removeModuleElement(tables, tablesMap, name);
}
void Module::removeElementSegment(Name name) {
  removeModuleElement(elementSegments, elementSegmentsMap, name);
}
void Module::removeMemory(Name name) {
  removeModuleElement(memories, memoriesMap, name);
}
void Module::removeDataSegment(Name name) {
  removeModuleElement(dataSegments, dataSegmentsMap, name);
}
void Module::removeGlobal(Name name) {
  removeModuleElement(globals, globalsMap, name);
}
void Module::removeTag(Name name) { removeModuleElement(tags, tagsMap, name); }

template<typename Vector, typename Map, typename Elem>
void removeModuleElements(Vector& v,
                          Map& m,
                          std::function<bool(Elem* elem)> pred) {
  for (auto it = m.begin(); it != m.end();) {
    if (pred(it->second)) {
      it = m.erase(it);
    } else {
      it++;
    }
  }
  v.erase(
    std::remove_if(v.begin(), v.end(), [&](auto& e) { return pred(e.get()); }),
    v.end());
}

void Module::removeExports(std::function<bool(Export*)> pred) {
  removeModuleElements(exports, exportsMap, pred);
}
void Module::removeFunctions(std::function<bool(Function*)> pred) {
  removeModuleElements(functions, functionsMap, pred);
}
void Module::removeTables(std::function<bool(Table*)> pred) {
  removeModuleElements(tables, tablesMap, pred);
}
void Module::removeElementSegments(std::function<bool(ElementSegment*)> pred) {
  removeModuleElements(elementSegments, elementSegmentsMap, pred);
}
void Module::removeMemories(std::function<bool(Memory*)> pred) {
  removeModuleElements(memories, memoriesMap, pred);
}
void Module::removeDataSegments(std::function<bool(DataSegment*)> pred) {
  removeModuleElements(dataSegments, dataSegmentsMap, pred);
}
void Module::removeGlobals(std::function<bool(Global*)> pred) {
  removeModuleElements(globals, globalsMap, pred);
}
void Module::removeTags(std::function<bool(Tag*)> pred) {
  removeModuleElements(tags, tagsMap, pred);
}

void Module::updateFunctionsMap() {
  functionsMap.clear();
  for (auto& curr : functions) {
    functionsMap[curr->name] = curr.get();
  }
  assert(functionsMap.size() == functions.size());
}

void Module::updateDataSegmentsMap() {
  dataSegmentsMap.clear();
  for (auto& curr : dataSegments) {
    dataSegmentsMap[curr->name] = curr.get();
  }
  assert(dataSegmentsMap.size() == dataSegments.size());
}

void Module::updateMaps() {
  updateFunctionsMap();
  exportsMap.clear();
  for (auto& curr : exports) {
    exportsMap[curr->name] = curr.get();
  }
  assert(exportsMap.size() == exports.size());
  tablesMap.clear();
  for (auto& curr : tables) {
    tablesMap[curr->name] = curr.get();
  }
  assert(tablesMap.size() == tables.size());
  elementSegmentsMap.clear();
  for (auto& curr : elementSegments) {
    elementSegmentsMap[curr->name] = curr.get();
  }
  assert(elementSegmentsMap.size() == elementSegments.size());
  memoriesMap.clear();
  for (auto& curr : memories) {
    memoriesMap[curr->name] = curr.get();
  }
  assert(memoriesMap.size() == memories.size());
  updateDataSegmentsMap();
  globalsMap.clear();
  for (auto& curr : globals) {
    globalsMap[curr->name] = curr.get();
  }
  assert(globalsMap.size() == globals.size());
  tagsMap.clear();
  for (auto& curr : tags) {
    tagsMap[curr->name] = curr.get();
  }
  assert(tagsMap.size() == tags.size());
}

void Module::clearDebugInfo() {
  debugInfoFileNames.clear();
  debugInfoSymbolNames.clear();
}

} // namespace wasm
