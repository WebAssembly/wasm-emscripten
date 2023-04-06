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

//===============================
// Binaryen C API implementation
//===============================

#include <mutex>

#include "binaryen-c.h"
#include "cfg/Relooper.h"
#include "ir/utils.h"
#include "pass.h"
#include "shell-interface.h"
#include "support/colors.h"
#include "wasm-binary.h"
#include "wasm-builder.h"
#include "wasm-interpreter.h"
#include "wasm-s-parser.h"
#include "wasm-stack.h"
#include "wasm-validator.h"
#include "wasm.h"
#include "wasm2js.h"
#include <iostream>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace wasm;

// Literal utilities

static_assert(sizeof(BinaryenLiteral) == sizeof(Literal),
              "Binaryen C API literal must match wasm.h");

BinaryenLiteral toBinaryenLiteral(Literal x) {
  BinaryenLiteral ret;
  ret.type = x.type.getID();
  assert(x.type.isSingle());
  if (x.type.isBasic()) {
    switch (x.type.getBasic()) {
      case Type::i32:
        ret.i32 = x.geti32();
        return ret;
      case Type::i64:
        ret.i64 = x.geti64();
        return ret;
      case Type::f32:
        ret.i32 = x.reinterpreti32();
        return ret;
      case Type::f64:
        ret.i64 = x.reinterpreti64();
        return ret;
      case Type::v128:
        memcpy(&ret.v128, x.getv128Ptr(), 16);
        return ret;
      case Type::none:
      case Type::unreachable:
        WASM_UNREACHABLE("unexpected type");
    }
  }
  assert(x.type.isRef());
  auto heapType = x.type.getHeapType();
  if (heapType.isBasic()) {
    switch (heapType.getBasic()) {
      case HeapType::i31:
        WASM_UNREACHABLE("TODO: i31");
      case HeapType::ext:
        WASM_UNREACHABLE("TODO: extern literals");
      case HeapType::any:
      case HeapType::eq:
      case HeapType::func:
      case HeapType::struct_:
      case HeapType::array:
        WASM_UNREACHABLE("invalid type");
      case HeapType::string:
      case HeapType::stringview_wtf8:
      case HeapType::stringview_wtf16:
      case HeapType::stringview_iter:
        WASM_UNREACHABLE("TODO: string literals");
      case HeapType::none:
      case HeapType::noext:
      case HeapType::nofunc:
        // Null.
        return ret;
    }
  }
  if (heapType.isSignature()) {
    ret.func = x.getFunc().str.data();
    return ret;
  }
  assert(x.isData());
  WASM_UNREACHABLE("TODO: gc data");
}

Literal fromBinaryenLiteral(BinaryenLiteral x) {
  auto type = Type(x.type);
  if (type.isBasic()) {
    switch (type.getBasic()) {
      case Type::i32:
        return Literal(x.i32);
      case Type::i64:
        return Literal(x.i64);
      case Type::f32:
        return Literal(x.i32).castToF32();
      case Type::f64:
        return Literal(x.i64).castToF64();
      case Type::v128:
        return Literal(x.v128);
      case Type::none:
      case Type::unreachable:
        WASM_UNREACHABLE("unexpected type");
    }
  }
  assert(type.isRef());
  auto heapType = type.getHeapType();
  if (heapType.isBasic()) {
    switch (heapType.getBasic()) {
      case HeapType::i31:
        WASM_UNREACHABLE("TODO: i31");
      case HeapType::ext:
      case HeapType::any:
        WASM_UNREACHABLE("TODO: extern literals");
      case HeapType::eq:
      case HeapType::func:
      case HeapType::struct_:
      case HeapType::array:
        WASM_UNREACHABLE("invalid type");
      case HeapType::string:
      case HeapType::stringview_wtf8:
      case HeapType::stringview_wtf16:
      case HeapType::stringview_iter:
        WASM_UNREACHABLE("TODO: string literals");
      case HeapType::none:
      case HeapType::noext:
      case HeapType::nofunc:
        assert(type.isNullable());
        return Literal::makeNull(heapType);
    }
  }
  if (heapType.isSignature()) {
    return Literal::makeFunc(Name(x.func), heapType);
  }
  assert(heapType.isData());
  WASM_UNREACHABLE("TODO: gc data");
}

// Mutexes (global for now; in theory if multiple modules
// are used at once this should be optimized to be per-
// module, but likely it doesn't matter)

static std::mutex BinaryenFunctionMutex;

// Optimization options
static PassOptions globalPassOptions =
  PassOptions::getWithDefaultOptimizationOptions();

extern "C" {

//
// ========== Module Creation ==========
//

// Core types

BinaryenType BinaryenTypeNone(void) { return Type::none; }
BinaryenType BinaryenTypeInt32(void) { return Type::i32; }
BinaryenType BinaryenTypeInt64(void) { return Type::i64; }
BinaryenType BinaryenTypeFloat32(void) { return Type::f32; }
BinaryenType BinaryenTypeFloat64(void) { return Type::f64; }
BinaryenType BinaryenTypeVec128(void) { return Type::v128; }
BinaryenType BinaryenTypeFuncref(void) {
  return Type(HeapType::func, Nullable).getID();
}
BinaryenType BinaryenTypeExternref(void) {
  return Type(HeapType::ext, Nullable).getID();
}
BinaryenType BinaryenTypeAnyref(void) {
  return Type(HeapType::any, Nullable).getID();
}
BinaryenType BinaryenTypeEqref(void) {
  return Type(HeapType::eq, Nullable).getID();
}
BinaryenType BinaryenTypeI31ref(void) {
  return Type(HeapType::i31, Nullable).getID();
}
BinaryenType BinaryenTypeStructref(void) {
  return Type(HeapType::struct_, Nullable).getID();
}
BinaryenType BinaryenTypeArrayref(void) {
  return Type(HeapType::array, Nullable).getID();
}
BinaryenType BinaryenTypeStringref() {
  return Type(HeapType::string, Nullable).getID();
}
BinaryenType BinaryenTypeStringviewWTF8() {
  return Type(HeapType::stringview_wtf8, Nullable).getID();
}
BinaryenType BinaryenTypeStringviewWTF16() {
  return Type(HeapType::stringview_wtf16, Nullable).getID();
}
BinaryenType BinaryenTypeStringviewIter() {
  return Type(HeapType::stringview_iter, Nullable).getID();
}
BinaryenType BinaryenTypeNullref() {
  return Type(HeapType::none, Nullable).getID();
}
BinaryenType BinaryenTypeNullExternref(void) {
  return Type(HeapType::noext, Nullable).getID();
}
BinaryenType BinaryenTypeNullFuncref(void) {
  return Type(HeapType::nofunc, Nullable).getID();
}
BinaryenType BinaryenTypeUnreachable(void) { return Type::unreachable; }
BinaryenType BinaryenTypeAuto(void) { return uintptr_t(-1); }

BinaryenType BinaryenTypeCreate(BinaryenType* types, BinaryenIndex numTypes) {
  std::vector<Type> typeVec;
  typeVec.reserve(numTypes);
  for (BinaryenIndex i = 0; i < numTypes; ++i) {
    typeVec.push_back(Type(types[i]));
  }
  return Type(typeVec).getID();
}

uint32_t BinaryenTypeArity(BinaryenType t) { return Type(t).size(); }

void BinaryenTypeExpand(BinaryenType t, BinaryenType* buf) {
  Type types(t);
  size_t i = 0;
  for (const auto& type : types) {
    buf[i++] = type.getID();
  }
}

WASM_DEPRECATED BinaryenType BinaryenNone(void) { return Type::none; }
WASM_DEPRECATED BinaryenType BinaryenInt32(void) { return Type::i32; }
WASM_DEPRECATED BinaryenType BinaryenInt64(void) { return Type::i64; }
WASM_DEPRECATED BinaryenType BinaryenFloat32(void) { return Type::f32; }
WASM_DEPRECATED BinaryenType BinaryenFloat64(void) { return Type::f64; }
WASM_DEPRECATED BinaryenType BinaryenUndefined(void) { return uint32_t(-1); }

// Packed types

BinaryenPackedType BinaryenPackedTypeNotPacked(void) {
  return Field::PackedType::not_packed;
}
BinaryenPackedType BinaryenPackedTypeInt8(void) {
  return Field::PackedType::i8;
}
BinaryenPackedType BinaryenPackedTypeInt16(void) {
  return Field::PackedType::i16;
}

// Heap types

BinaryenHeapType BinaryenHeapTypeExt() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::ext);
}
BinaryenHeapType BinaryenHeapTypeFunc() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::func);
}
BinaryenHeapType BinaryenHeapTypeAny() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::any);
}
BinaryenHeapType BinaryenHeapTypeEq() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::eq);
}
BinaryenHeapType BinaryenHeapTypeI31() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::i31);
}
BinaryenHeapType BinaryenHeapTypeStruct() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::struct_);
}
BinaryenHeapType BinaryenHeapTypeArray() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::array);
}
BinaryenHeapType BinaryenHeapTypeString() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::string);
}
BinaryenHeapType BinaryenHeapTypeStringviewWTF8() {
  return static_cast<BinaryenHeapType>(
    HeapType::BasicHeapType::stringview_wtf8);
}
BinaryenHeapType BinaryenHeapTypeStringviewWTF16() {
  return static_cast<BinaryenHeapType>(
    HeapType::BasicHeapType::stringview_wtf16);
}
BinaryenHeapType BinaryenHeapTypeStringviewIter() {
  return static_cast<BinaryenHeapType>(
    HeapType::BasicHeapType::stringview_iter);
}
BinaryenHeapType BinaryenHeapTypeNone() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::none);
}
BinaryenHeapType BinaryenHeapTypeNoext() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::noext);
}
BinaryenHeapType BinaryenHeapTypeNofunc() {
  return static_cast<BinaryenHeapType>(HeapType::BasicHeapType::nofunc);
}

bool BinaryenHeapTypeIsBasic(BinaryenHeapType heapType) {
  return HeapType(heapType).isBasic();
}
bool BinaryenHeapTypeIsSignature(BinaryenHeapType heapType) {
  return HeapType(heapType).isSignature();
}
bool BinaryenHeapTypeIsStruct(BinaryenHeapType heapType) {
  return HeapType(heapType).isStruct();
}
bool BinaryenHeapTypeIsArray(BinaryenHeapType heapType) {
  return HeapType(heapType).isArray();
}
bool BinaryenHeapTypeIsBottom(BinaryenHeapType heapType) {
  return HeapType(heapType).isBottom();
}
BinaryenHeapType BinaryenHeapTypeGetBottom(BinaryenHeapType heapType) {
  return static_cast<BinaryenHeapType>(HeapType(heapType).getBottom());
}
bool BinaryenHeapTypeIsSubType(BinaryenHeapType left, BinaryenHeapType right) {
  return HeapType::isSubType(HeapType(left), HeapType(right));
}
BinaryenIndex BinaryenStructTypeGetNumFields(BinaryenHeapType heapType) {
  auto ht = HeapType(heapType);
  assert(ht.isStruct());
  return ht.getStruct().fields.size();
}
BinaryenType BinaryenStructTypeGetFieldType(BinaryenHeapType heapType,
                                            BinaryenIndex index) {
  auto ht = HeapType(heapType);
  assert(ht.isStruct());
  auto& fields = ht.getStruct().fields;
  assert(index < fields.size());
  return fields[index].type.getID();
}
BinaryenPackedType
BinaryenStructTypeGetFieldPackedType(BinaryenHeapType heapType,
                                     BinaryenIndex index) {
  auto ht = HeapType(heapType);
  assert(ht.isStruct());
  auto& fields = ht.getStruct().fields;
  assert(index < fields.size());
  return fields[index].packedType;
}
bool BinaryenStructTypeIsFieldMutable(BinaryenHeapType heapType,
                                      BinaryenIndex index) {
  auto ht = HeapType(heapType);
  assert(ht.isStruct());
  auto& fields = ht.getStruct().fields;
  assert(index < fields.size());
  return fields[index].mutable_;
}
BinaryenType BinaryenArrayTypeGetElementType(BinaryenHeapType heapType) {
  auto ht = HeapType(heapType);
  assert(ht.isArray());
  return ht.getArray().element.type.getID();
}
BinaryenPackedType
BinaryenArrayTypeGetElementPackedType(BinaryenHeapType heapType) {
  auto ht = HeapType(heapType);
  assert(ht.isArray());
  return ht.getArray().element.packedType;
}
bool BinaryenArrayTypeIsElementMutable(BinaryenHeapType heapType) {
  auto ht = HeapType(heapType);
  assert(ht.isArray());
  return ht.getArray().element.mutable_;
}
BinaryenType BinaryenSignatureTypeGetParams(BinaryenHeapType heapType) {
  auto ht = HeapType(heapType);
  assert(ht.isSignature());
  return ht.getSignature().params.getID();
}
BinaryenType BinaryenSignatureTypeGetResults(BinaryenHeapType heapType) {
  auto ht = HeapType(heapType);
  assert(ht.isSignature());
  return ht.getSignature().results.getID();
}

BinaryenHeapType BinaryenTypeGetHeapType(BinaryenType type) {
  return Type(type).getHeapType().getID();
}
bool BinaryenTypeIsNullable(BinaryenType type) {
  return Type(type).isNullable();
}
BinaryenType BinaryenTypeFromHeapType(BinaryenHeapType heapType,
                                      bool nullable) {
  return Type(HeapType(heapType),
              nullable ? Nullability::Nullable : Nullability::NonNullable)
    .getID();
}

// TypeSystem

BinaryenTypeSystem BinaryenTypeSystemNominal() {
  return static_cast<BinaryenTypeSystem>(TypeSystem::Nominal);
}
BinaryenTypeSystem BinaryenTypeSystemIsorecursive() {
  return static_cast<BinaryenTypeSystem>(TypeSystem::Isorecursive);
}
BinaryenTypeSystem BinaryenGetTypeSystem() {
  return BinaryenTypeSystem(getTypeSystem());
}
void BinaryenSetTypeSystem(BinaryenTypeSystem typeSystem) {
  setTypeSystem(TypeSystem(typeSystem));
}

// Expression ids

BinaryenExpressionId BinaryenInvalidId(void) {
  return Expression::Id::InvalidId;
}

#define DELEGATE(CLASS_TO_VISIT)                                               \
  BinaryenExpressionId Binaryen##CLASS_TO_VISIT##Id(void) {                    \
    return Expression::Id::CLASS_TO_VISIT##Id;                                 \
  }

#include "wasm-delegations.def"

// External kinds

BinaryenExternalKind BinaryenExternalFunction(void) {
  return static_cast<BinaryenExternalKind>(ExternalKind::Function);
}
BinaryenExternalKind BinaryenExternalTable(void) {
  return static_cast<BinaryenExternalKind>(ExternalKind::Table);
}
BinaryenExternalKind BinaryenExternalMemory(void) {
  return static_cast<BinaryenExternalKind>(ExternalKind::Memory);
}
BinaryenExternalKind BinaryenExternalGlobal(void) {
  return static_cast<BinaryenExternalKind>(ExternalKind::Global);
}
BinaryenExternalKind BinaryenExternalTag(void) {
  return static_cast<BinaryenExternalKind>(ExternalKind::Tag);
}

// Features

BinaryenFeatures BinaryenFeatureMVP(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::MVP);
}
BinaryenFeatures BinaryenFeatureAtomics(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::Atomics);
}
BinaryenFeatures BinaryenFeatureBulkMemory(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::BulkMemory);
}
BinaryenFeatures BinaryenFeatureMutableGlobals(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::MutableGlobals);
}
BinaryenFeatures BinaryenFeatureNontrappingFPToInt(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::TruncSat);
}
BinaryenFeatures BinaryenFeatureSignExt(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::SignExt);
}
BinaryenFeatures BinaryenFeatureSIMD128(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::SIMD);
}
BinaryenFeatures BinaryenFeatureExceptionHandling(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::ExceptionHandling);
}
BinaryenFeatures BinaryenFeatureTailCall(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::TailCall);
}
BinaryenFeatures BinaryenFeatureReferenceTypes(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::ReferenceTypes);
}
BinaryenFeatures BinaryenFeatureMultivalue(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::Multivalue);
}
BinaryenFeatures BinaryenFeatureGC(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::GC);
}
BinaryenFeatures BinaryenFeatureMemory64(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::Memory64);
}
BinaryenFeatures BinaryenFeatureRelaxedSIMD(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::RelaxedSIMD);
}
BinaryenFeatures BinaryenFeatureExtendedConst(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::ExtendedConst);
}
BinaryenFeatures BinaryenFeatureStrings(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::Strings);
}
BinaryenFeatures BinaryenFeatureMultiMemories(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::MultiMemories);
}
BinaryenFeatures BinaryenFeatureAll(void) {
  return static_cast<BinaryenFeatures>(FeatureSet::All);
}

// Modules

BinaryenModuleRef BinaryenModuleCreate(void) { return new Module(); }
void BinaryenModuleDispose(BinaryenModuleRef module) { delete (Module*)module; }

// Literals

BinaryenLiteral BinaryenLiteralInt32(int32_t x) {
  return toBinaryenLiteral(Literal(x));
}
BinaryenLiteral BinaryenLiteralInt64(int64_t x) {
  return toBinaryenLiteral(Literal(x));
}
BinaryenLiteral BinaryenLiteralFloat32(float x) {
  return toBinaryenLiteral(Literal(x));
}
BinaryenLiteral BinaryenLiteralFloat64(double x) {
  return toBinaryenLiteral(Literal(x));
}
BinaryenLiteral BinaryenLiteralVec128(const uint8_t x[16]) {
  return toBinaryenLiteral(Literal(x));
}
BinaryenLiteral BinaryenLiteralFloat32Bits(int32_t x) {
  return toBinaryenLiteral(Literal(x).castToF32());
}
BinaryenLiteral BinaryenLiteralFloat64Bits(int64_t x) {
  return toBinaryenLiteral(Literal(x).castToF64());
}

// Expressions

