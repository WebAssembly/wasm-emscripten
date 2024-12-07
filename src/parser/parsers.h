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

#ifndef parser_parsers_h
#define parser_parsers_h

#include "common.h"
#include "contexts.h"
#include "lexer.h"
#include "wat-parser-internal.h"

namespace wasm::WATParser {

using namespace std::string_view_literals;

// Types
template<typename Ctx>
Result<typename Ctx::HeapTypeT> absheaptype(Ctx&, Shareability);
template<typename Ctx> Result<typename Ctx::HeapTypeT> heaptype(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::RefTypeT> maybeReftype(Ctx&);
template<typename Ctx> Result<typename Ctx::RefTypeT> reftype(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::TypeT> tupletype(Ctx&);
template<typename Ctx> Result<typename Ctx::TypeT> valtype(Ctx&);
template<typename Ctx>
MaybeResult<typename Ctx::ParamsT> params(Ctx&, bool allowNames = true);
template<typename Ctx> MaybeResult<typename Ctx::ResultsT> results(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::SignatureT> functype(Ctx&);
template<typename Ctx> Result<typename Ctx::FieldT> storagetype(Ctx&);
template<typename Ctx> Result<typename Ctx::FieldT> fieldtype(Ctx&);
template<typename Ctx> Result<typename Ctx::FieldsT> fields(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::StructT> structtype(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::ArrayT> arraytype(Ctx&);
template<typename Ctx> Result<typename Ctx::LimitsT> limits32(Ctx&);
template<typename Ctx> Result<typename Ctx::LimitsT> limits64(Ctx&);
template<typename Ctx> Result<typename Ctx::MemTypeT> memtype(Ctx&);
template<typename Ctx>
Result<typename Ctx::MemTypeT> memtypeContinued(Ctx&, Type addressType);
template<typename Ctx> Result<typename Ctx::TableTypeT> tabletype(Ctx&);
template<typename Ctx>
Result<typename Ctx::TableTypeT> tabletypeContinued(Ctx&, Type addressType);
template<typename Ctx> Result<typename Ctx::GlobalTypeT> globaltype(Ctx&);
template<typename Ctx> Result<uint32_t> tupleArity(Ctx&);

// Instructions
template<typename Ctx>
MaybeResult<> foldedBlockinstr(Ctx&, const std::vector<Annotation>&);
template<typename Ctx>
MaybeResult<> unfoldedBlockinstr(Ctx&, const std::vector<Annotation>&);
template<typename Ctx>
MaybeResult<> blockinstr(Ctx&, const std::vector<Annotation>&);
template<typename Ctx>
MaybeResult<> plaininstr(Ctx&, const std::vector<Annotation>&);
template<typename Ctx> MaybeResult<> instr(Ctx&);
template<typename Ctx> MaybeResult<> foldedinstr(Ctx&);
template<typename Ctx> Result<> instrs(Ctx&);
template<typename Ctx> Result<> foldedinstrs(Ctx&);
template<typename Ctx> Result<typename Ctx::ExprT> expr(Ctx&);
template<typename Ctx> Result<typename Ctx::MemargT> memarg(Ctx&, uint32_t);
template<typename Ctx> Result<typename Ctx::BlockTypeT> blocktype(Ctx&);
template<typename Ctx>
MaybeResult<> block(Ctx&, const std::vector<Annotation>&, bool);
template<typename Ctx>
MaybeResult<> ifelse(Ctx&, const std::vector<Annotation>&, bool);
template<typename Ctx>
MaybeResult<> loop(Ctx&, const std::vector<Annotation>&, bool);
template<typename Ctx>
MaybeResult<> trycatch(Ctx&, const std::vector<Annotation>&, bool);
template<typename Ctx> MaybeResult<typename Ctx::CatchT> catchinstr(Ctx&);
template<typename Ctx>
MaybeResult<> trytable(Ctx&, const std::vector<Annotation>&, bool);
template<typename Ctx>
Result<> makeUnreachable(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeNop(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeBinary(Ctx&, Index, const std::vector<Annotation>&, BinaryOp op);
template<typename Ctx>
Result<> makeUnary(Ctx&, Index, const std::vector<Annotation>&, UnaryOp op);
template<typename Ctx>
Result<> makeSelect(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeDrop(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeMemorySize(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeMemoryGrow(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeLocalGet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeLocalTee(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeLocalSet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeGlobalGet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeGlobalSet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeConst(Ctx&, Index, const std::vector<Annotation>&, Type type);
template<typename Ctx>
Result<> makeLoad(Ctx&,
                  Index,
                  const std::vector<Annotation>&,
                  Type type,
                  bool signed_,
                  int bytes,
                  bool isAtomic);
template<typename Ctx>
Result<> makeStore(Ctx&,
                   Index,
                   const std::vector<Annotation>&,
                   Type type,
                   int bytes,
                   bool isAtomic);
template<typename Ctx>
Result<> makeAtomicRMW(Ctx&,
                       Index,
                       const std::vector<Annotation>&,
                       AtomicRMWOp op,
                       Type type,
                       uint8_t bytes);
template<typename Ctx>
Result<> makeAtomicCmpxchg(
  Ctx&, Index, const std::vector<Annotation>&, Type type, uint8_t bytes);
template<typename Ctx>
Result<> makeAtomicWait(Ctx&, Index, const std::vector<Annotation>&, Type type);
template<typename Ctx>
Result<> makeAtomicNotify(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeAtomicFence(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeSIMDExtract(
  Ctx&, Index, const std::vector<Annotation>&, SIMDExtractOp op, size_t lanes);
template<typename Ctx>
Result<> makeSIMDReplace(
  Ctx&, Index, const std::vector<Annotation>&, SIMDReplaceOp op, size_t lanes);
template<typename Ctx>
Result<> makeSIMDShuffle(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<>
makeSIMDTernary(Ctx&, Index, const std::vector<Annotation>&, SIMDTernaryOp op);
template<typename Ctx>
Result<>
makeSIMDShift(Ctx&, Index, const std::vector<Annotation>&, SIMDShiftOp op);
template<typename Ctx>
Result<> makeSIMDLoad(
  Ctx&, Index, const std::vector<Annotation>&, SIMDLoadOp op, int bytes);
template<typename Ctx>
Result<> makeSIMDLoadStoreLane(Ctx&,
                               Index,
                               const std::vector<Annotation>&,
                               SIMDLoadStoreLaneOp op,
                               int bytes);
template<typename Ctx>
Result<> makeMemoryInit(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeDataDrop(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeMemoryCopy(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeMemoryFill(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makePop(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeCall(Ctx&, Index, const std::vector<Annotation>&, bool isReturn);
template<typename Ctx>
Result<>
makeCallIndirect(Ctx&, Index, const std::vector<Annotation>&, bool isReturn);
template<typename Ctx>
Result<>
makeBreak(Ctx&, Index, const std::vector<Annotation>&, bool isConditional);
template<typename Ctx>
Result<> makeBreakTable(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeReturn(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRefNull(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRefIsNull(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRefFunc(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRefEq(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableGet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableSet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableSize(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableGrow(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableFill(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableCopy(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTableInit(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeThrow(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRethrow(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeThrowRef(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTupleMake(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTupleExtract(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeTupleDrop(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<>
makeCallRef(Ctx&, Index, const std::vector<Annotation>&, bool isReturn);
template<typename Ctx>
Result<>
makeRefI31(Ctx&, Index, const std::vector<Annotation>&, Shareability share);
template<typename Ctx>
Result<> makeI31Get(Ctx&, Index, const std::vector<Annotation>&, bool signed_);
template<typename Ctx>
Result<> makeRefTest(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRefCast(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<>
makeBrOnNull(Ctx&, Index, const std::vector<Annotation>&, bool onFail = false);
template<typename Ctx>
Result<>
makeBrOnCast(Ctx&, Index, const std::vector<Annotation>&, bool onFail = false);
template<typename Ctx>
Result<>
makeStructNew(Ctx&, Index, const std::vector<Annotation>&, bool default_);
template<typename Ctx>
Result<> makeStructGet(Ctx&,
                       Index,
                       const std::vector<Annotation>&,
                       bool signed_ = false);
template<typename Ctx>
Result<> makeStructSet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<>
makeArrayNew(Ctx&, Index, const std::vector<Annotation>&, bool default_);
template<typename Ctx>
Result<> makeArrayNewData(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayNewElem(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayNewFixed(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<>
makeArrayGet(Ctx&, Index, const std::vector<Annotation>&, bool signed_ = false);
template<typename Ctx>
Result<> makeArraySet(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayLen(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayCopy(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayFill(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayInitData(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeArrayInitElem(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeRefAs(Ctx&, Index, const std::vector<Annotation>&, RefAsOp op);
template<typename Ctx>
Result<>
makeStringNew(Ctx&, Index, const std::vector<Annotation>&, StringNewOp op);
template<typename Ctx>
Result<> makeStringConst(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeStringMeasure(Ctx&,
                           Index,
                           const std::vector<Annotation>&,
                           StringMeasureOp op);
template<typename Ctx>
Result<> makeStringEncode(Ctx&,
                          Index,
                          const std::vector<Annotation>&,
                          StringEncodeOp op);
template<typename Ctx>
Result<> makeStringConcat(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeStringEq(Ctx&, Index, const std::vector<Annotation>&, StringEqOp);
template<typename Ctx>
Result<> makeStringWTF16Get(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeStringSliceWTF(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeContNew(Ctx*, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeContBind(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeSuspend(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeResume(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeResumeThrow(Ctx&, Index, const std::vector<Annotation>&);
template<typename Ctx>
Result<> makeStackSwitch(Ctx&, Index, const std::vector<Annotation>&);

template<typename Ctx>
Result<> ignore(Ctx&, Index, const std::vector<Annotation>&) {
  return Ok{};
}

// Modules
template<typename Ctx>
MaybeResult<typename Ctx::HeapTypeT> maybeTypeidx(Ctx& ctx);
template<typename Ctx> Result<typename Ctx::HeapTypeT> typeidx(Ctx&);
template<typename Ctx>
Result<typename Ctx::FieldIdxT> fieldidx(Ctx&, typename Ctx::HeapTypeT);
template<typename Ctx> MaybeResult<typename Ctx::FuncIdxT> maybeFuncidx(Ctx&);
template<typename Ctx> Result<typename Ctx::FuncIdxT> funcidx(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::TableIdxT> maybeTableidx(Ctx&);
template<typename Ctx> Result<typename Ctx::TableIdxT> tableidx(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::TableIdxT> maybeTableuse(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::MemoryIdxT> maybeMemidx(Ctx&);
template<typename Ctx> Result<typename Ctx::MemoryIdxT> memidx(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::MemoryIdxT> maybeMemuse(Ctx&);
template<typename Ctx> Result<typename Ctx::GlobalIdxT> globalidx(Ctx&);
template<typename Ctx> Result<typename Ctx::ElemIdxT> elemidx(Ctx&);
template<typename Ctx> Result<typename Ctx::DataIdxT> dataidx(Ctx&);
template<typename Ctx> Result<typename Ctx::LocalIdxT> localidx(Ctx&);
template<typename Ctx>
MaybeResult<typename Ctx::LabelIdxT> maybeLabelidx(Ctx&,
                                                   bool inDelegate = false);
template<typename Ctx>
Result<typename Ctx::LabelIdxT> labelidx(Ctx&, bool inDelegate = false);
template<typename Ctx> Result<typename Ctx::TagIdxT> tagidx(Ctx&);
template<typename Ctx>
Result<typename Ctx::TypeUseT> typeuse(Ctx&, bool allowNames = true);
MaybeResult<ImportNames> inlineImport(Lexer&);
Result<std::vector<Name>> inlineExports(Lexer&);
template<typename Ctx> Result<> comptype(Ctx&);
template<typename Ctx> Result<> sharecomptype(Ctx&);
template<typename Ctx> Result<> subtype(Ctx&);
template<typename Ctx> MaybeResult<> typedef_(Ctx&);
template<typename Ctx> MaybeResult<> rectype(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::LocalsT> locals(Ctx&);
template<typename Ctx> MaybeResult<> import_(Ctx&);
template<typename Ctx> MaybeResult<> func(Ctx&);
template<typename Ctx> MaybeResult<> table(Ctx&);
template<typename Ctx> MaybeResult<> memory(Ctx&);
template<typename Ctx> MaybeResult<> global(Ctx&);
template<typename Ctx> MaybeResult<> export_(Ctx&);
template<typename Ctx> MaybeResult<> start(Ctx&);
template<typename Ctx> MaybeResult<typename Ctx::ExprT> maybeElemexpr(Ctx&);
template<typename Ctx> Result<typename Ctx::ElemListT> elemlist(Ctx&, bool);
template<typename Ctx> MaybeResult<> elem(Ctx&);
template<typename Ctx> Result<typename Ctx::DataStringT> datastring(Ctx&);
template<typename Ctx> MaybeResult<> data(Ctx&);
template<typename Ctx> MaybeResult<> tag(Ctx&);
template<typename Ctx> MaybeResult<> modulefield(Ctx&);
template<typename Ctx> Result<> module(Ctx&);

// =====
// Types
// =====

// absheaptype ::= 'func' | 'extern' | ...
template<typename Ctx>
Result<typename Ctx::HeapTypeT> absheaptype(Ctx& ctx, Shareability share) {
  if (ctx.in.takeKeyword("func"sv)) {
    return ctx.makeFuncType(share);
  }
  if (ctx.in.takeKeyword("any"sv)) {
    return ctx.makeAnyType(share);
  }
  if (ctx.in.takeKeyword("extern"sv)) {
    return ctx.makeExternType(share);
  }
  if (ctx.in.takeKeyword("eq"sv)) {
    return ctx.makeEqType(share);
  }
  if (ctx.in.takeKeyword("i31"sv)) {
    return ctx.makeI31Type(share);
  }
  if (ctx.in.takeKeyword("struct"sv)) {
    return ctx.makeStructType(share);
  }
  if (ctx.in.takeKeyword("array"sv)) {
    return ctx.makeArrayType(share);
  }
  if (ctx.in.takeKeyword("exn"sv)) {
    return ctx.makeExnType(share);
  }
  if (ctx.in.takeKeyword("string"sv)) {
    return ctx.makeStringType(share);
  }
  if (ctx.in.takeKeyword("cont"sv)) {
    return ctx.makeContType(share);
  }
  if (ctx.in.takeKeyword("none"sv)) {
    return ctx.makeNoneType(share);
  }
  if (ctx.in.takeKeyword("noextern"sv)) {
    return ctx.makeNoextType(share);
  }
  if (ctx.in.takeKeyword("nofunc"sv)) {
    return ctx.makeNofuncType(share);
  }
  if (ctx.in.takeKeyword("noexn"sv)) {
    return ctx.makeNoexnType(share);
  }
  if (ctx.in.takeKeyword("nocont"sv)) {
    return ctx.makeNocontType(share);
  }
  return ctx.in.err("expected abstract heap type");
}

// heaptype ::= x:typeidx                      => types[x]
//            | t:absheaptype                  => unshared t
//            | '(' 'shared' t:absheaptype ')' => shared t
template<typename Ctx> Result<typename Ctx::HeapTypeT> heaptype(Ctx& ctx) {
  if (auto t = maybeTypeidx(ctx)) {
    CHECK_ERR(t);
    return *t;
  }

  auto share = ctx.in.takeSExprStart("shared"sv) ? Shared : Unshared;
  auto t = absheaptype(ctx, share);
  CHECK_ERR(t);
  if (share == Shared && !ctx.in.takeRParen()) {
    return ctx.in.err("expected end of shared abstract heap type");
  }
  return *t;
}

// reftype ::= 'funcref'   => funcref
//           | 'externref' => externref
//           | 'anyref'    => anyref
//           | 'eqref'     => eqref
//           | 'i31ref'    => i31ref
//           | 'structref' => structref
//           | 'arrayref'  => arrayref
//           | '(' ref null? t:heaptype ')' => ref null? t
template<typename Ctx> MaybeResult<typename Ctx::TypeT> maybeReftype(Ctx& ctx) {
  if (ctx.in.takeKeyword("funcref"sv)) {
    return ctx.makeRefType(ctx.makeFuncType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("externref"sv)) {
    return ctx.makeRefType(ctx.makeExternType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("anyref"sv)) {
    return ctx.makeRefType(ctx.makeAnyType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("eqref"sv)) {
    return ctx.makeRefType(ctx.makeEqType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("i31ref"sv)) {
    return ctx.makeRefType(ctx.makeI31Type(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("structref"sv)) {
    return ctx.makeRefType(ctx.makeStructType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("arrayref"sv)) {
    return ctx.makeRefType(ctx.makeArrayType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("exnref"sv)) {
    return ctx.makeRefType(ctx.makeExnType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("stringref"sv)) {
    return ctx.makeRefType(ctx.makeStringType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("contref"sv)) {
    return ctx.makeRefType(ctx.makeContType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("nullref"sv)) {
    return ctx.makeRefType(ctx.makeNoneType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("nullexternref"sv)) {
    return ctx.makeRefType(ctx.makeNoextType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("nullfuncref"sv)) {
    return ctx.makeRefType(ctx.makeNofuncType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("nullexnref"sv)) {
    return ctx.makeRefType(ctx.makeNoexnType(Unshared), Nullable);
  }
  if (ctx.in.takeKeyword("nullcontref"sv)) {
    return ctx.makeRefType(ctx.makeNocontType(Unshared), Nullable);
  }

  if (!ctx.in.takeSExprStart("ref"sv)) {
    return {};
  }

  auto nullability = ctx.in.takeKeyword("null"sv) ? Nullable : NonNullable;

  auto type = heaptype(ctx);
  CHECK_ERR(type);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of reftype");
  }

  return ctx.makeRefType(*type, nullability);
}

template<typename Ctx> Result<typename Ctx::TypeT> reftype(Ctx& ctx) {
  if (auto t = maybeReftype(ctx)) {
    CHECK_ERR(t);
    return *t;
  }
  return ctx.in.err("expected reftype");
}

// tupletype ::= '(' 'tuple' valtype* ')'
template<typename Ctx> MaybeResult<typename Ctx::TypeT> tupletype(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("tuple"sv)) {
    return {};
  }
  auto elems = ctx.makeTupleElemList();
  size_t numElems = 0;
  while (!ctx.in.takeRParen()) {
    auto elem = singlevaltype(ctx);
    CHECK_ERR(elem);
    ctx.appendTupleElem(elems, *elem);
    ++numElems;
  }
  if (numElems < 2) {
    return ctx.in.err("tuples must have at least two elements");
  }
  return ctx.makeTupleType(elems);
}

// numtype ::= 'i32' => i32
//           | 'i64' => i64
//           | 'f32' => f32
//           | 'f64' => f64
// vectype ::= 'v128' => v128
// singlevaltype ::= t:numtype => t
//                 | t:vectype => t
//                 | t:reftype => t
template<typename Ctx> Result<typename Ctx::TypeT> singlevaltype(Ctx& ctx) {
  if (ctx.in.takeKeyword("i32"sv)) {
    return ctx.makeI32();
  } else if (ctx.in.takeKeyword("i64"sv)) {
    return ctx.makeI64();
  } else if (ctx.in.takeKeyword("f32"sv)) {
    return ctx.makeF32();
  } else if (ctx.in.takeKeyword("f64"sv)) {
    return ctx.makeF64();
  } else if (ctx.in.takeKeyword("v128"sv)) {
    return ctx.makeV128();
  } else if (auto type = maybeReftype(ctx)) {
    CHECK_ERR(type);
    return *type;
  } else {
    return ctx.in.err("expected valtype");
  }
}

// valtype ::= singlevaltype | tupletype
template<typename Ctx> Result<typename Ctx::TypeT> valtype(Ctx& ctx) {
  if (auto type = tupletype(ctx)) {
    CHECK_ERR(type);
    return *type;
  }
  return singlevaltype(ctx);
}

// param  ::= '(' 'param id? t:valtype ')' => [t]
//          | '(' 'param t*:valtype* ')' => [t*]
// params ::= param*
template<typename Ctx>
MaybeResult<typename Ctx::ParamsT> params(Ctx& ctx, bool allowNames) {
  bool hasAny = false;
  auto res = ctx.makeParams();
  while (ctx.in.takeSExprStart("param"sv)) {
    hasAny = true;
    auto pos = ctx.in.getPos();
    if (auto id = ctx.in.takeID()) {
      // Single named param
      if (!allowNames) {
        return ctx.in.err(pos, "unexpected named parameter");
      }
      auto type = valtype(ctx);
      CHECK_ERR(type);
      if (!ctx.in.takeRParen()) {
        return ctx.in.err("expected end of param");
      }
      ctx.appendParam(res, *id, *type);
    } else {
      // Repeated unnamed params
      while (!ctx.in.takeRParen()) {
        auto type = valtype(ctx);
        CHECK_ERR(type);
        ctx.appendParam(res, {}, *type);
      }
    }
  }
  if (hasAny) {
    return res;
  }
  return {};
}

// result  ::= '(' 'result' t*:valtype ')' => [t*]
// results ::= result*
template<typename Ctx> MaybeResult<typename Ctx::ResultsT> results(Ctx& ctx) {
  bool hasAny = false;
  auto res = ctx.makeResults();
  while (ctx.in.takeSExprStart("result"sv)) {
    hasAny = true;
    while (!ctx.in.takeRParen()) {
      auto type = valtype(ctx);
      CHECK_ERR(type);
      ctx.appendResult(res, *type);
    }
  }
  if (hasAny) {
    return res;
  }
  return {};
}

// functype ::= '(' 'func' t1*:vec(param) t2*:vec(result) ')' => [t1*] -> [t2*]
template<typename Ctx>
MaybeResult<typename Ctx::SignatureT> functype(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("func"sv)) {
    return {};
  }

  auto parsedParams = params(ctx);
  CHECK_ERR(parsedParams);

  auto parsedResults = results(ctx);
  CHECK_ERR(parsedResults);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of functype");
  }

  return ctx.makeFuncType(parsedParams.getPtr(), parsedResults.getPtr());
}

// conttype ::= '(' 'cont'  x:typeidx ')' => cont x
template<typename Ctx>
MaybeResult<typename Ctx::ContinuationT> conttype(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("cont"sv)) {
    return {};
  }

  auto x = typeidx(ctx);
  CHECK_ERR(x);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of cont type");
  }

  return ctx.makeContType(*x);
}

// storagetype ::= valtype | packedtype
// packedtype  ::= i8 | i16
template<typename Ctx> Result<typename Ctx::FieldT> storagetype(Ctx& ctx) {
  if (ctx.in.takeKeyword("i8"sv)) {
    return ctx.makeI8();
  }
  if (ctx.in.takeKeyword("i16"sv)) {
    return ctx.makeI16();
  }
  auto type = valtype(ctx);
  CHECK_ERR(type);
  return ctx.makeStorageType(*type);
}

// fieldtype   ::= t:storagetype               => const t
//               | '(' 'mut' t:storagetype ')' => var t
template<typename Ctx> Result<typename Ctx::FieldT> fieldtype(Ctx& ctx) {
  auto mutability = Immutable;
  if (ctx.in.takeSExprStart("mut"sv)) {
    mutability = Mutable;
  }

  auto field = storagetype(ctx);
  CHECK_ERR(field);

  if (mutability == Mutable) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of field type");
    }
  }

  return ctx.makeFieldType(*field, mutability);
}

// field ::= '(' 'field' id t:fieldtype ')' => [(id, t)]
//         | '(' 'field' t*:fieldtype* ')'  => [(_, t*)*]
//         | fieldtype
template<typename Ctx> Result<typename Ctx::FieldsT> fields(Ctx& ctx) {
  auto res = ctx.makeFields();
  while (true) {
    if (ctx.in.empty() || ctx.in.peekRParen()) {
      return res;
    }
    if (ctx.in.takeSExprStart("field")) {
      if (auto id = ctx.in.takeID()) {
        auto field = fieldtype(ctx);
        CHECK_ERR(field);
        if (!ctx.in.takeRParen()) {
          return ctx.in.err("expected end of field");
        }
        ctx.appendField(res, *id, *field);
      } else {
        while (!ctx.in.takeRParen()) {
          auto field = fieldtype(ctx);
          CHECK_ERR(field);
          ctx.appendField(res, {}, *field);
        }
      }
    } else {
      auto field = fieldtype(ctx);
      CHECK_ERR(field);
      ctx.appendField(res, {}, *field);
    }
  }
}

// structtype ::= '(' 'struct' field* ')'
template<typename Ctx> MaybeResult<typename Ctx::StructT> structtype(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("struct"sv)) {
    return {};
  }
  auto namedFields = fields(ctx);
  CHECK_ERR(namedFields);
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of struct definition");
  }

  return ctx.makeStruct(*namedFields);
}

// arraytype ::= '(' 'array' field ')'
template<typename Ctx> MaybeResult<typename Ctx::ArrayT> arraytype(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("array"sv)) {
    return {};
  }
  auto namedFields = fields(ctx);
  CHECK_ERR(namedFields);
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of array definition");
  }

  if (auto array = ctx.makeArray(*namedFields)) {
    return *array;
  }
  return ctx.in.err("expected exactly one field in array definition");
}

// limits32 ::= n:u32 m:u32?
template<typename Ctx> Result<typename Ctx::LimitsT> limits32(Ctx& ctx) {
  auto n = ctx.in.takeU32();
  if (!n) {
    return ctx.in.err("expected initial size");
  }
  std::optional<uint64_t> m = ctx.in.takeU32();
  return ctx.makeLimits(uint64_t(*n), m);
}

// limits64 ::= n:u64 m:u64?
template<typename Ctx> Result<typename Ctx::LimitsT> limits64(Ctx& ctx) {
  auto n = ctx.in.takeU64();
  if (!n) {
    return ctx.in.err("expected initial size");
  }
  std::optional<uint64_t> m = ctx.in.takeU64();
  return ctx.makeLimits(uint64_t(*n), m);
}

// memtype ::= (limits32 | 'i32' limits32 | 'i64' limit64) shared?
//  note: the index type 'i32' or 'i64' is already parsed to simplify parsing of
//  memory abbreviations.
template<typename Ctx> Result<typename Ctx::MemTypeT> memtype(Ctx& ctx) {
  Type addressType = Type::i32;
  if (ctx.in.takeKeyword("i64"sv)) {
    addressType = Type::i64;
  } else {
    ctx.in.takeKeyword("i32"sv);
  }
  return memtypeContinued(ctx, addressType);
}

template<typename Ctx>
Result<typename Ctx::MemTypeT> memtypeContinued(Ctx& ctx, Type addressType) {
  assert(addressType == Type::i32 || addressType == Type::i64);
  auto limits = addressType == Type::i32 ? limits32(ctx) : limits64(ctx);
  CHECK_ERR(limits);
  bool shared = false;
  if (ctx.in.takeKeyword("shared"sv)) {
    shared = true;
  }
  return ctx.makeMemType(addressType, *limits, shared);
}

// tabletype ::= (limits32 | 'i32' limits32 | 'i64' limit64) reftype
template<typename Ctx> Result<typename Ctx::TableTypeT> tabletype(Ctx& ctx) {
  Type addressType = Type::i32;
  if (ctx.in.takeKeyword("i64"sv)) {
    addressType = Type::i64;
  } else {
    ctx.in.takeKeyword("i32"sv);
  }
  return tabletypeContinued(ctx, addressType);
}

template<typename Ctx>
Result<typename Ctx::TableTypeT> tabletypeContinued(Ctx& ctx,
                                                    Type addressType) {
  auto limits = addressType == Type::i32 ? limits32(ctx) : limits64(ctx);
  CHECK_ERR(limits);
  auto type = reftype(ctx);
  CHECK_ERR(type);
  return ctx.makeTableType(addressType, *limits, *type);
}

// globaltype ::= t:valtype               => const t
//              | '(' 'mut' t:valtype ')' => var t
template<typename Ctx> Result<typename Ctx::GlobalTypeT> globaltype(Ctx& ctx) {
  auto mutability = Immutable;
  if (ctx.in.takeSExprStart("mut"sv)) {
    mutability = Mutable;
  }

  auto type = valtype(ctx);
  CHECK_ERR(type);

  if (mutability == Mutable && !ctx.in.takeRParen()) {
    return ctx.in.err("expected end of globaltype");
  }

  return ctx.makeGlobalType(mutability, *type);
}

// arity ::= x:u32    (if x >=2 )
template<typename Ctx> Result<uint32_t> tupleArity(Ctx& ctx) {
  auto arity = ctx.in.takeU32();
  if (!arity) {
    return ctx.in.err("expected tuple arity");
  }
  if (*arity < 2) {
    return ctx.in.err("tuple arity must be at least 2");
  }
  return *arity;
}

// ============
// Instructions
// ============

// blockinstr ::= block | loop | if-else | try-catch | try_table
template<typename Ctx>
MaybeResult<> foldedBlockinstr(Ctx& ctx,
                               const std::vector<Annotation>& annotations) {
  ctx.setSrcLoc(annotations);
  if (auto i = block(ctx, annotations, true)) {
    return i;
  }
  if (auto i = ifelse(ctx, annotations, true)) {
    return i;
  }
  if (auto i = loop(ctx, annotations, true)) {
    return i;
  }
  if (auto i = trycatch(ctx, annotations, true)) {
    return i;
  }
  if (auto i = trytable(ctx, annotations, true)) {
    return i;
  }
  return {};
}

template<typename Ctx>
MaybeResult<> unfoldedBlockinstr(Ctx& ctx,
                                 const std::vector<Annotation>& annotations) {
  ctx.setSrcLoc(annotations);
  if (auto i = block(ctx, annotations, false)) {
    return i;
  }
  if (auto i = ifelse(ctx, annotations, false)) {
    return i;
  }
  if (auto i = loop(ctx, annotations, false)) {
    return i;
  }
  if (auto i = trycatch(ctx, annotations, false)) {
    return i;
  }
  if (auto i = trytable(ctx, annotations, false)) {
    return i;
  }
  return {};
}

template<typename Ctx>
MaybeResult<> blockinstr(Ctx& ctx, const std::vector<Annotation>& annotations) {
  if (auto i = foldedBlockinstr(ctx, annotations)) {
    return i;
  }
  if (auto i = unfoldedBlockinstr(ctx, annotations)) {
    return i;
  }
  return {};
}

// plaininstr ::= ... all plain instructions ...
template<typename Ctx>
MaybeResult<> plaininstr(Ctx& ctx, const std::vector<Annotation>& annotations) {
  ctx.setSrcLoc(annotations);
  auto pos = ctx.in.getPos();
  auto keyword = ctx.in.takeKeyword();
  if (!keyword) {
    return {};
  }

#include <gen-s-parser.inc>
}

// instr ::= plaininstr | blockinstr
template<typename Ctx> MaybeResult<> instr(Ctx& ctx) {
  // Check for valid strings that are not instructions.
  if (auto keyword = ctx.in.peekKeyword()) {
    if (keyword == "end"sv || keyword == "then"sv || keyword == "else"sv ||
        keyword == "catch"sv || keyword == "catch_all"sv ||
        keyword == "delegate"sv || keyword == "ref"sv) {
      return {};
    }
  }
  if (auto inst = blockinstr(ctx, ctx.in.getAnnotations())) {
    return inst;
  }
  if (auto inst = plaininstr(ctx, ctx.in.getAnnotations())) {
    return inst;
  }
  // TODO: Handle folded plain instructions as well.
  return {};
}

template<typename Ctx> MaybeResult<> foldedinstr(Ctx& ctx) {
  // We must have an '(' to start a folded instruction.
  if (!ctx.in.peekLParen()) {
    return {};
  }

  // Check for valid strings that look like folded instructions but are not.
  if (ctx.in.peekSExprStart("then"sv) || ctx.in.peekSExprStart("else")) {
    return {};
  }

  // A stack of (start, end) position pairs defining the positions of
  // instructions that need to be parsed after their folded children.
  struct InstrInfo {
    size_t start;
    std::optional<size_t> end;
    std::vector<Annotation> annotations;
  };
  std::vector<InstrInfo> foldedInstrs;

  do {
    if (ctx.in.takeRParen()) {
      // We've reached the end of a folded instruction. Parse it for real.
      auto info = std::move(foldedInstrs.back());
      if (!info.end) {
        return ctx.in.err("unexpected end of folded instruction");
      }
      foldedInstrs.pop_back();

      WithPosition with(ctx, info.start);
      auto inst = plaininstr(ctx, std::move(info.annotations));
      assert(inst && "unexpectedly failed to parse instruction");
      CHECK_ERR(inst);
      assert(ctx.in.getPos() == *info.end && "expected end of instruction");
      continue;
    }

    auto annotations = ctx.in.takeAnnotations();

    // We're not ending an instruction, so we must be starting a new one. Maybe
    // it is a block instruction.
    if (auto blockinst = foldedBlockinstr(ctx, annotations)) {
      CHECK_ERR(blockinst);
      continue;
    }

    // We must be starting a new plain instruction.
    if (!ctx.in.takeLParen()) {
      return ctx.in.err("expected folded instruction");
    }
    foldedInstrs.push_back({ctx.in.getPos(), {}, std::move(annotations)});

    // Consume the span for the instruction without meaningfully parsing it yet.
    // It will be parsed for real using the real context after its s-expression
    // children have been found and parsed.
    NullCtx nullCtx(ctx.in);
    if (auto inst = plaininstr(nullCtx, {})) {
      CHECK_ERR(inst);
      ctx.in = nullCtx.in;
    } else {
      return ctx.in.err("expected instruction");
    }

    // The folded instruction we just started ends here.
    assert(!foldedInstrs.back().end);
    foldedInstrs.back().end = ctx.in.getPos();
  } while (!foldedInstrs.empty());

  return Ok{};
}

template<typename Ctx> Result<> instrs(Ctx& ctx) {
  while (true) {
    if (auto inst = instr(ctx)) {
      CHECK_ERR(inst);
      continue;
    }
    if (auto inst = foldedinstr(ctx)) {
      CHECK_ERR(inst);
      continue;
    }
    break;
  }
  return Ok{};
}

template<typename Ctx> Result<> foldedinstrs(Ctx& ctx) {
  while (auto inst = foldedinstr(ctx)) {
    CHECK_ERR(inst);
  }
  return Ok{};
}

template<typename Ctx> Result<typename Ctx::ExprT> expr(Ctx& ctx) {
  CHECK_ERR(instrs(ctx));
  return ctx.makeExpr();
}

// memarg_n ::= o:offset a:align_n
// offset   ::= 'offset='o:u64 => o | _ => 0
// align_n  ::= 'align='a:u32 => a | _ => n
template<typename Ctx>
Result<typename Ctx::MemargT> memarg(Ctx& ctx, uint32_t n) {
  uint64_t offset = 0;
  uint32_t align = n;
  if (auto o = ctx.in.takeOffset()) {
    offset = *o;
  }
  if (auto a = ctx.in.takeAlign()) {
    align = *a;
  }
  return ctx.getMemarg(offset, align);
}

// blocktype ::= (t:result)? => t? | x,I:typeuse => x if I = {}
template<typename Ctx> Result<typename Ctx::BlockTypeT> blocktype(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  auto initialLexer = ctx.in;

  if (auto res = results(ctx)) {
    CHECK_ERR(res);
    if (ctx.getResultsSize(*res) == 1) {
      return ctx.getBlockTypeFromResult(*res);
    }
  }

  // We either had no results or multiple results. Reset and parse again as a
  // type use.
  ctx.in = initialLexer;
  auto use = typeuse(ctx, false);
  CHECK_ERR(use);

  auto type = ctx.getBlockTypeFromTypeUse(pos, *use);
  CHECK_ERR(type);
  return *type;
}

// block ::= 'block' label blocktype instr* 'end' id?   if id = {} or id = label
//         | '(' 'block' label blocktype instr* ')'
template<typename Ctx>
MaybeResult<>
block(Ctx& ctx, const std::vector<Annotation>& annotations, bool folded) {
  auto pos = ctx.in.getPos();

  if ((folded && !ctx.in.takeSExprStart("block"sv)) ||
      (!folded && !ctx.in.takeKeyword("block"sv))) {
    return {};
  }

  auto label = ctx.in.takeID();

  auto type = blocktype(ctx);
  CHECK_ERR(type);

  CHECK_ERR(ctx.makeBlock(pos, annotations, label, *type));

  CHECK_ERR(instrs(ctx));

  if (folded) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of block");
    }
  } else {
    if (!ctx.in.takeKeyword("end"sv)) {
      return ctx.in.err("expected 'end' at end of block");
    }
    auto id = ctx.in.takeID();
    if (id && id != label) {
      return ctx.in.err("end label does not match block label");
    }
  }

  return ctx.visitEnd();
}

// if ::= 'if' label blocktype instr1* ('else' id1? instr2*)? 'end' id2?
//      | '(' 'if' label blocktype foldedinstr* '(' 'then' instr1* ')'
//            ('(' 'else' instr2* ')')? ')'
template<typename Ctx>
MaybeResult<>
ifelse(Ctx& ctx, const std::vector<Annotation>& annotations, bool folded) {
  auto pos = ctx.in.getPos();

  if ((folded && !ctx.in.takeSExprStart("if"sv)) ||
      (!folded && !ctx.in.takeKeyword("if"sv))) {
    return {};
  }

  auto label = ctx.in.takeID();

  auto type = blocktype(ctx);
  CHECK_ERR(type);

  if (folded) {
    CHECK_ERR(foldedinstrs(ctx));
    ctx.setSrcLoc(annotations);
  }

  CHECK_ERR(ctx.makeIf(pos, annotations, label, *type));

  if (folded && !ctx.in.takeSExprStart("then"sv)) {
    return ctx.in.err("expected 'then' before if instructions");
  }

  CHECK_ERR(instrs(ctx));

  if (folded && !ctx.in.takeRParen()) {
    return ctx.in.err("expected ')' at end of then block");
  }

  if ((folded && ctx.in.takeSExprStart("else"sv)) ||
      (!folded && ctx.in.takeKeyword("else"sv))) {
    auto id1 = ctx.in.takeID();
    if (id1 && id1 != label) {
      return ctx.in.err("else label does not match if label");
    }

    CHECK_ERR(ctx.visitElse());

    CHECK_ERR(instrs(ctx));

    if (folded && !ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of else block");
    }
  }

  if (folded) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of if");
    }
  } else {
    if (!ctx.in.takeKeyword("end"sv)) {
      return ctx.in.err("expected 'end' at end of if");
    }
    auto id2 = ctx.in.takeID();
    if (id2 && id2 != label) {
      return ctx.in.err("end label does not match if label");
    }
  }

  return ctx.visitEnd();
}

// loop ::= 'loop' label blocktype instr* 'end' id?
//        | '(' 'loop' label blocktype instr* ')'
template<typename Ctx>
MaybeResult<>
loop(Ctx& ctx, const std::vector<Annotation>& annotations, bool folded) {
  auto pos = ctx.in.getPos();

  if ((folded && !ctx.in.takeSExprStart("loop"sv)) ||
      (!folded && !ctx.in.takeKeyword("loop"sv))) {
    return {};
  }

  auto label = ctx.in.takeID();

  auto type = blocktype(ctx);
  CHECK_ERR(type);

  CHECK_ERR(ctx.makeLoop(pos, annotations, label, *type));

  CHECK_ERR(instrs(ctx));

  if (folded) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of loop");
    }
  } else {
    if (!ctx.in.takeKeyword("end"sv)) {
      return ctx.in.err("expected 'end' at end of loop");
    }
    auto id = ctx.in.takeID();
    if (id && id != label) {
      return ctx.in.err("end label does not match loop label");
    }
  }
  return ctx.visitEnd();
}

// trycatch ::= 'try' label blocktype instr* ('catch' id? tagidx instr*)*
//                  ('catch_all' id? instr*)? 'end' id?
//            | '(' 'try' label blocktype '(' 'do' instr* ')'
//                  ('(' 'catch' tagidx instr* ')')*
//                  ('(' 'catch_all' instr* ')')? ')'
//            | 'try' label blocktype instr* 'deledate' label
//            | '(' 'try' label blocktype '(' 'do' instr* ')'
//                '(' 'delegate' label ')' ')'
template<typename Ctx>
MaybeResult<>
trycatch(Ctx& ctx, const std::vector<Annotation>& annotations, bool folded) {
  auto pos = ctx.in.getPos();

  if ((folded && !ctx.in.takeSExprStart("try"sv)) ||
      (!folded && !ctx.in.takeKeyword("try"sv))) {
    return {};
  }

  auto label = ctx.in.takeID();

  auto type = blocktype(ctx);
  CHECK_ERR(type);

  CHECK_ERR(ctx.makeTry(pos, annotations, label, *type));

  if (folded) {
    if (!ctx.in.takeSExprStart("do"sv)) {
      return ctx.in.err("expected 'do' in try");
    }
  }

  CHECK_ERR(instrs(ctx));

  if (folded) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of do");
    }
  }

  if ((folded && ctx.in.takeSExprStart("delegate")) ||
      (!folded && ctx.in.takeKeyword("delegate"))) {
    auto delegatePos = ctx.in.getPos();

    auto label = labelidx(ctx, true);
    CHECK_ERR(label);

    if (folded) {
      if (!ctx.in.takeRParen()) {
        return ctx.in.err("expected ')' at end of delegate");
      }
      if (!ctx.in.takeRParen()) {
        return ctx.in.err("expected ')' at end of try");
      }
    }

    CHECK_ERR(ctx.visitDelegate(delegatePos, *label));
    return Ok{};
  }

  while (true) {
    auto catchPos = ctx.in.getPos();

    if ((folded && !ctx.in.takeSExprStart("catch"sv)) ||
        (!folded && !ctx.in.takeKeyword("catch"sv))) {
      break;
    }

    // It can be ambiguous whether the name after `catch` is intended to be the
    // optional ID or the tag identifier. For example:
    //
    // (tag $t)
    // (func $ambiguous
    //   try $t
    //   catch $t
    //   end
    // )
    //
    // When parsing the `catch`, the parser first tries to parse an optional ID
    // that must match the label of the `try`, and it succeeds because it sees
    // `$t` after the catch. However, when it then tries to parse the mandatory
    // tag index, it fails because the next token is `end`. The problem is that
    // the `$t` after the `catch` was the tag name and there was no optional ID
    // after all. The parser sets `parseID = false` and resets to just after the
    // `catch`, and now it skips parsing the optional ID so it correctly parses
    // the `$t` as a tag name.
    bool parseID = !folded;
    auto afterCatchPos = ctx.in.getPos();
    while (true) {
      if (!folded && parseID) {
        auto id = ctx.in.takeID();
        if (id && id != label) {
          // Instead of returning an error, retry without the ID.
          parseID = false;
          ctx.in.setPos(afterCatchPos);
          continue;
        }
      }

      auto tag = tagidx(ctx);
      if (parseID && tag.getErr()) {
        // Instead of returning an error, retry without the ID.
        parseID = false;
        ctx.in.setPos(afterCatchPos);
        continue;
      }
      CHECK_ERR(tag);

      CHECK_ERR(ctx.visitCatch(catchPos, *tag));

      CHECK_ERR(instrs(ctx));

      if (folded) {
        if (!ctx.in.takeRParen()) {
          return ctx.in.err("expected ')' at end of catch");
        }
      }
      break;
    }
  }

  if ((folded && ctx.in.takeSExprStart("catch_all"sv)) ||
      (!folded && ctx.in.takeKeyword("catch_all"sv))) {
    auto catchPos = ctx.in.getPos();

    if (!folded) {
      auto id = ctx.in.takeID();
      if (id && id != label) {
        return ctx.in.err("catch_all label does not match try label");
      }
    }

    CHECK_ERR(ctx.visitCatchAll(catchPos));

    CHECK_ERR(instrs(ctx));

    if (folded) {
      if (!ctx.in.takeRParen()) {
        return ctx.in.err("expected ')' at end of catch_all");
      }
    }
  }

  if (folded) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of try");
    }
  } else {
    if (!ctx.in.takeKeyword("end"sv)) {
      return ctx.in.err("expected 'end' at end of try");
    }

    auto id = ctx.in.takeID();
    if (id && id != label) {
      return ctx.in.err("end label does not match try label");
    }
  }
  return ctx.visitEnd();
}

template<typename Ctx> MaybeResult<typename Ctx::CatchT> catchinstr(Ctx& ctx) {
  typename Ctx::CatchT result;
  if (ctx.in.takeSExprStart("catch"sv)) {
    auto tag = tagidx(ctx);
    CHECK_ERR(tag);
    auto label = labelidx(ctx);
    CHECK_ERR(label);
    result = ctx.makeCatch(*tag, *label);
  } else if (ctx.in.takeSExprStart("catch_ref"sv)) {
    auto tag = tagidx(ctx);
    CHECK_ERR(tag);
    auto label = labelidx(ctx);
    CHECK_ERR(label);
    result = ctx.makeCatchRef(*tag, *label);
  } else if (ctx.in.takeSExprStart("catch_all"sv)) {
    auto label = labelidx(ctx);
    CHECK_ERR(label);
    result = ctx.makeCatchAll(*label);
  } else if (ctx.in.takeSExprStart("catch_all_ref"sv)) {
    auto label = labelidx(ctx);
    CHECK_ERR(label);
    result = ctx.makeCatchAllRef(*label);
  } else {
    return {};
  }

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected ')' at end of catch clause");
  }

  return result;
}

// trytable ::= 'try_table' label blocktype catchinstr* instr* 'end' id?
//            | '(' 'try_table' label blocktype catchinstr* instr* ')'
template<typename Ctx>
MaybeResult<>
trytable(Ctx& ctx, const std::vector<Annotation>& annotations, bool folded) {
  auto pos = ctx.in.getPos();

  if ((folded && !ctx.in.takeSExprStart("try_table"sv)) ||
      (!folded && !ctx.in.takeKeyword("try_table"sv))) {
    return {};
  }

  auto label = ctx.in.takeID();

  auto type = blocktype(ctx);
  CHECK_ERR(type);

  auto catches = ctx.makeCatchList();
  while (auto c = catchinstr(ctx)) {
    CHECK_ERR(c);
    ctx.appendCatch(catches, *c);
  }

  CHECK_ERR(ctx.makeTryTable(pos, annotations, label, *type, catches));

  CHECK_ERR(instrs(ctx));

  if (folded) {
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of try_table");
    }
  } else {
    if (!ctx.in.takeKeyword("end"sv)) {
      return ctx.in.err("expected 'end' at end of try_table");
    }

    auto id = ctx.in.takeID();
    if (id && id != label) {
      return ctx.in.err("end label does not match try_table label");
    }
  }
  return ctx.visitEnd();
}

template<typename Ctx>
Result<> makeUnreachable(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations) {
  return ctx.makeUnreachable(pos, annotations);
}

template<typename Ctx>
Result<>
makeNop(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeNop(pos, annotations);
}

template<typename Ctx>
Result<> makeBinary(Ctx& ctx,
                    Index pos,
                    const std::vector<Annotation>& annotations,
                    BinaryOp op) {
  return ctx.makeBinary(pos, annotations, op);
}

template<typename Ctx>
Result<> makeUnary(Ctx& ctx,
                   Index pos,
                   const std::vector<Annotation>& annotations,
                   UnaryOp op) {
  return ctx.makeUnary(pos, annotations, op);
}

template<typename Ctx>
Result<>
makeSelect(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto res = results(ctx);
  CHECK_ERR(res);
  return ctx.makeSelect(pos, annotations, res.getPtr());
}

template<typename Ctx>
Result<>
makeDrop(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeDrop(pos, annotations);
}

template<typename Ctx>
Result<> makeMemorySize(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  return ctx.makeMemorySize(pos, annotations, mem.getPtr());
}

template<typename Ctx>
Result<> makeMemoryGrow(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  return ctx.makeMemoryGrow(pos, annotations, mem.getPtr());
}

template<typename Ctx>
Result<>
makeLocalGet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto local = localidx(ctx);
  CHECK_ERR(local);
  return ctx.makeLocalGet(pos, annotations, *local);
}

template<typename Ctx>
Result<>
makeLocalTee(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto local = localidx(ctx);
  CHECK_ERR(local);
  return ctx.makeLocalTee(pos, annotations, *local);
}

template<typename Ctx>
Result<>
makeLocalSet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto local = localidx(ctx);
  CHECK_ERR(local);
  return ctx.makeLocalSet(pos, annotations, *local);
}

template<typename Ctx>
Result<>
makeGlobalGet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto global = globalidx(ctx);
  CHECK_ERR(global);
  return ctx.makeGlobalGet(pos, annotations, *global);
}

template<typename Ctx>
Result<>
makeGlobalSet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto global = globalidx(ctx);
  CHECK_ERR(global);
  return ctx.makeGlobalSet(pos, annotations, *global);
}

template<typename Ctx>
Result<> makeConst(Ctx& ctx,
                   Index pos,
                   const std::vector<Annotation>& annotations,
                   Type type) {
  assert(type.isBasic());
  switch (type.getBasic()) {
    case Type::i32:
      if (auto c = ctx.in.takeI32()) {
        return ctx.makeI32Const(pos, annotations, *c);
      }
      return ctx.in.err("expected i32");
    case Type::i64:
      if (auto c = ctx.in.takeI64()) {
        return ctx.makeI64Const(pos, annotations, *c);
      }
      return ctx.in.err("expected i64");
    case Type::f32:
      if (auto c = ctx.in.takeF32()) {
        return ctx.makeF32Const(pos, annotations, *c);
      }
      return ctx.in.err("expected f32");
    case Type::f64:
      if (auto c = ctx.in.takeF64()) {
        return ctx.makeF64Const(pos, annotations, *c);
      }
      return ctx.in.err("expected f64");
    case Type::v128:
      if (ctx.in.takeKeyword("i8x16"sv)) {
        std::array<uint8_t, 16> vals;
        for (size_t i = 0; i < 16; ++i) {
          auto val = ctx.in.takeI8();
          if (!val) {
            return ctx.in.err("expected i8 value");
          }
          vals[i] = *val;
        }
        return ctx.makeI8x16Const(pos, annotations, vals);
      }
      if (ctx.in.takeKeyword("i16x8"sv)) {
        std::array<uint16_t, 8> vals;
        for (size_t i = 0; i < 8; ++i) {
          auto val = ctx.in.takeI16();
          if (!val) {
            return ctx.in.err("expected i16 value");
          }
          vals[i] = *val;
        }
        return ctx.makeI16x8Const(pos, annotations, vals);
      }
      if (ctx.in.takeKeyword("i32x4"sv)) {
        std::array<uint32_t, 4> vals;
        for (size_t i = 0; i < 4; ++i) {
          auto val = ctx.in.takeI32();
          if (!val) {
            return ctx.in.err("expected i32 value");
          }
          vals[i] = *val;
        }
        return ctx.makeI32x4Const(pos, annotations, vals);
      }
      if (ctx.in.takeKeyword("i64x2"sv)) {
        std::array<uint64_t, 2> vals;
        for (size_t i = 0; i < 2; ++i) {
          auto val = ctx.in.takeI64();
          if (!val) {
            return ctx.in.err("expected i64 value");
          }
          vals[i] = *val;
        }
        return ctx.makeI64x2Const(pos, annotations, vals);
      }
      if (ctx.in.takeKeyword("f32x4"sv)) {
        std::array<float, 4> vals;
        for (size_t i = 0; i < 4; ++i) {
          auto val = ctx.in.takeF32();
          if (!val) {
            return ctx.in.err("expected f32 value");
          }
          vals[i] = *val;
        }
        return ctx.makeF32x4Const(pos, annotations, vals);
      }
      if (ctx.in.takeKeyword("f64x2"sv)) {
        std::array<double, 2> vals;
        for (size_t i = 0; i < 2; ++i) {
          auto val = ctx.in.takeF64();
          if (!val) {
            return ctx.in.err("expected f64 value");
          }
          vals[i] = *val;
        }
        return ctx.makeF64x2Const(pos, annotations, vals);
      }
      return ctx.in.err("expected SIMD vector shape");
    case Type::none:
    case Type::unreachable:
      break;
  }
  WASM_UNREACHABLE("unexpected type");
}

template<typename Ctx>
Result<> makeLoad(Ctx& ctx,
                  Index pos,
                  const std::vector<Annotation>& annotations,
                  Type type,
                  bool signed_,
                  int bytes,
                  bool isAtomic) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, bytes);
  CHECK_ERR(arg);
  return ctx.makeLoad(
    pos, annotations, type, signed_, bytes, isAtomic, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeStore(Ctx& ctx,
                   Index pos,
                   const std::vector<Annotation>& annotations,
                   Type type,
                   int bytes,
                   bool isAtomic) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, bytes);
  CHECK_ERR(arg);
  return ctx.makeStore(
    pos, annotations, type, bytes, isAtomic, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeAtomicRMW(Ctx& ctx,
                       Index pos,
                       const std::vector<Annotation>& annotations,
                       AtomicRMWOp op,
                       Type type,
                       uint8_t bytes) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, bytes);
  CHECK_ERR(arg);
  return ctx.makeAtomicRMW(
    pos, annotations, op, type, bytes, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeAtomicCmpxchg(Ctx& ctx,
                           Index pos,
                           const std::vector<Annotation>& annotations,
                           Type type,
                           uint8_t bytes) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, bytes);
  CHECK_ERR(arg);
  return ctx.makeAtomicCmpxchg(
    pos, annotations, type, bytes, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeAtomicWait(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations,
                        Type type) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, type == Type::i32 ? 4 : 8);
  CHECK_ERR(arg);
  return ctx.makeAtomicWait(pos, annotations, type, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeAtomicNotify(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, 4);
  CHECK_ERR(arg);
  return ctx.makeAtomicNotify(pos, annotations, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeAtomicFence(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations) {
  return ctx.makeAtomicFence(pos, annotations);
}

template<typename Ctx>
Result<> makeSIMDExtract(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations,
                         SIMDExtractOp op,
                         size_t) {
  auto lane = ctx.in.takeU8();
  if (!lane) {
    return ctx.in.err("expected lane index");
  }
  return ctx.makeSIMDExtract(pos, annotations, op, *lane);
}

template<typename Ctx>
Result<> makeSIMDReplace(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations,
                         SIMDReplaceOp op,
                         size_t lanes) {
  auto lane = ctx.in.takeU8();
  if (!lane) {
    return ctx.in.err("expected lane index");
  }
  return ctx.makeSIMDReplace(pos, annotations, op, *lane);
}

template<typename Ctx>
Result<> makeSIMDShuffle(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations) {
  std::array<uint8_t, 16> lanes;
  for (int i = 0; i < 16; ++i) {
    auto lane = ctx.in.takeU8();
    if (!lane) {
      return ctx.in.err("expected lane index");
    }
    lanes[i] = *lane;
  }
  return ctx.makeSIMDShuffle(pos, annotations, lanes);
}

template<typename Ctx>
Result<> makeSIMDTernary(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations,
                         SIMDTernaryOp op) {
  return ctx.makeSIMDTernary(pos, annotations, op);
}

template<typename Ctx>
Result<> makeSIMDShift(Ctx& ctx,
                       Index pos,
                       const std::vector<Annotation>& annotations,
                       SIMDShiftOp op) {
  return ctx.makeSIMDShift(pos, annotations, op);
}

template<typename Ctx>
Result<> makeSIMDLoad(Ctx& ctx,
                      Index pos,
                      const std::vector<Annotation>& annotations,
                      SIMDLoadOp op,
                      int bytes) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  auto arg = memarg(ctx, bytes);
  CHECK_ERR(arg);
  return ctx.makeSIMDLoad(pos, annotations, op, mem.getPtr(), *arg);
}

template<typename Ctx>
Result<> makeSIMDLoadStoreLane(Ctx& ctx,
                               Index pos,
                               const std::vector<Annotation>& annotations,
                               SIMDLoadStoreLaneOp op,
                               int bytes) {
  auto reset = ctx.in.getPos();

  auto retry = [&]() -> Result<> {
    // We failed to parse. Maybe the lane index was accidentally parsed as the
    // optional memory index. Try again without parsing a memory index.
    WithPosition with(ctx, reset);
    auto arg = memarg(ctx, bytes);
    CHECK_ERR(arg);
    auto lane = ctx.in.takeU8();
    if (!lane) {
      return ctx.in.err("expected lane index");
    }
    return ctx.makeSIMDLoadStoreLane(
      pos, annotations, op, nullptr, *arg, *lane);
  };

  auto mem = maybeMemidx(ctx);
  if (mem.getErr()) {
    return retry();
  }
  auto arg = memarg(ctx, bytes);
  CHECK_ERR(arg);
  auto lane = ctx.in.takeU8();
  if (!lane) {
    return retry();
  }
  return ctx.makeSIMDLoadStoreLane(
    pos, annotations, op, mem.getPtr(), *arg, *lane);
}

template<typename Ctx>
Result<> makeMemoryInit(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations) {
  auto reset = ctx.in.getPos();

  auto retry = [&]() -> Result<> {
    // We failed to parse. Maybe the data index was accidentally parsed as the
    // optional memory index. Try again without parsing a memory index.
    WithPosition with(ctx, reset);
    auto data = dataidx(ctx);
    CHECK_ERR(data);
    return ctx.makeMemoryInit(pos, annotations, nullptr, *data);
  };

  auto mem = maybeMemidx(ctx);
  if (mem.getErr()) {
    return retry();
  }
  auto data = dataidx(ctx);
  if (data.getErr()) {
    return retry();
  }
  return ctx.makeMemoryInit(pos, annotations, mem.getPtr(), *data);
}

template<typename Ctx>
Result<>
makeDataDrop(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto data = dataidx(ctx);
  CHECK_ERR(data);
  return ctx.makeDataDrop(pos, annotations, *data);
}

template<typename Ctx>
Result<> makeMemoryCopy(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations) {
  auto destMem = maybeMemidx(ctx);
  CHECK_ERR(destMem);
  std::optional<typename Ctx::MemoryIdxT> srcMem = std::nullopt;
  if (destMem) {
    auto mem = memidx(ctx);
    CHECK_ERR(mem);
    srcMem = *mem;
  }
  return ctx.makeMemoryCopy(
    pos, annotations, destMem.getPtr(), srcMem ? &*srcMem : nullptr);
}

template<typename Ctx>
Result<> makeMemoryFill(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations) {
  auto mem = maybeMemidx(ctx);
  CHECK_ERR(mem);
  return ctx.makeMemoryFill(pos, annotations, mem.getPtr());
}

template<typename Ctx>
Result<>
makePop(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = valtype(ctx);
  CHECK_ERR(type);
  return ctx.makePop(pos, annotations, *type);
}

template<typename Ctx>
Result<> makeCall(Ctx& ctx,
                  Index pos,
                  const std::vector<Annotation>& annotations,
                  bool isReturn) {
  auto func = funcidx(ctx);
  CHECK_ERR(func);
  return ctx.makeCall(pos, annotations, *func, isReturn);
}

template<typename Ctx>
Result<> makeCallIndirect(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations,
                          bool isReturn) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  auto type = typeuse(ctx, false);
  CHECK_ERR(type);
  return ctx.makeCallIndirect(
    pos, annotations, table.getPtr(), *type, isReturn);
}

template<typename Ctx>
Result<> makeBreak(Ctx& ctx,
                   Index pos,
                   const std::vector<Annotation>& annotations,
                   bool isConditional) {
  auto label = labelidx(ctx);
  CHECK_ERR(label);
  return ctx.makeBreak(pos, annotations, *label, isConditional);
}

template<typename Ctx>
Result<> makeBreakTable(Ctx& ctx,
                        Index pos,
                        const std::vector<Annotation>& annotations) {
  std::vector<typename Ctx::LabelIdxT> labels;
  // Parse at least one label; return an error only if we parse none.
  while (true) {
    auto label = maybeLabelidx(ctx);
    if (!label) {
      break;
    }
    CHECK_ERR(label);
    labels.push_back(*label);
  }
  if (labels.empty()) {
    return ctx.in.err("expected label");
  }
  auto defaultLabel = labels.back();
  labels.pop_back();
  return ctx.makeSwitch(pos, annotations, labels, defaultLabel);
}

template<typename Ctx>
Result<>
makeReturn(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeReturn(pos, annotations);
}

template<typename Ctx>
Result<>
makeRefNull(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto t = heaptype(ctx);
  CHECK_ERR(t);
  return ctx.makeRefNull(pos, annotations, *t);
}

template<typename Ctx>
Result<>
makeRefIsNull(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeRefIsNull(pos, annotations);
}

template<typename Ctx>
Result<>
makeRefFunc(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto func = funcidx(ctx);
  CHECK_ERR(func);
  return ctx.makeRefFunc(pos, annotations, *func);
}

template<typename Ctx>
Result<>
makeRefEq(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeRefEq(pos, annotations);
}

template<typename Ctx>
Result<>
makeTableGet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  return ctx.makeTableGet(pos, annotations, table.getPtr());
}

template<typename Ctx>
Result<>
makeTableSet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  return ctx.makeTableSet(pos, annotations, table.getPtr());
}

template<typename Ctx>
Result<>
makeTableSize(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  return ctx.makeTableSize(pos, annotations, table.getPtr());
}

template<typename Ctx>
Result<>
makeTableGrow(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  return ctx.makeTableGrow(pos, annotations, table.getPtr());
}

template<typename Ctx>
Result<>
makeTableFill(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  return ctx.makeTableFill(pos, annotations, table.getPtr());
}

template<typename Ctx>
Result<>
makeTableCopy(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto destTable = maybeTableidx(ctx);
  CHECK_ERR(destTable);
  auto srcTable = maybeTableidx(ctx);
  CHECK_ERR(srcTable);
  if (destTable && !srcTable) {
    return ctx.in.err("expected table index or identifier");
  }
  return ctx.makeTableCopy(
    pos, annotations, destTable.getPtr(), srcTable.getPtr());
}

template<typename Ctx>
Result<>
makeTableInit(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto table = maybeTableidx(ctx);
  CHECK_ERR(table);
  auto elem = elemidx(ctx);
  CHECK_ERR(elem);
  return ctx.makeTableInit(pos, annotations, table.getPtr(), *elem);
}

template<typename Ctx>
Result<>
makeThrow(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto tag = tagidx(ctx);
  CHECK_ERR(tag);
  return ctx.makeThrow(pos, annotations, *tag);
}

template<typename Ctx>
Result<>
makeRethrow(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto label = labelidx(ctx);
  CHECK_ERR(label);
  return ctx.makeRethrow(pos, annotations, *label);
}

template<typename Ctx>
Result<>
makeThrowRef(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeThrowRef(pos, annotations);
}

template<typename Ctx>
Result<>
makeTupleMake(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto arity = tupleArity(ctx);
  CHECK_ERR(arity);
  return ctx.makeTupleMake(pos, annotations, *arity);
}

template<typename Ctx>
Result<> makeTupleExtract(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations) {
  auto arity = tupleArity(ctx);
  CHECK_ERR(arity);
  auto index = ctx.in.takeU32();
  if (!index) {
    return ctx.in.err("expected tuple index");
  }
  return ctx.makeTupleExtract(pos, annotations, *arity, *index);
}

template<typename Ctx>
Result<>
makeTupleDrop(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto arity = tupleArity(ctx);
  CHECK_ERR(arity);
  return ctx.makeTupleDrop(pos, annotations, *arity);
}

template<typename Ctx>
Result<> makeCallRef(Ctx& ctx,
                     Index pos,
                     const std::vector<Annotation>& annotations,
                     bool isReturn) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  return ctx.makeCallRef(pos, annotations, *type, isReturn);
}

template<typename Ctx>
Result<> makeRefI31(Ctx& ctx,
                    Index pos,
                    const std::vector<Annotation>& annotations,
                    Shareability share) {
  return ctx.makeRefI31(pos, annotations, share);
}

template<typename Ctx>
Result<> makeI31Get(Ctx& ctx,
                    Index pos,
                    const std::vector<Annotation>& annotations,
                    bool signed_) {
  return ctx.makeI31Get(pos, annotations, signed_);
}

template<typename Ctx>
Result<>
makeRefTest(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = reftype(ctx);
  CHECK_ERR(type);
  return ctx.makeRefTest(pos, annotations, *type);
}

template<typename Ctx>
Result<>
makeRefCast(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = reftype(ctx);
  CHECK_ERR(type);
  return ctx.makeRefCast(pos, annotations, *type);
}

template<typename Ctx>
Result<> makeBrOnNull(Ctx& ctx,
                      Index pos,
                      const std::vector<Annotation>& annotations,
                      bool onFail) {
  auto label = labelidx(ctx);
  CHECK_ERR(label);
  return ctx.makeBrOn(
    pos, annotations, *label, onFail ? BrOnNonNull : BrOnNull);
}

template<typename Ctx>
Result<> makeBrOnCast(Ctx& ctx,
                      Index pos,
                      const std::vector<Annotation>& annotations,
                      bool onFail) {
  auto label = labelidx(ctx);
  CHECK_ERR(label);
  auto in = reftype(ctx);
  CHECK_ERR(in);
  auto out = reftype(ctx);
  CHECK_ERR(out);
  return ctx.makeBrOn(
    pos, annotations, *label, onFail ? BrOnCastFail : BrOnCast, *in, *out);
}

template<typename Ctx>
Result<> makeStructNew(Ctx& ctx,
                       Index pos,
                       const std::vector<Annotation>& annotations,
                       bool default_) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  if (default_) {
    return ctx.makeStructNewDefault(pos, annotations, *type);
  }
  return ctx.makeStructNew(pos, annotations, *type);
}

template<typename Ctx>
Result<> makeStructGet(Ctx& ctx,
                       Index pos,
                       const std::vector<Annotation>& annotations,
                       bool signed_) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto field = fieldidx(ctx, *type);
  CHECK_ERR(field);
  return ctx.makeStructGet(pos, annotations, *type, *field, signed_);
}

template<typename Ctx>
Result<>
makeStructSet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto field = fieldidx(ctx, *type);
  CHECK_ERR(field);
  return ctx.makeStructSet(pos, annotations, *type, *field);
}

template<typename Ctx>
Result<> makeArrayNew(Ctx& ctx,
                      Index pos,
                      const std::vector<Annotation>& annotations,
                      bool default_) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  if (default_) {
    return ctx.makeArrayNewDefault(pos, annotations, *type);
  }
  return ctx.makeArrayNew(pos, annotations, *type);
}

template<typename Ctx>
Result<> makeArrayNewData(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto data = dataidx(ctx);
  CHECK_ERR(data);
  return ctx.makeArrayNewData(pos, annotations, *type, *data);
}

template<typename Ctx>
Result<> makeArrayNewElem(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto elem = elemidx(ctx);
  CHECK_ERR(elem);
  return ctx.makeArrayNewElem(pos, annotations, *type, *elem);
}

template<typename Ctx>
Result<> makeArrayNewFixed(Ctx& ctx,
                           Index pos,
                           const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto arity = ctx.in.takeU32();
  if (!arity) {
    return ctx.in.err(pos, "expected array.new_fixed arity");
  }
  return ctx.makeArrayNewFixed(pos, annotations, *type, *arity);
}

template<typename Ctx>
Result<> makeArrayGet(Ctx& ctx,
                      Index pos,
                      const std::vector<Annotation>& annotations,
                      bool signed_) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  return ctx.makeArrayGet(pos, annotations, *type, signed_);
}

template<typename Ctx>
Result<>
makeArraySet(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  return ctx.makeArraySet(pos, annotations, *type);
}

template<typename Ctx>
Result<>
makeArrayLen(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  return ctx.makeArrayLen(pos, annotations);
}

template<typename Ctx>
Result<>
makeArrayCopy(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto destType = typeidx(ctx);
  CHECK_ERR(destType);
  auto srcType = typeidx(ctx);
  CHECK_ERR(srcType);
  return ctx.makeArrayCopy(pos, annotations, *destType, *srcType);
}

template<typename Ctx>
Result<>
makeArrayFill(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  return ctx.makeArrayFill(pos, annotations, *type);
}

template<typename Ctx>
Result<> makeArrayInitData(Ctx& ctx,
                           Index pos,
                           const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto data = dataidx(ctx);
  CHECK_ERR(data);
  return ctx.makeArrayInitData(pos, annotations, *type, *data);
}

template<typename Ctx>
Result<> makeArrayInitElem(Ctx& ctx,
                           Index pos,
                           const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto elem = elemidx(ctx);
  CHECK_ERR(elem);
  return ctx.makeArrayInitElem(pos, annotations, *type, *elem);
}

template<typename Ctx>
Result<> makeRefAs(Ctx& ctx,
                   Index pos,
                   const std::vector<Annotation>& annotations,
                   RefAsOp op) {
  return ctx.makeRefAs(pos, annotations, op);
}

template<typename Ctx>
Result<> makeStringNew(Ctx& ctx,
                       Index pos,
                       const std::vector<Annotation>& annotations,
                       StringNewOp op) {
  return ctx.makeStringNew(pos, annotations, op);
}

template<typename Ctx>
Result<> makeStringConst(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations) {
  auto str = ctx.in.takeString();
  if (!str) {
    return ctx.in.err("expected string");
  }
  return ctx.makeStringConst(pos, annotations, *str);
}

template<typename Ctx>
Result<> makeStringMeasure(Ctx& ctx,
                           Index pos,
                           const std::vector<Annotation>& annotations,
                           StringMeasureOp op) {
  return ctx.makeStringMeasure(pos, annotations, op);
}

template<typename Ctx>
Result<> makeStringEncode(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations,
                          StringEncodeOp op) {
  return ctx.makeStringEncode(pos, annotations, op);
}

template<typename Ctx>
Result<> makeStringConcat(Ctx& ctx,
                          Index pos,
                          const std::vector<Annotation>& annotations) {
  return ctx.makeStringConcat(pos, annotations);
}

template<typename Ctx>
Result<> makeStringEq(Ctx& ctx,
                      Index pos,
                      const std::vector<Annotation>& annotations,
                      StringEqOp op) {
  return ctx.makeStringEq(pos, annotations, op);
}

template<typename Ctx>
Result<> makeStringWTF16Get(Ctx& ctx,
                            Index pos,
                            const std::vector<Annotation>& annotations) {
  return ctx.makeStringWTF16Get(pos, annotations);
}

template<typename Ctx>
Result<> makeStringSliceWTF(Ctx& ctx,
                            Index pos,
                            const std::vector<Annotation>& annotations) {
  return ctx.makeStringSliceWTF(pos, annotations);
}

template<typename Ctx>
Result<>
makeContNew(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);

  return ctx.makeContNew(pos, annotations, *type);
}

// contbind ::= 'cont.bind' typeidx typeidx
template<typename Ctx>
Result<>
makeContBind(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto sourceType = typeidx(ctx);
  CHECK_ERR(sourceType);

  auto targetType = typeidx(ctx);
  CHECK_ERR(targetType);

  return ctx.makeContBind(pos, annotations, *sourceType, *targetType);
}

template<typename Ctx>
Result<>
makeSuspend(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto tag = tagidx(ctx);
  CHECK_ERR(tag);

  return ctx.makeSuspend(pos, annotations, *tag);
}

// resumetable ::= ('(' 'on' tagidx labelidx | 'on' tagidx switch ')')*
template<typename Ctx>
Result<typename Ctx::OnClauseListT> makeResumeTable(Ctx& ctx) {
  auto resumetable = ctx.makeOnClauseList();
  while (ctx.in.takeSExprStart("on"sv)) {
    auto tag = tagidx(ctx);
    CHECK_ERR(tag);
    if (ctx.in.takeKeyword("switch")) {
      ctx.appendOnClause(resumetable, ctx.makeOnSwitch(*tag));
    } else {
      auto label = labelidx(ctx);
      CHECK_ERR(label);
      ctx.appendOnClause(resumetable, ctx.makeOnLabel(*tag, *label));
    }
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected ')' at end of handler clause");
    }
  }
  return resumetable;
}

// resume ::= 'resume' typeidx resumetable
template<typename Ctx>
Result<>
makeResume(Ctx& ctx, Index pos, const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);

  auto resumetable = makeResumeTable(ctx);
  CHECK_ERR(resumetable);

  return ctx.makeResume(pos, annotations, *type, *resumetable);
}

// resume_throw ::= 'resume_throw' typeidx tagidx ('(' 'on' tagidx labelidx |
// 'on' tagidx switch ')')*
template<typename Ctx>
Result<> makeResumeThrow(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto exnTag = tagidx(ctx);
  CHECK_ERR(exnTag);

  auto resumetable = makeResumeTable(ctx);
  CHECK_ERR(resumetable);

  return ctx.makeResumeThrow(pos, annotations, *type, *exnTag, *resumetable);
}

// switch ::= 'switch' typeidx tagidx
template<typename Ctx>
Result<> makeStackSwitch(Ctx& ctx,
                         Index pos,
                         const std::vector<Annotation>& annotations) {
  auto type = typeidx(ctx);
  CHECK_ERR(type);
  auto tag = tagidx(ctx);
  CHECK_ERR(tag);

  return ctx.makeStackSwitch(pos, annotations, *type, *tag);
}

// =======
// Modules
// =======

// typeidx ::= x:u32 => x
//           | v:id  => x (if types[x] = v)
template<typename Ctx>
MaybeResult<typename Ctx::HeapTypeT> maybeTypeidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getHeapTypeFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    // TODO: Fix position to point to start of id, not next element.
    auto idx = ctx.getTypeIndex(*id);
    CHECK_ERR(idx);
    return ctx.getHeapTypeFromIdx(*idx);
  }
  return {};
}

template<typename Ctx> Result<typename Ctx::HeapTypeT> typeidx(Ctx& ctx) {
  if (auto t = maybeTypeidx(ctx)) {
    CHECK_ERR(t);
    return *t;
  }
  return ctx.in.err("expected type index or identifier");
}

// fieldidx ::= x:u32 => x
//            | v:id  => x (if t.fields[x] = v)
template<typename Ctx>
Result<typename Ctx::FieldIdxT> fieldidx(Ctx& ctx,
                                         typename Ctx::HeapTypeT type) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getFieldFromIdx(type, *x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getFieldFromName(type, *id);
  }
  return ctx.in.err("expected field index or identifier");
}

// funcidx ::= x:u32 => x
//           | v:id => x (if t.funcs[x] = v)
template<typename Ctx>
MaybeResult<typename Ctx::FuncIdxT> maybeFuncidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getFuncFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getFuncFromName(*id);
  }
  return {};
}

template<typename Ctx> Result<typename Ctx::FuncIdxT> funcidx(Ctx& ctx) {
  if (auto idx = maybeFuncidx(ctx)) {
    CHECK_ERR(idx);
    return *idx;
  }
  return ctx.in.err("expected function index or identifier");
}

// tableidx ::= x:u23 => x
//            | v:id => x (if tables[x] = v)
template<typename Ctx>
MaybeResult<typename Ctx::TableIdxT> maybeTableidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getTableFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getTableFromName(*id);
  }
  return {};
}

template<typename Ctx> Result<typename Ctx::TableIdxT> tableidx(Ctx& ctx) {
  if (auto idx = maybeTableidx(ctx)) {
    CHECK_ERR(idx);
    return *idx;
  }
  return ctx.in.err("expected table index or identifier");
}

// tableuse ::= '(' 'table' x:tableidx ')'
template<typename Ctx>
MaybeResult<typename Ctx::TableIdxT> maybeTableuse(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("table"sv)) {
    return {};
  }
  auto idx = tableidx(ctx);
  CHECK_ERR(idx);
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("Expected end of memory use");
  }
  return *idx;
}

// memidx ::= x:u32 => x
//          | v:id  => x (if memories[x] = v)
template<typename Ctx>
MaybeResult<typename Ctx::MemoryIdxT> maybeMemidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getMemoryFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getMemoryFromName(*id);
  }
  return {};
}

template<typename Ctx> Result<typename Ctx::MemoryIdxT> memidx(Ctx& ctx) {
  if (auto idx = maybeMemidx(ctx)) {
    CHECK_ERR(idx);
    return *idx;
  }
  return ctx.in.err("expected memory index or identifier");
}

// memuse ::= '(' 'memory' x:memidx ')' => x
template<typename Ctx>
MaybeResult<typename Ctx::MemoryIdxT> maybeMemuse(Ctx& ctx) {
  if (!ctx.in.takeSExprStart("memory"sv)) {
    return {};
  }
  auto idx = memidx(ctx);
  CHECK_ERR(idx);
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of memory use");
  }
  return *idx;
}

// globalidx ::= x:u32 => x
//             | v:id  => x (if globals[x] = v)
template<typename Ctx> Result<typename Ctx::GlobalIdxT> globalidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getGlobalFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getGlobalFromName(*id);
  }
  return ctx.in.err("expected global index or identifier");
}

// elemidx ::= x:u32 => x
//           | v:id => x (if elems[x] = v)
template<typename Ctx> Result<typename Ctx::ElemIdxT> elemidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getElemFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getElemFromName(*id);
  }
  return ctx.in.err("expected elem index or identifier");
}

// dataidx ::= x:u32 => x
//           | v:id => x (if datas[x] = v)
template<typename Ctx> Result<typename Ctx::DataIdxT> dataidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getDataFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getDataFromName(*id);
  }
  return ctx.in.err("expected data index or identifier");
}

// localidx ::= x:u32 => x
//            | v:id  => x (if locals[x] = v)
template<typename Ctx> Result<typename Ctx::LocalIdxT> localidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getLocalFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getLocalFromName(*id);
  }
  return ctx.in.err("expected local index or identifier");
}

template<typename Ctx>
Result<typename Ctx::LabelIdxT> labelidx(Ctx& ctx, bool inDelegate) {
  if (auto idx = maybeLabelidx(ctx, inDelegate)) {
    CHECK_ERR(idx);
    return *idx;
  }
  return ctx.in.err("expected label index or identifier");
}

// labelidx ::= x:u32 => x
//            | v:id => x (if labels[x] = v)
template<typename Ctx>
MaybeResult<typename Ctx::LabelIdxT> maybeLabelidx(Ctx& ctx, bool inDelegate) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getLabelFromIdx(*x, inDelegate);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getLabelFromName(*id, inDelegate);
  }
  return {};
}

// tagidx ::= x:u32 => x
//          | v:id => x (if tags[x] = v)
template<typename Ctx> Result<typename Ctx::TagIdxT> tagidx(Ctx& ctx) {
  if (auto x = ctx.in.takeU32()) {
    return ctx.getTagFromIdx(*x);
  }
  if (auto id = ctx.in.takeID()) {
    return ctx.getTagFromName(*id);
  }
  return ctx.in.err("expected tag index or identifier");
}

// typeuse ::= '(' 'type' x:typeidx ')'                                => x, []
//                 (if typedefs[x] = [t1*] -> [t2*]
//           | '(' 'type' x:typeidx ')' ((t1,IDs):param)* (t2:result)* => x, IDs
//                 (if typedefs[x] = [t1*] -> [t2*])
//           | ((t1,IDs):param)* (t2:result)*                          => x, IDs
//                 (if x is minimum s.t. typedefs[x] = [t1*] -> [t2*])
template<typename Ctx>
Result<typename Ctx::TypeUseT> typeuse(Ctx& ctx, bool allowNames) {
  auto pos = ctx.in.getPos();
  std::optional<typename Ctx::HeapTypeT> type;
  if (ctx.in.takeSExprStart("type"sv)) {
    auto x = typeidx(ctx);
    CHECK_ERR(x);

    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of type use");
    }

    type = *x;
  }

  auto namedParams = params(ctx, allowNames);
  CHECK_ERR(namedParams);

  auto resultTypes = results(ctx);
  CHECK_ERR(resultTypes);

  return ctx.makeTypeUse(pos, type, namedParams.getPtr(), resultTypes.getPtr());
}

// ('(' 'import' mod:name nm:name ')')?
inline MaybeResult<ImportNames> inlineImport(Lexer& in) {
  if (!in.takeSExprStart("import"sv)) {
    return {};
  }
  auto mod = in.takeName();
  if (!mod) {
    return in.err("expected import module");
  }
  auto nm = in.takeName();
  if (!nm) {
    return in.err("expected import name");
  }
  if (!in.takeRParen()) {
    return in.err("expected end of import");
  }
  // TODO: Return Ok when parsing Decls.
  return {{*mod, *nm}};
}

// ('(' 'export' name ')')*
inline Result<std::vector<Name>> inlineExports(Lexer& in) {
  std::vector<Name> exports;
  while (in.takeSExprStart("export"sv)) {
    auto name = in.takeName();
    if (!name) {
      return in.err("expected export name");
    }
    if (!in.takeRParen()) {
      return in.err("expected end of import");
    }
    exports.push_back(*name);
  }
  return exports;
}

// comptype ::= ft:functype   => ft
//            | ct:conttype   => ct
//            | st:structtype => st
//            | at:arraytype  => at
template<typename Ctx> Result<> comptype(Ctx& ctx) {
  if (auto type = functype(ctx)) {
    CHECK_ERR(type);
    ctx.addFuncType(*type);
    return Ok{};
  }
  if (auto type = conttype(ctx)) {
    CHECK_ERR(type);
    ctx.addContType(*type);
    return Ok{};
  }
  if (auto type = structtype(ctx)) {
    CHECK_ERR(type);
    ctx.addStructType(*type);
    return Ok{};
  }
  if (auto type = arraytype(ctx)) {
    CHECK_ERR(type);
    ctx.addArrayType(*type);
    return Ok{};
  }
  return ctx.in.err("expected type description");
}

// sharecomptype ::= '(' 'shared' t:comptype ')' => shared t
//                 | t:comptype => unshared t
template<typename Ctx> Result<> sharecomptype(Ctx& ctx) {
  if (ctx.in.takeSExprStart("shared"sv)) {
    ctx.setShared();
    CHECK_ERR(comptype(ctx));
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of shared comptype");
    }
  } else {
    CHECK_ERR(comptype(ctx));
  }
  return Ok{};
}

// subtype ::= '(' 'sub' typeidx? sharecomptype ')'  | sharecomptype
template<typename Ctx> Result<> subtype(Ctx& ctx) {
  if (ctx.in.takeSExprStart("sub"sv)) {
    if (!ctx.in.takeKeyword("final"sv)) {
      ctx.setOpen();
    }
    if (auto super = maybeTypeidx(ctx)) {
      CHECK_ERR(super);
      CHECK_ERR(ctx.addSubtype(*super));
    }

    CHECK_ERR(sharecomptype(ctx));

    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of subtype definition");
    }
  } else {
    CHECK_ERR(sharecomptype(ctx));
  }
  return Ok{};
}

// typedef ::= '(' 'type' id? subtype ')'
template<typename Ctx> MaybeResult<> typedef_(Ctx& ctx) {
  auto pos = ctx.in.getPos();

  if (!ctx.in.takeSExprStart("type"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto sub = subtype(ctx);
  CHECK_ERR(sub);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of type definition");
  }

  ctx.finishTypeDef(name, pos);
  return Ok{};
}

// rectype ::= '(' 'rec' subtype* ')'
//           | subtype
template<typename Ctx> MaybeResult<> rectype(Ctx& ctx) {
  auto pos = ctx.in.getPos();

  if (ctx.in.takeSExprStart("rec"sv)) {
    size_t startIndex = ctx.getRecGroupStartIndex();
    size_t groupLen = 0;
    while (auto type = typedef_(ctx)) {
      CHECK_ERR(type);
      ++groupLen;
    }
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected type definition or end of recursion group");
    }
    ctx.addRecGroup(startIndex, groupLen);
  } else if (auto type = typedef_(ctx)) {
    CHECK_ERR(type);
  } else {
    return {};
  }

  ctx.finishRectype(pos);
  return Ok{};
}

// local  ::= '(' 'local id? t:valtype ')' => [t]
//          | '(' 'local t*:valtype* ')' => [t*]
// locals ::= local*
template<typename Ctx> MaybeResult<typename Ctx::LocalsT> locals(Ctx& ctx) {
  bool hasAny = false;
  auto res = ctx.makeLocals();
  while (ctx.in.takeSExprStart("local"sv)) {
    hasAny = true;
    if (auto id = ctx.in.takeID()) {
      // Single named local
      auto type = valtype(ctx);
      CHECK_ERR(type);
      if (!ctx.in.takeRParen()) {
        return ctx.in.err("expected end of local");
      }
      ctx.appendLocal(res, *id, *type);
    } else {
      // Repeated unnamed locals
      while (!ctx.in.takeRParen()) {
        auto type = valtype(ctx);
        CHECK_ERR(type);
        ctx.appendLocal(res, {}, *type);
      }
    }
  }
  if (hasAny) {
    return res;
  }
  return {};
}

// import ::= '(' 'import' mod:name nm:name importdesc ')'
// importdesc ::= '(' 'func' id? typeuse ')'
//              | '(' 'table' id? tabletype ')'
//              | '(' 'memory' id? memtype ')'
//              | '(' 'global' id? globaltype ')'
//              | '(' 'tag' id? typeuse ')'
template<typename Ctx> MaybeResult<> import_(Ctx& ctx) {
  auto pos = ctx.in.getPos();

  if (!ctx.in.takeSExprStart("import"sv)) {
    return {};
  }

  auto mod = ctx.in.takeName();
  if (!mod) {
    return ctx.in.err("expected import module name");
  }

  auto nm = ctx.in.takeName();
  if (!nm) {
    return ctx.in.err("expected import name");
  }
  ImportNames names{*mod, *nm};

  if (ctx.in.takeSExprStart("func"sv)) {
    auto name = ctx.in.takeID();
    auto type = typeuse(ctx);
    CHECK_ERR(type);
    // TODO: function import annotations
    CHECK_ERR(ctx.addFunc(
      name ? *name : Name{}, {}, &names, *type, std::nullopt, {}, pos));
  } else if (ctx.in.takeSExprStart("table"sv)) {
    auto name = ctx.in.takeID();
    auto type = tabletype(ctx);
    CHECK_ERR(type);
    CHECK_ERR(ctx.addTable(name ? *name : Name{}, {}, &names, *type, pos));
  } else if (ctx.in.takeSExprStart("memory"sv)) {
    auto name = ctx.in.takeID();
    auto type = memtype(ctx);
    CHECK_ERR(type);
    CHECK_ERR(ctx.addMemory(name ? *name : Name{}, {}, &names, *type, pos));
  } else if (ctx.in.takeSExprStart("global"sv)) {
    auto name = ctx.in.takeID();
    auto type = globaltype(ctx);
    CHECK_ERR(type);
    CHECK_ERR(ctx.addGlobal(
      name ? *name : Name{}, {}, &names, *type, std::nullopt, pos));
  } else if (ctx.in.takeSExprStart("tag"sv)) {
    auto name = ctx.in.takeID();
    auto type = typeuse(ctx);
    CHECK_ERR(type);
    CHECK_ERR(ctx.addTag(name ? *name : Name{}, {}, &names, *type, pos));
  } else {
    return ctx.in.err("expected import description");
  }

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of import description");
  }
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of import");
  }

  return Ok{};
}

// func ::= '(' 'func' id? ('(' 'export' name ')')*
//              x,I:typeuse t*:vec(local) (in:instr)* ')'
//        | '(' 'func' id? ('(' 'export' name ')')*
//              '(' 'import' mod:name nm:name ')' typeuse ')'
template<typename Ctx> MaybeResult<> func(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  auto annotations = ctx.in.getAnnotations();

  if (!ctx.in.takeSExprStart("func"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto exports = inlineExports(ctx.in);
  CHECK_ERR(exports);

  auto import = inlineImport(ctx.in);
  CHECK_ERR(import);

  auto type = typeuse(ctx);
  CHECK_ERR(type);

  std::optional<typename Ctx::LocalsT> localVars;
  if (!import) {
    if (auto l = locals(ctx)) {
      CHECK_ERR(l);
      localVars = *l;
    }
    if (!ctx.skipFunctionBody()) {
      CHECK_ERR(instrs(ctx));
      ctx.setSrcLoc(ctx.in.takeAnnotations());
    }
  }

  if (!ctx.skipFunctionBody() && !ctx.in.takeRParen()) {
    return ctx.in.err("expected end of function");
  }

  CHECK_ERR(ctx.addFunc(name,
                        *exports,
                        import.getPtr(),
                        *type,
                        localVars,
                        std::move(annotations),
                        pos));
  return Ok{};
}

// table ::= '(' 'table' id? ('(' 'export' name ')')*
//               '(' 'import' mod:name nm:name ')'? index_type? tabletype ')'
//         | '(' 'table' id? ('(' 'export' name ')')* index_type?
//               reftype '(' 'elem' (elemexpr* | funcidx*) ')' ')'
template<typename Ctx> MaybeResult<> table(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("table"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto exports = inlineExports(ctx.in);
  CHECK_ERR(exports);

  auto import = inlineImport(ctx.in);
  CHECK_ERR(import);

  auto addressType = Type::i32;
  if (ctx.in.takeKeyword("i64"sv)) {
    addressType = Type::i64;
  } else {
    ctx.in.takeKeyword("i32"sv);
  }

  // Reftype if we have inline elements.
  auto type = maybeReftype(ctx);
  CHECK_ERR(type);

  std::optional<typename Ctx::TableTypeT> ttype;
  std::optional<typename Ctx::ElemListT> elems;
  if (type) {
    // We should have inline elements.
    if (!ctx.in.takeSExprStart("elem"sv)) {
      return ctx.in.err("expected table limits or inline elements");
    }
    if (import) {
      return ctx.in.err("imported tables cannot have inline elements");
    }

    auto list = ctx.makeElemList(*type);
    bool foundElem = false;
    while (auto elem = maybeElemexpr(ctx)) {
      CHECK_ERR(elem);
      ctx.appendElem(list, *elem);
      foundElem = true;
    }

    // If there were no elemexprs, then maybe we have funcidxs instead.
    if (!foundElem) {
      while (auto func = maybeFuncidx(ctx)) {
        CHECK_ERR(func);
        ctx.appendFuncElem(list, *func);
      }
    }

    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of inline elems");
    }
    ttype = ctx.makeTableType(addressType, ctx.getLimitsFromElems(list), *type);
    elems = std::move(list);
  } else {
    auto tabtype = tabletypeContinued(ctx, addressType);
    CHECK_ERR(tabtype);
    ttype = *tabtype;
  }

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of table declaration");
  }

  CHECK_ERR(ctx.addTable(name, *exports, import.getPtr(), *ttype, pos));

  if (elems) {
    CHECK_ERR(ctx.addImplicitElems(*type, std::move(*elems)));
  }

  return Ok{};
}

// memory ::= '(' 'memory' id? ('(' 'export' name ')')* index_type?
//                ('(' 'data' b:datastring ')' | memtype) ')'
//          | '(' 'memory' id? ('(' 'export' name ')')*
//                '(' 'import' mod:name nm:name ')' index_type? memtype ')'
template<typename Ctx> MaybeResult<> memory(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("memory"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto exports = inlineExports(ctx.in);
  CHECK_ERR(exports);

  auto import = inlineImport(ctx.in);
  CHECK_ERR(import);

  auto addressType = Type::i32;
  if (ctx.in.takeKeyword("i64"sv)) {
    addressType = Type::i64;
  } else {
    ctx.in.takeKeyword("i32"sv);
  }

  std::optional<typename Ctx::MemTypeT> mtype;
  std::optional<typename Ctx::DataStringT> data;
  if (ctx.in.takeSExprStart("data"sv)) {
    if (import) {
      return ctx.in.err("imported memories cannot have inline data");
    }
    auto datastr = datastring(ctx);
    CHECK_ERR(datastr);
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of inline data");
    }
    mtype =
      ctx.makeMemType(addressType, ctx.getLimitsFromData(*datastr), false);
    data = *datastr;
  } else {
    auto type = memtypeContinued(ctx, addressType);
    CHECK_ERR(type);
    mtype = *type;
  }

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of memory declaration");
  }

  CHECK_ERR(ctx.addMemory(name, *exports, import.getPtr(), *mtype, pos));

  if (data) {
    CHECK_ERR(ctx.addImplicitData(std::move(*data)));
  }

  return Ok{};
}

// global ::= '(' 'global' id? ('(' 'export' name ')')* gt:globaltype e:expr ')'
//          | '(' 'global' id? ('(' 'export' name ')')*
//                '(' 'import' mod:name nm:name ')' gt:globaltype ')'
template<typename Ctx> MaybeResult<> global(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("global"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto exports = inlineExports(ctx.in);
  CHECK_ERR(exports);

  auto import = inlineImport(ctx.in);
  CHECK_ERR(import);

  auto type = globaltype(ctx);
  CHECK_ERR(type);

  std::optional<typename Ctx::ExprT> exp;
  if (!import) {
    auto e = expr(ctx);
    CHECK_ERR(e);
    exp = *e;
  }

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of global");
  }

  CHECK_ERR(ctx.addGlobal(name, *exports, import.getPtr(), *type, exp, pos));
  return Ok{};
}

// export ::= '(' 'export' nm:name exportdesc ')'
// exportdesc ::= '(' 'func' x:funcidx ')'
//              | '(' 'table' x:tableidx ')'
//              | '(' 'memory' x:memidx ')'
//              | '(' 'global' x:globalidx ')'
//              | '(' 'tag' x:tagidx ')'
template<typename Ctx> MaybeResult<> export_(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("export"sv)) {
    return {};
  }

  auto name = ctx.in.takeName();
  if (!name) {
    return ctx.in.err("expected export name");
  }

  if (ctx.in.takeSExprStart("func"sv)) {
    auto idx = funcidx(ctx);
    CHECK_ERR(idx);
    CHECK_ERR(ctx.addExport(pos, *idx, *name, ExternalKind::Function));
  } else if (ctx.in.takeSExprStart("table"sv)) {
    auto idx = tableidx(ctx);
    CHECK_ERR(idx);
    CHECK_ERR(ctx.addExport(pos, *idx, *name, ExternalKind::Table));
  } else if (ctx.in.takeSExprStart("memory"sv)) {
    auto idx = memidx(ctx);
    CHECK_ERR(idx);
    CHECK_ERR(ctx.addExport(pos, *idx, *name, ExternalKind::Memory));
  } else if (ctx.in.takeSExprStart("global"sv)) {
    auto idx = globalidx(ctx);
    CHECK_ERR(idx);
    CHECK_ERR(ctx.addExport(pos, *idx, *name, ExternalKind::Global));
  } else if (ctx.in.takeSExprStart("tag"sv)) {
    auto idx = tagidx(ctx);
    CHECK_ERR(idx);
    CHECK_ERR(ctx.addExport(pos, *idx, *name, ExternalKind::Tag));
  } else {
    return ctx.in.err("expected export description");
  }

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of export description");
  }
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of export");
  }
  return Ok{};
}

// start ::= '(' 'start' funcidx ')'
template<typename Ctx> MaybeResult<> start(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("start"sv)) {
    return {};
  }
  auto func = funcidx(ctx);
  CHECK_ERR(func);

  CHECK_ERR(ctx.addStart(*func, pos));

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of start declaration");
  }
  return Ok{};
}

// elemexpr ::= '(' 'item' expr ')' | '(' instr ')'
template<typename Ctx>
MaybeResult<typename Ctx::ExprT> maybeElemexpr(Ctx& ctx) {
  MaybeResult<typename Ctx::ExprT> result;
  if (ctx.in.takeSExprStart("item"sv)) {
    result = expr(ctx);
  } else if (ctx.in.takeLParen()) {
    // TODO: `instr` should included both folded and unfolded instrs.
    if (auto inst = instr(ctx)) {
      CHECK_ERR(inst);
    } else {
      return ctx.in.err("expected instruction");
    }
    result = ctx.makeExpr();
  } else {
    return {};
  }
  CHECK_ERR(result);
  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of element expression");
  }
  return result;
}

// elemlist ::= reftype elemexpr* | 'func' funcidx*
//            | funcidx* (iff the tableuse is omitted)
template<typename Ctx>
Result<typename Ctx::ElemListT> elemlist(Ctx& ctx, bool legacy) {
  if (auto type = maybeReftype(ctx)) {
    CHECK_ERR(type);
    auto res = ctx.makeElemList(*type);
    while (auto elem = maybeElemexpr(ctx)) {
      CHECK_ERR(elem);
      ctx.appendElem(res, *elem);
    }
    return res;
  } else if (ctx.in.takeKeyword("func"sv) || legacy) {
    auto res = ctx.makeFuncElemList();
    while (auto func = maybeFuncidx(ctx)) {
      CHECK_ERR(func);
      ctx.appendFuncElem(res, *func);
    }
    return res;
  }
  return ctx.in.err("expected element list");
}

// elem ::= '(' 'elem' id? x:tableuse? ('(' ('offset' e:expr | e:instr) ')')?
//               elemlist ')'
//        | '(' 'elem' id? 'declare' elemlist ')'
template<typename Ctx> MaybeResult<> elem(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("elem"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  bool isDeclare = false;
  MaybeResult<typename Ctx::TableIdxT> table;
  std::optional<typename Ctx::ExprT> offset;

  if (ctx.in.takeKeyword("declare"sv)) {
    isDeclare = true;
  } else {
    table = maybeTableuse(ctx);
    CHECK_ERR(table);

    if (ctx.in.takeSExprStart("offset")) {
      auto off = expr(ctx);
      CHECK_ERR(off);
      offset = *off;
    } else {
      // This may be an abbreviated offset instruction or it may be the
      // beginning of the elemlist.
      auto beforeLParen = ctx.in.getPos();
      if (ctx.in.takeLParen()) {
        if (auto inst = instr(ctx)) {
          CHECK_ERR(inst);
          auto off = ctx.makeExpr();
          CHECK_ERR(off);
          offset = *off;
        } else {
          // This must be the beginning of the elemlist instead.
          ctx.in.setPos(beforeLParen);
        }
      }
    }
    if (offset && !ctx.in.takeRParen()) {
      return ctx.in.err("expected end of offset expression");
    }
  }

  // If there is no explicit tableuse, we can use the legacy elemlist format.
  bool legacy = !table;
  auto elems = elemlist(ctx, legacy);
  CHECK_ERR(elems);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of element segment");
  }

  if (isDeclare) {
    CHECK_ERR(ctx.addDeclareElem(name, std::move(*elems), pos));
  } else {
    CHECK_ERR(
      ctx.addElem(name, table.getPtr(), offset, std::move(*elems), pos));
  }

  return Ok{};
}

// datastring ::= (b:string)* => concat(b*)
template<typename Ctx> Result<typename Ctx::DataStringT> datastring(Ctx& ctx) {
  auto data = ctx.makeDataString();
  while (auto str = ctx.in.takeString()) {
    ctx.appendDataString(data, *str);
  }
  return data;
}

// data ::= '(' 'data' id? b*:datastring ')' => {init b*, mode passive}
//        | '(' 'data' id? x:memuse? ('(' 'offset' e:expr ')' | e:instr)
//               b*:datastring ')
//             => {init b*, mode active {memory x, offset e}}
template<typename Ctx> MaybeResult<> data(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("data"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto mem = maybeMemuse(ctx);
  CHECK_ERR(mem);

  std::optional<typename Ctx::ExprT> offset;
  if (ctx.in.takeSExprStart("offset"sv)) {
    auto e = expr(ctx);
    CHECK_ERR(e);
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of offset expression");
    }
    offset = *e;
  } else if (ctx.in.takeLParen()) {
    CHECK_ERR(instr(ctx));
    auto offsetExpr = ctx.makeExpr();
    CHECK_ERR(offsetExpr);
    offset = *offsetExpr;
    if (!ctx.in.takeRParen()) {
      return ctx.in.err("expected end of offset instruction");
    }
  }

  if (mem && !offset) {
    return ctx.in.err("expected offset for active segment");
  }

  auto str = datastring(ctx);
  CHECK_ERR(str);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of data segment");
  }

  CHECK_ERR(ctx.addData(name, mem.getPtr(), offset, std::move(*str), pos));

  return Ok{};
}

// tag ::= '(' 'tag' id? ('(' 'export' name ')')*
//            ('(' 'import' mod:name nm:name ')')? typeuse ')'
template<typename Ctx> MaybeResult<> tag(Ctx& ctx) {
  auto pos = ctx.in.getPos();
  if (!ctx.in.takeSExprStart("tag"sv)) {
    return {};
  }

  Name name;
  if (auto id = ctx.in.takeID()) {
    name = *id;
  }

  auto exports = inlineExports(ctx.in);
  CHECK_ERR(exports);

  auto import = inlineImport(ctx.in);
  CHECK_ERR(import);

  auto type = typeuse(ctx);
  CHECK_ERR(type);

  if (!ctx.in.takeRParen()) {
    return ctx.in.err("expected end of tag");
  }

  CHECK_ERR(ctx.addTag(name, *exports, import.getPtr(), *type, pos));
  return Ok{};
}

// modulefield ::= deftype
//               | import
//               | func
//               | table
//               | memory
//               | global
//               | export
//               | start
//               | elem
//               | data
//               | tag
template<typename Ctx> MaybeResult<> modulefield(Ctx& ctx) {
  if (ctx.in.empty() || ctx.in.peekRParen()) {
    return {};
  }
  if (auto res = rectype(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = import_(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = func(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = table(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = memory(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = global(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = export_(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = start(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = elem(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = data(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  if (auto res = tag(ctx)) {
    CHECK_ERR(res);
    return Ok{};
  }
  return ctx.in.err("unrecognized module field");
}

// module ::= '(' 'module' id? (m:modulefield)* ')'
//          | (m:modulefield)* eof
template<typename Ctx> Result<> module(Ctx& ctx) {
  bool outer = ctx.in.takeSExprStart("module"sv);

  if (outer) {
    if (auto id = ctx.in.takeID()) {
      ctx.wasm.name = *id;
    }
  }

  while (auto field = modulefield(ctx)) {
    CHECK_ERR(field);
  }

  if (outer && !ctx.in.takeRParen()) {
    return ctx.in.err("expected end of module");
  }

  return Ok{};
}

} // namespace wasm::WATParser

#endif // parser_parsers_h
