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

//================
// Binaryen C API
//
// The first part of the API lets you create modules and their parts.
//
// The second part of the API lets you perform operations on modules.
//
// The third part of the API lets you provide a general control-flow
//   graph (CFG) as input.
//
// The final part of the API contains miscellaneous utilities like
//   debugging for the API itself.
//
// ---------------
//
// Thread safety: You can create Expressions in parallel, as they do not
//                refer to global state. BinaryenAddFunction is also
//                thread-safe, which means that you can create functions and
//                their contents in multiple threads. This is important since
//                functions are where the majority of the work is done.
//                Other methods - creating imports, exports, etc. - are
//                not currently thread-safe (as there is typically no need
//                to parallelize them).
//
//================

#ifndef wasm_binaryen_c_h
#define wasm_binaryen_c_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __GNUC__
#define WASM_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define WASM_DEPRECATED __declspec(deprecated)
#else
#define WASM_DEPRECATED
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#define BINARYEN_API EMSCRIPTEN_KEEPALIVE
#elif defined(_MSC_VER) && !defined(BUILD_STATIC_LIBRARY)
#define BINARYEN_API __declspec(dllexport)
#else
#define BINARYEN_API
#endif

#ifdef __cplusplus
#define BINARYEN_REF(NAME)                                                     \
  namespace wasm {                                                             \
  class NAME;                                                                  \
  };                                                                           \
  typedef class wasm::NAME* Binaryen##NAME##Ref;
#else
#define BINARYEN_REF(NAME) typedef struct Binaryen##NAME* Binaryen##NAME##Ref;
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// ========== Module Creation ==========
//

// BinaryenIndex
//
// Used for internal indexes and list sizes.

typedef uint32_t BinaryenIndex;

// Core types (call to get the value of each; you can cache them, they
// never change)

typedef uintptr_t BinaryenType;

BINARYEN_API BinaryenType BinaryenTypeNone(void);
BINARYEN_API BinaryenType BinaryenTypeInt32(void);
BINARYEN_API BinaryenType BinaryenTypeInt64(void);
BINARYEN_API BinaryenType BinaryenTypeFloat32(void);
BINARYEN_API BinaryenType BinaryenTypeFloat64(void);
BINARYEN_API BinaryenType BinaryenTypeVec128(void);
BINARYEN_API BinaryenType BinaryenTypeFuncref(void);
BINARYEN_API BinaryenType BinaryenTypeExternref(void);
BINARYEN_API BinaryenType BinaryenTypeAnyref(void);
BINARYEN_API BinaryenType BinaryenTypeEqref(void);
BINARYEN_API BinaryenType BinaryenTypeI31ref(void);
BINARYEN_API BinaryenType BinaryenTypeDataref(void);
BINARYEN_API BinaryenType BinaryenTypeUnreachable(void);
// Not a real type. Used as the last parameter to BinaryenBlock to let
// the API figure out the type instead of providing one.
BINARYEN_API BinaryenType BinaryenTypeAuto(void);
BINARYEN_API BinaryenType BinaryenTypeCreate(BinaryenType* valueTypes,
                                             uint32_t numTypes);
BINARYEN_API uint32_t BinaryenTypeArity(BinaryenType t);
BINARYEN_API void BinaryenTypeExpand(BinaryenType t, BinaryenType* buf);

WASM_DEPRECATED BinaryenType BinaryenNone(void);
WASM_DEPRECATED BinaryenType BinaryenInt32(void);
WASM_DEPRECATED BinaryenType BinaryenInt64(void);
WASM_DEPRECATED BinaryenType BinaryenFloat32(void);
WASM_DEPRECATED BinaryenType BinaryenFloat64(void);
WASM_DEPRECATED BinaryenType BinaryenUndefined(void);

// Expression ids (call to get the value of each; you can cache them)

typedef uint32_t BinaryenExpressionId;

BINARYEN_API BinaryenExpressionId BinaryenInvalidId(void);

#define DELEGATE(CLASS_TO_VISIT)                                               \
  BINARYEN_API BinaryenExpressionId Binaryen##CLASS_TO_VISIT##Id(void);

#include "wasm-delegations.h"

// External kinds (call to get the value of each; you can cache them)

typedef uint32_t BinaryenExternalKind;

BINARYEN_API BinaryenExternalKind BinaryenExternalFunction(void);
BINARYEN_API BinaryenExternalKind BinaryenExternalTable(void);
BINARYEN_API BinaryenExternalKind BinaryenExternalMemory(void);
BINARYEN_API BinaryenExternalKind BinaryenExternalGlobal(void);
BINARYEN_API BinaryenExternalKind BinaryenExternalEvent(void);

// Features. Call to get the value of each; you can cache them. Use bitwise
// operators to combine and test particular features.

typedef uint32_t BinaryenFeatures;