BinaryenOp BinaryenClzInt32(void) { return ClzInt32; }
BinaryenOp BinaryenCtzInt32(void) { return CtzInt32; }
BinaryenOp BinaryenPopcntInt32(void) { return PopcntInt32; }
BinaryenOp BinaryenNegFloat32(void) { return NegFloat32; }
BinaryenOp BinaryenAbsFloat32(void) { return AbsFloat32; }
BinaryenOp BinaryenCeilFloat32(void) { return CeilFloat32; }
BinaryenOp BinaryenFloorFloat32(void) { return FloorFloat32; }
BinaryenOp BinaryenTruncFloat32(void) { return TruncFloat32; }
BinaryenOp BinaryenNearestFloat32(void) { return NearestFloat32; }
BinaryenOp BinaryenSqrtFloat32(void) { return SqrtFloat32; }
BinaryenOp BinaryenEqZInt32(void) { return EqZInt32; }
BinaryenOp BinaryenClzInt64(void) { return ClzInt64; }
BinaryenOp BinaryenCtzInt64(void) { return CtzInt64; }
BinaryenOp BinaryenPopcntInt64(void) { return PopcntInt64; }
BinaryenOp BinaryenNegFloat64(void) { return NegFloat64; }
BinaryenOp BinaryenAbsFloat64(void) { return AbsFloat64; }
BinaryenOp BinaryenCeilFloat64(void) { return CeilFloat64; }
BinaryenOp BinaryenFloorFloat64(void) { return FloorFloat64; }
BinaryenOp BinaryenTruncFloat64(void) { return TruncFloat64; }
BinaryenOp BinaryenNearestFloat64(void) { return NearestFloat64; }
BinaryenOp BinaryenSqrtFloat64(void) { return SqrtFloat64; }
BinaryenOp BinaryenEqZInt64(void) { return EqZInt64; }
BinaryenOp BinaryenExtendSInt32(void) { return ExtendSInt32; }
BinaryenOp BinaryenExtendUInt32(void) { return ExtendUInt32; }
BinaryenOp BinaryenWrapInt64(void) { return WrapInt64; }
BinaryenOp BinaryenTruncSFloat32ToInt32(void) { return TruncSFloat32ToInt32; }
BinaryenOp BinaryenTruncSFloat32ToInt64(void) { return TruncSFloat32ToInt64; }
BinaryenOp BinaryenTruncUFloat32ToInt32(void) { return TruncUFloat32ToInt32; }
BinaryenOp BinaryenTruncUFloat32ToInt64(void) { return TruncUFloat32ToInt64; }
BinaryenOp BinaryenTruncSFloat64ToInt32(void) { return TruncSFloat64ToInt32; }
BinaryenOp BinaryenTruncSFloat64ToInt64(void) { return TruncSFloat64ToInt64; }
BinaryenOp BinaryenTruncUFloat64ToInt32(void) { return TruncUFloat64ToInt32; }
BinaryenOp BinaryenTruncUFloat64ToInt64(void) { return TruncUFloat64ToInt64; }
BinaryenOp BinaryenReinterpretFloat32(void) { return ReinterpretFloat32; }
BinaryenOp BinaryenReinterpretFloat64(void) { return ReinterpretFloat64; }
BinaryenOp BinaryenExtendS8Int32(void) { return ExtendS8Int32; }
BinaryenOp BinaryenExtendS16Int32(void) { return ExtendS16Int32; }
BinaryenOp BinaryenExtendS8Int64(void) { return ExtendS8Int64; }
BinaryenOp BinaryenExtendS16Int64(void) { return ExtendS16Int64; }
BinaryenOp BinaryenExtendS32Int64(void) { return ExtendS32Int64; }
BinaryenOp BinaryenConvertSInt32ToFloat32(void) {
  return ConvertSInt32ToFloat32;
}
BinaryenOp BinaryenConvertSInt32ToFloat64(void) {
  return ConvertSInt32ToFloat64;
}
BinaryenOp BinaryenConvertUInt32ToFloat32(void) {
  return ConvertUInt32ToFloat32;
}
BinaryenOp BinaryenConvertUInt32ToFloat64(void) {
  return ConvertUInt32ToFloat64;
}
BinaryenOp BinaryenConvertSInt64ToFloat32(void) {
  return ConvertSInt64ToFloat32;
}
BinaryenOp BinaryenConvertSInt64ToFloat64(void) {
  return ConvertSInt64ToFloat64;
}
BinaryenOp BinaryenConvertUInt64ToFloat32(void) {
  return ConvertUInt64ToFloat32;
}
BinaryenOp BinaryenConvertUInt64ToFloat64(void) {
  return ConvertUInt64ToFloat64;
}
BinaryenOp BinaryenPromoteFloat32(void) { return PromoteFloat32; }
BinaryenOp BinaryenDemoteFloat64(void) { return DemoteFloat64; }
BinaryenOp BinaryenReinterpretInt32(void) { return ReinterpretInt32; }
BinaryenOp BinaryenReinterpretInt64(void) { return ReinterpretInt64; }
BinaryenOp BinaryenAddInt32(void) { return AddInt32; }
BinaryenOp BinaryenSubInt32(void) { return SubInt32; }
BinaryenOp BinaryenMulInt32(void) { return MulInt32; }
BinaryenOp BinaryenDivSInt32(void) { return DivSInt32; }
BinaryenOp BinaryenDivUInt32(void) { return DivUInt32; }
BinaryenOp BinaryenRemSInt32(void) { return RemSInt32; }
BinaryenOp BinaryenRemUInt32(void) { return RemUInt32; }
BinaryenOp BinaryenAndInt32(void) { return AndInt32; }
BinaryenOp BinaryenOrInt32(void) { return OrInt32; }
BinaryenOp BinaryenXorInt32(void) { return XorInt32; }
BinaryenOp BinaryenShlInt32(void) { return ShlInt32; }
BinaryenOp BinaryenShrUInt32(void) { return ShrUInt32; }
BinaryenOp BinaryenShrSInt32(void) { return ShrSInt32; }
BinaryenOp BinaryenRotLInt32(void) { return RotLInt32; }
BinaryenOp BinaryenRotRInt32(void) { return RotRInt32; }
BinaryenOp BinaryenEqInt32(void) { return EqInt32; }
BinaryenOp BinaryenNeInt32(void) { return NeInt32; }
BinaryenOp BinaryenLtSInt32(void) { return LtSInt32; }
BinaryenOp BinaryenLtUInt32(void) { return LtUInt32; }
BinaryenOp BinaryenLeSInt32(void) { return LeSInt32; }
BinaryenOp BinaryenLeUInt32(void) { return LeUInt32; }
BinaryenOp BinaryenGtSInt32(void) { return GtSInt32; }
BinaryenOp BinaryenGtUInt32(void) { return GtUInt32; }
BinaryenOp BinaryenGeSInt32(void) { return GeSInt32; }
BinaryenOp BinaryenGeUInt32(void) { return GeUInt32; }
BinaryenOp BinaryenAddInt64(void) { return AddInt64; }
BinaryenOp BinaryenSubInt64(void) { return SubInt64; }
BinaryenOp BinaryenMulInt64(void) { return MulInt64; }
BinaryenOp BinaryenDivSInt64(void) { return DivSInt64; }
BinaryenOp BinaryenDivUInt64(void) { return DivUInt64; }
BinaryenOp BinaryenRemSInt64(void) { return RemSInt64; }
BinaryenOp BinaryenRemUInt64(void) { return RemUInt64; }
BinaryenOp BinaryenAndInt64(void) { return AndInt64; }
BinaryenOp BinaryenOrInt64(void) { return OrInt64; }
BinaryenOp BinaryenXorInt64(void) { return XorInt64; }
BinaryenOp BinaryenShlInt64(void) { return ShlInt64; }
BinaryenOp BinaryenShrUInt64(void) { return ShrUInt64; }
BinaryenOp BinaryenShrSInt64(void) { return ShrSInt64; }
BinaryenOp BinaryenRotLInt64(void) { return RotLInt64; }
BinaryenOp BinaryenRotRInt64(void) { return RotRInt64; }
BinaryenOp BinaryenEqInt64(void) { return EqInt64; }
BinaryenOp BinaryenNeInt64(void) { return NeInt64; }
BinaryenOp BinaryenLtSInt64(void) { return LtSInt64; }
BinaryenOp BinaryenLtUInt64(void) { return LtUInt64; }
BinaryenOp BinaryenLeSInt64(void) { return LeSInt64; }
BinaryenOp BinaryenLeUInt64(void) { return LeUInt64; }
BinaryenOp BinaryenGtSInt64(void) { return GtSInt64; }
BinaryenOp BinaryenGtUInt64(void) { return GtUInt64; }
BinaryenOp BinaryenGeSInt64(void) { return GeSInt64; }
BinaryenOp BinaryenGeUInt64(void) { return GeUInt64; }
BinaryenOp BinaryenAddFloat32(void) { return AddFloat32; }
BinaryenOp BinaryenSubFloat32(void) { return SubFloat32; }
BinaryenOp BinaryenMulFloat32(void) { return MulFloat32; }
BinaryenOp BinaryenDivFloat32(void) { return DivFloat32; }
BinaryenOp BinaryenCopySignFloat32(void) { return CopySignFloat32; }
BinaryenOp BinaryenMinFloat32(void) { return MinFloat32; }
BinaryenOp BinaryenMaxFloat32(void) { return MaxFloat32; }
BinaryenOp BinaryenEqFloat32(void) { return EqFloat32; }
BinaryenOp BinaryenNeFloat32(void) { return NeFloat32; }
BinaryenOp BinaryenLtFloat32(void) { return LtFloat32; }
BinaryenOp BinaryenLeFloat32(void) { return LeFloat32; }
BinaryenOp BinaryenGtFloat32(void) { return GtFloat32; }
BinaryenOp BinaryenGeFloat32(void) { return GeFloat32; }
BinaryenOp BinaryenAddFloat64(void) { return AddFloat64; }
BinaryenOp BinaryenSubFloat64(void) { return SubFloat64; }
BinaryenOp BinaryenMulFloat64(void) { return MulFloat64; }
BinaryenOp BinaryenDivFloat64(void) { return DivFloat64; }
BinaryenOp BinaryenCopySignFloat64(void) { return CopySignFloat64; }
BinaryenOp BinaryenMinFloat64(void) { return MinFloat64; }
BinaryenOp BinaryenMaxFloat64(void) { return MaxFloat64; }
BinaryenOp BinaryenEqFloat64(void) { return EqFloat64; }
BinaryenOp BinaryenNeFloat64(void) { return NeFloat64; }
BinaryenOp BinaryenLtFloat64(void) { return LtFloat64; }
BinaryenOp BinaryenLeFloat64(void) { return LeFloat64; }
BinaryenOp BinaryenGtFloat64(void) { return GtFloat64; }
BinaryenOp BinaryenGeFloat64(void) { return GeFloat64; }
BinaryenOp BinaryenAtomicRMWAdd(void) { return RMWAdd; }
BinaryenOp BinaryenAtomicRMWSub(void) { return RMWSub; }
BinaryenOp BinaryenAtomicRMWAnd(void) { return RMWAnd; }
BinaryenOp BinaryenAtomicRMWOr(void) { return RMWOr; }
BinaryenOp BinaryenAtomicRMWXor(void) { return RMWXor; }
BinaryenOp BinaryenAtomicRMWXchg(void) { return RMWXchg; }
BinaryenOp BinaryenTruncSatSFloat32ToInt32(void) {
  return TruncSatSFloat32ToInt32;
}
BinaryenOp BinaryenTruncSatSFloat32ToInt64(void) {
  return TruncSatSFloat32ToInt64;
}
BinaryenOp BinaryenTruncSatUFloat32ToInt32(void) {
  return TruncSatUFloat32ToInt32;
}
BinaryenOp BinaryenTruncSatUFloat32ToInt64(void) {
  return TruncSatUFloat32ToInt64;
}
BinaryenOp BinaryenTruncSatSFloat64ToInt32(void) {
  return TruncSatSFloat64ToInt32;
}
BinaryenOp BinaryenTruncSatSFloat64ToInt64(void) {
  return TruncSatSFloat64ToInt64;
}
BinaryenOp BinaryenTruncSatUFloat64ToInt32(void) {
  return TruncSatUFloat64ToInt32;
}
BinaryenOp BinaryenTruncSatUFloat64ToInt64(void) {
  return TruncSatUFloat64ToInt64;
}
BinaryenOp BinaryenSplatVecI8x16(void) { return SplatVecI8x16; }
BinaryenOp BinaryenExtractLaneSVecI8x16(void) { return ExtractLaneSVecI8x16; }
BinaryenOp BinaryenExtractLaneUVecI8x16(void) { return ExtractLaneUVecI8x16; }
BinaryenOp BinaryenReplaceLaneVecI8x16(void) { return ReplaceLaneVecI8x16; }
BinaryenOp BinaryenSplatVecI16x8(void) { return SplatVecI16x8; }
BinaryenOp BinaryenExtractLaneSVecI16x8(void) { return ExtractLaneSVecI16x8; }
BinaryenOp BinaryenExtractLaneUVecI16x8(void) { return ExtractLaneUVecI16x8; }
BinaryenOp BinaryenReplaceLaneVecI16x8(void) { return ReplaceLaneVecI16x8; }
BinaryenOp BinaryenSplatVecI32x4(void) { return SplatVecI32x4; }
BinaryenOp BinaryenExtractLaneVecI32x4(void) { return ExtractLaneVecI32x4; }
BinaryenOp BinaryenReplaceLaneVecI32x4(void) { return ReplaceLaneVecI32x4; }
BinaryenOp BinaryenSplatVecI64x2(void) { return SplatVecI64x2; }
BinaryenOp BinaryenExtractLaneVecI64x2(void) { return ExtractLaneVecI64x2; }
BinaryenOp BinaryenReplaceLaneVecI64x2(void) { return ReplaceLaneVecI64x2; }
BinaryenOp BinaryenSplatVecF32x4(void) { return SplatVecF32x4; }
BinaryenOp BinaryenExtractLaneVecF32x4(void) { return ExtractLaneVecF32x4; }
BinaryenOp BinaryenReplaceLaneVecF32x4(void) { return ReplaceLaneVecF32x4; }
BinaryenOp BinaryenSplatVecF64x2(void) { return SplatVecF64x2; }
BinaryenOp BinaryenExtractLaneVecF64x2(void) { return ExtractLaneVecF64x2; }
BinaryenOp BinaryenReplaceLaneVecF64x2(void) { return ReplaceLaneVecF64x2; }
BinaryenOp BinaryenEqVecI8x16(void) { return EqVecI8x16; }
BinaryenOp BinaryenNeVecI8x16(void) { return NeVecI8x16; }
BinaryenOp BinaryenLtSVecI8x16(void) { return LtSVecI8x16; }
BinaryenOp BinaryenLtUVecI8x16(void) { return LtUVecI8x16; }
BinaryenOp BinaryenGtSVecI8x16(void) { return GtSVecI8x16; }
BinaryenOp BinaryenGtUVecI8x16(void) { return GtUVecI8x16; }
BinaryenOp BinaryenLeSVecI8x16(void) { return LeSVecI8x16; }
BinaryenOp BinaryenLeUVecI8x16(void) { return LeUVecI8x16; }
BinaryenOp BinaryenGeSVecI8x16(void) { return GeSVecI8x16; }
BinaryenOp BinaryenGeUVecI8x16(void) { return GeUVecI8x16; }
BinaryenOp BinaryenEqVecI16x8(void) { return EqVecI16x8; }
BinaryenOp BinaryenNeVecI16x8(void) { return NeVecI16x8; }
BinaryenOp BinaryenLtSVecI16x8(void) { return LtSVecI16x8; }
BinaryenOp BinaryenLtUVecI16x8(void) { return LtUVecI16x8; }
BinaryenOp BinaryenGtSVecI16x8(void) { return GtSVecI16x8; }
BinaryenOp BinaryenGtUVecI16x8(void) { return GtUVecI16x8; }
BinaryenOp BinaryenLeSVecI16x8(void) { return LeSVecI16x8; }
BinaryenOp BinaryenLeUVecI16x8(void) { return LeUVecI16x8; }
BinaryenOp BinaryenGeSVecI16x8(void) { return GeSVecI16x8; }
BinaryenOp BinaryenGeUVecI16x8(void) { return GeUVecI16x8; }
BinaryenOp BinaryenEqVecI32x4(void) { return EqVecI32x4; }
BinaryenOp BinaryenNeVecI32x4(void) { return NeVecI32x4; }
BinaryenOp BinaryenLtSVecI32x4(void) { return LtSVecI32x4; }
BinaryenOp BinaryenLtUVecI32x4(void) { return LtUVecI32x4; }
BinaryenOp BinaryenGtSVecI32x4(void) { return GtSVecI32x4; }
BinaryenOp BinaryenGtUVecI32x4(void) { return GtUVecI32x4; }
BinaryenOp BinaryenLeSVecI32x4(void) { return LeSVecI32x4; }
BinaryenOp BinaryenLeUVecI32x4(void) { return LeUVecI32x4; }
BinaryenOp BinaryenGeSVecI32x4(void) { return GeSVecI32x4; }
BinaryenOp BinaryenGeUVecI32x4(void) { return GeUVecI32x4; }
BinaryenOp BinaryenEqVecI64x2(void) { return EqVecI64x2; }
BinaryenOp BinaryenNeVecI64x2(void) { return NeVecI64x2; }
BinaryenOp BinaryenLtSVecI64x2(void) { return LtSVecI64x2; }
BinaryenOp BinaryenGtSVecI64x2(void) { return GtSVecI64x2; }
BinaryenOp BinaryenLeSVecI64x2(void) { return LeSVecI64x2; }
BinaryenOp BinaryenGeSVecI64x2(void) { return GeSVecI64x2; }
BinaryenOp BinaryenEqVecF32x4(void) { return EqVecF32x4; }
BinaryenOp BinaryenNeVecF32x4(void) { return NeVecF32x4; }
BinaryenOp BinaryenLtVecF32x4(void) { return LtVecF32x4; }
BinaryenOp BinaryenGtVecF32x4(void) { return GtVecF32x4; }
BinaryenOp BinaryenLeVecF32x4(void) { return LeVecF32x4; }
BinaryenOp BinaryenGeVecF32x4(void) { return GeVecF32x4; }
BinaryenOp BinaryenEqVecF64x2(void) { return EqVecF64x2; }
BinaryenOp BinaryenNeVecF64x2(void) { return NeVecF64x2; }
BinaryenOp BinaryenLtVecF64x2(void) { return LtVecF64x2; }
BinaryenOp BinaryenGtVecF64x2(void) { return GtVecF64x2; }
BinaryenOp BinaryenLeVecF64x2(void) { return LeVecF64x2; }
BinaryenOp BinaryenGeVecF64x2(void) { return GeVecF64x2; }
BinaryenOp BinaryenNotVec128(void) { return NotVec128; }
BinaryenOp BinaryenAndVec128(void) { return AndVec128; }
BinaryenOp BinaryenOrVec128(void) { return OrVec128; }
BinaryenOp BinaryenXorVec128(void) { return XorVec128; }
BinaryenOp BinaryenAndNotVec128(void) { return AndNotVec128; }
BinaryenOp BinaryenBitselectVec128(void) { return Bitselect; }
BinaryenOp BinaryenRelaxedFmaVecF32x4(void) { return RelaxedFmaVecF32x4; }
BinaryenOp BinaryenRelaxedFmsVecF32x4(void) { return RelaxedFmsVecF32x4; }
BinaryenOp BinaryenRelaxedFmaVecF64x2(void) { return RelaxedFmaVecF64x2; }
BinaryenOp BinaryenRelaxedFmsVecF64x2(void) { return RelaxedFmsVecF64x2; }
BinaryenOp BinaryenLaneselectI8x16(void) { return LaneselectI8x16; }
BinaryenOp BinaryenLaneselectI16x8(void) { return LaneselectI16x8; }
BinaryenOp BinaryenLaneselectI32x4(void) { return LaneselectI32x4; }
BinaryenOp BinaryenLaneselectI64x2(void) { return LaneselectI64x2; }
BinaryenOp BinaryenDotI8x16I7x16AddSToVecI32x4(void) {
  return DotI8x16I7x16AddSToVecI32x4;
}
BinaryenOp BinaryenAnyTrueVec128(void) { return AnyTrueVec128; }
BinaryenOp BinaryenAbsVecI8x16(void) { return AbsVecI8x16; }
BinaryenOp BinaryenNegVecI8x16(void) { return NegVecI8x16; }
BinaryenOp BinaryenAllTrueVecI8x16(void) { return AllTrueVecI8x16; }
BinaryenOp BinaryenBitmaskVecI8x16(void) { return BitmaskVecI8x16; }
BinaryenOp BinaryenPopcntVecI8x16(void) { return PopcntVecI8x16; }
BinaryenOp BinaryenShlVecI8x16(void) { return ShlVecI8x16; }
BinaryenOp BinaryenShrSVecI8x16(void) { return ShrSVecI8x16; }
BinaryenOp BinaryenShrUVecI8x16(void) { return ShrUVecI8x16; }
BinaryenOp BinaryenAddVecI8x16(void) { return AddVecI8x16; }
BinaryenOp BinaryenAddSatSVecI8x16(void) { return AddSatSVecI8x16; }
BinaryenOp BinaryenAddSatUVecI8x16(void) { return AddSatUVecI8x16; }
BinaryenOp BinaryenSubVecI8x16(void) { return SubVecI8x16; }
BinaryenOp BinaryenSubSatSVecI8x16(void) { return SubSatSVecI8x16; }
BinaryenOp BinaryenSubSatUVecI8x16(void) { return SubSatUVecI8x16; }
BinaryenOp BinaryenMinSVecI8x16(void) { return MinSVecI8x16; }
BinaryenOp BinaryenMinUVecI8x16(void) { return MinUVecI8x16; }
BinaryenOp BinaryenMaxSVecI8x16(void) { return MaxSVecI8x16; }
BinaryenOp BinaryenMaxUVecI8x16(void) { return MaxUVecI8x16; }
BinaryenOp BinaryenAvgrUVecI8x16(void) { return AvgrUVecI8x16; }
BinaryenOp BinaryenAbsVecI16x8(void) { return AbsVecI16x8; }
BinaryenOp BinaryenNegVecI16x8(void) { return NegVecI16x8; }
BinaryenOp BinaryenAllTrueVecI16x8(void) { return AllTrueVecI16x8; }
BinaryenOp BinaryenBitmaskVecI16x8(void) { return BitmaskVecI16x8; }
BinaryenOp BinaryenShlVecI16x8(void) { return ShlVecI16x8; }
BinaryenOp BinaryenShrSVecI16x8(void) { return ShrSVecI16x8; }
BinaryenOp BinaryenShrUVecI16x8(void) { return ShrUVecI16x8; }
BinaryenOp BinaryenAddVecI16x8(void) { return AddVecI16x8; }
BinaryenOp BinaryenAddSatSVecI16x8(void) { return AddSatSVecI16x8; }
BinaryenOp BinaryenAddSatUVecI16x8(void) { return AddSatUVecI16x8; }
BinaryenOp BinaryenSubVecI16x8(void) { return SubVecI16x8; }
BinaryenOp BinaryenSubSatSVecI16x8(void) { return SubSatSVecI16x8; }
BinaryenOp BinaryenSubSatUVecI16x8(void) { return SubSatUVecI16x8; }
BinaryenOp BinaryenMulVecI16x8(void) { return MulVecI16x8; }
BinaryenOp BinaryenMinSVecI16x8(void) { return MinSVecI16x8; }
BinaryenOp BinaryenMinUVecI16x8(void) { return MinUVecI16x8; }
BinaryenOp BinaryenMaxSVecI16x8(void) { return MaxSVecI16x8; }
BinaryenOp BinaryenMaxUVecI16x8(void) { return MaxUVecI16x8; }
BinaryenOp BinaryenAvgrUVecI16x8(void) { return AvgrUVecI16x8; }
BinaryenOp BinaryenQ15MulrSatSVecI16x8(void) { return Q15MulrSatSVecI16x8; }
BinaryenOp BinaryenExtMulLowSVecI16x8(void) { return ExtMulLowSVecI16x8; }
BinaryenOp BinaryenExtMulHighSVecI16x8(void) { return ExtMulHighSVecI16x8; }
BinaryenOp BinaryenExtMulLowUVecI16x8(void) { return ExtMulLowUVecI16x8; }
BinaryenOp BinaryenExtMulHighUVecI16x8(void) { return ExtMulHighUVecI16x8; }
BinaryenOp BinaryenAbsVecI32x4(void) { return AbsVecI32x4; }
BinaryenOp BinaryenNegVecI32x4(void) { return NegVecI32x4; }
BinaryenOp BinaryenAllTrueVecI32x4(void) { return AllTrueVecI32x4; }
BinaryenOp BinaryenBitmaskVecI32x4(void) { return BitmaskVecI32x4; }
BinaryenOp BinaryenShlVecI32x4(void) { return ShlVecI32x4; }
BinaryenOp BinaryenShrSVecI32x4(void) { return ShrSVecI32x4; }
BinaryenOp BinaryenShrUVecI32x4(void) { return ShrUVecI32x4; }
BinaryenOp BinaryenAddVecI32x4(void) { return AddVecI32x4; }
BinaryenOp BinaryenSubVecI32x4(void) { return SubVecI32x4; }
BinaryenOp BinaryenMulVecI32x4(void) { return MulVecI32x4; }
BinaryenOp BinaryenMinSVecI32x4(void) { return MinSVecI32x4; }
BinaryenOp BinaryenMinUVecI32x4(void) { return MinUVecI32x4; }
BinaryenOp BinaryenMaxSVecI32x4(void) { return MaxSVecI32x4; }
BinaryenOp BinaryenMaxUVecI32x4(void) { return MaxUVecI32x4; }
BinaryenOp BinaryenDotSVecI16x8ToVecI32x4(void) {
  return DotSVecI16x8ToVecI32x4;
}
BinaryenOp BinaryenExtMulLowSVecI32x4(void) { return ExtMulLowSVecI32x4; }
BinaryenOp BinaryenExtMulHighSVecI32x4(void) { return ExtMulHighSVecI32x4; }
BinaryenOp BinaryenExtMulLowUVecI32x4(void) { return ExtMulLowUVecI32x4; }
BinaryenOp BinaryenExtMulHighUVecI32x4(void) { return ExtMulHighUVecI32x4; }
BinaryenOp BinaryenAbsVecI64x2(void) { return AbsVecI64x2; }
BinaryenOp BinaryenNegVecI64x2(void) { return NegVecI64x2; }
BinaryenOp BinaryenAllTrueVecI64x2(void) { return AllTrueVecI64x2; }
BinaryenOp BinaryenBitmaskVecI64x2(void) { return BitmaskVecI64x2; }
BinaryenOp BinaryenShlVecI64x2(void) { return ShlVecI64x2; }
BinaryenOp BinaryenShrSVecI64x2(void) { return ShrSVecI64x2; }
BinaryenOp BinaryenShrUVecI64x2(void) { return ShrUVecI64x2; }
BinaryenOp BinaryenAddVecI64x2(void) { return AddVecI64x2; }
BinaryenOp BinaryenSubVecI64x2(void) { return SubVecI64x2; }
BinaryenOp BinaryenMulVecI64x2(void) { return MulVecI64x2; }
BinaryenOp BinaryenExtMulLowSVecI64x2(void) { return ExtMulLowSVecI64x2; }
BinaryenOp BinaryenExtMulHighSVecI64x2(void) { return ExtMulHighSVecI64x2; }
BinaryenOp BinaryenExtMulLowUVecI64x2(void) { return ExtMulLowUVecI64x2; }
BinaryenOp BinaryenExtMulHighUVecI64x2(void) { return ExtMulHighUVecI64x2; }
BinaryenOp BinaryenAbsVecF32x4(void) { return AbsVecF32x4; }
BinaryenOp BinaryenNegVecF32x4(void) { return NegVecF32x4; }
BinaryenOp BinaryenSqrtVecF32x4(void) { return SqrtVecF32x4; }
BinaryenOp BinaryenAddVecF32x4(void) { return AddVecF32x4; }
BinaryenOp BinaryenSubVecF32x4(void) { return SubVecF32x4; }
BinaryenOp BinaryenMulVecF32x4(void) { return MulVecF32x4; }
BinaryenOp BinaryenDivVecF32x4(void) { return DivVecF32x4; }
BinaryenOp BinaryenMinVecF32x4(void) { return MinVecF32x4; }
BinaryenOp BinaryenMaxVecF32x4(void) { return MaxVecF32x4; }
BinaryenOp BinaryenPMinVecF32x4(void) { return PMinVecF32x4; }
BinaryenOp BinaryenCeilVecF32x4(void) { return CeilVecF32x4; }
BinaryenOp BinaryenFloorVecF32x4(void) { return FloorVecF32x4; }
BinaryenOp BinaryenTruncVecF32x4(void) { return TruncVecF32x4; }
BinaryenOp BinaryenNearestVecF32x4(void) { return NearestVecF32x4; }
BinaryenOp BinaryenPMaxVecF32x4(void) { return PMaxVecF32x4; }
BinaryenOp BinaryenAbsVecF64x2(void) { return AbsVecF64x2; }
BinaryenOp BinaryenNegVecF64x2(void) { return NegVecF64x2; }
BinaryenOp BinaryenSqrtVecF64x2(void) { return SqrtVecF64x2; }
BinaryenOp BinaryenAddVecF64x2(void) { return AddVecF64x2; }
BinaryenOp BinaryenSubVecF64x2(void) { return SubVecF64x2; }
BinaryenOp BinaryenMulVecF64x2(void) { return MulVecF64x2; }
BinaryenOp BinaryenDivVecF64x2(void) { return DivVecF64x2; }
BinaryenOp BinaryenMinVecF64x2(void) { return MinVecF64x2; }
BinaryenOp BinaryenMaxVecF64x2(void) { return MaxVecF64x2; }
BinaryenOp BinaryenPMinVecF64x2(void) { return PMinVecF64x2; }
BinaryenOp BinaryenPMaxVecF64x2(void) { return PMaxVecF64x2; }
BinaryenOp BinaryenCeilVecF64x2(void) { return CeilVecF64x2; }
BinaryenOp BinaryenFloorVecF64x2(void) { return FloorVecF64x2; }
BinaryenOp BinaryenTruncVecF64x2(void) { return TruncVecF64x2; }
BinaryenOp BinaryenNearestVecF64x2(void) { return NearestVecF64x2; }
BinaryenOp BinaryenExtAddPairwiseSVecI8x16ToI16x8(void) {
  return ExtAddPairwiseSVecI8x16ToI16x8;
}
BinaryenOp BinaryenExtAddPairwiseUVecI8x16ToI16x8(void) {
  return ExtAddPairwiseUVecI8x16ToI16x8;
}
BinaryenOp BinaryenExtAddPairwiseSVecI16x8ToI32x4(void) {
  return ExtAddPairwiseSVecI16x8ToI32x4;
}
BinaryenOp BinaryenExtAddPairwiseUVecI16x8ToI32x4(void) {
  return ExtAddPairwiseUVecI16x8ToI32x4;
}
BinaryenOp BinaryenTruncSatSVecF32x4ToVecI32x4(void) {
  return TruncSatSVecF32x4ToVecI32x4;
}
BinaryenOp BinaryenTruncSatUVecF32x4ToVecI32x4(void) {
  return TruncSatUVecF32x4ToVecI32x4;
}
BinaryenOp BinaryenConvertSVecI32x4ToVecF32x4(void) {
  return ConvertSVecI32x4ToVecF32x4;
}
BinaryenOp BinaryenConvertUVecI32x4ToVecF32x4(void) {
  return ConvertUVecI32x4ToVecF32x4;
}
BinaryenOp BinaryenLoad8SplatVec128(void) { return Load8SplatVec128; }
BinaryenOp BinaryenLoad16SplatVec128(void) { return Load16SplatVec128; }
BinaryenOp BinaryenLoad32SplatVec128(void) { return Load32SplatVec128; }
BinaryenOp BinaryenLoad64SplatVec128(void) { return Load64SplatVec128; }
BinaryenOp BinaryenLoad8x8SVec128(void) { return Load8x8SVec128; }
BinaryenOp BinaryenLoad8x8UVec128(void) { return Load8x8UVec128; }
BinaryenOp BinaryenLoad16x4SVec128(void) { return Load16x4SVec128; }
BinaryenOp BinaryenLoad16x4UVec128(void) { return Load16x4UVec128; }
BinaryenOp BinaryenLoad32x2SVec128(void) { return Load32x2SVec128; }
BinaryenOp BinaryenLoad32x2UVec128(void) { return Load32x2UVec128; }
BinaryenOp BinaryenLoad32ZeroVec128(void) { return Load32ZeroVec128; }
BinaryenOp BinaryenLoad64ZeroVec128(void) { return Load64ZeroVec128; }
BinaryenOp BinaryenLoad8LaneVec128(void) { return Load8LaneVec128; }
BinaryenOp BinaryenLoad16LaneVec128(void) { return Load16LaneVec128; }
BinaryenOp BinaryenLoad32LaneVec128(void) { return Load32LaneVec128; }
BinaryenOp BinaryenLoad64LaneVec128(void) { return Load64LaneVec128; }
BinaryenOp BinaryenStore8LaneVec128(void) { return Store8LaneVec128; }
BinaryenOp BinaryenStore16LaneVec128(void) { return Store16LaneVec128; }
BinaryenOp BinaryenStore32LaneVec128(void) { return Store32LaneVec128; }
BinaryenOp BinaryenStore64LaneVec128(void) { return Store64LaneVec128; }
BinaryenOp BinaryenNarrowSVecI16x8ToVecI8x16(void) {
  return NarrowSVecI16x8ToVecI8x16;
}
BinaryenOp BinaryenNarrowUVecI16x8ToVecI8x16(void) {
  return NarrowUVecI16x8ToVecI8x16;
}
BinaryenOp BinaryenNarrowSVecI32x4ToVecI16x8(void) {
  return NarrowSVecI32x4ToVecI16x8;
}
BinaryenOp BinaryenNarrowUVecI32x4ToVecI16x8(void) {
  return NarrowUVecI32x4ToVecI16x8;
}
BinaryenOp BinaryenExtendLowSVecI8x16ToVecI16x8(void) {
  return ExtendLowSVecI8x16ToVecI16x8;
}
BinaryenOp BinaryenExtendHighSVecI8x16ToVecI16x8(void) {
  return ExtendHighSVecI8x16ToVecI16x8;
}
BinaryenOp BinaryenExtendLowUVecI8x16ToVecI16x8(void) {
  return ExtendLowUVecI8x16ToVecI16x8;
}
BinaryenOp BinaryenExtendHighUVecI8x16ToVecI16x8(void) {
  return ExtendHighUVecI8x16ToVecI16x8;
}
BinaryenOp BinaryenExtendLowSVecI16x8ToVecI32x4(void) {
  return ExtendLowSVecI16x8ToVecI32x4;
}
BinaryenOp BinaryenExtendHighSVecI16x8ToVecI32x4(void) {
  return ExtendHighSVecI16x8ToVecI32x4;
}
BinaryenOp BinaryenExtendLowUVecI16x8ToVecI32x4(void) {
  return ExtendLowUVecI16x8ToVecI32x4;
}
BinaryenOp BinaryenExtendHighUVecI16x8ToVecI32x4(void) {
  return ExtendHighUVecI16x8ToVecI32x4;
}
BinaryenOp BinaryenExtendLowSVecI32x4ToVecI64x2(void) {
  return ExtendLowSVecI32x4ToVecI64x2;
}
BinaryenOp BinaryenExtendHighSVecI32x4ToVecI64x2(void) {
  return ExtendHighSVecI32x4ToVecI64x2;
}
BinaryenOp BinaryenExtendLowUVecI32x4ToVecI64x2(void) {
  return ExtendLowUVecI32x4ToVecI64x2;
}
BinaryenOp BinaryenExtendHighUVecI32x4ToVecI64x2(void) {
  return ExtendHighUVecI32x4ToVecI64x2;
}
BinaryenOp BinaryenConvertLowSVecI32x4ToVecF64x2(void) {
  return ConvertLowSVecI32x4ToVecF64x2;
}
BinaryenOp BinaryenConvertLowUVecI32x4ToVecF64x2(void) {
  return ConvertLowUVecI32x4ToVecF64x2;
}
BinaryenOp BinaryenTruncSatZeroSVecF64x2ToVecI32x4(void) {
  return TruncSatZeroSVecF64x2ToVecI32x4;
}
BinaryenOp BinaryenTruncSatZeroUVecF64x2ToVecI32x4(void) {
  return TruncSatZeroUVecF64x2ToVecI32x4;
}
BinaryenOp BinaryenDemoteZeroVecF64x2ToVecF32x4(void) {
  return DemoteZeroVecF64x2ToVecF32x4;
}
BinaryenOp BinaryenPromoteLowVecF32x4ToVecF64x2(void) {
  return PromoteLowVecF32x4ToVecF64x2;
}
BinaryenOp BinaryenRelaxedTruncSVecF32x4ToVecI32x4(void) {
  return RelaxedTruncSVecF32x4ToVecI32x4;
}
BinaryenOp BinaryenRelaxedTruncUVecF32x4ToVecI32x4(void) {
  return RelaxedTruncUVecF32x4ToVecI32x4;
}
BinaryenOp BinaryenRelaxedTruncZeroSVecF64x2ToVecI32x4(void) {
  return RelaxedTruncZeroSVecF64x2ToVecI32x4;
}
BinaryenOp BinaryenRelaxedTruncZeroUVecF64x2ToVecI32x4(void) {
  return RelaxedTruncZeroUVecF64x2ToVecI32x4;
}
BinaryenOp BinaryenSwizzleVecI8x16(void) { return SwizzleVecI8x16; }
BinaryenOp BinaryenRelaxedSwizzleVecI8x16(void) {
  return RelaxedSwizzleVecI8x16;
}
BinaryenOp BinaryenRelaxedMinVecF32x4(void) { return RelaxedMinVecF32x4; }
BinaryenOp BinaryenRelaxedMaxVecF32x4(void) { return RelaxedMaxVecF32x4; }
BinaryenOp BinaryenRelaxedMinVecF64x2(void) { return RelaxedMinVecF64x2; }
BinaryenOp BinaryenRelaxedMaxVecF64x2(void) { return RelaxedMaxVecF64x2; }
BinaryenOp BinaryenRelaxedQ15MulrSVecI16x8(void) {
  return RelaxedQ15MulrSVecI16x8;
}
BinaryenOp BinaryenDotI8x16I7x16SToVecI16x8(void) {
  return DotI8x16I7x16SToVecI16x8;
}
BinaryenOp BinaryenRefAsNonNull(void) { return RefAsNonNull; }
BinaryenOp BinaryenRefAsExternInternalize(void) { return ExternInternalize; }
BinaryenOp BinaryenRefAsExternExternalize(void) { return ExternExternalize; }
BinaryenOp BinaryenBrOnNull(void) { return BrOnNull; }
BinaryenOp BinaryenBrOnNonNull(void) { return BrOnNonNull; }
BinaryenOp BinaryenBrOnCast(void) { return BrOnCast; }
BinaryenOp BinaryenBrOnCastFail(void) { return BrOnCastFail; };
BinaryenOp BinaryenStringNewUTF8(void) { return StringNewUTF8; }
BinaryenOp BinaryenStringNewWTF8(void) { return StringNewWTF8; }
BinaryenOp BinaryenStringNewReplace(void) { return StringNewReplace; }
BinaryenOp BinaryenStringNewWTF16(void) { return StringNewWTF16; }
BinaryenOp BinaryenStringNewUTF8Array(void) { return StringNewUTF8Array; }
BinaryenOp BinaryenStringNewWTF8Array(void) { return StringNewWTF8Array; }
BinaryenOp BinaryenStringNewReplaceArray(void) { return StringNewReplaceArray; }
BinaryenOp BinaryenStringNewWTF16Array(void) { return StringNewWTF16Array; }
BinaryenOp BinaryenStringNewFromCodePoint(void) {
  return StringNewFromCodePoint;
}
BinaryenOp BinaryenStringMeasureUTF8(void) { return StringMeasureUTF8; }
BinaryenOp BinaryenStringMeasureWTF8(void) { return StringMeasureWTF8; }
BinaryenOp BinaryenStringMeasureWTF16(void) { return StringMeasureWTF16; }
BinaryenOp BinaryenStringMeasureIsUSV(void) { return StringMeasureIsUSV; }
BinaryenOp BinaryenStringMeasureWTF16View(void) {
  return StringMeasureWTF16View;
}
BinaryenOp BinaryenStringEncodeUTF8(void) { return StringEncodeUTF8; }
BinaryenOp BinaryenStringEncodeWTF8(void) { return StringEncodeWTF8; }
BinaryenOp BinaryenStringEncodeWTF16(void) { return StringEncodeWTF16; }
BinaryenOp BinaryenStringEncodeUTF8Array(void) { return StringEncodeUTF8Array; }
BinaryenOp BinaryenStringEncodeWTF8Array(void) { return StringEncodeWTF8Array; }
BinaryenOp BinaryenStringEncodeWTF16Array(void) {
  return StringEncodeWTF16Array;
}
BinaryenOp BinaryenStringAsWTF8(void) { return StringAsWTF8; }
BinaryenOp BinaryenStringAsWTF16(void) { return StringAsWTF16; }
BinaryenOp BinaryenStringAsIter(void) { return StringAsIter; }
BinaryenOp BinaryenStringIterMoveAdvance(void) { return StringIterMoveAdvance; }
BinaryenOp BinaryenStringIterMoveRewind(void) { return StringIterMoveRewind; }
BinaryenOp BinaryenStringSliceWTF8(void) { return StringSliceWTF8; }
BinaryenOp BinaryenStringSliceWTF16(void) { return StringSliceWTF16; }
BinaryenOp BinaryenStringEqEqual(void) { return StringEqEqual; }
BinaryenOp BinaryenStringEqCompare(void) { return StringEqCompare; }

