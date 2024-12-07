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

#ifndef parser_context_h
#define parser_context_h

#include "common.h"
#include "ir/names.h"
#include "lexer.h"
#include "support/name.h"
#include "support/result.h"
#include "support/string.h"
#include "wasm-builder.h"
#include "wasm-ir-builder.h"
#include "wasm.h"

namespace wasm::WATParser {

using IndexMap = std::unordered_map<Name, Index>;

inline std::vector<Type> getUnnamedTypes(const std::vector<NameType>& named) {
  std::vector<Type> types;
  types.reserve(named.size());
  for (auto& t : named) {
    types.push_back(t.type);
  }
  return types;
}

struct Limits {
  uint64_t initial;
  std::optional<uint64_t> max;
};

struct MemType {
  Type addressType;
  Limits limits;
  bool shared;
};

struct Memarg {
  uint64_t offset;
  uint32_t align;
};

struct TableType {
  Type addressType;
  Limits limits;
};

// The location, possible name, and index in the respective module index space
// of a module-level definition in the input.
struct DefPos {
  Name name;
  Index pos;
  Index index;
  std::vector<Annotation> annotations;
};

struct GlobalType {
  Mutability mutability;
  Type type;
};

// A signature type and parameter names (possibly empty), used for parsing
// function types.
struct TypeUse {
  HeapType type;
  std::vector<Name> names;
};

struct NullTypeParserCtx {
  using IndexT = Ok;
  using HeapTypeT = Ok;
  using TupleElemListT = Ok;
  using TypeT = Ok;
  using ParamsT = Ok;
  using ResultsT = size_t;
  using BlockTypeT = Ok;
  using SignatureT = Ok;
  using ContinuationT = Ok;
  using StorageT = Ok;
  using FieldT = Ok;
  using FieldsT = Ok;
  using StructT = Ok;
  using ArrayT = Ok;
  using LimitsT = Ok;
  using MemTypeT = Ok;
  using GlobalTypeT = Ok;
  using TypeUseT = Ok;
  using LocalsT = Ok;
  using ElemListT = Ok;
  using DataStringT = Ok;

  HeapTypeT makeFuncType(Shareability) { return Ok{}; }
  HeapTypeT makeAnyType(Shareability) { return Ok{}; }
  HeapTypeT makeExternType(Shareability) { return Ok{}; }
  HeapTypeT makeEqType(Shareability) { return Ok{}; }
  HeapTypeT makeI31Type(Shareability) { return Ok{}; }
  HeapTypeT makeStructType(Shareability) { return Ok{}; }
  HeapTypeT makeArrayType(Shareability) { return Ok{}; }
  HeapTypeT makeExnType(Shareability) { return Ok{}; }
  HeapTypeT makeStringType(Shareability) { return Ok{}; }
  HeapTypeT makeContType(Shareability) { return Ok{}; }
  HeapTypeT makeNoneType(Shareability) { return Ok{}; }
  HeapTypeT makeNoextType(Shareability) { return Ok{}; }
  HeapTypeT makeNofuncType(Shareability) { return Ok{}; }
  HeapTypeT makeNoexnType(Shareability) { return Ok{}; }
  HeapTypeT makeNocontType(Shareability) { return Ok{}; }

  TypeT makeI32() { return Ok{}; }
  TypeT makeI64() { return Ok{}; }
  TypeT makeF32() { return Ok{}; }
  TypeT makeF64() { return Ok{}; }
  TypeT makeV128() { return Ok{}; }

  TypeT makeRefType(HeapTypeT, Nullability) { return Ok{}; }

  TupleElemListT makeTupleElemList() { return Ok{}; }
  void appendTupleElem(TupleElemListT&, TypeT) {}
  TypeT makeTupleType(TupleElemListT) { return Ok{}; }

  ParamsT makeParams() { return Ok{}; }
  void appendParam(ParamsT&, Name, TypeT) {}

  // We have to count results because whether or not a block introduces a
  // typeuse that may implicitly define a type depends on how many results it
  // has.
  size_t makeResults() { return 0; }
  void appendResult(size_t& results, TypeT) { ++results; }
  size_t getResultsSize(size_t results) { return results; }

  SignatureT makeFuncType(ParamsT*, ResultsT*) { return Ok{}; }
  ContinuationT makeContType(HeapTypeT) { return Ok{}; }

  StorageT makeI8() { return Ok{}; }
  StorageT makeI16() { return Ok{}; }
  StorageT makeStorageType(TypeT) { return Ok{}; }

  FieldT makeFieldType(StorageT, Mutability) { return Ok{}; }

  FieldsT makeFields() { return Ok{}; }
  void appendField(FieldsT&, Name, FieldT) {}

  StructT makeStruct(FieldsT&) { return Ok{}; }

  std::optional<ArrayT> makeArray(FieldsT&) { return Ok{}; }

  GlobalTypeT makeGlobalType(Mutability, TypeT) { return Ok{}; }

  LocalsT makeLocals() { return Ok{}; }
  void appendLocal(LocalsT&, Name, TypeT) {}

  Result<Index> getTypeIndex(Name) { return 1; }
  Result<HeapTypeT> getHeapTypeFromIdx(Index) { return Ok{}; }

  DataStringT makeDataString() { return Ok{}; }
  void appendDataString(DataStringT&, std::string_view) {}

  MemTypeT makeMemType(Type, LimitsT, bool) { return Ok{}; }

  BlockTypeT getBlockTypeFromResult(size_t results) { return Ok{}; }

  Result<> getBlockTypeFromTypeUse(Index, TypeUseT) { return Ok{}; }

  bool skipFunctionBody() { return false; }
};

template<typename Ctx> struct TypeParserCtx {
  using IndexT = Index;
  using HeapTypeT = HeapType;
  using TypeT = Type;
  using ParamsT = std::vector<NameType>;
  using ResultsT = std::vector<Type>;
  using BlockTypeT = HeapType;
  using SignatureT = Signature;
  using ContinuationT = Continuation;
  using StorageT = Field;
  using FieldT = Field;
  using FieldsT = std::pair<std::vector<Name>, std::vector<Field>>;
  using StructT = std::pair<std::vector<Name>, Struct>;
  using ArrayT = Array;
  using LimitsT = Ok;
  using MemTypeT = Ok;
  using LocalsT = std::vector<NameType>;
  using DataStringT = Ok;

  // Map heap type names to their indices.
  const IndexMap& typeIndices;

  TypeParserCtx(const IndexMap& typeIndices) : typeIndices(typeIndices) {}

  Ctx& self() { return *static_cast<Ctx*>(this); }

  HeapTypeT makeFuncType(Shareability share) {
    return HeapTypes::func.getBasic(share);
  }
  HeapTypeT makeAnyType(Shareability share) {
    return HeapTypes::any.getBasic(share);
  }
  HeapTypeT makeExternType(Shareability share) {
    return HeapTypes::ext.getBasic(share);
  }
  HeapTypeT makeEqType(Shareability share) {
    return HeapTypes::eq.getBasic(share);
  }
  HeapTypeT makeI31Type(Shareability share) {
    return HeapTypes::i31.getBasic(share);
  }
  HeapTypeT makeStructType(Shareability share) {
    return HeapTypes::struct_.getBasic(share);
  }
  HeapTypeT makeArrayType(Shareability share) {
    return HeapTypes::array.getBasic(share);
  }
  HeapTypeT makeExnType(Shareability share) {
    return HeapTypes::exn.getBasic(share);
  }
  HeapTypeT makeStringType(Shareability share) {
    return HeapTypes::string.getBasic(share);
  }
  HeapTypeT makeContType(Shareability share) {
    return HeapTypes::cont.getBasic(share);
  }
  HeapTypeT makeNoneType(Shareability share) {
    return HeapTypes::none.getBasic(share);
  }
  HeapTypeT makeNoextType(Shareability share) {
    return HeapTypes::noext.getBasic(share);
  }
  HeapTypeT makeNofuncType(Shareability share) {
    return HeapTypes::nofunc.getBasic(share);
  }
  HeapTypeT makeNoexnType(Shareability share) {
    return HeapTypes::noexn.getBasic(share);
  }
  HeapTypeT makeNocontType(Shareability share) {
    return HeapTypes::nocont.getBasic(share);
  }

  TypeT makeI32() { return Type::i32; }
  TypeT makeI64() { return Type::i64; }
  TypeT makeF32() { return Type::f32; }
  TypeT makeF64() { return Type::f64; }
  TypeT makeV128() { return Type::v128; }

  TypeT makeRefType(HeapTypeT ht, Nullability nullability) {
    return Type(ht, nullability);
  }

  std::vector<Type> makeTupleElemList() { return {}; }
  void appendTupleElem(std::vector<Type>& elems, Type elem) {
    elems.push_back(elem);
  }
  Result<TypeT> makeTupleType(const std::vector<Type>& types) {
    return Tuple(types);
  }

  ParamsT makeParams() { return {}; }
  void appendParam(ParamsT& params, Name id, TypeT type) {
    params.push_back({id, type});
  }

  ResultsT makeResults() { return {}; }
  void appendResult(ResultsT& results, TypeT type) { results.push_back(type); }
  size_t getResultsSize(const ResultsT& results) { return results.size(); }

  SignatureT makeFuncType(ParamsT* params, ResultsT* results) {
    std::vector<Type> empty;
    const auto& paramTypes = params ? getUnnamedTypes(*params) : empty;
    const auto& resultTypes = results ? *results : empty;
    return Signature(self().makeTupleType(paramTypes),
                     self().makeTupleType(resultTypes));
  }

  ContinuationT makeContType(HeapTypeT ft) { return Continuation(ft); }

  StorageT makeI8() { return Field(Field::i8, Immutable); }
  StorageT makeI16() { return Field(Field::i16, Immutable); }
  StorageT makeStorageType(TypeT type) { return Field(type, Immutable); }

  FieldT makeFieldType(FieldT field, Mutability mutability) {
    if (field.packedType == Field::not_packed) {
      return Field(field.type, mutability);
    }
    return Field(field.packedType, mutability);
  }

  FieldsT makeFields() { return {}; }
  void appendField(FieldsT& fields, Name name, FieldT field) {
    fields.first.push_back(name);
    fields.second.push_back(field);
  }

  StructT makeStruct(FieldsT& fields) {
    return {std::move(fields.first), Struct(std::move(fields.second))};
  }

  std::optional<ArrayT> makeArray(FieldsT& fields) {
    if (fields.second.size() == 1) {
      return Array(fields.second[0]);
    }
    return {};
  }

  LocalsT makeLocals() { return {}; }
  void appendLocal(LocalsT& locals, Name id, TypeT type) {
    locals.push_back({id, type});
  }

  Result<Index> getTypeIndex(Name id) {
    auto it = typeIndices.find(id);
    if (it == typeIndices.end()) {
      return self().in.err("unknown type identifier");
    }
    return it->second;
  }

  DataStringT makeDataString() { return Ok{}; }
  void appendDataString(DataStringT&, std::string_view) {}

  Result<LimitsT> makeLimits(uint64_t, std::optional<uint64_t>) { return Ok{}; }
  LimitsT getLimitsFromData(DataStringT) { return Ok{}; }

  MemTypeT makeMemType(Type, LimitsT, bool) { return Ok{}; }

  HeapType getBlockTypeFromResult(const std::vector<Type> results) {
    assert(results.size() == 1);
    return HeapType(Signature(Type::none, results[0]));
  }

  bool skipFunctionBody() { return false; }
};

struct NullInstrParserCtx {
  using ExprT = Ok;
  using CatchT = Ok;
  using CatchListT = Ok;
  using OnClauseT = Ok;
  using OnClauseListT = Ok;
  using TagLabelListT = Ok;

