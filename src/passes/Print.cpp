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

//
// Print out text in s-expression format
//

#include <ir/iteration.h>
#include <ir/module-utils.h>
#include <ir/table-utils.h>
#include <pass.h>
#include <pretty_printing.h>
#include <wasm-stack.h>
#include <wasm.h>

namespace wasm {

static std::ostream& printExpression(Expression* expression,
                                     std::ostream& o,
                                     bool minify = false,
                                     bool full = false,
                                     Module* wasm = nullptr);

static std::ostream&
printStackInst(StackInst* inst, std::ostream& o, Function* func = nullptr);

static std::ostream&
printStackIR(StackIR* ir, std::ostream& o, Function* func = nullptr);

namespace {

bool isFullForced() {
  if (getenv("BINARYEN_PRINT_FULL")) {
    return std::stoi(getenv("BINARYEN_PRINT_FULL")) != 0;
  }
  return false;
}

std::ostream& printName(Name name, std::ostream& o) {
  // we need to quote names if they have tricky chars
  if (!name.str || !strpbrk(name.str, "()")) {
    o << '$' << name.str;
  } else {
    o << "\"$" << name.str << '"';
  }
  return o;
}

static std::ostream& printLocal(Index index, Function* func, std::ostream& o) {
  Name name;
  if (func) {
    name = func->getLocalNameOrDefault(index);
  }
  if (!name) {
    name = Name::fromInt(index);
  }
  return printName(name, o);
}

namespace {

// Helper for printing the name of a type. This output is guaranteed to not
// contain spaces.
struct TypeNamePrinter {
  // Optional. If present, the module's HeapType names will be used.
  Module* wasm;

  // Keep track of the first depth at which we see each HeapType so if we see it
  // again, we can unambiguously refer to it without infinitely recursing.
  size_t currHeapTypeDepth = 0;
  std::unordered_map<HeapType, size_t> heapTypeDepths;

  // The stream we are printing to.
  std::ostream& os;

  TypeNamePrinter(std::ostream& os, Module* wasm = nullptr)
    : wasm(wasm), os(os) {}

  void print(Type type);
  void print(HeapType heapType);
  void print(const Tuple& tuple);
  void print(const Field& field);
  void print(const Signature& sig);
  void print(const Struct& struct_);
  void print(const Array& array);
  void print(const Rtt& rtt);

  // FIXME: This hard limit on how many times we call print() avoids extremely
  //        large outputs, which can be inconveniently large in some cases, but
  //        we should have a better mechanism for this.
  static const size_t MaxPrints = 100;

  size_t prints = 0;