BinaryenExpressionRef BinaryenBlock(BinaryenModuleRef module,
                                    const char* name,
                                    BinaryenExpressionRef* children,
                                    BinaryenIndex numChildren,
                                    BinaryenType type) {
  auto* ret = ((Module*)module)->allocator.alloc<Block>();
  if (name) {
    ret->name = name;
  }
  for (BinaryenIndex i = 0; i < numChildren; i++) {
    ret->list.push_back((Expression*)children[i]);
  }
  if (type != BinaryenTypeAuto()) {
    ret->finalize(Type(type));
  } else {
    ret->finalize();
  }
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenIf(BinaryenModuleRef module,
                                 BinaryenExpressionRef condition,
                                 BinaryenExpressionRef ifTrue,
                                 BinaryenExpressionRef ifFalse) {
  return static_cast<Expression*>(Builder(*(Module*)module)
                                    .makeIf((Expression*)condition,
                                            (Expression*)ifTrue,
                                            (Expression*)ifFalse));
}
BinaryenExpressionRef BinaryenLoop(BinaryenModuleRef module,
                                   const char* name,
                                   BinaryenExpressionRef body) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeLoop(name ? Name(name) : Name(), (Expression*)body));
}
BinaryenExpressionRef BinaryenBreak(BinaryenModuleRef module,
                                    const char* name,
                                    BinaryenExpressionRef condition,
                                    BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeBreak(name, (Expression*)value, (Expression*)condition));
}
BinaryenExpressionRef BinaryenSwitch(BinaryenModuleRef module,
                                     const char** names,
                                     BinaryenIndex numNames,
                                     const char* defaultName,
                                     BinaryenExpressionRef condition,
                                     BinaryenExpressionRef value) {
  auto* ret = ((Module*)module)->allocator.alloc<Switch>();
  for (BinaryenIndex i = 0; i < numNames; i++) {
    ret->targets.push_back(names[i]);
  }
  ret->default_ = defaultName;
  ret->condition = (Expression*)condition;
  ret->value = (Expression*)value;
  ret->finalize();
  return static_cast<Expression*>(ret);
}
static BinaryenExpressionRef makeBinaryenCall(BinaryenModuleRef module,
                                              const char* target,
                                              BinaryenExpressionRef* operands,
                                              BinaryenIndex numOperands,
                                              BinaryenType returnType,
                                              bool isReturn) {
  auto* ret = ((Module*)module)->allocator.alloc<Call>();
  ret->target = target;
  for (BinaryenIndex i = 0; i < numOperands; i++) {
    ret->operands.push_back((Expression*)operands[i]);
  }
  ret->type = Type(returnType);
  ret->isReturn = isReturn;
  ret->finalize();
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenCall(BinaryenModuleRef module,
                                   const char* target,
                                   BinaryenExpressionRef* operands,
                                   BinaryenIndex numOperands,
                                   BinaryenType returnType) {
  return makeBinaryenCall(
    module, target, operands, numOperands, returnType, false);
}
BinaryenExpressionRef BinaryenReturnCall(BinaryenModuleRef module,
                                         const char* target,
                                         BinaryenExpressionRef* operands,
                                         BinaryenIndex numOperands,
                                         BinaryenType returnType) {
  return makeBinaryenCall(
    module, target, operands, numOperands, returnType, true);
}
static BinaryenExpressionRef
makeBinaryenCallIndirect(BinaryenModuleRef module,
                         const char* table,
                         BinaryenExpressionRef target,
                         BinaryenExpressionRef* operands,
                         BinaryenIndex numOperands,
                         BinaryenType params,
                         BinaryenType results,
                         bool isReturn) {
  auto* ret = ((Module*)module)->allocator.alloc<CallIndirect>();
  ret->table = table;
  ret->target = (Expression*)target;
  for (BinaryenIndex i = 0; i < numOperands; i++) {
    ret->operands.push_back((Expression*)operands[i]);
  }
  ret->heapType = Signature(Type(params), Type(results));
  ret->type = Type(results);
  ret->isReturn = isReturn;
  ret->finalize();
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenCallIndirect(BinaryenModuleRef module,
                                           const char* table,
                                           BinaryenExpressionRef target,
                                           BinaryenExpressionRef* operands,
                                           BinaryenIndex numOperands,
                                           BinaryenType params,
                                           BinaryenType results) {
  return makeBinaryenCallIndirect(
    module, table, target, operands, numOperands, params, results, false);
}
BinaryenExpressionRef
BinaryenReturnCallIndirect(BinaryenModuleRef module,
                           const char* table,
                           BinaryenExpressionRef target,
                           BinaryenExpressionRef* operands,
                           BinaryenIndex numOperands,
                           BinaryenType params,
                           BinaryenType results) {
  return makeBinaryenCallIndirect(
    module, table, target, operands, numOperands, params, results, true);
}
BinaryenExpressionRef BinaryenLocalGet(BinaryenModuleRef module,
                                       BinaryenIndex index,
                                       BinaryenType type) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeLocalGet(index, Type(type)));
}
BinaryenExpressionRef BinaryenLocalSet(BinaryenModuleRef module,
                                       BinaryenIndex index,
                                       BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeLocalSet(index, (Expression*)value));
}
BinaryenExpressionRef BinaryenLocalTee(BinaryenModuleRef module,
                                       BinaryenIndex index,
                                       BinaryenExpressionRef value,
                                       BinaryenType type) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeLocalTee(index, (Expression*)value, Type(type)));
}
BinaryenExpressionRef BinaryenGlobalGet(BinaryenModuleRef module,
                                        const char* name,
                                        BinaryenType type) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeGlobalGet(name, Type(type)));
}
BinaryenExpressionRef BinaryenGlobalSet(BinaryenModuleRef module,
                                        const char* name,
                                        BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeGlobalSet(name, (Expression*)value));
}

// All memory instructions should pass their memory name parameter through this
// helper function. It maintains compatibility for when JS calls memory
// instructions that don't specify a memory name (send null), by assuming the
// singly defined memory is the intended one. This function takes in the memory
// name passed to API functions to avoid duplicating the nullptr logic check in
// each instruction
static Name getMemoryName(BinaryenModuleRef module, const char* memoryName) {
  if (memoryName == nullptr && module->memories.size() == 1) {
    return module->memories[0]->name;
  }

  return memoryName;
}

BinaryenExpressionRef BinaryenLoad(BinaryenModuleRef module,
                                   uint32_t bytes,
                                   bool signed_,
                                   uint32_t offset,
                                   uint32_t align,
                                   BinaryenType type,
                                   BinaryenExpressionRef ptr,
                                   const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeLoad(bytes,
                !!signed_,
                offset,
                align ? align : bytes,
                (Expression*)ptr,
                Type(type),
                getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenStore(BinaryenModuleRef module,
                                    uint32_t bytes,
                                    uint32_t offset,
                                    uint32_t align,
                                    BinaryenExpressionRef ptr,
                                    BinaryenExpressionRef value,
                                    BinaryenType type,
                                    const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStore(bytes,
                 offset,
                 align ? align : bytes,
                 (Expression*)ptr,
                 (Expression*)value,
                 Type(type),
                 getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenConst(BinaryenModuleRef module,
                                    BinaryenLiteral value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeConst(fromBinaryenLiteral(value)));
}
BinaryenExpressionRef BinaryenUnary(BinaryenModuleRef module,
                                    BinaryenOp op,
                                    BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeUnary(UnaryOp(op), (Expression*)value));
}
BinaryenExpressionRef BinaryenBinary(BinaryenModuleRef module,
                                     BinaryenOp op,
                                     BinaryenExpressionRef left,
                                     BinaryenExpressionRef right) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeBinary(BinaryOp(op), (Expression*)left, (Expression*)right));
}
BinaryenExpressionRef BinaryenSelect(BinaryenModuleRef module,
                                     BinaryenExpressionRef condition,
                                     BinaryenExpressionRef ifTrue,
                                     BinaryenExpressionRef ifFalse,
                                     BinaryenType type) {
  auto* ret = ((Module*)module)->allocator.alloc<Select>();
  ret->condition = (Expression*)condition;
  ret->ifTrue = (Expression*)ifTrue;
  ret->ifFalse = (Expression*)ifFalse;
  if (type != BinaryenTypeAuto()) {
    ret->finalize(Type(type));
  } else {
    ret->finalize();
  }
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenDrop(BinaryenModuleRef module,
                                   BinaryenExpressionRef value) {
  auto* ret = Builder(*(Module*)module).makeDrop((Expression*)value);
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenReturn(BinaryenModuleRef module,
                                     BinaryenExpressionRef value) {
  auto* ret = Builder(*(Module*)module).makeReturn((Expression*)value);
  return static_cast<Expression*>(ret);
}

static Builder::MemoryInfo getMemoryInfo(bool memoryIs64) {
  return memoryIs64 ? Builder::MemoryInfo::Memory64
                    : Builder::MemoryInfo::Memory32;
}

BinaryenExpressionRef BinaryenMemorySize(BinaryenModuleRef module,
                                         const char* memoryName,
                                         bool memoryIs64) {
  auto* ret = Builder(*(Module*)module)
                .makeMemorySize(getMemoryName(module, memoryName),
                                getMemoryInfo(memoryIs64));
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenMemoryGrow(BinaryenModuleRef module,
                                         BinaryenExpressionRef delta,
                                         const char* memoryName,
                                         bool memoryIs64) {
  auto* ret = Builder(*(Module*)module)
                .makeMemoryGrow((Expression*)delta,
                                getMemoryName(module, memoryName),
                                getMemoryInfo(memoryIs64));
  return static_cast<Expression*>(ret);
}
BinaryenExpressionRef BinaryenNop(BinaryenModuleRef module) {
  return static_cast<Expression*>(Builder(*(Module*)module).makeNop());
}
BinaryenExpressionRef BinaryenUnreachable(BinaryenModuleRef module) {
  return static_cast<Expression*>(Builder(*(Module*)module).makeUnreachable());
}
BinaryenExpressionRef BinaryenAtomicLoad(BinaryenModuleRef module,
                                         uint32_t bytes,
                                         uint32_t offset,
                                         BinaryenType type,
                                         BinaryenExpressionRef ptr,
                                         const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeAtomicLoad(bytes,
                      offset,
                      (Expression*)ptr,
                      Type(type),
                      getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenAtomicStore(BinaryenModuleRef module,
                                          uint32_t bytes,
                                          uint32_t offset,
                                          BinaryenExpressionRef ptr,
                                          BinaryenExpressionRef value,
                                          BinaryenType type,
                                          const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeAtomicStore(bytes,
                       offset,
                       (Expression*)ptr,
                       (Expression*)value,
                       Type(type),
                       getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenAtomicRMW(BinaryenModuleRef module,
                                        BinaryenOp op,
                                        BinaryenIndex bytes,
                                        BinaryenIndex offset,
                                        BinaryenExpressionRef ptr,
                                        BinaryenExpressionRef value,
                                        BinaryenType type,
                                        const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeAtomicRMW(AtomicRMWOp(op),
                     bytes,
                     offset,
                     (Expression*)ptr,
                     (Expression*)value,
                     Type(type),
                     getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenAtomicCmpxchg(BinaryenModuleRef module,
                                            BinaryenIndex bytes,
                                            BinaryenIndex offset,
                                            BinaryenExpressionRef ptr,
                                            BinaryenExpressionRef expected,
                                            BinaryenExpressionRef replacement,
                                            BinaryenType type,
                                            const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeAtomicCmpxchg(bytes,
                         offset,
                         (Expression*)ptr,
                         (Expression*)expected,
                         (Expression*)replacement,
                         Type(type),
                         getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenAtomicWait(BinaryenModuleRef module,
                                         BinaryenExpressionRef ptr,
                                         BinaryenExpressionRef expected,
                                         BinaryenExpressionRef timeout,
                                         BinaryenType expectedType,
                                         const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeAtomicWait((Expression*)ptr,
                      (Expression*)expected,
                      (Expression*)timeout,
                      Type(expectedType),
                      0,
                      getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenAtomicNotify(BinaryenModuleRef module,
                                           BinaryenExpressionRef ptr,
                                           BinaryenExpressionRef notifyCount,
                                           const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeAtomicNotify((Expression*)ptr,
                        (Expression*)notifyCount,
                        0,
                        getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenAtomicFence(BinaryenModuleRef module) {
  return static_cast<Expression*>(Builder(*(Module*)module).makeAtomicFence());
}
BinaryenExpressionRef BinaryenSIMDExtract(BinaryenModuleRef module,
                                          BinaryenOp op,
                                          BinaryenExpressionRef vec,
                                          uint8_t index) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDExtract(SIMDExtractOp(op), (Expression*)vec, index));
}
BinaryenExpressionRef BinaryenSIMDReplace(BinaryenModuleRef module,
                                          BinaryenOp op,
                                          BinaryenExpressionRef vec,
                                          uint8_t index,
                                          BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDReplace(
        SIMDReplaceOp(op), (Expression*)vec, index, (Expression*)value));
}
BinaryenExpressionRef BinaryenSIMDShuffle(BinaryenModuleRef module,
                                          BinaryenExpressionRef left,
                                          BinaryenExpressionRef right,
                                          const uint8_t mask_[16]) {
  assert(mask_); // nullptr would be wrong
  std::array<uint8_t, 16> mask;
  memcpy(mask.data(), mask_, 16);
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDShuffle((Expression*)left, (Expression*)right, mask));
}
BinaryenExpressionRef BinaryenSIMDTernary(BinaryenModuleRef module,
                                          BinaryenOp op,
                                          BinaryenExpressionRef a,
                                          BinaryenExpressionRef b,
                                          BinaryenExpressionRef c) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDTernary(
        SIMDTernaryOp(op), (Expression*)a, (Expression*)b, (Expression*)c));
}
BinaryenExpressionRef BinaryenSIMDShift(BinaryenModuleRef module,
                                        BinaryenOp op,
                                        BinaryenExpressionRef vec,
                                        BinaryenExpressionRef shift) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDShift(SIMDShiftOp(op), (Expression*)vec, (Expression*)shift));
}
BinaryenExpressionRef BinaryenSIMDLoad(BinaryenModuleRef module,
                                       BinaryenOp op,
                                       uint32_t offset,
                                       uint32_t align,
                                       BinaryenExpressionRef ptr,
                                       const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDLoad(SIMDLoadOp(op),
                    Address(offset),
                    Address(align),
                    (Expression*)ptr,
                    getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenSIMDLoadStoreLane(BinaryenModuleRef module,
                                                BinaryenOp op,
                                                uint32_t offset,
                                                uint32_t align,
                                                uint8_t index,
                                                BinaryenExpressionRef ptr,
                                                BinaryenExpressionRef vec,
                                                const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp(op),
                             Address(offset),
                             Address(align),
                             index,
                             (Expression*)ptr,
                             (Expression*)vec,
                             getMemoryName(module, memoryName)));
}
BinaryenExpressionRef BinaryenMemoryInit(BinaryenModuleRef module,
                                         const char* segment,
                                         BinaryenExpressionRef dest,
                                         BinaryenExpressionRef offset,
                                         BinaryenExpressionRef size,
                                         const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeMemoryInit(Name(segment),
                      (Expression*)dest,
                      (Expression*)offset,
                      (Expression*)size,
                      getMemoryName(module, memoryName)));
}

BinaryenExpressionRef BinaryenDataDrop(BinaryenModuleRef module,
                                       const char* segment) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeDataDrop(Name(segment)));
}

BinaryenExpressionRef BinaryenMemoryCopy(BinaryenModuleRef module,
                                         BinaryenExpressionRef dest,
                                         BinaryenExpressionRef source,
                                         BinaryenExpressionRef size,
                                         const char* destMemory,
                                         const char* sourceMemory) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeMemoryCopy((Expression*)dest,
                      (Expression*)source,
                      (Expression*)size,
                      getMemoryName(module, destMemory),
                      getMemoryName(module, sourceMemory)));
}

BinaryenExpressionRef BinaryenMemoryFill(BinaryenModuleRef module,
                                         BinaryenExpressionRef dest,
                                         BinaryenExpressionRef value,
                                         BinaryenExpressionRef size,
                                         const char* memoryName) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeMemoryFill((Expression*)dest,
                      (Expression*)value,
                      (Expression*)size,
                      getMemoryName(module, memoryName)));
}

BinaryenExpressionRef BinaryenTupleMake(BinaryenModuleRef module,
                                        BinaryenExpressionRef* operands,
                                        BinaryenIndex numOperands) {
  std::vector<Expression*> ops;
  ops.resize(numOperands);
  for (size_t i = 0; i < numOperands; ++i) {
    ops[i] = (Expression*)operands[i];
  }
  return static_cast<Expression*>(Builder(*(Module*)module).makeTupleMake(ops));
}

BinaryenExpressionRef BinaryenTupleExtract(BinaryenModuleRef module,
                                           BinaryenExpressionRef tuple,
                                           BinaryenIndex index) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeTupleExtract((Expression*)tuple, index));
}

BinaryenExpressionRef BinaryenPop(BinaryenModuleRef module, BinaryenType type) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makePop(Type(type)));
}

BinaryenExpressionRef BinaryenRefNull(BinaryenModuleRef module,
                                      BinaryenType type) {
  Type type_(type);
  assert(type_.isNullable());
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRefNull(type_.getHeapType()));
}

BinaryenExpressionRef BinaryenRefIsNull(BinaryenModuleRef module,
                                        BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRefIsNull((Expression*)value));
}

BinaryenExpressionRef BinaryenRefAs(BinaryenModuleRef module,
                                    BinaryenOp op,
                                    BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRefAs(RefAsOp(op), (Expression*)value));
}

BinaryenExpressionRef
BinaryenRefFunc(BinaryenModuleRef module, const char* func, BinaryenType type) {
  // TODO: consider changing the C API to receive a heap type
  Type type_(type);
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRefFunc(func, type_.getHeapType()));
}

BinaryenExpressionRef BinaryenRefEq(BinaryenModuleRef module,
                                    BinaryenExpressionRef left,
                                    BinaryenExpressionRef right) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRefEq((Expression*)left, (Expression*)right));
}

BinaryenExpressionRef BinaryenTableGet(BinaryenModuleRef module,
                                       const char* name,
                                       BinaryenExpressionRef index,
                                       BinaryenType type) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeTableGet(name, (Expression*)index, Type(type)));
}

BinaryenExpressionRef BinaryenTableSet(BinaryenModuleRef module,
                                       const char* name,
                                       BinaryenExpressionRef index,
                                       BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeTableSet(name, (Expression*)index, (Expression*)value));
}

BinaryenExpressionRef BinaryenTableSize(BinaryenModuleRef module,
                                        const char* name) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeTableSize(name));
}

BinaryenExpressionRef BinaryenTableGrow(BinaryenModuleRef module,
                                        const char* name,
                                        BinaryenExpressionRef value,
                                        BinaryenExpressionRef delta) {
  if (value == nullptr) {
    auto tableType = (*(Module*)module).getTableOrNull(name)->type;
    value = BinaryenRefNull(module, (BinaryenType)tableType.getID());
  }
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeTableGrow(name, (Expression*)value, (Expression*)delta));
}

BinaryenExpressionRef BinaryenTry(BinaryenModuleRef module,
                                  const char* name,
                                  BinaryenExpressionRef body,
                                  const char** catchTags,
                                  BinaryenIndex numCatchTags,
                                  BinaryenExpressionRef* catchBodies,
                                  BinaryenIndex numCatchBodies,
                                  const char* delegateTarget) {
  auto* ret = ((Module*)module)->allocator.alloc<Try>();
  if (name) {
    ret->name = name;
  }
  ret->body = (Expression*)body;
  for (BinaryenIndex i = 0; i < numCatchTags; i++) {
    ret->catchTags.push_back(catchTags[i]);
  }
  for (BinaryenIndex i = 0; i < numCatchBodies; i++) {
    ret->catchBodies.push_back((Expression*)catchBodies[i]);
  }
  if (delegateTarget) {
    ret->delegateTarget = delegateTarget;
  }
  ret->finalize();
  return static_cast<Expression*>(ret);
}

BinaryenExpressionRef BinaryenThrow(BinaryenModuleRef module,
                                    const char* tag,
                                    BinaryenExpressionRef* operands,
                                    BinaryenIndex numOperands) {
  std::vector<Expression*> args;
  for (BinaryenIndex i = 0; i < numOperands; i++) {
    args.push_back((Expression*)operands[i]);
  }
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeThrow(tag, args));
}

BinaryenExpressionRef BinaryenRethrow(BinaryenModuleRef module,
                                      const char* target) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRethrow(target));
}

BinaryenExpressionRef BinaryenI31New(BinaryenModuleRef module,
                                     BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeI31New((Expression*)value));
}

BinaryenExpressionRef BinaryenI31Get(BinaryenModuleRef module,
                                     BinaryenExpressionRef i31,
                                     bool signed_) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeI31Get((Expression*)i31, signed_ != 0));
}
BinaryenExpressionRef BinaryenCallRef(BinaryenModuleRef module,
                                      BinaryenExpressionRef target,
                                      BinaryenExpressionRef* operands,
                                      BinaryenIndex numOperands,
                                      BinaryenType type,
                                      bool isReturn) {
  std::vector<Expression*> args;
  for (BinaryenIndex i = 0; i < numOperands; i++) {
    args.push_back((Expression*)operands[i]);
  }
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeCallRef((Expression*)target, args, Type(type), isReturn));
}
BinaryenExpressionRef BinaryenRefTest(BinaryenModuleRef module,
                                      BinaryenExpressionRef ref,
                                      BinaryenType castType) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeRefTest((Expression*)ref, Type(castType)));
}
BinaryenExpressionRef BinaryenRefCast(BinaryenModuleRef module,
                                      BinaryenExpressionRef ref,
                                      BinaryenType type) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeRefCast((Expression*)ref, Type(type), RefCast::Safety::Safe));
}
BinaryenExpressionRef BinaryenBrOn(BinaryenModuleRef module,
                                   BinaryenOp op,
                                   const char* name,
                                   BinaryenExpressionRef ref,
                                   BinaryenType castType) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeBrOn(BrOnOp(op), name, (Expression*)ref, Type(castType)));
}
BinaryenExpressionRef BinaryenStructNew(BinaryenModuleRef module,
                                        BinaryenExpressionRef* operands,
                                        BinaryenIndex numOperands,
                                        BinaryenHeapType type) {
  std::vector<Expression*> args;
  for (BinaryenIndex i = 0; i < numOperands; i++) {
    args.push_back((Expression*)operands[i]);
  }
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeStructNew(HeapType(type), args));
}
BinaryenExpressionRef BinaryenStructGet(BinaryenModuleRef module,
                                        BinaryenIndex index,
                                        BinaryenExpressionRef ref,
                                        BinaryenType type,
                                        bool signed_) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStructGet(index, (Expression*)ref, Type(type), signed_));
}
BinaryenExpressionRef BinaryenStructSet(BinaryenModuleRef module,
                                        BinaryenIndex index,
                                        BinaryenExpressionRef ref,
                                        BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStructSet(index, (Expression*)ref, (Expression*)value));
}
BinaryenExpressionRef BinaryenArrayNew(BinaryenModuleRef module,
                                       BinaryenHeapType type,
                                       BinaryenExpressionRef size,
                                       BinaryenExpressionRef init) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeArrayNew(HeapType(type), (Expression*)size, (Expression*)init));
}
BinaryenExpressionRef BinaryenArrayNewFixed(BinaryenModuleRef module,
                                            BinaryenHeapType type,
                                            BinaryenExpressionRef* values,
                                            BinaryenIndex numValues) {
  std::vector<Expression*> vals;
  for (BinaryenIndex i = 0; i < numValues; i++) {
    vals.push_back((Expression*)values[i]);
  }
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeArrayNewFixed(HeapType(type), vals));
}
BinaryenExpressionRef BinaryenArrayGet(BinaryenModuleRef module,
                                       BinaryenExpressionRef ref,
                                       BinaryenExpressionRef index,
                                       BinaryenType type,
                                       bool signed_) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeArrayGet((Expression*)ref, (Expression*)index, Type(type), signed_));
}
BinaryenExpressionRef BinaryenArraySet(BinaryenModuleRef module,
                                       BinaryenExpressionRef ref,
                                       BinaryenExpressionRef index,
                                       BinaryenExpressionRef value) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeArraySet((Expression*)ref, (Expression*)index, (Expression*)value));
}
BinaryenExpressionRef BinaryenArrayLen(BinaryenModuleRef module,
                                       BinaryenExpressionRef ref) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeArrayLen((Expression*)ref));
}
BinaryenExpressionRef BinaryenArrayCopy(BinaryenModuleRef module,
                                        BinaryenExpressionRef destRef,
                                        BinaryenExpressionRef destIndex,
                                        BinaryenExpressionRef srcRef,
                                        BinaryenExpressionRef srcIndex,
                                        BinaryenExpressionRef length) {
  return static_cast<Expression*>(Builder(*(Module*)module)
                                    .makeArrayCopy((Expression*)destRef,
                                                   (Expression*)destIndex,
                                                   (Expression*)srcRef,
                                                   (Expression*)srcIndex,
                                                   (Expression*)length));
}
BinaryenExpressionRef BinaryenStringNew(BinaryenModuleRef module,
                                        BinaryenOp op,
                                        BinaryenExpressionRef ptr,
                                        BinaryenExpressionRef length,
                                        BinaryenExpressionRef start,
                                        BinaryenExpressionRef end,
                                        bool try_) {
  Builder builder(*(Module*)module);
  return static_cast<Expression*>(
    length ? builder.makeStringNew(
               StringNewOp(op), (Expression*)ptr, (Expression*)length, try_)
           : builder.makeStringNew(StringNewOp(op),
                                   (Expression*)ptr,
                                   (Expression*)start,
                                   (Expression*)end,
                                   try_));
}
BinaryenExpressionRef BinaryenStringConst(BinaryenModuleRef module,
                                          const char* name) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeStringConst(name));
}
BinaryenExpressionRef BinaryenStringMeasure(BinaryenModuleRef module,
                                            BinaryenOp op,
                                            BinaryenExpressionRef ref) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStringMeasure(StringMeasureOp(op), (Expression*)ref));
}
BinaryenExpressionRef BinaryenStringEncode(BinaryenModuleRef module,
                                           BinaryenOp op,
                                           BinaryenExpressionRef ref,
                                           BinaryenExpressionRef ptr,
                                           BinaryenExpressionRef start) {
  return static_cast<Expression*>(Builder(*(Module*)module)
                                    .makeStringEncode(StringEncodeOp(op),
                                                      (Expression*)ref,
                                                      (Expression*)ptr,
                                                      (Expression*)start));
}
BinaryenExpressionRef BinaryenStringConcat(BinaryenModuleRef module,
                                           BinaryenExpressionRef left,
                                           BinaryenExpressionRef right) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStringConcat((Expression*)left, (Expression*)right));
}
BinaryenExpressionRef BinaryenStringEq(BinaryenModuleRef module,
                                       BinaryenOp op,
                                       BinaryenExpressionRef left,
                                       BinaryenExpressionRef right) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStringEq(StringEqOp(op), (Expression*)left, (Expression*)right));
}
BinaryenExpressionRef BinaryenStringAs(BinaryenModuleRef module,
                                       BinaryenOp op,
                                       BinaryenExpressionRef ref) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeStringAs(StringAsOp(op), (Expression*)ref));
}
BinaryenExpressionRef BinaryenStringWTF8Advance(BinaryenModuleRef module,
                                                BinaryenExpressionRef ref,
                                                BinaryenExpressionRef pos,
                                                BinaryenExpressionRef bytes) {
  return static_cast<Expression*>(Builder(*(Module*)module)
                                    .makeStringWTF8Advance((Expression*)ref,
                                                           (Expression*)pos,
                                                           (Expression*)bytes));
}
BinaryenExpressionRef BinaryenStringWTF16Get(BinaryenModuleRef module,
                                             BinaryenExpressionRef ref,
                                             BinaryenExpressionRef pos) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStringWTF16Get((Expression*)ref, (Expression*)pos));
}
BinaryenExpressionRef BinaryenStringIterNext(BinaryenModuleRef module,
                                             BinaryenExpressionRef ref) {
  return static_cast<Expression*>(
    Builder(*(Module*)module).makeStringIterNext((Expression*)ref));
}
BinaryenExpressionRef BinaryenStringIterMove(BinaryenModuleRef module,
                                             BinaryenOp op,
                                             BinaryenExpressionRef ref,
                                             BinaryenExpressionRef num) {
  return static_cast<Expression*>(Builder(*(Module*)module)
                                    .makeStringIterMove(StringIterMoveOp(op),
                                                        (Expression*)ref,
                                                        (Expression*)num));
}
BinaryenExpressionRef BinaryenStringSliceWTF(BinaryenModuleRef module,
                                             BinaryenOp op,
                                             BinaryenExpressionRef ref,
                                             BinaryenExpressionRef start,
                                             BinaryenExpressionRef end) {
  return static_cast<Expression*>(Builder(*(Module*)module)
                                    .makeStringSliceWTF(StringSliceWTFOp(op),
                                                        (Expression*)ref,
                                                        (Expression*)start,
                                                        (Expression*)end));
}
BinaryenExpressionRef BinaryenStringSliceIter(BinaryenModuleRef module,
                                              BinaryenExpressionRef ref,
                                              BinaryenExpressionRef num) {
  return static_cast<Expression*>(
    Builder(*(Module*)module)
      .makeStringSliceIter((Expression*)ref, (Expression*)num));
}