  using FieldIdxT = Ok;
  using FuncIdxT = Ok;
  using LocalIdxT = Ok;
  using TableIdxT = Ok;
  using MemoryIdxT = Ok;
  using GlobalIdxT = Ok;
  using ElemIdxT = Ok;
  using DataIdxT = Ok;
  using LabelIdxT = Ok;
  using TagIdxT = Ok;

  using MemargT = Ok;

  Result<> makeExpr() { return Ok{}; }

  template<typename HeapTypeT> FieldIdxT getFieldFromIdx(HeapTypeT, uint32_t) {
    return Ok{};
  }
  template<typename HeapTypeT> FieldIdxT getFieldFromName(HeapTypeT, Name) {
    return Ok{};
  }
  FuncIdxT getFuncFromIdx(uint32_t) { return Ok{}; }
  FuncIdxT getFuncFromName(Name) { return Ok{}; }
  LocalIdxT getLocalFromIdx(uint32_t) { return Ok{}; }
  LocalIdxT getLocalFromName(Name) { return Ok{}; }
  GlobalIdxT getGlobalFromIdx(uint32_t) { return Ok{}; }
  GlobalIdxT getGlobalFromName(Name) { return Ok{}; }
  TableIdxT getTableFromIdx(uint32_t) { return Ok{}; }
  TableIdxT getTableFromName(Name) { return Ok{}; }
  MemoryIdxT getMemoryFromIdx(uint32_t) { return Ok{}; }
  MemoryIdxT getMemoryFromName(Name) { return Ok{}; }
  ElemIdxT getElemFromIdx(uint32_t) { return Ok{}; }
  ElemIdxT getElemFromName(Name) { return Ok{}; }
  DataIdxT getDataFromIdx(uint32_t) { return Ok{}; }
  DataIdxT getDataFromName(Name) { return Ok{}; }
  LabelIdxT getLabelFromIdx(uint32_t, bool) { return Ok{}; }
  LabelIdxT getLabelFromName(Name, bool) { return Ok{}; }
  TagIdxT getTagFromIdx(uint32_t) { return Ok{}; }
  TagIdxT getTagFromName(Name) { return Ok{}; }

  MemargT getMemarg(uint64_t, uint32_t) { return Ok{}; }

  template<typename BlockTypeT>
  Result<> makeBlock(Index,
                     const std::vector<Annotation>&,
                     std::optional<Name>,
                     BlockTypeT) {
    return Ok{};
  }
  template<typename BlockTypeT>
  Result<> makeIf(Index,
                  const std::vector<Annotation>&,
                  std::optional<Name>,
                  BlockTypeT) {
    return Ok{};
  }
  Result<> visitElse() { return Ok{}; }
  template<typename BlockTypeT>
  Result<> makeLoop(Index,
                    const std::vector<Annotation>&,
                    std::optional<Name>,
                    BlockTypeT) {
    return Ok{};
  }
  template<typename BlockTypeT>
  Result<> makeTry(Index,
                   const std::vector<Annotation>&,
                   std::optional<Name>,
                   BlockTypeT) {
    return Ok{};
  }
  Result<> visitCatch(Index, TagIdxT) { return Ok{}; }
  Result<> visitCatchAll(Index) { return Ok{}; }
  Result<> visitDelegate(Index, LabelIdxT) { return Ok{}; }
  Result<> visitEnd() { return Ok{}; }

  CatchListT makeCatchList() { return Ok{}; }
  void appendCatch(CatchListT&, CatchT) {}
  CatchT makeCatch(TagIdxT, LabelIdxT) { return Ok{}; }
  CatchT makeCatchRef(TagIdxT, LabelIdxT) { return Ok{}; }
  CatchT makeCatchAll(LabelIdxT) { return Ok{}; }
  CatchT makeCatchAllRef(LabelIdxT) { return Ok{}; }
  template<typename BlockTypeT>
  Result<> makeTryTable(Index,
                        const std::vector<Annotation>&,
                        std::optional<Name>,
                        BlockTypeT,
                        CatchListT) {
    return Ok{};
  }

  OnClauseListT makeOnClauseList() { return Ok{}; }
  void appendOnClause(OnClauseListT&, OnClauseT) {}
  OnClauseT makeOnLabel(TagIdxT, LabelIdxT) { return Ok{}; }
  OnClauseT makeOnSwitch(TagIdxT) { return Ok{}; }

  void setSrcLoc(const std::vector<Annotation>&) {}