  bool exceededLimit() {
    if (prints >= MaxPrints) {
      os << "?";
      return true;
    }
    prints++;
    return false;
  }
};

void TypeNamePrinter::print(Type type) {
  if (exceededLimit()) {
    return;
  }
  if (type.isBasic()) {
    os << type;
  } else if (type.isTuple()) {
    print(type.getTuple());
  } else if (type.isRtt()) {
    print(type.getRtt());
  } else if (type.isRef()) {
    os << "ref";
    if (type.isNullable()) {
      os << "?";
    }
    os << '|';
    print(type.getHeapType());
    os << '|';
  } else {
    WASM_UNREACHABLE("unexpected type");
  }
}

void TypeNamePrinter::print(HeapType type) {
  if (exceededLimit()) {
    return;
  }
  if (type.isBasic()) {
    os << type;
    return;
  }
  // If there is a name for this type in this module, use it.
  // FIXME: in theory there could be two types, one with a name, and one
  // without, and the one without gets an automatic name that matches the
  // other's. To check for that, if (first) we could assert at the very end of
  // this function that the automatic name is not present in the given names.
  if (wasm && wasm->typeNames.count(type)) {
    os << '$' << wasm->typeNames[type].name;
    return;
  }
  // If we have seen this HeapType before, just print its relative depth instead
  // of infinitely recursing.
  auto it = heapTypeDepths.find(type);
  if (it != heapTypeDepths.end()) {
    assert(it->second <= currHeapTypeDepth);
    size_t relativeDepth = currHeapTypeDepth - it->second;
    os << "..." << relativeDepth;
    return;
  }

  // If this is the top-level heap type, add a $
  if (currHeapTypeDepth == 0) {
    os << "$";
  }

  // Update the context for the current HeapType before recursing.
  heapTypeDepths[type] = ++currHeapTypeDepth;

  if (type.isSignature()) {
    print(type.getSignature());
  } else if (type.isStruct()) {
    print(type.getStruct());
  } else if (type.isArray()) {
    print(type.getArray());
  } else {
    WASM_UNREACHABLE("unexpected type");
  }

  // Restore the previous context after the recursion.
  heapTypeDepths.erase(type);
  --currHeapTypeDepth;
}

void TypeNamePrinter::print(const Tuple& tuple) {
  auto sep = "";
  for (auto type : tuple.types) {
    os << sep;
    sep = "_";
    print(type);
  }
}

void TypeNamePrinter::print(const Field& field) {
  if (field.mutable_) {
    os << "mut:";
  }
  if (field.type == Type::i32 && field.packedType != Field::not_packed) {
    if (field.packedType == Field::i8) {
      os << "i8";
    } else if (field.packedType == Field::i16) {
      os << "i16";
    } else {
      WASM_UNREACHABLE("invalid packed type");
    }
  } else {
    print(field.type);
  }
}

void TypeNamePrinter::print(const Signature& sig) {
  // TODO: Switch to using an unambiguous delimiter rather than differentiating
  // only the top level with a different arrow.
  print(sig.params);
  if (currHeapTypeDepth == 1) {
    os << "_=>_";
  } else {
    os << "_->_";
  }
  print(sig.results);
}

void TypeNamePrinter::print(const Struct& struct_) {
  os << '{';
  auto sep = "";
  for (const auto& field : struct_.fields) {
    os << sep;
    sep = "_";
    print(field);
  }
  os << '}';
}

void TypeNamePrinter::print(const Array& array) {
  os << '[';
  print(array.element);
  os << ']';
}

void TypeNamePrinter::print(const Rtt& rtt) {
  os << "rtt_";
  if (rtt.hasDepth()) {
    os << rtt.depth << '_';
  }
  print(rtt.heapType);
}

} // anonymous namespace

static std::ostream& printType(std::ostream& o, Type type, Module* wasm) {
  if (type.isBasic()) {
    o << type;
  } else if (type.isTuple()) {
    o << '(';
    auto sep = "";
    for (const auto& t : type) {
      o << sep;
      printType(o, t, wasm);
      sep = " ";
    }
    o << ')';
  } else if (type.isRtt()) {
    auto rtt = type.getRtt();
    o << "(rtt ";
    if (rtt.hasDepth()) {
      o << rtt.depth << ' ';
    }
    TypeNamePrinter(o, wasm).print(rtt.heapType);
    o << ')';
  } else if (type.isRef() && !type.isBasic()) {
    o << "(ref ";
    if (type.isNullable()) {
      o << "null ";
    }
    TypeNamePrinter(o, wasm).print(type.getHeapType());
    o << ')';
  } else {
    WASM_UNREACHABLE("unexpected type");
  }
  return o;
}

static std::ostream& printPrefixedTypes(std::ostream& o,
                                        const char* prefix,
                                        Type type,
                                        Module* wasm) {
  o << '(' << prefix;
  if (type == Type::none) {
    return o << ')';
  }
  if (type.isTuple()) {
    // Tuple types are not printed in parens, we can just emit them one after
    // the other in the same list as the "result".
    for (auto t : type) {
      o << ' ';
      printType(o, t, wasm);
    }
  } else {
    o << ' ';
    printType(o, type, wasm);
  }
  o << ')';
  return o;
}

static std::ostream& printResultType(std::ostream& o, Type type, Module* wasm) {
  return printPrefixedTypes(o, "result", type, wasm);
}

static std::ostream& printParamType(std::ostream& o, Type type, Module* wasm) {
  return printPrefixedTypes(o, "param", type, wasm);
}

// Generic processing of a struct's field, given an optional module. Calls func
// with the field name, if it is present, or with a null Name if not.
template<typename T>
void processFieldName(Module* wasm, HeapType type, Index index, T func) {
  if (wasm) {
    auto it = wasm->typeNames.find(type);
    if (it != wasm->typeNames.end()) {
      auto& fieldNames = it->second.fieldNames;
      auto it = fieldNames.find(index);
      if (it != fieldNames.end()) {
        auto name = it->second;
        if (name.is()) {
          func(it->second);
          return;
        }
      }
    }
  }
  func(Name());
}

} // anonymous namespace

// Printing "unreachable" as a instruction prefix type is not valid in wasm text
// format. Print something else to make it pass.
static Type forceConcrete(Type type) {
  return type.isConcrete() ? type : Type::i32;
}

// Prints the internal contents of an expression: everything but
// the children.
struct PrintExpressionContents
  : public OverriddenVisitor<PrintExpressionContents> {
  Module* wasm = nullptr;
  Function* currFunction = nullptr;
  std::ostream& o;
  FeatureSet features;

  PrintExpressionContents(Module* wasm, Function* currFunction, std::ostream& o)
    : wasm(wasm), currFunction(currFunction), o(o), features(wasm->features) {}

  PrintExpressionContents(Function* currFunction, std::ostream& o)
    : currFunction(currFunction), o(o), features(FeatureSet::All) {}

  void visitBlock(Block* curr) {
    printMedium(o, "block");
    if (curr->name.is()) {
      o << ' ';
      printName(curr->name, o);
    }
    if (curr->type.isConcrete()) {
      o << ' ';
      printResultType(o, curr->type, wasm);
    }
  }
  void visitIf(If* curr) {
    printMedium(o, "if");
    if (curr->type.isConcrete()) {
      o << ' ';
      printResultType(o, curr->type, wasm);
    }
  }
  void visitLoop(Loop* curr) {
    printMedium(o, "loop");
    if (curr->name.is()) {
      o << ' ';
      printName(curr->name, o);
    }
    if (curr->type.isConcrete()) {
      o << ' ';
      printResultType(o, curr->type, wasm);
    }
  }
  void visitBreak(Break* curr) {
    if (curr->condition) {
      printMedium(o, "br_if ");
    } else {
      printMedium(o, "br ");
    }
    printName(curr->name, o);
  }
  void visitSwitch(Switch* curr) {
    printMedium(o, "br_table");
    for (auto& t : curr->targets) {
      o << ' ';
      printName(t, o);
    }
    o << ' ';
    printName(curr->default_, o);
  }
  void visitCall(Call* curr) {
    if (curr->isReturn) {
      printMedium(o, "return_call ");
    } else {
      printMedium(o, "call ");
    }
    printName(curr->target, o);
  }
  void visitCallIndirect(CallIndirect* curr) {
    if (curr->isReturn) {
      printMedium(o, "return_call_indirect ");
    } else {
      printMedium(o, "call_indirect ");
    }

    if (features.hasReferenceTypes()) {
      printName(curr->table, o);
      o << ' ';
    }

    o << '(';
    printMinor(o, "type ");
    TypeNamePrinter(o, wasm).print(HeapType(curr->sig));
    o << ')';
  }
  void visitLocalGet(LocalGet* curr) {
    printMedium(o, "local.get ");
    printLocal(curr->index, currFunction, o);
  }
  void visitLocalSet(LocalSet* curr) {
    if (curr->isTee()) {
      printMedium(o, "local.tee ");
    } else {
      printMedium(o, "local.set ");
    }
    printLocal(curr->index, currFunction, o);
  }
  void visitGlobalGet(GlobalGet* curr) {
    printMedium(o, "global.get ");
    printName(curr->name, o);
  }
  void visitGlobalSet(GlobalSet* curr) {
    printMedium(o, "global.set ");
    printName(curr->name, o);
  }
  void visitLoad(Load* curr) {
    prepareColor(o) << forceConcrete(curr->type);
    if (curr->isAtomic) {
      o << ".atomic";
    }
    o << ".load";
    if (curr->type != Type::unreachable &&
        curr->bytes < curr->type.getByteSize()) {
      if (curr->bytes == 1) {
        o << '8';
      } else if (curr->bytes == 2) {
        o << "16";
      } else if (curr->bytes == 4) {
        o << "32";
      } else {
        abort();
      }
      o << (curr->signed_ ? "_s" : "_u");
    }
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
    if (curr->align != curr->bytes) {
      o << " align=" << curr->align;
    }
  }
  void visitStore(Store* curr) {
    prepareColor(o) << forceConcrete(curr->valueType);
    if (curr->isAtomic) {
      o << ".atomic";
    }
    o << ".store";
    if (curr->bytes < 4 || (curr->valueType == Type::i64 && curr->bytes < 8)) {
      if (curr->bytes == 1) {
        o << '8';
      } else if (curr->bytes == 2) {
        o << "16";
      } else if (curr->bytes == 4) {
        o << "32";
      } else {
        abort();
      }
    }
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
    if (curr->align != curr->bytes) {
      o << " align=" << curr->align;
    }
  }
  static void printRMWSize(std::ostream& o, Type type, uint8_t bytes) {
    prepareColor(o) << forceConcrete(type) << ".atomic.rmw";
    if (type != Type::unreachable && bytes != type.getByteSize()) {
      if (bytes == 1) {
        o << '8';
      } else if (bytes == 2) {
        o << "16";
      } else if (bytes == 4) {
        o << "32";
      } else {
        WASM_UNREACHABLE("invalid RMW byte length");
      }
    }
    o << '.';
  }
  void visitAtomicRMW(AtomicRMW* curr) {
    prepareColor(o);
    printRMWSize(o, curr->type, curr->bytes);
    switch (curr->op) {
      case RMWAdd:
        o << "add";
        break;
      case RMWSub:
        o << "sub";
        break;
      case RMWAnd:
        o << "and";
        break;
      case RMWOr:
        o << "or";
        break;
      case RMWXor:
        o << "xor";
        break;
      case RMWXchg:
        o << "xchg";
        break;
    }
    if (curr->type != Type::unreachable &&
        curr->bytes != curr->type.getByteSize()) {
      o << "_u";
    }
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
  }
  void visitAtomicCmpxchg(AtomicCmpxchg* curr) {
    prepareColor(o);
    printRMWSize(o, curr->type, curr->bytes);
    o << "cmpxchg";
    if (curr->type != Type::unreachable &&
        curr->bytes != curr->type.getByteSize()) {
      o << "_u";
    }
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
  }
  void visitAtomicWait(AtomicWait* curr) {
    prepareColor(o);
    Type type = forceConcrete(curr->expectedType);
    assert(type == Type::i32 || type == Type::i64);
    o << "memory.atomic.wait" << (type == Type::i32 ? "32" : "64");
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
  }
  void visitAtomicNotify(AtomicNotify* curr) {
    printMedium(o, "memory.atomic.notify");
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
  }
  void visitAtomicFence(AtomicFence* curr) { printMedium(o, "atomic.fence"); }
  void visitSIMDExtract(SIMDExtract* curr) {
    prepareColor(o);
    switch (curr->op) {
      case ExtractLaneSVecI8x16:
        o << "i8x16.extract_lane_s";
        break;
      case ExtractLaneUVecI8x16:
        o << "i8x16.extract_lane_u";
        break;
      case ExtractLaneSVecI16x8:
        o << "i16x8.extract_lane_s";
        break;
      case ExtractLaneUVecI16x8:
        o << "i16x8.extract_lane_u";
        break;
      case ExtractLaneVecI32x4:
        o << "i32x4.extract_lane";
        break;
      case ExtractLaneVecI64x2:
        o << "i64x2.extract_lane";
        break;
      case ExtractLaneVecF32x4:
        o << "f32x4.extract_lane";
        break;
      case ExtractLaneVecF64x2:
        o << "f64x2.extract_lane";
        break;
    }
    restoreNormalColor(o);
    o << " " << int(curr->index);
  }
  void visitSIMDReplace(SIMDReplace* curr) {
    prepareColor(o);
    switch (curr->op) {
      case ReplaceLaneVecI8x16:
        o << "i8x16.replace_lane";
        break;
      case ReplaceLaneVecI16x8:
        o << "i16x8.replace_lane";
        break;
      case ReplaceLaneVecI32x4:
        o << "i32x4.replace_lane";
        break;
      case ReplaceLaneVecI64x2:
        o << "i64x2.replace_lane";
        break;
      case ReplaceLaneVecF32x4:
        o << "f32x4.replace_lane";
        break;
      case ReplaceLaneVecF64x2:
        o << "f64x2.replace_lane";
        break;
    }
    restoreNormalColor(o);
    o << " " << int(curr->index);
  }
  void visitSIMDShuffle(SIMDShuffle* curr) {
    prepareColor(o);
    o << "i8x16.shuffle";
    restoreNormalColor(o);
    for (uint8_t mask_index : curr->mask) {
      o << " " << std::to_string(mask_index);
    }
  }
  void visitSIMDTernary(SIMDTernary* curr) {
    prepareColor(o);
    switch (curr->op) {
      case Bitselect:
        o << "v128.bitselect";
        break;
    }
    restoreNormalColor(o);
  }
  void visitSIMDShift(SIMDShift* curr) {
    prepareColor(o);
    switch (curr->op) {
      case ShlVecI8x16:
        o << "i8x16.shl";
        break;
      case ShrSVecI8x16:
        o << "i8x16.shr_s";
        break;
      case ShrUVecI8x16:
        o << "i8x16.shr_u";
        break;
      case ShlVecI16x8:
        o << "i16x8.shl";
        break;
      case ShrSVecI16x8:
        o << "i16x8.shr_s";
        break;
      case ShrUVecI16x8:
        o << "i16x8.shr_u";
        break;
      case ShlVecI32x4:
        o << "i32x4.shl";
        break;
      case ShrSVecI32x4:
        o << "i32x4.shr_s";
        break;
      case ShrUVecI32x4:
        o << "i32x4.shr_u";
        break;
      case ShlVecI64x2:
        o << "i64x2.shl";
        break;
      case ShrSVecI64x2:
        o << "i64x2.shr_s";
        break;
      case ShrUVecI64x2:
        o << "i64x2.shr_u";
        break;
    }
    restoreNormalColor(o);
  }
  void visitSIMDLoad(SIMDLoad* curr) {
    prepareColor(o);
    switch (curr->op) {
      case Load8SplatVec128:
        o << "v128.load8_splat";
        break;
      case Load16SplatVec128:
        o << "v128.load16_splat";
        break;
      case Load32SplatVec128:
        o << "v128.load32_splat";
        break;
      case Load64SplatVec128:
        o << "v128.load64_splat";
        break;
      case Load8x8SVec128:
        o << "v128.load8x8_s";
        break;
      case Load8x8UVec128:
        o << "v128.load8x8_u";
        break;
      case Load16x4SVec128:
        o << "v128.load16x4_s";
        break;
      case Load16x4UVec128:
        o << "v128.load16x4_u";
        break;
      case Load32x2SVec128:
        o << "v128.load32x2_s";
        break;
      case Load32x2UVec128:
        o << "v128.load32x2_u";
        break;
      case Load32ZeroVec128:
        o << "v128.load32_zero";
        break;
      case Load64ZeroVec128:
        o << "v128.load64_zero";
        break;
    }
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
    if (curr->align != curr->getMemBytes()) {
      o << " align=" << curr->align;
    }
  }
  void visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
    prepareColor(o);
    switch (curr->op) {
      case Load8LaneVec128:
        o << "v128.load8_lane";
        break;
      case Load16LaneVec128:
        o << "v128.load16_lane";
        break;
      case Load32LaneVec128:
        o << "v128.load32_lane";
        break;
      case Load64LaneVec128:
        o << "v128.load64_lane";
        break;
      case Store8LaneVec128:
        o << "v128.store8_lane";
        break;
      case Store16LaneVec128:
        o << "v128.store16_lane";
        break;
      case Store32LaneVec128:
        o << "v128.store32_lane";
        break;
      case Store64LaneVec128:
        o << "v128.store64_lane";
        break;
    }
    restoreNormalColor(o);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
    if (curr->align != curr->getMemBytes()) {
      o << " align=" << curr->align;
    }
    o << " " << int(curr->index);
  }
  void visitMemoryInit(MemoryInit* curr) {
    prepareColor(o);
    o << "memory.init";
    restoreNormalColor(o);
    o << ' ' << curr->segment;
  }
  void visitDataDrop(DataDrop* curr) {
    prepareColor(o);
    o << "data.drop";
    restoreNormalColor(o);
    o << ' ' << curr->segment;
  }
  void visitMemoryCopy(MemoryCopy* curr) {
    prepareColor(o);
    o << "memory.copy";
    restoreNormalColor(o);
  }
  void visitMemoryFill(MemoryFill* curr) {
    prepareColor(o);
    o << "memory.fill";
    restoreNormalColor(o);
  }
  void visitConst(Const* curr) {
    o << curr->value.type << ".const " << curr->value;
  }
  void visitUnary(Unary* curr) {
    prepareColor(o);
    switch (curr->op) {
      case ClzInt32:
        o << "i32.clz";
        break;
      case CtzInt32:
        o << "i32.ctz";
        break;
      case PopcntInt32:
        o << "i32.popcnt";
        break;
      case EqZInt32:
        o << "i32.eqz";
        break;
      case ClzInt64:
        o << "i64.clz";
        break;
      case CtzInt64:
        o << "i64.ctz";
        break;
      case PopcntInt64:
        o << "i64.popcnt";
        break;
      case EqZInt64:
        o << "i64.eqz";
        break;
      case NegFloat32:
        o << "f32.neg";
        break;
      case AbsFloat32:
        o << "f32.abs";
        break;
      case CeilFloat32:
        o << "f32.ceil";
        break;
      case FloorFloat32:
        o << "f32.floor";
        break;
      case TruncFloat32:
        o << "f32.trunc";
        break;
      case NearestFloat32:
        o << "f32.nearest";
        break;
      case SqrtFloat32:
        o << "f32.sqrt";
        break;
      case NegFloat64:
        o << "f64.neg";
        break;
      case AbsFloat64:
        o << "f64.abs";
        break;
      case CeilFloat64:
        o << "f64.ceil";
        break;
      case FloorFloat64:
        o << "f64.floor";
        break;
      case TruncFloat64:
        o << "f64.trunc";
        break;
      case NearestFloat64:
        o << "f64.nearest";
        break;
      case SqrtFloat64:
        o << "f64.sqrt";
        break;
      case ExtendSInt32:
        o << "i64.extend_i32_s";
        break;
      case ExtendUInt32:
        o << "i64.extend_i32_u";
        break;
      case WrapInt64:
        o << "i32.wrap_i64";
        break;
      case TruncSFloat32ToInt32:
        o << "i32.trunc_f32_s";
        break;
      case TruncSFloat32ToInt64:
        o << "i64.trunc_f32_s";
        break;
      case TruncUFloat32ToInt32:
        o << "i32.trunc_f32_u";
        break;
      case TruncUFloat32ToInt64:
        o << "i64.trunc_f32_u";
        break;
      case TruncSFloat64ToInt32:
        o << "i32.trunc_f64_s";
        break;
      case TruncSFloat64ToInt64:
        o << "i64.trunc_f64_s";
        break;
      case TruncUFloat64ToInt32:
        o << "i32.trunc_f64_u";
        break;
      case TruncUFloat64ToInt64:
        o << "i64.trunc_f64_u";
        break;
      case ReinterpretFloat32:
        o << "i32.reinterpret_f32";
        break;
      case ReinterpretFloat64:
        o << "i64.reinterpret_f64";
        break;
      case ConvertUInt32ToFloat32:
        o << "f32.convert_i32_u";
        break;
      case ConvertUInt32ToFloat64:
        o << "f64.convert_i32_u";
        break;
      case ConvertSInt32ToFloat32:
        o << "f32.convert_i32_s";
        break;
      case ConvertSInt32ToFloat64:
        o << "f64.convert_i32_s";
        break;
      case ConvertUInt64ToFloat32:
        o << "f32.convert_i64_u";
        break;
      case ConvertUInt64ToFloat64:
        o << "f64.convert_i64_u";
        break;
      case ConvertSInt64ToFloat32:
        o << "f32.convert_i64_s";
        break;
      case ConvertSInt64ToFloat64:
        o << "f64.convert_i64_s";
        break;
      case PromoteFloat32:
        o << "f64.promote_f32";
        break;
      case DemoteFloat64:
        o << "f32.demote_f64";
        break;
      case ReinterpretInt32:
        o << "f32.reinterpret_i32";
        break;
      case ReinterpretInt64:
        o << "f64.reinterpret_i64";
        break;
      case ExtendS8Int32:
        o << "i32.extend8_s";
        break;
      case ExtendS16Int32:
        o << "i32.extend16_s";
        break;
      case ExtendS8Int64:
        o << "i64.extend8_s";
        break;
      case ExtendS16Int64:
        o << "i64.extend16_s";
        break;
      case ExtendS32Int64:
        o << "i64.extend32_s";
        break;
      case TruncSatSFloat32ToInt32:
        o << "i32.trunc_sat_f32_s";
        break;
      case TruncSatUFloat32ToInt32:
        o << "i32.trunc_sat_f32_u";
        break;
      case TruncSatSFloat64ToInt32:
        o << "i32.trunc_sat_f64_s";
        break;
      case TruncSatUFloat64ToInt32:
        o << "i32.trunc_sat_f64_u";
        break;
      case TruncSatSFloat32ToInt64:
        o << "i64.trunc_sat_f32_s";
        break;
      case TruncSatUFloat32ToInt64:
        o << "i64.trunc_sat_f32_u";
        break;
      case TruncSatSFloat64ToInt64:
        o << "i64.trunc_sat_f64_s";
        break;
      case TruncSatUFloat64ToInt64:
        o << "i64.trunc_sat_f64_u";
        break;
      case SplatVecI8x16:
        o << "i8x16.splat";
        break;
      case SplatVecI16x8:
        o << "i16x8.splat";
        break;
      case SplatVecI32x4:
        o << "i32x4.splat";
        break;
      case SplatVecI64x2:
        o << "i64x2.splat";
        break;
      case SplatVecF32x4:
        o << "f32x4.splat";
        break;
      case SplatVecF64x2:
        o << "f64x2.splat";
        break;
      case NotVec128:
        o << "v128.not";
        break;
      case AnyTrueVec128:
        o << "v128.any_true";
        break;
      case AbsVecI8x16:
        o << "i8x16.abs";
        break;
      case NegVecI8x16:
        o << "i8x16.neg";
        break;
      case AllTrueVecI8x16:
        o << "i8x16.all_true";
        break;
      case BitmaskVecI8x16:
        o << "i8x16.bitmask";
        break;
      case PopcntVecI8x16:
        o << "i8x16.popcnt";
        break;
      case AbsVecI16x8:
        o << "i16x8.abs";
        break;
      case NegVecI16x8:
        o << "i16x8.neg";
        break;
      case AllTrueVecI16x8:
        o << "i16x8.all_true";
        break;
      case BitmaskVecI16x8:
        o << "i16x8.bitmask";
        break;
      case AbsVecI32x4:
        o << "i32x4.abs";
        break;
      case NegVecI32x4:
        o << "i32x4.neg";
        break;
      case AllTrueVecI32x4:
        o << "i32x4.all_true";
        break;
      case BitmaskVecI32x4:
        o << "i32x4.bitmask";
        break;
      case AbsVecI64x2:
        o << "i64x2.abs";
        break;
      case NegVecI64x2:
        o << "i64x2.neg";
        break;
      case AllTrueVecI64x2:
        o << "i64x2.all_true";
        break;
      case BitmaskVecI64x2:
        o << "i64x2.bitmask";
        break;
      case AbsVecF32x4:
        o << "f32x4.abs";
        break;
      case NegVecF32x4:
        o << "f32x4.neg";
        break;
      case SqrtVecF32x4:
        o << "f32x4.sqrt";
        break;
      case CeilVecF32x4:
        o << "f32x4.ceil";
        break;
      case FloorVecF32x4:
        o << "f32x4.floor";
        break;
      case TruncVecF32x4:
        o << "f32x4.trunc";
        break;
      case NearestVecF32x4:
        o << "f32x4.nearest";
        break;
      case AbsVecF64x2:
        o << "f64x2.abs";
        break;
      case NegVecF64x2:
        o << "f64x2.neg";
        break;
      case SqrtVecF64x2:
        o << "f64x2.sqrt";
        break;
      case CeilVecF64x2:
        o << "f64x2.ceil";
        break;
      case FloorVecF64x2:
        o << "f64x2.floor";
        break;
      case TruncVecF64x2:
        o << "f64x2.trunc";
        break;
      case NearestVecF64x2:
        o << "f64x2.nearest";
        break;
      case ExtAddPairwiseSVecI8x16ToI16x8:
        o << "i16x8.extadd_pairwise_i8x16_s";
        break;
      case ExtAddPairwiseUVecI8x16ToI16x8:
        o << "i16x8.extadd_pairwise_i8x16_u";
        break;
      case ExtAddPairwiseSVecI16x8ToI32x4:
        o << "i32x4.extadd_pairwise_i16x8_s";
        break;
      case ExtAddPairwiseUVecI16x8ToI32x4:
        o << "i32x4.extadd_pairwise_i16x8_u";
        break;
      case TruncSatSVecF32x4ToVecI32x4:
        o << "i32x4.trunc_sat_f32x4_s";
        break;
      case TruncSatUVecF32x4ToVecI32x4:
        o << "i32x4.trunc_sat_f32x4_u";
        break;
      case ConvertSVecI32x4ToVecF32x4:
        o << "f32x4.convert_i32x4_s";
        break;
      case ConvertUVecI32x4ToVecF32x4:
        o << "f32x4.convert_i32x4_u";
        break;
      case ExtendLowSVecI8x16ToVecI16x8:
        o << "i16x8.extend_low_i8x16_s";
        break;
      case ExtendHighSVecI8x16ToVecI16x8:
        o << "i16x8.extend_high_i8x16_s";
        break;
      case ExtendLowUVecI8x16ToVecI16x8:
        o << "i16x8.extend_low_i8x16_u";
        break;
      case ExtendHighUVecI8x16ToVecI16x8:
        o << "i16x8.extend_high_i8x16_u";
        break;
      case ExtendLowSVecI16x8ToVecI32x4:
        o << "i32x4.extend_low_i16x8_s";
        break;
      case ExtendHighSVecI16x8ToVecI32x4:
        o << "i32x4.extend_high_i16x8_s";
        break;
      case ExtendLowUVecI16x8ToVecI32x4:
        o << "i32x4.extend_low_i16x8_u";
        break;
      case ExtendHighUVecI16x8ToVecI32x4:
        o << "i32x4.extend_high_i16x8_u";
        break;
      case ExtendLowSVecI32x4ToVecI64x2:
        o << "i64x2.extend_low_i32x4_s";
        break;
      case ExtendHighSVecI32x4ToVecI64x2:
        o << "i64x2.extend_high_i32x4_s";
        break;
      case ExtendLowUVecI32x4ToVecI64x2:
        o << "i64x2.extend_low_i32x4_u";
        break;
      case ExtendHighUVecI32x4ToVecI64x2:
        o << "i64x2.extend_high_i32x4_u";
        break;
      case ConvertLowSVecI32x4ToVecF64x2:
        o << "f64x2.convert_low_i32x4_s";
        break;
      case ConvertLowUVecI32x4ToVecF64x2:
        o << "f64x2.convert_low_i32x4_u";
        break;
      case TruncSatZeroSVecF64x2ToVecI32x4:
        o << "i32x4.trunc_sat_f64x2_s_zero";
        break;
      case TruncSatZeroUVecF64x2ToVecI32x4:
        o << "i32x4.trunc_sat_f64x2_u_zero";
        break;
      case DemoteZeroVecF64x2ToVecF32x4:
        o << "f32x4.demote_f64x2_zero";
        break;
      case PromoteLowVecF32x4ToVecF64x2:
        o << "f64x2.promote_low_f32x4";
        break;
      case InvalidUnary:
        WASM_UNREACHABLE("unvalid unary operator");
    }
    restoreNormalColor(o);
  }
  void visitBinary(Binary* curr) {
    prepareColor(o);
    switch (curr->op) {
      case AddInt32:
        o << "i32.add";
        break;
      case SubInt32:
        o << "i32.sub";
        break;
      case MulInt32:
        o << "i32.mul";
        break;
      case DivSInt32:
        o << "i32.div_s";
        break;
      case DivUInt32:
        o << "i32.div_u";
        break;
      case RemSInt32:
        o << "i32.rem_s";
        break;
      case RemUInt32:
        o << "i32.rem_u";
        break;
      case AndInt32:
        o << "i32.and";
        break;
      case OrInt32:
        o << "i32.or";
        break;
      case XorInt32:
        o << "i32.xor";
        break;
      case ShlInt32:
        o << "i32.shl";
        break;
      case ShrUInt32:
        o << "i32.shr_u";
        break;
      case ShrSInt32:
        o << "i32.shr_s";
        break;
      case RotLInt32:
        o << "i32.rotl";
        break;
      case RotRInt32:
        o << "i32.rotr";
        break;
      case EqInt32:
        o << "i32.eq";
        break;
      case NeInt32:
        o << "i32.ne";
        break;
      case LtSInt32:
        o << "i32.lt_s";
        break;
      case LtUInt32:
        o << "i32.lt_u";
        break;
      case LeSInt32:
        o << "i32.le_s";
        break;
      case LeUInt32:
        o << "i32.le_u";
        break;
      case GtSInt32:
        o << "i32.gt_s";
        break;
      case GtUInt32:
        o << "i32.gt_u";
        break;
      case GeSInt32:
        o << "i32.ge_s";
        break;
      case GeUInt32:
        o << "i32.ge_u";
        break;

      case AddInt64:
        o << "i64.add";
        break;
      case SubInt64:
        o << "i64.sub";
        break;
      case MulInt64:
        o << "i64.mul";
        break;
      case DivSInt64:
        o << "i64.div_s";
        break;
      case DivUInt64:
        o << "i64.div_u";
        break;
      case RemSInt64:
        o << "i64.rem_s";
        break;
      case RemUInt64:
        o << "i64.rem_u";
        break;
      case AndInt64:
        o << "i64.and";
        break;
      case OrInt64:
        o << "i64.or";
        break;
      case XorInt64:
        o << "i64.xor";
        break;
      case ShlInt64:
        o << "i64.shl";
        break;
      case ShrUInt64:
        o << "i64.shr_u";
        break;
      case ShrSInt64:
        o << "i64.shr_s";
        break;
      case RotLInt64:
        o << "i64.rotl";
        break;
      case RotRInt64:
        o << "i64.rotr";
        break;
      case EqInt64:
        o << "i64.eq";
        break;
      case NeInt64:
        o << "i64.ne";
        break;
      case LtSInt64:
        o << "i64.lt_s";
        break;
      case LtUInt64:
        o << "i64.lt_u";
        break;
      case LeSInt64:
        o << "i64.le_s";
        break;
      case LeUInt64:
        o << "i64.le_u";
        break;
      case GtSInt64:
        o << "i64.gt_s";
        break;
      case GtUInt64:
        o << "i64.gt_u";
        break;
      case GeSInt64:
        o << "i64.ge_s";
        break;
      case GeUInt64:
        o << "i64.ge_u";
        break;

      case AddFloat32:
        o << "f32.add";
        break;
      case SubFloat32:
        o << "f32.sub";
        break;
      case MulFloat32:
        o << "f32.mul";
        break;
      case DivFloat32:
        o << "f32.div";
        break;
      case CopySignFloat32:
        o << "f32.copysign";
        break;
      case MinFloat32:
        o << "f32.min";
        break;
      case MaxFloat32:
        o << "f32.max";
        break;
      case EqFloat32:
        o << "f32.eq";
        break;
      case NeFloat32:
        o << "f32.ne";
        break;
      case LtFloat32:
        o << "f32.lt";
        break;
      case LeFloat32:
        o << "f32.le";
        break;
      case GtFloat32:
        o << "f32.gt";
        break;
      case GeFloat32:
        o << "f32.ge";
        break;

      case AddFloat64:
        o << "f64.add";
        break;
      case SubFloat64:
        o << "f64.sub";
        break;
      case MulFloat64:
        o << "f64.mul";
        break;
      case DivFloat64:
        o << "f64.div";
        break;
      case CopySignFloat64:
        o << "f64.copysign";
        break;
      case MinFloat64:
        o << "f64.min";
        break;
      case MaxFloat64:
        o << "f64.max";
        break;
      case EqFloat64:
        o << "f64.eq";
        break;
      case NeFloat64:
        o << "f64.ne";
        break;
      case LtFloat64:
        o << "f64.lt";
        break;
      case LeFloat64:
        o << "f64.le";
        break;
      case GtFloat64:
        o << "f64.gt";
        break;
      case GeFloat64:
        o << "f64.ge";
        break;

      case EqVecI8x16:
        o << "i8x16.eq";
        break;
      case NeVecI8x16:
        o << "i8x16.ne";
        break;
      case LtSVecI8x16:
        o << "i8x16.lt_s";
        break;
      case LtUVecI8x16:
        o << "i8x16.lt_u";
        break;
      case GtSVecI8x16:
        o << "i8x16.gt_s";
        break;
      case GtUVecI8x16:
        o << "i8x16.gt_u";
        break;
      case LeSVecI8x16:
        o << "i8x16.le_s";
        break;
      case LeUVecI8x16:
        o << "i8x16.le_u";
        break;
      case GeSVecI8x16:
        o << "i8x16.ge_s";
        break;
      case GeUVecI8x16:
        o << "i8x16.ge_u";
        break;
      case EqVecI16x8:
        o << "i16x8.eq";
        break;
      case NeVecI16x8:
        o << "i16x8.ne";
        break;
      case LtSVecI16x8:
        o << "i16x8.lt_s";
        break;
      case LtUVecI16x8:
        o << "i16x8.lt_u";
        break;
      case GtSVecI16x8:
        o << "i16x8.gt_s";
        break;
      case GtUVecI16x8:
        o << "i16x8.gt_u";
        break;
      case LeSVecI16x8:
        o << "i16x8.le_s";
        break;
      case LeUVecI16x8:
        o << "i16x8.le_u";
        break;
      case GeSVecI16x8:
        o << "i16x8.ge_s";
        break;
      case GeUVecI16x8:
        o << "i16x8.ge_u";
        break;
      case EqVecI32x4:
        o << "i32x4.eq";
        break;
      case NeVecI32x4:
        o << "i32x4.ne";
        break;
      case LtSVecI32x4:
        o << "i32x4.lt_s";
        break;
      case LtUVecI32x4:
        o << "i32x4.lt_u";
        break;
      case GtSVecI32x4:
        o << "i32x4.gt_s";
        break;
      case GtUVecI32x4:
        o << "i32x4.gt_u";
        break;
      case LeSVecI32x4:
        o << "i32x4.le_s";
        break;
      case LeUVecI32x4:
        o << "i32x4.le_u";
        break;
      case GeSVecI32x4:
        o << "i32x4.ge_s";
        break;
      case GeUVecI32x4:
        o << "i32x4.ge_u";
        break;
      case EqVecI64x2:
        o << "i64x2.eq";
        break;
      case NeVecI64x2:
        o << "i64x2.ne";
        break;
      case LtSVecI64x2:
        o << "i64x2.lt_s";
        break;
      case GtSVecI64x2:
        o << "i64x2.gt_s";
        break;
      case LeSVecI64x2:
        o << "i64x2.le_s";
        break;
      case GeSVecI64x2:
        o << "i64x2.ge_s";
        break;
      case EqVecF32x4:
        o << "f32x4.eq";
        break;
      case NeVecF32x4:
        o << "f32x4.ne";
        break;
      case LtVecF32x4:
        o << "f32x4.lt";
        break;
      case GtVecF32x4:
        o << "f32x4.gt";
        break;
      case LeVecF32x4:
        o << "f32x4.le";
        break;
      case GeVecF32x4:
        o << "f32x4.ge";
        break;
      case EqVecF64x2:
        o << "f64x2.eq";
        break;
      case NeVecF64x2:
        o << "f64x2.ne";
        break;
      case LtVecF64x2:
        o << "f64x2.lt";
        break;
      case GtVecF64x2:
        o << "f64x2.gt";
        break;
      case LeVecF64x2:
        o << "f64x2.le";
        break;
      case GeVecF64x2:
        o << "f64x2.ge";
        break;

      case AndVec128:
        o << "v128.and";
        break;
      case OrVec128:
        o << "v128.or";
        break;
      case XorVec128:
        o << "v128.xor";
        break;
      case AndNotVec128:
        o << "v128.andnot";
        break;

      case AddVecI8x16:
        o << "i8x16.add";
        break;
      case AddSatSVecI8x16:
        o << "i8x16.add_sat_s";
        break;
      case AddSatUVecI8x16:
        o << "i8x16.add_sat_u";
        break;
      case SubVecI8x16:
        o << "i8x16.sub";
        break;
      case SubSatSVecI8x16:
        o << "i8x16.sub_sat_s";
        break;
      case SubSatUVecI8x16:
        o << "i8x16.sub_sat_u";
        break;
      case MinSVecI8x16:
        o << "i8x16.min_s";
        break;
      case MinUVecI8x16:
        o << "i8x16.min_u";
        break;
      case MaxSVecI8x16:
        o << "i8x16.max_s";
        break;
      case MaxUVecI8x16:
        o << "i8x16.max_u";
        break;
      case AvgrUVecI8x16:
        o << "i8x16.avgr_u";
        break;
      case AddVecI16x8:
        o << "i16x8.add";
        break;
      case AddSatSVecI16x8:
        o << "i16x8.add_sat_s";
        break;
      case AddSatUVecI16x8:
        o << "i16x8.add_sat_u";
        break;
      case SubVecI16x8:
        o << "i16x8.sub";
        break;
      case SubSatSVecI16x8:
        o << "i16x8.sub_sat_s";
        break;
      case SubSatUVecI16x8:
        o << "i16x8.sub_sat_u";
        break;
      case MulVecI16x8:
        o << "i16x8.mul";
        break;
      case MinSVecI16x8:
        o << "i16x8.min_s";
        break;
      case MinUVecI16x8:
        o << "i16x8.min_u";
        break;
      case MaxSVecI16x8:
        o << "i16x8.max_s";
        break;
      case MaxUVecI16x8:
        o << "i16x8.max_u";
        break;
      case AvgrUVecI16x8:
        o << "i16x8.avgr_u";
        break;
      case Q15MulrSatSVecI16x8:
        o << "i16x8.q15mulr_sat_s";
        break;
      case ExtMulLowSVecI16x8:
        o << "i16x8.extmul_low_i8x16_s";
        break;
      case ExtMulHighSVecI16x8:
        o << "i16x8.extmul_high_i8x16_s";
        break;
      case ExtMulLowUVecI16x8:
        o << "i16x8.extmul_low_i8x16_u";
        break;
      case ExtMulHighUVecI16x8:
        o << "i16x8.extmul_high_i8x16_u";
        break;

      case AddVecI32x4:
        o << "i32x4.add";
        break;
      case SubVecI32x4:
        o << "i32x4.sub";
        break;
      case MulVecI32x4:
        o << "i32x4.mul";
        break;
      case MinSVecI32x4:
        o << "i32x4.min_s";
        break;
      case MinUVecI32x4:
        o << "i32x4.min_u";
        break;
      case MaxSVecI32x4:
        o << "i32x4.max_s";
        break;
      case MaxUVecI32x4:
        o << "i32x4.max_u";
        break;
      case DotSVecI16x8ToVecI32x4:
        o << "i32x4.dot_i16x8_s";
        break;
      case ExtMulLowSVecI32x4:
        o << "i32x4.extmul_low_i16x8_s";
        break;
      case ExtMulHighSVecI32x4:
        o << "i32x4.extmul_high_i16x8_s";
        break;
      case ExtMulLowUVecI32x4:
        o << "i32x4.extmul_low_i16x8_u";
        break;
      case ExtMulHighUVecI32x4:
        o << "i32x4.extmul_high_i16x8_u";
        break;

      case AddVecI64x2:
        o << "i64x2.add";
        break;
      case SubVecI64x2:
        o << "i64x2.sub";
        break;
      case MulVecI64x2:
        o << "i64x2.mul";
        break;
      case ExtMulLowSVecI64x2:
        o << "i64x2.extmul_low_i32x4_s";
        break;
      case ExtMulHighSVecI64x2:
        o << "i64x2.extmul_high_i32x4_s";
        break;
      case ExtMulLowUVecI64x2:
        o << "i64x2.extmul_low_i32x4_u";
        break;
      case ExtMulHighUVecI64x2:
        o << "i64x2.extmul_high_i32x4_u";
        break;

      case AddVecF32x4:
        o << "f32x4.add";
        break;
      case SubVecF32x4:
        o << "f32x4.sub";
        break;
      case MulVecF32x4:
        o << "f32x4.mul";
        break;
      case DivVecF32x4:
        o << "f32x4.div";
        break;
      case MinVecF32x4:
        o << "f32x4.min";
        break;
      case MaxVecF32x4:
        o << "f32x4.max";
        break;
      case PMinVecF32x4:
        o << "f32x4.pmin";
        break;
      case PMaxVecF32x4:
        o << "f32x4.pmax";
        break;
      case AddVecF64x2:
        o << "f64x2.add";
        break;
      case SubVecF64x2:
        o << "f64x2.sub";
        break;
      case MulVecF64x2:
        o << "f64x2.mul";
        break;
      case DivVecF64x2:
        o << "f64x2.div";
        break;
      case MinVecF64x2:
        o << "f64x2.min";
        break;
      case MaxVecF64x2:
        o << "f64x2.max";
        break;
      case PMinVecF64x2:
        o << "f64x2.pmin";
        break;
      case PMaxVecF64x2:
        o << "f64x2.pmax";
        break;

      case NarrowSVecI16x8ToVecI8x16:
        o << "i8x16.narrow_i16x8_s";
        break;
      case NarrowUVecI16x8ToVecI8x16:
        o << "i8x16.narrow_i16x8_u";
        break;
      case NarrowSVecI32x4ToVecI16x8:
        o << "i16x8.narrow_i32x4_s";
        break;
      case NarrowUVecI32x4ToVecI16x8:
        o << "i16x8.narrow_i32x4_u";
        break;

      case SwizzleVec8x16:
        o << "i8x16.swizzle";
        break;

      case InvalidBinary:
        WASM_UNREACHABLE("unvalid binary operator");
    }
    restoreNormalColor(o);
  }
  void visitSelect(Select* curr) {
    prepareColor(o) << "select";
    restoreNormalColor(o);
    if (curr->type.isRef()) {
      o << ' ';
      printResultType(o, curr->type, wasm);
    }
  }
  void visitDrop(Drop* curr) { printMedium(o, "drop"); }
  void visitReturn(Return* curr) { printMedium(o, "return"); }
  void visitMemorySize(MemorySize* curr) { printMedium(o, "memory.size"); }
  void visitMemoryGrow(MemoryGrow* curr) { printMedium(o, "memory.grow"); }
  void visitTableGet(TableGet* curr) {
    printMedium(o, "table.get ");
    printName(curr->table, o);
  }
  void visitTableSet(TableSet* curr) {
    printMedium(o, "table.set ");
    printName(curr->table, o) << ' ';
  }
  void visitTableSize(TableSize* curr) {
    printMedium(o, "table.size ");
    printName(curr->table, o) << ' ';
  }
  void visitTableGrow(TableGrow* curr) {
    printMedium(o, "table.grow ");
    printName(curr->table, o) << ' ';
  }
  void visitTableFill(TableFill* curr) {
    printMedium(o, "table.fill ");
    printName(curr->table, o) << ' ';
  }
  void visitTableCopy(TableCopy* curr) {
    printMedium(o, "table.copy ");
    printName(curr->destTable, o) << ' ';
    printName(curr->srcTable, o) << ' ';
  }
  void visitTableInit(TableInit* curr) {
    printMedium(o, "table.init") << ' ';
    printName(curr->table, o) << ' ';
    printName(curr->segment, o) << ' ';
  }
  void visitElemDrop(ElemDrop* curr) {
    printMedium(o, "elem.drop ");
    printName(curr->segment, o);
  }