// Expression utility

BinaryenExpressionId BinaryenExpressionGetId(BinaryenExpressionRef expr) {
  return ((Expression*)expr)->_id;
}
BinaryenType BinaryenExpressionGetType(BinaryenExpressionRef expr) {
  return ((Expression*)expr)->type.getID();
}
void BinaryenExpressionSetType(BinaryenExpressionRef expr, BinaryenType type) {
  ((Expression*)expr)->type = Type(type);
}
void BinaryenExpressionPrint(BinaryenExpressionRef expr) {
  std::cout << *(Expression*)expr << '\n';
}
void BinaryenExpressionFinalize(BinaryenExpressionRef expr) {
  ReFinalizeNode().visit((Expression*)expr);
}

BinaryenExpressionRef BinaryenExpressionCopy(BinaryenExpressionRef expr,
                                             BinaryenModuleRef module) {
  return ExpressionManipulator::copy(expr, *(Module*)module);
}

// Specific expression utility

// Block
const char* BinaryenBlockGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  return static_cast<Block*>(expression)->name.str.data();
}
void BinaryenBlockSetName(BinaryenExpressionRef expr, const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  // may be null or empty
  static_cast<Block*>(expression)->name = name;
}
BinaryenIndex BinaryenBlockGetNumChildren(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  return static_cast<Block*>(expression)->list.size();
}
BinaryenExpressionRef BinaryenBlockGetChildAt(BinaryenExpressionRef expr,
                                              BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  assert(index < static_cast<Block*>(expression)->list.size());
  return static_cast<Block*>(expression)->list[index];
}
void BinaryenBlockSetChildAt(BinaryenExpressionRef expr,
                             BinaryenIndex index,
                             BinaryenExpressionRef childExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  assert(childExpr);
  auto& list = static_cast<Block*>(expression)->list;
  assert(index < list.size());
  list[index] = (Expression*)childExpr;
}
BinaryenIndex BinaryenBlockAppendChild(BinaryenExpressionRef expr,
                                       BinaryenExpressionRef childExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  assert(childExpr);
  auto& list = static_cast<Block*>(expression)->list;
  auto index = list.size();
  list.push_back((Expression*)childExpr);
  return index;
}
void BinaryenBlockInsertChildAt(BinaryenExpressionRef expr,
                                BinaryenIndex index,
                                BinaryenExpressionRef childExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  assert(childExpr);
  static_cast<Block*>(expression)->list.insertAt(index, (Expression*)childExpr);
}
BinaryenExpressionRef BinaryenBlockRemoveChildAt(BinaryenExpressionRef expr,
                                                 BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Block>());
  return static_cast<Block*>(expression)->list.removeAt(index);
}
// If
BinaryenExpressionRef BinaryenIfGetCondition(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<If>());
  return static_cast<If*>(expression)->condition;
}
void BinaryenIfSetCondition(BinaryenExpressionRef expr,
                            BinaryenExpressionRef condExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<If>());
  assert(condExpr);
  static_cast<If*>(expression)->condition = (Expression*)condExpr;
}
BinaryenExpressionRef BinaryenIfGetIfTrue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<If>());
  return static_cast<If*>(expression)->ifTrue;
}
void BinaryenIfSetIfTrue(BinaryenExpressionRef expr,
                         BinaryenExpressionRef ifTrueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<If>());
  assert(ifTrueExpr);
  static_cast<If*>(expression)->ifTrue = (Expression*)ifTrueExpr;
}
BinaryenExpressionRef BinaryenIfGetIfFalse(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<If>());
  return static_cast<If*>(expression)->ifFalse;
}
void BinaryenIfSetIfFalse(BinaryenExpressionRef expr,
                          BinaryenExpressionRef ifFalseExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<If>());
  // may be null
  static_cast<If*>(expression)->ifFalse = (Expression*)ifFalseExpr;
}
// Loop
const char* BinaryenLoopGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Loop>());
  return static_cast<Loop*>(expression)->name.str.data();
}
void BinaryenLoopSetName(BinaryenExpressionRef expr, const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Loop>());
  // may be null or empty
  static_cast<Loop*>(expression)->name = name;
}
BinaryenExpressionRef BinaryenLoopGetBody(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Loop>());
  return static_cast<Loop*>(expression)->body;
}
void BinaryenLoopSetBody(BinaryenExpressionRef expr,
                         BinaryenExpressionRef bodyExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Loop>());
  assert(bodyExpr);
  static_cast<Loop*>(expression)->body = (Expression*)bodyExpr;
}
// Break
const char* BinaryenBreakGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Break>());
  return static_cast<Break*>(expression)->name.str.data();
}
void BinaryenBreakSetName(BinaryenExpressionRef expr, const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Break>());
  assert(name);
  static_cast<Break*>(expression)->name = name;
}
BinaryenExpressionRef BinaryenBreakGetCondition(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Break>());
  return static_cast<Break*>(expression)->condition;
}
void BinaryenBreakSetCondition(BinaryenExpressionRef expr,
                               BinaryenExpressionRef condExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Break>());
  // may be null (br)
  static_cast<Break*>(expression)->condition = (Expression*)condExpr;
}
BinaryenExpressionRef BinaryenBreakGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Break>());
  return static_cast<Break*>(expression)->value;
}
void BinaryenBreakSetValue(BinaryenExpressionRef expr,
                           BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Break>());
  // may be null
  static_cast<Break*>(expression)->value = (Expression*)valueExpr;
}
// Switch
BinaryenIndex BinaryenSwitchGetNumNames(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  return static_cast<Switch*>(expression)->targets.size();
}
const char* BinaryenSwitchGetNameAt(BinaryenExpressionRef expr,
                                    BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  assert(index < static_cast<Switch*>(expression)->targets.size());
  return static_cast<Switch*>(expression)->targets[index].str.data();
}
void BinaryenSwitchSetNameAt(BinaryenExpressionRef expr,
                             BinaryenIndex index,
                             const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  assert(index < static_cast<Switch*>(expression)->targets.size());
  assert(name);
  static_cast<Switch*>(expression)->targets[index] = name;
}
BinaryenIndex BinaryenSwitchAppendName(BinaryenExpressionRef expr,
                                       const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  assert(name);
  auto& list = static_cast<Switch*>(expression)->targets;
  auto index = list.size();
  list.push_back(name);
  return index;
}
void BinaryenSwitchInsertNameAt(BinaryenExpressionRef expr,
                                BinaryenIndex index,
                                const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  assert(name);
  static_cast<Switch*>(expression)->targets.insertAt(index, name);
}
const char* BinaryenSwitchRemoveNameAt(BinaryenExpressionRef expr,
                                       BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  return static_cast<Switch*>(expression)->targets.removeAt(index).str.data();
}
const char* BinaryenSwitchGetDefaultName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  return static_cast<Switch*>(expression)->default_.str.data();
}
void BinaryenSwitchSetDefaultName(BinaryenExpressionRef expr,
                                  const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  // may be null or empty
  static_cast<Switch*>(expression)->default_ = name;
}
BinaryenExpressionRef BinaryenSwitchGetCondition(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  return static_cast<Switch*>(expression)->condition;
}
void BinaryenSwitchSetCondition(BinaryenExpressionRef expr,
                                BinaryenExpressionRef condExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  assert(condExpr);
  static_cast<Switch*>(expression)->condition = (Expression*)condExpr;
}
BinaryenExpressionRef BinaryenSwitchGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  return static_cast<Switch*>(expression)->value;
}
void BinaryenSwitchSetValue(BinaryenExpressionRef expr,
                            BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Switch>());
  // may be null
  static_cast<Switch*>(expression)->value = (Expression*)valueExpr;
}
// Call
const char* BinaryenCallGetTarget(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  return static_cast<Call*>(expression)->target.str.data();
}
void BinaryenCallSetTarget(BinaryenExpressionRef expr, const char* target) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  assert(target);
  static_cast<Call*>(expression)->target = target;
}
BinaryenIndex BinaryenCallGetNumOperands(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  return static_cast<Call*>(expression)->operands.size();
}
BinaryenExpressionRef BinaryenCallGetOperandAt(BinaryenExpressionRef expr,
                                               BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  assert(index < static_cast<Call*>(expression)->operands.size());
  return static_cast<Call*>(expression)->operands[index];
}
void BinaryenCallSetOperandAt(BinaryenExpressionRef expr,
                              BinaryenIndex index,
                              BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  assert(index < static_cast<Call*>(expression)->operands.size());
  assert(operandExpr);
  static_cast<Call*>(expression)->operands[index] = (Expression*)operandExpr;
}
BinaryenIndex BinaryenCallAppendOperand(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  assert(operandExpr);
  auto& list = static_cast<Call*>(expression)->operands;
  auto index = list.size();
  list.push_back((Expression*)operandExpr);
  return index;
}
void BinaryenCallInsertOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index,
                                 BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  assert(operandExpr);
  static_cast<Call*>(expression)
    ->operands.insertAt(index, (Expression*)operandExpr);
}
BinaryenExpressionRef BinaryenCallRemoveOperandAt(BinaryenExpressionRef expr,
                                                  BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  return static_cast<Call*>(expression)->operands.removeAt(index);
}
bool BinaryenCallIsReturn(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  return static_cast<Call*>(expression)->isReturn;
}
void BinaryenCallSetReturn(BinaryenExpressionRef expr, bool isReturn) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Call>());
  static_cast<Call*>(expression)->isReturn = isReturn != 0;
}
// CallIndirect
BinaryenExpressionRef
BinaryenCallIndirectGetTarget(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)->target;
}
void BinaryenCallIndirectSetTarget(BinaryenExpressionRef expr,
                                   BinaryenExpressionRef targetExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  assert(targetExpr);
  static_cast<CallIndirect*>(expression)->target = (Expression*)targetExpr;
}
const char* BinaryenCallIndirectGetTable(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)->table.str.data();
}
void BinaryenCallIndirectSetTable(BinaryenExpressionRef expr,
                                  const char* table) {
  Name name(table);
  auto* expression = (Expression*)expr;

  assert(expression->is<CallIndirect>());
  static_cast<CallIndirect*>(expression)->table = name;
}
BinaryenIndex BinaryenCallIndirectGetNumOperands(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)->operands.size();
}
BinaryenExpressionRef
BinaryenCallIndirectGetOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  assert(index < static_cast<CallIndirect*>(expression)->operands.size());
  return static_cast<CallIndirect*>(expression)->operands[index];
}
void BinaryenCallIndirectSetOperandAt(BinaryenExpressionRef expr,
                                      BinaryenIndex index,
                                      BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  assert(index < static_cast<CallIndirect*>(expression)->operands.size());
  assert(operandExpr);
  static_cast<CallIndirect*>(expression)->operands[index] =
    (Expression*)operandExpr;
}
BinaryenIndex
BinaryenCallIndirectAppendOperand(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  assert(operandExpr);
  auto& list = static_cast<CallIndirect*>(expression)->operands;
  auto index = list.size();
  list.push_back((Expression*)operandExpr);
  return index;
}
void BinaryenCallIndirectInsertOperandAt(BinaryenExpressionRef expr,
                                         BinaryenIndex index,
                                         BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  assert(operandExpr);
  static_cast<CallIndirect*>(expression)
    ->operands.insertAt(index, (Expression*)operandExpr);
}
BinaryenExpressionRef
BinaryenCallIndirectRemoveOperandAt(BinaryenExpressionRef expr,
                                    BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)->operands.removeAt(index);
}
bool BinaryenCallIndirectIsReturn(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)->isReturn;
}
void BinaryenCallIndirectSetReturn(BinaryenExpressionRef expr, bool isReturn) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  static_cast<CallIndirect*>(expression)->isReturn = isReturn != 0;
}
BinaryenType BinaryenCallIndirectGetParams(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)
    ->heapType.getSignature()
    .params.getID();
}
void BinaryenCallIndirectSetParams(BinaryenExpressionRef expr,
                                   BinaryenType params) {
  auto* call = ((Expression*)expr)->cast<CallIndirect>();
  call->heapType =
    Signature(Type(params), call->heapType.getSignature().results);
}
BinaryenType BinaryenCallIndirectGetResults(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallIndirect>());
  return static_cast<CallIndirect*>(expression)
    ->heapType.getSignature()
    .results.getID();
}
void BinaryenCallIndirectSetResults(BinaryenExpressionRef expr,
                                    BinaryenType results) {
  auto* call = ((Expression*)expr)->cast<CallIndirect>();
  call->heapType =
    Signature(call->heapType.getSignature().params, Type(results));
}
// LocalGet
BinaryenIndex BinaryenLocalGetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalGet>());
  return static_cast<LocalGet*>(expression)->index;
}
void BinaryenLocalGetSetIndex(BinaryenExpressionRef expr, BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalGet>());
  static_cast<LocalGet*>(expression)->index = index;
}
// LocalSet
bool BinaryenLocalSetIsTee(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalSet>());
  return static_cast<LocalSet*>(expression)->isTee();
  // has no setter
}
BinaryenIndex BinaryenLocalSetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalSet>());
  return static_cast<LocalSet*>(expression)->index;
}
void BinaryenLocalSetSetIndex(BinaryenExpressionRef expr, BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalSet>());
  static_cast<LocalSet*>(expression)->index = index;
}
BinaryenExpressionRef BinaryenLocalSetGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalSet>());
  return static_cast<LocalSet*>(expression)->value;
}
void BinaryenLocalSetSetValue(BinaryenExpressionRef expr,
                              BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<LocalSet>());
  assert(valueExpr);
  static_cast<LocalSet*>(expression)->value = (Expression*)valueExpr;
}
// GlobalGet
const char* BinaryenGlobalGetGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<GlobalGet>());
  return static_cast<GlobalGet*>(expression)->name.str.data();
}
void BinaryenGlobalGetSetName(BinaryenExpressionRef expr, const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<GlobalGet>());
  assert(name);
  static_cast<GlobalGet*>(expression)->name = name;
}
// GlobalSet
const char* BinaryenGlobalSetGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<GlobalSet>());
  return static_cast<GlobalSet*>(expression)->name.str.data();
}
void BinaryenGlobalSetSetName(BinaryenExpressionRef expr, const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<GlobalSet>());
  assert(name);
  static_cast<GlobalSet*>(expression)->name = name;
}
BinaryenExpressionRef BinaryenGlobalSetGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<GlobalSet>());
  return static_cast<GlobalSet*>(expression)->value;
}
void BinaryenGlobalSetSetValue(BinaryenExpressionRef expr,
                               BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<GlobalSet>());
  assert(valueExpr);
  static_cast<GlobalSet*>(expression)->value = (Expression*)valueExpr;
}
// TableGet
const char* BinaryenTableGetGetTable(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGet>());
  return static_cast<TableGet*>(expression)->table.str.data();
}
void BinaryenTableGetSetTable(BinaryenExpressionRef expr, const char* table) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGet>());
  assert(table);
  static_cast<TableGet*>(expression)->table = table;
}
BinaryenExpressionRef BinaryenTableGetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGet>());
  return static_cast<TableGet*>(expression)->index;
}
void BinaryenTableGetSetIndex(BinaryenExpressionRef expr,
                              BinaryenExpressionRef indexExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGet>());
  assert(indexExpr);
  static_cast<TableGet*>(expression)->index = (Expression*)indexExpr;
}
// TableSet
const char* BinaryenTableSetGetTable(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSet>());
  return static_cast<TableSet*>(expression)->table.str.data();
}
void BinaryenTableSetSetTable(BinaryenExpressionRef expr, const char* table) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSet>());
  assert(table);
  static_cast<TableSet*>(expression)->table = table;
}
BinaryenExpressionRef BinaryenTableSetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSet>());
  return static_cast<TableSet*>(expression)->index;
}
void BinaryenTableSetSetIndex(BinaryenExpressionRef expr,
                              BinaryenExpressionRef indexExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSet>());
  assert(indexExpr);
  static_cast<TableSet*>(expression)->index = (Expression*)indexExpr;
}
BinaryenExpressionRef BinaryenTableSetGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSet>());
  return static_cast<TableSet*>(expression)->value;
}
void BinaryenTableSetSetValue(BinaryenExpressionRef expr,
                              BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSet>());
  assert(valueExpr);
  static_cast<TableSet*>(expression)->value = (Expression*)valueExpr;
}
// TableSize
const char* BinaryenTableSizeGetTable(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSize>());
  return static_cast<TableSize*>(expression)->table.str.data();
}
void BinaryenTableSizeSetTable(BinaryenExpressionRef expr, const char* table) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableSize>());
  assert(table);
  static_cast<TableSize*>(expression)->table = table;
}
// TableGrow
const char* BinaryenTableGrowGetTable(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGrow>());
  return static_cast<TableGrow*>(expression)->table.str.data();
}
void BinaryenTableGrowSetTable(BinaryenExpressionRef expr, const char* table) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGrow>());
  assert(table);
  static_cast<TableGrow*>(expression)->table = table;
}
BinaryenExpressionRef BinaryenTableGrowGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGrow>());
  return static_cast<TableGrow*>(expression)->value;
}
void BinaryenTableGrowSetValue(BinaryenExpressionRef expr,
                               BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGrow>());
  assert(valueExpr);
  static_cast<TableGrow*>(expression)->value = (Expression*)valueExpr;
}
BinaryenExpressionRef BinaryenTableGrowGetDelta(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGrow>());
  return static_cast<TableGrow*>(expression)->delta;
}
void BinaryenTableGrowSetDelta(BinaryenExpressionRef expr,
                               BinaryenExpressionRef deltaExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TableGrow>());
  assert(deltaExpr);
  static_cast<TableGrow*>(expression)->delta = (Expression*)deltaExpr;
}
// MemoryGrow
BinaryenExpressionRef BinaryenMemoryGrowGetDelta(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryGrow>());
  return static_cast<MemoryGrow*>(expression)->delta;
}
void BinaryenMemoryGrowSetDelta(BinaryenExpressionRef expr,
                                BinaryenExpressionRef deltaExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryGrow>());
  assert(deltaExpr);
  static_cast<MemoryGrow*>(expression)->delta = (Expression*)deltaExpr;
}
// Load
bool BinaryenLoadIsAtomic(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  return static_cast<Load*>(expression)->isAtomic;
}
void BinaryenLoadSetAtomic(BinaryenExpressionRef expr, bool isAtomic) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  static_cast<Load*>(expression)->isAtomic = isAtomic != 0;
}
bool BinaryenLoadIsSigned(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  return static_cast<Load*>(expression)->signed_;
}
void BinaryenLoadSetSigned(BinaryenExpressionRef expr, bool isSigned) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  static_cast<Load*>(expression)->signed_ = isSigned != 0;
}
uint32_t BinaryenLoadGetBytes(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  return static_cast<Load*>(expression)->bytes;
}
void BinaryenLoadSetBytes(BinaryenExpressionRef expr, uint32_t bytes) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  static_cast<Load*>(expression)->bytes = bytes;
}
uint32_t BinaryenLoadGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  return static_cast<Load*>(expression)->offset;
}
void BinaryenLoadSetOffset(BinaryenExpressionRef expr, uint32_t offset) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  static_cast<Load*>(expression)->offset = offset;
}
uint32_t BinaryenLoadGetAlign(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  return static_cast<Load*>(expression)->align;
}
void BinaryenLoadSetAlign(BinaryenExpressionRef expr, uint32_t align) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  static_cast<Load*>(expression)->align = align;
}
BinaryenExpressionRef BinaryenLoadGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  return static_cast<Load*>(expression)->ptr;
}
void BinaryenLoadSetPtr(BinaryenExpressionRef expr,
                        BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Load>());
  assert(ptrExpr);
  static_cast<Load*>(expression)->ptr = (Expression*)ptrExpr;
}
// Store
bool BinaryenStoreIsAtomic(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->isAtomic;
}
void BinaryenStoreSetAtomic(BinaryenExpressionRef expr, bool isAtomic) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  static_cast<Store*>(expression)->isAtomic = isAtomic != 0;
}
uint32_t BinaryenStoreGetBytes(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->bytes;
}
void BinaryenStoreSetBytes(BinaryenExpressionRef expr, uint32_t bytes) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  static_cast<Store*>(expression)->bytes = bytes;
}
uint32_t BinaryenStoreGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->offset;
}
void BinaryenStoreSetOffset(BinaryenExpressionRef expr, uint32_t offset) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  static_cast<Store*>(expression)->offset = offset;
}
uint32_t BinaryenStoreGetAlign(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->align;
}
void BinaryenStoreSetAlign(BinaryenExpressionRef expr, uint32_t align) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  static_cast<Store*>(expression)->align = align;
}
BinaryenExpressionRef BinaryenStoreGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->ptr;
}
void BinaryenStoreSetPtr(BinaryenExpressionRef expr,
                         BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  assert(ptrExpr);
  static_cast<Store*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef BinaryenStoreGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->value;
}
void BinaryenStoreSetValue(BinaryenExpressionRef expr,
                           BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  assert(valueExpr);
  static_cast<Store*>(expression)->value = (Expression*)valueExpr;
}
BinaryenType BinaryenStoreGetValueType(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  return static_cast<Store*>(expression)->valueType.getID();
}
void BinaryenStoreSetValueType(BinaryenExpressionRef expr,
                               BinaryenType valueType) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Store>());
  static_cast<Store*>(expression)->valueType = Type(valueType);
}
// Const
int32_t BinaryenConstGetValueI32(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  return static_cast<Const*>(expression)->value.geti32();
}
void BinaryenConstSetValueI32(BinaryenExpressionRef expr, int32_t value) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  static_cast<Const*>(expression)->value = Literal(value);
}
int64_t BinaryenConstGetValueI64(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  return static_cast<Const*>(expression)->value.geti64();
}
void BinaryenConstSetValueI64(BinaryenExpressionRef expr, int64_t value) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  static_cast<Const*>(expression)->value = Literal(value);
}
int32_t BinaryenConstGetValueI64Low(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  return (int32_t)(static_cast<Const*>(expression)->value.geti64() &
                   0xffffffff);
}
void BinaryenConstSetValueI64Low(BinaryenExpressionRef expr, int32_t valueLow) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  auto& value = static_cast<Const*>(expression)->value;
  int64_t valueI64 = value.type == Type::i64 ? value.geti64() : 0;
  static_cast<Const*>(expression)->value =
    Literal((valueI64 & ~0xffffffff) | (int64_t(valueLow) & 0xffffffff));
}
int32_t BinaryenConstGetValueI64High(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  return (int32_t)(static_cast<Const*>(expression)->value.geti64() >> 32);
}
void BinaryenConstSetValueI64High(BinaryenExpressionRef expr,
                                  int32_t valueHigh) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  auto& value = static_cast<Const*>(expression)->value;
  int64_t valueI64 = value.type == Type::i64 ? value.geti64() : 0;
  static_cast<Const*>(expression)->value =
    Literal((int64_t(valueHigh) << 32) | (valueI64 & 0xffffffff));
}
float BinaryenConstGetValueF32(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  return static_cast<Const*>(expression)->value.getf32();
}
void BinaryenConstSetValueF32(BinaryenExpressionRef expr, float value) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  static_cast<Const*>(expression)->value = Literal(value);
}
double BinaryenConstGetValueF64(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  return static_cast<Const*>(expression)->value.getf64();
}
void BinaryenConstSetValueF64(BinaryenExpressionRef expr, double value) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  static_cast<Const*>(expression)->value = Literal(value);
}
void BinaryenConstGetValueV128(BinaryenExpressionRef expr, uint8_t* out) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  memcpy(out, static_cast<Const*>(expression)->value.getv128().data(), 16);
}
void BinaryenConstSetValueV128(BinaryenExpressionRef expr,
                               const uint8_t value[16]) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Const>());
  assert(value); // nullptr would be wrong
  static_cast<Const*>(expression)->value = Literal(value);
}
// Unary
BinaryenOp BinaryenUnaryGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Unary>());
  return static_cast<Unary*>(expression)->op;
}
void BinaryenUnarySetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Unary>());
  static_cast<Unary*>(expression)->op = UnaryOp(op);
}
BinaryenExpressionRef BinaryenUnaryGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Unary>());
  return static_cast<Unary*>(expression)->value;
}
void BinaryenUnarySetValue(BinaryenExpressionRef expr,
                           BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Unary>());
  assert(valueExpr);
  static_cast<Unary*>(expression)->value = (Expression*)valueExpr;
}
// Binary
BinaryenOp BinaryenBinaryGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Binary>());
  return static_cast<Binary*>(expression)->op;
}
void BinaryenBinarySetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Binary>());
  static_cast<Binary*>(expression)->op = BinaryOp(op);
}
BinaryenExpressionRef BinaryenBinaryGetLeft(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Binary>());
  return static_cast<Binary*>(expression)->left;
}
void BinaryenBinarySetLeft(BinaryenExpressionRef expr,
                           BinaryenExpressionRef leftExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Binary>());
  assert(leftExpr);
  static_cast<Binary*>(expression)->left = (Expression*)leftExpr;
}
BinaryenExpressionRef BinaryenBinaryGetRight(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Binary>());
  return static_cast<Binary*>(expression)->right;
}
void BinaryenBinarySetRight(BinaryenExpressionRef expr,
                            BinaryenExpressionRef rightExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Binary>());
  assert(rightExpr);
  static_cast<Binary*>(expression)->right = (Expression*)rightExpr;
}
// Select
BinaryenExpressionRef BinaryenSelectGetIfTrue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Select>());
  return static_cast<Select*>(expression)->ifTrue;
}
void BinaryenSelectSetIfTrue(BinaryenExpressionRef expr,
                             BinaryenExpressionRef ifTrueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Select>());
  assert(ifTrueExpr);
  static_cast<Select*>(expression)->ifTrue = (Expression*)ifTrueExpr;
}
BinaryenExpressionRef BinaryenSelectGetIfFalse(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Select>());
  return static_cast<Select*>(expression)->ifFalse;
}
void BinaryenSelectSetIfFalse(BinaryenExpressionRef expr,
                              BinaryenExpressionRef ifFalseExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Select>());
  assert(ifFalseExpr);
  static_cast<Select*>(expression)->ifFalse = (Expression*)ifFalseExpr;
}
BinaryenExpressionRef BinaryenSelectGetCondition(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Select>());
  return static_cast<Select*>(expression)->condition;
}
void BinaryenSelectSetCondition(BinaryenExpressionRef expr,
                                BinaryenExpressionRef condExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Select>());
  assert(condExpr);
  static_cast<Select*>(expression)->condition = (Expression*)condExpr;
}
// Drop
BinaryenExpressionRef BinaryenDropGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Drop>());
  return static_cast<Drop*>(expression)->value;
}
void BinaryenDropSetValue(BinaryenExpressionRef expr,
                          BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Drop>());
  assert(valueExpr);
  static_cast<Drop*>(expression)->value = (Expression*)valueExpr;
}
// Return
BinaryenExpressionRef BinaryenReturnGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Return>());
  return static_cast<Return*>(expression)->value;
}
void BinaryenReturnSetValue(BinaryenExpressionRef expr,
                            BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Return>());
  // may be null
  static_cast<Return*>(expression)->value = (Expression*)valueExpr;
}
// AtomicRMW
BinaryenOp BinaryenAtomicRMWGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  return static_cast<AtomicRMW*>(expression)->op;
}
void BinaryenAtomicRMWSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  static_cast<AtomicRMW*>(expression)->op = AtomicRMWOp(op);
}
uint32_t BinaryenAtomicRMWGetBytes(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  return static_cast<AtomicRMW*>(expression)->bytes;
}
void BinaryenAtomicRMWSetBytes(BinaryenExpressionRef expr, uint32_t bytes) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  static_cast<AtomicRMW*>(expression)->bytes = bytes;
}
uint32_t BinaryenAtomicRMWGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  return static_cast<AtomicRMW*>(expression)->offset;
}
void BinaryenAtomicRMWSetOffset(BinaryenExpressionRef expr, uint32_t offset) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  static_cast<AtomicRMW*>(expression)->offset = offset;
}
BinaryenExpressionRef BinaryenAtomicRMWGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  return static_cast<AtomicRMW*>(expression)->ptr;
}
void BinaryenAtomicRMWSetPtr(BinaryenExpressionRef expr,
                             BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  assert(ptrExpr);
  static_cast<AtomicRMW*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef BinaryenAtomicRMWGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  return static_cast<AtomicRMW*>(expression)->value;
}
void BinaryenAtomicRMWSetValue(BinaryenExpressionRef expr,
                               BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicRMW>());
  assert(valueExpr);
  static_cast<AtomicRMW*>(expression)->value = (Expression*)valueExpr;
}
// AtomicCmpxchg
uint32_t BinaryenAtomicCmpxchgGetBytes(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  return static_cast<AtomicCmpxchg*>(expression)->bytes;
}
void BinaryenAtomicCmpxchgSetBytes(BinaryenExpressionRef expr, uint32_t bytes) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  static_cast<AtomicCmpxchg*>(expression)->bytes = bytes;
}
uint32_t BinaryenAtomicCmpxchgGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  return static_cast<AtomicCmpxchg*>(expression)->offset;
}
void BinaryenAtomicCmpxchgSetOffset(BinaryenExpressionRef expr,
                                    uint32_t offset) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  static_cast<AtomicCmpxchg*>(expression)->offset = offset;
}
BinaryenExpressionRef BinaryenAtomicCmpxchgGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  return static_cast<AtomicCmpxchg*>(expression)->ptr;
}
void BinaryenAtomicCmpxchgSetPtr(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  assert(ptrExpr);
  static_cast<AtomicCmpxchg*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef
BinaryenAtomicCmpxchgGetExpected(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  return static_cast<AtomicCmpxchg*>(expression)->expected;
}
void BinaryenAtomicCmpxchgSetExpected(BinaryenExpressionRef expr,
                                      BinaryenExpressionRef expectedExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  assert(expectedExpr);
  static_cast<AtomicCmpxchg*>(expression)->expected = (Expression*)expectedExpr;
}
BinaryenExpressionRef
BinaryenAtomicCmpxchgGetReplacement(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  return static_cast<AtomicCmpxchg*>(expression)->replacement;
}
void BinaryenAtomicCmpxchgSetReplacement(
  BinaryenExpressionRef expr, BinaryenExpressionRef replacementExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicCmpxchg>());
  assert(replacementExpr);
  static_cast<AtomicCmpxchg*>(expression)->replacement =
    (Expression*)replacementExpr;
}
// AtomicWait
BinaryenExpressionRef BinaryenAtomicWaitGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  return static_cast<AtomicWait*>(expression)->ptr;
}
void BinaryenAtomicWaitSetPtr(BinaryenExpressionRef expr,
                              BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  assert(ptrExpr);
  static_cast<AtomicWait*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef
BinaryenAtomicWaitGetExpected(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  return static_cast<AtomicWait*>(expression)->expected;
}
void BinaryenAtomicWaitSetExpected(BinaryenExpressionRef expr,
                                   BinaryenExpressionRef expectedExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  assert(expectedExpr);
  static_cast<AtomicWait*>(expression)->expected = (Expression*)expectedExpr;
}
BinaryenExpressionRef BinaryenAtomicWaitGetTimeout(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  return static_cast<AtomicWait*>(expression)->timeout;
}
void BinaryenAtomicWaitSetTimeout(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef timeoutExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  assert(timeoutExpr);
  static_cast<AtomicWait*>(expression)->timeout = (Expression*)timeoutExpr;
}
BinaryenType BinaryenAtomicWaitGetExpectedType(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  return static_cast<AtomicWait*>(expression)->expectedType.getID();
}
void BinaryenAtomicWaitSetExpectedType(BinaryenExpressionRef expr,
                                       BinaryenType expectedType) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicWait>());
  static_cast<AtomicWait*>(expression)->expectedType = Type(expectedType);
}
// AtomicNotify
BinaryenExpressionRef BinaryenAtomicNotifyGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicNotify>());
  return static_cast<AtomicNotify*>(expression)->ptr;
}
void BinaryenAtomicNotifySetPtr(BinaryenExpressionRef expr,
                                BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicNotify>());
  assert(ptrExpr);
  static_cast<AtomicNotify*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef
BinaryenAtomicNotifyGetNotifyCount(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicNotify>());
  return static_cast<AtomicNotify*>(expression)->notifyCount;
}
void BinaryenAtomicNotifySetNotifyCount(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef notifyCountExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicNotify>());
  assert(notifyCountExpr);
  static_cast<AtomicNotify*>(expression)->notifyCount =
    (Expression*)notifyCountExpr;
}
// AtomicFence
uint8_t BinaryenAtomicFenceGetOrder(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicFence>());
  return static_cast<AtomicFence*>(expression)->order;
}
void BinaryenAtomicFenceSetOrder(BinaryenExpressionRef expr, uint8_t order) {
  auto* expression = (Expression*)expr;
  assert(expression->is<AtomicFence>());
  static_cast<AtomicFence*>(expression)->order = order;
}
// SIMDExtract
BinaryenOp BinaryenSIMDExtractGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDExtract>());
  return static_cast<SIMDExtract*>(expression)->op;
}
void BinaryenSIMDExtractSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDExtract>());
  static_cast<SIMDExtract*>(expression)->op = SIMDExtractOp(op);
}
BinaryenExpressionRef BinaryenSIMDExtractGetVec(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDExtract>());
  return static_cast<SIMDExtract*>(expression)->vec;
}
void BinaryenSIMDExtractSetVec(BinaryenExpressionRef expr,
                               BinaryenExpressionRef vecExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDExtract>());
  assert(vecExpr);
  static_cast<SIMDExtract*>(expression)->vec = (Expression*)vecExpr;
}
uint8_t BinaryenSIMDExtractGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDExtract>());
  return static_cast<SIMDExtract*>(expression)->index;
}
void BinaryenSIMDExtractSetIndex(BinaryenExpressionRef expr, uint8_t index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDExtract>());
  static_cast<SIMDExtract*>(expression)->index = index;
}
// SIMDReplace
BinaryenOp BinaryenSIMDReplaceGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  return static_cast<SIMDReplace*>(expression)->op;
}
void BinaryenSIMDReplaceSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  static_cast<SIMDReplace*>(expression)->op = SIMDReplaceOp(op);
}
BinaryenExpressionRef BinaryenSIMDReplaceGetVec(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  return static_cast<SIMDReplace*>(expression)->vec;
}
void BinaryenSIMDReplaceSetVec(BinaryenExpressionRef expr,
                               BinaryenExpressionRef vecExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  assert(vecExpr);
  static_cast<SIMDReplace*>(expression)->vec = (Expression*)vecExpr;
}
uint8_t BinaryenSIMDReplaceGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  return static_cast<SIMDReplace*>(expression)->index;
}
void BinaryenSIMDReplaceSetIndex(BinaryenExpressionRef expr, uint8_t index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  static_cast<SIMDReplace*>(expression)->index = index;
}
BinaryenExpressionRef BinaryenSIMDReplaceGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  return static_cast<SIMDReplace*>(expression)->value;
}
void BinaryenSIMDReplaceSetValue(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDReplace>());
  assert(valueExpr);
  static_cast<SIMDReplace*>(expression)->value = (Expression*)valueExpr;
}
// SIMDShuffle
BinaryenExpressionRef BinaryenSIMDShuffleGetLeft(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShuffle>());
  return static_cast<SIMDShuffle*>(expression)->left;
}
void BinaryenSIMDShuffleSetLeft(BinaryenExpressionRef expr,
                                BinaryenExpressionRef leftExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShuffle>());
  assert(leftExpr);
  static_cast<SIMDShuffle*>(expression)->left = (Expression*)leftExpr;
}
BinaryenExpressionRef BinaryenSIMDShuffleGetRight(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShuffle>());
  return static_cast<SIMDShuffle*>(expression)->right;
}
void BinaryenSIMDShuffleSetRight(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef rightExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShuffle>());
  assert(rightExpr);
  static_cast<SIMDShuffle*>(expression)->right = (Expression*)rightExpr;
}
void BinaryenSIMDShuffleGetMask(BinaryenExpressionRef expr, uint8_t* mask) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShuffle>());
  assert(mask); // nullptr would be wrong
  memcpy(mask, static_cast<SIMDShuffle*>(expression)->mask.data(), 16);
}
void BinaryenSIMDShuffleSetMask(BinaryenExpressionRef expr,
                                const uint8_t mask_[16]) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShuffle>());
  assert(mask_); // nullptr would be wrong
  auto& mask = static_cast<SIMDShuffle*>(expression)->mask;
  memcpy(mask.data(), mask_, 16);
}
// SIMDTernary
BinaryenOp BinaryenSIMDTernaryGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  return static_cast<SIMDTernary*>(expression)->op;
}
void BinaryenSIMDTernarySetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  static_cast<SIMDTernary*>(expression)->op = SIMDTernaryOp(op);
}
BinaryenExpressionRef BinaryenSIMDTernaryGetA(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  return static_cast<SIMDTernary*>(expression)->a;
}
void BinaryenSIMDTernarySetA(BinaryenExpressionRef expr,
                             BinaryenExpressionRef aExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  assert(aExpr);
  static_cast<SIMDTernary*>(expression)->a = (Expression*)aExpr;
}
BinaryenExpressionRef BinaryenSIMDTernaryGetB(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  return static_cast<SIMDTernary*>(expression)->b;
}
void BinaryenSIMDTernarySetB(BinaryenExpressionRef expr,
                             BinaryenExpressionRef bExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  assert(bExpr);
  static_cast<SIMDTernary*>(expression)->b = (Expression*)bExpr;
}
BinaryenExpressionRef BinaryenSIMDTernaryGetC(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  return static_cast<SIMDTernary*>(expression)->c;
}
void BinaryenSIMDTernarySetC(BinaryenExpressionRef expr,
                             BinaryenExpressionRef cExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDTernary>());
  assert(cExpr);
  static_cast<SIMDTernary*>(expression)->c = (Expression*)cExpr;
}
// SIMDShift
BinaryenOp BinaryenSIMDShiftGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShift>());
  return static_cast<SIMDShift*>(expression)->op;
}
void BinaryenSIMDShiftSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShift>());
  static_cast<SIMDShift*>(expression)->op = SIMDShiftOp(op);
}
BinaryenExpressionRef BinaryenSIMDShiftGetVec(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShift>());
  return static_cast<SIMDShift*>(expression)->vec;
}
void BinaryenSIMDShiftSetVec(BinaryenExpressionRef expr,
                             BinaryenExpressionRef vecExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShift>());
  assert(vecExpr);
  static_cast<SIMDShift*>(expression)->vec = (Expression*)vecExpr;
}
BinaryenExpressionRef BinaryenSIMDShiftGetShift(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShift>());
  return static_cast<SIMDShift*>(expression)->shift;
}
void BinaryenSIMDShiftSetShift(BinaryenExpressionRef expr,
                               BinaryenExpressionRef shiftExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDShift>());
  assert(shiftExpr);
  static_cast<SIMDShift*>(expression)->shift = (Expression*)shiftExpr;
}
// SIMDLoad
BinaryenOp BinaryenSIMDLoadGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  return static_cast<SIMDLoad*>(expression)->op;
}
void BinaryenSIMDLoadSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  static_cast<SIMDLoad*>(expression)->op = SIMDLoadOp(op);
}
uint32_t BinaryenSIMDLoadGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  return static_cast<SIMDLoad*>(expression)->offset;
}
void BinaryenSIMDLoadSetOffset(BinaryenExpressionRef expr, uint32_t offset) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  static_cast<SIMDLoad*>(expression)->offset = offset;
}
uint32_t BinaryenSIMDLoadGetAlign(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  return static_cast<SIMDLoad*>(expression)->align;
}
void BinaryenSIMDLoadSetAlign(BinaryenExpressionRef expr, uint32_t align) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  static_cast<SIMDLoad*>(expression)->align = align;
}
BinaryenExpressionRef BinaryenSIMDLoadGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  return static_cast<SIMDLoad*>(expression)->ptr;
}
void BinaryenSIMDLoadSetPtr(BinaryenExpressionRef expr,
                            BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoad>());
  assert(ptrExpr);
  static_cast<SIMDLoad*>(expression)->ptr = (Expression*)ptrExpr;
}
// SIMDLoadStoreLane
BinaryenOp BinaryenSIMDLoadStoreLaneGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->op;
}
void BinaryenSIMDLoadStoreLaneSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  static_cast<SIMDLoadStoreLane*>(expression)->op = SIMDLoadStoreLaneOp(op);
}
uint32_t BinaryenSIMDLoadStoreLaneGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->offset;
}
void BinaryenSIMDLoadStoreLaneSetOffset(BinaryenExpressionRef expr,
                                        uint32_t offset) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  static_cast<SIMDLoadStoreLane*>(expression)->offset = offset;
}
uint32_t BinaryenSIMDLoadStoreLaneGetAlign(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->align;
}
void BinaryenSIMDLoadStoreLaneSetAlign(BinaryenExpressionRef expr,
                                       uint32_t align) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  static_cast<SIMDLoadStoreLane*>(expression)->align = align;
}
uint8_t BinaryenSIMDLoadStoreLaneGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->index;
}
void BinaryenSIMDLoadStoreLaneSetIndex(BinaryenExpressionRef expr,
                                       uint8_t index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  static_cast<SIMDLoadStoreLane*>(expression)->index = index;
}
BinaryenExpressionRef
BinaryenSIMDLoadStoreLaneGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->ptr;
}
void BinaryenSIMDLoadStoreLaneSetPtr(BinaryenExpressionRef expr,
                                     BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  assert(ptrExpr);
  static_cast<SIMDLoadStoreLane*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef
BinaryenSIMDLoadStoreLaneGetVec(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->vec;
}
void BinaryenSIMDLoadStoreLaneSetVec(BinaryenExpressionRef expr,
                                     BinaryenExpressionRef vecExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  assert(vecExpr);
  static_cast<SIMDLoadStoreLane*>(expression)->vec = (Expression*)vecExpr;
}
bool BinaryenSIMDLoadStoreLaneIsStore(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<SIMDLoadStoreLane>());
  return static_cast<SIMDLoadStoreLane*>(expression)->isStore();
}
// MemoryInit
const char* BinaryenMemoryInitGetSegment(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  return static_cast<MemoryInit*>(expression)->segment.str.data();
}
void BinaryenMemoryInitSetSegment(BinaryenExpressionRef expr,
                                  const char* segment) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  static_cast<MemoryInit*>(expression)->segment = Name(segment);
}
BinaryenExpressionRef BinaryenMemoryInitGetDest(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  return static_cast<MemoryInit*>(expression)->dest;
}
void BinaryenMemoryInitSetDest(BinaryenExpressionRef expr,
                               BinaryenExpressionRef destExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  assert(destExpr);
  static_cast<MemoryInit*>(expression)->dest = (Expression*)destExpr;
}
BinaryenExpressionRef BinaryenMemoryInitGetOffset(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  return static_cast<MemoryInit*>(expression)->offset;
}
void BinaryenMemoryInitSetOffset(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef offsetExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  assert(offsetExpr);
  static_cast<MemoryInit*>(expression)->offset = (Expression*)offsetExpr;
}
BinaryenExpressionRef BinaryenMemoryInitGetSize(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  return static_cast<MemoryInit*>(expression)->size;
}
void BinaryenMemoryInitSetSize(BinaryenExpressionRef expr,
                               BinaryenExpressionRef sizeExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryInit>());
  assert(sizeExpr);
  static_cast<MemoryInit*>(expression)->size = (Expression*)sizeExpr;
}
// DataDrop
const char* BinaryenDataDropGetSegment(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<DataDrop>());
  return static_cast<DataDrop*>(expression)->segment.str.data();
}
void BinaryenDataDropSetSegment(BinaryenExpressionRef expr,
                                const char* segment) {
  auto* expression = (Expression*)expr;
  assert(expression->is<DataDrop>());
  static_cast<DataDrop*>(expression)->segment = Name(segment);
}
// MemoryCopy
BinaryenExpressionRef BinaryenMemoryCopyGetDest(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryCopy>());
  return static_cast<MemoryCopy*>(expression)->dest;
}
void BinaryenMemoryCopySetDest(BinaryenExpressionRef expr,
                               BinaryenExpressionRef destExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryCopy>());
  assert(destExpr);
  static_cast<MemoryCopy*>(expression)->dest = (Expression*)destExpr;
}
BinaryenExpressionRef BinaryenMemoryCopyGetSource(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryCopy>());
  return static_cast<MemoryCopy*>(expression)->source;
}
void BinaryenMemoryCopySetSource(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef sourceExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryCopy>());
  assert(sourceExpr);
  static_cast<MemoryCopy*>(expression)->source = (Expression*)sourceExpr;
}
BinaryenExpressionRef BinaryenMemoryCopyGetSize(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryCopy>());
  return static_cast<MemoryCopy*>(expression)->size;
}
void BinaryenMemoryCopySetSize(BinaryenExpressionRef expr,
                               BinaryenExpressionRef sizeExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryCopy>());
  assert(sizeExpr);
  static_cast<MemoryCopy*>(expression)->size = (Expression*)sizeExpr;
}
// MemoryFill
BinaryenExpressionRef BinaryenMemoryFillGetDest(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryFill>());
  return static_cast<MemoryFill*>(expression)->dest;
}
void BinaryenMemoryFillSetDest(BinaryenExpressionRef expr,
                               BinaryenExpressionRef destExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryFill>());
  assert(destExpr);
  static_cast<MemoryFill*>(expression)->dest = (Expression*)destExpr;
}
BinaryenExpressionRef BinaryenMemoryFillGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryFill>());
  return static_cast<MemoryFill*>(expression)->value;
}
void BinaryenMemoryFillSetValue(BinaryenExpressionRef expr,
                                BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryFill>());
  assert(valueExpr);
  static_cast<MemoryFill*>(expression)->value = (Expression*)valueExpr;
}
BinaryenExpressionRef BinaryenMemoryFillGetSize(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryFill>());
  return static_cast<MemoryFill*>(expression)->size;
}
void BinaryenMemoryFillSetSize(BinaryenExpressionRef expr,
                               BinaryenExpressionRef sizeExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<MemoryFill>());
  assert(sizeExpr);
  static_cast<MemoryFill*>(expression)->size = (Expression*)sizeExpr;
}
// RefIsNull
BinaryenExpressionRef BinaryenRefIsNullGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefIsNull>());
  return static_cast<RefIsNull*>(expression)->value;
}
void BinaryenRefIsNullSetValue(BinaryenExpressionRef expr,
                               BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefIsNull>());
  assert(valueExpr);
  static_cast<RefIsNull*>(expression)->value = (Expression*)valueExpr;
}
// RefAs
BinaryenOp BinaryenRefAsGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefAs>());
  return static_cast<RefAs*>(expression)->op;
}
void BinaryenRefAsSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefAs>());
  static_cast<RefAs*>(expression)->op = RefAsOp(op);
}
BinaryenExpressionRef BinaryenRefAsGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefAs>());
  return static_cast<RefAs*>(expression)->value;
}
void BinaryenRefAsSetValue(BinaryenExpressionRef expr,
                           BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefAs>());
  assert(valueExpr);
  static_cast<RefAs*>(expression)->value = (Expression*)valueExpr;
}
// RefFunc
const char* BinaryenRefFuncGetFunc(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefFunc>());
  return static_cast<RefFunc*>(expression)->func.str.data();
}
void BinaryenRefFuncSetFunc(BinaryenExpressionRef expr, const char* funcName) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefFunc>());
  static_cast<RefFunc*>(expression)->func = funcName;
}
// RefEq
BinaryenExpressionRef BinaryenRefEqGetLeft(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefEq>());
  return static_cast<RefEq*>(expression)->left;
}
void BinaryenRefEqSetLeft(BinaryenExpressionRef expr,
                          BinaryenExpressionRef left) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefEq>());
  static_cast<RefEq*>(expression)->left = (Expression*)left;
}
BinaryenExpressionRef BinaryenRefEqGetRight(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefEq>());
  return static_cast<RefEq*>(expression)->right;
}
void BinaryenRefEqSetRight(BinaryenExpressionRef expr,
                           BinaryenExpressionRef right) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefEq>());
  static_cast<RefEq*>(expression)->right = (Expression*)right;
}
// Try
const char* BinaryenTryGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->name.str.data();
}
void BinaryenTrySetName(BinaryenExpressionRef expr, const char* name) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  static_cast<Try*>(expression)->name = name;
}
BinaryenExpressionRef BinaryenTryGetBody(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->body;
}
void BinaryenTrySetBody(BinaryenExpressionRef expr,
                        BinaryenExpressionRef bodyExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(bodyExpr);
  static_cast<Try*>(expression)->body = (Expression*)bodyExpr;
}
BinaryenIndex BinaryenTryGetNumCatchTags(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->catchTags.size();
}
BinaryenIndex BinaryenTryGetNumCatchBodies(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->catchBodies.size();
}
const char* BinaryenTryGetCatchTagAt(BinaryenExpressionRef expr,
                                     BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(index < static_cast<Try*>(expression)->catchTags.size());
  return static_cast<Try*>(expression)->catchTags[index].str.data();
}
void BinaryenTrySetCatchTagAt(BinaryenExpressionRef expr,
                              BinaryenIndex index,
                              const char* catchTag) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(index < static_cast<Try*>(expression)->catchTags.size());
  assert(catchTag);
  static_cast<Try*>(expression)->catchTags[index] = catchTag;
}
BinaryenIndex BinaryenTryAppendCatchTag(BinaryenExpressionRef expr,
                                        const char* catchTag) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(catchTag);
  auto& list = static_cast<Try*>(expression)->catchTags;
  auto index = list.size();
  list.push_back(catchTag);
  return index;
}
void BinaryenTryInsertCatchTagAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index,
                                 const char* catchTag) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(catchTag);
  static_cast<Try*>(expression)->catchTags.insertAt(index, catchTag);
}
const char* BinaryenTryRemoveCatchTagAt(BinaryenExpressionRef expr,
                                        BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->catchTags.removeAt(index).str.data();
}
BinaryenExpressionRef BinaryenTryGetCatchBodyAt(BinaryenExpressionRef expr,
                                                BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(index < static_cast<Try*>(expression)->catchBodies.size());
  return static_cast<Try*>(expression)->catchBodies[index];
}
void BinaryenTrySetCatchBodyAt(BinaryenExpressionRef expr,
                               BinaryenIndex index,
                               BinaryenExpressionRef catchExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(index < static_cast<Try*>(expression)->catchBodies.size());
  assert(catchExpr);
  static_cast<Try*>(expression)->catchBodies[index] = (Expression*)catchExpr;
}
BinaryenIndex BinaryenTryAppendCatchBody(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef catchExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(catchExpr);
  auto& list = static_cast<Try*>(expression)->catchBodies;
  auto index = list.size();
  list.push_back((Expression*)catchExpr);
  return index;
}
void BinaryenTryInsertCatchBodyAt(BinaryenExpressionRef expr,
                                  BinaryenIndex index,
                                  BinaryenExpressionRef catchExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  assert(catchExpr);
  static_cast<Try*>(expression)
    ->catchBodies.insertAt(index, (Expression*)catchExpr);
}
BinaryenExpressionRef BinaryenTryRemoveCatchBodyAt(BinaryenExpressionRef expr,
                                                   BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->catchBodies.removeAt(index);
}
bool BinaryenTryHasCatchAll(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->hasCatchAll();
}
const char* BinaryenTryGetDelegateTarget(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->delegateTarget.str.data();
}
void BinaryenTrySetDelegateTarget(BinaryenExpressionRef expr,
                                  const char* delegateTarget) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  static_cast<Try*>(expression)->delegateTarget = delegateTarget;
}
bool BinaryenTryIsDelegate(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Try>());
  return static_cast<Try*>(expression)->isDelegate();
}
// Throw
const char* BinaryenThrowGetTag(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  return static_cast<Throw*>(expression)->tag.str.data();
}
void BinaryenThrowSetTag(BinaryenExpressionRef expr, const char* tagName) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  static_cast<Throw*>(expression)->tag = tagName;
}
BinaryenIndex BinaryenThrowGetNumOperands(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  return static_cast<Throw*>(expression)->operands.size();
}
BinaryenExpressionRef BinaryenThrowGetOperandAt(BinaryenExpressionRef expr,
                                                BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  assert(index < static_cast<Throw*>(expression)->operands.size());
  return static_cast<Throw*>(expression)->operands[index];
}
void BinaryenThrowSetOperandAt(BinaryenExpressionRef expr,
                               BinaryenIndex index,
                               BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  assert(index < static_cast<Throw*>(expression)->operands.size());
  assert(operandExpr);
  static_cast<Throw*>(expression)->operands[index] = (Expression*)operandExpr;
}
BinaryenIndex BinaryenThrowAppendOperand(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  assert(operandExpr);
  auto& list = static_cast<Throw*>(expression)->operands;
  auto index = list.size();
  list.push_back((Expression*)operandExpr);
  return index;
}
void BinaryenThrowInsertOperandAt(BinaryenExpressionRef expr,
                                  BinaryenIndex index,
                                  BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  assert(operandExpr);
  static_cast<Throw*>(expression)
    ->operands.insertAt(index, (Expression*)operandExpr);
}
BinaryenExpressionRef BinaryenThrowRemoveOperandAt(BinaryenExpressionRef expr,
                                                   BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Throw>());
  return static_cast<Throw*>(expression)->operands.removeAt(index);
}
// Rethrow
const char* BinaryenRethrowGetTarget(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Rethrow>());
  return static_cast<Rethrow*>(expression)->target.str.data();
}
void BinaryenRethrowSetTarget(BinaryenExpressionRef expr, const char* target) {
  auto* expression = (Expression*)expr;
  assert(expression->is<Rethrow>());
  static_cast<Rethrow*>(expression)->target = target;
}
// TupleMake
BinaryenIndex BinaryenTupleMakeGetNumOperands(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleMake>());
  return static_cast<TupleMake*>(expression)->operands.size();
}
BinaryenExpressionRef BinaryenTupleMakeGetOperandAt(BinaryenExpressionRef expr,
                                                    BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleMake>());
  return static_cast<TupleMake*>(expression)->operands[index];
}
void BinaryenTupleMakeSetOperandAt(BinaryenExpressionRef expr,
                                   BinaryenIndex index,
                                   BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleMake>());
  assert(index < static_cast<TupleMake*>(expression)->operands.size());
  assert(operandExpr);
  static_cast<TupleMake*>(expression)->operands[index] =
    (Expression*)operandExpr;
}
BinaryenIndex
BinaryenTupleMakeAppendOperand(BinaryenExpressionRef expr,
                               BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleMake>());
  assert(operandExpr);
  auto& list = static_cast<TupleMake*>(expression)->operands;
  auto index = list.size();
  list.push_back((Expression*)operandExpr);
  return index;
}
void BinaryenTupleMakeInsertOperandAt(BinaryenExpressionRef expr,
                                      BinaryenIndex index,
                                      BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleMake>());
  assert(operandExpr);
  static_cast<TupleMake*>(expression)
    ->operands.insertAt(index, (Expression*)operandExpr);
}
BinaryenExpressionRef
BinaryenTupleMakeRemoveOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleMake>());
  return static_cast<TupleMake*>(expression)->operands.removeAt(index);
}
// TupleExtract
BinaryenExpressionRef BinaryenTupleExtractGetTuple(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleExtract>());
  return static_cast<TupleExtract*>(expression)->tuple;
}
void BinaryenTupleExtractSetTuple(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef tupleExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleExtract>());
  assert(tupleExpr);
  static_cast<TupleExtract*>(expression)->tuple = (Expression*)tupleExpr;
}
BinaryenIndex BinaryenTupleExtractGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleExtract>());
  return static_cast<TupleExtract*>(expression)->index;
}
void BinaryenTupleExtractSetIndex(BinaryenExpressionRef expr,
                                  BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<TupleExtract>());
  static_cast<TupleExtract*>(expression)->index = index;
}
// I31New
BinaryenExpressionRef BinaryenI31NewGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<I31New>());
  return static_cast<I31New*>(expression)->value;
}
void BinaryenI31NewSetValue(BinaryenExpressionRef expr,
                            BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<I31New>());
  assert(valueExpr);
  static_cast<I31New*>(expression)->value = (Expression*)valueExpr;
}
// I31Get
BinaryenExpressionRef BinaryenI31GetGetI31(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<I31Get>());
  return static_cast<I31Get*>(expression)->i31;
}
void BinaryenI31GetSetI31(BinaryenExpressionRef expr,
                          BinaryenExpressionRef i31Expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<I31Get>());
  assert(i31Expr);
  static_cast<I31Get*>(expression)->i31 = (Expression*)i31Expr;
}
bool BinaryenI31GetIsSigned(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<I31Get>());
  return static_cast<I31Get*>(expression)->signed_;
}
void BinaryenI31GetSetSigned(BinaryenExpressionRef expr, bool signed_) {
  auto* expression = (Expression*)expr;
  assert(expression->is<I31Get>());
  static_cast<I31Get*>(expression)->signed_ = signed_ != 0;
}
// CallRef
BinaryenIndex BinaryenCallRefGetNumOperands(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  return static_cast<CallRef*>(expression)->operands.size();
}
BinaryenExpressionRef BinaryenCallRefGetOperandAt(BinaryenExpressionRef expr,
                                                  BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  assert(index < static_cast<CallRef*>(expression)->operands.size());
  return static_cast<CallRef*>(expression)->operands[index];
}
void BinaryenCallRefSetOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index,
                                 BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  assert(index < static_cast<CallRef*>(expression)->operands.size());
  assert(operandExpr);
  static_cast<CallRef*>(expression)->operands[index] = (Expression*)operandExpr;
}
BinaryenIndex BinaryenCallRefAppendOperand(BinaryenExpressionRef expr,
                                           BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  assert(operandExpr);
  auto& list = static_cast<CallRef*>(expression)->operands;
  auto index = list.size();
  list.push_back((Expression*)operandExpr);
  return index;
}
void BinaryenCallRefInsertOperandAt(BinaryenExpressionRef expr,
                                    BinaryenIndex index,
                                    BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  assert(operandExpr);
  static_cast<CallRef*>(expression)
    ->operands.insertAt(index, (Expression*)operandExpr);
}
BinaryenExpressionRef BinaryenCallRefRemoveOperandAt(BinaryenExpressionRef expr,
                                                     BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  return static_cast<CallRef*>(expression)->operands.removeAt(index);
}
BinaryenExpressionRef BinaryenCallRefGetTarget(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  return static_cast<CallRef*>(expression)->target;
}
void BinaryenCallRefSetTarget(BinaryenExpressionRef expr,
                              BinaryenExpressionRef targetExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  assert(targetExpr);
  static_cast<CallRef*>(expression)->target = (Expression*)targetExpr;
}
bool BinaryenCallRefIsReturn(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  return static_cast<CallRef*>(expression)->isReturn;
}
void BinaryenCallRefSetReturn(BinaryenExpressionRef expr, bool isReturn) {
  auto* expression = (Expression*)expr;
  assert(expression->is<CallRef>());
  static_cast<CallRef*>(expression)->isReturn = isReturn;
}
// RefTest
BinaryenExpressionRef BinaryenRefTestGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefTest>());
  return static_cast<RefTest*>(expression)->ref;
}
void BinaryenRefTestSetRef(BinaryenExpressionRef expr,
                           BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefTest>());
  assert(refExpr);
  static_cast<RefTest*>(expression)->ref = (Expression*)refExpr;
}
BinaryenType BinaryenRefTestGetCastType(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefTest>());
  return static_cast<RefTest*>(expression)->castType.getID();
}
void BinaryenRefTestSetCastType(BinaryenExpressionRef expr,
                                BinaryenType castType) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefTest>());
  static_cast<RefTest*>(expression)->castType = Type(castType);
}
// RefCast
BinaryenExpressionRef BinaryenRefCastGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefCast>());
  return static_cast<RefCast*>(expression)->ref;
}
void BinaryenRefCastSetRef(BinaryenExpressionRef expr,
                           BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<RefCast>());
  assert(refExpr);
  static_cast<RefCast*>(expression)->ref = (Expression*)refExpr;
}
// BrOn
BinaryenOp BinaryenBrOnGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  return static_cast<BrOn*>(expression)->op;
}
void BinaryenBrOnSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  static_cast<BrOn*>(expression)->op = BrOnOp(op);
}
const char* BinaryenBrOnGetName(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  return static_cast<BrOn*>(expression)->name.str.data();
}
void BinaryenBrOnSetName(BinaryenExpressionRef expr, const char* nameStr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  assert(nameStr);
  static_cast<BrOn*>(expression)->name = nameStr;
}
BinaryenExpressionRef BinaryenBrOnGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  return static_cast<BrOn*>(expression)->ref;
}
void BinaryenBrOnSetRef(BinaryenExpressionRef expr,
                        BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  assert(refExpr);
  static_cast<BrOn*>(expression)->ref = (Expression*)refExpr;
}
BinaryenType BinaryenBrOnGetCastType(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  return static_cast<BrOn*>(expression)->castType.getID();
}
void BinaryenBrOnSetCastType(BinaryenExpressionRef expr,
                             BinaryenType castType) {
  auto* expression = (Expression*)expr;
  assert(expression->is<BrOn>());
  static_cast<BrOn*>(expression)->castType = Type(castType);
}
// StructNew
BinaryenIndex BinaryenStructNewGetNumOperands(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructNew>());
  return static_cast<StructNew*>(expression)->operands.size();
}
BinaryenExpressionRef BinaryenStructNewGetOperandAt(BinaryenExpressionRef expr,
                                                    BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructNew>());
  assert(index < static_cast<StructNew*>(expression)->operands.size());
  return static_cast<StructNew*>(expression)->operands[index];
}
void BinaryenStructNewSetOperandAt(BinaryenExpressionRef expr,
                                   BinaryenIndex index,
                                   BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructNew>());
  assert(index < static_cast<StructNew*>(expression)->operands.size());
  assert(operandExpr);
  static_cast<StructNew*>(expression)->operands[index] =
    (Expression*)operandExpr;
}
BinaryenIndex
BinaryenStructNewAppendOperand(BinaryenExpressionRef expr,
                               BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructNew>());
  assert(operandExpr);
  auto& list = static_cast<StructNew*>(expression)->operands;
  auto index = list.size();
  list.push_back((Expression*)operandExpr);
  return index;
}
void BinaryenStructNewInsertOperandAt(BinaryenExpressionRef expr,
                                      BinaryenIndex index,
                                      BinaryenExpressionRef operandExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructNew>());
  assert(operandExpr);
  static_cast<StructNew*>(expression)
    ->operands.insertAt(index, (Expression*)operandExpr);
}
BinaryenExpressionRef
BinaryenStructNewRemoveOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructNew>());
  return static_cast<StructNew*>(expression)->operands.removeAt(index);
}
// StructGet
BinaryenIndex BinaryenStructGetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructGet>());
  return static_cast<StructGet*>(expression)->index;
}
void BinaryenStructGetSetIndex(BinaryenExpressionRef expr,
                               BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructGet>());
  static_cast<StructGet*>(expression)->index = index;
}
BinaryenExpressionRef BinaryenStructGetGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructGet>());
  return static_cast<StructGet*>(expression)->ref;
}
void BinaryenStructGetSetRef(BinaryenExpressionRef expr,
                             BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructGet>());
  assert(refExpr);
  static_cast<StructGet*>(expression)->ref = (Expression*)refExpr;
}
bool BinaryenStructGetIsSigned(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructGet>());
  return static_cast<StructGet*>(expression)->signed_;
}
void BinaryenStructGetSetSigned(BinaryenExpressionRef expr, bool signed_) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructGet>());
  static_cast<StructGet*>(expression)->signed_ = signed_;
}
// StructSet
BinaryenIndex BinaryenStructSetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructSet>());
  return static_cast<StructSet*>(expression)->index;
}
void BinaryenStructSetSetIndex(BinaryenExpressionRef expr,
                               BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructSet>());
  static_cast<StructSet*>(expression)->index = index;
}
BinaryenExpressionRef BinaryenStructSetGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructSet>());
  return static_cast<StructSet*>(expression)->ref;
}
void BinaryenStructSetSetRef(BinaryenExpressionRef expr,
                             BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructSet>());
  assert(refExpr);
  static_cast<StructSet*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef BinaryenStructSetGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructSet>());
  return static_cast<StructSet*>(expression)->value;
}
void BinaryenStructSetSetValue(BinaryenExpressionRef expr,
                               BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StructSet>());
  assert(valueExpr);
  static_cast<StructSet*>(expression)->value = (Expression*)valueExpr;
}
// ArrayNew
BinaryenExpressionRef BinaryenArrayNewGetInit(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNew>());
  return static_cast<ArrayNew*>(expression)->init;
}
void BinaryenArrayNewSetInit(BinaryenExpressionRef expr,
                             BinaryenExpressionRef initExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNew>());
  // may be null
  static_cast<ArrayNew*>(expression)->init = (Expression*)initExpr;
}
BinaryenExpressionRef BinaryenArrayNewGetSize(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNew>());
  return static_cast<ArrayNew*>(expression)->size;
}
void BinaryenArrayNewSetSize(BinaryenExpressionRef expr,
                             BinaryenExpressionRef sizeExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNew>());
  assert(sizeExpr);
  static_cast<ArrayNew*>(expression)->size = (Expression*)sizeExpr;
}
// ArrayNewFixed
BinaryenIndex BinaryenArrayNewFixedGetNumValues(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNewFixed>());
  return static_cast<ArrayNewFixed*>(expression)->values.size();
}
BinaryenExpressionRef
BinaryenArrayNewFixedGetValueAt(BinaryenExpressionRef expr,
                                BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNewFixed>());
  assert(index < static_cast<ArrayNewFixed*>(expression)->values.size());
  return static_cast<ArrayNewFixed*>(expression)->values[index];
}
void BinaryenArrayNewFixedSetValueAt(BinaryenExpressionRef expr,
                                     BinaryenIndex index,
                                     BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNewFixed>());
  assert(index < static_cast<ArrayNewFixed*>(expression)->values.size());
  assert(valueExpr);
  static_cast<ArrayNewFixed*>(expression)->values[index] =
    (Expression*)valueExpr;
}
BinaryenIndex
BinaryenArrayNewFixedAppendValue(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNewFixed>());
  assert(valueExpr);
  auto& list = static_cast<ArrayNewFixed*>(expression)->values;
  auto index = list.size();
  list.push_back((Expression*)valueExpr);
  return index;
}
void BinaryenArrayNewFixedInsertValueAt(BinaryenExpressionRef expr,
                                        BinaryenIndex index,
                                        BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNewFixed>());
  assert(valueExpr);
  static_cast<ArrayNewFixed*>(expression)
    ->values.insertAt(index, (Expression*)valueExpr);
}
BinaryenExpressionRef
BinaryenArrayNewFixedRemoveValueAt(BinaryenExpressionRef expr,
                                   BinaryenIndex index) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayNewFixed>());
  return static_cast<ArrayNewFixed*>(expression)->values.removeAt(index);
}
// ArrayGet
BinaryenExpressionRef BinaryenArrayGetGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayGet>());
  return static_cast<ArrayGet*>(expression)->ref;
}
void BinaryenArrayGetSetRef(BinaryenExpressionRef expr,
                            BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayGet>());
  assert(refExpr);
  static_cast<ArrayGet*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef BinaryenArrayGetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayGet>());
  return static_cast<ArrayGet*>(expression)->index;
}
void BinaryenArrayGetSetIndex(BinaryenExpressionRef expr,
                              BinaryenExpressionRef indexExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayGet>());
  assert(indexExpr);
  static_cast<ArrayGet*>(expression)->index = (Expression*)indexExpr;
}
bool BinaryenArrayGetIsSigned(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayGet>());
  return static_cast<ArrayGet*>(expression)->signed_;
}
void BinaryenArrayGetSetSigned(BinaryenExpressionRef expr, bool signed_) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayGet>());
  static_cast<ArrayGet*>(expression)->signed_ = signed_;
}
// ArraySet
BinaryenExpressionRef BinaryenArraySetGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArraySet>());
  return static_cast<ArraySet*>(expression)->ref;
}
void BinaryenArraySetSetRef(BinaryenExpressionRef expr,
                            BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArraySet>());
  assert(refExpr);
  static_cast<ArraySet*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef BinaryenArraySetGetIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArraySet>());
  return static_cast<ArraySet*>(expression)->index;
}
void BinaryenArraySetSetIndex(BinaryenExpressionRef expr,
                              BinaryenExpressionRef indexExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArraySet>());
  assert(indexExpr);
  static_cast<ArraySet*>(expression)->index = (Expression*)indexExpr;
}
BinaryenExpressionRef BinaryenArraySetGetValue(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArraySet>());
  return static_cast<ArraySet*>(expression)->value;
}
void BinaryenArraySetSetValue(BinaryenExpressionRef expr,
                              BinaryenExpressionRef valueExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArraySet>());
  assert(valueExpr);
  static_cast<ArraySet*>(expression)->value = (Expression*)valueExpr;
}
// ArrayLen
BinaryenExpressionRef BinaryenArrayLenGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayLen>());
  return static_cast<ArrayLen*>(expression)->ref;
}
void BinaryenArrayLenSetRef(BinaryenExpressionRef expr,
                            BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayLen>());
  assert(refExpr);
  static_cast<ArrayLen*>(expression)->ref = (Expression*)refExpr;
}
// ArrayCopy
BinaryenExpressionRef BinaryenArrayCopyGetDestRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  return static_cast<ArrayCopy*>(expression)->destRef;
}
void BinaryenArrayCopySetDestRef(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef destRefExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  assert(destRefExpr);
  static_cast<ArrayCopy*>(expression)->destRef = (Expression*)destRefExpr;
}
BinaryenExpressionRef
BinaryenArrayCopyGetDestIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  return static_cast<ArrayCopy*>(expression)->destIndex;
}
void BinaryenArrayCopySetDestIndex(BinaryenExpressionRef expr,
                                   BinaryenExpressionRef destIndexExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  assert(destIndexExpr);
  static_cast<ArrayCopy*>(expression)->destIndex = (Expression*)destIndexExpr;
}
BinaryenExpressionRef BinaryenArrayCopyGetSrcRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  return static_cast<ArrayCopy*>(expression)->srcRef;
}
void BinaryenArrayCopySetSrcRef(BinaryenExpressionRef expr,
                                BinaryenExpressionRef srcRefExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  assert(srcRefExpr);
  static_cast<ArrayCopy*>(expression)->srcRef = (Expression*)srcRefExpr;
}
BinaryenExpressionRef BinaryenArrayCopyGetSrcIndex(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  return static_cast<ArrayCopy*>(expression)->srcIndex;
}
void BinaryenArrayCopySetSrcIndex(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef srcIndexExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  assert(srcIndexExpr);
  static_cast<ArrayCopy*>(expression)->srcIndex = (Expression*)srcIndexExpr;
}
BinaryenExpressionRef BinaryenArrayCopyGetLength(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  return static_cast<ArrayCopy*>(expression)->length;
}
void BinaryenArrayCopySetLength(BinaryenExpressionRef expr,
                                BinaryenExpressionRef lengthExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<ArrayCopy>());
  assert(lengthExpr);
  static_cast<ArrayCopy*>(expression)->length = (Expression*)lengthExpr;
}
// StringNew
BinaryenOp BinaryenStringNewGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  return static_cast<StringNew*>(expression)->op;
}
void BinaryenStringNewSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  static_cast<StringNew*>(expression)->op = StringNewOp(op);
}
BinaryenExpressionRef BinaryenStringNewGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  return static_cast<StringNew*>(expression)->ptr;
}
void BinaryenStringNewSetPtr(BinaryenExpressionRef expr,
                             BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  assert(ptrExpr);
  static_cast<StringNew*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef BinaryenStringNewGetLength(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  return static_cast<StringNew*>(expression)->length;
}
void BinaryenStringNewSetLength(BinaryenExpressionRef expr,
                                BinaryenExpressionRef lengthExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  // may be null (linear memory only)
  static_cast<StringNew*>(expression)->length = (Expression*)lengthExpr;
}
BinaryenExpressionRef BinaryenStringNewGetStart(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  return static_cast<StringNew*>(expression)->start;
}
void BinaryenStringNewSetStart(BinaryenExpressionRef expr,
                               BinaryenExpressionRef startExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  // may be null (GC only)
  static_cast<StringNew*>(expression)->start = (Expression*)startExpr;
}
BinaryenExpressionRef BinaryenStringNewGetEnd(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  return static_cast<StringNew*>(expression)->end;
}
void BinaryenStringNewSetEnd(BinaryenExpressionRef expr,
                             BinaryenExpressionRef endExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  // may be null (GC only)
  static_cast<StringNew*>(expression)->end = (Expression*)endExpr;
}
void BinaryenStringNewSetTry(BinaryenExpressionRef expr, bool try_) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  static_cast<StringNew*>(expression)->try_ = try_;
}
bool BinaryenStringNewIsTry(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringNew>());
  return static_cast<StringNew*>(expression)->try_;
}
// StringConst
const char* BinaryenStringConstGetString(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringConst>());
  return static_cast<StringConst*>(expression)->string.str.data();
}
void BinaryenStringConstSetString(BinaryenExpressionRef expr,
                                  const char* stringStr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringConst>());
  assert(stringStr);
  static_cast<StringConst*>(expression)->string = stringStr;
}
// StringMeasure
BinaryenOp BinaryenStringMeasureGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringMeasure>());
  return static_cast<StringMeasure*>(expression)->op;
}
void BinaryenStringMeasureSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringMeasure>());
  static_cast<StringMeasure*>(expression)->op = StringMeasureOp(op);
}
BinaryenExpressionRef BinaryenStringMeasureGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringMeasure>());
  return static_cast<StringMeasure*>(expression)->ref;
}
void BinaryenStringMeasureSetRef(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringMeasure>());
  assert(refExpr);
  static_cast<StringMeasure*>(expression)->ref = (Expression*)refExpr;
}
// StringEncode
BinaryenOp BinaryenStringEncodeGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  return static_cast<StringEncode*>(expression)->op;
}
void BinaryenStringEncodeSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  static_cast<StringEncode*>(expression)->op = StringEncodeOp(op);
}
BinaryenExpressionRef BinaryenStringEncodeGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  return static_cast<StringEncode*>(expression)->ref;
}
void BinaryenStringEncodeSetRef(BinaryenExpressionRef expr,
                                BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  assert(refExpr);
  static_cast<StringEncode*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef BinaryenStringEncodeGetPtr(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  return static_cast<StringEncode*>(expression)->ptr;
}
void BinaryenStringEncodeSetPtr(BinaryenExpressionRef expr,
                                BinaryenExpressionRef ptrExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  assert(ptrExpr);
  static_cast<StringEncode*>(expression)->ptr = (Expression*)ptrExpr;
}
BinaryenExpressionRef BinaryenStringEncodeGetStart(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  return static_cast<StringEncode*>(expression)->start;
}
void BinaryenStringEncodeSetStart(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef startExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEncode>());
  // may be null (GC only)
  static_cast<StringEncode*>(expression)->start = (Expression*)startExpr;
}
// StringConcat
BinaryenExpressionRef BinaryenStringConcatGetLeft(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringConcat>());
  return static_cast<StringConcat*>(expression)->left;
}
void BinaryenStringConcatSetLeft(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef leftExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringConcat>());
  assert(leftExpr);
  static_cast<StringConcat*>(expression)->left = (Expression*)leftExpr;
}
BinaryenExpressionRef BinaryenStringConcatGetRight(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringConcat>());
  return static_cast<StringConcat*>(expression)->right;
}
void BinaryenStringConcatSetRight(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef rightExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringConcat>());
  assert(rightExpr);
  static_cast<StringConcat*>(expression)->right = (Expression*)rightExpr;
}
// StringEq
BinaryenOp BinaryenStringEqGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEq>());
  return static_cast<StringEq*>(expression)->op;
}
void BinaryenStringEqSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEq>());
  static_cast<StringEq*>(expression)->op = StringEqOp(op);
}
BinaryenExpressionRef BinaryenStringEqGetLeft(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEq>());
  return static_cast<StringEq*>(expression)->left;
}
void BinaryenStringEqSetLeft(BinaryenExpressionRef expr,
                             BinaryenExpressionRef leftExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEq>());
  assert(leftExpr);
  static_cast<StringEq*>(expression)->left = (Expression*)leftExpr;
}
BinaryenExpressionRef BinaryenStringEqGetRight(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEq>());
  return static_cast<StringEq*>(expression)->right;
}
void BinaryenStringEqSetRight(BinaryenExpressionRef expr,
                              BinaryenExpressionRef rightExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringEq>());
  assert(rightExpr);
  static_cast<StringEq*>(expression)->right = (Expression*)rightExpr;
}
// StringAs
BinaryenOp BinaryenStringAsGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringAs>());
  return static_cast<StringAs*>(expression)->op;
}
void BinaryenStringAsSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringAs>());
  static_cast<StringAs*>(expression)->op = StringAsOp(op);
}
BinaryenExpressionRef BinaryenStringAsGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringAs>());
  return static_cast<StringAs*>(expression)->ref;
}
void BinaryenStringAsSetRef(BinaryenExpressionRef expr,
                            BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringAs>());
  assert(refExpr);
  static_cast<StringAs*>(expression)->ref = (Expression*)refExpr;
}
// StringWTF8Advance
BinaryenExpressionRef
BinaryenStringWTF8AdvanceGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF8Advance>());
  return static_cast<StringWTF8Advance*>(expression)->ref;
}
void BinaryenStringWTF8AdvanceSetRef(BinaryenExpressionRef expr,
                                     BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF8Advance>());
  assert(refExpr);
  static_cast<StringWTF8Advance*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef
BinaryenStringWTF8AdvanceGetPos(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF8Advance>());
  return static_cast<StringWTF8Advance*>(expression)->pos;
}
void BinaryenStringWTF8AdvanceSetPos(BinaryenExpressionRef expr,
                                     BinaryenExpressionRef posExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF8Advance>());
  assert(posExpr);
  static_cast<StringWTF8Advance*>(expression)->pos = (Expression*)posExpr;
}
BinaryenExpressionRef
BinaryenStringWTF8AdvanceGetBytes(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF8Advance>());
  return static_cast<StringWTF8Advance*>(expression)->bytes;
}
void BinaryenStringWTF8AdvanceSetBytes(BinaryenExpressionRef expr,
                                       BinaryenExpressionRef bytesExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF8Advance>());
  assert(bytesExpr);
  static_cast<StringWTF8Advance*>(expression)->bytes = (Expression*)bytesExpr;
}
// StringWTF16Get
BinaryenExpressionRef BinaryenStringWTF16GetGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF16Get>());
  return static_cast<StringWTF16Get*>(expression)->ref;
}
void BinaryenStringWTF16GetSetRef(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF16Get>());
  assert(refExpr);
  static_cast<StringWTF16Get*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef BinaryenStringWTF16GetGetPos(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF16Get>());
  return static_cast<StringWTF16Get*>(expression)->pos;
}
void BinaryenStringWTF16GetSetPos(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef posExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringWTF16Get>());
  assert(posExpr);
  static_cast<StringWTF16Get*>(expression)->pos = (Expression*)posExpr;
}
// StringIterNext
BinaryenExpressionRef BinaryenStringIterNextGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterNext>());
  return static_cast<StringIterNext*>(expression)->ref;
}
void BinaryenStringIterNextSetRef(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterNext>());
  assert(refExpr);
  static_cast<StringIterNext*>(expression)->ref = (Expression*)refExpr;
}
// StringIterMove
BinaryenOp BinaryenStringIterMoveGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterMove>());
  return static_cast<StringIterMove*>(expression)->op;
}
void BinaryenStringIterMoveSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterMove>());
  static_cast<StringIterMove*>(expression)->op = StringIterMoveOp(op);
}
BinaryenExpressionRef BinaryenStringIterMoveGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterMove>());
  return static_cast<StringIterMove*>(expression)->ref;
}
void BinaryenStringIterMoveSetRef(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterMove>());
  assert(refExpr);
  static_cast<StringIterMove*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef BinaryenStringIterMoveGetNum(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterMove>());
  return static_cast<StringIterMove*>(expression)->num;
}
void BinaryenStringIterMoveSetNum(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef numExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringIterMove>());
  assert(numExpr);
  static_cast<StringIterMove*>(expression)->num = (Expression*)numExpr;
}
// StringSliceWTF
BinaryenOp BinaryenStringSliceWTFGetOp(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  return static_cast<StringSliceWTF*>(expression)->op;
}
void BinaryenStringSliceWTFSetOp(BinaryenExpressionRef expr, BinaryenOp op) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  static_cast<StringSliceWTF*>(expression)->op = StringSliceWTFOp(op);
}
BinaryenExpressionRef BinaryenStringSliceWTFGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  return static_cast<StringSliceWTF*>(expression)->ref;
}
void BinaryenStringSliceWTFSetRef(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  assert(refExpr);
  static_cast<StringSliceWTF*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef
BinaryenStringSliceWTFGetStart(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  return static_cast<StringSliceWTF*>(expression)->start;
}
void BinaryenStringSliceWTFSetStart(BinaryenExpressionRef expr,
                                    BinaryenExpressionRef startExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  assert(startExpr);
  static_cast<StringSliceWTF*>(expression)->start = (Expression*)startExpr;
}
BinaryenExpressionRef BinaryenStringSliceWTFGetEnd(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  return static_cast<StringSliceWTF*>(expression)->end;
}
void BinaryenStringSliceWTFSetEnd(BinaryenExpressionRef expr,
                                  BinaryenExpressionRef endExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceWTF>());
  assert(endExpr);
  static_cast<StringSliceWTF*>(expression)->end = (Expression*)endExpr;
}
// StringSliceIter
BinaryenExpressionRef
BinaryenStringSliceIterGetRef(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceIter>());
  return static_cast<StringSliceIter*>(expression)->ref;
}
void BinaryenStringSliceIterSetRef(BinaryenExpressionRef expr,
                                   BinaryenExpressionRef refExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceIter>());
  assert(refExpr);
  static_cast<StringSliceIter*>(expression)->ref = (Expression*)refExpr;
}
BinaryenExpressionRef
BinaryenStringSliceIterGetNum(BinaryenExpressionRef expr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceIter>());
  return static_cast<StringSliceIter*>(expression)->num;
}
void BinaryenStringSliceIterSetNum(BinaryenExpressionRef expr,
                                   BinaryenExpressionRef numExpr) {
  auto* expression = (Expression*)expr;
  assert(expression->is<StringSliceIter>());
  assert(numExpr);
  static_cast<StringSliceIter*>(expression)->num = (Expression*)numExpr;
}