  Result<> makeUnreachable(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  Result<> makeNop(Index, const std::vector<Annotation>&) { return Ok{}; }
  Result<> makeBinary(Index, const std::vector<Annotation>&, BinaryOp) {
    return Ok{};
  }
  Result<> makeUnary(Index, const std::vector<Annotation>&, UnaryOp) {
    return Ok{};
  }
  template<typename ResultsT>
  Result<> makeSelect(Index, const std::vector<Annotation>&, ResultsT*) {
    return Ok{};
  }
  Result<> makeDrop(Index, const std::vector<Annotation>&) { return Ok{}; }
  Result<> makeMemorySize(Index, const std::vector<Annotation>&, MemoryIdxT*) {
    return Ok{};
  }
  Result<> makeMemoryGrow(Index, const std::vector<Annotation>&, MemoryIdxT*) {
    return Ok{};
  }
  Result<> makeLocalGet(Index, const std::vector<Annotation>&, LocalIdxT) {
    return Ok{};
  }
  Result<> makeLocalTee(Index, const std::vector<Annotation>&, LocalIdxT) {
    return Ok{};
  }
  Result<> makeLocalSet(Index, const std::vector<Annotation>&, LocalIdxT) {
    return Ok{};
  }
  Result<> makeGlobalGet(Index, const std::vector<Annotation>&, GlobalIdxT) {
    return Ok{};
  }
  Result<> makeGlobalSet(Index, const std::vector<Annotation>&, GlobalIdxT) {
    return Ok{};
  }

  Result<> makeI32Const(Index, const std::vector<Annotation>&, uint32_t) {
    return Ok{};
  }
  Result<> makeI64Const(Index, const std::vector<Annotation>&, uint64_t) {
    return Ok{};
  }
  Result<> makeF32Const(Index, const std::vector<Annotation>&, float) {
    return Ok{};
  }
  Result<> makeF64Const(Index, const std::vector<Annotation>&, double) {
    return Ok{};
  }
  Result<> makeI8x16Const(Index,
                          const std::vector<Annotation>&,
                          const std::array<uint8_t, 16>&) {
    return Ok{};
  }
  Result<> makeI16x8Const(Index,
                          const std::vector<Annotation>&,
                          const std::array<uint16_t, 8>&) {
    return Ok{};
  }
  Result<> makeI32x4Const(Index,
                          const std::vector<Annotation>&,
                          const std::array<uint32_t, 4>&) {
    return Ok{};
  }
  Result<> makeI64x2Const(Index,
                          const std::vector<Annotation>&,
                          const std::array<uint64_t, 2>&) {
    return Ok{};
  }
  Result<> makeF32x4Const(Index,
                          const std::vector<Annotation>&,
                          const std::array<float, 4>&) {
    return Ok{};
  }
  Result<> makeF64x2Const(Index,
                          const std::vector<Annotation>&,
                          const std::array<double, 2>&) {
    return Ok{};
  }
  Result<> makeLoad(Index,
                    const std::vector<Annotation>&,
                    Type,
                    bool,
                    int,
                    bool,
                    MemoryIdxT*,
                    MemargT) {
    return Ok{};
  }
  Result<> makeStore(Index,
                     const std::vector<Annotation>&,
                     Type,
                     int,
                     bool,
                     MemoryIdxT*,
                     MemargT) {
    return Ok{};
  }
  Result<> makeAtomicRMW(Index,
                         const std::vector<Annotation>&,
                         AtomicRMWOp,
                         Type,
                         int,
                         MemoryIdxT*,
                         MemargT) {
    return Ok{};
  }
  Result<> makeAtomicCmpxchg(
    Index, const std::vector<Annotation>&, Type, int, MemoryIdxT*, MemargT) {
    return Ok{};
  }
  Result<> makeAtomicWait(
    Index, const std::vector<Annotation>&, Type, MemoryIdxT*, MemargT) {
    return Ok{};
  }
  Result<> makeAtomicNotify(Index,
                            const std::vector<Annotation>&,
                            MemoryIdxT*,
                            MemargT) {
    return Ok{};
  }
  Result<> makeAtomicFence(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  Result<> makeSIMDExtract(Index,
                           const std::vector<Annotation>&,
                           SIMDExtractOp,
                           uint8_t) {
    return Ok{};
  }
  Result<> makeSIMDReplace(Index,
                           const std::vector<Annotation>&,
                           SIMDReplaceOp,
                           uint8_t) {
    return Ok{};
  }
  Result<> makeSIMDShuffle(Index,
                           const std::vector<Annotation>&,
                           const std::array<uint8_t, 16>&) {
    return Ok{};
  }
  Result<>
  makeSIMDTernary(Index, const std::vector<Annotation>&, SIMDTernaryOp) {
    return Ok{};
  }
  Result<> makeSIMDShift(Index, const std::vector<Annotation>&, SIMDShiftOp) {
    return Ok{};
  }
  Result<> makeSIMDLoad(
    Index, const std::vector<Annotation>&, SIMDLoadOp, MemoryIdxT*, MemargT) {
    return Ok{};
  }
  Result<> makeSIMDLoadStoreLane(Index,
                                 const std::vector<Annotation>&,
                                 SIMDLoadStoreLaneOp,
                                 MemoryIdxT*,
                                 MemargT,
                                 uint8_t) {
    return Ok{};
  }
  Result<>
  makeMemoryInit(Index, const std::vector<Annotation>&, MemoryIdxT*, DataIdxT) {
    return Ok{};
  }
  Result<> makeDataDrop(Index, const std::vector<Annotation>&, DataIdxT) {
    return Ok{};
  }

  Result<> makeMemoryCopy(Index,
                          const std::vector<Annotation>&,
                          MemoryIdxT*,
                          MemoryIdxT*) {
    return Ok{};
  }
  Result<> makeMemoryFill(Index, const std::vector<Annotation>&, MemoryIdxT*) {
    return Ok{};
  }
  template<typename TypeT>
  Result<> makePop(Index, const std::vector<Annotation>&, TypeT) {
    return Ok{};
  }
  Result<> makeCall(Index, const std::vector<Annotation>&, FuncIdxT, bool) {
    return Ok{};
  }
  template<typename TypeUseT>
  Result<> makeCallIndirect(
    Index, const std::vector<Annotation>&, TableIdxT*, TypeUseT, bool) {
    return Ok{};
  }
  Result<> makeBreak(Index, const std::vector<Annotation>&, LabelIdxT, bool) {
    return Ok{};
  }
  Result<> makeSwitch(Index,
                      const std::vector<Annotation>&,
                      const std::vector<LabelIdxT>&,
                      LabelIdxT) {
    return Ok{};
  }
  Result<> makeReturn(Index, const std::vector<Annotation>&) { return Ok{}; }
  template<typename HeapTypeT>
  Result<> makeRefNull(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  Result<> makeRefIsNull(Index, const std::vector<Annotation>&) { return Ok{}; }
  Result<> makeRefFunc(Index, const std::vector<Annotation>&, FuncIdxT) {
    return Ok{};
  }
  Result<> makeRefEq(Index, const std::vector<Annotation>&) { return Ok{}; }
  Result<> makeTableGet(Index, const std::vector<Annotation>&, TableIdxT*) {
    return Ok{};
  }
  Result<> makeTableSet(Index, const std::vector<Annotation>&, TableIdxT*) {
    return Ok{};
  }
  Result<> makeTableSize(Index, const std::vector<Annotation>&, TableIdxT*) {
    return Ok{};
  }
  Result<> makeTableGrow(Index, const std::vector<Annotation>&, TableIdxT*) {
    return Ok{};
  }
  Result<> makeTableFill(Index, const std::vector<Annotation>&, TableIdxT*) {
    return Ok{};
  }
  Result<>
  makeTableCopy(Index, const std::vector<Annotation>&, TableIdxT*, TableIdxT*) {
    return Ok{};
  }
  Result<>
  makeTableInit(Index, const std::vector<Annotation>&, TableIdxT*, ElemIdxT) {
    return Ok{};
  }
  Result<> makeThrow(Index, const std::vector<Annotation>&, TagIdxT) {
    return Ok{};
  }
  Result<> makeRethrow(Index, const std::vector<Annotation>&, LabelIdxT) {
    return Ok{};
  }
  Result<> makeThrowRef(Index, const std::vector<Annotation>&) { return Ok{}; }
  Result<> makeTupleMake(Index, const std::vector<Annotation>&, uint32_t) {
    return Ok{};
  }
  Result<>
  makeTupleExtract(Index, const std::vector<Annotation>&, uint32_t, uint32_t) {
    return Ok{};
  }
  Result<> makeTupleDrop(Index, const std::vector<Annotation>&, uint32_t) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeCallRef(Index, const std::vector<Annotation>&, HeapTypeT, bool) {
    return Ok{};
  }
  Result<>
  makeRefI31(Index, const std::vector<Annotation>&, Shareability share) {
    return Ok{};
  }
  Result<> makeI31Get(Index, const std::vector<Annotation>&, bool) {
    return Ok{};
  }
  template<typename TypeT>
  Result<> makeRefTest(Index, const std::vector<Annotation>&, TypeT) {
    return Ok{};
  }
  template<typename TypeT>
  Result<> makeRefCast(Index, const std::vector<Annotation>&, TypeT) {
    return Ok{};
  }

  Result<> makeBrOn(Index, const std::vector<Annotation>&, LabelIdxT, BrOnOp) {
    return Ok{};
  }

  template<typename TypeT>
  Result<> makeBrOn(
    Index, const std::vector<Annotation>&, LabelIdxT, BrOnOp, TypeT, TypeT) {
    return Ok{};
  }

  template<typename HeapTypeT>
  Result<> makeStructNew(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeStructNewDefault(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeStructGet(
    Index, const std::vector<Annotation>&, HeapTypeT, FieldIdxT, bool) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeStructSet(Index, const std::vector<Annotation>&, HeapTypeT, FieldIdxT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeArrayNew(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeArrayNewDefault(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeArrayNewData(Index, const std::vector<Annotation>&, HeapTypeT, DataIdxT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeArrayNewElem(Index, const std::vector<Annotation>&, HeapTypeT, ElemIdxT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeArrayNewFixed(Index,
                             const std::vector<Annotation>&,
                             HeapTypeT,
                             uint32_t) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeArrayGet(Index, const std::vector<Annotation>&, HeapTypeT, bool) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeArraySet(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  Result<> makeArrayLen(Index, const std::vector<Annotation>&) { return Ok{}; }
  template<typename HeapTypeT>
  Result<>
  makeArrayCopy(Index, const std::vector<Annotation>&, HeapTypeT, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeArrayFill(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeArrayInitData(Index,
                             const std::vector<Annotation>&,
                             HeapTypeT,
                             DataIdxT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeArrayInitElem(Index,
                             const std::vector<Annotation>&,
                             HeapTypeT,
                             ElemIdxT) {
    return Ok{};
  }
  Result<> makeRefAs(Index, const std::vector<Annotation>&, RefAsOp) {
    return Ok{};
  }
  Result<> makeStringNew(Index, const std::vector<Annotation>&, StringNewOp) {
    return Ok{};
  }
  Result<>
  makeStringConst(Index, const std::vector<Annotation>&, std::string_view) {
    return Ok{};
  }
  Result<>
  makeStringMeasure(Index, const std::vector<Annotation>&, StringMeasureOp) {
    return Ok{};
  }
  Result<>
  makeStringEncode(Index, const std::vector<Annotation>&, StringEncodeOp) {
    return Ok{};
  }
  Result<> makeStringConcat(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  Result<> makeStringEq(Index, const std::vector<Annotation>&, StringEqOp) {
    return Ok{};
  }
  Result<> makeStringWTF8Advance(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  Result<> makeStringWTF16Get(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  Result<> makeStringIterNext(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  Result<> makeStringSliceWTF(Index, const std::vector<Annotation>&) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeContNew(Index, const std::vector<Annotation>&, HeapTypeT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeContBind(Index, const std::vector<Annotation>&, HeapTypeT, HeapTypeT) {
    return Ok{};
  }
  Result<> makeSuspend(Index, const std::vector<Annotation>&, TagIdxT) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeResume(Index,
                      const std::vector<Annotation>&,
                      HeapTypeT,
                      const TagLabelListT&) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<> makeResumeThrow(Index,
                           const std::vector<Annotation>&,
                           HeapTypeT,
                           TagIdxT,
                           const TagLabelListT&) {
    return Ok{};
  }
  template<typename HeapTypeT>
  Result<>
  makeStackSwitch(Index, const std::vector<Annotation>&, HeapTypeT, TagIdxT) {
    return Ok{};
  }
};

struct NullCtx : NullTypeParserCtx, NullInstrParserCtx {
  Lexer in;
  NullCtx(const Lexer& in) : in(in) {}
  Result<> makeTypeUse(Index, std::optional<HeapTypeT>, ParamsT*, ResultsT*) {
    return Ok{};
  }
};

// Phase 1: Parse definition spans for top-level module elements and determine
// their indices and names.
struct ParseDeclsCtx : NullTypeParserCtx, NullInstrParserCtx {
  using ExprT = Ok;
  using LimitsT = Limits;
  using ElemListT = Index;
  using DataStringT = std::vector<char>;
  using TableTypeT = TableType;
  using MemTypeT = MemType;

  Lexer in;

  // At this stage we only look at types to find implicit type definitions,
  // which are inserted directly into the context. We cannot materialize or
  // validate any types because we don't know what types exist yet.
  //
  // Declared module elements are inserted into the module, but their bodies are
  // not filled out until later parsing phases.
  Module& wasm;

  // The module element definitions we are parsing in this phase.
  std::vector<DefPos> recTypeDefs;
  std::vector<DefPos> typeDefs;
  std::vector<DefPos> funcDefs;
  std::vector<DefPos> tableDefs;
  std::vector<DefPos> memoryDefs;
  std::vector<DefPos> globalDefs;
  std::vector<DefPos> startDefs;
  std::vector<DefPos> elemDefs;
  std::vector<DefPos> dataDefs;
  std::vector<DefPos> tagDefs;

  // Positions of export definitions.
  std::vector<Index> exportDefs;

  // Positions of typeuses that might implicitly define new types.
  std::vector<Index> implicitTypeDefs;

  // Map table indices to the indices of their implicit, in-line element
  // segments. We need these to find associated segments in later parsing phases
  // where we can parse their types and instructions.
  std::unordered_map<Index, Index> implicitElemIndices;

  // Counters used for generating names for module elements.
  int funcCounter = 0;
  int tableCounter = 0;
  int memoryCounter = 0;
  int globalCounter = 0;
  int elemCounter = 0;
  int dataCounter = 0;
  int tagCounter = 0;

  // Used to verify that all imports come before all non-imports.
  bool hasNonImport = false;

  Result<> checkImport(Index pos, ImportNames* import) {
    if (import) {
      if (hasNonImport) {
        return in.err(pos, "import after non-import");
      }
    } else {
      hasNonImport = true;
    }
    return Ok{};
  }

  ParseDeclsCtx(Lexer& in, Module& wasm) : in(in), wasm(wasm) {}

  void addFuncType(SignatureT) {}
  void addContType(ContinuationT) {}
  void addStructType(StructT) {}
  void addArrayType(ArrayT) {}
  void setOpen() {}
  void setShared() {}
  Result<> addSubtype(HeapTypeT) { return Ok{}; }
  void finishTypeDef(Name name, Index pos) {
    // TODO: type annotations
    typeDefs.push_back({name, pos, Index(typeDefs.size()), {}});
  }
  size_t getRecGroupStartIndex() { return 0; }
  void addRecGroup(Index, size_t) {}
  void finishRectype(Index pos) {
    // TODO: type annotations
    recTypeDefs.push_back({{}, pos, Index(recTypeDefs.size()), {}});
  }

  Limits makeLimits(uint64_t n, std::optional<uint64_t> m) {
    return Limits{n, m};
  }

  Index makeElemList(TypeT) { return 0; }
  Index makeFuncElemList() { return 0; }
  void appendElem(Index& elems, ExprT) { ++elems; }
  void appendFuncElem(Index& elems, FuncIdxT) { ++elems; }

  Limits getLimitsFromElems(Index elems) { return {elems, elems}; }

  TableType makeTableType(Type addressType, Limits limits, TypeT) {
    return {addressType, limits};
  }

  std::vector<char> makeDataString() { return {}; }
  void appendDataString(std::vector<char>& data, std::string_view str) {
    data.insert(data.end(), str.begin(), str.end());
  }

  Limits getLimitsFromData(const std::vector<char>& data) {
    uint64_t size = (data.size() + Memory::kPageSize - 1) / Memory::kPageSize;
    return {size, size};
  }

  MemType makeMemType(Type addressType, Limits limits, bool shared) {
    return {addressType, limits, shared};
  }

  Result<TypeUseT>
  makeTypeUse(Index pos, std::optional<HeapTypeT> type, ParamsT*, ResultsT*) {
    if (!type) {
      implicitTypeDefs.push_back(pos);
    }
    return Ok{};
  }

  Result<Function*> addFuncDecl(Index pos, Name name, ImportNames* importNames);
  Result<> addFunc(Name name,
                   const std::vector<Name>& exports,
                   ImportNames* import,
                   TypeUseT type,
                   std::optional<LocalsT>,
                   std::vector<Annotation>&&,
                   Index pos);

  Result<Table*> addTableDecl(Index pos,
                              Name name,
                              ImportNames* importNames,
                              TableType limits);
  Result<>
  addTable(Name, const std::vector<Name>&, ImportNames*, TableType, Index);

  // TODO: Record index of implicit elem for use when parsing types and instrs.
  Result<> addImplicitElems(TypeT, ElemListT&& elems);

  Result<Memory*>
  addMemoryDecl(Index pos, Name name, ImportNames* importNames, MemType type);

  Result<> addMemory(Name name,
                     const std::vector<Name>& exports,
                     ImportNames* import,
                     MemType type,
                     Index pos);

  Result<> addImplicitData(DataStringT&& data);

  Result<Global*> addGlobalDecl(Index pos, Name name, ImportNames* importNames);

  Result<> addGlobal(Name name,
                     const std::vector<Name>& exports,
                     ImportNames* import,
                     GlobalTypeT,
                     std::optional<ExprT>,
                     Index pos);

  Result<> addStart(FuncIdxT, Index pos) {
    if (!startDefs.empty()) {
      return Err{"unexpected extra 'start' function"};
    }
    // TODO: start function annotations.
    startDefs.push_back({{}, pos, 0, {}});
    return Ok{};
  }

  Result<> addElem(Name, TableIdxT*, std::optional<ExprT>, ElemListT&&, Index);

  Result<> addDeclareElem(Name, ElemListT&&, Index) { return Ok{}; }

  Result<> addData(Name name,
                   MemoryIdxT*,
                   std::optional<ExprT>,
                   std::vector<char>&& data,
                   Index pos);

  Result<Tag*> addTagDecl(Index pos, Name name, ImportNames* importNames);

  Result<> addTag(Name name,
                  const std::vector<Name>& exports,
                  ImportNames* import,
                  TypeUseT type,
                  Index pos);

  Result<> addExport(Index pos, Ok, Name, ExternalKind) {
    exportDefs.push_back(pos);
    return Ok{};
  }
};

// Phase 2: Parse type definitions into a TypeBuilder.
struct ParseTypeDefsCtx : TypeParserCtx<ParseTypeDefsCtx> {
  Lexer in;

  // We update slots in this builder as we parse type definitions.
  TypeBuilder& builder;

  // Parse the names of types and fields as we go.
  std::vector<TypeNames> names;

  // The index of the subtype definition we are parsing.
  Index index = 0;

  ParseTypeDefsCtx(Lexer& in, TypeBuilder& builder, const IndexMap& typeIndices)
    : TypeParserCtx<ParseTypeDefsCtx>(typeIndices), in(in), builder(builder),
      names(builder.size()) {}

  TypeT makeRefType(HeapTypeT ht, Nullability nullability) {
    return builder.getTempRefType(ht, nullability);
  }

  TypeT makeTupleType(const std::vector<Type> types) {
    return builder.getTempTupleType(types);
  }

  Result<HeapTypeT> getHeapTypeFromIdx(Index idx) {
    if (idx >= builder.size()) {
      return in.err("type index out of bounds");
    }
    return builder[idx];
  }

  void addFuncType(SignatureT& type) { builder[index] = type; }
  void addContType(ContinuationT& type) { builder[index] = type; }

  void addStructType(StructT& type) {
    auto& [fieldNames, str] = type;
    builder[index] = str;
    for (Index i = 0; i < fieldNames.size(); ++i) {
      if (auto name = fieldNames[i]; name.is()) {
        names[index].fieldNames[i] = name;
      }
    }
  }

  void addArrayType(ArrayT& type) { builder[index] = type; }

  void setOpen() { builder[index].setOpen(); }

  void setShared() { builder[index].setShared(); }

  Result<> addSubtype(HeapTypeT super) {
    builder[index].subTypeOf(super);
    return Ok{};
  }

  void finishTypeDef(Name name, Index pos) { names[index++].name = name; }

  size_t getRecGroupStartIndex() { return index; }

  void addRecGroup(Index start, size_t len) {
    builder.createRecGroup(start, len);
  }

  void finishRectype(Index) {}
};

// Phase 3: Parse type uses to find implicitly defined types.
struct ParseImplicitTypeDefsCtx : TypeParserCtx<ParseImplicitTypeDefsCtx> {
  using TypeUseT = Ok;

  Lexer in;

  // Types parsed so far.
  std::vector<HeapType>& types;

  // Map typeuse positions without an explicit type to the correct type.
  std::unordered_map<Index, HeapType>& implicitTypes;

  // Map signatures to the first defined heap type they match.
  std::unordered_map<Signature, HeapType> sigTypes;

  ParseImplicitTypeDefsCtx(Lexer& in,
                           std::vector<HeapType>& types,
                           std::unordered_map<Index, HeapType>& implicitTypes,
                           const IndexMap& typeIndices)
    : TypeParserCtx<ParseImplicitTypeDefsCtx>(typeIndices), in(in),
      types(types), implicitTypes(implicitTypes) {
    for (auto type : types) {
      if (type.isSignature() && type.getRecGroup().size() == 1 &&
          !type.getDeclaredSuperType() && !type.isOpen() && !type.isShared()) {
        sigTypes.insert({type.getSignature(), type});
      }
    }
  }

  Result<HeapTypeT> getHeapTypeFromIdx(Index idx) {
    if (idx >= types.size()) {
      return in.err("type index out of bounds");
    }
    return types[idx];
  }

  Result<TypeUseT> makeTypeUse(Index pos,
                               std::optional<HeapTypeT>,
                               ParamsT* params,
                               ResultsT* results) {
    std::vector<Type> paramTypes;
    if (params) {
      paramTypes = getUnnamedTypes(*params);
    }

    std::vector<Type> resultTypes;
    if (results) {
      resultTypes = *results;
    }

    auto sig = Signature(Type(paramTypes), Type(resultTypes));
    auto [it, inserted] = sigTypes.insert({sig, HeapType::func});
    if (inserted) {
      auto type = HeapType(sig);
      it->second = type;
      types.push_back(type);
    }
    implicitTypes.insert({pos, it->second});

    return Ok{};
  }
};

// Phase 4: Parse and set the types of module elements.
struct ParseModuleTypesCtx : TypeParserCtx<ParseModuleTypesCtx>,
                             NullInstrParserCtx {
  // In this phase we have constructed all the types, so we can materialize and
  // validate them when they are used.

  using GlobalTypeT = GlobalType;
  using TableTypeT = Type;
  using TypeUseT = TypeUse;

  using ElemListT = Type;

  Lexer in;

  Module& wasm;

  const std::vector<HeapType>& types;
  const std::unordered_map<Index, HeapType>& implicitTypes;
  const std::unordered_map<Index, Index>& implicitElemIndices;

  // The index of the current type.
  Index index = 0;

  ParseModuleTypesCtx(
    Lexer& in,
    Module& wasm,
    const std::vector<HeapType>& types,
    const std::unordered_map<Index, HeapType>& implicitTypes,
    const std::unordered_map<Index, Index>& implicitElemIndices,
    const IndexMap& typeIndices)
    : TypeParserCtx<ParseModuleTypesCtx>(typeIndices), in(in), wasm(wasm),
      types(types), implicitTypes(implicitTypes),
      implicitElemIndices(implicitElemIndices) {}

  bool skipFunctionBody() { return true; }

  Result<HeapTypeT> getHeapTypeFromIdx(Index idx) {
    if (idx >= types.size()) {
      return in.err("type index out of bounds");
    }
    return types[idx];
  }

  Result<TypeUseT> makeTypeUse(Index pos,
                               std::optional<HeapTypeT> type,
                               ParamsT* params,
                               ResultsT* results) {
    std::vector<Name> ids;
    if (params) {
      ids.reserve(params->size());
      for (auto& p : *params) {
        ids.push_back(p.name);
      }
    }

    if (type) {
      return TypeUse{*type, ids};
    }

    auto it = implicitTypes.find(pos);
    assert(it != implicitTypes.end());

    return TypeUse{it->second, ids};
  }

  Result<HeapType> getBlockTypeFromTypeUse(Index pos, TypeUse use) {
    return use.type;
  }

  GlobalTypeT makeGlobalType(Mutability mutability, TypeT type) {
    return {mutability, type};
  }

  Type makeElemList(Type type) { return type; }
  Type makeFuncElemList() { return Type(HeapType::func, Nullable); }
  void appendElem(ElemListT&, ExprT) {}
  void appendFuncElem(ElemListT&, FuncIdxT) {}

  LimitsT getLimitsFromElems(ElemListT) { return Ok{}; }

  Type makeTableType(Type addressType, LimitsT, Type type) { return type; }

  LimitsT getLimitsFromData(DataStringT) { return Ok{}; }
  MemTypeT makeMemType(Type, LimitsT, bool) { return Ok{}; }

  Result<> addFunc(Name name,
                   const std::vector<Name>&,
                   ImportNames*,
                   TypeUse type,
                   std::optional<LocalsT> locals,
                   std::vector<Annotation>&&,
                   Index pos) {
    auto& f = wasm.functions[index];
    if (!type.type.isSignature()) {
      return in.err(pos, "expected signature type");
    }
    f->type = type.type;
    for (Index i = 0; i < type.names.size(); ++i) {
      if (type.names[i].is()) {
        f->setLocalName(i, type.names[i]);
      }
    }
    if (locals) {
      for (auto& l : *locals) {
        Builder::addVar(f.get(), l.name, l.type);
      }
    }
    return Ok{};
  }

  Result<> addTable(
    Name, const std::vector<Name>&, ImportNames*, Type ttype, Index pos) {
    auto& t = wasm.tables[index];
    if (!ttype.isRef()) {
      return in.err(pos, "expected reference type");
    }
    t->type = ttype;
    return Ok{};
  }

  Result<> addImplicitElems(Type type, ElemListT&&) {
    auto& t = wasm.tables[index];
    auto& e = wasm.elementSegments[implicitElemIndices.at(index)];
    e->type = t->type;
    return Ok{};
  }

  Result<>
  addMemory(Name, const std::vector<Name>&, ImportNames*, MemTypeT, Index) {
    return Ok{};
  }

  Result<> addImplicitData(DataStringT&& data) { return Ok{}; }

  Result<> addGlobal(Name,
                     const std::vector<Name>&,
                     ImportNames*,
                     GlobalType type,
                     std::optional<ExprT>,
                     Index) {
    auto& g = wasm.globals[index];
    g->mutable_ = type.mutability;
    g->type = type.type;
    return Ok{};
  }

  Result<>
  addElem(Name, TableIdxT*, std::optional<ExprT>, ElemListT&& type, Index) {
    auto& e = wasm.elementSegments[index];
    e->type = type;
    return Ok{};
  }

  Result<> addDeclareElem(Name, ElemListT&&, Index) { return Ok{}; }

  Result<>
  addTag(Name, const std::vector<Name>&, ImportNames*, TypeUse use, Index pos) {
    auto& t = wasm.tags[index];
    if (!use.type.isSignature()) {
      return in.err(pos, "tag type must be a signature");
    }
    t->sig = use.type.getSignature();
    return Ok{};
  }
};

// Phase 5: Parse module element definitions, including instructions.
struct ParseDefsCtx : TypeParserCtx<ParseDefsCtx> {
  using GlobalTypeT = Ok;
  using TableTypeT = Ok;
  using TypeUseT = HeapType;

  using FieldIdxT = Index;
  using FuncIdxT = Name;
  using LocalIdxT = Index;
  using LabelIdxT = Index;
  using GlobalIdxT = Name;
  using TableIdxT = Name;
  using MemoryIdxT = Name;
  using ElemIdxT = Name;
  using DataIdxT = Name;
  using TagIdxT = Name;

  using MemargT = Memarg;

  using ExprT = Expression*;
  using ElemListT = std::vector<Expression*>;

  struct CatchInfo;
  using CatchT = CatchInfo;
  using CatchListT = std::vector<CatchInfo>;

  using TagLabelListT = std::vector<std::pair<TagIdxT, LabelIdxT>>;

  struct OnClauseInfo;
  using OnClauseListT = std::vector<OnClauseInfo>;

  Lexer in;

  Module& wasm;
  Builder builder;

  const std::vector<HeapType>& types;
  const std::unordered_map<Index, HeapType>& implicitTypes;
  const std::unordered_map<HeapType, std::unordered_map<Name, Index>>&
    typeNames;
  const std::unordered_map<Index, Index>& implicitElemIndices;

  std::unordered_map<std::string_view, Index> debugSymbolNameIndices;
  std::unordered_map<std::string_view, Index> debugFileIndices;

  // The index of the current module element.
  Index index = 0;

  // The current function being parsed, used to create scratch locals, type
  // local.get, etc.
  Function* func = nullptr;

  IRBuilder irBuilder;

  Result<> visitFunctionStart(Function* func) {
    this->func = func;
    CHECK_ERR(irBuilder.visitFunctionStart(func));
    return Ok{};
  }

  ParseDefsCtx(
    Lexer& in,
    Module& wasm,
    const std::vector<HeapType>& types,
    const std::unordered_map<Index, HeapType>& implicitTypes,
    const std::unordered_map<HeapType, std::unordered_map<Name, Index>>&
      typeNames,
    const std::unordered_map<Index, Index>& implicitElemIndices,
    const IndexMap& typeIndices)
    : TypeParserCtx(typeIndices), in(in), wasm(wasm), builder(wasm),
      types(types), implicitTypes(implicitTypes), typeNames(typeNames),
      implicitElemIndices(implicitElemIndices), irBuilder(wasm) {}

  template<typename T> Result<T> withLoc(Index pos, Result<T> res) {
    if (auto err = res.getErr()) {
      return in.err(pos, err->msg);
    }
    return res;
  }

  template<typename T> Result<T> withLoc(Result<T> res) {
    return withLoc(in.getPos(), res);
  }

  HeapType getBlockTypeFromResult(const std::vector<Type> results) {
    assert(results.size() == 1);
    return HeapType(Signature(Type::none, results[0]));
  }

  Result<HeapType> getBlockTypeFromTypeUse(Index pos, HeapType type) {
    assert(type.isSignature());
    if (type.getSignature().params != Type::none) {
      return in.err(pos, "block parameters not yet supported");
    }
    // TODO: Once we support block parameters, return an error here if any of
    // them are named.
    return type;
  }

  GlobalTypeT makeGlobalType(Mutability, TypeT) { return Ok{}; }

  std::vector<Expression*> makeElemList(TypeT) { return {}; }
  std::vector<Expression*> makeFuncElemList() { return {}; }
  void appendElem(std::vector<Expression*>& elems, Expression* expr) {
    elems.push_back(expr);
  }
  void appendFuncElem(std::vector<Expression*>& elems, Name func) {
    auto type = wasm.getFunction(func)->type;
    elems.push_back(builder.makeRefFunc(func, type));
  }

  LimitsT getLimitsFromElems(std::vector<Expression*>& elems) { return Ok{}; }

  TableTypeT makeTableType(Type, LimitsT, Type) { return Ok{}; }

  struct CatchInfo {
    Name tag;
    Index label;
    bool isRef;
  };

  std::vector<CatchInfo> makeCatchList() { return {}; }
  void appendCatch(std::vector<CatchInfo>& list, CatchInfo info) {
    list.push_back(info);
  }
  CatchInfo makeCatch(Name tag, Index label) { return {tag, label, false}; }
  CatchInfo makeCatchRef(Name tag, Index label) { return {tag, label, true}; }
  CatchInfo makeCatchAll(Index label) { return {{}, label, false}; }
  CatchInfo makeCatchAllRef(Index label) { return {{}, label, true}; }

  struct OnClauseInfo {
    Name tag;
    Index label; // unset when isOnSwitch = true.
    bool isOnSwitch;
  };

  OnClauseInfo makeOnLabel(Name tag, Index label) {
    return {tag, label, false};
  }
  OnClauseInfo makeOnSwitch(Name tag) { return {tag, {}, true}; }

  OnClauseListT makeOnClauseList() { return {}; }
  void appendOnClause(std::vector<OnClauseInfo>& list, OnClauseInfo info) {
    list.push_back(info);
  }

  Result<HeapTypeT> getHeapTypeFromIdx(Index idx) {
    if (idx >= types.size()) {
      return in.err("type index out of bounds");
    }
    return types[idx];
  }

  Result<Index> getFieldFromIdx(HeapType type, uint32_t idx) {
    if (!type.isStruct()) {
      return in.err("expected struct type");
    }
    if (idx >= type.getStruct().fields.size()) {
      return in.err("struct index out of bounds");
    }
    return idx;
  }

  Result<Index> getFieldFromName(HeapType type, Name name) {
    if (auto typeIt = typeNames.find(type); typeIt != typeNames.end()) {
      const auto& fieldIdxs = typeIt->second;
      if (auto fieldIt = fieldIdxs.find(name); fieldIt != fieldIdxs.end()) {
        return fieldIt->second;
      }
    }
    return in.err("unrecognized field name");
  }

  Result<Index> getLocalFromIdx(uint32_t idx) {
    if (!func) {
      return in.err("cannot access locals outside of a function");
    }
    if (idx >= func->getNumLocals()) {
      return in.err("local index out of bounds");
    }
    return idx;
  }

  Result<Name> getFuncFromIdx(uint32_t idx) {
    if (idx >= wasm.functions.size()) {
      return in.err("function index out of bounds");
    }
    return wasm.functions[idx]->name;
  }

  Result<Name> getFuncFromName(Name name) {
    if (!wasm.getFunctionOrNull(name)) {
      return in.err("function $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<Index> getLocalFromName(Name name) {
    if (!func) {
      return in.err("cannot access locals outside of a function");
    }
    if (!func->hasLocalIndex(name)) {
      return in.err("local $" + name.toString() + " does not exist");
    }
    return func->getLocalIndex(name);
  }

  Result<Name> getGlobalFromIdx(uint32_t idx) {
    if (idx >= wasm.globals.size()) {
      return in.err("global index out of bounds");
    }
    return wasm.globals[idx]->name;
  }

  Result<Name> getGlobalFromName(Name name) {
    if (!wasm.getGlobalOrNull(name)) {
      return in.err("global $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<Name> getTableFromIdx(uint32_t idx) {
    if (idx >= wasm.tables.size()) {
      return in.err("table index out of bounds");
    }
    return wasm.tables[idx]->name;
  }

  Result<Name> getTableFromName(Name name) {
    if (!wasm.getTableOrNull(name)) {
      return in.err("table $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<Name> getMemoryFromIdx(uint32_t idx) {
    if (idx >= wasm.memories.size()) {
      return in.err("memory index out of bounds");
    }
    return wasm.memories[idx]->name;
  }

  Result<Name> getMemoryFromName(Name name) {
    if (!wasm.getMemoryOrNull(name)) {
      return in.err("memory $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<Name> getElemFromIdx(uint32_t idx) {
    if (idx >= wasm.elementSegments.size()) {
      return in.err("elem index out of bounds");
    }
    return wasm.elementSegments[idx]->name;
  }

  Result<Name> getElemFromName(Name name) {
    if (!wasm.getElementSegmentOrNull(name)) {
      return in.err("elem $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<Name> getDataFromIdx(uint32_t idx) {
    if (idx >= wasm.dataSegments.size()) {
      return in.err("data index out of bounds");
    }
    return wasm.dataSegments[idx]->name;
  }

  Result<Name> getDataFromName(Name name) {
    if (!wasm.getDataSegmentOrNull(name)) {
      return in.err("data $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<Index> getLabelFromIdx(uint32_t idx, bool) { return idx; }

  Result<Index> getLabelFromName(Name name, bool inDelegate) {
    return irBuilder.getLabelIndex(name, inDelegate);
  }

  Result<Name> getTagFromIdx(uint32_t idx) {
    if (idx >= wasm.tags.size()) {
      return in.err("tag index out of bounds");
    }
    return wasm.tags[idx]->name;
  }

  Result<Name> getTagFromName(Name name) {
    if (!wasm.getTagOrNull(name)) {
      return in.err("tag $" + name.toString() + " does not exist");
    }
    return name;
  }

  Result<TypeUseT> makeTypeUse(Index pos,
                               std::optional<HeapTypeT> type,
                               ParamsT* params,
                               ResultsT* results);

  Result<> addFunc(Name,
                   const std::vector<Name>&,
                   ImportNames*,
                   TypeUseT,
                   std::optional<LocalsT>,
                   std::vector<Annotation>&&,
                   Index) {
    return Ok{};
  }

  Result<>
  addTable(Name, const std::vector<Name>&, ImportNames*, TableTypeT, Index) {
    return Ok{};
  }

  Result<>
  addMemory(Name, const std::vector<Name>&, ImportNames*, TableTypeT, Index) {
    return Ok{};
  }

  Result<> addGlobal(Name,
                     const std::vector<Name>&,
                     ImportNames*,
                     GlobalTypeT,
                     std::optional<ExprT> exp,
                     Index);

  Result<> addStart(Name name, Index pos) {
    wasm.start = name;
    return Ok{};
  }

  Result<> addImplicitElems(Type type, std::vector<Expression*>&& elems);

  Result<> addDeclareElem(Name, std::vector<Expression*>&&, Index) {
    // TODO: Validate that referenced functions appear in a declarative element
    // segment.
    return Ok{};
  }

  Result<> addElem(Name,
                   Name* table,
                   std::optional<Expression*> offset,
                   std::vector<Expression*>&& elems,
                   Index pos);

  Result<>
  addData(Name, Name* mem, std::optional<ExprT> offset, DataStringT, Index pos);

  Result<>
  addTag(Name, const std::vector<Name>, ImportNames*, TypeUseT, Index) {
    return Ok{};
  }

  Result<> addExport(Index pos, Name value, Name name, ExternalKind kind) {
    if (wasm.getExportOrNull(name)) {
      return in.err(pos, "duplicate export");
    }
    wasm.addExport(builder.makeExport(name, value, kind));
    return Ok{};
  }

  Result<Index> addScratchLocal(Index pos, Type type) {
    if (!func) {
      return in.err(pos,
                    "scratch local required, but there is no function context");
    }
    Name name = Names::getValidLocalName(*func, "scratch");
    return Builder::addVar(func, name, type);
  }

  Result<Expression*> makeExpr() { return withLoc(irBuilder.build()); }

  Memarg getMemarg(uint64_t offset, uint32_t align) { return {offset, align}; }

  Result<Name> getTable(Index pos, Name* table) {
    if (table) {
      return *table;
    }
    if (wasm.tables.empty()) {
      return in.err(pos, "table required, but there is no table");
    }
    return wasm.tables[0]->name;
  }

  Result<Name> getMemory(Index pos, Name* mem) {
    if (mem) {
      return *mem;
    }
    if (wasm.memories.empty()) {
      return in.err(pos, "memory required, but there is no memory");
    }
    return wasm.memories[0]->name;
  }

  void setSrcLoc(const std::vector<Annotation>& annotations) {
    const Annotation* annotation = nullptr;
    for (auto& a : annotations) {
      if (a.kind == srcAnnotationKind) {
        annotation = &a;
      }
    }
    if (!annotation) {
      return;
    }
    Lexer lexer(annotation->contents);
    if (lexer.empty()) {
      irBuilder.setDebugLocation(std::nullopt);
      return;
    }

    auto contents = lexer.next();

    auto fileSize = contents.find(':');
    if (fileSize == 0 || fileSize == contents.npos) {
      return;
    }
    auto file = contents.substr(0, fileSize);
    contents = contents.substr(fileSize + 1);

    auto lineSize = contents.find(':');
    if (lineSize == contents.npos) {
      return;
    }
    lexer = Lexer(contents.substr(0, lineSize));
    auto line = lexer.takeU32();
    if (!line || !lexer.empty()) {
      return;
    }
    contents = contents.substr(lineSize + 1);

    auto colSize = contents.find(':');
    if (colSize == contents.npos) {
      colSize = contents.size();
      if (colSize == 0) {
        return;
      }
    }
    lexer = Lexer(contents.substr(0, colSize));
    auto col = lexer.takeU32();
    if (!col) {
      return;
    }

    std::optional<BinaryLocation> symbolNameIndex;
    if (colSize != contents.size()) {
      contents = contents.substr(colSize + 1);
      auto symbolName = contents;
      auto [it, inserted] = debugSymbolNameIndices.insert(
        {symbolName, debugSymbolNameIndices.size()});
      if (inserted) {
        assert(wasm.debugInfoSymbolNames.size() == it->second);
        wasm.debugInfoSymbolNames.push_back(std::string(symbolName));
      }
      symbolNameIndex = it->second;
    }

    // TODO: If we ever parallelize the parse, access to
    // `wasm.debugInfoFileNames` will have to be protected by a lock.
    auto [it, inserted] =
      debugFileIndices.insert({file, debugFileIndices.size()});
    if (inserted) {
      assert(wasm.debugInfoFileNames.size() == it->second);
      wasm.debugInfoFileNames.push_back(std::string(file));
    }
    irBuilder.setDebugLocation(
      Function::DebugLocation({it->second, *line, *col, symbolNameIndex}));
  }

  Result<> makeBlock(Index pos,
                     const std::vector<Annotation>& annotations,
                     std::optional<Name> label,
                     HeapType type) {
    // TODO: validate labels?
    // TODO: Move error on input types to here?
    return withLoc(pos,
                   irBuilder.makeBlock(label ? *label : Name{},
                                       type.getSignature().results));
  }

  Result<> makeIf(Index pos,
                  const std::vector<Annotation>& annotations,
                  std::optional<Name> label,
                  HeapType type) {
    // TODO: validate labels?
    // TODO: Move error on input types to here?
    return withLoc(
      pos,
      irBuilder.makeIf(label ? *label : Name{}, type.getSignature().results));
  }

  Result<> visitElse() { return withLoc(irBuilder.visitElse()); }

  Result<> makeLoop(Index pos,
                    const std::vector<Annotation>& annotations,
                    std::optional<Name> label,
                    HeapType type) {
    // TODO: validate labels?
    // TODO: Move error on input types to here?
    return withLoc(
      pos,
      irBuilder.makeLoop(label ? *label : Name{}, type.getSignature().results));
  }

  Result<> makeTry(Index pos,
                   const std::vector<Annotation>& annotations,
                   std::optional<Name> label,
                   HeapType type) {
    // TODO: validate labels?
    // TODO: Move error on input types to here?
    return withLoc(
      pos,
      irBuilder.makeTry(label ? *label : Name{}, type.getSignature().results));
  }

  Result<> makeTryTable(Index pos,
                        const std::vector<Annotation>& annotations,
                        std::optional<Name> label,
                        HeapType type,
                        const std::vector<CatchInfo>& info) {
    std::vector<Name> tags;
    std::vector<Index> labels;
    std::vector<bool> isRefs;
    for (auto& info : info) {
      tags.push_back(info.tag);
      labels.push_back(info.label);
      isRefs.push_back(info.isRef);
    }
    return withLoc(pos,
                   irBuilder.makeTryTable(label ? *label : Name{},
                                          type.getSignature().results,
                                          tags,
                                          labels,
                                          isRefs));
  }

  Result<> visitCatch(Index pos, Name tag) {
    return withLoc(pos, irBuilder.visitCatch(tag));
  }

  Result<> visitCatchAll(Index pos) {
    return withLoc(pos, irBuilder.visitCatchAll());
  }

  Result<> visitDelegate(Index pos, Index label) {
    return withLoc(pos, irBuilder.visitDelegate(label));
  }

  Result<> visitEnd() { return withLoc(irBuilder.visitEnd()); }

  Result<> makeUnreachable(Index pos,
                           const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeUnreachable());
  }

  Result<> makeNop(Index pos, const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeNop());
  }

  Result<> makeBinary(Index pos,
                      const std::vector<Annotation>& annotations,
                      BinaryOp op) {
    return withLoc(pos, irBuilder.makeBinary(op));
  }

  Result<>
  makeUnary(Index pos, const std::vector<Annotation>& annotations, UnaryOp op) {
    return withLoc(pos, irBuilder.makeUnary(op));
  }

  Result<> makeSelect(Index pos,
                      const std::vector<Annotation>& annotations,
                      std::vector<Type>* res) {
    if (res && res->size()) {
      if (res->size() > 1) {
        return in.err(pos, "select may not have more than one result type");
      }
      return withLoc(pos, irBuilder.makeSelect((*res)[0]));
    }
    return withLoc(pos, irBuilder.makeSelect());
  }

  Result<> makeDrop(Index pos, const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeDrop());
  }

  Result<> makeMemorySize(Index pos,
                          const std::vector<Annotation>& annotations,
                          Name* mem) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos, irBuilder.makeMemorySize(*m));
  }

  Result<> makeMemoryGrow(Index pos,
                          const std::vector<Annotation>& annotations,
                          Name* mem) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos, irBuilder.makeMemoryGrow(*m));
  }

  Result<> makeLocalGet(Index pos,
                        const std::vector<Annotation>& annotations,
                        Index local) {
    return withLoc(pos, irBuilder.makeLocalGet(local));
  }

  Result<> makeLocalTee(Index pos,
                        const std::vector<Annotation>& annotations,
                        Index local) {
    return withLoc(pos, irBuilder.makeLocalTee(local));
  }

  Result<> makeLocalSet(Index pos,
                        const std::vector<Annotation>& annotations,
                        Index local) {
    return withLoc(pos, irBuilder.makeLocalSet(local));
  }

  Result<> makeGlobalGet(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name global) {
    return withLoc(pos, irBuilder.makeGlobalGet(global));
  }

  Result<> makeGlobalSet(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name global) {
    assert(wasm.getGlobalOrNull(global));
    return withLoc(pos, irBuilder.makeGlobalSet(global));
  }

  Result<> makeI32Const(Index pos,
                        const std::vector<Annotation>& annotations,
                        uint32_t c) {
    return withLoc(pos, irBuilder.makeConst(Literal(c)));
  }

  Result<> makeI64Const(Index pos,
                        const std::vector<Annotation>& annotations,
                        uint64_t c) {
    return withLoc(pos, irBuilder.makeConst(Literal(c)));
  }

  Result<>
  makeF32Const(Index pos, const std::vector<Annotation>& annotations, float c) {
    return withLoc(pos, irBuilder.makeConst(Literal(c)));
  }

  Result<> makeF64Const(Index pos,
                        const std::vector<Annotation>& annotations,
                        double c) {
    return withLoc(pos, irBuilder.makeConst(Literal(c)));
  }

  Result<> makeI8x16Const(Index pos,
                          const std::vector<Annotation>& annotations,
                          const std::array<uint8_t, 16>& vals) {
    std::array<Literal, 16> lanes;
    for (size_t i = 0; i < 16; ++i) {
      lanes[i] = Literal(uint32_t(vals[i]));
    }
    return withLoc(pos, irBuilder.makeConst(Literal(lanes)));
  }

  Result<> makeI16x8Const(Index pos,
                          const std::vector<Annotation>& annotations,
                          const std::array<uint16_t, 8>& vals) {
    std::array<Literal, 8> lanes;
    for (size_t i = 0; i < 8; ++i) {
      lanes[i] = Literal(uint32_t(vals[i]));
    }
    return withLoc(pos, irBuilder.makeConst(Literal(lanes)));
  }

  Result<> makeI32x4Const(Index pos,
                          const std::vector<Annotation>& annotations,
                          const std::array<uint32_t, 4>& vals) {
    std::array<Literal, 4> lanes;
    for (size_t i = 0; i < 4; ++i) {
      lanes[i] = Literal(vals[i]);
    }
    return withLoc(pos, irBuilder.makeConst(Literal(lanes)));
  }

  Result<> makeI64x2Const(Index pos,
                          const std::vector<Annotation>& annotations,
                          const std::array<uint64_t, 2>& vals) {
    std::array<Literal, 2> lanes;
    for (size_t i = 0; i < 2; ++i) {
      lanes[i] = Literal(vals[i]);
    }
    return withLoc(pos, irBuilder.makeConst(Literal(lanes)));
  }

  Result<> makeF32x4Const(Index pos,
                          const std::vector<Annotation>& annotations,
                          const std::array<float, 4>& vals) {
    std::array<Literal, 4> lanes;
    for (size_t i = 0; i < 4; ++i) {
      lanes[i] = Literal(vals[i]);
    }
    return withLoc(pos, irBuilder.makeConst(Literal(lanes)));
  }

  Result<> makeF64x2Const(Index pos,
                          const std::vector<Annotation>& annotations,
                          const std::array<double, 2>& vals) {
    std::array<Literal, 2> lanes;
    for (size_t i = 0; i < 2; ++i) {
      lanes[i] = Literal(vals[i]);
    }
    return withLoc(pos, irBuilder.makeConst(Literal(lanes)));
  }

  Result<> makeLoad(Index pos,
                    const std::vector<Annotation>& annotations,
                    Type type,
                    bool signed_,
                    int bytes,
                    bool isAtomic,
                    Name* mem,
                    Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    if (isAtomic) {
      return withLoc(pos,
                     irBuilder.makeAtomicLoad(bytes, memarg.offset, type, *m));
    }
    return withLoc(pos,
                   irBuilder.makeLoad(
                     bytes, signed_, memarg.offset, memarg.align, type, *m));
  }

  Result<> makeStore(Index pos,
                     const std::vector<Annotation>& annotations,
                     Type type,
                     int bytes,
                     bool isAtomic,
                     Name* mem,
                     Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    if (isAtomic) {
      return withLoc(pos,
                     irBuilder.makeAtomicStore(bytes, memarg.offset, type, *m));
    }
    return withLoc(
      pos, irBuilder.makeStore(bytes, memarg.offset, memarg.align, type, *m));
  }

  Result<> makeAtomicRMW(Index pos,
                         const std::vector<Annotation>& annotations,
                         AtomicRMWOp op,
                         Type type,
                         int bytes,
                         Name* mem,
                         Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos,
                   irBuilder.makeAtomicRMW(op, bytes, memarg.offset, type, *m));
  }

  Result<> makeAtomicCmpxchg(Index pos,
                             const std::vector<Annotation>& annotations,
                             Type type,
                             int bytes,
                             Name* mem,
                             Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos,
                   irBuilder.makeAtomicCmpxchg(bytes, memarg.offset, type, *m));
  }

  Result<> makeAtomicWait(Index pos,
                          const std::vector<Annotation>& annotations,
                          Type type,
                          Name* mem,
                          Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos, irBuilder.makeAtomicWait(type, memarg.offset, *m));
  }

  Result<> makeAtomicNotify(Index pos,
                            const std::vector<Annotation>& annotations,
                            Name* mem,
                            Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos, irBuilder.makeAtomicNotify(memarg.offset, *m));
  }

  Result<> makeAtomicFence(Index pos,
                           const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeAtomicFence());
  }

  Result<> makeSIMDExtract(Index pos,
                           const std::vector<Annotation>& annotations,
                           SIMDExtractOp op,
                           uint8_t lane) {
    return withLoc(pos, irBuilder.makeSIMDExtract(op, lane));
  }

  Result<> makeSIMDReplace(Index pos,
                           const std::vector<Annotation>& annotations,
                           SIMDReplaceOp op,
                           uint8_t lane) {
    return withLoc(pos, irBuilder.makeSIMDReplace(op, lane));
  }

  Result<> makeSIMDShuffle(Index pos,
                           const std::vector<Annotation>& annotations,
                           const std::array<uint8_t, 16>& lanes) {
    return withLoc(pos, irBuilder.makeSIMDShuffle(lanes));
  }

  Result<> makeSIMDTernary(Index pos,
                           const std::vector<Annotation>& annotations,
                           SIMDTernaryOp op) {
    return withLoc(pos, irBuilder.makeSIMDTernary(op));
  }

  Result<> makeSIMDShift(Index pos,
                         const std::vector<Annotation>& annotations,
                         SIMDShiftOp op) {
    return withLoc(pos, irBuilder.makeSIMDShift(op));
  }

  Result<> makeSIMDLoad(Index pos,
                        const std::vector<Annotation>& annotations,
                        SIMDLoadOp op,
                        Name* mem,
                        Memarg memarg) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos,
                   irBuilder.makeSIMDLoad(op, memarg.offset, memarg.align, *m));
  }

  Result<> makeSIMDLoadStoreLane(Index pos,
                                 const std::vector<Annotation>& annotations,
                                 SIMDLoadStoreLaneOp op,
                                 Name* mem,
                                 Memarg memarg,
                                 uint8_t lane) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos,
                   irBuilder.makeSIMDLoadStoreLane(
                     op, memarg.offset, memarg.align, lane, *m));
  }

  Result<> makeMemoryInit(Index pos,
                          const std::vector<Annotation>& annotations,
                          Name* mem,
                          Name data) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos, irBuilder.makeMemoryInit(data, *m));
  }

  Result<> makeDataDrop(Index pos,
                        const std::vector<Annotation>& annotations,
                        Name data) {
    return withLoc(pos, irBuilder.makeDataDrop(data));
  }

  Result<> makeMemoryCopy(Index pos,
                          const std::vector<Annotation>& annotations,
                          Name* destMem,
                          Name* srcMem) {
    auto destMemory = getMemory(pos, destMem);
    CHECK_ERR(destMemory);
    auto srcMemory = getMemory(pos, srcMem);
    CHECK_ERR(srcMemory);
    return withLoc(pos, irBuilder.makeMemoryCopy(*destMemory, *srcMemory));
  }

  Result<> makeMemoryFill(Index pos,
                          const std::vector<Annotation>& annotations,
                          Name* mem) {
    auto m = getMemory(pos, mem);
    CHECK_ERR(m);
    return withLoc(pos, irBuilder.makeMemoryFill(*m));
  }

  Result<>
  makePop(Index pos, const std::vector<Annotation>& annotations, Type type) {
    return withLoc(pos, irBuilder.makePop(type));
  }

  Result<> makeCall(Index pos,
                    const std::vector<Annotation>& annotations,
                    Name func,
                    bool isReturn) {
    return withLoc(pos, irBuilder.makeCall(func, isReturn));
  }

  Result<> makeCallIndirect(Index pos,
                            const std::vector<Annotation>& annotations,
                            Name* table,
                            HeapType type,
                            bool isReturn) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeCallIndirect(*t, type, isReturn));
  }

  Result<> makeBreak(Index pos,
                     const std::vector<Annotation>& annotations,
                     Index label,
                     bool isConditional) {
    return withLoc(pos, irBuilder.makeBreak(label, isConditional));
  }

  Result<> makeSwitch(Index pos,
                      const std::vector<Annotation>& annotations,
                      const std::vector<Index> labels,
                      Index defaultLabel) {
    return withLoc(pos, irBuilder.makeSwitch(labels, defaultLabel));
  }

  Result<> makeReturn(Index pos, const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeReturn());
  }

  Result<> makeRefNull(Index pos,
                       const std::vector<Annotation>& annotations,
                       HeapType type) {
    return withLoc(pos, irBuilder.makeRefNull(type));
  }

  Result<> makeRefIsNull(Index pos,
                         const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeRefIsNull());
  }

  Result<> makeRefFunc(Index pos,
                       const std::vector<Annotation>& annotations,
                       Name func) {
    return withLoc(pos, irBuilder.makeRefFunc(func));
  }

  Result<> makeRefEq(Index pos, const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeRefEq());
  }

  Result<> makeTableGet(Index pos,
                        const std::vector<Annotation>& annotations,
                        Name* table) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeTableGet(*t));
  }

  Result<> makeTableSet(Index pos,
                        const std::vector<Annotation>& annotations,
                        Name* table) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeTableSet(*t));
  }

  Result<> makeTableSize(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name* table) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeTableSize(*t));
  }

  Result<> makeTableGrow(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name* table) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeTableGrow(*t));
  }

  Result<> makeTableFill(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name* table) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeTableFill(*t));
  }