  void visitRefNull(RefNull* curr) {
    printMedium(o, "ref.null ");
    TypeNamePrinter(o, wasm).print(curr->type.getHeapType());
  }
  void visitRefIs(RefIs* curr) {
    switch (curr->op) {
      case RefIsNull:
        printMedium(o, "ref.is_null");
        break;
      case RefIsFunc:
        printMedium(o, "ref.is_func");
        break;
      case RefIsData:
        printMedium(o, "ref.is_data");
        break;
      case RefIsI31:
        printMedium(o, "ref.is_i31");
        break;
      default:
        WASM_UNREACHABLE("unimplemented ref.is_*");
    }
  }
  void visitRefFunc(RefFunc* curr) {
    printMedium(o, "ref.func ");
    printName(curr->func, o);
  }
  void visitRefEq(RefEq* curr) { printMedium(o, "ref.eq"); }
  void visitTry(Try* curr) {
    printMedium(o, "try");
    if (curr->name.is()) {
      o << ' ';
      printName(curr->name, o);
    }
    if (curr->type.isConcrete()) {
      o << ' ';
      printResultType(o, curr->type, wasm);
    }
  }
  void visitThrow(Throw* curr) {
    printMedium(o, "throw ");
    printName(curr->event, o);
  }
  void visitRethrow(Rethrow* curr) {
    printMedium(o, "rethrow ");
    printName(curr->target, o);
  }
  void visitNop(Nop* curr) { printMinor(o, "nop"); }
  void visitUnreachable(Unreachable* curr) { printMinor(o, "unreachable"); }
  void visitPop(Pop* curr) {
    prepareColor(o) << "pop";
    for (auto type : curr->type) {
      assert(type.isBasic() && "TODO: print and parse compound types");
      o << " " << type;
    }
    restoreNormalColor(o);
  }
  void visitTupleMake(TupleMake* curr) { printMedium(o, "tuple.make"); }
  void visitTupleExtract(TupleExtract* curr) {
    printMedium(o, "tuple.extract ");
    o << curr->index;
  }
  void visitI31New(I31New* curr) { printMedium(o, "i31.new"); }
  void visitI31Get(I31Get* curr) {
    printMedium(o, curr->signed_ ? "i31.get_s" : "i31.get_u");
  }
  void visitCallRef(CallRef* curr) {
    if (curr->isReturn) {
      printMedium(o, "return_call_ref");
    } else {
      printMedium(o, "call_ref");
    }
  }
  void visitRefTest(RefTest* curr) { printMedium(o, "ref.test"); }
  void visitRefCast(RefCast* curr) { printMedium(o, "ref.cast"); }
  void visitBrOn(BrOn* curr) {
    switch (curr->op) {
      case BrOnNull:
        printMedium(o, "br_on_null ");
        break;
      case BrOnNonNull:
        printMedium(o, "br_on_non_null ");
        break;
      case BrOnCast:
        printMedium(o, "br_on_cast ");
        break;
      case BrOnCastFail:
        printMedium(o, "br_on_cast_fail ");
        break;
      case BrOnFunc:
        printMedium(o, "br_on_func ");
        break;
      case BrOnNonFunc:
        printMedium(o, "br_on_non_func ");
        break;
      case BrOnData:
        printMedium(o, "br_on_data ");
        break;
      case BrOnNonData:
        printMedium(o, "br_on_non_data ");
        break;
      case BrOnI31:
        printMedium(o, "br_on_i31 ");
        break;
      case BrOnNonI31:
        printMedium(o, "br_on_non_i31 ");
        break;
      default:
        WASM_UNREACHABLE("invalid ref.is_*");
    }
    printName(curr->name, o);
  }
  void visitRttCanon(RttCanon* curr) {
    printMedium(o, "rtt.canon ");
    TypeNamePrinter(o, wasm).print(curr->type.getRtt().heapType);
  }
  void visitRttSub(RttSub* curr) {
    if (curr->fresh) {
      printMedium(o, "rtt.fresh_sub ");
    } else {
      printMedium(o, "rtt.sub ");
    }
    TypeNamePrinter(o, wasm).print(curr->type.getRtt().heapType);
  }
  void visitStructNew(StructNew* curr) {
    printMedium(o, "struct.new_");
    if (curr->isWithDefault()) {
      o << "default_";
    }
    o << "with_rtt ";
    TypeNamePrinter(o, wasm).print(curr->rtt->type.getHeapType());
  }
  void printUnreachableReplacement() {
    // If we cannot print a valid unreachable instruction (say, a struct.get,
    // where if the ref is unreachable, we don't know what heap type to print),
    // then print the children in a block, which is good enough as this
    // instruction is never reached anyhow.
    printMedium(o, "block");
  }
  void printFieldName(HeapType type, Index index) {
    processFieldName(wasm, type, index, [&](Name name) {
      if (name.is()) {
        o << '$' << name;
      } else {
        o << index;
      }
    });
  }
  void visitStructGet(StructGet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement();
      return;
    }
    auto heapType = curr->ref->type.getHeapType();
    const auto& field = heapType.getStruct().fields[curr->index];
    if (field.type == Type::i32 && field.packedType != Field::not_packed) {
      if (curr->signed_) {
        printMedium(o, "struct.get_s ");
      } else {
        printMedium(o, "struct.get_u ");
      }
    } else {
      printMedium(o, "struct.get ");
    }
    TypeNamePrinter(o, wasm).print(heapType);
    o << ' ';
    printFieldName(heapType, curr->index);
  }
  void visitStructSet(StructSet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement();
      return;
    }
    printMedium(o, "struct.set ");
    auto heapType = curr->ref->type.getHeapType();
    TypeNamePrinter(o, wasm).print(heapType);
    o << ' ';
    printFieldName(heapType, curr->index);
  }
  void visitArrayNew(ArrayNew* curr) {
    printMedium(o, "array.new_");
    if (curr->isWithDefault()) {
      o << "default_";
    }
    o << "with_rtt ";
    TypeNamePrinter(o, wasm).print(curr->rtt->type.getHeapType());
  }
  void visitArrayGet(ArrayGet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement();
      return;
    }
    const auto& element = curr->ref->type.getHeapType().getArray().element;
    if (element.type == Type::i32 && element.packedType != Field::not_packed) {
      if (curr->signed_) {
        printMedium(o, "array.get_s ");
      } else {
        printMedium(o, "array.get_u ");
      }
    } else {
      printMedium(o, "array.get ");
    }
    TypeNamePrinter(o, wasm).print(curr->ref->type.getHeapType());
  }
  void visitArraySet(ArraySet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement();
      return;
    }
    printMedium(o, "array.set ");
    TypeNamePrinter(o, wasm).print(curr->ref->type.getHeapType());
  }
  void visitArrayLen(ArrayLen* curr) {
    printMedium(o, "array.len ");
    TypeNamePrinter(o, wasm).print(curr->ref->type.getHeapType());
  }
  void visitArrayCopy(ArrayCopy* curr) {
    if (curr->srcRef->type == Type::unreachable ||
        curr->destRef->type == Type::unreachable) {
      printUnreachableReplacement();
      return;
    }
    printMedium(o, "array.copy ");
    TypeNamePrinter(o, wasm).print(curr->destRef->type.getHeapType());
    o << ' ';
    TypeNamePrinter(o, wasm).print(curr->srcRef->type.getHeapType());
  }
  void visitRefAs(RefAs* curr) {
    switch (curr->op) {
      case RefAsNonNull:
        printMedium(o, "ref.as_non_null");
        break;
      case RefAsFunc:
        printMedium(o, "ref.as_func");
        break;
      case RefAsData:
        printMedium(o, "ref.as_data");
        break;
      case RefAsI31:
        printMedium(o, "ref.as_i31");
        break;
      default:
        WASM_UNREACHABLE("invalid ref.is_*");
    }
  }
};