// Functions

BinaryenFunctionRef BinaryenAddFunction(BinaryenModuleRef module,
                                        const char* name,
                                        BinaryenType params,
                                        BinaryenType results,
                                        BinaryenType* varTypes,
                                        BinaryenIndex numVarTypes,
                                        BinaryenExpressionRef body) {
  auto* ret = new Function;
  ret->setExplicitName(name);
  // TODO: Take a HeapType rather than params and results.
  ret->type = Signature(Type(params), Type(results));
  for (BinaryenIndex i = 0; i < numVarTypes; i++) {
    ret->vars.push_back(Type(varTypes[i]));
  }
  ret->body = (Expression*)body;

  // Lock. This can be called from multiple threads at once, and is a
  // point where they all access and modify the module.
  {
    std::lock_guard<std::mutex> lock(BinaryenFunctionMutex);
    ((Module*)module)->addFunction(ret);
  }

  return ret;
}
BinaryenFunctionRef BinaryenGetFunction(BinaryenModuleRef module,
                                        const char* name) {
  return ((Module*)module)->getFunctionOrNull(name);
}
void BinaryenRemoveFunction(BinaryenModuleRef module, const char* name) {
  ((Module*)module)->removeFunction(name);
}
BinaryenIndex BinaryenGetNumFunctions(BinaryenModuleRef module) {
  return ((Module*)module)->functions.size();
}
BinaryenFunctionRef BinaryenGetFunctionByIndex(BinaryenModuleRef module,
                                               BinaryenIndex index) {
  const auto& functions = ((Module*)module)->functions;
  if (functions.size() <= index) {
    Fatal() << "invalid function index.";
  }
  return functions[index].get();
}

// Globals

BinaryenGlobalRef BinaryenAddGlobal(BinaryenModuleRef module,
                                    const char* name,
                                    BinaryenType type,
                                    bool mutable_,
                                    BinaryenExpressionRef init) {
  auto* ret = new Global();
  ret->setExplicitName(name);
  ret->type = Type(type);
  ret->mutable_ = mutable_;
  ret->init = (Expression*)init;
  ((Module*)module)->addGlobal(ret);
  return ret;
}
BinaryenGlobalRef BinaryenGetGlobal(BinaryenModuleRef module,
                                    const char* name) {
  return ((Module*)module)->getGlobalOrNull(name);
}
void BinaryenRemoveGlobal(BinaryenModuleRef module, const char* name) {
  ((Module*)module)->removeGlobal(name);
}
BinaryenIndex BinaryenGetNumGlobals(BinaryenModuleRef module) {
  return ((Module*)module)->globals.size();
}
BinaryenGlobalRef BinaryenGetGlobalByIndex(BinaryenModuleRef module,
                                           BinaryenIndex index) {
  const auto& globals = ((Module*)module)->globals;
  if (globals.size() <= index) {
    Fatal() << "invalid global index.";
  }
  return globals[index].get();
}

// Tags

BinaryenTagRef BinaryenAddTag(BinaryenModuleRef module,
                              const char* name,
                              BinaryenType params,
                              BinaryenType results) {
  auto* ret = new Tag();
  ret->setExplicitName(name);
  ret->sig = Signature(Type(params), Type(results));
  ((Module*)module)->addTag(ret);
  return ret;
}

BinaryenTagRef BinaryenGetTag(BinaryenModuleRef module, const char* name) {
  return ((Module*)module)->getTagOrNull(name);
}
void BinaryenRemoveTag(BinaryenModuleRef module, const char* name) {
  ((Module*)module)->removeTag(name);
}

// Imports

void BinaryenAddFunctionImport(BinaryenModuleRef module,
                               const char* internalName,
                               const char* externalModuleName,
                               const char* externalBaseName,
                               BinaryenType params,
                               BinaryenType results) {
  auto* func = ((Module*)module)->getFunctionOrNull(internalName);
  if (func == nullptr) {
    auto func = std::make_unique<Function>();
    func->name = internalName;
    func->module = externalModuleName;
    func->base = externalBaseName;
    // TODO: Take a HeapType rather than params and results.
    func->type = Signature(Type(params), Type(results));
    ((Module*)module)->addFunction(std::move(func));
  } else {
    // already exists so just set module and base
    func->module = externalModuleName;
    func->base = externalBaseName;
  }
}
void BinaryenAddTableImport(BinaryenModuleRef module,
                            const char* internalName,
                            const char* externalModuleName,
                            const char* externalBaseName) {
  auto* table = ((Module*)module)->getTableOrNull(internalName);
  if (table == nullptr) {
    auto table = std::make_unique<Table>();
    table->name = internalName;
    table->module = externalModuleName;
    table->base = externalBaseName;
    ((Module*)module)->addTable(std::move(table));
  } else {
    // already exists so just set module and base
    table->module = externalModuleName;
    table->base = externalBaseName;
  }
}
void BinaryenAddMemoryImport(BinaryenModuleRef module,
                             const char* internalName,
                             const char* externalModuleName,
                             const char* externalBaseName,
                             uint8_t shared) {
  auto* memory = ((Module*)module)->getMemoryOrNull(internalName);
  if (memory == nullptr) {
    auto memory = std::make_unique<Memory>();
    memory->name = internalName;
    memory->module = externalModuleName;
    memory->base = externalBaseName;
    memory->shared = shared;
    ((Module*)module)->addMemory(std::move(memory));
  } else {
    // already exists so just set module and base
    memory->module = externalModuleName;
    memory->base = externalBaseName;
  }
}
void BinaryenAddGlobalImport(BinaryenModuleRef module,
                             const char* internalName,
                             const char* externalModuleName,
                             const char* externalBaseName,
                             BinaryenType globalType,
                             bool mutable_) {
  auto* glob = ((Module*)module)->getGlobalOrNull(internalName);
  if (glob == nullptr) {
    auto glob = std::make_unique<Global>();
    glob->name = internalName;
    glob->module = externalModuleName;
    glob->base = externalBaseName;
    glob->type = Type(globalType);
    glob->mutable_ = mutable_;
    ((Module*)module)->addGlobal(std::move(glob));
  } else {
    // already exists so just set module and base
    glob->module = externalModuleName;
    glob->base = externalBaseName;
  }
}
void BinaryenAddTagImport(BinaryenModuleRef module,
                          const char* internalName,
                          const char* externalModuleName,
                          const char* externalBaseName,
                          BinaryenType params,
                          BinaryenType results) {
  auto* tag = ((Module*)module)->getGlobalOrNull(internalName);
  if (tag == nullptr) {
    auto tag = std::make_unique<Tag>();
    tag->name = internalName;
    tag->module = externalModuleName;
    tag->base = externalBaseName;
    tag->sig = Signature(Type(params), Type(results));
    ((Module*)module)->addTag(std::move(tag));
  } else {
    // already exists so just set module and base
    tag->module = externalModuleName;
    tag->base = externalBaseName;
  }
}

// Exports

WASM_DEPRECATED BinaryenExportRef BinaryenAddExport(BinaryenModuleRef module,
                                                    const char* internalName,
                                                    const char* externalName) {
  return BinaryenAddFunctionExport(module, internalName, externalName);
}
BinaryenExportRef BinaryenAddFunctionExport(BinaryenModuleRef module,
                                            const char* internalName,
                                            const char* externalName) {
  auto* ret = new Export();
  ret->value = internalName;
  ret->name = externalName;
  ret->kind = ExternalKind::Function;
  ((Module*)module)->addExport(ret);
  return ret;
}
BinaryenExportRef BinaryenAddTableExport(BinaryenModuleRef module,
                                         const char* internalName,
                                         const char* externalName) {
  auto* ret = new Export();
  ret->value = internalName;
  ret->name = externalName;
  ret->kind = ExternalKind::Table;
  ((Module*)module)->addExport(ret);
  return ret;
}
BinaryenExportRef BinaryenAddMemoryExport(BinaryenModuleRef module,
                                          const char* internalName,
                                          const char* externalName) {
  auto* ret = new Export();
  ret->value = internalName;
  ret->name = externalName;
  ret->kind = ExternalKind::Memory;
  ((Module*)module)->addExport(ret);
  return ret;
}
BinaryenExportRef BinaryenAddGlobalExport(BinaryenModuleRef module,
                                          const char* internalName,
                                          const char* externalName) {
  auto* ret = new Export();
  ret->value = internalName;
  ret->name = externalName;
  ret->kind = ExternalKind::Global;
  ((Module*)module)->addExport(ret);
  return ret;
}
BinaryenExportRef BinaryenAddTagExport(BinaryenModuleRef module,
                                       const char* internalName,
                                       const char* externalName) {
  auto* ret = new Export();
  ret->value = internalName;
  ret->name = externalName;
  ret->kind = ExternalKind::Tag;
  ((Module*)module)->addExport(ret);
  return ret;
}
BinaryenExportRef BinaryenGetExport(BinaryenModuleRef module,
                                    const char* externalName) {
  return ((Module*)module)->getExportOrNull(externalName);
}
void BinaryenRemoveExport(BinaryenModuleRef module, const char* externalName) {
  ((Module*)module)->removeExport(externalName);
}
BinaryenIndex BinaryenGetNumExports(BinaryenModuleRef module) {
  return ((Module*)module)->exports.size();
}
BinaryenExportRef BinaryenGetExportByIndex(BinaryenModuleRef module,
                                           BinaryenIndex index) {
  const auto& exports = ((Module*)module)->exports;
  if (exports.size() <= index) {
    Fatal() << "invalid export index.";
  }
  return exports[index].get();
}

BinaryenTableRef BinaryenAddTable(BinaryenModuleRef module,
                                  const char* name,
                                  BinaryenIndex initial,
                                  BinaryenIndex maximum,
                                  BinaryenType tableType) {
  auto table = Builder::makeTable(name, Type(tableType), initial, maximum);
  table->hasExplicitName = true;
  return ((Module*)module)->addTable(std::move(table));
}
void BinaryenRemoveTable(BinaryenModuleRef module, const char* table) {
  ((Module*)module)->removeTable(table);
}
BinaryenIndex BinaryenGetNumTables(BinaryenModuleRef module) {
  return ((Module*)module)->tables.size();
}
BinaryenTableRef BinaryenGetTable(BinaryenModuleRef module, const char* name) {
  return ((Module*)module)->getTableOrNull(name);
}
BinaryenTableRef BinaryenGetTableByIndex(BinaryenModuleRef module,
                                         BinaryenIndex index) {
  const auto& tables = ((Module*)module)->tables;
  if (tables.size() <= index) {
    Fatal() << "invalid table index.";
  }
  return tables[index].get();
}
BinaryenElementSegmentRef
BinaryenAddActiveElementSegment(BinaryenModuleRef module,
                                const char* table,
                                const char* name,
                                const char** funcNames,
                                BinaryenIndex numFuncNames,
                                BinaryenExpressionRef offset) {
  auto segment = std::make_unique<ElementSegment>(table, (Expression*)offset);
  segment->setExplicitName(name);
  for (BinaryenIndex i = 0; i < numFuncNames; i++) {
    auto* func = ((Module*)module)->getFunctionOrNull(funcNames[i]);
    if (func == nullptr) {
      Fatal() << "invalid function '" << funcNames[i] << "'.";
    }
    segment->data.push_back(
      Builder(*(Module*)module).makeRefFunc(funcNames[i], func->type));
  }
  return ((Module*)module)->addElementSegment(std::move(segment));
}
BinaryenElementSegmentRef
BinaryenAddPassiveElementSegment(BinaryenModuleRef module,
                                 const char* name,
                                 const char** funcNames,
                                 BinaryenIndex numFuncNames) {
  auto segment = std::make_unique<ElementSegment>();
  segment->setExplicitName(name);
  for (BinaryenIndex i = 0; i < numFuncNames; i++) {
    auto* func = ((Module*)module)->getFunctionOrNull(funcNames[i]);
    if (func == nullptr) {
      Fatal() << "invalid function '" << funcNames[i] << "'.";
    }
    segment->data.push_back(
      Builder(*(Module*)module).makeRefFunc(funcNames[i], func->type));
  }
  return ((Module*)module)->addElementSegment(std::move(segment));
}
void BinaryenRemoveElementSegment(BinaryenModuleRef module, const char* name) {
  ((Module*)module)->removeElementSegment(name);
}
BinaryenElementSegmentRef BinaryenGetElementSegment(BinaryenModuleRef module,
                                                    const char* name) {
  return ((Module*)module)->getElementSegmentOrNull(name);
}
BinaryenElementSegmentRef
BinaryenGetElementSegmentByIndex(BinaryenModuleRef module,
                                 BinaryenIndex index) {
  const auto& elementSegments = ((Module*)module)->elementSegments;
  if (elementSegments.size() <= index) {
    Fatal() << "invalid table index.";
  }
  return elementSegments[index].get();
}
BinaryenIndex BinaryenGetNumElementSegments(BinaryenModuleRef module) {
  return ((Module*)module)->elementSegments.size();
}
BinaryenExpressionRef
BinaryenElementSegmentGetOffset(BinaryenElementSegmentRef elem) {
  if (((ElementSegment*)elem)->table.isNull()) {
    Fatal() << "elem segment is passive.";
  }
  return ((ElementSegment*)elem)->offset;
}
BinaryenIndex BinaryenElementSegmentGetLength(BinaryenElementSegmentRef elem) {
  return ((ElementSegment*)elem)->data.size();
}
const char* BinaryenElementSegmentGetData(BinaryenElementSegmentRef elem,
                                          BinaryenIndex dataId) {
  const auto& data = ((ElementSegment*)elem)->data;
  if (data.size() <= dataId) {
    Fatal() << "invalid segment data id.";
  }
  if (data[dataId]->is<RefNull>()) {
    return NULL;
  } else if (auto* get = data[dataId]->dynCast<RefFunc>()) {
    return get->func.str.data();
  } else {
    Fatal() << "invalid expression in segment data.";
  }
}

// Memory.

void BinaryenSetMemory(BinaryenModuleRef module,
                       BinaryenIndex initial,
                       BinaryenIndex maximum,
                       const char* exportName,
                       const char** segments,
                       bool* segmentPassive,
                       BinaryenExpressionRef* segmentOffsets,
                       BinaryenIndex* segmentSizes,
                       BinaryenIndex numSegments,
                       bool shared,
                       bool memory64,
                       const char* name) {
  auto memory = std::make_unique<Memory>();
  memory->name = name ? name : "0";
  memory->initial = initial;
  memory->max = int32_t(maximum); // Make sure -1 extends.
  memory->shared = shared;
  memory->indexType = memory64 ? Type::i64 : Type::i32;
  if (exportName) {
    auto memoryExport = std::make_unique<Export>();
    memoryExport->name = exportName;
    memoryExport->value = memory->name;
    memoryExport->kind = ExternalKind::Memory;
    ((Module*)module)->addExport(memoryExport.release());
  }
  ((Module*)module)->removeDataSegments([&](DataSegment* curr) {
    return true;
  });
  for (BinaryenIndex i = 0; i < numSegments; i++) {
    auto curr = Builder::makeDataSegment(Name::fromInt(i),
                                         memory->name,
                                         segmentPassive[i],
                                         (Expression*)segmentOffsets[i],
                                         segments[i],
                                         segmentSizes[i]);
    curr->hasExplicitName = false;
    ((Module*)module)->addDataSegment(std::move(curr));
  }
  ((Module*)module)->removeMemories([&](Memory* curr) { return true; });
  ((Module*)module)->addMemory(std::move(memory));
}

// Memory segments

uint32_t BinaryenGetNumMemorySegments(BinaryenModuleRef module) {
  return ((Module*)module)->dataSegments.size();
}
uint32_t BinaryenGetMemorySegmentByteOffset(BinaryenModuleRef module,
                                            BinaryenIndex id) {
  auto* wasm = (Module*)module;
  if (wasm->dataSegments.size() <= id) {
    Fatal() << "invalid segment id.";
  }

  auto globalOffset = [&](const Expression* const& expr,
                          int64_t& result) -> bool {
    if (auto* c = expr->dynCast<Const>()) {
      result = c->value.getInteger();
      return true;
    }
    return false;
  };

  const auto& segment = wasm->dataSegments[id];

  int64_t ret;
  if (globalOffset(segment->offset, ret)) {
    return ret;
  }
  if (auto* get = segment->offset->dynCast<GlobalGet>()) {
    Global* global = wasm->getGlobal(get->name);
    if (globalOffset(global->init, ret)) {
      return ret;
    }
  }

  Fatal() << "non-constant offsets aren't supported yet";
  return 0;
}
bool BinaryenHasMemory(BinaryenModuleRef module) {
  return !((Module*)module)->memories.empty();
}
BinaryenIndex BinaryenMemoryGetInitial(BinaryenModuleRef module,
                                       const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  return memory->initial;
}
bool BinaryenMemoryHasMax(BinaryenModuleRef module, const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  return memory->hasMax();
}
BinaryenIndex BinaryenMemoryGetMax(BinaryenModuleRef module, const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  return memory->max;
}
const char* BinaryenMemoryImportGetModule(BinaryenModuleRef module,
                                          const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  if (memory->imported()) {
    return memory->module.str.data();
  } else {
    return "";
  }
}
const char* BinaryenMemoryImportGetBase(BinaryenModuleRef module,
                                        const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  if (memory->imported()) {
    return memory->base.str.data();
  } else {
    return "";
  }
}
bool BinaryenMemoryIsShared(BinaryenModuleRef module, const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  return memory->shared;
}
bool BinaryenMemoryIs64(BinaryenModuleRef module, const char* name) {
  // Maintaining compatibility for instructions with a single memory
  if (name == nullptr && module->memories.size() == 1) {
    name = module->memories[0]->name.str.data();
  }
  auto* memory = ((Module*)module)->getMemoryOrNull(name);
  if (memory == nullptr) {
    Fatal() << "invalid memory '" << name << "'.";
  }
  return memory->is64();
}
size_t BinaryenGetMemorySegmentByteLength(BinaryenModuleRef module,
                                          BinaryenIndex id) {
  const auto& segments = ((Module*)module)->dataSegments;
  if (segments.size() <= id) {
    Fatal() << "invalid segment id.";
  }
  return segments[id]->data.size();
}
bool BinaryenGetMemorySegmentPassive(BinaryenModuleRef module,
                                     BinaryenIndex id) {
  const auto& segments = ((Module*)module)->dataSegments;
  if (segments.size() <= id) {
    Fatal() << "invalid segment id.";
  }
  return segments[id]->isPassive;
}
void BinaryenCopyMemorySegmentData(BinaryenModuleRef module,
                                   BinaryenIndex id,
                                   char* buffer) {
  const auto& segments = ((Module*)module)->dataSegments;
  if (segments.size() <= id) {
    Fatal() << "invalid segment id.";
  }
  const auto& segment = segments[id];
  std::copy(segment->data.cbegin(), segment->data.cend(), buffer);
}

// Start function. One per module

void BinaryenSetStart(BinaryenModuleRef module, BinaryenFunctionRef start) {
  ((Module*)module)->addStart(((Function*)start)->name);
}

// Features

BinaryenFeatures BinaryenModuleGetFeatures(BinaryenModuleRef module) {
  return ((Module*)module)->features.features;
}

void BinaryenModuleSetFeatures(BinaryenModuleRef module,
                               BinaryenFeatures features) {
  ((Module*)module)->features.features = features;
}

//
// ========== Module Operations ==========
//

BinaryenModuleRef BinaryenModuleParse(const char* text) {
  auto* wasm = new Module;
  try {
    SExpressionParser parser(text);
    Element& root = *parser.root;
    SExpressionWasmBuilder builder(*wasm, *root[0], IRProfile::Normal);
  } catch (ParseException& p) {
    p.dump(std::cerr);
    Fatal() << "error in parsing wasm text";
  }
  return wasm;
}

void BinaryenModulePrint(BinaryenModuleRef module) {
  std::cout << *(Module*)module;
}

void BinaryenModulePrintStackIR(BinaryenModuleRef module, bool optimize) {
  wasm::printStackIR(std::cout, (Module*)module, optimize);
}

void BinaryenModulePrintAsmjs(BinaryenModuleRef module) {
  auto* wasm = (Module*)module;
  Wasm2JSBuilder::Flags flags;
  Wasm2JSBuilder wasm2js(flags, globalPassOptions);
  auto asmjs = wasm2js.processWasm(wasm);
  JSPrinter jser(true, true, asmjs);
  Output out("", Flags::Text); // stdout
  Wasm2JSGlue glue(*wasm, out, flags, "asmFunc");
  glue.emitPre();
  jser.printAst();
  std::cout << jser.buffer << std::endl;
  glue.emitPost();
}

bool BinaryenModuleValidate(BinaryenModuleRef module) {
  return WasmValidator().validate(*(Module*)module);
}

void BinaryenModuleOptimize(BinaryenModuleRef module) {
  PassRunner passRunner((Module*)module);
  passRunner.options = globalPassOptions;
  passRunner.addDefaultOptimizationPasses();
  passRunner.run();
}

void BinaryenModuleUpdateMaps(BinaryenModuleRef module) {
  ((Module*)module)->updateMaps();
}

int BinaryenGetOptimizeLevel(void) { return globalPassOptions.optimizeLevel; }

void BinaryenSetOptimizeLevel(int level) {
  globalPassOptions.optimizeLevel = level;
}

int BinaryenGetShrinkLevel(void) { return globalPassOptions.shrinkLevel; }

void BinaryenSetShrinkLevel(int level) {
  globalPassOptions.shrinkLevel = level;
}

bool BinaryenGetDebugInfo(void) { return globalPassOptions.debugInfo; }

void BinaryenSetDebugInfo(bool on) { globalPassOptions.debugInfo = on != 0; }

bool BinaryenGetLowMemoryUnused(void) {
  return globalPassOptions.lowMemoryUnused;
}

void BinaryenSetLowMemoryUnused(bool on) {
  globalPassOptions.lowMemoryUnused = on != 0;
}

bool BinaryenGetZeroFilledMemory(void) {
  return globalPassOptions.zeroFilledMemory;
}

void BinaryenSetZeroFilledMemory(bool on) {
  globalPassOptions.zeroFilledMemory = on != 0;
}

bool BinaryenGetFastMath(void) { return globalPassOptions.fastMath; }

void BinaryenSetFastMath(bool value) { globalPassOptions.fastMath = value; }

const char* BinaryenGetPassArgument(const char* key) {
  assert(key);
  const auto& args = globalPassOptions.arguments;
  auto it = args.find(key);
  if (it == args.end()) {
    return nullptr;
  }
  // internalize the string so it remains valid while the module is
  return Name(it->second).str.data();
}

void BinaryenSetPassArgument(const char* key, const char* value) {
  assert(key);
  if (value) {
    globalPassOptions.arguments[key] = value;
  } else {
    globalPassOptions.arguments.erase(key);
  }
}

void BinaryenClearPassArguments(void) { globalPassOptions.arguments.clear(); }

BinaryenIndex BinaryenGetAlwaysInlineMaxSize(void) {
  return globalPassOptions.inlining.alwaysInlineMaxSize;
}

void BinaryenSetAlwaysInlineMaxSize(BinaryenIndex size) {
  globalPassOptions.inlining.alwaysInlineMaxSize = size;
}

BinaryenIndex BinaryenGetFlexibleInlineMaxSize(void) {
  return globalPassOptions.inlining.flexibleInlineMaxSize;
}

void BinaryenSetFlexibleInlineMaxSize(BinaryenIndex size) {
  globalPassOptions.inlining.flexibleInlineMaxSize = size;
}

BinaryenIndex BinaryenGetOneCallerInlineMaxSize(void) {
  return globalPassOptions.inlining.oneCallerInlineMaxSize;
}

void BinaryenSetOneCallerInlineMaxSize(BinaryenIndex size) {
  globalPassOptions.inlining.oneCallerInlineMaxSize = size;
}

bool BinaryenGetAllowInliningFunctionsWithLoops(void) {
  return globalPassOptions.inlining.allowFunctionsWithLoops;
}

void BinaryenSetAllowInliningFunctionsWithLoops(bool enabled) {
  globalPassOptions.inlining.allowFunctionsWithLoops = enabled;
}

void BinaryenModuleRunPasses(BinaryenModuleRef module,
                             const char** passes,
                             BinaryenIndex numPasses) {
  PassRunner passRunner((Module*)module);
  passRunner.options = globalPassOptions;
  for (BinaryenIndex i = 0; i < numPasses; i++) {
    passRunner.add(passes[i]);
  }
  passRunner.run();
}

void BinaryenModuleAutoDrop(BinaryenModuleRef module) {
  auto* wasm = (Module*)module;
  PassRunner runner(wasm, globalPassOptions);
  AutoDrop().run(&runner, wasm);
}

static BinaryenBufferSizes writeModule(BinaryenModuleRef module,
                                       char* output,
                                       size_t outputSize,
                                       const char* sourceMapUrl,
                                       char* sourceMap,
                                       size_t sourceMapSize) {
  BufferWithRandomAccess buffer;
  WasmBinaryWriter writer((Module*)module, buffer);
  writer.setNamesSection(globalPassOptions.debugInfo);
  std::ostringstream os;
  if (sourceMapUrl) {
    writer.setSourceMap(&os, sourceMapUrl);
  }
  writer.write();
  size_t bytes = std::min(buffer.size(), outputSize);
  std::copy_n(buffer.begin(), bytes, output);
  size_t sourceMapBytes = 0;
  if (sourceMapUrl) {
    auto str = os.str();
    sourceMapBytes = std::min(str.length(), sourceMapSize);
    std::copy_n(str.c_str(), sourceMapBytes, sourceMap);
  }
  return {bytes, sourceMapBytes};
}

size_t
BinaryenModuleWrite(BinaryenModuleRef module, char* output, size_t outputSize) {
  return writeModule((Module*)module, output, outputSize, nullptr, nullptr, 0)
    .outputBytes;
}

size_t BinaryenModuleWriteText(BinaryenModuleRef module,
                               char* output,
                               size_t outputSize) {
  // use a stringstream as an std::ostream. Extract the std::string
  // representation, and then store in the output.
  std::stringstream ss;
  ss << *(Module*)module;

  const auto temp = ss.str();
  const auto ctemp = temp.c_str();

  strncpy(output, ctemp, outputSize);
  return std::min(outputSize, temp.size());
}

