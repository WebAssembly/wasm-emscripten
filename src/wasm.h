/*
 * Copyright 2015 WebAssembly Community Group participants
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

//
// wasm.h: Define Binaryen IR, a representation for WebAssembly, with
//         all core parts in one simple header file.
//
// For more overview, see README.md
//

#ifndef wasm_wasm_h
#define wasm_wasm_h

#include <algorithm>
#include <array>
#include <cassert>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "literal.h"
#include "mixed_arena.h"
#include "support/name.h"
#include "wasm-features.h"
#include "wasm-type.h"

namespace wasm {

// An index in a wasm module
typedef uint32_t Index;

// An address in linear memory.
struct Address {
  typedef uint32_t address32_t;
  typedef uint64_t address64_t;
  address64_t addr;
  Address() : addr(0) {}
  Address(uint64_t a) : addr(a) {}
  Address& operator=(uint64_t a) {
    addr = a;
    return *this;
  }
  operator address64_t() const { return addr; }
  Address& operator++(int) {
    ++addr;
    return *this;
  }
};

enum class IRProfile { Normal, Poppy };

// Operators

enum UnaryOp {
  // int
  ClzInt32,
  ClzInt64,
  CtzInt32,
  CtzInt64,
  PopcntInt32,
  PopcntInt64,

  // float
  NegFloat32,
  NegFloat64,
  AbsFloat32,
  AbsFloat64,
  CeilFloat32,
  CeilFloat64,
  FloorFloat32,
  FloorFloat64,
  TruncFloat32,
  TruncFloat64,
  NearestFloat32,
  NearestFloat64,
  SqrtFloat32,
  SqrtFloat64,

  // relational
  EqZInt32,
  EqZInt64,

  // conversions
  // extend i32 to i64
  ExtendSInt32,
  ExtendUInt32,
  // i64 to i32
  WrapInt64,
  // float to int
  TruncSFloat32ToInt32,
  TruncSFloat32ToInt64,
  TruncUFloat32ToInt32,
  TruncUFloat32ToInt64,
  TruncSFloat64ToInt32,
  TruncSFloat64ToInt64,
  TruncUFloat64ToInt32,
  TruncUFloat64ToInt64,
  // reintepret bits to int
  ReinterpretFloat32,
  ReinterpretFloat64,
  // int to float
  ConvertSInt32ToFloat32,
  ConvertSInt32ToFloat64,
  ConvertUInt32ToFloat32,
  ConvertUInt32ToFloat64,
  ConvertSInt64ToFloat32,
  ConvertSInt64ToFloat64,
  ConvertUInt64ToFloat32,
  ConvertUInt64ToFloat64,
  // f32 to f64
  PromoteFloat32,
  // f64 to f32
  DemoteFloat64,
  // reinterpret bits to float
  ReinterpretInt32,
  ReinterpretInt64,

  // Extend signed subword-sized integer. This differs from e.g. ExtendSInt32
  // because the input integer is in an i64 value insetad of an i32 value.
  ExtendS8Int32,
  ExtendS16Int32,
  ExtendS8Int64,
  ExtendS16Int64,
  ExtendS32Int64,

  // Saturating float-to-int
  TruncSatSFloat32ToInt32,
  TruncSatUFloat32ToInt32,
  TruncSatSFloat64ToInt32,
  TruncSatUFloat64ToInt32,
  TruncSatSFloat32ToInt64,
  TruncSatUFloat32ToInt64,
  TruncSatSFloat64ToInt64,
  TruncSatUFloat64ToInt64,

  // SIMD splats
  SplatVecI8x16,
  SplatVecI16x8,
  SplatVecI32x4,
  SplatVecI64x2,
  SplatVecF32x4,
  SplatVecF64x2,

  // SIMD arithmetic
  NotVec128,
  AnyTrueVec128,
  AbsVecI8x16,
  NegVecI8x16,
  AllTrueVecI8x16,
  BitmaskVecI8x16,
  PopcntVecI8x16,
  AbsVecI16x8,
  NegVecI16x8,
  AllTrueVecI16x8,
  BitmaskVecI16x8,
  AbsVecI32x4,
  NegVecI32x4,
  AllTrueVecI32x4,
  BitmaskVecI32x4,
  AbsVecI64x2,
  NegVecI64x2,
  AllTrueVecI64x2,
  BitmaskVecI64x2,
  AbsVecF32x4,
  NegVecF32x4,
  SqrtVecF32x4,
  CeilVecF32x4,
  FloorVecF32x4,
  TruncVecF32x4,
  NearestVecF32x4,
  AbsVecF64x2,
  NegVecF64x2,
  SqrtVecF64x2,
  CeilVecF64x2,
  FloorVecF64x2,
  TruncVecF64x2,
  NearestVecF64x2,
  ExtAddPairwiseSVecI8x16ToI16x8,
  ExtAddPairwiseUVecI8x16ToI16x8,
  ExtAddPairwiseSVecI16x8ToI32x4,
  ExtAddPairwiseUVecI16x8ToI32x4,

  // SIMD conversions
  TruncSatSVecF32x4ToVecI32x4,
  TruncSatUVecF32x4ToVecI32x4,
  ConvertSVecI32x4ToVecF32x4,
  ConvertUVecI32x4ToVecF32x4,
  ExtendLowSVecI8x16ToVecI16x8,
  ExtendHighSVecI8x16ToVecI16x8,
  ExtendLowUVecI8x16ToVecI16x8,
  ExtendHighUVecI8x16ToVecI16x8,
  ExtendLowSVecI16x8ToVecI32x4,
  ExtendHighSVecI16x8ToVecI32x4,
  ExtendLowUVecI16x8ToVecI32x4,
  ExtendHighUVecI16x8ToVecI32x4,
  ExtendLowSVecI32x4ToVecI64x2,
  ExtendHighSVecI32x4ToVecI64x2,
  ExtendLowUVecI32x4ToVecI64x2,
  ExtendHighUVecI32x4ToVecI64x2,

  ConvertLowSVecI32x4ToVecF64x2,
  ConvertLowUVecI32x4ToVecF64x2,
  TruncSatZeroSVecF64x2ToVecI32x4,
  TruncSatZeroUVecF64x2ToVecI32x4,
  DemoteZeroVecF64x2ToVecF32x4,
  PromoteLowVecF32x4ToVecF64x2,

  InvalidUnary
};

enum BinaryOp {
  // int or float
  AddInt32,
  SubInt32,
  MulInt32,

  // int
  DivSInt32,
  DivUInt32,
  RemSInt32,
  RemUInt32,
  AndInt32,
  OrInt32,
  XorInt32,
  ShlInt32,
  ShrSInt32,
  ShrUInt32,
  RotLInt32,
  RotRInt32,

  // relational ops
  // int or float
  EqInt32,
  NeInt32,
  // int
  LtSInt32,
  LtUInt32,
  LeSInt32,
  LeUInt32,
  GtSInt32,
  GtUInt32,
  GeSInt32,
  GeUInt32,

  // int or float
  AddInt64,
  SubInt64,
  MulInt64,

  // int
  DivSInt64,
  DivUInt64,
  RemSInt64,
  RemUInt64,
  AndInt64,
  OrInt64,
  XorInt64,
  ShlInt64,
  ShrSInt64,
  ShrUInt64,
  RotLInt64,
  RotRInt64,

  // relational ops
  // int or float
  EqInt64,
  NeInt64,
  // int
  LtSInt64,
  LtUInt64,
  LeSInt64,
  LeUInt64,
  GtSInt64,
  GtUInt64,
  GeSInt64,
  GeUInt64,

  // int or float
  AddFloat32,
  SubFloat32,
  MulFloat32,

  // float
  DivFloat32,
  CopySignFloat32,
  MinFloat32,
  MaxFloat32,

  // relational ops
  // int or float
  EqFloat32,
  NeFloat32,
  // float
  LtFloat32,
  LeFloat32,
  GtFloat32,
  GeFloat32,

  // int or float
  AddFloat64,
  SubFloat64,
  MulFloat64,

  // float
  DivFloat64,
  CopySignFloat64,
  MinFloat64,
  MaxFloat64,

  // relational ops
  // int or float
  EqFloat64,
  NeFloat64,
  // float
  LtFloat64,
  LeFloat64,
  GtFloat64,
  GeFloat64,

  // SIMD relational ops (return vectors)
  EqVecI8x16,
  NeVecI8x16,
  LtSVecI8x16,
  LtUVecI8x16,
  GtSVecI8x16,
  GtUVecI8x16,
  LeSVecI8x16,
  LeUVecI8x16,
  GeSVecI8x16,
  GeUVecI8x16,
  EqVecI16x8,
  NeVecI16x8,
  LtSVecI16x8,
  LtUVecI16x8,
  GtSVecI16x8,
  GtUVecI16x8,
  LeSVecI16x8,
  LeUVecI16x8,
  GeSVecI16x8,
  GeUVecI16x8,
  EqVecI32x4,
  NeVecI32x4,
  LtSVecI32x4,
  LtUVecI32x4,
  GtSVecI32x4,
  GtUVecI32x4,
  LeSVecI32x4,
  LeUVecI32x4,
  GeSVecI32x4,
  GeUVecI32x4,
  EqVecI64x2,
  NeVecI64x2,
  LtSVecI64x2,
  GtSVecI64x2,
  LeSVecI64x2,
  GeSVecI64x2,
  EqVecF32x4,
  NeVecF32x4,
  LtVecF32x4,
  GtVecF32x4,
  LeVecF32x4,
  GeVecF32x4,
  EqVecF64x2,
  NeVecF64x2,
  LtVecF64x2,
  GtVecF64x2,
  LeVecF64x2,
  GeVecF64x2,

  // SIMD arithmetic
  AndVec128,
  OrVec128,
  XorVec128,
  AndNotVec128,
  AddVecI8x16,
  AddSatSVecI8x16,
  AddSatUVecI8x16,
  SubVecI8x16,
  SubSatSVecI8x16,
  SubSatUVecI8x16,
  MinSVecI8x16,
  MinUVecI8x16,
  MaxSVecI8x16,
  MaxUVecI8x16,
  AvgrUVecI8x16,
  AddVecI16x8,
  AddSatSVecI16x8,
  AddSatUVecI16x8,
  SubVecI16x8,
  SubSatSVecI16x8,
  SubSatUVecI16x8,
  MulVecI16x8,
  MinSVecI16x8,
  MinUVecI16x8,
  MaxSVecI16x8,
  MaxUVecI16x8,
  AvgrUVecI16x8,
  Q15MulrSatSVecI16x8,
  ExtMulLowSVecI16x8,
  ExtMulHighSVecI16x8,
  ExtMulLowUVecI16x8,
  ExtMulHighUVecI16x8,
  AddVecI32x4,
  SubVecI32x4,
  MulVecI32x4,
  MinSVecI32x4,
  MinUVecI32x4,
  MaxSVecI32x4,
  MaxUVecI32x4,
  DotSVecI16x8ToVecI32x4,
  ExtMulLowSVecI32x4,
  ExtMulHighSVecI32x4,
  ExtMulLowUVecI32x4,
  ExtMulHighUVecI32x4,
  AddVecI64x2,
  SubVecI64x2,
  MulVecI64x2,
  ExtMulLowSVecI64x2,
  ExtMulHighSVecI64x2,
  ExtMulLowUVecI64x2,
  ExtMulHighUVecI64x2,
  AddVecF32x4,
  SubVecF32x4,
  MulVecF32x4,
  DivVecF32x4,
  MinVecF32x4,
  MaxVecF32x4,
  PMinVecF32x4,
  PMaxVecF32x4,
  AddVecF64x2,
  SubVecF64x2,
  MulVecF64x2,
  DivVecF64x2,
  MinVecF64x2,
  MaxVecF64x2,
  PMinVecF64x2,
  PMaxVecF64x2,

  // SIMD Conversion
  NarrowSVecI16x8ToVecI8x16,
  NarrowUVecI16x8ToVecI8x16,
  NarrowSVecI32x4ToVecI16x8,
  NarrowUVecI32x4ToVecI16x8,

  // SIMD Swizzle
  SwizzleVec8x16,

  InvalidBinary
};

enum AtomicRMWOp { RMWAdd, RMWSub, RMWAnd, RMWOr, RMWXor, RMWXchg };

enum SIMDExtractOp {
  ExtractLaneSVecI8x16,
  ExtractLaneUVecI8x16,
  ExtractLaneSVecI16x8,
  ExtractLaneUVecI16x8,
  ExtractLaneVecI32x4,
  ExtractLaneVecI64x2,
  ExtractLaneVecF32x4,
  ExtractLaneVecF64x2
};

enum SIMDReplaceOp {
  ReplaceLaneVecI8x16,
  ReplaceLaneVecI16x8,
  ReplaceLaneVecI32x4,
  ReplaceLaneVecI64x2,
  ReplaceLaneVecF32x4,
  ReplaceLaneVecF64x2,
};

enum SIMDShiftOp {
  ShlVecI8x16,
  ShrSVecI8x16,
  ShrUVecI8x16,
  ShlVecI16x8,
  ShrSVecI16x8,
  ShrUVecI16x8,
  ShlVecI32x4,
  ShrSVecI32x4,
  ShrUVecI32x4,
  ShlVecI64x2,
  ShrSVecI64x2,
  ShrUVecI64x2
};

enum SIMDLoadOp {
  Load8SplatVec128,
  Load16SplatVec128,
  Load32SplatVec128,
  Load64SplatVec128,
  Load8x8SVec128,
  Load8x8UVec128,
  Load16x4SVec128,
  Load16x4UVec128,
  Load32x2SVec128,
  Load32x2UVec128,
  Load32ZeroVec128,
  Load64ZeroVec128,
};

enum SIMDLoadStoreLaneOp {
  Load8LaneVec128,
  Load16LaneVec128,
  Load32LaneVec128,
  Load64LaneVec128,
  Store8LaneVec128,
  Store16LaneVec128,
  Store32LaneVec128,
  Store64LaneVec128,
};

enum SIMDTernaryOp {
  Bitselect,
};

enum RefIsOp {
  RefIsNull,
  RefIsFunc,
  RefIsData,
  RefIsI31,
};

enum RefAsOp {
  RefAsNonNull,
  RefAsFunc,
  RefAsData,
  RefAsI31,
};

enum BrOnOp {
  BrOnNull,
  BrOnNonNull,
  BrOnCast,
  BrOnCastFail,
  BrOnFunc,
  BrOnNonFunc,
  BrOnData,
  BrOnNonData,
  BrOnI31,
  BrOnNonI31,
};

//
// Expressions
//
// Note that little is provided in terms of constructors for these. The
// rationale is that writing  new Something(a, b, c, d, e)  is not the clearest,
// and it would be better to write   new Something(name=a, leftOperand=b...
// etc., but C++ lacks named operands, so in asm2wasm etc. you will see things
// like
//   auto x = new Something();
//   x->name = a;
//   x->leftOperand = b;
//   ..
// which is less compact but less ambiguous. See wasm-builder.h for a more
// friendly API for building nodes.
//
// Most nodes have no need of internal allocation, and when arena-allocated
// they drop the provided arena on the floor. You can create random instances
// of those that are not in an arena without issue. However, the nodes that
// have internal allocation will need an allocator provided to them in order
// to be constructed.

class Expression {
public:
  enum Id {
    InvalidId = 0,
    BlockId,
    IfId,
    LoopId,
    BreakId,
    SwitchId,
    CallId,
    CallIndirectId,
    LocalGetId,
    LocalSetId,
    GlobalGetId,
    GlobalSetId,
    LoadId,
    StoreId,
    ConstId,
    UnaryId,
    BinaryId,
    SelectId,
    DropId,
    ReturnId,
    MemorySizeId,
    MemoryGrowId,
    NopId,
    UnreachableId,
    AtomicRMWId,
    AtomicCmpxchgId,
    AtomicWaitId,
    AtomicNotifyId,
    AtomicFenceId,
    SIMDExtractId,
    SIMDReplaceId,
    SIMDShuffleId,
    SIMDTernaryId,
    SIMDShiftId,
    SIMDLoadId,
    SIMDLoadStoreLaneId,
    MemoryInitId,
    DataDropId,
    MemoryCopyId,
    MemoryFillId,
    PopId,
    RefNullId,
    RefIsId,
    RefFuncId,
    RefEqId,
    TryId,
    ThrowId,
    RethrowId,
    TupleMakeId,
    TupleExtractId,
    I31NewId,
    I31GetId,
    CallRefId,
    RefTestId,
    RefCastId,
    BrOnId,
    RttCanonId,
    RttSubId,
    StructNewId,
    StructGetId,
    StructSetId,
    ArrayNewId,
    ArrayGetId,
    ArraySetId,
    ArrayLenId,
    ArrayCopyId,
    RefAsId,
    TableGetId,
    TableSetId,
    TableSizeId,
    TableGrowId,
    TableFillId,
    TableCopyId,
    TableInitId,
    ElemDropId,
    NumExpressionIds
  };
  Id _id;

  // the type of the expression: its *output*, not necessarily its input(s)
  Type type = Type::none;

  Expression(Id id) : _id(id) {}

  void finalize() {}

  template<class T> bool is() const {
    static_assert(std::is_base_of<Expression, T>::value,
                  "Expression is not a base of destination type T");
    return int(_id) == int(T::SpecificId);
  }

  template<class T> T* dynCast() {
    static_assert(std::is_base_of<Expression, T>::value,
                  "Expression is not a base of destination type T");
    return int(_id) == int(T::SpecificId) ? (T*)this : nullptr;
  }

  template<class T> const T* dynCast() const {
    static_assert(std::is_base_of<Expression, T>::value,
                  "Expression is not a base of destination type T");
    return int(_id) == int(T::SpecificId) ? (const T*)this : nullptr;
  }

  template<class T> T* cast() {
    static_assert(std::is_base_of<Expression, T>::value,
                  "Expression is not a base of destination type T");
    assert(int(_id) == int(T::SpecificId));
    return (T*)this;
  }

  template<class T> const T* cast() const {
    static_assert(std::is_base_of<Expression, T>::value,
                  "Expression is not a base of destination type T");
    assert(int(_id) == int(T::SpecificId));
    return (const T*)this;
  }

  // Print the expression to stderr. Meant for use while debugging.
  void dump();
};

const char* getExpressionName(Expression* curr);

Literal getLiteralFromConstExpression(Expression* curr);
Literals getLiteralsFromConstExpression(Expression* curr);

typedef ArenaVector<Expression*> ExpressionList;

template<Expression::Id SID> class SpecificExpression : public Expression {
public:
  enum {
    SpecificId = SID // compile-time access to the type for the class
  };

  SpecificExpression() : Expression(SID) {}
};

class Nop : public SpecificExpression<Expression::NopId> {
public:
  Nop() = default;
  Nop(MixedArena& allocator) {}
};

class Block : public SpecificExpression<Expression::BlockId> {
public:
  Block(MixedArena& allocator) : list(allocator) {}

  Name name;
  ExpressionList list;

  // set the type purely based on its contents. this scans the block, so it is
  // not fast.
  void finalize();

  // set the type given you know its type, which is the case when parsing
  // s-expression or binary, as explicit types are given. the only additional
  // work this does is to set the type to unreachable in the cases that is
  // needed (which may require scanning the block)
  void finalize(Type type_);

  enum Breakability { Unknown, HasBreak, NoBreak };

  // set the type given you know its type, and you know if there is a break to
  // this block. this avoids the need to scan the contents of the block in the
  // case that it might be unreachable, so it is recommended if you already know
  // the type and breakability anyhow.
  void finalize(Type type_, Breakability breakability);
};

class If : public SpecificExpression<Expression::IfId> {
public:
  If() : ifFalse(nullptr) {}
  If(MixedArena& allocator) : If() {}

  Expression* condition;
  Expression* ifTrue;
  Expression* ifFalse;

  // set the type given you know its type, which is the case when parsing
  // s-expression or binary, as explicit types are given. the only additional
  // work this does is to set the type to unreachable in the cases that is
  // needed.
  void finalize(Type type_);

  // set the type purely based on its contents.
  void finalize();
};

class Loop : public SpecificExpression<Expression::LoopId> {
public:
  Loop() = default;
  Loop(MixedArena& allocator) {}

  Name name;
  Expression* body;

  // set the type given you know its type, which is the case when parsing
  // s-expression or binary, as explicit types are given. the only additional
  // work this does is to set the type to unreachable in the cases that is
  // needed.
  void finalize(Type type_);

  // set the type purely based on its contents.
  void finalize();
};

class Break : public SpecificExpression<Expression::BreakId> {
public:
  Break() : value(nullptr), condition(nullptr) {}
  Break(MixedArena& allocator) : Break() { type = Type::unreachable; }

  Name name;
  Expression* value;
  Expression* condition;

  void finalize();
};

class Switch : public SpecificExpression<Expression::SwitchId> {
public:
  Switch(MixedArena& allocator) : targets(allocator) {
    type = Type::unreachable;
  }

  ArenaVector<Name> targets;
  Name default_;
  Expression* condition = nullptr;
  Expression* value = nullptr;

  void finalize();
};

class Call : public SpecificExpression<Expression::CallId> {
public:
  Call(MixedArena& allocator) : operands(allocator) {}

  ExpressionList operands;
  Name target;
  bool isReturn = false;

  void finalize();
};

class CallIndirect : public SpecificExpression<Expression::CallIndirectId> {
public:
  CallIndirect(MixedArena& allocator) : operands(allocator) {}
  Signature sig;
  ExpressionList operands;
  Expression* target;
  Name table;
  bool isReturn = false;

  void finalize();
};

class LocalGet : public SpecificExpression<Expression::LocalGetId> {
public:
  LocalGet() = default;
  LocalGet(MixedArena& allocator) {}

  Index index;
};

class LocalSet : public SpecificExpression<Expression::LocalSetId> {
public:
  LocalSet() = default;
  LocalSet(MixedArena& allocator) {}

  void finalize();

  Index index;
  Expression* value;

  bool isTee() const;
  void makeTee(Type type);
  void makeSet();
};

class GlobalGet : public SpecificExpression<Expression::GlobalGetId> {
public:
  GlobalGet() = default;
  GlobalGet(MixedArena& allocator) {}

  Name name;
};

class GlobalSet : public SpecificExpression<Expression::GlobalSetId> {
public:
  GlobalSet() = default;
  GlobalSet(MixedArena& allocator) {}

  Name name;
  Expression* value;

  void finalize();
};

class Load : public SpecificExpression<Expression::LoadId> {
public:
  Load() = default;
  Load(MixedArena& allocator) {}

  uint8_t bytes;
  bool signed_;
  Address offset;
  Address align;
  bool isAtomic;
  Expression* ptr;

  // type must be set during creation, cannot be inferred

  void finalize();
};

class Store : public SpecificExpression<Expression::StoreId> {
public:
  Store() = default;
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
public:
  AtomicRMW() = default;
  AtomicRMW(MixedArena& allocator) : AtomicRMW() {}

  AtomicRMWOp op;
  uint8_t bytes;
  Address offset;
  Expression* ptr;
  Expression* value;

  void finalize();
};

class AtomicCmpxchg : public SpecificExpression<Expression::AtomicCmpxchgId> {
public:
  AtomicCmpxchg() = default;
  AtomicCmpxchg(MixedArena& allocator) : AtomicCmpxchg() {}

  uint8_t bytes;
  Address offset;
  Expression* ptr;
  Expression* expected;
  Expression* replacement;

  void finalize();
};

class AtomicWait : public SpecificExpression<Expression::AtomicWaitId> {
public:
  AtomicWait() = default;
  AtomicWait(MixedArena& allocator) : AtomicWait() {}

  Address offset;
  Expression* ptr;
  Expression* expected;
  Expression* timeout;
  Type expectedType;

  void finalize();
};

class AtomicNotify : public SpecificExpression<Expression::AtomicNotifyId> {
public:
  AtomicNotify() = default;
  AtomicNotify(MixedArena& allocator) : AtomicNotify() {}

  Address offset;
  Expression* ptr;
  Expression* notifyCount;

  void finalize();
};

class AtomicFence : public SpecificExpression<Expression::AtomicFenceId> {
public:
  AtomicFence() = default;
  AtomicFence(MixedArena& allocator) : AtomicFence() {}

  // Current wasm threads only supports sequentialy consistent atomics, but
  // other orderings may be added in the future. This field is reserved for
  // that, and currently set to 0.
  uint8_t order = 0;

  void finalize();
};

class SIMDExtract : public SpecificExpression<Expression::SIMDExtractId> {
public:
  SIMDExtract() = default;
  SIMDExtract(MixedArena& allocator) : SIMDExtract() {}

  SIMDExtractOp op;
  Expression* vec;
  uint8_t index;

  void finalize();
};

class SIMDReplace : public SpecificExpression<Expression::SIMDReplaceId> {
public:
  SIMDReplace() = default;
  SIMDReplace(MixedArena& allocator) : SIMDReplace() {}

  SIMDReplaceOp op;
  Expression* vec;
  uint8_t index;
  Expression* value;

  void finalize();
};

class SIMDShuffle : public SpecificExpression<Expression::SIMDShuffleId> {
public:
  SIMDShuffle() = default;
  SIMDShuffle(MixedArena& allocator) : SIMDShuffle() {}

  Expression* left;
  Expression* right;
  std::array<uint8_t, 16> mask;

  void finalize();
};

class SIMDTernary : public SpecificExpression<Expression::SIMDTernaryId> {
public:
  SIMDTernary() = default;
  SIMDTernary(MixedArena& allocator) : SIMDTernary() {}

  SIMDTernaryOp op;
  Expression* a;
  Expression* b;
  Expression* c;

  void finalize();
};

class SIMDShift : public SpecificExpression<Expression::SIMDShiftId> {
public:
  SIMDShift() = default;
  SIMDShift(MixedArena& allocator) : SIMDShift() {}

  SIMDShiftOp op;
  Expression* vec;
  Expression* shift;

  void finalize();
};

class SIMDLoad : public SpecificExpression<Expression::SIMDLoadId> {
public:
  SIMDLoad() = default;
  SIMDLoad(MixedArena& allocator) {}

  SIMDLoadOp op;
  Address offset;
  Address align;
  Expression* ptr;

  Index getMemBytes();
  void finalize();
};

class SIMDLoadStoreLane
  : public SpecificExpression<Expression::SIMDLoadStoreLaneId> {
public:
  SIMDLoadStoreLane() = default;
  SIMDLoadStoreLane(MixedArena& allocator) {}

  SIMDLoadStoreLaneOp op;
  Address offset;
  Address align;
  uint8_t index;
  Expression* ptr;
  Expression* vec;

  bool isStore();
  bool isLoad() { return !isStore(); }
  Index getMemBytes();
  void finalize();
};

class MemoryInit : public SpecificExpression<Expression::MemoryInitId> {
public:
  MemoryInit() = default;
  MemoryInit(MixedArena& allocator) : MemoryInit() {}

  Index segment;
  Expression* dest;
  Expression* offset;
  Expression* size;

  void finalize();
};

class DataDrop : public SpecificExpression<Expression::DataDropId> {
public:
  DataDrop() = default;
  DataDrop(MixedArena& allocator) : DataDrop() {}

  Index segment;

  void finalize();
};

class MemoryCopy : public SpecificExpression<Expression::MemoryCopyId> {
public:
  MemoryCopy() = default;
  MemoryCopy(MixedArena& allocator) : MemoryCopy() {}

  Expression* dest;
  Expression* source;
  Expression* size;

  void finalize();
};

class MemoryFill : public SpecificExpression<Expression::MemoryFillId> {
public:
  MemoryFill() = default;
  MemoryFill(MixedArena& allocator) : MemoryFill() {}

  Expression* dest;
  Expression* value;
  Expression* size;

  void finalize();
};

class Const : public SpecificExpression<Expression::ConstId> {
public:
  Const() = default;
  Const(MixedArena& allocator) {}

  Literal value;

  Const* set(Literal value_);

  void finalize();
};

class Unary : public SpecificExpression<Expression::UnaryId> {
public:
  Unary() = default;
  Unary(MixedArena& allocator) {}

  UnaryOp op;
  Expression* value;

  bool isRelational();

  void finalize();
};

class Binary : public SpecificExpression<Expression::BinaryId> {
public:
  Binary() = default;
  Binary(MixedArena& allocator) {}

  BinaryOp op;
  Expression* left;
  Expression* right;

  // the type is always the type of the operands,
  // except for relationals

  bool isRelational();

  void finalize();
};

class Select : public SpecificExpression<Expression::SelectId> {
public:
  Select() = default;
  Select(MixedArena& allocator) {}

  Expression* ifTrue;
  Expression* ifFalse;
  Expression* condition;

  void finalize();
  void finalize(Type type_);
};

class Drop : public SpecificExpression<Expression::DropId> {
public:
  Drop() = default;
  Drop(MixedArena& allocator) {}

  Expression* value;

  void finalize();
};

class Return : public SpecificExpression<Expression::ReturnId> {
public:
  Return() { type = Type::unreachable; }
  Return(MixedArena& allocator) : Return() {}

  Expression* value = nullptr;
};

class MemorySize : public SpecificExpression<Expression::MemorySizeId> {
public:
  MemorySize() { type = Type::i32; }
  MemorySize(MixedArena& allocator) : MemorySize() {}

  Type ptrType = Type::i32;

  void make64();
  void finalize();
};

class MemoryGrow : public SpecificExpression<Expression::MemoryGrowId> {
public:
  MemoryGrow() { type = Type::i32; }
  MemoryGrow(MixedArena& allocator) : MemoryGrow() {}

  Expression* delta = nullptr;
  Type ptrType = Type::i32;

  void make64();
  void finalize();
};

class TableGet : public SpecificExpression<Expression::TableGetId> {
public:
  TableGet(MixedArena& allocator) {}

  Name table;
  Expression* offset = nullptr;

  void finalize();
  void finalize(Type type_);
};

class TableSet : public SpecificExpression<Expression::TableSetId> {
public:
  TableSet(MixedArena& allocator) {}

  Name table;
  Expression* offset = nullptr;
  Expression* value = nullptr;

  void finalize();
};

class TableSize : public SpecificExpression<Expression::TableSizeId> {
public:
  TableSize() { type = Type::i32; }
  TableSize(MixedArena& allocator) : TableSize() {}

  Name table;

  void finalize();
};

class TableGrow : public SpecificExpression<Expression::TableGrowId> {
public:
  TableGrow(MixedArena& allocator) {}

  Name table;
  Expression* delta = nullptr;
  Expression* initialValue = nullptr;

  void finalize();
};

class TableFill : public SpecificExpression<Expression::TableFillId> {
public:
  TableFill(MixedArena& allocator) {}

  Name table;
  Expression* dest;
  Expression* value;
  Expression* size;

  void finalize();
};

class TableCopy : public SpecificExpression<Expression::TableCopyId> {
public:
  TableCopy(MixedArena& allocator) {}

  Name srcTable;
  Name destTable;

  Expression* srcOffset = nullptr;
  Expression* destOffset = nullptr;
  Expression* size = nullptr;

  void finalize();
};

class TableInit : public SpecificExpression<Expression::TableInitId> {
public:
  TableInit() = default;
  TableInit(MixedArena& allocator) {}

  Name table;
  Name segment;

  Expression* srcOffset = nullptr;
  Expression* destOffset = nullptr;
  Expression* size = nullptr;

  void finalize();
};

class ElemDrop : public SpecificExpression<Expression::ElemDropId> {
public:
  ElemDrop() = default;
  ElemDrop(MixedArena& allocator) {}

  Name segment;

  void finalize();
};

class Unreachable : public SpecificExpression<Expression::UnreachableId> {
public:
  Unreachable() { type = Type::unreachable; }
  Unreachable(MixedArena& allocator) : Unreachable() {}
};

// Represents a pop of a value that arrives as an implicit argument to the
// current block. Currently used in exception handling.
class Pop : public SpecificExpression<Expression::PopId> {
public:
  Pop() = default;
  Pop(MixedArena& allocator) {}
};

class RefNull : public SpecificExpression<Expression::RefNullId> {
public:
  RefNull() = default;
  RefNull(MixedArena& allocator) {}

  void finalize();
  void finalize(HeapType heapType);
  void finalize(Type type);
};

class RefIs : public SpecificExpression<Expression::RefIsId> {
public:
  RefIs(MixedArena& allocator) {}

  // RefIs can represent ref.is_null, ref.is_func, ref.is_data, and ref.is_i31.
  RefIsOp op;

  Expression* value;

  void finalize();
};

class RefFunc : public SpecificExpression<Expression::RefFuncId> {
public:
  RefFunc(MixedArena& allocator) {}

  Name func;

  void finalize();
  void finalize(Type type_);
};

class RefEq : public SpecificExpression<Expression::RefEqId> {
public:
  RefEq(MixedArena& allocator) {}

  Expression* left;
  Expression* right;

  void finalize();
};

class Try : public SpecificExpression<Expression::TryId> {
public:
  Try(MixedArena& allocator) : catchEvents(allocator), catchBodies(allocator) {}

  Name name; // label that can only be targeted by 'delegate's
  Expression* body;
  ArenaVector<Name> catchEvents;
  ExpressionList catchBodies;
  Name delegateTarget; // target try's label

  bool hasCatchAll() const {
    return catchBodies.size() - catchEvents.size() == 1;
  }
  bool isCatch() const { return !catchBodies.empty(); }
  bool isDelegate() const { return delegateTarget.is(); }
  void finalize();
  void finalize(Type type_);
};

class Throw : public SpecificExpression<Expression::ThrowId> {
public:
  Throw(MixedArena& allocator) : operands(allocator) {}

  Name event;
  ExpressionList operands;

  void finalize();
};

class Rethrow : public SpecificExpression<Expression::RethrowId> {
public:
  Rethrow(MixedArena& allocator) {}

  Name target;

  void finalize();
};

class TupleMake : public SpecificExpression<Expression::TupleMakeId> {
public:
  TupleMake(MixedArena& allocator) : operands(allocator) {}

  ExpressionList operands;

  void finalize();
};

class TupleExtract : public SpecificExpression<Expression::TupleExtractId> {
public:
  TupleExtract(MixedArena& allocator) {}

  Expression* tuple;
  Index index;

  void finalize();
};

class I31New : public SpecificExpression<Expression::I31NewId> {
public:
  I31New(MixedArena& allocator) {}

  Expression* value;

  void finalize();
};

class I31Get : public SpecificExpression<Expression::I31GetId> {
public:
  I31Get(MixedArena& allocator) {}

  Expression* i31;
  bool signed_;

  void finalize();
};

class CallRef : public SpecificExpression<Expression::CallRefId> {
public:
  CallRef(MixedArena& allocator) : operands(allocator) {}
  ExpressionList operands;
  Expression* target;
  bool isReturn = false;

  void finalize();
  void finalize(Type type_);
};

class RefTest : public SpecificExpression<Expression::RefTestId> {
public:
  RefTest(MixedArena& allocator) {}

  Expression* ref;
  Expression* rtt;

  void finalize();
};

class RefCast : public SpecificExpression<Expression::RefCastId> {
public:
  RefCast(MixedArena& allocator) {}

  Expression* ref;
  Expression* rtt;

  void finalize();
};

class BrOn : public SpecificExpression<Expression::BrOnId> {
public:
  BrOn(MixedArena& allocator) {}

  BrOnOp op;
  Name name;
  Expression* ref;

  // BrOnCast* has an rtt that is used in the cast.
  Expression* rtt;

  // TODO: BrOnNull also has an optional extra value in the spec, which we do
  //       not support. See also the discussion on
  //       https://github.com/WebAssembly/function-references/issues/45
  //       - depending on the decision there, we may want to move BrOnNull into
  //       Break or a new class of its own.

  void finalize();

  // Returns the type sent on the branch, if it is taken.
  Type getSentType();
};

class RttCanon : public SpecificExpression<Expression::RttCanonId> {
public:
  RttCanon(MixedArena& allocator) {}

  void finalize();
};

class RttSub : public SpecificExpression<Expression::RttSubId> {
public:
  RttSub(MixedArena& allocator) {}

  Expression* parent;

  // rtt.fresh_sub is like rtt.sub, but never caching or canonicalizing (i.e.,
  // it always returns a fresh RTT, non-identical to any other RTT in the
  // system).
  bool fresh = false;

  void finalize();
};

class StructNew : public SpecificExpression<Expression::StructNewId> {
public:
  StructNew(MixedArena& allocator) : operands(allocator) {}

  Expression* rtt;
  // A struct.new_with_default has empty operands. This does leave the case of a
  // struct with no fields ambiguous, but it doesn't make a difference in that
  // case, and binaryen doesn't guarantee roundtripping binaries anyhow.
  ExpressionList operands;

  bool isWithDefault() { return operands.empty(); }

  void finalize();
};

class StructGet : public SpecificExpression<Expression::StructGetId> {
public:
  StructGet(MixedArena& allocator) {}

  Index index;
  Expression* ref;
  // Packed fields have a sign.
  bool signed_ = false;

  void finalize();
};

class StructSet : public SpecificExpression<Expression::StructSetId> {
public:
  StructSet(MixedArena& allocator) {}

  Index index;
  Expression* ref;
  Expression* value;

  void finalize();
};

class ArrayNew : public SpecificExpression<Expression::ArrayNewId> {
public:
  ArrayNew(MixedArena& allocator) {}

  // If set, then the initial value is assigned to all entries in the array. If
  // not set, this is array.new_with_default and the default of the type is
  // used.
  Expression* init = nullptr;
  Expression* size;
  Expression* rtt;

  bool isWithDefault() { return !init; }

  void finalize();
};

class ArrayGet : public SpecificExpression<Expression::ArrayGetId> {
public:
  ArrayGet(MixedArena& allocator) {}

  Expression* ref;
  Expression* index;
  // Packed fields have a sign.
  bool signed_ = false;

  void finalize();
};

class ArraySet : public SpecificExpression<Expression::ArraySetId> {
public:
  ArraySet(MixedArena& allocator) {}

  Expression* ref;
  Expression* index;
  Expression* value;

  void finalize();
};

class ArrayLen : public SpecificExpression<Expression::ArrayLenId> {
public:
  ArrayLen(MixedArena& allocator) {}

  Expression* ref;

  void finalize();
};

class ArrayCopy : public SpecificExpression<Expression::ArrayCopyId> {
public:
  ArrayCopy(MixedArena& allocator) {}

  Expression* destRef;
  Expression* destIndex;
  Expression* srcRef;
  Expression* srcIndex;
  Expression* length;

  void finalize();
};

class RefAs : public SpecificExpression<Expression::RefAsId> {
public:
  RefAs(MixedArena& allocator) {}

  RefAsOp op;

  Expression* value;

  void finalize();
};

// Globals

struct Named {
  Name name;

  // Explicit names are ones that we read from the input file and
  // will be written the name section in the output file.
  // Implicit names are names that binaryen generated for internal
  // use only and will not be written the name section.
  bool hasExplicitName = false;

  void setName(Name name_, bool hasExplicitName_) {
    name = name_;
    hasExplicitName = hasExplicitName_;
  }

  void setExplicitName(Name name_) { setName(name_, true); }
};

struct Importable : Named {
  // If these are set, then this is an import, as module.base
  Name module, base;

  bool imported() const { return module.is(); }
};

class Function;

// Represents an offset into a wasm binary file. This is used for debug info.
// For now, assume this is 32 bits as that's the size limit of wasm files
// anyhow.
using BinaryLocation = uint32_t;

// Represents a mapping of wasm module elements to their location in the
// binary representation. This is used for general debugging info support.
// Offsets are relative to the beginning of the code section, as in DWARF.
struct BinaryLocations {
  struct Span {
    BinaryLocation start = 0, end = 0;
  };

  // Track the range of addresses an expressions appears at. This is the
  // contiguous range that all instructions have - control flow instructions
  // have additional opcodes later (like an end for a block or loop), see
  // just after this.
  std::unordered_map<Expression*, Span> expressions;

  // Track the extra delimiter positions that some instructions, in particular
  // control flow, have, like 'end' for loop and block. We keep these in a
  // separate map because they are rare and we optimize for the storage space
  // for the common type of instruction which just needs a Span.
  // For "else" (from an if) we use index 0, and for catch (from a try) we use
  // indexes 0 and above.
  // We use automatic zero-initialization here because that indicates a "null"
  // debug value, indicating the information is not present.
  using DelimiterLocations = ZeroInitSmallVector<BinaryLocation, 1>;

  enum DelimiterId : size_t { Else = 0, Invalid = size_t(-1) };

  std::unordered_map<Expression*, DelimiterLocations> delimiters;

  // DWARF debug info can refer to multiple interesting positions in a function.
  struct FunctionLocations {
    // The very start of the function, where the binary has a size LEB.
    BinaryLocation start = 0;
    // The area where we declare locals, which is right after the size LEB.
    BinaryLocation declarations = 0;
    // The end, which is one past the final "end" instruction byte.
    BinaryLocation end = 0;
  };

  std::unordered_map<Function*, FunctionLocations> functions;
};

// Forward declarations of Stack IR, as functions can contain it, see
// the stackIR property.
// Stack IR is a secondary IR to the main IR defined in this file (Binaryen
// IR). See wasm-stack.h.
class StackInst;

using StackIR = std::vector<StackInst*>;

class Function : public Importable {
public:
  Signature sig; // parameters and return value
  IRProfile profile = IRProfile::Normal;
  std::vector<Type> vars; // non-param locals

  // The body of the function
  Expression* body = nullptr;

  // If present, this stack IR was generated from the main Binaryen IR body,
  // and possibly optimized. If it is present when writing to wasm binary,
  // it will be emitted instead of the main Binaryen IR.
  //
  // Note that no special care is taken to synchronize the two IRs - if you
  // emit stack IR and then optimize the main IR, you need to recompute the
  // stack IR. The Pass system will throw away Stack IR if a pass is run
  // that declares it may modify Binaryen IR.
  std::unique_ptr<StackIR> stackIR;

  // local names. these are optional.
  std::unordered_map<Index, Name> localNames;
  std::unordered_map<Name, Index> localIndices;

  // Source maps debugging info: map expression nodes to their file, line, col.
  struct DebugLocation {
    BinaryLocation fileIndex, lineNumber, columnNumber;
    bool operator==(const DebugLocation& other) const {
      return fileIndex == other.fileIndex && lineNumber == other.lineNumber &&
             columnNumber == other.columnNumber;
    }
    bool operator!=(const DebugLocation& other) const {
      return !(*this == other);
    }
    bool operator<(const DebugLocation& other) const {
      return fileIndex != other.fileIndex
               ? fileIndex < other.fileIndex
               : lineNumber != other.lineNumber
                   ? lineNumber < other.lineNumber
                   : columnNumber < other.columnNumber;
    }
  };
  std::unordered_map<Expression*, DebugLocation> debugLocations;
  std::set<DebugLocation> prologLocation;
  std::set<DebugLocation> epilogLocation;

  // General debugging info support: track instructions and the function itself.
  std::unordered_map<Expression*, BinaryLocations::Span> expressionLocations;
  std::unordered_map<Expression*, BinaryLocations::DelimiterLocations>
    delimiterLocations;
  BinaryLocations::FunctionLocations funcLocation;

  size_t getNumParams();
  size_t getNumVars();
  size_t getNumLocals();

  bool isParam(Index index);
  bool isVar(Index index);

  Name getLocalName(Index index);
  Index getLocalIndex(Name name);
  Index getVarIndexBase();
  Type getLocalType(Index index);

  Name getLocalNameOrDefault(Index index);
  Name getLocalNameOrGeneric(Index index);

  bool hasLocalName(Index index) const;
  void setLocalName(Index index, Name name);

  void clearNames();
  void clearDebugInfo();
};

// The kind of an import or export.
enum class ExternalKind {
  Function = 0,
  Table = 1,
  Memory = 2,
  Global = 3,
  Event = 4,
  Invalid = -1
};

class Export {
public:
  // exported name - note that this is the key, as the internal name is
  // non-unique (can have multiple exports for an internal, also over kinds)
  Name name;
  Name value; // internal name
  ExternalKind kind;
};

class ElementSegment : public Named {
public:
  Name table;
  Expression* offset;
  Type type = Type::funcref;
  std::vector<Expression*> data;

  ElementSegment() = default;
  ElementSegment(Name table, Expression* offset, Type type = Type::funcref)
    : table(table), offset(offset), type(type) {}
  ElementSegment(Name table,
                 Expression* offset,
                 Type type,
                 std::vector<Expression*>& init)
    : table(table), offset(offset), type(type) {
    data.swap(init);
  }
};

class Table : public Importable {
public:
  static const Address::address32_t kPageSize = 1;
  static const Index kUnlimitedSize = Index(-1);
  // In wasm32/64, the maximum table size is limited by a 32-bit pointer: 4GB
  static const Index kMaxSize = Index(-1);

  Address initial = 0;
  Address max = kMaxSize;
  Type type = Type::funcref;

  bool hasMax() { return max != kUnlimitedSize; }
  void clear() {
    name = "";
    initial = 0;
    max = kMaxSize;
  }
};

class Memory : public Importable {
public:
  static const Address::address32_t kPageSize = 64 * 1024;
  static const Address::address64_t kUnlimitedSize = Address::address64_t(-1);
  // In wasm32, the maximum memory size is limited by a 32-bit pointer: 4GB
  static const Address::address32_t kMaxSize32 =
    (uint64_t(4) * 1024 * 1024 * 1024) / kPageSize;

  struct Segment {
    // For use in name section only
    Name name;
    bool isPassive = false;
    Expression* offset = nullptr;
    std::vector<char> data; // TODO: optimize
    Segment() = default;
    Segment(Expression* offset) : offset(offset) {}
    Segment(Expression* offset, const char* init, Address size)
      : offset(offset) {
      data.resize(size);
      std::copy_n(init, size, data.begin());
    }
    Segment(Expression* offset, std::vector<char>& init) : offset(offset) {
      data.swap(init);
    }
    Segment(Name name,
            bool isPassive,
            Expression* offset,
            const char* init,
            Address size)
      : name(name), isPassive(isPassive), offset(offset) {
      data.resize(size);
      std::copy_n(init, size, data.begin());
    }
  };

  bool exists = false;
  Address initial = 0; // sizes are in pages
  Address max = kMaxSize32;
  std::vector<Segment> segments;

  bool shared = false;
  Type indexType = Type::i32;

  Memory() { name = Name::fromInt(0); }
  bool hasMax() { return max != kUnlimitedSize; }
  bool is64() { return indexType == Type::i64; }
  void clear() {
    exists = false;
    name = "";
    initial = 0;
    max = kMaxSize32;
    segments.clear();
    shared = false;
    indexType = Type::i32;
  }
};

class Global : public Importable {
public:
  Type type;
  Expression* init = nullptr;
  bool mutable_ = false;
};

// Kinds of event attributes.
enum WasmEventAttribute : unsigned { WASM_EVENT_ATTRIBUTE_EXCEPTION = 0x0 };

class Event : public Importable {
public:
  // Kind of event. Currently only WASM_EVENT_ATTRIBUTE_EXCEPTION is possible.
  uint32_t attribute = WASM_EVENT_ATTRIBUTE_EXCEPTION;
  Signature sig;
};

// "Opaque" data, not part of the core wasm spec, that is held in binaries.
// May be parsed/handled by utility code elsewhere, but not in wasm.h
class UserSection {
public:
  std::string name;
  std::vector<char> data;
};

// The optional "dylink" section is used in dynamic linking.
class DylinkSection {
public:
  Index memorySize, memoryAlignment, tableSize, tableAlignment;
  std::vector<Name> neededDynlibs;
};

class Module {
public:
  // wasm contents (generally you shouldn't access these from outside, except
  // maybe for iterating; use add*() and the get() functions)
  std::vector<std::unique_ptr<Export>> exports;
  std::vector<std::unique_ptr<Function>> functions;
  std::vector<std::unique_ptr<Global>> globals;
  std::vector<std::unique_ptr<Event>> events;
  std::vector<std::unique_ptr<ElementSegment>> elementSegments;
  std::vector<std::unique_ptr<Table>> tables;

  Memory memory;
  Name start;

  std::vector<UserSection> userSections;

  // Optional user section IR representation.
  std::unique_ptr<DylinkSection> dylinkSection;

  // Source maps debug info.
  std::vector<std::string> debugInfoFileNames;

  // `features` are the features allowed to be used in this module and should be
  // respected regardless of the value of`hasFeaturesSection`.
  // `hasFeaturesSection` means we read a features section and will emit one
  // too.
  FeatureSet features = FeatureSet::MVP;
  bool hasFeaturesSection = false;

  // Module name, if specified. Serves a documentary role only.
  Name name;

  // Optional type name information, used in printing only. Note that Types are
  // globally interned, but type names are specific to a module.
  struct TypeNames {
    // The name of the type.
    Name name;
    // For a Struct, names of fields.
    std::unordered_map<Index, Name> fieldNames;
  };
  std::unordered_map<HeapType, TypeNames> typeNames;

  MixedArena allocator;

private:
  // TODO: add a build option where Names are just indices, and then these
  // methods are not needed
  // exports map is by the *exported* name, which is unique
  std::unordered_map<Name, Export*> exportsMap;
  std::unordered_map<Name, Function*> functionsMap;
  std::unordered_map<Name, Table*> tablesMap;
  std::unordered_map<Name, ElementSegment*> elementSegmentsMap;
  std::unordered_map<Name, Global*> globalsMap;
  std::unordered_map<Name, Event*> eventsMap;

public:
  Module() = default;

  Export* getExport(Name name);
  Function* getFunction(Name name);
  Table* getTable(Name name);
  ElementSegment* getElementSegment(Name name);
  Global* getGlobal(Name name);
  Event* getEvent(Name name);

  Export* getExportOrNull(Name name);
  Table* getTableOrNull(Name name);
  ElementSegment* getElementSegmentOrNull(Name name);
  Function* getFunctionOrNull(Name name);
  Global* getGlobalOrNull(Name name);
  Event* getEventOrNull(Name name);

  Export* addExport(Export* curr);
  Function* addFunction(Function* curr);
  Global* addGlobal(Global* curr);
  Event* addEvent(Event* curr);

  Export* addExport(std::unique_ptr<Export>&& curr);
  Function* addFunction(std::unique_ptr<Function>&& curr);
  Table* addTable(std::unique_ptr<Table>&& curr);
  ElementSegment* addElementSegment(std::unique_ptr<ElementSegment>&& curr);
  Global* addGlobal(std::unique_ptr<Global>&& curr);
  Event* addEvent(std::unique_ptr<Event>&& curr);

  void addStart(const Name& s);

  void removeExport(Name name);
  void removeFunction(Name name);
  void removeTable(Name name);
  void removeElementSegment(Name name);
  void removeGlobal(Name name);
  void removeEvent(Name name);

  void removeExports(std::function<bool(Export*)> pred);
  void removeFunctions(std::function<bool(Function*)> pred);
  void removeTables(std::function<bool(Table*)> pred);
  void removeElementSegments(std::function<bool(ElementSegment*)> pred);
  void removeGlobals(std::function<bool(Global*)> pred);
  void removeEvents(std::function<bool(Event*)> pred);

  void updateMaps();

  void clearDebugInfo();
};

using ModuleExpression = std::pair<Module&, Expression*>;

} // namespace wasm

namespace std {
template<> struct hash<wasm::Address> {
  size_t operator()(const wasm::Address a) const {
    return std::hash<wasm::Address::address64_t>()(a.addr);
  }
};

std::ostream& operator<<(std::ostream& o, wasm::Module& module);
std::ostream& operator<<(std::ostream& o, wasm::Expression& expression);
std::ostream& operator<<(std::ostream& o, wasm::ModuleExpression pair);
std::ostream& operator<<(std::ostream& o, wasm::StackInst& inst);
std::ostream& operator<<(std::ostream& o, wasm::StackIR& ir);

} // namespace std

#endif // wasm_wasm_h