// Prints an expression in s-expr format, including both the
// internal contents and the nested children.
struct PrintSExpression : public UnifiedExpressionVisitor<PrintSExpression> {
  std::ostream& o;
  unsigned indent = 0;

  bool minify;
  const char* maybeSpace;
  const char* maybeNewLine;

  bool full = false;    // whether to not elide nodes in output when possible
                        // (like implicit blocks) and to emit types
  bool stackIR = false; // whether to print stack IR if it is present
                        // (if false, and Stack IR is there, we just
                        // note it exists)

  Module* currModule = nullptr;
  Function* currFunction = nullptr;
  Function::DebugLocation lastPrintedLocation;
  bool debugInfo;

  // Used to print delegate's depth argument when it throws to the caller
  int controlFlowDepth = 0;

  PrintSExpression(std::ostream& o) : o(o) {
    setMinify(false);
    if (!full) {
      full = isFullForced();
    }
  }

  void printDebugLocation(const Function::DebugLocation& location) {
    if (lastPrintedLocation == location) {
      return;
    }
    lastPrintedLocation = location;
    auto fileName = currModule->debugInfoFileNames[location.fileIndex];
    o << ";;@ " << fileName << ":" << location.lineNumber << ":"
      << location.columnNumber << '\n';
    doIndent(o, indent);
  }