size_t BinaryenModuleWriteStackIR(BinaryenModuleRef module,
                                  char* output,
                                  size_t outputSize,
                                  bool optimize) {
  // use a stringstream as an std::ostream. Extract the std::string
  // representation, and then store in the output.
  std::stringstream ss;
  wasm::printStackIR(ss, (Module*)module, optimize);

  const auto temp = ss.str();
  const auto ctemp = temp.c_str();

  strncpy(output, ctemp, outputSize);
  return std::min(outputSize, temp.size());
}

BinaryenBufferSizes BinaryenModuleWriteWithSourceMap(BinaryenModuleRef module,
                                                     const char* url,
                                                     char* output,
                                                     size_t outputSize,
                                                     char* sourceMap,
                                                     size_t sourceMapSize) {
  assert(url);
  assert(sourceMap);
  return writeModule(
    (Module*)module, output, outputSize, url, sourceMap, sourceMapSize);
}

BinaryenModuleAllocateAndWriteResult
BinaryenModuleAllocateAndWrite(BinaryenModuleRef module,
                               const char* sourceMapUrl) {
  BufferWithRandomAccess buffer;
  WasmBinaryWriter writer((Module*)module, buffer);
  writer.setNamesSection(globalPassOptions.debugInfo);
  std::ostringstream os;
  if (sourceMapUrl) {
    writer.setSourceMap(&os, sourceMapUrl);
  }
  writer.write();
  void* binary = malloc(buffer.size());
  std::copy_n(buffer.begin(), buffer.size(), static_cast<char*>(binary));
  char* sourceMap = nullptr;
  if (sourceMapUrl) {
    auto str = os.str();
    const size_t len = str.length() + 1;
    sourceMap = (char*)malloc(len);
    std::copy_n(str.c_str(), len, sourceMap);
  }
  return {binary, buffer.size(), sourceMap};
}

char* BinaryenModuleAllocateAndWriteText(BinaryenModuleRef module) {
  std::ostringstream os;
  bool colors = Colors::isEnabled();

  Colors::setEnabled(false); // do not use colors for writing
  os << *(Module*)module;
  Colors::setEnabled(colors); // restore colors state

  auto str = os.str();
  const size_t len = str.length() + 1;
  char* output = (char*)malloc(len);
  std::copy_n(str.c_str(), len, output);
  return output;
}

char* BinaryenModuleAllocateAndWriteStackIR(BinaryenModuleRef module,
                                            bool optimize) {
  std::ostringstream os;
  bool colors = Colors::isEnabled();

  Colors::setEnabled(false); // do not use colors for writing
  wasm::printStackIR(os, (Module*)module, optimize);
  Colors::setEnabled(colors); // restore colors state

  auto str = os.str();
  const size_t len = str.length() + 1;
  char* output = (char*)malloc(len);
  std::copy_n(str.c_str(), len, output);
  return output;
}

BinaryenModuleRef BinaryenModuleRead(char* input, size_t inputSize) {
  auto* wasm = new Module;
  std::vector<char> buffer(false);
  buffer.resize(inputSize);
  std::copy_n(input, inputSize, buffer.begin());
  try {
    // TODO: allow providing features in the C API
    WasmBinaryBuilder parser(*wasm, FeatureSet::MVP, buffer);
    parser.read();
  } catch (ParseException& p) {
    p.dump(std::cerr);
    Fatal() << "error in parsing wasm binary";
  }
  return wasm;
}

void BinaryenModuleInterpret(BinaryenModuleRef module) {
  ShellExternalInterface interface;
  ModuleRunner instance(*(Module*)module, &interface, {});
}

BinaryenIndex BinaryenModuleAddDebugInfoFileName(BinaryenModuleRef module,
                                                 const char* filename) {
  auto& debugInfoFileNames = ((Module*)module)->debugInfoFileNames;
  BinaryenIndex index = debugInfoFileNames.size();
  debugInfoFileNames.push_back(filename);
  return index;
}

const char* BinaryenModuleGetDebugInfoFileName(BinaryenModuleRef module,
                                               BinaryenIndex index) {
  const auto& debugInfoFileNames = ((Module*)module)->debugInfoFileNames;
  return index < debugInfoFileNames.size()
           ? debugInfoFileNames.at(index).c_str()
           : nullptr;
}

//
// ========== Function Operations ==========
//

// TODO: add BinaryenFunctionGetType

const char* BinaryenFunctionGetName(BinaryenFunctionRef func) {
  return ((Function*)func)->name.str.data();
}
BinaryenType BinaryenFunctionGetParams(BinaryenFunctionRef func) {
  return ((Function*)func)->getParams().getID();
}
BinaryenType BinaryenFunctionGetResults(BinaryenFunctionRef func) {
  return ((Function*)func)->getResults().getID();
}
BinaryenIndex BinaryenFunctionGetNumVars(BinaryenFunctionRef func) {
  return ((Function*)func)->vars.size();
}
BinaryenType BinaryenFunctionGetVar(BinaryenFunctionRef func,
                                    BinaryenIndex index) {
  const auto& vars = ((Function*)func)->vars;
  assert(index < vars.size());
  return vars[index].getID();
}
BinaryenIndex BinaryenFunctionGetNumLocals(BinaryenFunctionRef func) {
  return ((Function*)func)->getNumLocals();
}
bool BinaryenFunctionHasLocalName(BinaryenFunctionRef func,
                                  BinaryenIndex index) {
  return ((Function*)func)->hasLocalName(index);
}
const char* BinaryenFunctionGetLocalName(BinaryenFunctionRef func,
                                         BinaryenIndex index) {
  return ((Function*)func)->getLocalName(index).str.data();
}
void BinaryenFunctionSetLocalName(BinaryenFunctionRef func,
                                  BinaryenIndex index,
                                  const char* name) {
  ((Function*)func)->setLocalName(index, name);
}
BinaryenExpressionRef BinaryenFunctionGetBody(BinaryenFunctionRef func) {
  return ((Function*)func)->body;
}
void BinaryenFunctionSetBody(BinaryenFunctionRef func,
                             BinaryenExpressionRef body) {
  assert(body);
  ((Function*)func)->body = (Expression*)body;
}
void BinaryenFunctionOptimize(BinaryenFunctionRef func,
                              BinaryenModuleRef module) {
  PassRunner passRunner((Module*)module);
  passRunner.options = globalPassOptions;
  passRunner.addDefaultFunctionOptimizationPasses();
  passRunner.runOnFunction((Function*)func);
}
void BinaryenFunctionRunPasses(BinaryenFunctionRef func,
                               BinaryenModuleRef module,
                               const char** passes,
                               BinaryenIndex numPasses) {
  PassRunner passRunner((Module*)module);
  passRunner.options = globalPassOptions;
  for (BinaryenIndex i = 0; i < numPasses; i++) {
    passRunner.add(passes[i]);
  }
  passRunner.runOnFunction((Function*)func);
}
void BinaryenFunctionSetDebugLocation(BinaryenFunctionRef func,
                                      BinaryenExpressionRef expr,
                                      BinaryenIndex fileIndex,
                                      BinaryenIndex lineNumber,
                                      BinaryenIndex columnNumber) {
  Function::DebugLocation loc;
  loc.fileIndex = fileIndex;
  loc.lineNumber = lineNumber;
  loc.columnNumber = columnNumber;
  ((Function*)func)->debugLocations[(Expression*)expr] = loc;
}

//
// =========== Table operations ===========
//

const char* BinaryenTableGetName(BinaryenTableRef table) {
  return ((Table*)table)->name.str.data();
}
void BinaryenTableSetName(BinaryenTableRef table, const char* name) {
  ((Table*)table)->name = name;
}
BinaryenIndex BinaryenTableGetInitial(BinaryenTableRef table) {
  return ((Table*)table)->initial;
}
void BinaryenTableSetInitial(BinaryenTableRef table, BinaryenIndex initial) {
  ((Table*)table)->initial = initial;
}
bool BinaryenTableHasMax(BinaryenTableRef table) {
  return ((Table*)table)->hasMax();
}
BinaryenIndex BinaryenTableGetMax(BinaryenTableRef table) {
  return ((Table*)table)->max;
}
void BinaryenTableSetMax(BinaryenTableRef table, BinaryenIndex max) {
  ((Table*)table)->max = max;
}

//
// =========== ElementSegment operations ===========
//
const char* BinaryenElementSegmentGetName(BinaryenElementSegmentRef elem) {
  return ((ElementSegment*)elem)->name.str.data();
}
void BinaryenElementSegmentSetName(BinaryenElementSegmentRef elem,
                                   const char* name) {
  ((ElementSegment*)elem)->name = name;
}
const char* BinaryenElementSegmentGetTable(BinaryenElementSegmentRef elem) {
  return ((ElementSegment*)elem)->table.str.data();
}
void BinaryenElementSegmentSetTable(BinaryenElementSegmentRef elem,
                                    const char* table) {
  ((ElementSegment*)elem)->table = table;
}
bool BinaryenElementSegmentIsPassive(BinaryenElementSegmentRef elem) {
  return ((ElementSegment*)elem)->table.isNull();
}

//
// =========== Global operations ===========
//

const char* BinaryenGlobalGetName(BinaryenGlobalRef global) {
  return ((Global*)global)->name.str.data();
}
BinaryenType BinaryenGlobalGetType(BinaryenGlobalRef global) {
  return ((Global*)global)->type.getID();
}
bool BinaryenGlobalIsMutable(BinaryenGlobalRef global) {
  return ((Global*)global)->mutable_;
}
BinaryenExpressionRef BinaryenGlobalGetInitExpr(BinaryenGlobalRef global) {
  return ((Global*)global)->init;
}

//
// =========== Tag operations ===========
//

const char* BinaryenTagGetName(BinaryenTagRef tag) {
  return ((Tag*)tag)->name.str.data();
}
BinaryenType BinaryenTagGetParams(BinaryenTagRef tag) {
  return ((Tag*)tag)->sig.params.getID();
}

BinaryenType BinaryenTagGetResults(BinaryenTagRef tag) {
  return ((Tag*)tag)->sig.results.getID();
}

//
// =========== Import operations ===========
//

const char* BinaryenFunctionImportGetModule(BinaryenFunctionRef import) {
  auto* func = (Function*)import;
  if (func->imported()) {
    return func->module.str.data();
  } else {
    return "";
  }
}
const char* BinaryenTableImportGetModule(BinaryenTableRef import) {
  auto* table = (Table*)import;
  if (table->imported()) {
    return table->module.str.data();
  } else {
    return "";
  }
}
const char* BinaryenGlobalImportGetModule(BinaryenGlobalRef import) {
  auto* global = (Global*)import;
  if (global->imported()) {
    return global->module.str.data();
  } else {
    return "";
  }
}
const char* BinaryenTagImportGetModule(BinaryenTagRef import) {
  auto* tag = (Tag*)import;
  if (tag->imported()) {
    return tag->module.str.data();
  } else {
    return "";
  }
}
const char* BinaryenFunctionImportGetBase(BinaryenFunctionRef import) {
  auto* func = (Function*)import;
  if (func->imported()) {
    return func->base.str.data();
  } else {
    return "";
  }
}
void BinaryenFunctionImportToFunction(BinaryenFunctionRef import) {
  auto* func = (Function*)import;
  if (func->imported()) {
    func->module = Name();
    func->base = Name();
    func->setExplicitName(func->name);
  }
}
const char* BinaryenTableImportGetBase(BinaryenTableRef import) {
  auto* table = (Table*)import;
  if (table->imported()) {
    return table->base.str.data();
  } else {
    return "";
  }
}
const char* BinaryenGlobalImportGetBase(BinaryenGlobalRef import) {
  auto* global = (Global*)import;
  if (global->imported()) {
    return global->base.str.data();
  } else {
    return "";
  }
}
const char* BinaryenTagImportGetBase(BinaryenTagRef import) {
  auto* tag = (Tag*)import;
  if (tag->imported()) {
    return tag->base.str.data();
  } else {
    return "";
  }
}

//
// =========== Export operations ===========
//

BinaryenExternalKind BinaryenExportGetKind(BinaryenExportRef export_) {
  return BinaryenExternalKind(((Export*)export_)->kind);
}
const char* BinaryenExportGetName(BinaryenExportRef export_) {
  return ((Export*)export_)->name.str.data();
}
const char* BinaryenExportGetValue(BinaryenExportRef export_) {
  return ((Export*)export_)->value.str.data();
}

//
// ========= Custom sections =========
//

void BinaryenAddCustomSection(BinaryenModuleRef module,
                              const char* name,
                              const char* contents,
                              BinaryenIndex contentsSize) {
  wasm::CustomSection customSection;
  customSection.name = name;
  customSection.data = std::vector<char>(contents, contents + contentsSize);
  ((Module*)module)->customSections.push_back(customSection);
}

//
// ========= Effect analyzer =========
//

BinaryenSideEffects BinaryenSideEffectNone(void) {
  return static_cast<BinaryenSideEffects>(EffectAnalyzer::SideEffects::None);
}
BinaryenSideEffects BinaryenSideEffectBranches(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::Branches);
}
BinaryenSideEffects BinaryenSideEffectCalls(void) {
  return static_cast<BinaryenSideEffects>(EffectAnalyzer::SideEffects::Calls);
}
BinaryenSideEffects BinaryenSideEffectReadsLocal(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::ReadsLocal);
}
BinaryenSideEffects BinaryenSideEffectWritesLocal(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::WritesLocal);
}
BinaryenSideEffects BinaryenSideEffectReadsGlobal(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::ReadsGlobal);
}
BinaryenSideEffects BinaryenSideEffectWritesGlobal(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::WritesGlobal);
}
BinaryenSideEffects BinaryenSideEffectReadsMemory(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::ReadsMemory);
}
BinaryenSideEffects BinaryenSideEffectWritesMemory(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::WritesMemory);
}
BinaryenSideEffects BinaryenSideEffectReadsTable(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::ReadsTable);
}
BinaryenSideEffects BinaryenSideEffectWritesTable(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::WritesTable);
}
BinaryenSideEffects BinaryenSideEffectImplicitTrap(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::ImplicitTrap);
}
BinaryenSideEffects BinaryenSideEffectTrapsNeverHappen(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::TrapsNeverHappen);
}
BinaryenSideEffects BinaryenSideEffectIsAtomic(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::IsAtomic);
}
BinaryenSideEffects BinaryenSideEffectThrows(void) {
  return static_cast<BinaryenSideEffects>(EffectAnalyzer::SideEffects::Throws);
}
BinaryenSideEffects BinaryenSideEffectDanglingPop(void) {
  return static_cast<BinaryenSideEffects>(
    EffectAnalyzer::SideEffects::DanglingPop);
}
BinaryenSideEffects BinaryenSideEffectAny(void) {
  return static_cast<BinaryenSideEffects>(EffectAnalyzer::SideEffects::Any);
}

BinaryenSideEffects BinaryenExpressionGetSideEffects(BinaryenExpressionRef expr,
                                                     BinaryenModuleRef module) {
  return EffectAnalyzer(globalPassOptions, *(Module*)module, (Expression*)expr)
    .getSideEffects();
}

//
// ========== CFG / Relooper ==========
//

RelooperRef RelooperCreate(BinaryenModuleRef module) {
  return RelooperRef(new CFG::Relooper((Module*)module));
}

RelooperBlockRef RelooperAddBlock(RelooperRef relooper,
                                  BinaryenExpressionRef code) {
  return RelooperBlockRef(
    ((CFG::Relooper*)relooper)->AddBlock((Expression*)code));
}

void RelooperAddBranch(RelooperBlockRef from,
                       RelooperBlockRef to,
                       BinaryenExpressionRef condition,
                       BinaryenExpressionRef code) {
  ((CFG::Block*)from)
    ->AddBranchTo((CFG::Block*)to, (Expression*)condition, (Expression*)code);
}

RelooperBlockRef RelooperAddBlockWithSwitch(RelooperRef relooper,
                                            BinaryenExpressionRef code,
                                            BinaryenExpressionRef condition) {
  return RelooperBlockRef(
    ((CFG::Relooper*)relooper)
      ->AddBlock((Expression*)code, (Expression*)condition));
}

void RelooperAddBranchForSwitch(RelooperBlockRef from,
                                RelooperBlockRef to,
                                BinaryenIndex* indexes,
                                BinaryenIndex numIndexes,
                                BinaryenExpressionRef code) {
  std::vector<Index> values;
  for (Index i = 0; i < numIndexes; i++) {
    values.push_back(indexes[i]);
  }
  ((CFG::Block*)from)
    ->AddSwitchBranchTo((CFG::Block*)to, std::move(values), (Expression*)code);
}

BinaryenExpressionRef RelooperRenderAndDispose(RelooperRef relooper,
                                               RelooperBlockRef entry,
                                               BinaryenIndex labelHelper) {
  auto* R = (CFG::Relooper*)relooper;
  R->Calculate((CFG::Block*)entry);
  CFG::RelooperBuilder builder(*R->Module, labelHelper);
  auto* ret = R->Render(builder);
  delete R;
  return BinaryenExpressionRef(ret);
}

//
// ========= ExpressionRunner =========
//

namespace wasm {

// Evaluates a suspected constant expression via the C-API. Inherits most of its
// functionality from ConstantExpressionRunner, which it shares with the
// precompute pass, but must be `final` so we can `delete` its instances.
class CExpressionRunner final
  : public ConstantExpressionRunner<CExpressionRunner> {
public:
  CExpressionRunner(Module* module,
                    CExpressionRunner::Flags flags,
                    Index maxDepth,
                    Index maxLoopIterations)
    : ConstantExpressionRunner<CExpressionRunner>(
        module, flags, maxDepth, maxLoopIterations) {}
};

} // namespace wasm

ExpressionRunnerFlags ExpressionRunnerFlagsDefault() {
  return CExpressionRunner::FlagValues::DEFAULT;
}

ExpressionRunnerFlags ExpressionRunnerFlagsPreserveSideeffects() {
  return CExpressionRunner::FlagValues::PRESERVE_SIDEEFFECTS;
}

ExpressionRunnerFlags ExpressionRunnerFlagsTraverseCalls() {
  return CExpressionRunner::FlagValues::TRAVERSE_CALLS;
}

ExpressionRunnerRef ExpressionRunnerCreate(BinaryenModuleRef module,
                                           ExpressionRunnerFlags flags,
                                           BinaryenIndex maxDepth,
                                           BinaryenIndex maxLoopIterations) {
  return static_cast<ExpressionRunnerRef>(
    new CExpressionRunner((Module*)module, flags, maxDepth, maxLoopIterations));
}

bool ExpressionRunnerSetLocalValue(ExpressionRunnerRef runner,
                                   BinaryenIndex index,
                                   BinaryenExpressionRef value) {
  auto* R = (CExpressionRunner*)runner;
  auto setFlow = R->visit(value);
  if (!setFlow.breaking()) {
    R->setLocalValue(index, setFlow.values);
    return 1;
  }
  return 0;
}

bool ExpressionRunnerSetGlobalValue(ExpressionRunnerRef runner,
                                    const char* name,
                                    BinaryenExpressionRef value) {
  auto* R = (CExpressionRunner*)runner;
  auto setFlow = R->visit(value);
  if (!setFlow.breaking()) {
    R->setGlobalValue(name, setFlow.values);
    return 1;
  }
  return 0;
}

BinaryenExpressionRef
ExpressionRunnerRunAndDispose(ExpressionRunnerRef runner,
                              BinaryenExpressionRef expr) {
  auto* R = (CExpressionRunner*)runner;
  Expression* ret = nullptr;
  try {
    auto flow = R->visit(expr);
    if (!flow.breaking() && !flow.values.empty()) {
      ret = flow.getConstExpression(*R->getModule());
    }
  } catch (CExpressionRunner::NonconstantException&) {
  }
  delete R;
  return ret;
}

//
// ========= Type builder =========
//

TypeBuilderErrorReason TypeBuilderErrorReasonSelfSupertype() {
  return static_cast<TypeBuilderErrorReason>(
    TypeBuilder::ErrorReason::SelfSupertype);
}
TypeBuilderErrorReason TypeBuilderErrorReasonInvalidSupertype() {
  return static_cast<TypeBuilderErrorReason>(
    TypeBuilder::ErrorReason::InvalidSupertype);
}
TypeBuilderErrorReason TypeBuilderErrorReasonForwardSupertypeReference() {
  return static_cast<TypeBuilderErrorReason>(
    TypeBuilder::ErrorReason::ForwardSupertypeReference);
}
TypeBuilderErrorReason TypeBuilderErrorReasonForwardChildReference() {
  return static_cast<TypeBuilderErrorReason>(
    TypeBuilder::ErrorReason::ForwardChildReference);
}

TypeBuilderRef TypeBuilderCreate(BinaryenIndex size) {
  return static_cast<TypeBuilderRef>(new TypeBuilder(size));
}
void TypeBuilderGrow(TypeBuilderRef builder, BinaryenIndex count) {
  ((TypeBuilder*)builder)->grow(count);
}
BinaryenIndex TypeBuilderGetSize(TypeBuilderRef builder) {
  return ((TypeBuilder*)builder)->size();
}
void TypeBuilderSetBasicHeapType(TypeBuilderRef builder,
                                 BinaryenIndex index,
                                 BinaryenBasicHeapType basicHeapType) {
  ((TypeBuilder*)builder)
    ->setHeapType(index, HeapType::BasicHeapType(basicHeapType));
}
void TypeBuilderSetSignatureType(TypeBuilderRef builder,
                                 BinaryenIndex index,
                                 BinaryenType paramTypes,
                                 BinaryenType resultTypes) {
  ((TypeBuilder*)builder)
    ->setHeapType(index, Signature(Type(paramTypes), Type(resultTypes)));
}
void TypeBuilderSetStructType(TypeBuilderRef builder,
                              BinaryenIndex index,
                              BinaryenType* fieldTypes,
                              BinaryenPackedType* fieldPackedTypes,
                              bool* fieldMutables,
                              int numFields) {
  auto* B = (TypeBuilder*)builder;
  FieldList fields;
  for (int cur = 0; cur < numFields; ++cur) {
    Field field(Type(fieldTypes[cur]),
                fieldMutables[cur] ? Mutability::Mutable
                                   : Mutability::Immutable);
    if (field.type == Type::i32) {
      field.packedType = Field::PackedType(fieldPackedTypes[cur]);
    } else {
      assert(fieldPackedTypes[cur] == Field::PackedType::not_packed);
    }
    fields.push_back(field);
  }
  B->setHeapType(index, Struct(fields));
}
void TypeBuilderSetArrayType(TypeBuilderRef builder,
                             BinaryenIndex index,
                             BinaryenType elementType,
                             BinaryenPackedType elementPackedType,
                             int elementMutable) {
  auto* B = (TypeBuilder*)builder;
  Field element(Type(elementType),
                elementMutable ? Mutability::Mutable : Mutability::Immutable);
  if (element.type == Type::i32) {
    element.packedType = Field::PackedType(elementPackedType);
  } else {
    assert(elementPackedType == Field::PackedType::not_packed);
  }
  B->setHeapType(index, Array(element));
}
bool TypeBuilderIsBasic(TypeBuilderRef builder, BinaryenIndex index) {
  return ((TypeBuilder*)builder)->isBasic(index);
}
BinaryenBasicHeapType TypeBuilderGetBasic(TypeBuilderRef builder,
                                          BinaryenIndex index) {
  return BinaryenBasicHeapType(((TypeBuilder*)builder)->getBasic(index));
}
BinaryenHeapType TypeBuilderGetTempHeapType(TypeBuilderRef builder,
                                            BinaryenIndex index) {
  return ((TypeBuilder*)builder)->getTempHeapType(index).getID();
}
BinaryenType TypeBuilderGetTempTupleType(TypeBuilderRef builder,
                                         BinaryenType* types,
                                         BinaryenIndex numTypes) {
  TypeList typeList(numTypes);
  for (BinaryenIndex cur = 0; cur < numTypes; ++cur) {
    typeList[cur] = Type(types[cur]);
  }
  return ((TypeBuilder*)builder)->getTempTupleType(Tuple(typeList)).getID();
}
BinaryenType TypeBuilderGetTempRefType(TypeBuilderRef builder,
                                       BinaryenHeapType heapType,
                                       int nullable) {
  return ((TypeBuilder*)builder)
    ->getTempRefType(HeapType(heapType), nullable ? Nullable : NonNullable)
    .getID();
}
void TypeBuilderSetSubType(TypeBuilderRef builder,
                           BinaryenIndex index,
                           BinaryenHeapType superType) {
  ((TypeBuilder*)builder)->setSubType(index, HeapType(superType));
}
void TypeBuilderCreateRecGroup(TypeBuilderRef builder,
                               BinaryenIndex index,
                               BinaryenIndex length) {
  ((TypeBuilder*)builder)->createRecGroup(index, length);
}
bool TypeBuilderBuildAndDispose(TypeBuilderRef builder,
                                BinaryenHeapType* heapTypes,
                                BinaryenIndex* errorIndex,
                                TypeBuilderErrorReason* errorReason) {
  auto* B = (TypeBuilder*)builder;
  auto result = B->build();
  if (auto err = result.getError()) {
    *errorIndex = err->index;
    *errorReason = static_cast<TypeBuilderErrorReason>(err->reason);
    delete B;
    return false;
  }
  auto types = *result;
  for (size_t cur = 0; cur < types.size(); ++cur) {
    heapTypes[cur] = types[cur].getID();
  }
  delete B;
  return true;
}

void BinaryenModuleSetTypeName(BinaryenModuleRef module,
                               BinaryenHeapType heapType,
                               const char* name) {
  ((Module*)module)->typeNames[HeapType(heapType)].name = name;
}
void BinaryenModuleSetFieldName(BinaryenModuleRef module,
                                BinaryenHeapType heapType,
                                BinaryenIndex index,
                                const char* name) {
  ((Module*)module)->typeNames[HeapType(heapType)].fieldNames[index] = name;
}

//
// ========= Utilities =========
//

void BinaryenSetColorsEnabled(bool enabled) { Colors::setEnabled(enabled); }

bool BinaryenAreColorsEnabled() { return Colors::isEnabled(); }

#ifdef __EMSCRIPTEN__
// Internal binaryen.js APIs

// Returns the size of a Literal object.
EMSCRIPTEN_KEEPALIVE
size_t BinaryenSizeofLiteral(void) { return sizeof(Literal); }

// Returns the size of an allocate and write result object.
EMSCRIPTEN_KEEPALIVE
size_t BinaryenSizeofAllocateAndWriteResult(void) {
  return sizeof(BinaryenModuleAllocateAndWriteResult);
}

// Helpers for accessing Binaryen's memory from another module without the
// need to round-trip through JS, e.g. when allocating and initializing
// strings passed to / reading strings returned by the C-API.

// TODO: Remove these once Wasm supports multiple memories.

// Stores an 8-bit integer to Binaryen memory.
EMSCRIPTEN_KEEPALIVE
void _i32_store8(int8_t* ptr, int8_t value) { *ptr = value; }

// Stores a 16-bit integer to Binaryen memory.
EMSCRIPTEN_KEEPALIVE
void _i32_store16(int16_t* ptr, int16_t value) { *ptr = value; }

// Stores a 32-bit integer to Binaryen memory.
EMSCRIPTEN_KEEPALIVE
void _i32_store(int32_t* ptr, int32_t value) { *ptr = value; }

// Stores a 32-bit float to Binaryen memory.
EMSCRIPTEN_KEEPALIVE
void _f32_store(float* ptr, float value) { *ptr = value; }

// Stores a 64-bit float to Binaryen memory.
EMSCRIPTEN_KEEPALIVE
void _f64_store(double* ptr, double value) { *ptr = value; }

// Loads an 8-bit signed integer from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
int8_t _i32_load8_s(int8_t* ptr) { return *ptr; }

// Loads an 8-bit unsigned integer from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
uint8_t _i32_load8_u(uint8_t* ptr) { return *ptr; }

// Loads a 16-bit signed integer from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
int16_t _i32_load16_s(int16_t* ptr) { return *ptr; }

// Loads a 16-bit unsigned integer from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
uint16_t _i32_load16_u(uint16_t* ptr) { return *ptr; }

// Loads a 32-bit integer from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
int32_t _i32_load(int32_t* ptr) { return *ptr; }

// Loads a 32-bit float from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
float _f32_load(float* ptr) { return *ptr; }

// Loads a 64-bit float from Binaryen memory.
EMSCRIPTEN_KEEPALIVE
double _f64_load(double* ptr) { return *ptr; }

#endif // __EMSCRIPTEN__

} // extern "C"