BINARYEN_API BinaryenFeatures BinaryenFeatureMVP(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureAtomics(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureBulkMemory(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureMutableGlobals(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureNontrappingFPToInt(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureSignExt(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureSIMD128(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureExceptionHandling(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureTailCall(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureReferenceTypes(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureMultivalue(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureGC(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureMemory64(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureTypedFunctionReferences(void);
BINARYEN_API BinaryenFeatures BinaryenFeatureAll(void);

// Modules
//
// Modules contain lists of functions, imports, exports, function types. The
// Add* methods create them on a module. The module owns them and will free
// their memory when the module is disposed of.
//
// Expressions are also allocated inside modules, and freed with the module.
// They are not created by Add* methods, since they are not added directly on
// the module, instead, they are arguments to other expressions (and then they
// are the children of that AST node), or to a function (and then they are the
// body of that function).
//
// A module can also contain a function table for indirect calls, a memory,
// and a start method.

BINARYEN_REF(Module);

BINARYEN_API BinaryenModuleRef BinaryenModuleCreate(void);
BINARYEN_API void BinaryenModuleDispose(BinaryenModuleRef module);

// Literals. These are passed by value.

struct BinaryenLiteral {
  uintptr_t type;
  union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    uint8_t v128[16];
    const char* func;
  };
};

BINARYEN_API struct BinaryenLiteral BinaryenLiteralInt32(int32_t x);
BINARYEN_API struct BinaryenLiteral BinaryenLiteralInt64(int64_t x);
BINARYEN_API struct BinaryenLiteral BinaryenLiteralFloat32(float x);
BINARYEN_API struct BinaryenLiteral BinaryenLiteralFloat64(double x);
BINARYEN_API struct BinaryenLiteral BinaryenLiteralVec128(const uint8_t x[16]);
BINARYEN_API struct BinaryenLiteral BinaryenLiteralFloat32Bits(int32_t x);
BINARYEN_API struct BinaryenLiteral BinaryenLiteralFloat64Bits(int64_t x);

// Expressions
//
// Some expressions have a BinaryenOp, which is the more
// specific operation/opcode.
//
// Some expressions have optional parameters, like Return may not
// return a value. You can supply a NULL pointer in those cases.
//
// For more information, see wasm.h

typedef int32_t BinaryenOp;

BINARYEN_API BinaryenOp BinaryenClzInt32(void);
BINARYEN_API BinaryenOp BinaryenCtzInt32(void);
BINARYEN_API BinaryenOp BinaryenPopcntInt32(void);
BINARYEN_API BinaryenOp BinaryenNegFloat32(void);
BINARYEN_API BinaryenOp BinaryenAbsFloat32(void);
BINARYEN_API BinaryenOp BinaryenCeilFloat32(void);
BINARYEN_API BinaryenOp BinaryenFloorFloat32(void);
BINARYEN_API BinaryenOp BinaryenTruncFloat32(void);
BINARYEN_API BinaryenOp BinaryenNearestFloat32(void);
BINARYEN_API BinaryenOp BinaryenSqrtFloat32(void);
BINARYEN_API BinaryenOp BinaryenEqZInt32(void);
BINARYEN_API BinaryenOp BinaryenClzInt64(void);
BINARYEN_API BinaryenOp BinaryenCtzInt64(void);
BINARYEN_API BinaryenOp BinaryenPopcntInt64(void);
BINARYEN_API BinaryenOp BinaryenNegFloat64(void);
BINARYEN_API BinaryenOp BinaryenAbsFloat64(void);
BINARYEN_API BinaryenOp BinaryenCeilFloat64(void);
BINARYEN_API BinaryenOp BinaryenFloorFloat64(void);
BINARYEN_API BinaryenOp BinaryenTruncFloat64(void);
BINARYEN_API BinaryenOp BinaryenNearestFloat64(void);
BINARYEN_API BinaryenOp BinaryenSqrtFloat64(void);
BINARYEN_API BinaryenOp BinaryenEqZInt64(void);
BINARYEN_API BinaryenOp BinaryenExtendSInt32(void);
BINARYEN_API BinaryenOp BinaryenExtendUInt32(void);
BINARYEN_API BinaryenOp BinaryenWrapInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncSFloat32ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncSFloat32ToInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncUFloat32ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncUFloat32ToInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncSFloat64ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncSFloat64ToInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncUFloat64ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncUFloat64ToInt64(void);
BINARYEN_API BinaryenOp BinaryenReinterpretFloat32(void);
BINARYEN_API BinaryenOp BinaryenReinterpretFloat64(void);
BINARYEN_API BinaryenOp BinaryenConvertSInt32ToFloat32(void);
BINARYEN_API BinaryenOp BinaryenConvertSInt32ToFloat64(void);
BINARYEN_API BinaryenOp BinaryenConvertUInt32ToFloat32(void);
BINARYEN_API BinaryenOp BinaryenConvertUInt32ToFloat64(void);
BINARYEN_API BinaryenOp BinaryenConvertSInt64ToFloat32(void);
BINARYEN_API BinaryenOp BinaryenConvertSInt64ToFloat64(void);
BINARYEN_API BinaryenOp BinaryenConvertUInt64ToFloat32(void);
BINARYEN_API BinaryenOp BinaryenConvertUInt64ToFloat64(void);
BINARYEN_API BinaryenOp BinaryenPromoteFloat32(void);
BINARYEN_API BinaryenOp BinaryenDemoteFloat64(void);
BINARYEN_API BinaryenOp BinaryenReinterpretInt32(void);
BINARYEN_API BinaryenOp BinaryenReinterpretInt64(void);
BINARYEN_API BinaryenOp BinaryenExtendS8Int32(void);
BINARYEN_API BinaryenOp BinaryenExtendS16Int32(void);
BINARYEN_API BinaryenOp BinaryenExtendS8Int64(void);
BINARYEN_API BinaryenOp BinaryenExtendS16Int64(void);
BINARYEN_API BinaryenOp BinaryenExtendS32Int64(void);
BINARYEN_API BinaryenOp BinaryenAddInt32(void);
BINARYEN_API BinaryenOp BinaryenSubInt32(void);
BINARYEN_API BinaryenOp BinaryenMulInt32(void);
BINARYEN_API BinaryenOp BinaryenDivSInt32(void);
BINARYEN_API BinaryenOp BinaryenDivUInt32(void);
BINARYEN_API BinaryenOp BinaryenRemSInt32(void);
BINARYEN_API BinaryenOp BinaryenRemUInt32(void);
BINARYEN_API BinaryenOp BinaryenAndInt32(void);
BINARYEN_API BinaryenOp BinaryenOrInt32(void);
BINARYEN_API BinaryenOp BinaryenXorInt32(void);
BINARYEN_API BinaryenOp BinaryenShlInt32(void);
BINARYEN_API BinaryenOp BinaryenShrUInt32(void);
BINARYEN_API BinaryenOp BinaryenShrSInt32(void);
BINARYEN_API BinaryenOp BinaryenRotLInt32(void);
BINARYEN_API BinaryenOp BinaryenRotRInt32(void);
BINARYEN_API BinaryenOp BinaryenEqInt32(void);
BINARYEN_API BinaryenOp BinaryenNeInt32(void);
BINARYEN_API BinaryenOp BinaryenLtSInt32(void);
BINARYEN_API BinaryenOp BinaryenLtUInt32(void);
BINARYEN_API BinaryenOp BinaryenLeSInt32(void);
BINARYEN_API BinaryenOp BinaryenLeUInt32(void);
BINARYEN_API BinaryenOp BinaryenGtSInt32(void);
BINARYEN_API BinaryenOp BinaryenGtUInt32(void);
BINARYEN_API BinaryenOp BinaryenGeSInt32(void);
BINARYEN_API BinaryenOp BinaryenGeUInt32(void);
BINARYEN_API BinaryenOp BinaryenAddInt64(void);
BINARYEN_API BinaryenOp BinaryenSubInt64(void);
BINARYEN_API BinaryenOp BinaryenMulInt64(void);
BINARYEN_API BinaryenOp BinaryenDivSInt64(void);
BINARYEN_API BinaryenOp BinaryenDivUInt64(void);
BINARYEN_API BinaryenOp BinaryenRemSInt64(void);
BINARYEN_API BinaryenOp BinaryenRemUInt64(void);
BINARYEN_API BinaryenOp BinaryenAndInt64(void);
BINARYEN_API BinaryenOp BinaryenOrInt64(void);
BINARYEN_API BinaryenOp BinaryenXorInt64(void);
BINARYEN_API BinaryenOp BinaryenShlInt64(void);
BINARYEN_API BinaryenOp BinaryenShrUInt64(void);
BINARYEN_API BinaryenOp BinaryenShrSInt64(void);
BINARYEN_API BinaryenOp BinaryenRotLInt64(void);
BINARYEN_API BinaryenOp BinaryenRotRInt64(void);
BINARYEN_API BinaryenOp BinaryenEqInt64(void);
BINARYEN_API BinaryenOp BinaryenNeInt64(void);
BINARYEN_API BinaryenOp BinaryenLtSInt64(void);
BINARYEN_API BinaryenOp BinaryenLtUInt64(void);
BINARYEN_API BinaryenOp BinaryenLeSInt64(void);
BINARYEN_API BinaryenOp BinaryenLeUInt64(void);
BINARYEN_API BinaryenOp BinaryenGtSInt64(void);
BINARYEN_API BinaryenOp BinaryenGtUInt64(void);
BINARYEN_API BinaryenOp BinaryenGeSInt64(void);
BINARYEN_API BinaryenOp BinaryenGeUInt64(void);
BINARYEN_API BinaryenOp BinaryenAddFloat32(void);
BINARYEN_API BinaryenOp BinaryenSubFloat32(void);
BINARYEN_API BinaryenOp BinaryenMulFloat32(void);
BINARYEN_API BinaryenOp BinaryenDivFloat32(void);
BINARYEN_API BinaryenOp BinaryenCopySignFloat32(void);
BINARYEN_API BinaryenOp BinaryenMinFloat32(void);
BINARYEN_API BinaryenOp BinaryenMaxFloat32(void);
BINARYEN_API BinaryenOp BinaryenEqFloat32(void);
BINARYEN_API BinaryenOp BinaryenNeFloat32(void);
BINARYEN_API BinaryenOp BinaryenLtFloat32(void);
BINARYEN_API BinaryenOp BinaryenLeFloat32(void);
BINARYEN_API BinaryenOp BinaryenGtFloat32(void);
BINARYEN_API BinaryenOp BinaryenGeFloat32(void);
BINARYEN_API BinaryenOp BinaryenAddFloat64(void);
BINARYEN_API BinaryenOp BinaryenSubFloat64(void);
BINARYEN_API BinaryenOp BinaryenMulFloat64(void);
BINARYEN_API BinaryenOp BinaryenDivFloat64(void);
BINARYEN_API BinaryenOp BinaryenCopySignFloat64(void);
BINARYEN_API BinaryenOp BinaryenMinFloat64(void);
BINARYEN_API BinaryenOp BinaryenMaxFloat64(void);
BINARYEN_API BinaryenOp BinaryenEqFloat64(void);
BINARYEN_API BinaryenOp BinaryenNeFloat64(void);
BINARYEN_API BinaryenOp BinaryenLtFloat64(void);
BINARYEN_API BinaryenOp BinaryenLeFloat64(void);
BINARYEN_API BinaryenOp BinaryenGtFloat64(void);
BINARYEN_API BinaryenOp BinaryenGeFloat64(void);
BINARYEN_API BinaryenOp BinaryenAtomicRMWAdd(void);
BINARYEN_API BinaryenOp BinaryenAtomicRMWSub(void);
BINARYEN_API BinaryenOp BinaryenAtomicRMWAnd(void);
BINARYEN_API BinaryenOp BinaryenAtomicRMWOr(void);
BINARYEN_API BinaryenOp BinaryenAtomicRMWXor(void);
BINARYEN_API BinaryenOp BinaryenAtomicRMWXchg(void);
BINARYEN_API BinaryenOp BinaryenTruncSatSFloat32ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncSatSFloat32ToInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncSatUFloat32ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncSatUFloat32ToInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncSatSFloat64ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncSatSFloat64ToInt64(void);
BINARYEN_API BinaryenOp BinaryenTruncSatUFloat64ToInt32(void);
BINARYEN_API BinaryenOp BinaryenTruncSatUFloat64ToInt64(void);
BINARYEN_API BinaryenOp BinaryenSplatVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenReplaceLaneVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenSplatVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenReplaceLaneVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenSplatVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenReplaceLaneVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenSplatVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenReplaceLaneVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenSplatVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenReplaceLaneVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenSplatVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenExtractLaneVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenReplaceLaneVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenEqVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenNeVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenLtSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenLtUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenGtSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenGtUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenLeSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenLeUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenGeSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenGeUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenEqVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenNeVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenLtSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenLtUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenGtSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenGtUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenLeSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenLeUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenGeSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenGeUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenEqVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenNeVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenLtSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenLtUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenGtSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenGtUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenLeSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenLeUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenGeSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenGeUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenEqVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenNeVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenLtSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenGtSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenLeSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenGeSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenEqVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenNeVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenLtVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenGtVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenLeVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenGeVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenEqVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenNeVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenLtVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenGtVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenLeVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenGeVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenNotVec128(void);
BINARYEN_API BinaryenOp BinaryenAndVec128(void);
BINARYEN_API BinaryenOp BinaryenOrVec128(void);
BINARYEN_API BinaryenOp BinaryenXorVec128(void);
BINARYEN_API BinaryenOp BinaryenAndNotVec128(void);
BINARYEN_API BinaryenOp BinaryenBitselectVec128(void);
BINARYEN_API BinaryenOp BinaryenAnyTrueVec128(void);
BINARYEN_API BinaryenOp BinaryenPopcntVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAbsVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenNegVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAllTrueVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenBitmaskVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenShlVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenShrSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenShrUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAddVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAddSatSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAddSatUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenSubVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenSubSatSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenSubSatUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenMinSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenMinUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenMaxSVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenMaxUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAvgrUVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenAbsVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenNegVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenAllTrueVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenBitmaskVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenShlVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenShrSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenShrUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenAddVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenAddSatSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenAddSatUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenSubVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenSubSatSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenSubSatUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenMulVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenMinSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenMinUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenMaxSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenMaxUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenAvgrUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenQ15MulrSatSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtMulLowSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtMulHighSVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtMulLowUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtMulHighUVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenAbsVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenNegVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenAllTrueVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenBitmaskVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenShlVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenShrSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenShrUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenAddVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenSubVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenMulVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenMinSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenMinUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenMaxSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenMaxUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenDotSVecI16x8ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtMulLowSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtMulHighSVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtMulLowUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtMulHighUVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenAbsVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenNegVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenAllTrueVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenBitmaskVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenShlVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenShrSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenShrUVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenAddVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenSubVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenMulVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtMulLowSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtMulHighSVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtMulLowUVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtMulHighUVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenAbsVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenNegVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenSqrtVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenAddVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenSubVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenMulVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenDivVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenMinVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenMaxVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenPMinVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenPMaxVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenCeilVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenFloorVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenTruncVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenNearestVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenAbsVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenNegVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenSqrtVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenAddVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenSubVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenMulVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenDivVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenMinVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenMaxVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenPMinVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenPMaxVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenCeilVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenFloorVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenTruncVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenNearestVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenExtAddPairwiseSVecI8x16ToI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtAddPairwiseUVecI8x16ToI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtAddPairwiseSVecI16x8ToI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtAddPairwiseUVecI16x8ToI32x4(void);
BINARYEN_API BinaryenOp BinaryenTruncSatSVecF32x4ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenTruncSatUVecF32x4ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenConvertSVecI32x4ToVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenConvertUVecI32x4ToVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenLoad8SplatVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad16SplatVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad32SplatVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad64SplatVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad8x8SVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad8x8UVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad16x4SVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad16x4UVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad32x2SVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad32x2UVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad32ZeroVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad64ZeroVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad8LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad16LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad32LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenLoad64LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenStore8LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenStore16LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenStore32LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenStore64LaneVec128(void);
BINARYEN_API BinaryenOp BinaryenNarrowSVecI16x8ToVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenNarrowUVecI16x8ToVecI8x16(void);
BINARYEN_API BinaryenOp BinaryenNarrowSVecI32x4ToVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenNarrowUVecI32x4ToVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtendLowSVecI8x16ToVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtendHighSVecI8x16ToVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtendLowUVecI8x16ToVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtendHighUVecI8x16ToVecI16x8(void);
BINARYEN_API BinaryenOp BinaryenExtendLowSVecI16x8ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtendHighSVecI16x8ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtendLowUVecI16x8ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtendHighUVecI16x8ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenExtendLowSVecI32x4ToVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtendHighSVecI32x4ToVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtendLowUVecI32x4ToVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenExtendHighUVecI32x4ToVecI64x2(void);
BINARYEN_API BinaryenOp BinaryenConvertLowSVecI32x4ToVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenConvertLowUVecI32x4ToVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenTruncSatZeroSVecF64x2ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenTruncSatZeroUVecF64x2ToVecI32x4(void);
BINARYEN_API BinaryenOp BinaryenDemoteZeroVecF64x2ToVecF32x4(void);
BINARYEN_API BinaryenOp BinaryenPromoteLowVecF32x4ToVecF64x2(void);
BINARYEN_API BinaryenOp BinaryenSwizzleVec8x16(void);
BINARYEN_API BinaryenOp BinaryenRefIsNull(void);
BINARYEN_API BinaryenOp BinaryenRefIsFunc(void);
BINARYEN_API BinaryenOp BinaryenRefIsData(void);
BINARYEN_API BinaryenOp BinaryenRefIsI31(void);
BINARYEN_API BinaryenOp BinaryenRefAsNonNull(void);
BINARYEN_API BinaryenOp BinaryenRefAsFunc(void);
BINARYEN_API BinaryenOp BinaryenRefAsData(void);
BINARYEN_API BinaryenOp BinaryenRefAsI31(void);

BINARYEN_REF(Expression);

// Block: name can be NULL. Specifying BinaryenUndefined() as the 'type'
//        parameter indicates that the block's type shall be figured out
//        automatically instead of explicitly providing it. This conforms
//        to the behavior before the 'type' parameter has been introduced.
BINARYEN_API BinaryenExpressionRef
BinaryenBlock(BinaryenModuleRef module,
              const char* name,
              BinaryenExpressionRef* children,
              BinaryenIndex numChildren,
              BinaryenType type);
// If: ifFalse can be NULL
BINARYEN_API BinaryenExpressionRef BinaryenIf(BinaryenModuleRef module,
                                              BinaryenExpressionRef condition,
                                              BinaryenExpressionRef ifTrue,
                                              BinaryenExpressionRef ifFalse);
BINARYEN_API BinaryenExpressionRef BinaryenLoop(BinaryenModuleRef module,
                                                const char* in,
                                                BinaryenExpressionRef body);
// Break: value and condition can be NULL
BINARYEN_API BinaryenExpressionRef
BinaryenBreak(BinaryenModuleRef module,
              const char* name,
              BinaryenExpressionRef condition,
              BinaryenExpressionRef value);
// Switch: value can be NULL
BINARYEN_API BinaryenExpressionRef
BinaryenSwitch(BinaryenModuleRef module,
               const char** names,
               BinaryenIndex numNames,
               const char* defaultName,
               BinaryenExpressionRef condition,
               BinaryenExpressionRef value);
// Call: Note the 'returnType' parameter. You must declare the
//       type returned by the function being called, as that
//       function might not have been created yet, so we don't
//       know what it is.
BINARYEN_API BinaryenExpressionRef BinaryenCall(BinaryenModuleRef module,
                                                const char* target,
                                                BinaryenExpressionRef* operands,
                                                BinaryenIndex numOperands,
                                                BinaryenType returnType);
BINARYEN_API BinaryenExpressionRef
BinaryenCallIndirect(BinaryenModuleRef module,
                     const char* table,
                     BinaryenExpressionRef target,
                     BinaryenExpressionRef* operands,
                     BinaryenIndex numOperands,
                     BinaryenType params,
                     BinaryenType results);
BINARYEN_API BinaryenExpressionRef
BinaryenReturnCall(BinaryenModuleRef module,
                   const char* target,
                   BinaryenExpressionRef* operands,
                   BinaryenIndex numOperands,
                   BinaryenType returnType);
BINARYEN_API BinaryenExpressionRef
BinaryenReturnCallIndirect(BinaryenModuleRef module,
                           const char* table,
                           BinaryenExpressionRef target,
                           BinaryenExpressionRef* operands,
                           BinaryenIndex numOperands,
                           BinaryenType params,
                           BinaryenType results);

// LocalGet: Note the 'type' parameter. It might seem redundant, since the
//           local at that index must have a type. However, this API lets you
//           build code "top-down": create a node, then its parents, and so
//           on, and finally create the function at the end. (Note that in fact
//           you do not mention a function when creating ExpressionRefs, only
//           a module.) And since LocalGet is a leaf node, we need to be told
//           its type. (Other nodes detect their type either from their
//           type or their opcode, or failing that, their children. But
//           LocalGet has no children, it is where a "stream" of type info
//           begins.)
//           Note also that the index of a local can refer to a param or
//           a var, that is, either a parameter to the function or a variable
//           declared when you call BinaryenAddFunction. See BinaryenAddFunction
//           for more details.
BINARYEN_API BinaryenExpressionRef BinaryenLocalGet(BinaryenModuleRef module,
                                                    BinaryenIndex index,
                                                    BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenLocalSet(
  BinaryenModuleRef module, BinaryenIndex index, BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef BinaryenLocalTee(BinaryenModuleRef module,
                                                    BinaryenIndex index,
                                                    BinaryenExpressionRef value,
                                                    BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenGlobalGet(BinaryenModuleRef module,
                                                     const char* name,
                                                     BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenGlobalSet(
  BinaryenModuleRef module, const char* name, BinaryenExpressionRef value);
// Load: align can be 0, in which case it will be the natural alignment (equal
// to bytes)
BINARYEN_API BinaryenExpressionRef BinaryenLoad(BinaryenModuleRef module,
                                                uint32_t bytes,
                                                bool signed_,
                                                uint32_t offset,
                                                uint32_t align,
                                                BinaryenType type,
                                                BinaryenExpressionRef ptr);
// Store: align can be 0, in which case it will be the natural alignment (equal
// to bytes)
BINARYEN_API BinaryenExpressionRef BinaryenStore(BinaryenModuleRef module,
                                                 uint32_t bytes,
                                                 uint32_t offset,
                                                 uint32_t align,
                                                 BinaryenExpressionRef ptr,
                                                 BinaryenExpressionRef value,
                                                 BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenConst(BinaryenModuleRef module,
                                                 struct BinaryenLiteral value);
BINARYEN_API BinaryenExpressionRef BinaryenUnary(BinaryenModuleRef module,
                                                 BinaryenOp op,
                                                 BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef BinaryenBinary(BinaryenModuleRef module,
                                                  BinaryenOp op,
                                                  BinaryenExpressionRef left,
                                                  BinaryenExpressionRef right);
BINARYEN_API BinaryenExpressionRef
BinaryenSelect(BinaryenModuleRef module,
               BinaryenExpressionRef condition,
               BinaryenExpressionRef ifTrue,
               BinaryenExpressionRef ifFalse,
               BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenDrop(BinaryenModuleRef module,
                                                BinaryenExpressionRef value);
// Return: value can be NULL
BINARYEN_API BinaryenExpressionRef BinaryenReturn(BinaryenModuleRef module,
                                                  BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef BinaryenMemorySize(BinaryenModuleRef module);
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryGrow(BinaryenModuleRef module, BinaryenExpressionRef delta);
BINARYEN_API BinaryenExpressionRef BinaryenNop(BinaryenModuleRef module);
BINARYEN_API BinaryenExpressionRef
BinaryenUnreachable(BinaryenModuleRef module);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicLoad(BinaryenModuleRef module,
                   uint32_t bytes,
                   uint32_t offset,
                   BinaryenType type,
                   BinaryenExpressionRef ptr);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicStore(BinaryenModuleRef module,
                    uint32_t bytes,
                    uint32_t offset,
                    BinaryenExpressionRef ptr,
                    BinaryenExpressionRef value,
                    BinaryenType type);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicRMW(BinaryenModuleRef module,
                  BinaryenOp op,
                  BinaryenIndex bytes,
                  BinaryenIndex offset,
                  BinaryenExpressionRef ptr,
                  BinaryenExpressionRef value,
                  BinaryenType type);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicCmpxchg(BinaryenModuleRef module,
                      BinaryenIndex bytes,
                      BinaryenIndex offset,
                      BinaryenExpressionRef ptr,
                      BinaryenExpressionRef expected,
                      BinaryenExpressionRef replacement,
                      BinaryenType type);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicWait(BinaryenModuleRef module,
                   BinaryenExpressionRef ptr,
                   BinaryenExpressionRef expected,
                   BinaryenExpressionRef timeout,
                   BinaryenType type);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicNotify(BinaryenModuleRef module,
                     BinaryenExpressionRef ptr,
                     BinaryenExpressionRef notifyCount);
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicFence(BinaryenModuleRef module);
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDExtract(BinaryenModuleRef module,
                    BinaryenOp op,
                    BinaryenExpressionRef vec,
                    uint8_t index);
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDReplace(BinaryenModuleRef module,
                    BinaryenOp op,
                    BinaryenExpressionRef vec,
                    uint8_t index,
                    BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDShuffle(BinaryenModuleRef module,
                    BinaryenExpressionRef left,
                    BinaryenExpressionRef right,
                    const uint8_t mask[16]);
BINARYEN_API BinaryenExpressionRef BinaryenSIMDTernary(BinaryenModuleRef module,
                                                       BinaryenOp op,
                                                       BinaryenExpressionRef a,
                                                       BinaryenExpressionRef b,
                                                       BinaryenExpressionRef c);
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDShift(BinaryenModuleRef module,
                  BinaryenOp op,
                  BinaryenExpressionRef vec,
                  BinaryenExpressionRef shift);
BINARYEN_API BinaryenExpressionRef BinaryenSIMDLoad(BinaryenModuleRef module,
                                                    BinaryenOp op,
                                                    uint32_t offset,
                                                    uint32_t align,
                                                    BinaryenExpressionRef ptr);
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDLoadStoreLane(BinaryenModuleRef module,
                          BinaryenOp op,
                          uint32_t offset,
                          uint32_t align,
                          uint8_t index,
                          BinaryenExpressionRef ptr,
                          BinaryenExpressionRef vec);
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryInit(BinaryenModuleRef module,
                   uint32_t segment,
                   BinaryenExpressionRef dest,
                   BinaryenExpressionRef offset,
                   BinaryenExpressionRef size);
BINARYEN_API BinaryenExpressionRef BinaryenDataDrop(BinaryenModuleRef module,
                                                    uint32_t segment);
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryCopy(BinaryenModuleRef module,
                   BinaryenExpressionRef dest,
                   BinaryenExpressionRef source,
                   BinaryenExpressionRef size);
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryFill(BinaryenModuleRef module,
                   BinaryenExpressionRef dest,
                   BinaryenExpressionRef value,
                   BinaryenExpressionRef size);
BINARYEN_API BinaryenExpressionRef
BinaryenTableGet(BinaryenModuleRef module,
                 const char* table,
                 BinaryenExpressionRef offset,
                 BinaryenType type);
BINARYEN_API BinaryenExpressionRef
BinaryenTableSet(BinaryenModuleRef module,
                 const char* table,
                 BinaryenExpressionRef value,
                 BinaryenExpressionRef offset);
BINARYEN_API BinaryenExpressionRef BinaryenTableSize(BinaryenModuleRef module,
                                                     const char* table);
BINARYEN_API BinaryenExpressionRef
BinaryenTableGrow(BinaryenModuleRef module,
                  const char* table,
                  BinaryenExpressionRef delta,
                  BinaryenExpressionRef initialValue);
BINARYEN_API BinaryenExpressionRef
BinaryenTableFill(BinaryenModuleRef module,
                  const char* table,
                  BinaryenExpressionRef size,
                  BinaryenExpressionRef value,
                  BinaryenExpressionRef dest);
BINARYEN_API BinaryenExpressionRef
BinaryenTableCopy(BinaryenModuleRef module,
                  const char* srcTable,
                  const char* destTable,
                  BinaryenExpressionRef size,
                  BinaryenExpressionRef srcOffset,
                  BinaryenExpressionRef destOffset);
BINARYEN_API BinaryenExpressionRef
BinaryenTableInit(BinaryenModuleRef module,
                  const char* table,
                  const char* segment,
                  BinaryenExpressionRef size,
                  BinaryenExpressionRef srcOffset,
                  BinaryenExpressionRef destOffset);
BINARYEN_API BinaryenExpressionRef BinaryenElemDrop(BinaryenModuleRef module,
                                                    const char* segment);
BINARYEN_API BinaryenExpressionRef BinaryenRefNull(BinaryenModuleRef module,
                                                   BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenRefIs(BinaryenModuleRef module,
                                                 BinaryenOp op,
                                                 BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef BinaryenRefAs(BinaryenModuleRef module,
                                                 BinaryenOp op,
                                                 BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef BinaryenRefFunc(BinaryenModuleRef module,
                                                   const char* func,
                                                   BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenRefEq(BinaryenModuleRef module,
                                                 BinaryenExpressionRef left,
                                                 BinaryenExpressionRef right);
// Try: name can be NULL. delegateTarget should be NULL in try-catch.
BINARYEN_API BinaryenExpressionRef
BinaryenTry(BinaryenModuleRef module,
            const char* name,
            BinaryenExpressionRef body,
            const char** catchEvents,
            BinaryenIndex numCatchEvents,
            BinaryenExpressionRef* catchBodies,
            BinaryenIndex numCatchBodies,
            const char* delegateTarget);
BINARYEN_API BinaryenExpressionRef
BinaryenThrow(BinaryenModuleRef module,
              const char* event,
              BinaryenExpressionRef* operands,
              BinaryenIndex numOperands);
BINARYEN_API BinaryenExpressionRef BinaryenRethrow(BinaryenModuleRef module,
                                                   const char* target);
BINARYEN_API BinaryenExpressionRef
BinaryenTupleMake(BinaryenModuleRef module,
                  BinaryenExpressionRef* operands,
                  BinaryenIndex numOperands);
BINARYEN_API BinaryenExpressionRef BinaryenTupleExtract(
  BinaryenModuleRef module, BinaryenExpressionRef tuple, BinaryenIndex index);
BINARYEN_API BinaryenExpressionRef BinaryenPop(BinaryenModuleRef module,
                                               BinaryenType type);
BINARYEN_API BinaryenExpressionRef BinaryenI31New(BinaryenModuleRef module,
                                                  BinaryenExpressionRef value);
BINARYEN_API BinaryenExpressionRef BinaryenI31Get(BinaryenModuleRef module,
                                                  BinaryenExpressionRef i31,
                                                  bool signed_);
// TODO (gc): ref.test
// TODO (gc): ref.cast
// TODO (gc): br_on_cast
// TODO (gc): rtt.canon
// TODO (gc): rtt.sub
// TODO (gc): struct.new
// TODO (gc): struct.get
// TODO (gc): struct.set
// TODO (gc): array.new
// TODO (gc): array.get
// TODO (gc): array.set
// TODO (gc): array.len

// Expression

// Gets the id (kind) of the given expression.
BINARYEN_API BinaryenExpressionId
BinaryenExpressionGetId(BinaryenExpressionRef expr);
// Gets the type of the given expression.
BINARYEN_API BinaryenType BinaryenExpressionGetType(BinaryenExpressionRef expr);
// Sets the type of the given expression.
BINARYEN_API void BinaryenExpressionSetType(BinaryenExpressionRef expr,
                                            BinaryenType type);
// Prints text format of the given expression to stdout.
BINARYEN_API void BinaryenExpressionPrint(BinaryenExpressionRef expr);
// Re-finalizes an expression after it has been modified.
BINARYEN_API void BinaryenExpressionFinalize(BinaryenExpressionRef expr);
// Makes a deep copy of the given expression.
BINARYEN_API BinaryenExpressionRef
BinaryenExpressionCopy(BinaryenExpressionRef expr, BinaryenModuleRef module);

// Block

// Gets the name (label) of a `block` expression.
BINARYEN_API const char* BinaryenBlockGetName(BinaryenExpressionRef expr);
// Sets the name (label) of a `block` expression.
BINARYEN_API void BinaryenBlockSetName(BinaryenExpressionRef expr,
                                       const char* name);
// Gets the number of child expressions of a `block` expression.
BINARYEN_API BinaryenIndex
BinaryenBlockGetNumChildren(BinaryenExpressionRef expr);
// Gets the child expression at the specified index of a `block` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenBlockGetChildAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Sets (replaces) the child expression at the specified index of a `block`
// expression.
BINARYEN_API void BinaryenBlockSetChildAt(BinaryenExpressionRef expr,
                                          BinaryenIndex index,
                                          BinaryenExpressionRef childExpr);
// Appends a child expression to a `block` expression, returning its insertion
// index.
BINARYEN_API BinaryenIndex BinaryenBlockAppendChild(
  BinaryenExpressionRef expr, BinaryenExpressionRef childExpr);
// Inserts a child expression at the specified index of a `block` expression,
// moving existing children including the one previously at that index one index
// up.
BINARYEN_API void BinaryenBlockInsertChildAt(BinaryenExpressionRef expr,
                                             BinaryenIndex index,
                                             BinaryenExpressionRef childExpr);
// Removes the child expression at the specified index of a `block` expression,
// moving all subsequent children one index down. Returns the child expression.
BINARYEN_API BinaryenExpressionRef
BinaryenBlockRemoveChildAt(BinaryenExpressionRef expr, BinaryenIndex index);

// If

// Gets the condition expression of an `if` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenIfGetCondition(BinaryenExpressionRef expr);
// Sets the condition expression of an `if` expression.
BINARYEN_API void BinaryenIfSetCondition(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef condExpr);
// Gets the ifTrue (then) expression of an `if` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenIfGetIfTrue(BinaryenExpressionRef expr);
// Sets the ifTrue (then) expression of an `if` expression.
BINARYEN_API void BinaryenIfSetIfTrue(BinaryenExpressionRef expr,
                                      BinaryenExpressionRef ifTrueExpr);
// Gets the ifFalse (else) expression, if any, of an `if` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenIfGetIfFalse(BinaryenExpressionRef expr);
// Sets the ifFalse (else) expression, if any, of an `if` expression.
BINARYEN_API void BinaryenIfSetIfFalse(BinaryenExpressionRef expr,
                                       BinaryenExpressionRef ifFalseExpr);

// Loop

// Gets the name (label) of a `loop` expression.
BINARYEN_API const char* BinaryenLoopGetName(BinaryenExpressionRef expr);
// Sets the name (label) of a `loop` expression.
BINARYEN_API void BinaryenLoopSetName(BinaryenExpressionRef expr,
                                      const char* name);
// Gets the body expression of a `loop` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenLoopGetBody(BinaryenExpressionRef expr);
// Sets the body expression of a `loop` expression.
BINARYEN_API void BinaryenLoopSetBody(BinaryenExpressionRef expr,
                                      BinaryenExpressionRef bodyExpr);

// Break

// Gets the name (target label) of a `br` or `br_if` expression.
BINARYEN_API const char* BinaryenBreakGetName(BinaryenExpressionRef expr);
// Sets the name (target label) of a `br` or `br_if` expression.
BINARYEN_API void BinaryenBreakSetName(BinaryenExpressionRef expr,
                                       const char* name);
// Gets the condition expression, if any, of a `br_if` expression. No condition
// indicates a `br` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenBreakGetCondition(BinaryenExpressionRef expr);
// Sets the condition expression, if any, of a `br_if` expression. No condition
// makes it a `br` expression.
BINARYEN_API void BinaryenBreakSetCondition(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef condExpr);
// Gets the value expression, if any, of a `br` or `br_if` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenBreakGetValue(BinaryenExpressionRef expr);
// Sets the value expression, if any, of a `br` or `br_if` expression.
BINARYEN_API void BinaryenBreakSetValue(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef valueExpr);

// Switch

// Gets the number of names (target labels) of a `br_table` expression.
BINARYEN_API BinaryenIndex
BinaryenSwitchGetNumNames(BinaryenExpressionRef expr);
// Gets the name (target label) at the specified index of a `br_table`
// expression.
BINARYEN_API const char* BinaryenSwitchGetNameAt(BinaryenExpressionRef expr,
                                                 BinaryenIndex index);
// Sets the name (target label) at the specified index of a `br_table`
// expression.
BINARYEN_API void BinaryenSwitchSetNameAt(BinaryenExpressionRef expr,
                                          BinaryenIndex index,
                                          const char* name);
// Appends a name to a `br_table` expression, returning its insertion index.
BINARYEN_API BinaryenIndex BinaryenSwitchAppendName(BinaryenExpressionRef expr,
                                                    const char* name);
// Inserts a name at the specified index of a `br_table` expression, moving
// existing names including the one previously at that index one index up.
BINARYEN_API void BinaryenSwitchInsertNameAt(BinaryenExpressionRef expr,
                                             BinaryenIndex index,
                                             const char* name);
// Removes the name at the specified index of a `br_table` expression, moving
// all subsequent names one index down. Returns the name.
BINARYEN_API const char* BinaryenSwitchRemoveNameAt(BinaryenExpressionRef expr,
                                                    BinaryenIndex index);
// Gets the default name (target label), if any, of a `br_table` expression.
BINARYEN_API const char*
BinaryenSwitchGetDefaultName(BinaryenExpressionRef expr);
// Sets the default name (target label), if any, of a `br_table` expression.
BINARYEN_API void BinaryenSwitchSetDefaultName(BinaryenExpressionRef expr,
                                               const char* name);
// Gets the condition expression of a `br_table` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSwitchGetCondition(BinaryenExpressionRef expr);
// Sets the condition expression of a `br_table` expression.
BINARYEN_API void BinaryenSwitchSetCondition(BinaryenExpressionRef expr,
                                             BinaryenExpressionRef condExpr);
// Gets the value expression, if any, of a `br_table` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSwitchGetValue(BinaryenExpressionRef expr);
// Sets the value expression, if any, of a `br_table` expression.
BINARYEN_API void BinaryenSwitchSetValue(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef valueExpr);

// Call

// Gets the target function name of a `call` expression.
BINARYEN_API const char* BinaryenCallGetTarget(BinaryenExpressionRef expr);
// Sets the target function name of a `call` expression.
BINARYEN_API void BinaryenCallSetTarget(BinaryenExpressionRef expr,
                                        const char* target);
// Gets the number of operands of a `call` expression.
BINARYEN_API BinaryenIndex
BinaryenCallGetNumOperands(BinaryenExpressionRef expr);
// Gets the operand expression at the specified index of a `call` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenCallGetOperandAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Sets the operand expression at the specified index of a `call` expression.
BINARYEN_API void BinaryenCallSetOperandAt(BinaryenExpressionRef expr,
                                           BinaryenIndex index,
                                           BinaryenExpressionRef operandExpr);
// Appends an operand expression to a `call` expression, returning its insertion
// index.
BINARYEN_API BinaryenIndex BinaryenCallAppendOperand(
  BinaryenExpressionRef expr, BinaryenExpressionRef operandExpr);
// Inserts an operand expression at the specified index of a `call` expression,
// moving existing operands including the one previously at that index one index
// up.
BINARYEN_API void
BinaryenCallInsertOperandAt(BinaryenExpressionRef expr,
                            BinaryenIndex index,
                            BinaryenExpressionRef operandExpr);
// Removes the operand expression at the specified index of a `call` expression,
// moving all subsequent operands one index down. Returns the operand
// expression.
BINARYEN_API BinaryenExpressionRef
BinaryenCallRemoveOperandAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Gets whether the specified `call` expression is a tail call.
BINARYEN_API bool BinaryenCallIsReturn(BinaryenExpressionRef expr);
// Sets whether the specified `call` expression is a tail call.
BINARYEN_API void BinaryenCallSetReturn(BinaryenExpressionRef expr,
                                        bool isReturn);

// CallIndirect

// Gets the target expression of a `call_indirect` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenCallIndirectGetTarget(BinaryenExpressionRef expr);
// Sets the target expression of a `call_indirect` expression.
BINARYEN_API void
BinaryenCallIndirectSetTarget(BinaryenExpressionRef expr,
                              BinaryenExpressionRef targetExpr);
// Gets the table name of a `call_indirect` expression.
BINARYEN_API const char*
BinaryenCallIndirectGetTable(BinaryenExpressionRef expr);
// Sets the table name of a `call_indirect` expression.
BINARYEN_API void BinaryenCallIndirectSetTable(BinaryenExpressionRef expr,
                                               const char* table);
// Gets the number of operands of a `call_indirect` expression.
BINARYEN_API BinaryenIndex
BinaryenCallIndirectGetNumOperands(BinaryenExpressionRef expr);
// Gets the operand expression at the specified index of a `call_indirect`
// expression.
BINARYEN_API BinaryenExpressionRef BinaryenCallIndirectGetOperandAt(
  BinaryenExpressionRef expr, BinaryenIndex index);
// Sets the operand expression at the specified index of a `call_indirect`
// expression.
BINARYEN_API void
BinaryenCallIndirectSetOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index,
                                 BinaryenExpressionRef operandExpr);
// Appends an operand expression to a `call_indirect` expression, returning its
// insertion index.
BINARYEN_API BinaryenIndex BinaryenCallIndirectAppendOperand(
  BinaryenExpressionRef expr, BinaryenExpressionRef operandExpr);
// Inserts an operand expression at the specified index of a `call_indirect`
// expression, moving existing operands including the one previously at that
// index one index up.
BINARYEN_API void
BinaryenCallIndirectInsertOperandAt(BinaryenExpressionRef expr,
                                    BinaryenIndex index,
                                    BinaryenExpressionRef operandExpr);
// Removes the operand expression at the specified index of a `call_indirect`
// expression, moving all subsequent operands one index down. Returns the
// operand expression.
BINARYEN_API BinaryenExpressionRef BinaryenCallIndirectRemoveOperandAt(
  BinaryenExpressionRef expr, BinaryenIndex index);
// Gets whether the specified `call_indirect` expression is a tail call.
BINARYEN_API bool BinaryenCallIndirectIsReturn(BinaryenExpressionRef expr);
// Sets whether the specified `call_indirect` expression is a tail call.
BINARYEN_API void BinaryenCallIndirectSetReturn(BinaryenExpressionRef expr,
                                                bool isReturn);
// Gets the parameter types of the specified `call_indirect` expression.
BINARYEN_API BinaryenType
BinaryenCallIndirectGetParams(BinaryenExpressionRef expr);
// Sets the parameter types of the specified `call_indirect` expression.
BINARYEN_API void BinaryenCallIndirectSetParams(BinaryenExpressionRef expr,
                                                BinaryenType params);
// Gets the result types of the specified `call_indirect` expression.
BINARYEN_API BinaryenType
BinaryenCallIndirectGetResults(BinaryenExpressionRef expr);
// Sets the result types of the specified `call_indirect` expression.
BINARYEN_API void BinaryenCallIndirectSetResults(BinaryenExpressionRef expr,
                                                 BinaryenType params);

// LocalGet

// Gets the local index of a `local.get` expression.
BINARYEN_API BinaryenIndex BinaryenLocalGetGetIndex(BinaryenExpressionRef expr);
// Sets the local index of a `local.get` expression.
BINARYEN_API void BinaryenLocalGetSetIndex(BinaryenExpressionRef expr,
                                           BinaryenIndex index);

// LocalSet

// Gets whether a `local.set` tees its value (is a `local.tee`). True if the
// expression has a type other than `none`.
BINARYEN_API bool BinaryenLocalSetIsTee(BinaryenExpressionRef expr);
// Gets the local index of a `local.set` or `local.tee` expression.
BINARYEN_API BinaryenIndex BinaryenLocalSetGetIndex(BinaryenExpressionRef expr);
// Sets the local index of a `local.set` or `local.tee` expression.
BINARYEN_API void BinaryenLocalSetSetIndex(BinaryenExpressionRef expr,
                                           BinaryenIndex index);
// Gets the value expression of a `local.set` or `local.tee` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenLocalSetGetValue(BinaryenExpressionRef expr);
// Sets the value expression of a `local.set` or `local.tee` expression.
BINARYEN_API void BinaryenLocalSetSetValue(BinaryenExpressionRef expr,
                                           BinaryenExpressionRef valueExpr);

// GlobalGet

// Gets the name of the global being accessed by a `global.get` expression.
BINARYEN_API const char* BinaryenGlobalGetGetName(BinaryenExpressionRef expr);
// Sets the name of the global being accessed by a `global.get` expression.
BINARYEN_API void BinaryenGlobalGetSetName(BinaryenExpressionRef expr,
                                           const char* name);

// GlobalSet

// Gets the name of the global being accessed by a `global.set` expression.
BINARYEN_API const char* BinaryenGlobalSetGetName(BinaryenExpressionRef expr);
// Sets the name of the global being accessed by a `global.set` expression.
BINARYEN_API void BinaryenGlobalSetSetName(BinaryenExpressionRef expr,
                                           const char* name);
// Gets the value expression of a `global.set` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenGlobalSetGetValue(BinaryenExpressionRef expr);
// Sets the value expression of a `global.set` expression.
BINARYEN_API void BinaryenGlobalSetSetValue(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef valueExpr);

// MemoryGrow

// Gets the delta of a `memory.grow` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryGrowGetDelta(BinaryenExpressionRef expr);
// Sets the delta of a `memory.grow` expression.
BINARYEN_API void BinaryenMemoryGrowSetDelta(BinaryenExpressionRef expr,
                                             BinaryenExpressionRef delta);

// Load

// Gets whether a `load` expression is atomic (is an `atomic.load`).
BINARYEN_API bool BinaryenLoadIsAtomic(BinaryenExpressionRef expr);
// Sets whether a `load` expression is atomic (is an `atomic.load`).
BINARYEN_API void BinaryenLoadSetAtomic(BinaryenExpressionRef expr,
                                        bool isAtomic);
// Gets whether a `load` expression operates on a signed value (`_s`).
BINARYEN_API bool BinaryenLoadIsSigned(BinaryenExpressionRef expr);
// Sets whether a `load` expression operates on a signed value (`_s`).
BINARYEN_API void BinaryenLoadSetSigned(BinaryenExpressionRef expr,
                                        bool isSigned);
// Gets the constant offset of a `load` expression.
BINARYEN_API uint32_t BinaryenLoadGetOffset(BinaryenExpressionRef expr);
// Sets the constant offset of a `load` expression.
BINARYEN_API void BinaryenLoadSetOffset(BinaryenExpressionRef expr,
                                        uint32_t offset);
// Gets the number of bytes loaded by a `load` expression.
BINARYEN_API uint32_t BinaryenLoadGetBytes(BinaryenExpressionRef expr);
// Sets the number of bytes loaded by a `load` expression.
BINARYEN_API void BinaryenLoadSetBytes(BinaryenExpressionRef expr,
                                       uint32_t bytes);
// Gets the byte alignment of a `load` expression.
BINARYEN_API uint32_t BinaryenLoadGetAlign(BinaryenExpressionRef expr);
// Sets the byte alignment of a `load` expression.
BINARYEN_API void BinaryenLoadSetAlign(BinaryenExpressionRef expr,
                                       uint32_t align);
// Gets the pointer expression of a `load` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenLoadGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of a `load` expression.
BINARYEN_API void BinaryenLoadSetPtr(BinaryenExpressionRef expr,
                                     BinaryenExpressionRef ptrExpr);

// Store

// Gets whether a `store` expression is atomic (is an `atomic.store`).
BINARYEN_API bool BinaryenStoreIsAtomic(BinaryenExpressionRef expr);
// Sets whether a `store` expression is atomic (is an `atomic.store`).
BINARYEN_API void BinaryenStoreSetAtomic(BinaryenExpressionRef expr,
                                         bool isAtomic);
// Gets the number of bytes stored by a `store` expression.
BINARYEN_API uint32_t BinaryenStoreGetBytes(BinaryenExpressionRef expr);
// Sets the number of bytes stored by a `store` expression.
BINARYEN_API void BinaryenStoreSetBytes(BinaryenExpressionRef expr,
                                        uint32_t bytes);
// Gets the constant offset of a `store` expression.
BINARYEN_API uint32_t BinaryenStoreGetOffset(BinaryenExpressionRef expr);
// Sets the constant offset of a `store` expression.
BINARYEN_API void BinaryenStoreSetOffset(BinaryenExpressionRef expr,
                                         uint32_t offset);
// Gets the byte alignment of a `store` expression.
BINARYEN_API uint32_t BinaryenStoreGetAlign(BinaryenExpressionRef expr);
// Sets the byte alignment of a `store` expression.
BINARYEN_API void BinaryenStoreSetAlign(BinaryenExpressionRef expr,
                                        uint32_t align);
// Gets the pointer expression of a `store` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenStoreGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of a `store` expression.
BINARYEN_API void BinaryenStoreSetPtr(BinaryenExpressionRef expr,
                                      BinaryenExpressionRef ptrExpr);
// Gets the value expression of a `store` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenStoreGetValue(BinaryenExpressionRef expr);
// Sets the value expression of a `store` expression.
BINARYEN_API void BinaryenStoreSetValue(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef valueExpr);
// Gets the value type of a `store` expression.
BINARYEN_API BinaryenType BinaryenStoreGetValueType(BinaryenExpressionRef expr);
// Sets the value type of a `store` expression.
BINARYEN_API void BinaryenStoreSetValueType(BinaryenExpressionRef expr,
                                            BinaryenType valueType);

// Const

// Gets the 32-bit integer value of an `i32.const` expression.
BINARYEN_API int32_t BinaryenConstGetValueI32(BinaryenExpressionRef expr);
// Sets the 32-bit integer value of an `i32.const` expression.
BINARYEN_API void BinaryenConstSetValueI32(BinaryenExpressionRef expr,
                                           int32_t value);
// Gets the 64-bit integer value of an `i64.const` expression.
BINARYEN_API int64_t BinaryenConstGetValueI64(BinaryenExpressionRef expr);
// Sets the 64-bit integer value of an `i64.const` expression.
BINARYEN_API void BinaryenConstSetValueI64(BinaryenExpressionRef expr,
                                           int64_t value);
// Gets the low 32-bits of the 64-bit integer value of an `i64.const`
// expression.
BINARYEN_API int32_t BinaryenConstGetValueI64Low(BinaryenExpressionRef expr);
// Sets the low 32-bits of the 64-bit integer value of an `i64.const`
// expression.
BINARYEN_API void BinaryenConstSetValueI64Low(BinaryenExpressionRef expr,
                                              int32_t valueLow);
// Gets the high 32-bits of the 64-bit integer value of an `i64.const`
// expression.
BINARYEN_API int32_t BinaryenConstGetValueI64High(BinaryenExpressionRef expr);
// Sets the high 32-bits of the 64-bit integer value of an `i64.const`
// expression.
BINARYEN_API void BinaryenConstSetValueI64High(BinaryenExpressionRef expr,
                                               int32_t valueHigh);
// Gets the 32-bit float value of a `f32.const` expression.
BINARYEN_API float BinaryenConstGetValueF32(BinaryenExpressionRef expr);
// Sets the 32-bit float value of a `f32.const` expression.
BINARYEN_API void BinaryenConstSetValueF32(BinaryenExpressionRef expr,
                                           float value);
// Gets the 64-bit float (double) value of a `f64.const` expression.
BINARYEN_API double BinaryenConstGetValueF64(BinaryenExpressionRef expr);
// Sets the 64-bit float (double) value of a `f64.const` expression.
BINARYEN_API void BinaryenConstSetValueF64(BinaryenExpressionRef expr,
                                           double value);
// Reads the 128-bit vector value of a `v128.const` expression.
BINARYEN_API void BinaryenConstGetValueV128(BinaryenExpressionRef expr,
                                            uint8_t* out);
// Sets the 128-bit vector value of a `v128.const` expression.
BINARYEN_API void BinaryenConstSetValueV128(BinaryenExpressionRef expr,
                                            const uint8_t value[16]);

// Unary

// Gets the operation being performed by a unary expression.
BINARYEN_API BinaryenOp BinaryenUnaryGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a unary expression.
BINARYEN_API void BinaryenUnarySetOp(BinaryenExpressionRef expr, BinaryenOp op);
// Gets the value expression of a unary expression.
BINARYEN_API BinaryenExpressionRef
BinaryenUnaryGetValue(BinaryenExpressionRef expr);
// Sets the value expression of a unary expression.
BINARYEN_API void BinaryenUnarySetValue(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef valueExpr);

// Binary

// Gets the operation being performed by a binary expression.
BINARYEN_API BinaryenOp BinaryenBinaryGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a binary expression.
BINARYEN_API void BinaryenBinarySetOp(BinaryenExpressionRef expr,
                                      BinaryenOp op);
// Gets the left expression of a binary expression.
BINARYEN_API BinaryenExpressionRef
BinaryenBinaryGetLeft(BinaryenExpressionRef expr);
// Sets the left expression of a binary expression.
BINARYEN_API void BinaryenBinarySetLeft(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef leftExpr);
// Gets the right expression of a binary expression.
BINARYEN_API BinaryenExpressionRef
BinaryenBinaryGetRight(BinaryenExpressionRef expr);
// Sets the right expression of a binary expression.
BINARYEN_API void BinaryenBinarySetRight(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef rightExpr);

// Select

// Gets the expression becoming selected by a `select` expression if the
// condition turns out true.
BINARYEN_API BinaryenExpressionRef
BinaryenSelectGetIfTrue(BinaryenExpressionRef expr);
// Sets the expression becoming selected by a `select` expression if the
// condition turns out true.
BINARYEN_API void BinaryenSelectSetIfTrue(BinaryenExpressionRef expr,
                                          BinaryenExpressionRef ifTrueExpr);
// Gets the expression becoming selected by a `select` expression if the
// condition turns out false.
BINARYEN_API BinaryenExpressionRef
BinaryenSelectGetIfFalse(BinaryenExpressionRef expr);
// Sets the expression becoming selected by a `select` expression if the
// condition turns out false.
BINARYEN_API void BinaryenSelectSetIfFalse(BinaryenExpressionRef expr,
                                           BinaryenExpressionRef ifFalseExpr);
// Gets the condition expression of a `select` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSelectGetCondition(BinaryenExpressionRef expr);
// Sets the condition expression of a `select` expression.
BINARYEN_API void BinaryenSelectSetCondition(BinaryenExpressionRef expr,
                                             BinaryenExpressionRef condExpr);

// Drop

// Gets the value expression being dropped by a `drop` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenDropGetValue(BinaryenExpressionRef expr);
// Sets the value expression being dropped by a `drop` expression.
BINARYEN_API void BinaryenDropSetValue(BinaryenExpressionRef expr,
                                       BinaryenExpressionRef valueExpr);

// Return

// Gets the value expression, if any, being returned by a `return` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenReturnGetValue(BinaryenExpressionRef expr);
// Sets the value expression, if any, being returned by a `return` expression.
BINARYEN_API void BinaryenReturnSetValue(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef valueExpr);

// AtomicRMW

// Gets the operation being performed by an atomic read-modify-write expression.
BINARYEN_API BinaryenOp BinaryenAtomicRMWGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by an atomic read-modify-write expression.
BINARYEN_API void BinaryenAtomicRMWSetOp(BinaryenExpressionRef expr,
                                         BinaryenOp op);
// Gets the number of bytes affected by an atomic read-modify-write expression.
BINARYEN_API uint32_t BinaryenAtomicRMWGetBytes(BinaryenExpressionRef expr);
// Sets the number of bytes affected by an atomic read-modify-write expression.
BINARYEN_API void BinaryenAtomicRMWSetBytes(BinaryenExpressionRef expr,
                                            uint32_t bytes);
// Gets the constant offset of an atomic read-modify-write expression.
BINARYEN_API uint32_t BinaryenAtomicRMWGetOffset(BinaryenExpressionRef expr);
// Sets the constant offset of an atomic read-modify-write expression.
BINARYEN_API void BinaryenAtomicRMWSetOffset(BinaryenExpressionRef expr,
                                             uint32_t offset);
// Gets the pointer expression of an atomic read-modify-write expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicRMWGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of an atomic read-modify-write expression.
BINARYEN_API void BinaryenAtomicRMWSetPtr(BinaryenExpressionRef expr,
                                          BinaryenExpressionRef ptrExpr);
// Gets the value expression of an atomic read-modify-write expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicRMWGetValue(BinaryenExpressionRef expr);
// Sets the value expression of an atomic read-modify-write expression.
BINARYEN_API void BinaryenAtomicRMWSetValue(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef valueExpr);

// AtomicCmpxchg

// Gets the number of bytes affected by an atomic compare and exchange
// expression.
BINARYEN_API uint32_t BinaryenAtomicCmpxchgGetBytes(BinaryenExpressionRef expr);
// Sets the number of bytes affected by an atomic compare and exchange
// expression.
BINARYEN_API void BinaryenAtomicCmpxchgSetBytes(BinaryenExpressionRef expr,
                                                uint32_t bytes);
// Gets the constant offset of an atomic compare and exchange expression.
BINARYEN_API uint32_t
BinaryenAtomicCmpxchgGetOffset(BinaryenExpressionRef expr);
// Sets the constant offset of an atomic compare and exchange expression.
BINARYEN_API void BinaryenAtomicCmpxchgSetOffset(BinaryenExpressionRef expr,
                                                 uint32_t offset);
// Gets the pointer expression of an atomic compare and exchange expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicCmpxchgGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of an atomic compare and exchange expression.
BINARYEN_API void BinaryenAtomicCmpxchgSetPtr(BinaryenExpressionRef expr,
                                              BinaryenExpressionRef ptrExpr);
// Gets the expression representing the expected value of an atomic compare and
// exchange expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicCmpxchgGetExpected(BinaryenExpressionRef expr);
// Sets the expression representing the expected value of an atomic compare and
// exchange expression.
BINARYEN_API void
BinaryenAtomicCmpxchgSetExpected(BinaryenExpressionRef expr,
                                 BinaryenExpressionRef expectedExpr);
// Gets the replacement expression of an atomic compare and exchange expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicCmpxchgGetReplacement(BinaryenExpressionRef expr);
// Sets the replacement expression of an atomic compare and exchange expression.
BINARYEN_API void
BinaryenAtomicCmpxchgSetReplacement(BinaryenExpressionRef expr,
                                    BinaryenExpressionRef replacementExpr);

// AtomicWait

// Gets the pointer expression of an `memory.atomic.wait` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicWaitGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of an `memory.atomic.wait` expression.
BINARYEN_API void BinaryenAtomicWaitSetPtr(BinaryenExpressionRef expr,
                                           BinaryenExpressionRef ptrExpr);
// Gets the expression representing the expected value of an
// `memory.atomic.wait` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicWaitGetExpected(BinaryenExpressionRef expr);
// Sets the expression representing the expected value of an
// `memory.atomic.wait` expression.
BINARYEN_API void
BinaryenAtomicWaitSetExpected(BinaryenExpressionRef expr,
                              BinaryenExpressionRef expectedExpr);
// Gets the timeout expression of an `memory.atomic.wait` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicWaitGetTimeout(BinaryenExpressionRef expr);
// Sets the timeout expression of an `memory.atomic.wait` expression.
BINARYEN_API void
BinaryenAtomicWaitSetTimeout(BinaryenExpressionRef expr,
                             BinaryenExpressionRef timeoutExpr);
// Gets the expected type of an `memory.atomic.wait` expression.
BINARYEN_API BinaryenType
BinaryenAtomicWaitGetExpectedType(BinaryenExpressionRef expr);
// Sets the expected type of an `memory.atomic.wait` expression.
BINARYEN_API void BinaryenAtomicWaitSetExpectedType(BinaryenExpressionRef expr,
                                                    BinaryenType expectedType);

// AtomicNotify

// Gets the pointer expression of an `memory.atomic.notify` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicNotifyGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of an `memory.atomic.notify` expression.
BINARYEN_API void BinaryenAtomicNotifySetPtr(BinaryenExpressionRef expr,
                                             BinaryenExpressionRef ptrExpr);
// Gets the notify count expression of an `memory.atomic.notify` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenAtomicNotifyGetNotifyCount(BinaryenExpressionRef expr);
// Sets the notify count expression of an `memory.atomic.notify` expression.
BINARYEN_API void
BinaryenAtomicNotifySetNotifyCount(BinaryenExpressionRef expr,
                                   BinaryenExpressionRef notifyCountExpr);

// AtomicFence

// Gets the order of an `atomic.fence` expression.
BINARYEN_API uint8_t BinaryenAtomicFenceGetOrder(BinaryenExpressionRef expr);
// Sets the order of an `atomic.fence` expression.
BINARYEN_API void BinaryenAtomicFenceSetOrder(BinaryenExpressionRef expr,
                                              uint8_t order);

// SIMDExtract

// Gets the operation being performed by a SIMD extract expression.
BINARYEN_API BinaryenOp BinaryenSIMDExtractGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a SIMD extract expression.
BINARYEN_API void BinaryenSIMDExtractSetOp(BinaryenExpressionRef expr,
                                           BinaryenOp op);
// Gets the vector expression a SIMD extract expression extracts from.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDExtractGetVec(BinaryenExpressionRef expr);
// Sets the vector expression a SIMD extract expression extracts from.
BINARYEN_API void BinaryenSIMDExtractSetVec(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef vecExpr);
// Gets the index of the extracted lane of a SIMD extract expression.
BINARYEN_API uint8_t BinaryenSIMDExtractGetIndex(BinaryenExpressionRef expr);
// Sets the index of the extracted lane of a SIMD extract expression.
BINARYEN_API void BinaryenSIMDExtractSetIndex(BinaryenExpressionRef expr,
                                              uint8_t index);

// SIMDReplace

// Gets the operation being performed by a SIMD replace expression.
BINARYEN_API BinaryenOp BinaryenSIMDReplaceGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a SIMD replace expression.
BINARYEN_API void BinaryenSIMDReplaceSetOp(BinaryenExpressionRef expr,
                                           BinaryenOp op);
// Gets the vector expression a SIMD replace expression replaces in.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDReplaceGetVec(BinaryenExpressionRef expr);
// Sets the vector expression a SIMD replace expression replaces in.
BINARYEN_API void BinaryenSIMDReplaceSetVec(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef vecExpr);
// Gets the index of the replaced lane of a SIMD replace expression.
BINARYEN_API uint8_t BinaryenSIMDReplaceGetIndex(BinaryenExpressionRef expr);
// Sets the index of the replaced lane of a SIMD replace expression.
BINARYEN_API void BinaryenSIMDReplaceSetIndex(BinaryenExpressionRef expr,
                                              uint8_t index);
// Gets the value expression a SIMD replace expression replaces with.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDReplaceGetValue(BinaryenExpressionRef expr);
// Sets the value expression a SIMD replace expression replaces with.
BINARYEN_API void BinaryenSIMDReplaceSetValue(BinaryenExpressionRef expr,
                                              BinaryenExpressionRef valueExpr);

// SIMDShuffle

// Gets the left expression of a SIMD shuffle expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDShuffleGetLeft(BinaryenExpressionRef expr);
// Sets the left expression of a SIMD shuffle expression.
BINARYEN_API void BinaryenSIMDShuffleSetLeft(BinaryenExpressionRef expr,
                                             BinaryenExpressionRef leftExpr);
// Gets the right expression of a SIMD shuffle expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDShuffleGetRight(BinaryenExpressionRef expr);
// Sets the right expression of a SIMD shuffle expression.
BINARYEN_API void BinaryenSIMDShuffleSetRight(BinaryenExpressionRef expr,
                                              BinaryenExpressionRef rightExpr);
// Gets the 128-bit mask of a SIMD shuffle expression.
BINARYEN_API void BinaryenSIMDShuffleGetMask(BinaryenExpressionRef expr,
                                             uint8_t* mask);
// Sets the 128-bit mask of a SIMD shuffle expression.
BINARYEN_API void BinaryenSIMDShuffleSetMask(BinaryenExpressionRef expr,
                                             const uint8_t mask[16]);

// SIMDTernary

// Gets the operation being performed by a SIMD ternary expression.
BINARYEN_API BinaryenOp BinaryenSIMDTernaryGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a SIMD ternary expression.
BINARYEN_API void BinaryenSIMDTernarySetOp(BinaryenExpressionRef expr,
                                           BinaryenOp op);
// Gets the first operand expression of a SIMD ternary expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDTernaryGetA(BinaryenExpressionRef expr);
// Sets the first operand expression of a SIMD ternary expression.
BINARYEN_API void BinaryenSIMDTernarySetA(BinaryenExpressionRef expr,
                                          BinaryenExpressionRef aExpr);
// Gets the second operand expression of a SIMD ternary expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDTernaryGetB(BinaryenExpressionRef expr);
// Sets the second operand expression of a SIMD ternary expression.
BINARYEN_API void BinaryenSIMDTernarySetB(BinaryenExpressionRef expr,
                                          BinaryenExpressionRef bExpr);
// Gets the third operand expression of a SIMD ternary expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDTernaryGetC(BinaryenExpressionRef expr);
// Sets the third operand expression of a SIMD ternary expression.
BINARYEN_API void BinaryenSIMDTernarySetC(BinaryenExpressionRef expr,
                                          BinaryenExpressionRef cExpr);

// SIMDShift

// Gets the operation being performed by a SIMD shift expression.
BINARYEN_API BinaryenOp BinaryenSIMDShiftGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a SIMD shift expression.
BINARYEN_API void BinaryenSIMDShiftSetOp(BinaryenExpressionRef expr,
                                         BinaryenOp op);
// Gets the expression being shifted by a SIMD shift expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDShiftGetVec(BinaryenExpressionRef expr);
// Sets the expression being shifted by a SIMD shift expression.
BINARYEN_API void BinaryenSIMDShiftSetVec(BinaryenExpressionRef expr,
                                          BinaryenExpressionRef vecExpr);
// Gets the expression representing the shift of a SIMD shift expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDShiftGetShift(BinaryenExpressionRef expr);
// Sets the expression representing the shift of a SIMD shift expression.
BINARYEN_API void BinaryenSIMDShiftSetShift(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef shiftExpr);

// SIMDLoad

// Gets the operation being performed by a SIMD load expression.
BINARYEN_API BinaryenOp BinaryenSIMDLoadGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a SIMD load expression.
BINARYEN_API void BinaryenSIMDLoadSetOp(BinaryenExpressionRef expr,
                                        BinaryenOp op);
// Gets the constant offset of a SIMD load expression.
BINARYEN_API uint32_t BinaryenSIMDLoadGetOffset(BinaryenExpressionRef expr);
// Sets the constant offset of a SIMD load expression.
BINARYEN_API void BinaryenSIMDLoadSetOffset(BinaryenExpressionRef expr,
                                            uint32_t offset);
// Gets the byte alignment of a SIMD load expression.
BINARYEN_API uint32_t BinaryenSIMDLoadGetAlign(BinaryenExpressionRef expr);
// Sets the byte alignment of a SIMD load expression.
BINARYEN_API void BinaryenSIMDLoadSetAlign(BinaryenExpressionRef expr,
                                           uint32_t align);
// Gets the pointer expression of a SIMD load expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDLoadGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of a SIMD load expression.
BINARYEN_API void BinaryenSIMDLoadSetPtr(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef ptrExpr);

// SIMDLoadStoreLane

// Gets the operation being performed by a SIMD load/store lane expression.
BINARYEN_API BinaryenOp
BinaryenSIMDLoadStoreLaneGetOp(BinaryenExpressionRef expr);
// Sets the operation being performed by a SIMD load/store lane expression.
BINARYEN_API void BinaryenSIMDLoadStoreLaneSetOp(BinaryenExpressionRef expr,
                                                 BinaryenOp op);
// Gets the constant offset of a SIMD load/store lane expression.
BINARYEN_API uint32_t
BinaryenSIMDLoadStoreLaneGetOffset(BinaryenExpressionRef expr);
// Sets the constant offset of a SIMD load/store lane expression.
BINARYEN_API void BinaryenSIMDLoadStoreLaneSetOffset(BinaryenExpressionRef expr,
                                                     uint32_t offset);
// Gets the byte alignment of a SIMD load/store lane expression.
BINARYEN_API uint32_t
BinaryenSIMDLoadStoreLaneGetAlign(BinaryenExpressionRef expr);
// Sets the byte alignment of a SIMD load/store lane expression.
BINARYEN_API void BinaryenSIMDLoadStoreLaneSetAlign(BinaryenExpressionRef expr,
                                                    uint32_t align);
// Gets the lane index of a SIMD load/store lane expression.
BINARYEN_API uint8_t
BinaryenSIMDLoadStoreLaneGetIndex(BinaryenExpressionRef expr);
// Sets the lane index of a SIMD load/store lane expression.
BINARYEN_API void BinaryenSIMDLoadStoreLaneSetIndex(BinaryenExpressionRef expr,
                                                    uint8_t index);
// Gets the pointer expression of a SIMD load/store lane expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDLoadStoreLaneGetPtr(BinaryenExpressionRef expr);
// Sets the pointer expression of a SIMD load/store lane expression.
BINARYEN_API void
BinaryenSIMDLoadStoreLaneSetPtr(BinaryenExpressionRef expr,
                                BinaryenExpressionRef ptrExpr);
// Gets the vector expression of a SIMD load/store lane expression.
BINARYEN_API BinaryenExpressionRef
BinaryenSIMDLoadStoreLaneGetVec(BinaryenExpressionRef expr);
// Sets the vector expression of a SIMD load/store lane expression.
BINARYEN_API void
BinaryenSIMDLoadStoreLaneSetVec(BinaryenExpressionRef expr,
                                BinaryenExpressionRef vecExpr);
// Gets whether a SIMD load/store lane expression performs a store. Otherwise it
// performs a load.
BINARYEN_API bool BinaryenSIMDLoadStoreLaneIsStore(BinaryenExpressionRef expr);

// MemoryInit

// Gets the index of the segment being initialized by a `memory.init`
// expression.
BINARYEN_API uint32_t BinaryenMemoryInitGetSegment(BinaryenExpressionRef expr);
// Sets the index of the segment being initialized by a `memory.init`
// expression.
BINARYEN_API void BinaryenMemoryInitSetSegment(BinaryenExpressionRef expr,
                                               uint32_t segmentIndex);
// Gets the destination expression of a `memory.init` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryInitGetDest(BinaryenExpressionRef expr);
// Sets the destination expression of a `memory.init` expression.
BINARYEN_API void BinaryenMemoryInitSetDest(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef destExpr);
// Gets the offset expression of a `memory.init` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryInitGetOffset(BinaryenExpressionRef expr);
// Sets the offset expression of a `memory.init` expression.
BINARYEN_API void BinaryenMemoryInitSetOffset(BinaryenExpressionRef expr,
                                              BinaryenExpressionRef offsetExpr);
// Gets the size expression of a `memory.init` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryInitGetSize(BinaryenExpressionRef expr);
// Sets the size expression of a `memory.init` expression.
BINARYEN_API void BinaryenMemoryInitSetSize(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef sizeExpr);

// DataDrop

// Gets the index of the segment being dropped by a `data.drop` expression.
BINARYEN_API uint32_t BinaryenDataDropGetSegment(BinaryenExpressionRef expr);
// Sets the index of the segment being dropped by a `data.drop` expression.
BINARYEN_API void BinaryenDataDropSetSegment(BinaryenExpressionRef expr,
                                             uint32_t segmentIndex);

// MemoryCopy

// Gets the destination expression of a `memory.copy` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryCopyGetDest(BinaryenExpressionRef expr);
// Sets the destination expression of a `memory.copy` expression.
BINARYEN_API void BinaryenMemoryCopySetDest(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef destExpr);
// Gets the source expression of a `memory.copy` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryCopyGetSource(BinaryenExpressionRef expr);
// Sets the source expression of a `memory.copy` expression.
BINARYEN_API void BinaryenMemoryCopySetSource(BinaryenExpressionRef expr,
                                              BinaryenExpressionRef sourceExpr);
// Gets the size expression (number of bytes copied) of a `memory.copy`
// expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryCopyGetSize(BinaryenExpressionRef expr);
// Sets the size expression (number of bytes copied) of a `memory.copy`
// expression.
BINARYEN_API void BinaryenMemoryCopySetSize(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef sizeExpr);

// MemoryFill

// Gets the destination expression of a `memory.fill` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryFillGetDest(BinaryenExpressionRef expr);
// Sets the destination expression of a `memory.fill` expression.
BINARYEN_API void BinaryenMemoryFillSetDest(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef destExpr);
// Gets the value expression of a `memory.fill` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryFillGetValue(BinaryenExpressionRef expr);
// Sets the value expression of a `memory.fill` expression.
BINARYEN_API void BinaryenMemoryFillSetValue(BinaryenExpressionRef expr,
                                             BinaryenExpressionRef valueExpr);
// Gets the size expression (number of bytes filled) of a `memory.fill`
// expression.
BINARYEN_API BinaryenExpressionRef
BinaryenMemoryFillGetSize(BinaryenExpressionRef expr);
// Sets the size expression (number of bytes filled) of a `memory.fill`
// expression.
BINARYEN_API void BinaryenMemoryFillSetSize(BinaryenExpressionRef expr,
                                            BinaryenExpressionRef sizeExpr);

// RefIs

// Gets the operation performed by a `ref.is_*` expression.
BINARYEN_API BinaryenOp BinaryenRefIsGetOp(BinaryenExpressionRef expr);
// Sets the operation performed by a `ref.is_*` expression.
BINARYEN_API void BinaryenRefIsSetOp(BinaryenExpressionRef expr, BinaryenOp op);
// Gets the value expression tested by a `ref.is_*` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenRefIsGetValue(BinaryenExpressionRef expr);
// Sets the value expression tested by a `ref.is_*` expression.
BINARYEN_API void BinaryenRefIsSetValue(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef valueExpr);

// RefAs

// Gets the operation performed by a `ref.as_*` expression.
BINARYEN_API BinaryenOp BinaryenRefAsGetOp(BinaryenExpressionRef expr);
// Sets the operation performed by a `ref.as_*` expression.
BINARYEN_API void BinaryenRefAsSetOp(BinaryenExpressionRef expr, BinaryenOp op);
// Gets the value expression tested by a `ref.as_*` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenRefAsGetValue(BinaryenExpressionRef expr);
// Sets the value expression tested by a `ref.as_*` expression.
BINARYEN_API void BinaryenRefAsSetValue(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef valueExpr);

// RefFunc

// Gets the name of the function being wrapped by a `ref.func` expression.
BINARYEN_API const char* BinaryenRefFuncGetFunc(BinaryenExpressionRef expr);
// Sets the name of the function being wrapped by a `ref.func` expression.
BINARYEN_API void BinaryenRefFuncSetFunc(BinaryenExpressionRef expr,
                                         const char* funcName);

// RefEq

// Gets the left expression of a `ref.eq` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenRefEqGetLeft(BinaryenExpressionRef expr);
// Sets the left expression of a `ref.eq` expression.
BINARYEN_API void BinaryenRefEqSetLeft(BinaryenExpressionRef expr,
                                       BinaryenExpressionRef left);
// Gets the right expression of a `ref.eq` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenRefEqGetRight(BinaryenExpressionRef expr);
// Sets the right expression of a `ref.eq` expression.
BINARYEN_API void BinaryenRefEqSetRight(BinaryenExpressionRef expr,
                                        BinaryenExpressionRef right);

// Try

// Gets the name (label) of a `try` expression.
BINARYEN_API const char* BinaryenTryGetName(BinaryenExpressionRef expr);
// Sets the name (label) of a `try` expression.
BINARYEN_API void BinaryenTrySetName(BinaryenExpressionRef expr,
                                     const char* name);
// Gets the body expression of a `try` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenTryGetBody(BinaryenExpressionRef expr);
// Sets the body expression of a `try` expression.
BINARYEN_API void BinaryenTrySetBody(BinaryenExpressionRef expr,
                                     BinaryenExpressionRef bodyExpr);
// Gets the number of catch blocks (= the number of catch events) of a `try`
// expression.
BINARYEN_API BinaryenIndex
BinaryenTryGetNumCatchEvents(BinaryenExpressionRef expr);
// Gets the number of catch/catch_all blocks of a `try` expression.
BINARYEN_API BinaryenIndex
BinaryenTryGetNumCatchBodies(BinaryenExpressionRef expr);
// Gets the catch event at the specified index of a `try` expression.
BINARYEN_API const char* BinaryenTryGetCatchEventAt(BinaryenExpressionRef expr,
                                                    BinaryenIndex index);
// Sets the catch event at the specified index of a `try` expression.
BINARYEN_API void BinaryenTrySetCatchEventAt(BinaryenExpressionRef expr,
                                             BinaryenIndex index,
                                             const char* catchEvent);
// Appends a catch event to a `try` expression, returning its insertion index.
BINARYEN_API BinaryenIndex
BinaryenTryAppendCatchEvent(BinaryenExpressionRef expr, const char* catchEvent);
// Inserts a catch event at the specified index of a `try` expression, moving
// existing catch events including the one previously at that index one index
// up.
BINARYEN_API void BinaryenTryInsertCatchEventAt(BinaryenExpressionRef expr,
                                                BinaryenIndex index,
                                                const char* catchEvent);
// Removes the catch event at the specified index of a `try` expression, moving
// all subsequent catch events one index down. Returns the event.
BINARYEN_API const char*
BinaryenTryRemoveCatchEventAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Gets the catch body expression at the specified index of a `try` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenTryGetCatchBodyAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Sets the catch body expression at the specified index of a `try` expression.
BINARYEN_API void BinaryenTrySetCatchBodyAt(BinaryenExpressionRef expr,
                                            BinaryenIndex index,
                                            BinaryenExpressionRef catchExpr);
// Appends a catch expression to a `try` expression, returning its insertion
// index.
BINARYEN_API BinaryenIndex BinaryenTryAppendCatchBody(
  BinaryenExpressionRef expr, BinaryenExpressionRef catchExpr);
// Inserts a catch expression at the specified index of a `try` expression,
// moving existing catch bodies including the one previously at that index one
// index up.
BINARYEN_API void BinaryenTryInsertCatchBodyAt(BinaryenExpressionRef expr,
                                               BinaryenIndex index,
                                               BinaryenExpressionRef catchExpr);
// Removes the catch expression at the specified index of a `try` expression,
// moving all subsequent catch bodies one index down. Returns the catch
// expression.
BINARYEN_API BinaryenExpressionRef
BinaryenTryRemoveCatchBodyAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Gets whether a `try` expression has a catch_all clause.
BINARYEN_API bool BinaryenTryHasCatchAll(BinaryenExpressionRef expr);
// Gets the target label of a `delegate`.
BINARYEN_API const char*
BinaryenTryGetDelegateTarget(BinaryenExpressionRef expr);
// Sets the target label of a `delegate`.
BINARYEN_API void BinaryenTrySetDelegateTarget(BinaryenExpressionRef expr,
                                               const char* delegateTarget);
// Gets whether a `try` expression is a try-delegate.
BINARYEN_API bool BinaryenTryIsDelegate(BinaryenExpressionRef expr);

// Throw

// Gets the name of the event being thrown by a `throw` expression.
BINARYEN_API const char* BinaryenThrowGetEvent(BinaryenExpressionRef expr);
// Sets the name of the event being thrown by a `throw` expression.
BINARYEN_API void BinaryenThrowSetEvent(BinaryenExpressionRef expr,
                                        const char* eventName);
// Gets the number of operands of a `throw` expression.
BINARYEN_API BinaryenIndex
BinaryenThrowGetNumOperands(BinaryenExpressionRef expr);
// Gets the operand at the specified index of a `throw` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenThrowGetOperandAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Sets the operand at the specified index of a `throw` expression.
BINARYEN_API void BinaryenThrowSetOperandAt(BinaryenExpressionRef expr,
                                            BinaryenIndex index,
                                            BinaryenExpressionRef operandExpr);
// Appends an operand expression to a `throw` expression, returning its
// insertion index.
BINARYEN_API BinaryenIndex BinaryenThrowAppendOperand(
  BinaryenExpressionRef expr, BinaryenExpressionRef operandExpr);
// Inserts an operand expression at the specified index of a `throw` expression,
// moving existing operands including the one previously at that index one index
// up.
BINARYEN_API void
BinaryenThrowInsertOperandAt(BinaryenExpressionRef expr,
                             BinaryenIndex index,
                             BinaryenExpressionRef operandExpr);
// Removes the operand expression at the specified index of a `throw`
// expression, moving all subsequent operands one index down. Returns the
// operand expression.
BINARYEN_API BinaryenExpressionRef
BinaryenThrowRemoveOperandAt(BinaryenExpressionRef expr, BinaryenIndex index);

// Rethrow

// Gets the target catch's corresponding try label of a `rethrow` expression.
BINARYEN_API const char* BinaryenRethrowGetTarget(BinaryenExpressionRef expr);
// Sets the target catch's corresponding try label of a `rethrow` expression.
BINARYEN_API void BinaryenRethrowSetTarget(BinaryenExpressionRef expr,
                                           const char* target);

// TupleMake

// Gets the number of operands of a `tuple.make` expression.
BINARYEN_API BinaryenIndex
BinaryenTupleMakeGetNumOperands(BinaryenExpressionRef expr);
// Gets the operand at the specified index of a `tuple.make` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenTupleMakeGetOperandAt(BinaryenExpressionRef expr, BinaryenIndex index);
// Sets the operand at the specified index of a `tuple.make` expression.
BINARYEN_API void
BinaryenTupleMakeSetOperandAt(BinaryenExpressionRef expr,
                              BinaryenIndex index,
                              BinaryenExpressionRef operandExpr);
// Appends an operand expression to a `tuple.make` expression, returning its
// insertion index.
BINARYEN_API BinaryenIndex BinaryenTupleMakeAppendOperand(
  BinaryenExpressionRef expr, BinaryenExpressionRef operandExpr);
// Inserts an operand expression at the specified index of a `tuple.make`
// expression, moving existing operands including the one previously at that
// index one index up.
BINARYEN_API void
BinaryenTupleMakeInsertOperandAt(BinaryenExpressionRef expr,
                                 BinaryenIndex index,
                                 BinaryenExpressionRef operandExpr);
// Removes the operand expression at the specified index of a `tuple.make`
// expression, moving all subsequent operands one index down. Returns the
// operand expression.
BINARYEN_API BinaryenExpressionRef BinaryenTupleMakeRemoveOperandAt(
  BinaryenExpressionRef expr, BinaryenIndex index);

// TupleExtract

// Gets the tuple extracted from of a `tuple.extract` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenTupleExtractGetTuple(BinaryenExpressionRef expr);
// Sets the tuple extracted from of a `tuple.extract` expression.
BINARYEN_API void BinaryenTupleExtractSetTuple(BinaryenExpressionRef expr,
                                               BinaryenExpressionRef tupleExpr);
// Gets the index extracted at of a `tuple.extract` expression.
BINARYEN_API BinaryenIndex
BinaryenTupleExtractGetIndex(BinaryenExpressionRef expr);
// Sets the index extracted at of a `tuple.extract` expression.
BINARYEN_API void BinaryenTupleExtractSetIndex(BinaryenExpressionRef expr,
                                               BinaryenIndex index);

// I31New

// Gets the value expression of an `i31.new` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenI31NewGetValue(BinaryenExpressionRef expr);
// Sets the value expression of an `i31.new` expression.
BINARYEN_API void BinaryenI31NewSetValue(BinaryenExpressionRef expr,
                                         BinaryenExpressionRef valueExpr);

// I31Get

// Gets the i31 expression of an `i31.get` expression.
BINARYEN_API BinaryenExpressionRef
BinaryenI31GetGetI31(BinaryenExpressionRef expr);
// Sets the i31 expression of an `i31.get` expression.
BINARYEN_API void BinaryenI31GetSetI31(BinaryenExpressionRef expr,
                                       BinaryenExpressionRef i31Expr);
// Gets whether an `i31.get` expression returns a signed value (`_s`).
BINARYEN_API bool BinaryenI31GetIsSigned(BinaryenExpressionRef expr);
// Sets whether an `i31.get` expression returns a signed value (`_s`).
BINARYEN_API void BinaryenI31GetSetSigned(BinaryenExpressionRef expr,
                                          bool signed_);

// Functions

BINARYEN_REF(Function);

// Adds a function to the module. This is thread-safe.
// @varTypes: the types of variables. In WebAssembly, vars share
//            an index space with params. In other words, params come from
//            the function type, and vars are provided in this call, and
//            together they are all the locals. The order is first params
//            and then vars, so if you have one param it will be at index
//            0 (and written $0), and if you also have 2 vars they will be
//            at indexes 1 and 2, etc., that is, they share an index space.
BINARYEN_API BinaryenFunctionRef
BinaryenAddFunction(BinaryenModuleRef module,
                    const char* name,
                    BinaryenType params,
                    BinaryenType results,
                    BinaryenType* varTypes,
                    BinaryenIndex numVarTypes,
                    BinaryenExpressionRef body);
// Gets a function reference by name. Returns NULL if the function does not
// exist.
BINARYEN_API BinaryenFunctionRef BinaryenGetFunction(BinaryenModuleRef module,
                                                     const char* name);
// Removes a function by name.
BINARYEN_API void BinaryenRemoveFunction(BinaryenModuleRef module,
                                         const char* name);

// Gets the number of functions in the module.
BINARYEN_API BinaryenIndex BinaryenGetNumFunctions(BinaryenModuleRef module);
// Gets the function at the specified index.
BINARYEN_API BinaryenFunctionRef
BinaryenGetFunctionByIndex(BinaryenModuleRef module, BinaryenIndex index);

// Imports

BINARYEN_API void BinaryenAddFunctionImport(BinaryenModuleRef module,
                                            const char* internalName,
                                            const char* externalModuleName,
                                            const char* externalBaseName,
                                            BinaryenType params,
                                            BinaryenType results);
BINARYEN_API void BinaryenAddTableImport(BinaryenModuleRef module,
                                         const char* internalName,
                                         const char* externalModuleName,
                                         const char* externalBaseName);
BINARYEN_API void BinaryenAddMemoryImport(BinaryenModuleRef module,
                                          const char* internalName,
                                          const char* externalModuleName,
                                          const char* externalBaseName,
                                          uint8_t shared);
BINARYEN_API void BinaryenAddGlobalImport(BinaryenModuleRef module,
                                          const char* internalName,
                                          const char* externalModuleName,
                                          const char* externalBaseName,
                                          BinaryenType globalType,
                                          bool mutable_);
BINARYEN_API void BinaryenAddEventImport(BinaryenModuleRef module,
                                         const char* internalName,
                                         const char* externalModuleName,
                                         const char* externalBaseName,
                                         uint32_t attribute,
                                         BinaryenType params,
                                         BinaryenType results);

// Exports

BINARYEN_REF(Export);

WASM_DEPRECATED BinaryenExportRef BinaryenAddExport(BinaryenModuleRef module,
                                                    const char* internalName,
                                                    const char* externalName);
// Adds a function export to the module.
BINARYEN_API BinaryenExportRef BinaryenAddFunctionExport(
  BinaryenModuleRef module, const char* internalName, const char* externalName);
// Adds a table export to the module.
BINARYEN_API BinaryenExportRef BinaryenAddTableExport(BinaryenModuleRef module,
                                                      const char* internalName,
                                                      const char* externalName);
// Adds a memory export to the module.
BINARYEN_API BinaryenExportRef BinaryenAddMemoryExport(
  BinaryenModuleRef module, const char* internalName, const char* externalName);
// Adds a global export to the module.
BINARYEN_API BinaryenExportRef BinaryenAddGlobalExport(
  BinaryenModuleRef module, const char* internalName, const char* externalName);
// Adds an event export to the module.
BINARYEN_API BinaryenExportRef BinaryenAddEventExport(BinaryenModuleRef module,
                                                      const char* internalName,
                                                      const char* externalName);
// Gets an export reference by external name. Returns NULL if the export does
// not exist.
BINARYEN_API BinaryenExportRef BinaryenGetExport(BinaryenModuleRef module,
                                                 const char* externalName);
// Removes an export by external name.
BINARYEN_API void BinaryenRemoveExport(BinaryenModuleRef module,
                                       const char* externalName);
// Gets the number of exports in the module.
BINARYEN_API BinaryenIndex BinaryenGetNumExports(BinaryenModuleRef module);
// Gets the export at the specified index.
BINARYEN_API BinaryenExportRef
BinaryenGetExportByIndex(BinaryenModuleRef module, BinaryenIndex index);

// Globals

BINARYEN_REF(Global);

// Adds a global to the module.
BINARYEN_API BinaryenGlobalRef BinaryenAddGlobal(BinaryenModuleRef module,
                                                 const char* name,
                                                 BinaryenType type,
                                                 bool mutable_,
                                                 BinaryenExpressionRef init);
// Gets a global reference by name. Returns NULL if the global does not exist.
BINARYEN_API BinaryenGlobalRef BinaryenGetGlobal(BinaryenModuleRef module,
                                                 const char* name);
// Removes a global by name.
BINARYEN_API void BinaryenRemoveGlobal(BinaryenModuleRef module,
                                       const char* name);
// Gets the number of globals in the module.
BINARYEN_API BinaryenIndex BinaryenGetNumGlobals(BinaryenModuleRef module);
// Gets the global at the specified index.
BINARYEN_API BinaryenGlobalRef
BinaryenGetGlobalByIndex(BinaryenModuleRef module, BinaryenIndex index);

// Events

BINARYEN_REF(Event);

// Adds an event to the module.
BINARYEN_API BinaryenEventRef BinaryenAddEvent(BinaryenModuleRef module,
                                               const char* name,
                                               uint32_t attribute,
                                               BinaryenType params,
                                               BinaryenType results);
// Gets an event reference by name. Returns NULL if the event does not exist.
BINARYEN_API BinaryenEventRef BinaryenGetEvent(BinaryenModuleRef module,
                                               const char* name);
// Removes an event by name.
BINARYEN_API void BinaryenRemoveEvent(BinaryenModuleRef module,
                                      const char* name);

// Tables

BINARYEN_REF(Table);

BINARYEN_API BinaryenTableRef BinaryenAddTable(BinaryenModuleRef module,
                                               const char* table,
                                               BinaryenIndex initial,
                                               BinaryenIndex maximum);
BINARYEN_API void BinaryenRemoveTable(BinaryenModuleRef module,
                                      const char* table);
BINARYEN_API BinaryenIndex BinaryenGetNumTables(BinaryenModuleRef module);
BINARYEN_API BinaryenTableRef BinaryenGetTable(BinaryenModuleRef module,
                                               const char* name);
BINARYEN_API BinaryenTableRef BinaryenGetTableByIndex(BinaryenModuleRef module,
                                                      BinaryenIndex index);

// Elem segments

BINARYEN_REF(ElementSegment);

BINARYEN_API BinaryenElementSegmentRef
BinaryenAddActiveElementSegment(BinaryenModuleRef module,
                                const char* table,
                                const char* name,
                                const char** funcNames,
                                BinaryenIndex numFuncNames,
                                BinaryenExpressionRef offset);
BINARYEN_API BinaryenElementSegmentRef
BinaryenAddPassiveElementSegment(BinaryenModuleRef module,
                                 const char* name,
                                 const char** funcNames,
                                 BinaryenIndex numFuncNames);
BINARYEN_API void BinaryenRemoveElementSegment(BinaryenModuleRef module,
                                               const char* name);
BINARYEN_API BinaryenIndex
BinaryenGetNumElementSegments(BinaryenModuleRef module);
BINARYEN_API BinaryenElementSegmentRef
BinaryenGetElementSegment(BinaryenModuleRef module, const char* name);
BINARYEN_API BinaryenElementSegmentRef
BinaryenGetElementSegmentByIndex(BinaryenModuleRef module, BinaryenIndex index);

// Memory. One per module

// Each memory has data in segments, a start offset in segmentOffsets, and a
// size in segmentSizes. exportName can be NULL
BINARYEN_API void BinaryenSetMemory(BinaryenModuleRef module,
                                    BinaryenIndex initial,
                                    BinaryenIndex maximum,
                                    const char* exportName,
                                    const char** segments,
                                    bool* segmentPassive,
                                    BinaryenExpressionRef* segmentOffsets,
                                    BinaryenIndex* segmentSizes,
                                    BinaryenIndex numSegments,
                                    bool shared);

// Memory segments. Query utilities.

BINARYEN_API uint32_t BinaryenGetNumMemorySegments(BinaryenModuleRef module);
BINARYEN_API uint32_t
BinaryenGetMemorySegmentByteOffset(BinaryenModuleRef module, BinaryenIndex id);
BINARYEN_API size_t BinaryenGetMemorySegmentByteLength(BinaryenModuleRef module,
                                                       BinaryenIndex id);
BINARYEN_API bool BinaryenGetMemorySegmentPassive(BinaryenModuleRef module,
                                                  BinaryenIndex id);
BINARYEN_API void BinaryenCopyMemorySegmentData(BinaryenModuleRef module,
                                                BinaryenIndex id,
                                                char* buffer);

// Start function. One per module

BINARYEN_API void BinaryenSetStart(BinaryenModuleRef module,
                                   BinaryenFunctionRef start);

// Features

// These control what features are allowed when validation and in passes.
BINARYEN_API BinaryenFeatures
BinaryenModuleGetFeatures(BinaryenModuleRef module);
BINARYEN_API void BinaryenModuleSetFeatures(BinaryenModuleRef module,
                                            BinaryenFeatures features);

//
// ========== Module Operations ==========
//

// Parse a module in s-expression text format
BINARYEN_API BinaryenModuleRef BinaryenModuleParse(const char* text);

// Print a module to stdout in s-expression text format. Useful for debugging.
BINARYEN_API void BinaryenModulePrint(BinaryenModuleRef module);

// Print a module to stdout in asm.js syntax.
BINARYEN_API void BinaryenModulePrintAsmjs(BinaryenModuleRef module);

// Validate a module, showing errors on problems.
//  @return 0 if an error occurred, 1 if validated succesfully
BINARYEN_API bool BinaryenModuleValidate(BinaryenModuleRef module);

// Runs the standard optimization passes on the module. Uses the currently set
// global optimize and shrink level.
BINARYEN_API void BinaryenModuleOptimize(BinaryenModuleRef module);

// Gets the currently set optimize level. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -O1, -O2 (default), etc.
BINARYEN_API int BinaryenGetOptimizeLevel(void);

// Sets the optimization level to use. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -O1, -O2 (default), etc.
BINARYEN_API void BinaryenSetOptimizeLevel(int level);

// Gets the currently set shrink level. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -Os (default), -Oz.
BINARYEN_API int BinaryenGetShrinkLevel(void);

// Sets the shrink level to use. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -Os (default), -Oz.
BINARYEN_API void BinaryenSetShrinkLevel(int level);

// Gets whether generating debug information is currently enabled or not.
// Applies to all modules, globally.
BINARYEN_API bool BinaryenGetDebugInfo(void);

// Enables or disables debug information in emitted binaries.
// Applies to all modules, globally.
BINARYEN_API void BinaryenSetDebugInfo(bool on);

// Gets whether the low 1K of memory can be considered unused when optimizing.
// Applies to all modules, globally.
BINARYEN_API bool BinaryenGetLowMemoryUnused(void);

// Enables or disables whether the low 1K of memory can be considered unused
// when optimizing. Applies to all modules, globally.
BINARYEN_API void BinaryenSetLowMemoryUnused(bool on);

// Gets whether fast math optimizations are enabled, ignoring for example
// corner cases of floating-point math like NaN changes.
// Applies to all modules, globally.
BINARYEN_API bool BinaryenGetFastMath(void);

// Enables or disables fast math optimizations, ignoring for example
// corner cases of floating-point math like NaN changes.
// Applies to all modules, globally.
BINARYEN_API void BinaryenSetFastMath(bool value);

// Gets the value of the specified arbitrary pass argument.
// Applies to all modules, globally.
BINARYEN_API const char* BinaryenGetPassArgument(const char* name);

// Sets the value of the specified arbitrary pass argument. Removes the
// respective argument if `value` is NULL. Applies to all modules, globally.
BINARYEN_API void BinaryenSetPassArgument(const char* name, const char* value);

// Clears all arbitrary pass arguments.
// Applies to all modules, globally.
BINARYEN_API void BinaryenClearPassArguments();

// Gets the function size at which we always inline.
// Applies to all modules, globally.
BINARYEN_API BinaryenIndex BinaryenGetAlwaysInlineMaxSize(void);

// Sets the function size at which we always inline.
// Applies to all modules, globally.
BINARYEN_API void BinaryenSetAlwaysInlineMaxSize(BinaryenIndex size);

// Gets the function size which we inline when functions are lightweight.
// Applies to all modules, globally.
BINARYEN_API BinaryenIndex BinaryenGetFlexibleInlineMaxSize(void);

// Sets the function size which we inline when functions are lightweight.
// Applies to all modules, globally.
BINARYEN_API void BinaryenSetFlexibleInlineMaxSize(BinaryenIndex size);

// Gets the function size which we inline when there is only one caller.
// Applies to all modules, globally.
BINARYEN_API BinaryenIndex BinaryenGetOneCallerInlineMaxSize(void);

// Sets the function size which we inline when there is only one caller.
// Applies to all modules, globally.
BINARYEN_API void BinaryenSetOneCallerInlineMaxSize(BinaryenIndex size);

// Gets whether functions with loops are allowed to be inlined.
// Applies to all modules, globally.
BINARYEN_API bool BinaryenGetAllowInliningFunctionsWithLoops(void);

// Sets whether functions with loops are allowed to be inlined.
// Applies to all modules, globally.
BINARYEN_API void BinaryenSetAllowInliningFunctionsWithLoops(bool enabled);

// Runs the specified passes on the module. Uses the currently set global
// optimize and shrink level.
BINARYEN_API void BinaryenModuleRunPasses(BinaryenModuleRef module,
                                          const char** passes,
                                          BinaryenIndex numPasses);

// Auto-generate drop() operations where needed. This lets you generate code
// without worrying about where they are needed. (It is more efficient to do it
// yourself, but simpler to use autodrop).
BINARYEN_API void BinaryenModuleAutoDrop(BinaryenModuleRef module);

// Serialize a module into binary form. Uses the currently set global debugInfo
// option.
// @return how many bytes were written. This will be less than or equal to
//         outputSize
size_t BINARYEN_API BinaryenModuleWrite(BinaryenModuleRef module,
                                        char* output,
                                        size_t outputSize);

// Serialize a module in s-expression text format.
// @return how many bytes were written. This will be less than or equal to
//         outputSize
BINARYEN_API size_t BinaryenModuleWriteText(BinaryenModuleRef module,
                                            char* output,
                                            size_t outputSize);

typedef struct BinaryenBufferSizes {
  size_t outputBytes;
  size_t sourceMapBytes;
} BinaryenBufferSizes;

// Serialize a module into binary form including its source map. Uses the
// currently set global debugInfo option.
// @returns how many bytes were written. This will be less than or equal to
//          outputSize
BINARYEN_API BinaryenBufferSizes
BinaryenModuleWriteWithSourceMap(BinaryenModuleRef module,
                                 const char* url,
                                 char* output,
                                 size_t outputSize,
                                 char* sourceMap,
                                 size_t sourceMapSize);

// Result structure of BinaryenModuleAllocateAndWrite. Contained buffers have
// been allocated using malloc() and the user is expected to free() them
// manually once not needed anymore.
typedef struct BinaryenModuleAllocateAndWriteResult {
  void* binary;
  size_t binaryBytes;
  char* sourceMap;
} BinaryenModuleAllocateAndWriteResult;

// Serializes a module into binary form, optionally including its source map if
// sourceMapUrl has been specified. Uses the currently set global debugInfo
// option. Differs from BinaryenModuleWrite in that it implicitly allocates
// appropriate buffers using malloc(), and expects the user to free() them
// manually once not needed anymore.
BINARYEN_API BinaryenModuleAllocateAndWriteResult
BinaryenModuleAllocateAndWrite(BinaryenModuleRef module,
                               const char* sourceMapUrl);

// Serialize a module in s-expression form. Implicity allocates the returned
// char* with malloc(), and expects the user to free() them manually
// once not needed anymore.
BINARYEN_API char* BinaryenModuleAllocateAndWriteText(BinaryenModuleRef module);

// Deserialize a module from binary form.
BINARYEN_API BinaryenModuleRef BinaryenModuleRead(char* input,
                                                  size_t inputSize);

// Execute a module in the Binaryen interpreter. This will create an instance of
// the module, run it in the interpreter - which means running the start method
// - and then destroying the instance.
BINARYEN_API void BinaryenModuleInterpret(BinaryenModuleRef module);

// Adds a debug info file name to the module and returns its index.
BINARYEN_API BinaryenIndex BinaryenModuleAddDebugInfoFileName(
  BinaryenModuleRef module, const char* filename);

// Gets the name of the debug info file at the specified index. Returns `NULL`
// if it does not exist.
BINARYEN_API const char*
BinaryenModuleGetDebugInfoFileName(BinaryenModuleRef module,
                                   BinaryenIndex index);

//
// ========== Function Operations ==========
//

// Gets the name of the specified `Function`.
BINARYEN_API const char* BinaryenFunctionGetName(BinaryenFunctionRef func);
// Gets the type of the parameter at the specified index of the specified
// `Function`.
BINARYEN_API BinaryenType BinaryenFunctionGetParams(BinaryenFunctionRef func);
// Gets the result type of the specified `Function`.
BINARYEN_API BinaryenType BinaryenFunctionGetResults(BinaryenFunctionRef func);
// Gets the number of additional locals within the specified `Function`.
BINARYEN_API BinaryenIndex BinaryenFunctionGetNumVars(BinaryenFunctionRef func);
// Gets the type of the additional local at the specified index within the
// specified `Function`.
BINARYEN_API BinaryenType BinaryenFunctionGetVar(BinaryenFunctionRef func,
                                                 BinaryenIndex index);
// Gets the number of locals within the specified function. Includes parameters.
BINARYEN_API BinaryenIndex
BinaryenFunctionGetNumLocals(BinaryenFunctionRef func);
// Tests if the local at the specified index has a name.
BINARYEN_API bool BinaryenFunctionHasLocalName(BinaryenFunctionRef func,
                                               BinaryenIndex index);
// Gets the name of the local at the specified index.
BINARYEN_API const char* BinaryenFunctionGetLocalName(BinaryenFunctionRef func,
                                                      BinaryenIndex index);
// Sets the name of the local at the specified index.
BINARYEN_API void BinaryenFunctionSetLocalName(BinaryenFunctionRef func,
                                               BinaryenIndex index,
                                               const char* name);
// Gets the body of the specified `Function`.
BINARYEN_API BinaryenExpressionRef
BinaryenFunctionGetBody(BinaryenFunctionRef func);
// Sets the body of the specified `Function`.
BINARYEN_API void BinaryenFunctionSetBody(BinaryenFunctionRef func,
                                          BinaryenExpressionRef body);

// Runs the standard optimization passes on the function. Uses the currently set
// global optimize and shrink level.
BINARYEN_API void BinaryenFunctionOptimize(BinaryenFunctionRef func,
                                           BinaryenModuleRef module);

// Runs the specified passes on the function. Uses the currently set global
// optimize and shrink level.
BINARYEN_API void BinaryenFunctionRunPasses(BinaryenFunctionRef func,
                                            BinaryenModuleRef module,
                                            const char** passes,
                                            BinaryenIndex numPasses);

// Sets the debug location of the specified `Expression` within the specified
// `Function`.
BINARYEN_API void BinaryenFunctionSetDebugLocation(BinaryenFunctionRef func,
                                                   BinaryenExpressionRef expr,
                                                   BinaryenIndex fileIndex,
                                                   BinaryenIndex lineNumber,
                                                   BinaryenIndex columnNumber);

//
// ========== Table Operations ==========
//

// Gets the name of the specified `Table`.
BINARYEN_API const char* BinaryenTableGetName(BinaryenTableRef table);
// Sets the name of the specified `Table`.
BINARYEN_API void BinaryenTableSetName(BinaryenTableRef table,
                                       const char* name);
// Gets the initial number of pages of the specified `Table`.
BINARYEN_API BinaryenIndex BinaryenTableGetInitial(BinaryenTableRef table);
// Sets the initial number of pages of the specified `Table`.
BINARYEN_API void BinaryenTableSetInitial(BinaryenTableRef table,
                                          BinaryenIndex initial);
// Tests whether the specified `Table` has a maximum number of pages.
BINARYEN_API bool BinaryenTableHasMax(BinaryenTableRef table);
// Gets the maximum number of pages of the specified `Table`.
BINARYEN_API BinaryenIndex BinaryenTableGetMax(BinaryenTableRef table);
// Sets the maximum number of pages of the specified `Table`.
BINARYEN_API void BinaryenTableSetMax(BinaryenTableRef table,
                                      BinaryenIndex max);

//
// ========== Elem Segment Operations ==========
//

// Gets the name of the specified `ElementSegment`.
BINARYEN_API const char*
BinaryenElementSegmentGetName(BinaryenElementSegmentRef elem);
// Sets the name of the specified `ElementSegment`.
BINARYEN_API void BinaryenElementSegmentSetName(BinaryenElementSegmentRef elem,
                                                const char* name);
// Gets the table name of the specified `ElementSegment`.
BINARYEN_API const char*
BinaryenElementSegmentGetTable(BinaryenElementSegmentRef elem);
// Sets the table name of the specified `ElementSegment`.
BINARYEN_API void BinaryenElementSegmentSetTable(BinaryenElementSegmentRef elem,
                                                 const char* table);
// Gets the segment offset in case of active segments
BINARYEN_API BinaryenExpressionRef
BinaryenElementSegmentGetOffset(BinaryenElementSegmentRef elem);
// Gets the length of items in the segment
BINARYEN_API BinaryenIndex
BinaryenElementSegmentGetLength(BinaryenElementSegmentRef elem);
// Gets the item at the specified index
BINARYEN_API const char*
BinaryenElementSegmentGetData(BinaryenElementSegmentRef elem,
                              BinaryenIndex dataId);
// Returns true if the specified elem segment is passive
BINARYEN_API bool
BinaryenElementSegmentIsPassive(BinaryenElementSegmentRef elem);

//
// ========== Global Operations ==========
//

// Gets the name of the specified `Global`.
BINARYEN_API const char* BinaryenGlobalGetName(BinaryenGlobalRef global);
// Gets the name of the `GlobalType` associated with the specified `Global`. May
// be `NULL` if the signature is implicit.
BINARYEN_API BinaryenType BinaryenGlobalGetType(BinaryenGlobalRef global);
// Returns true if the specified `Global` is mutable.
BINARYEN_API bool BinaryenGlobalIsMutable(BinaryenGlobalRef global);
// Gets the initialization expression of the specified `Global`.
BINARYEN_API BinaryenExpressionRef
BinaryenGlobalGetInitExpr(BinaryenGlobalRef global);

//
// ========== Event Operations ==========
//

// Gets the name of the specified `Event`.
BINARYEN_API const char* BinaryenEventGetName(BinaryenEventRef event);
// Gets the attribute of the specified `Event`.
BINARYEN_API uint32_t BinaryenEventGetAttribute(BinaryenEventRef event);
// Gets the parameters type of the specified `Event`.
BINARYEN_API BinaryenType BinaryenEventGetParams(BinaryenEventRef event);
// Gets the results type of the specified `Event`.
BINARYEN_API BinaryenType BinaryenEventGetResults(BinaryenEventRef event);

//
// ========== Import Operations ==========
//

// Gets the external module name of the specified import.
BINARYEN_API const char*
BinaryenFunctionImportGetModule(BinaryenFunctionRef import);
BINARYEN_API const char* BinaryenTableImportGetModule(BinaryenTableRef import);
BINARYEN_API const char*
BinaryenGlobalImportGetModule(BinaryenGlobalRef import);
BINARYEN_API const char* BinaryenEventImportGetModule(BinaryenEventRef import);
// Gets the external base name of the specified import.
BINARYEN_API const char*
BinaryenFunctionImportGetBase(BinaryenFunctionRef import);
BINARYEN_API const char* BinaryenTableImportGetBase(BinaryenTableRef import);
BINARYEN_API const char* BinaryenGlobalImportGetBase(BinaryenGlobalRef import);
BINARYEN_API const char* BinaryenEventImportGetBase(BinaryenEventRef import);

//
// ========== Export Operations ==========
//

// Gets the external kind of the specified export.
BINARYEN_API BinaryenExternalKind
BinaryenExportGetKind(BinaryenExportRef export_);
// Gets the external name of the specified export.
BINARYEN_API const char* BinaryenExportGetName(BinaryenExportRef export_);
// Gets the internal name of the specified export.
BINARYEN_API const char* BinaryenExportGetValue(BinaryenExportRef export_);

//
// ========= Custom sections =========
//

BINARYEN_API void BinaryenAddCustomSection(BinaryenModuleRef module,
                                           const char* name,
                                           const char* contents,
                                           BinaryenIndex contentsSize);

//
// ========= Effect analyzer =========
//

typedef uint32_t BinaryenSideEffects;

BINARYEN_API BinaryenSideEffects BinaryenSideEffectNone(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectBranches(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectCalls(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectReadsLocal(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectWritesLocal(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectReadsGlobal(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectWritesGlobal(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectReadsMemory(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectWritesMemory(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectReadsTable(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectWritesTable(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectReadsElementSegment(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectDropsElementSegment(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectImplicitTrap(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectIsAtomic(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectThrows(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectDanglingPop(void);
BINARYEN_API BinaryenSideEffects BinaryenSideEffectAny(void);

BINARYEN_API BinaryenSideEffects BinaryenExpressionGetSideEffects(
  BinaryenExpressionRef expr, BinaryenFeatures features);

//
// ========== CFG / Relooper ==========
//
// General usage is (1) create a relooper, (2) create blocks, (3) add
// branches between them, (4) render the output.
//
// For more details, see src/cfg/Relooper.h and
// https://github.com/WebAssembly/binaryen/wiki/Compiling-to-WebAssembly-with-Binaryen#cfg-api

#ifdef __cplusplus
namespace CFG {
struct Relooper;
struct Block;
} // namespace CFG
typedef struct CFG::Relooper* RelooperRef;
typedef struct CFG::Block* RelooperBlockRef;
#else
typedef struct Relooper* RelooperRef;
typedef struct RelooperBlock* RelooperBlockRef;
#endif

// Create a relooper instance
BINARYEN_API RelooperRef RelooperCreate(BinaryenModuleRef module);

// Create a basic block that ends with nothing, or with some simple branching
BINARYEN_API RelooperBlockRef RelooperAddBlock(RelooperRef relooper,
                                               BinaryenExpressionRef code);

// Create a branch to another basic block
// The branch can have code on it, that is executed as the branch happens. this
// is useful for phis. otherwise, code can be NULL
BINARYEN_API void RelooperAddBranch(RelooperBlockRef from,
                                    RelooperBlockRef to,
                                    BinaryenExpressionRef condition,
                                    BinaryenExpressionRef code);

// Create a basic block that ends a switch on a condition
BINARYEN_API RelooperBlockRef
RelooperAddBlockWithSwitch(RelooperRef relooper,
                           BinaryenExpressionRef code,
                           BinaryenExpressionRef condition);

// Create a switch-style branch to another basic block. The block's switch table
// will have these indexes going to that target
BINARYEN_API void RelooperAddBranchForSwitch(RelooperBlockRef from,
                                             RelooperBlockRef to,
                                             BinaryenIndex* indexes,
                                             BinaryenIndex numIndexes,
                                             BinaryenExpressionRef code);

// Generate structed wasm control flow from the CFG of blocks and branches that
// were created on this relooper instance. This returns the rendered output, and
// also disposes of the relooper and its blocks and branches, as they are no
// longer needed.
// @param labelHelper To render irreducible control flow, we may need a helper
//        variable to guide us to the right target label. This value should be
//        an index of an i32 local variable that is free for us to use.
BINARYEN_API BinaryenExpressionRef RelooperRenderAndDispose(
  RelooperRef relooper, RelooperBlockRef entry, BinaryenIndex labelHelper);

//
// ========= ExpressionRunner ==========
//

#ifdef __cplusplus
namespace wasm {
class CExpressionRunner;
} // namespace wasm
typedef class wasm::CExpressionRunner* ExpressionRunnerRef;
#else
typedef struct CExpressionRunner* ExpressionRunnerRef;
#endif

typedef uint32_t ExpressionRunnerFlags;

// By default, just evaluate the expression, i.e. all we want to know is whether
// it computes down to a concrete value, where it is not necessary to preserve
// side effects like those of a `local.tee`.
BINARYEN_API ExpressionRunnerFlags ExpressionRunnerFlagsDefault();

// Be very careful to preserve any side effects. For example, if we are
// intending to replace the expression with a constant afterwards, even if we
// can technically evaluate down to a constant, we still cannot replace the
// expression if it also sets a local, which must be preserved in this scenario
// so subsequent code keeps functioning.
BINARYEN_API ExpressionRunnerFlags ExpressionRunnerFlagsPreserveSideeffects();

// Traverse through function calls, attempting to compute their concrete value.
// Must not be used in function-parallel scenarios, where the called function
// might be concurrently modified, leading to undefined behavior. Traversing
// another function reuses all of this runner's flags.
BINARYEN_API ExpressionRunnerFlags ExpressionRunnerFlagsTraverseCalls();

// Creates an ExpressionRunner instance
BINARYEN_API ExpressionRunnerRef
ExpressionRunnerCreate(BinaryenModuleRef module,
                       ExpressionRunnerFlags flags,
                       BinaryenIndex maxDepth,
                       BinaryenIndex maxLoopIterations);

// Sets a known local value to use. Order matters if expressions have side
// effects. For example, if the expression also sets a local, this side effect
// will also happen (not affected by any flags). Returns `true` if the
// expression actually evaluates to a constant.
BINARYEN_API bool ExpressionRunnerSetLocalValue(ExpressionRunnerRef runner,
                                                BinaryenIndex index,
                                                BinaryenExpressionRef value);

// Sets a known global value to use. Order matters if expressions have side
// effects. For example, if the expression also sets a local, this side effect
// will also happen (not affected by any flags). Returns `true` if the
// expression actually evaluates to a constant.
BINARYEN_API bool ExpressionRunnerSetGlobalValue(ExpressionRunnerRef runner,
                                                 const char* name,
                                                 BinaryenExpressionRef value);

// Runs the expression and returns the constant value expression it evaluates
// to, if any. Otherwise returns `NULL`. Also disposes the runner.
BINARYEN_API BinaryenExpressionRef ExpressionRunnerRunAndDispose(
  ExpressionRunnerRef runner, BinaryenExpressionRef expr);

//
// ========= Utilities =========
//

// Enable or disable coloring for the Wasm printer
BINARYEN_API void BinaryenSetColorsEnabled(bool enabled);

// Query whether color is enable for the Wasm printer
BINARYEN_API bool BinaryenAreColorsEnabled();
#ifdef __cplusplus
} // extern "C"
#endif

#endif // wasm_binaryen_c_h