  void printDebugLocation(Expression* curr) {
    if (currFunction) {
      // show an annotation, if there is one
      auto& debugLocations = currFunction->debugLocations;
      auto iter = debugLocations.find(curr);
      if (iter != debugLocations.end()) {
        printDebugLocation(iter->second);
      }
      // show a binary position, if there is one
      if (debugInfo) {
        auto iter = currFunction->expressionLocations.find(curr);
        if (iter != currFunction->expressionLocations.end()) {
          Colors::grey(o);
          o << ";; code offset: 0x" << std::hex << iter->second.start
            << std::dec << '\n';
          restoreNormalColor(o);
          doIndent(o, indent);
        }
      }
    }
  }

  // Prints debug info for a delimiter in an expression.
  void printDebugDelimiterLocation(Expression* curr, Index i) {
    if (currFunction && debugInfo) {
      auto iter = currFunction->delimiterLocations.find(curr);
      if (iter != currFunction->delimiterLocations.end()) {
        auto& locations = iter->second;
        Colors::grey(o);
        o << ";; code offset: 0x" << std::hex << locations[i] << std::dec
          << '\n';
        restoreNormalColor(o);
        doIndent(o, indent);
      }
    }
  }

  void printExpressionContents(Expression* curr) {
    if (currModule) {
      PrintExpressionContents(currModule, currFunction, o).visit(curr);
    } else {
      PrintExpressionContents(currFunction, o).visit(curr);
    }
  }