  Result<> makeTableCopy(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name* destTable,
                         Name* srcTable) {
    auto dest = getTable(pos, destTable);
    CHECK_ERR(dest);
    auto src = getTable(pos, srcTable);
    CHECK_ERR(src);
    return withLoc(pos, irBuilder.makeTableCopy(*dest, *src));
  }

  Result<> makeTableInit(Index pos,
                         const std::vector<Annotation>& annotations,
                         Name* table,
                         Name elem) {
    auto t = getTable(pos, table);
    CHECK_ERR(t);
    return withLoc(pos, irBuilder.makeTableInit(elem, *t));
  }

  Result<>
  makeThrow(Index pos, const std::vector<Annotation>& annotations, Name tag) {
    return withLoc(pos, irBuilder.makeThrow(tag));
  }

  Result<> makeRethrow(Index pos,
                       const std::vector<Annotation>& annotations,
                       Index label) {
    return withLoc(pos, irBuilder.makeRethrow(label));
  }

  Result<> makeThrowRef(Index pos, const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeThrowRef());
  }

  Result<> makeTupleMake(Index pos,
                         const std::vector<Annotation>& annotations,
                         uint32_t arity) {
    return withLoc(pos, irBuilder.makeTupleMake(arity));
  }

  Result<> makeTupleExtract(Index pos,
                            const std::vector<Annotation>& annotations,
                            uint32_t arity,
                            uint32_t index) {
    return withLoc(pos, irBuilder.makeTupleExtract(arity, index));
  }

  Result<> makeTupleDrop(Index pos,
                         const std::vector<Annotation>& annotations,
                         uint32_t arity) {
    return withLoc(pos, irBuilder.makeTupleDrop(arity));
  }

  Result<> makeCallRef(Index pos,
                       const std::vector<Annotation>& annotations,
                       HeapType type,
                       bool isReturn) {
    return withLoc(pos, irBuilder.makeCallRef(type, isReturn));
  }

  Result<> makeRefI31(Index pos,
                      const std::vector<Annotation>& annotations,
                      Shareability share) {
    return withLoc(pos, irBuilder.makeRefI31(share));
  }

  Result<> makeI31Get(Index pos,
                      const std::vector<Annotation>& annotations,
                      bool signed_) {
    return withLoc(pos, irBuilder.makeI31Get(signed_));
  }

  Result<> makeRefTest(Index pos,
                       const std::vector<Annotation>& annotations,
                       Type type) {
    return withLoc(pos, irBuilder.makeRefTest(type));
  }

  Result<> makeRefCast(Index pos,
                       const std::vector<Annotation>& annotations,
                       Type type) {
    return withLoc(pos, irBuilder.makeRefCast(type));
  }

  Result<> makeBrOn(Index pos,
                    const std::vector<Annotation>& annotations,
                    Index label,
                    BrOnOp op,
                    Type in = Type::none,
                    Type out = Type::none) {
    return withLoc(pos, irBuilder.makeBrOn(label, op, in, out));
  }

  Result<> makeStructNew(Index pos,
                         const std::vector<Annotation>& annotations,
                         HeapType type) {
    return withLoc(pos, irBuilder.makeStructNew(type));
  }

  Result<> makeStructNewDefault(Index pos,
                                const std::vector<Annotation>& annotations,
                                HeapType type) {
    return withLoc(pos, irBuilder.makeStructNewDefault(type));
  }

  Result<> makeStructGet(Index pos,
                         const std::vector<Annotation>& annotations,
                         HeapType type,
                         Index field,
                         bool signed_) {
    return withLoc(pos, irBuilder.makeStructGet(type, field, signed_));
  }

  Result<> makeStructSet(Index pos,
                         const std::vector<Annotation>& annotations,
                         HeapType type,
                         Index field) {
    return withLoc(pos, irBuilder.makeStructSet(type, field));
  }

  Result<> makeArrayNew(Index pos,
                        const std::vector<Annotation>& annotations,
                        HeapType type) {
    return withLoc(pos, irBuilder.makeArrayNew(type));
  }

  Result<> makeArrayNewDefault(Index pos,
                               const std::vector<Annotation>& annotations,
                               HeapType type) {
    return withLoc(pos, irBuilder.makeArrayNewDefault(type));
  }

  Result<> makeArrayNewData(Index pos,
                            const std::vector<Annotation>& annotations,
                            HeapType type,
                            Name data) {
    return withLoc(pos, irBuilder.makeArrayNewData(type, data));
  }

  Result<> makeArrayNewElem(Index pos,
                            const std::vector<Annotation>& annotations,
                            HeapType type,
                            Name elem) {
    return withLoc(pos, irBuilder.makeArrayNewElem(type, elem));
  }

  Result<> makeArrayNewFixed(Index pos,
                             const std::vector<Annotation>& annotations,
                             HeapType type,
                             uint32_t arity) {
    return withLoc(pos, irBuilder.makeArrayNewFixed(type, arity));
  }

  Result<> makeArrayGet(Index pos,
                        const std::vector<Annotation>& annotations,
                        HeapType type,
                        bool signed_) {
    return withLoc(pos, irBuilder.makeArrayGet(type, signed_));
  }

  Result<> makeArraySet(Index pos,
                        const std::vector<Annotation>& annotations,
                        HeapType type) {
    return withLoc(pos, irBuilder.makeArraySet(type));
  }

  Result<> makeArrayLen(Index pos, const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeArrayLen());
  }

  Result<> makeArrayCopy(Index pos,
                         const std::vector<Annotation>& annotations,
                         HeapType destType,
                         HeapType srcType) {
    return withLoc(pos, irBuilder.makeArrayCopy(destType, srcType));
  }

  Result<> makeArrayFill(Index pos,
                         const std::vector<Annotation>& annotations,
                         HeapType type) {
    return withLoc(pos, irBuilder.makeArrayFill(type));
  }

  Result<> makeArrayInitData(Index pos,
                             const std::vector<Annotation>& annotations,
                             HeapType type,
                             Name data) {
    return withLoc(pos, irBuilder.makeArrayInitData(type, data));
  }

  Result<> makeArrayInitElem(Index pos,
                             const std::vector<Annotation>& annotations,
                             HeapType type,
                             Name elem) {
    return withLoc(pos, irBuilder.makeArrayInitElem(type, elem));
  }

  Result<>
  makeRefAs(Index pos, const std::vector<Annotation>& annotations, RefAsOp op) {
    return withLoc(pos, irBuilder.makeRefAs(op));
  }

  Result<> makeStringNew(Index pos,
                         const std::vector<Annotation>& annotations,
                         StringNewOp op) {
    return withLoc(pos, irBuilder.makeStringNew(op));
  }

  Result<> makeStringConst(Index pos,
                           const std::vector<Annotation>& annotations,
                           std::string_view str) {
    // Re-encode from WTF-8 to WTF-16.
    std::stringstream wtf16;
    if (!String::convertWTF8ToWTF16(wtf16, str)) {
      return in.err(pos, "invalid string constant");
    }
    // TODO: Use wtf16.view() once we have C++20.
    return withLoc(pos, irBuilder.makeStringConst(wtf16.str()));
  }

  Result<> makeStringMeasure(Index pos,
                             const std::vector<Annotation>& annotations,
                             StringMeasureOp op) {
    return withLoc(pos, irBuilder.makeStringMeasure(op));
  }

  Result<> makeStringEncode(Index pos,
                            const std::vector<Annotation>& annotations,
                            StringEncodeOp op) {
    return withLoc(pos, irBuilder.makeStringEncode(op));
  }

  Result<> makeStringConcat(Index pos,
                            const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeStringConcat());
  }

  Result<> makeStringEq(Index pos,
                        const std::vector<Annotation>& annotations,
                        StringEqOp op) {
    return withLoc(pos, irBuilder.makeStringEq(op));
  }

  Result<> makeStringWTF16Get(Index pos,
                              const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeStringWTF16Get());
  }

  Result<> makeStringSliceWTF(Index pos,
                              const std::vector<Annotation>& annotations) {
    return withLoc(pos, irBuilder.makeStringSliceWTF());
  }

  Result<> makeContNew(Index pos,
                       const std::vector<Annotation>& annotations,
                       HeapType type) {
    return withLoc(pos, irBuilder.makeContNew(type));
  }

  Result<> makeContBind(Index pos,
                        const std::vector<Annotation>& annotations,
                        HeapType sourceType,
                        HeapType targetType) {
    return withLoc(pos, irBuilder.makeContBind(sourceType, targetType));
  }

  Result<>
  makeSuspend(Index pos, const std::vector<Annotation>& annotations, Name tag) {
    return withLoc(pos, irBuilder.makeSuspend(tag));
  }

  Result<> makeResume(Index pos,
                      const std::vector<Annotation>& annotations,
                      HeapType type,
                      const std::vector<OnClauseInfo>& resumetable) {
    std::vector<Name> tags;
    std::vector<std::optional<Index>> labels;
    tags.reserve(resumetable.size());
    labels.reserve(resumetable.size());
    for (const OnClauseInfo& info : resumetable) {
      tags.push_back(info.tag);
      if (info.isOnSwitch) {
        labels.push_back(std::nullopt);
      } else {
        labels.push_back(std::optional<Index>(info.label));
      }
    }
    return withLoc(pos, irBuilder.makeResume(type, tags, labels));
  }

  Result<> makeResumeThrow(Index pos,
                           const std::vector<Annotation>& annotations,
                           HeapType type,
                           Name tag,
                           const std::vector<OnClauseInfo>& resumetable) {
    std::vector<Name> tags;
    std::vector<std::optional<Index>> labels;
    tags.reserve(resumetable.size());
    labels.reserve(resumetable.size());
    for (const OnClauseInfo& info : resumetable) {
      tags.push_back(info.tag);
      if (info.isOnSwitch) {
        labels.push_back(std::nullopt);
      } else {
        labels.push_back(std::optional<Index>(info.label));
      }
    }
    return withLoc(pos, irBuilder.makeResumeThrow(type, tag, tags, labels));
  }

  Result<> makeStackSwitch(Index pos,
                           const std::vector<Annotation>& annotations,
                           HeapType type,
                           Name tag) {
    return withLoc(pos, irBuilder.makeStackSwitch(type, tag));
  }
};

} // namespace wasm::WATParser

#endif // parser_context_h