  void visit(Expression* curr) {
    printDebugLocation(curr);
    UnifiedExpressionVisitor<PrintSExpression>::visit(curr);
  }

  void setMinify(bool minify_) {
    minify = minify_;
    maybeSpace = minify ? "" : " ";
    maybeNewLine = minify ? "" : "\n";
  }

  void setFull(bool full_) { full = full_; }

  void setStackIR(bool stackIR_) { stackIR = stackIR_; }

  void setDebugInfo(bool debugInfo_) { debugInfo = debugInfo_; }

  void incIndent() {
    if (minify) {
      return;
    }
    o << '\n';
    indent++;
  }
  void decIndent() {
    if (!minify) {
      assert(indent > 0);
      indent--;
      doIndent(o, indent);
    }
    o << ')';
  }
  void printFullLine(Expression* expression) {
    if (!minify) {
      doIndent(o, indent);
    }
    if (full) {
      o << "[" << expression->type << "] ";
    }
    visit(expression);
    o << maybeNewLine;
  }

  // loop, if, and try can contain implicit blocks. But they are not needed to
  // be printed in some cases.
  void maybePrintImplicitBlock(Expression* curr, bool allowMultipleInsts) {
    auto block = curr->dynCast<Block>();
    if (!full && block && block->name.isNull() &&
        (allowMultipleInsts || block->list.size() == 1)) {
      for (auto expression : block->list) {
        printFullLine(expression);
      }
    } else {
      printFullLine(curr);
    }
  }

  // Generic visitor, overridden only when necessary.
  void visitExpression(Expression* curr) {
    o << '(';
    printExpressionContents(curr);
    auto it = ChildIterator(curr);
    if (!it.children.empty()) {
      incIndent();
      for (auto* child : it) {
        printFullLine(child);
      }
      decIndent();
    } else {
      o << ')';
    }
  }

  void visitBlock(Block* curr) {
    // special-case Block, because Block nesting (in their first element) can be
    // incredibly deep
    std::vector<Block*> stack;
    while (1) {
      if (stack.size() > 0) {
        doIndent(o, indent);
        printDebugLocation(curr);
      }
      stack.push_back(curr);
      if (full) {
        o << "[" << curr->type << "] ";
      }
      o << '(';
      printExpressionContents(curr);
      incIndent();
      if (curr->list.size() > 0 && curr->list[0]->is<Block>()) {
        // recurse into the first element
        curr = curr->list[0]->cast<Block>();
        continue;
      } else {
        break; // that's all we can recurse, start to unwind
      }
    }

    int startControlFlowDepth = controlFlowDepth;
    controlFlowDepth += stack.size();
    auto* top = stack.back();
    while (stack.size() > 0) {
      curr = stack.back();
      stack.pop_back();
      auto& list = curr->list;
      for (size_t i = 0; i < list.size(); i++) {
        if (curr != top && i == 0) {
          // one of the block recursions we already handled
          decIndent();
          if (full) {
            o << " ;; end block";
            auto* child = list[0]->cast<Block>();
            if (child->name.is()) {
              o << ' ' << child->name;
            }
          }
          o << '\n';
          continue;
        }
        printFullLine(list[i]);
      }
    }
    decIndent();
    if (full) {
      o << " ;; end block";
      if (curr->name.is()) {
        o << ' ' << curr->name;
      }
    }
    controlFlowDepth = startControlFlowDepth;
  }
  void visitIf(If* curr) {
    controlFlowDepth++;
    o << '(';
    printExpressionContents(curr);
    incIndent();
    printFullLine(curr->condition);
    maybePrintImplicitBlock(curr->ifTrue, false);
    if (curr->ifFalse) {
      // Note: debug info here is not used as LLVM does not emit ifs, and since
      // LLVM is the main source of DWARF, effectively we never encounter ifs
      // with DWARF.
      printDebugDelimiterLocation(curr, BinaryLocations::Else);
      maybePrintImplicitBlock(curr->ifFalse, false);
    }
    decIndent();
    if (full) {
      o << " ;; end if";
    }
    controlFlowDepth--;
  }
  void visitLoop(Loop* curr) {
    controlFlowDepth++;
    o << '(';
    printExpressionContents(curr);
    incIndent();
    maybePrintImplicitBlock(curr->body, true);
    decIndent();
    if (full) {
      o << " ;; end loop";
      if (curr->name.is()) {
        o << ' ' << curr->name;
      }
    }
    controlFlowDepth--;
  }
  // try-catch-end is written in the folded wat format as
  // (try
  //  (do
  //   ...
  //  )
  //  (catch $e
  //    ...
  //  )
  //  ...
  //  (catch_all
  //    ...
  //  )
  // )
  // The parenthesis wrapping do/catch/catch_all is just a syntax and does not
  // affect nested depths of instructions within.
  //
  // try-delegate is written in the forded format as
  // (try
  //  (do
  //    ...
  //  )
  //  (delegate $label)
  // )
  // When the 'delegate' delegates to the caller, we write the argument as an
  // immediate.
  void visitTry(Try* curr) {
    controlFlowDepth++;
    o << '(';
    printExpressionContents(curr);
    incIndent();
    doIndent(o, indent);
    o << '(';
    printMedium(o, "do");
    incIndent();
    maybePrintImplicitBlock(curr->body, true);
    decIndent();
    o << "\n";
    for (size_t i = 0; i < curr->catchEvents.size(); i++) {
      doIndent(o, indent);
      printDebugDelimiterLocation(curr, i);
      o << '(';
      printMedium(o, "catch ");
      printName(curr->catchEvents[i], o);
      incIndent();
      maybePrintImplicitBlock(curr->catchBodies[i], true);
      decIndent();
      o << "\n";
    }
    if (curr->hasCatchAll()) {
      doIndent(o, indent);
      printDebugDelimiterLocation(curr, curr->catchEvents.size());
      o << '(';
      printMedium(o, "catch_all");
      incIndent();
      maybePrintImplicitBlock(curr->catchBodies.back(), true);
      decIndent();
      o << "\n";
    }
    controlFlowDepth--;

    if (curr->isDelegate()) {
      doIndent(o, indent);
      o << '(';
      printMedium(o, "delegate ");
      if (curr->delegateTarget == DELEGATE_CALLER_TARGET) {
        o << controlFlowDepth;
      } else {
        printName(curr->delegateTarget, o);
      }
      o << ")\n";
    }
    decIndent();
    if (full) {
      o << " ;; end try";
    }
  }
  void printUnreachableReplacement(Expression* curr) {
    // See the parallel function in PrintExpressionContents for background.
    //
    // Emit a block with drops of the children.
    o << "(block";
    if (!minify) {
      o << " ;; (replaces something unreachable we can't emit)";
    }
    incIndent();
    for (auto* child : ChildIterator(curr)) {
      Drop drop;
      drop.value = child;
      printFullLine(&drop);
    }
    decIndent();
  }
  void visitStructSet(StructSet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement(curr);
      return;
    }
    visitExpression(curr);
  }
  void visitStructGet(StructGet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement(curr);
      return;
    }
    visitExpression(curr);
  }
  void visitArraySet(ArraySet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement(curr);
      return;
    }
    visitExpression(curr);
  }
  void visitArrayGet(ArrayGet* curr) {
    if (curr->ref->type == Type::unreachable) {
      printUnreachableReplacement(curr);
      return;
    }
    visitExpression(curr);
  }
  // Module-level visitors
  void handleSignature(Signature curr, Name name = Name()) {
    o << "(func";
    if (name.is()) {
      o << " $" << name;
    }
    if (curr.params.size() > 0) {
      o << maybeSpace;
      o << "(param ";
      auto sep = "";
      for (auto type : curr.params) {
        o << sep;
        printType(o, type, currModule);
        sep = " ";
      }
      o << ')';
    }
    if (curr.results.size() > 0) {
      o << maybeSpace;
      o << "(result ";
      auto sep = "";
      for (auto type : curr.results) {
        o << sep;
        printType(o, type, currModule);
        sep = " ";
      }
      o << ')';
    }
    o << ")";
  }
  void handleFieldBody(const Field& field) {
    if (field.mutable_) {
      o << "(mut ";
    }
    if (field.type == Type::i32 && field.packedType != Field::not_packed) {
      if (field.packedType == Field::i8) {
        o << "i8";
      } else if (field.packedType == Field::i16) {
        o << "i16";
      } else {
        WASM_UNREACHABLE("invalid packed type");
      }
    } else {
      printType(o, field.type, currModule);
    }
    if (field.mutable_) {
      o << ')';
    }
  }
  void handleArray(const Array& curr) {
    o << "(array ";
    handleFieldBody(curr.element);
    o << ')';
  }
  void handleStruct(const Struct& curr) {
    auto type = HeapType(curr);
    const auto& fields = curr.fields;
    o << "(struct ";
    auto sep = "";
    for (Index i = 0; i < fields.size(); i++) {
      o << sep << "(field ";
      processFieldName(currModule, type, i, [&](Name name) {
        if (name.is()) {
          o << '$' << name << ' ';
        }
      });
      handleFieldBody(fields[i]);
      o << ')';
      sep = " ";
    }
    o << ')';
  }
  void handleHeapType(HeapType type, Module* module) {
    if (type.isSignature()) {
      handleSignature(type.getSignature());
    } else if (type.isArray()) {
      handleArray(type.getArray());
    } else if (type.isStruct()) {
      handleStruct(type.getStruct());
    } else {
      o << type;
    }
    HeapType super;
    if (type.getSuperType(super)) {
      o << " (extends ";
      TypeNamePrinter(o, module).print(super);
      o << ')';
    }
  }
  void visitExport(Export* curr) {
    o << '(';
    printMedium(o, "export ");
    printText(o, curr->name.str) << " (";
    switch (curr->kind) {
      case ExternalKind::Function:
        o << "func";
        break;
      case ExternalKind::Table:
        o << "table";
        break;
      case ExternalKind::Memory:
        o << "memory";
        break;
      case ExternalKind::Global:
        o << "global";
        break;
      case ExternalKind::Event:
        o << "event";
        break;
      case ExternalKind::Invalid:
        WASM_UNREACHABLE("invalid ExternalKind");
    }
    o << ' ';
    printName(curr->value, o) << "))";
  }
  void emitImportHeader(Importable* curr) {
    printMedium(o, "import ");
    printText(o, curr->module.str) << ' ';
    printText(o, curr->base.str) << ' ';
  }
  void visitGlobal(Global* curr) {
    if (curr->imported()) {
      visitImportedGlobal(curr);
    } else {
      visitDefinedGlobal(curr);
    }
  }
  void emitGlobalType(Global* curr) {
    if (curr->mutable_) {
      o << "(mut ";
      printType(o, curr->type, currModule) << ')';
    } else {
      printType(o, curr->type, currModule);
    }
  }
  void visitImportedGlobal(Global* curr) {
    doIndent(o, indent);
    o << '(';
    emitImportHeader(curr);
    o << "(global ";
    printName(curr->name, o) << ' ';
    emitGlobalType(curr);
    o << "))" << maybeNewLine;
  }
  void visitDefinedGlobal(Global* curr) {
    doIndent(o, indent);
    o << '(';
    printMedium(o, "global ");
    printName(curr->name, o) << ' ';
    emitGlobalType(curr);
    o << ' ';
    visit(curr->init);
    o << ')';
    o << maybeNewLine;
  }
  void visitFunction(Function* curr) {
    if (curr->imported()) {
      visitImportedFunction(curr);
    } else {
      visitDefinedFunction(curr);
    }
  }
  void visitImportedFunction(Function* curr) {
    doIndent(o, indent);
    currFunction = curr;
    lastPrintedLocation = {0, 0, 0};
    o << '(';
    emitImportHeader(curr);
    handleSignature(curr->sig, curr->name);
    o << ')';
    o << maybeNewLine;
  }
  void visitDefinedFunction(Function* curr) {
    doIndent(o, indent);
    currFunction = curr;
    lastPrintedLocation = {0, 0, 0};
    if (currFunction->prologLocation.size()) {
      printDebugLocation(*currFunction->prologLocation.begin());
    }
    o << '(';
    printMajor(o, "func ");
    printName(curr->name, o);
    if (!stackIR && curr->stackIR && !minify) {
      o << " (; has Stack IR ;)";
    }
    if (curr->sig.params.size() > 0) {
      Index i = 0;
      for (const auto& param : curr->sig.params) {
        o << maybeSpace;
        o << '(';
        printMinor(o, "param ");
        printLocal(i, currFunction, o);
        o << ' ';
        printType(o, param, currModule) << ')';
        ++i;
      }
    }
    if (curr->sig.results != Type::none) {
      o << maybeSpace;
      printResultType(o, curr->sig.results, currModule);
    }
    incIndent();
    for (size_t i = curr->getVarIndexBase(); i < curr->getNumLocals(); i++) {
      doIndent(o, indent);
      o << '(';
      printMinor(o, "local ");
      printLocal(i, currFunction, o) << ' ';
      printType(o, curr->getLocalType(i), currModule) << ')';
      o << maybeNewLine;
    }
    // Print the body.
    if (!stackIR || !curr->stackIR) {
      // It is ok to emit a block here, as a function can directly contain a
      // list, even if our ast avoids that for simplicity. We can just do that
      // optimization here..
      if (!full && curr->body->is<Block>() &&
          curr->body->cast<Block>()->name.isNull()) {
        Block* block = curr->body->cast<Block>();
        for (auto item : block->list) {
          printFullLine(item);
        }
      } else {
        printFullLine(curr->body);
      }
      assert(controlFlowDepth == 0);
    } else {
      // Print the stack IR.
      printStackIR(curr->stackIR.get(), o, curr);
    }
    if (currFunction->epilogLocation.size() &&
        lastPrintedLocation != *currFunction->epilogLocation.begin()) {
      // Print last debug location: mix of decIndent and printDebugLocation
      // logic.
      doIndent(o, indent);
      if (!minify) {
        indent--;
      }
      printDebugLocation(*currFunction->epilogLocation.begin());
      o << ')';
    } else {
      decIndent();
    }
    o << maybeNewLine;
  }
  void visitEvent(Event* curr) {
    if (curr->imported()) {
      visitImportedEvent(curr);
    } else {
      visitDefinedEvent(curr);
    }
  }
  void visitImportedEvent(Event* curr) {
    doIndent(o, indent);
    o << '(';
    emitImportHeader(curr);
    o << "(event ";
    printName(curr->name, o);
    o << maybeSpace << "(attr " << curr->attribute << ')' << maybeSpace;
    printParamType(o, curr->sig.params, currModule);
    o << "))";
    o << maybeNewLine;
  }
  void visitDefinedEvent(Event* curr) {
    doIndent(o, indent);
    o << '(';
    printMedium(o, "event ");
    printName(curr->name, o);
    o << maybeSpace << "(attr " << curr->attribute << ')' << maybeSpace;
    printParamType(o, curr->sig.params, currModule);
    o << ")" << maybeNewLine;
  }
  void printTableHeader(Table* curr) {
    o << '(';
    printMedium(o, "table") << ' ';
    printName(curr->name, o) << ' ';
    o << curr->initial;
    if (curr->hasMax()) {
      o << ' ' << curr->max;
    }
    o << ' ';
    printType(o, curr->type, currModule) << ')';
  }
  void visitTable(Table* curr) {
    if (curr->imported()) {
      doIndent(o, indent);
      o << '(';
      emitImportHeader(curr);
      printTableHeader(curr);
      o << ')' << maybeNewLine;
    } else {
      doIndent(o, indent);
      printTableHeader(curr);
      o << maybeNewLine;
    }
  }
  void visitElementSegment(ElementSegment* curr) {
    bool allElementsRefFunc =
      std::all_of(curr->data.begin(), curr->data.end(), [](Expression* entry) {
        return entry->is<RefFunc>();
      });
    auto printElemType = [&]() {
      if (allElementsRefFunc) {
        o << "func";
      } else {
        printType(o, curr->type, currModule);
      }
    };

    doIndent(o, indent);
    o << '(';
    printMedium(o, "elem") << ' ';
    printName(curr->name, o);

    if (curr->table.is()) {
      if (!allElementsRefFunc || currModule->tables.size() > 1) {
        // tableuse
        o << " (table ";
        printName(curr->table, o);
        o << ")";
      }

      o << ' ';
      visit(curr->offset);

      if (!allElementsRefFunc || currModule->tables.size() > 1) {
        o << ' ';
        printElemType();
      }
    } else {
      o << ' ';
      printElemType();
    }

    if (allElementsRefFunc) {
      for (auto* entry : curr->data) {
        auto* refFunc = entry->cast<RefFunc>();
        o << ' ';
        printName(refFunc->func, o);
      }
    } else {
      for (auto* entry : curr->data) {
        o << ' ';
        printExpression(entry, o);
      }
    }
    o << ')' << maybeNewLine;
  }
  void printMemoryHeader(Memory* curr) {
    o << '(';
    printMedium(o, "memory") << ' ';
    printName(curr->name, o) << ' ';
    if (curr->shared) {
      o << '(';
      printMedium(o, "shared ");
    }
    if (curr->is64()) {
      o << "i64 ";
    }
    o << curr->initial;
    if (curr->hasMax()) {
      o << ' ' << curr->max;
    }
    if (curr->shared) {
      o << ")";
    }
    o << ")";
  }
  void visitMemory(Memory* curr) {
    if (!curr->exists) {
      return;
    }
    if (curr->imported()) {
      doIndent(o, indent);
      o << '(';
      emitImportHeader(curr);
      printMemoryHeader(&currModule->memory);
      o << ')' << maybeNewLine;
    } else {
      doIndent(o, indent);
      printMemoryHeader(curr);
      o << '\n';
    }
    for (auto segment : curr->segments) {
      doIndent(o, indent);
      o << '(';
      printMajor(o, "data ");
      if (segment.name.is()) {
        printName(segment.name, o);
        o << ' ';
      }
      if (!segment.isPassive) {
        visit(segment.offset);
        o << ' ';
      }
      o << "\"";
      for (size_t i = 0; i < segment.data.size(); i++) {
        unsigned char c = segment.data[i];
        switch (c) {
          case '\n':
            o << "\\n";
            break;
          case '\r':
            o << "\\0d";
            break;
          case '\t':
            o << "\\t";
            break;
          case '\f':
            o << "\\0c";
            break;
          case '\b':
            o << "\\08";
            break;
          case '\\':
            o << "\\\\";
            break;
          case '"':
            o << "\\\"";
            break;
          case '\'':
            o << "\\'";
            break;
          default: {
            if (c >= 32 && c < 127) {
              o << c;
            } else {
              o << std::hex << '\\' << (c / 16) << (c % 16) << std::dec;
            }
          }
        }
      }
      o << "\")" << maybeNewLine;
    }
  }
  void printDylinkSection(const std::unique_ptr<DylinkSection>& dylinkSection) {
    doIndent(o, indent) << ";; dylink section\n";
    doIndent(o, indent) << ";;   memorysize: " << dylinkSection->memorySize
                        << '\n';
    doIndent(o, indent) << ";;   memoryalignment: "
                        << dylinkSection->memoryAlignment << '\n';
    doIndent(o, indent) << ";;   tablesize: " << dylinkSection->tableSize
                        << '\n';
    doIndent(o, indent) << ";;   tablealignment: "
                        << dylinkSection->tableAlignment << '\n';
    for (auto& neededDynlib : dylinkSection->neededDynlibs) {
      doIndent(o, indent) << ";;   needed dynlib: " << neededDynlib << '\n';
    }
  }
  void visitModule(Module* curr) {
    currModule = curr;
    o << '(';
    printMajor(o, "module");
    if (curr->name.is()) {
      o << ' ';
      printName(curr->name, o);
    }
    incIndent();
    std::vector<HeapType> types;
    std::unordered_map<HeapType, Index> indices;
    ModuleUtils::collectHeapTypes(*curr, types, indices);
    for (auto type : types) {
      doIndent(o, indent);
      o << '(';
      printMedium(o, "type") << ' ';
      TypeNamePrinter(o, curr).print(type);
      o << ' ';
      handleHeapType(type, curr);
      o << ")" << maybeNewLine;
    }
    ModuleUtils::iterImportedMemories(
      *curr, [&](Memory* memory) { visitMemory(memory); });
    ModuleUtils::iterImportedTables(*curr,
                                    [&](Table* table) { visitTable(table); });
    ModuleUtils::iterImportedGlobals(
      *curr, [&](Global* global) { visitGlobal(global); });
    ModuleUtils::iterImportedFunctions(
      *curr, [&](Function* func) { visitFunction(func); });
    ModuleUtils::iterImportedEvents(*curr,
                                    [&](Event* event) { visitEvent(event); });
    ModuleUtils::iterDefinedGlobals(
      *curr, [&](Global* global) { visitGlobal(global); });
    ModuleUtils::iterDefinedMemories(
      *curr, [&](Memory* memory) { visitMemory(memory); });
    ModuleUtils::iterDefinedTables(*curr,
                                   [&](Table* table) { visitTable(table); });
    for (auto& segment : curr->elementSegments) {
      visitElementSegment(segment.get());
    }
    auto elemDeclareNames = TableUtils::getFunctionsNeedingElemDeclare(*curr);
    if (!elemDeclareNames.empty()) {
      doIndent(o, indent);
      printMedium(o, "(elem");
      o << " declare func";
      for (auto name : elemDeclareNames) {
        o << " $" << name;
      }
      o << ')' << maybeNewLine;
    }
    ModuleUtils::iterDefinedEvents(*curr,
                                   [&](Event* event) { visitEvent(event); });
    for (auto& child : curr->exports) {
      doIndent(o, indent);
      visitExport(child.get());
      o << maybeNewLine;
    }
    if (curr->start.is()) {
      doIndent(o, indent);
      o << '(';
      printMedium(o, "start") << ' ';
      printName(curr->start, o) << ')';
      o << maybeNewLine;
    }
    ModuleUtils::iterDefinedFunctions(
      *curr, [&](Function* func) { visitFunction(func); });
    if (curr->dylinkSection) {
      printDylinkSection(curr->dylinkSection);
    }
    for (auto& section : curr->userSections) {
      doIndent(o, indent);
      o << ";; custom section \"" << section.name << "\", size "
        << section.data.size();
      bool isPrintable = true;
      for (auto c : section.data) {
        if (!isprint(static_cast<unsigned char>(c))) {
          isPrintable = false;
          break;
        }
      }
      if (isPrintable) {
        o << ", contents: ";
        // std::quoted is not available in all the supported compilers yet.
        o << '"';
        for (auto c : section.data) {
          if (c == '\\' || c == '"') {
            o << '\\';
          }
          o << c;
        }
        o << '"';
      }
      o << maybeNewLine;
    }
    if (curr->hasFeaturesSection) {
      doIndent(o, indent);
      o << ";; features section: " << curr->features.toString() << '\n';
    }
    decIndent();
    o << maybeNewLine;
    currModule = nullptr;
  }
};

// Prints out a module
class Printer : public Pass {
protected:
  std::ostream& o;

public:
  Printer() : o(std::cout) {}
  Printer(std::ostream* o) : o(*o) {}

  bool modifiesBinaryenIR() override { return false; }

  void run(PassRunner* runner, Module* module) override {
    PrintSExpression print(o);
    print.setDebugInfo(runner->options.debugInfo);
    print.visitModule(module);
  }
};

Pass* createPrinterPass() { return new Printer(); }

// Prints out a minified module

class MinifiedPrinter : public Printer {
public:
  MinifiedPrinter() = default;
  MinifiedPrinter(std::ostream* o) : Printer(o) {}

  void run(PassRunner* runner, Module* module) override {
    PrintSExpression print(o);
    print.setMinify(true);
    print.setDebugInfo(runner->options.debugInfo);
    print.visitModule(module);
  }
};

Pass* createMinifiedPrinterPass() { return new MinifiedPrinter(); }

// Prints out a module withough elision, i.e., the full ast

class FullPrinter : public Printer {
public:
  FullPrinter() = default;
  FullPrinter(std::ostream* o) : Printer(o) {}

  void run(PassRunner* runner, Module* module) override {
    PrintSExpression print(o);
    print.setFull(true);
    print.setDebugInfo(runner->options.debugInfo);
    print.visitModule(module);
  }
};

Pass* createFullPrinterPass() { return new FullPrinter(); }

// Print Stack IR (if present)

class PrintStackIR : public Printer {
public:
  PrintStackIR() = default;
  PrintStackIR(std::ostream* o) : Printer(o) {}

  void run(PassRunner* runner, Module* module) override {
    PrintSExpression print(o);
    print.setDebugInfo(runner->options.debugInfo);
    print.setStackIR(true);
    print.visitModule(module);
  }
};

Pass* createPrintStackIRPass() { return new PrintStackIR(); }

static std::ostream& printExpression(Expression* expression,
                                     std::ostream& o,
                                     bool minify,
                                     bool full,
                                     Module* wasm) {
  if (!expression) {
    o << "(null expression)";
    return o;
  }
  PrintSExpression print(o);
  print.setMinify(minify);
  print.currModule = wasm;
  if (full || isFullForced()) {
    print.setFull(true);
    o << "[" << expression->type << "] ";
  }
  print.visit(expression);
  return o;
}

static std::ostream&
printStackInst(StackInst* inst, std::ostream& o, Function* func) {
  switch (inst->op) {
    case StackInst::Basic:
    case StackInst::BlockBegin:
    case StackInst::IfBegin:
    case StackInst::LoopBegin:
    case StackInst::TryBegin: {
      PrintExpressionContents(func, o).visit(inst->origin);
      break;
    }
    case StackInst::BlockEnd:
    case StackInst::IfEnd:
    case StackInst::LoopEnd:
    case StackInst::TryEnd: {
      printMedium(o, "end");
      o << " ;; type: ";
      TypeNamePrinter(o).print(inst->type);
      break;
    }
    case StackInst::IfElse: {
      printMedium(o, "else");
      break;
    }
    case StackInst::Catch: {
      // Because StackInst does not have info on which catch within a try this
      // is, we can't print the event name.
      printMedium(o, "catch");
      break;
    }
    case StackInst::CatchAll: {
      printMedium(o, "catch_all");
      break;
    }
    case StackInst::Delegate: {
      printMedium(o, "delegate ");
      printName(inst->origin->cast<Try>()->delegateTarget, o);
      break;
    }
    default:
      WASM_UNREACHABLE("unexpeted op");
  }
  return o;
}

static std::ostream&
printStackIR(StackIR* ir, std::ostream& o, Function* func) {
  size_t indent = func ? 2 : 0;
  auto doIndent = [&indent, &o]() {
    for (size_t j = 0; j < indent; j++) {
      o << ' ';
    }
  };

  int controlFlowDepth = 0;
  // Stack to track indices of catches within a try
  SmallVector<Index, 4> catchIndexStack;
  for (Index i = 0; i < (*ir).size(); i++) {
    auto* inst = (*ir)[i];
    if (!inst) {
      continue;
    }
    switch (inst->op) {
      case StackInst::Basic: {
        doIndent();
        // Pop is a pseudo instruction and should not be printed in the stack IR
        // format to make it valid wat form.
        if (inst->origin->is<Pop>()) {
          break;
        }

        PrintExpressionContents(func, o).visit(inst->origin);
        break;
      }
      case StackInst::TryBegin:
        catchIndexStack.push_back(0);
        [[fallthrough]];
      case StackInst::BlockBegin:
      case StackInst::IfBegin:
      case StackInst::LoopBegin: {
        controlFlowDepth++;
        doIndent();
        PrintExpressionContents(func, o).visit(inst->origin);
        indent++;
        break;
      }
      case StackInst::TryEnd:
        catchIndexStack.pop_back();
        [[fallthrough]];
      case StackInst::BlockEnd:
      case StackInst::IfEnd:
      case StackInst::LoopEnd: {
        controlFlowDepth--;
        indent--;
        doIndent();
        printMedium(o, "end");
        break;
      }
      case StackInst::IfElse: {
        indent--;
        doIndent();
        printMedium(o, "else");
        indent++;
        break;
      }
      case StackInst::Catch: {
        indent--;
        doIndent();
        printMedium(o, "catch ");
        Try* curr = inst->origin->cast<Try>();
        printName(curr->catchEvents[catchIndexStack.back()++], o);
        indent++;
        break;
      }
      case StackInst::CatchAll: {
        indent--;
        doIndent();
        printMedium(o, "catch_all");
        indent++;
        break;
      }
      case StackInst::Delegate: {
        controlFlowDepth--;
        indent--;
        doIndent();
        printMedium(o, "delegate ");
        Try* curr = inst->origin->cast<Try>();
        if (curr->delegateTarget == DELEGATE_CALLER_TARGET) {
          o << controlFlowDepth;
        } else {
          printName(curr->delegateTarget, o);
        }
        break;
      }
      default:
        WASM_UNREACHABLE("unexpeted op");
    }
    std::cout << '\n';
  }
  assert(controlFlowDepth == 0);
  return o;
}

} // namespace wasm

namespace std {

std::ostream& operator<<(std::ostream& o, wasm::Module& module) {
  wasm::PassRunner runner(&module);
  wasm::Printer(&o).run(&runner, &module);
  return o;
}

std::ostream& operator<<(std::ostream& o, wasm::Expression& expression) {
  return wasm::printExpression(&expression, o);
}

std::ostream& operator<<(std::ostream& o, wasm::Expression* expression) {
  return wasm::printExpression(expression, o);
}

std::ostream& operator<<(std::ostream& o, wasm::ModuleExpression pair) {
  return wasm::printExpression(pair.second, o, false, false, &pair.first);
}

std::ostream& operator<<(std::ostream& o, wasm::StackInst& inst) {
  return wasm::printStackInst(&inst, o);
}

std::ostream& operator<<(std::ostream& o, wasm::StackIR& ir) {
  return wasm::printStackIR(&ir, o);
}

} // namespace std
