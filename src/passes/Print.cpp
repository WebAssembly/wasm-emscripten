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

#include <algorithm>

#include <ir/iteration.h>
#include <ir/module-utils.h>
#include <ir/table-utils.h>
#include <ir/utils.h>
#include <pass.h>
#include <pretty_printing.h>
#include <support/string.h>
#include <wasm-stack.h>
#include <wasm-type-printing.h>
#include <wasm.h>

namespace wasm {

struct PrintSExpression;

static std::ostream& printExpression(Expression* expression,
                                     std::ostream& o,
                                     bool minify = false,
                                     bool full = false,
                                     Module* wasm = nullptr);

static std::ostream&
printStackInst(StackInst* inst, std::ostream& o, Function* func = nullptr);

static std::ostream& printStackIR(StackIR* ir, PrintSExpression&);

namespace {

bool checkIsFullForced() {
  if (getenv("BINARYEN_PRINT_FULL")) {
    return std::stoi(getenv("BINARYEN_PRINT_FULL")) != 0;
  }
  return false;
}

bool isFullForced() {
  static bool full = checkIsFullForced();
  return full;
}

std::ostream& printMemoryName(Name name, std::ostream& o, Module* wasm) {
  if (!wasm || wasm->memories.size() > 1) {
    o << ' ';
    name.print(o);
  }
  return o;
}

std::ostream& printLocal(Index index, Function* func, std::ostream& o) {
  Name name;
  if (func) {
    name = func->getLocalNameOrDefault(index);
  }
  if (!name) {
    name = Name::fromInt(index);
  }
  return name.print(o);
}

// Print a name from the type section, if available. Otherwise print the type
// normally.
void printTypeOrName(Type type, std::ostream& o, Module* wasm) {
  if (type.isRef() && wasm) {
    auto heapType = type.getHeapType();
    auto iter = wasm->typeNames.find(heapType);
    if (iter != wasm->typeNames.end()) {
      o << iter->second.name;
      if (type.isNullable()) {
        o << " null";
      }
      return;
    }
  }

  // No luck with a name, just print the test as best we can.
  o << type;
}

} // anonymous namespace

// Printing "unreachable" as a instruction prefix type is not valid in wasm text
// format. Print something else to make it pass.
static Type forceConcrete(Type type) {
  return type.isConcrete() ? type : Type::i32;
}

// Whatever type we print must be valid for the alignment.
static Type forceConcrete(Type type, Index align) {
  return type.isConcrete() ? type
         : align >= 16     ? Type::v128
         : align >= 8      ? Type::i64
                           : Type::i32;
}

struct PrintSExpression : public UnifiedExpressionVisitor<PrintSExpression> {
  std::ostream& o;
  unsigned indent = 0;

  bool minify;
  const char* maybeSpace;
  const char* maybeNewLine;

  // Whether to not elide nodes in output when possible (like implicit blocks)
  // and to emit types.
  bool full = false;
  // If present, it contains StackIR that we will print.
  std::optional<ModuleStackIR> moduleStackIR;

  Module* currModule = nullptr;
  Function* currFunction = nullptr;
  // Keep track of the last printed debug location to avoid printing
  // repeated debug locations for children. nullopt means that we have
  // not yet printed any debug location, or that we last printed an
  // annotation indicating that the expression had no associated
  // debug location.
  std::optional<Function::DebugLocation> lastPrintedLocation;
  bool debugInfo;

  // Used to print delegate's depth argument when it throws to the caller
  int controlFlowDepth = 0;

  std::vector<HeapType> heapTypes;
  std::unordered_map<Signature, HeapType> signatureTypes;

  // Track the print indent so that we can see when it changes. That affects how
  // we print debug annotations. In particular, we don't want to print repeated
  // debug locations for children, like this:
  //
  //  ;;@ file.cpp:20:4
  //  (block
  //    ;; no need to annotate here; children have the parent's location by
  //    ;; default anyhow
  //    (nop)
  //
  // But we do want to print an annotation even if it repeats if it is not a
  // child:
  //
  //  ;;@ file.cpp:20:4
  //  (block)
  //  ;;@ file.cpp:20:4 - this is clearer to annotate, to avoid confusion with
  //                      the case where there is no debug info on the nop
  //  (nop)
  //
  unsigned lastPrintIndent = 0;

  // Print type names by saved name or index if we have a module, or otherwise
  // by generating minimalist names. TODO: Handle conflicts between
  // user-provided names and the fallback indexed names.
  struct TypePrinter : TypeNameGeneratorBase<TypePrinter> {
    PrintSExpression& parent;
    DefaultTypeNameGenerator fallback;
    std::unordered_map<HeapType, TypeNames> fallbackNames;

    TypePrinter(PrintSExpression& parent, const std::vector<HeapType>& types)
      : parent(parent) {
      if (!parent.currModule) {
        return;
      }
      std::unordered_set<Name> usedNames;
      for (auto& [_, names] : parent.currModule->typeNames) {
        usedNames.insert(names.name);
      }
      size_t i = 0;
      // Use indices for any remaining type names, skipping any that are already
      // used.
      for (auto type : types) {
        if (parent.currModule->typeNames.count(type)) {
          ++i;
          continue;
        }
        Name name;
        do {
          name = std::to_string(i++);
        } while (usedNames.count(name));
        fallbackNames[type] = {name, {}};
      }
    }

    TypeNames getNames(HeapType type) {
      if (parent.currModule) {
        if (auto it = parent.currModule->typeNames.find(type);
            it != parent.currModule->typeNames.end()) {
          return it->second;
        }
        // In principle we should always have at least a fallback name for every
        // type in the module, so this lookup should never fail. In practice,
        // though, the `printExpression` variants deliberately avoid walking the
        // module to find unnamed types so they can be safely used in a
        // function-parallel context. That means we can have a module but not
        // have generated the fallback names, so this lookup can fail, in which
        // case we generate a name on demand.
        if (auto it = fallbackNames.find(type); it != fallbackNames.end()) {
          return it->second;
        }
      }
      return fallback.getNames(type);
    }

    Name getName(HeapType type) { return getNames(type).name; }
  } typePrinter;

  PrintSExpression(std::ostream& o) : o(o), typePrinter(*this, heapTypes) {
    setMinify(false);
    if (!full) {
      full = isFullForced();
    }
  }

  void setModule(Module* module);

  std::ostream& printType(Type type) { return o << typePrinter(type); }

  std::ostream& printHeapType(HeapType type) {
    if (type.isBasic()) {
      return o << type;
    }
    return typePrinter.getNames(type).name.print(o);
  }

  std::ostream& printPrefixedTypes(const char* prefix, Type type);

  std::ostream& printResultType(Type type) {
    return printPrefixedTypes("result", type);
  }

  std::ostream& printParamType(Type type) {
    return printPrefixedTypes("param", type);
  }

  std::ostream& printBlockType(Signature sig) {
    assert(sig.params == Type::none);
    if (sig.results == Type::none) {
      return o;
    }
    if (sig.results.isTuple()) {
      if (auto it = signatureTypes.find(sig); it != signatureTypes.end()) {
        o << "(type ";
        printHeapType(it->second);
        o << ") ";
      }
    }
    printResultType(sig.results);
    return o;
  }

  void
  printDebugLocation(const std::optional<Function::DebugLocation>& location);
  void printDebugLocation(Expression* curr);

  // Prints debug info for a delimiter in an expression.
  void printDebugDelimiterLocation(Expression* curr, Index i);

  void printExpressionContents(Expression* curr);

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

  void generateStackIR(const PassOptions& options) {
    moduleStackIR.emplace(*currModule, options);
  }

  void setDebugInfo(bool debugInfo_) { debugInfo = debugInfo_; }

  void incIndent();
  void decIndent();
  void printFullLine(Expression* expression);

  // loop, if, and try can contain implicit blocks. But they are not needed to
  // be printed in some cases.
  void maybePrintImplicitBlock(Expression* curr);

  // Generic visitor, overridden only when necessary.
  void visitExpression(Expression* curr);

  void visitBlock(Block* curr);
  void visitIf(If* curr);
  void visitLoop(Loop* curr);
  void visitTry(Try* curr);
  void visitTryTable(TryTable* curr);

  bool maybePrintUnreachableReplacement(Expression* curr, Type type);
  bool maybePrintUnreachableOrNullReplacement(Expression* curr, Type type);
  void visitCallRef(CallRef* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->target->type)) {
      visitExpression(curr);
    }
  }
  void visitRefCast(RefCast* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitStructNew(StructNew* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitStructSet(StructSet* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitStructGet(StructGet* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayNew(ArrayNew* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayNewData(ArrayNewData* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayNewElem(ArrayNewElem* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayNewFixed(ArrayNewFixed* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitArraySet(ArraySet* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayGet(ArrayGet* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayCopy(ArrayCopy* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->srcRef->type) &&
        !maybePrintUnreachableOrNullReplacement(curr, curr->destRef->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayFill(ArrayFill* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayInitData(ArrayInitData* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitArrayInitElem(ArrayInitElem* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->ref->type)) {
      visitExpression(curr);
    }
  }
  void visitContNew(ContNew* curr) {
    if (!maybePrintUnreachableReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitContBind(ContBind* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->cont->type) &&
        !maybePrintUnreachableOrNullReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitResume(Resume* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->cont->type) &&
        !maybePrintUnreachableOrNullReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitResumeThrow(ResumeThrow* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->cont->type) &&
        !maybePrintUnreachableOrNullReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }
  void visitStackSwitch(StackSwitch* curr) {
    if (!maybePrintUnreachableOrNullReplacement(curr, curr->cont->type) &&
        !maybePrintUnreachableOrNullReplacement(curr, curr->type)) {
      visitExpression(curr);
    }
  }

  // Module-level visitors
  void handleSignature(Function* curr, bool printImplicitNames = false);
  void visitExport(Export* curr);
  void emitImportHeader(Importable* curr);
  void visitGlobal(Global* curr);
  void emitGlobalType(Global* curr);
  void visitImportedGlobal(Global* curr);
  void visitDefinedGlobal(Global* curr);
  void visitFunction(Function* curr);
  void visitImportedFunction(Function* curr);
  void visitDefinedFunction(Function* curr);
  void visitTag(Tag* curr);
  void visitImportedTag(Tag* curr);
  void visitDefinedTag(Tag* curr);
  void printTableHeader(Table* curr);
  void visitTable(Table* curr);
  void visitElementSegment(ElementSegment* curr);
  void printMemoryHeader(Memory* curr);
  void visitMemory(Memory* curr);
  void visitDataSegment(DataSegment* curr);
  void printDylinkSection(const std::unique_ptr<DylinkSection>& dylinkSection);
  void visitModule(Module* curr);
};

// Prints the internal contents of an expression: everything but
// the children.
struct PrintExpressionContents
  : public OverriddenVisitor<PrintExpressionContents> {
  PrintSExpression& parent;
  Module* wasm = nullptr;
  Function* currFunction = nullptr;
  std::ostream& o;
  FeatureSet features;
  bool full;

  PrintExpressionContents(PrintSExpression& parent)
    : parent(parent), wasm(parent.currModule),
      currFunction(parent.currFunction), o(parent.o),
      features(wasm ? wasm->features : FeatureSet::All), full(isFullForced()) {}

  std::ostream& printType(Type type) { return parent.printType(type); }

  std::ostream& printHeapType(HeapType type) {
    return parent.printHeapType(type);
  }

  std::ostream& printResultType(Type type) {
    return parent.printResultType(type);
  }

  std::ostream& printParamType(Type type) {
    return parent.printParamType(type);
  }

  std::ostream& printBlockType(Signature sig) {
    return parent.printBlockType(sig);
  }

  void visitBlock(Block* curr) {
    printMedium(o, "block");
    if (curr->name.is()) {
      o << ' ';
      curr->name.print(o);
    }
    if (curr->type.isConcrete()) {
      o << ' ';
      printBlockType(Signature(Type::none, curr->type));
    }
  }
  void visitIf(If* curr) {
    printMedium(o, "if");
    // Ifs are unreachable if their condition is unreachable, but in that case
    // the arms might have some concrete type we have to account for to produce
    // valid wat.
    auto type = curr->type;
    if (curr->condition->type == Type::unreachable && curr->ifFalse) {
      type = Type::getLeastUpperBound(curr->ifTrue->type, curr->ifFalse->type);
    }
    if (type.isConcrete()) {
      o << ' ';
      printBlockType(Signature(Type::none, type));
    }
  }
  void visitLoop(Loop* curr) {
    printMedium(o, "loop");
    if (curr->name.is()) {
      o << ' ';
      curr->name.print(o);
    }
    if (curr->type.isConcrete()) {
      o << ' ';
      printBlockType(Signature(Type::none, curr->type));
    }
  }
  void visitBreak(Break* curr) {
    if (curr->condition) {
      printMedium(o, "br_if ");
    } else {
      printMedium(o, "br ");
    }
    curr->name.print(o);
  }
  void visitSwitch(Switch* curr) {
    printMedium(o, "br_table");
    for (auto& t : curr->targets) {
      o << ' ';
      t.print(o);
    }
    o << ' ';
    curr->default_.print(o);
  }
  void visitCall(Call* curr) {
    if (curr->isReturn) {
      printMedium(o, "return_call ");
    } else {
      printMedium(o, "call ");
    }
    curr->target.print(o);
  }
  void visitCallIndirect(CallIndirect* curr) {
    if (curr->isReturn) {
      printMedium(o, "return_call_indirect ");
    } else {
      printMedium(o, "call_indirect ");
    }

    if (features.hasReferenceTypes()) {
      curr->table.print(o);
      o << ' ';
    }

    o << '(';
    printMinor(o, "type ");

    printHeapType(curr->heapType);

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
    if (full && currFunction) {
      o << " (; local type: ";
      printType(currFunction->getLocalType(curr->index));
      o << " ;)";
    }
  }
  void visitGlobalGet(GlobalGet* curr) {
    printMedium(o, "global.get ");
    curr->name.print(o);
  }
  void visitGlobalSet(GlobalSet* curr) {
    printMedium(o, "global.set ");
    curr->name.print(o);
  }
  void visitLoad(Load* curr) {
    prepareColor(o) << forceConcrete(curr->type, curr->align);
    if (curr->isAtomic) {
      o << ".atomic";
    }
    o << ".load";
    if (curr->type != Type::unreachable &&
        curr->bytes < curr->type.getByteSize()) {
      if (curr->bytes == 1) {
        o << '8';
      } else if (curr->bytes == 2) {
        if (curr->type == Type::f32) {
          o << "_f16";
        } else {
          o << "16";
        }
      } else if (curr->bytes == 4) {
        o << "32";
      } else {
        abort();
      }
      if (curr->type != Type::f32) {
        o << (curr->signed_ ? "_s" : "_u");
      }
    }
    restoreNormalColor(o);
    printMemoryName(curr->memory, o, wasm);
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
        if (curr->valueType == Type::f32) {
          o << "_f16";
        } else {
          o << "16";
        }
      } else if (curr->bytes == 4) {
        o << "32";
      } else {
        abort();
      }
    }
    restoreNormalColor(o);
    printMemoryName(curr->memory, o, wasm);
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
    printMemoryName(curr->memory, o, wasm);
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
    printMemoryName(curr->memory, o, wasm);
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
    printMemoryName(curr->memory, o, wasm);
    if (curr->offset) {
      o << " offset=" << curr->offset;
    }
  }
  void visitAtomicNotify(AtomicNotify* curr) {
    printMedium(o, "memory.atomic.notify");
    printMemoryName(curr->memory, o, wasm);
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
      case ExtractLaneVecF16x8:
        o << "f16x8.extract_lane";
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
      case ReplaceLaneVecF16x8:
        o << "f16x8.replace_lane";
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
      case LaneselectI8x16:
        o << "i8x16.laneselect";
        break;
      case LaneselectI16x8:
        o << "i16x8.laneselect";
        break;
      case LaneselectI32x4:
        o << "i32x4.laneselect";
        break;
      case LaneselectI64x2:
        o << "i64x2.laneselect";
        break;
      case RelaxedMaddVecF16x8:
        o << "f16x8.relaxed_madd";
        break;
      case RelaxedNmaddVecF16x8:
        o << "f16x8.relaxed_nmadd";
        break;
      case RelaxedMaddVecF32x4:
        o << "f32x4.relaxed_madd";
        break;
      case RelaxedNmaddVecF32x4:
        o << "f32x4.relaxed_nmadd";
        break;
      case RelaxedMaddVecF64x2:
        o << "f64x2.relaxed_madd";
        break;
      case RelaxedNmaddVecF64x2:
        o << "f64x2.relaxed_nmadd";
        break;
      case DotI8x16I7x16AddSToVecI32x4:
        o << "i32x4.dot_i8x16_i7x16_add_s";
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
    printMemoryName(curr->memory, o, wasm);
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
    printMemoryName(curr->memory, o, wasm);
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
    printMemoryName(curr->memory, o, wasm);
    o << ' ';
    curr->segment.print(o);
  }
  void visitDataDrop(DataDrop* curr) {
    prepareColor(o);
    o << "data.drop";
    restoreNormalColor(o);
    o << ' ';
    curr->segment.print(o);
  }
  void visitMemoryCopy(MemoryCopy* curr) {
    prepareColor(o);
    o << "memory.copy";
    restoreNormalColor(o);
    printMemoryName(curr->destMemory, o, wasm);
    printMemoryName(curr->sourceMemory, o, wasm);
  }
  void visitMemoryFill(MemoryFill* curr) {
    prepareColor(o);
    o << "memory.fill";
    restoreNormalColor(o);
    printMemoryName(curr->memory, o, wasm);
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
      case SplatVecF16x8:
        o << "f16x8.splat";
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
      case AbsVecF16x8:
        o << "f16x8.abs";
        break;
      case NegVecF16x8:
        o << "f16x8.neg";
        break;
      case SqrtVecF16x8:
        o << "f16x8.sqrt";
        break;
      case CeilVecF16x8:
        o << "f16x8.ceil";
        break;
      case FloorVecF16x8:
        o << "f16x8.floor";
        break;
      case TruncVecF16x8:
        o << "f16x8.trunc";
        break;
      case NearestVecF16x8:
        o << "f16x8.nearest";
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
      case RelaxedTruncSVecF32x4ToVecI32x4:
        o << "i32x4.relaxed_trunc_f32x4_s";
        break;
      case RelaxedTruncUVecF32x4ToVecI32x4:
        o << "i32x4.relaxed_trunc_f32x4_u";
        break;
      case RelaxedTruncZeroSVecF64x2ToVecI32x4:
        o << "i32x4.relaxed_trunc_f64x2_s_zero";
        break;
      case RelaxedTruncZeroUVecF64x2ToVecI32x4:
        o << "i32x4.relaxed_trunc_f64x2_u_zero";
        break;
      case TruncSatSVecF16x8ToVecI16x8:
        o << "i16x8.trunc_sat_f16x8_s";
        break;
      case TruncSatUVecF16x8ToVecI16x8:
        o << "i16x8.trunc_sat_f16x8_u";
        break;
      case ConvertSVecI16x8ToVecF16x8:
        o << "f16x8.convert_i16x8_s";
        break;
      case ConvertUVecI16x8ToVecF16x8:
        o << "f16x8.convert_i16x8_u";
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
      case EqVecF16x8:
        o << "f16x8.eq";
        break;
      case NeVecF16x8:
        o << "f16x8.ne";
        break;
      case LtVecF16x8:
        o << "f16x8.lt";
        break;
      case GtVecF16x8:
        o << "f16x8.gt";
        break;
      case LeVecF16x8:
        o << "f16x8.le";
        break;
      case GeVecF16x8:
        o << "f16x8.ge";
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

      case AddVecF16x8:
        o << "f16x8.add";
        break;
      case SubVecF16x8:
        o << "f16x8.sub";
        break;
      case MulVecF16x8:
        o << "f16x8.mul";
        break;
      case DivVecF16x8:
        o << "f16x8.div";
        break;
      case MinVecF16x8:
        o << "f16x8.min";
        break;
      case MaxVecF16x8:
        o << "f16x8.max";
        break;
      case PMinVecF16x8:
        o << "f16x8.pmin";
        break;
      case PMaxVecF16x8:
        o << "f16x8.pmax";
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

      case SwizzleVecI8x16:
        o << "i8x16.swizzle";
        break;

      case RelaxedMinVecF32x4:
        o << "f32x4.relaxed_min";
        break;
      case RelaxedMaxVecF32x4:
        o << "f32x4.relaxed_max";
        break;
      case RelaxedMinVecF64x2:
        o << "f64x2.relaxed_min";
        break;
      case RelaxedMaxVecF64x2:
        o << "f64x2.relaxed_max";
        break;
      case RelaxedSwizzleVecI8x16:
        o << "i8x16.relaxed_swizzle";
        break;
      case RelaxedQ15MulrSVecI16x8:
        o << "i16x8.relaxed_q15mulr_s";
        break;
      case DotI8x16I7x16SToVecI16x8:
        o << "i16x8.dot_i8x16_i7x16_s";
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
      printResultType(curr->type);
    }
  }
  void visitDrop(Drop* curr) {
    if (curr->value->type.isTuple()) {
      printMedium(o, "tuple.drop ");
      o << curr->value->type.size();
    } else {
      printMedium(o, "drop");
    }
  }
  void visitReturn(Return* curr) { printMedium(o, "return"); }
  void visitMemorySize(MemorySize* curr) {
    printMedium(o, "memory.size");
    printMemoryName(curr->memory, o, wasm);
  }
  void visitMemoryGrow(MemoryGrow* curr) {
    printMedium(o, "memory.grow");
    printMemoryName(curr->memory, o, wasm);
  }
  void visitRefNull(RefNull* curr) {
    printMedium(o, "ref.null ");
    printHeapType(curr->type.getHeapType());
  }
  void visitRefIsNull(RefIsNull* curr) { printMedium(o, "ref.is_null"); }
  void visitRefFunc(RefFunc* curr) {
    printMedium(o, "ref.func ");
    curr->func.print(o);
  }
  void visitRefEq(RefEq* curr) { printMedium(o, "ref.eq"); }
  void visitTableGet(TableGet* curr) {
    printMedium(o, "table.get ");
    curr->table.print(o);
  }
  void visitTableSet(TableSet* curr) {
    printMedium(o, "table.set ");
    curr->table.print(o);
  }
  void visitTableSize(TableSize* curr) {
    printMedium(o, "table.size ");
    curr->table.print(o);
  }
  void visitTableGrow(TableGrow* curr) {
    printMedium(o, "table.grow ");
    curr->table.print(o);
  }
  void visitTableFill(TableFill* curr) {
    printMedium(o, "table.fill ");
    curr->table.print(o);
  }
  void visitTableCopy(TableCopy* curr) {
    printMedium(o, "table.copy ");
    curr->destTable.print(o);
    o << ' ';
    curr->sourceTable.print(o);
  }
  void visitTableInit(TableInit* curr) {
    printMedium(o, "table.init ");
    curr->table.print(o);
    o << ' ';
    curr->segment.print(o);
  }
  void visitTry(Try* curr) {
    printMedium(o, "try");
    if (curr->name.is()) {
      o << ' ';
      curr->name.print(o);
    }
    if (curr->type.isConcrete()) {
      o << ' ';
      printBlockType(Signature(Type::none, curr->type));
    }
  }
  void visitTryTable(TryTable* curr) {
    printMedium(o, "try_table");
    if (curr->type.isConcrete()) {
      o << ' ';
      printBlockType(Signature(Type::none, curr->type));
    }
    for (Index i = 0; i < curr->catchTags.size(); i++) {
      o << " (";
      if (curr->catchTags[i]) {
        printMedium(o, curr->catchRefs[i] ? "catch_ref " : "catch ");
        curr->catchTags[i].print(o);
        o << ' ';
      } else {
        printMedium(o, curr->catchRefs[i] ? "catch_all_ref " : "catch_all ");
      }
      curr->catchDests[i].print(o);
      o << ')';
    }
  }
  void visitThrow(Throw* curr) {
    printMedium(o, "throw ");
    curr->tag.print(o);
  }
  void visitRethrow(Rethrow* curr) {
    printMedium(o, "rethrow ");
    curr->target.print(o);
  }
  void visitThrowRef(ThrowRef* curr) { printMedium(o, "throw_ref"); }
  void visitNop(Nop* curr) { printMinor(o, "nop"); }
  void visitUnreachable(Unreachable* curr) { printMinor(o, "unreachable"); }
  void visitPop(Pop* curr) {
    prepareColor(o) << "pop ";
    printType(curr->type);
    restoreNormalColor(o);
  }
  void visitTupleMake(TupleMake* curr) {
    printMedium(o, "tuple.make ");
    o << curr->operands.size();
  }
  void visitTupleExtract(TupleExtract* curr) {
    printMedium(o, "tuple.extract ");
    // If the tuple is unreachable, its size will be reported as 1, but that's
    // not a valid tuple size. The size we print mostly doesn't matter if the
    // tuple is unreachable, but it does have to be valid.
    o << std::max(curr->tuple->type.size(), size_t(2)) << " ";
    o << curr->index;
  }
  void visitRefI31(RefI31* curr) {
    bool shared =
      curr->type != Type::unreachable && curr->type.getHeapType().isShared();
    printMedium(o, shared ? "ref.i31_shared" : "ref.i31");
  }
  void visitI31Get(I31Get* curr) {
    printMedium(o, curr->signed_ ? "i31.get_s" : "i31.get_u");
  }

  void visitCallRef(CallRef* curr) {
    printMedium(o, curr->isReturn ? "return_call_ref " : "call_ref ");
    printHeapType(curr->target->type.getHeapType());
  }
  void visitRefTest(RefTest* curr) {
    printMedium(o, "ref.test ");
    printType(curr->castType);
  }
  void visitRefCast(RefCast* curr) {
    printMedium(o, "ref.cast ");
    printType(curr->type);
  }

  void visitBrOn(BrOn* curr) {
    switch (curr->op) {
      case BrOnNull:
        printMedium(o, "br_on_null ");
        curr->name.print(o);
        return;
      case BrOnNonNull:
        printMedium(o, "br_on_non_null ");
        curr->name.print(o);
        return;
      case BrOnCast:
        printMedium(o, "br_on_cast ");
        curr->name.print(o);
        o << ' ';
        if (curr->ref->type == Type::unreachable) {
          // Need to print some reference type in the correct hierarchy rather
          // than unreachable, and the cast type itself is the best possible
          // option.
          printType(curr->castType);
        } else {
          printType(curr->ref->type);
        }
        o << ' ';
        printType(curr->castType);
        return;
      case BrOnCastFail:
        printMedium(o, "br_on_cast_fail ");
        curr->name.print(o);
        o << ' ';
        if (curr->ref->type == Type::unreachable) {
          printType(curr->castType);
        } else {
          printType(curr->ref->type);
        }
        o << ' ';
        printType(curr->castType);
        return;
    }
    WASM_UNREACHABLE("Unexpected br_on* op");
  }
  void visitStructNew(StructNew* curr) {
    printMedium(o, "struct.new");
    if (curr->isWithDefault()) {
      printMedium(o, "_default");
    }
    o << ' ';
    printHeapType(curr->type.getHeapType());
  }
  void printFieldName(HeapType type, Index index) {
    auto names = parent.typePrinter.getNames(type).fieldNames;
    if (auto it = names.find(index); it != names.end()) {
      it->second.print(o);
    } else {
      o << index;
    }
  }
  void visitStructGet(StructGet* curr) {
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
    printHeapType(heapType);
    o << ' ';
    printFieldName(heapType, curr->index);
  }
  void visitStructSet(StructSet* curr) {
    printMedium(o, "struct.set ");
    auto heapType = curr->ref->type.getHeapType();
    printHeapType(heapType);
    o << ' ';
    printFieldName(heapType, curr->index);
  }
  void visitArrayNew(ArrayNew* curr) {
    printMedium(o, "array.new");
    if (curr->isWithDefault()) {
      printMedium(o, "_default");
    }
    o << ' ';
    printHeapType(curr->type.getHeapType());
  }
  void visitArrayNewData(ArrayNewData* curr) {
    printMedium(o, "array.new_data");
    o << ' ';
    printHeapType(curr->type.getHeapType());
    o << ' ';
    curr->segment.print(o);
  }
  void visitArrayNewElem(ArrayNewElem* curr) {
    printMedium(o, "array.new_elem");
    o << ' ';
    printHeapType(curr->type.getHeapType());
    o << ' ';
    curr->segment.print(o);
  }
  void visitArrayNewFixed(ArrayNewFixed* curr) {
    printMedium(o, "array.new_fixed");
    o << ' ';
    printHeapType(curr->type.getHeapType());
    o << ' ';
    o << curr->values.size();
  }
  void visitArrayGet(ArrayGet* curr) {
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
    printHeapType(curr->ref->type.getHeapType());
  }
  void visitArraySet(ArraySet* curr) {
    printMedium(o, "array.set ");
    printHeapType(curr->ref->type.getHeapType());
  }
  void visitArrayLen(ArrayLen* curr) { printMedium(o, "array.len"); }
  void visitArrayCopy(ArrayCopy* curr) {
    printMedium(o, "array.copy ");
    printHeapType(curr->destRef->type.getHeapType());
    o << ' ';
    printHeapType(curr->srcRef->type.getHeapType());
  }
  void visitArrayFill(ArrayFill* curr) {
    printMedium(o, "array.fill ");
    printHeapType(curr->ref->type.getHeapType());
  }
  void visitArrayInitData(ArrayInitData* curr) {
    printMedium(o, "array.init_data ");
    printHeapType(curr->ref->type.getHeapType());
    o << ' ';
    curr->segment.print(o);
  }
  void visitArrayInitElem(ArrayInitElem* curr) {
    printMedium(o, "array.init_elem ");
    printHeapType(curr->ref->type.getHeapType());
    o << ' ';
    curr->segment.print(o);
  }
  void visitRefAs(RefAs* curr) {
    switch (curr->op) {
      case RefAsNonNull:
        printMedium(o, "ref.as_non_null");
        break;
      case AnyConvertExtern:
        printMedium(o, "any.convert_extern");
        break;
      case ExternConvertAny:
        printMedium(o, "extern.convert_any");
        break;
      default:
        WASM_UNREACHABLE("invalid ref.is_*");
    }
  }
  void visitStringNew(StringNew* curr) {
    switch (curr->op) {
      case StringNewLossyUTF8Array:
        printMedium(o, "string.new_lossy_utf8_array");
        break;
      case StringNewWTF16Array:
        printMedium(o, "string.new_wtf16_array");
        break;
      case StringNewFromCodePoint:
        printMedium(o, "string.from_code_point");
        break;
      default:
        WASM_UNREACHABLE("invalid string.new*");
    }
  }
  void visitStringConst(StringConst* curr) {
    printMedium(o, "string.const ");
    // Re-encode from WTF-16 to WTF-8.
    std::stringstream wtf8;
    [[maybe_unused]] bool valid =
      String::convertWTF16ToWTF8(wtf8, curr->string.str);
    assert(valid);
    // TODO: Use wtf8.view() once we have C++20.
    String::printEscaped(o, wtf8.str());
  }
  void visitStringMeasure(StringMeasure* curr) {
    switch (curr->op) {
      case StringMeasureUTF8:
        printMedium(o, "string.measure_utf8");
        break;
      case StringMeasureWTF16:
        printMedium(o, "string.measure_wtf16");
        break;
      default:
        WASM_UNREACHABLE("invalid string.measure*");
    }
  }
  void visitStringEncode(StringEncode* curr) {
    switch (curr->op) {
      case StringEncodeLossyUTF8Array:
        printMedium(o, "string.encode_lossy_utf8_array");
        break;
      case StringEncodeWTF16Array:
        printMedium(o, "string.encode_wtf16_array");
        break;
      default:
        WASM_UNREACHABLE("invalid string.encode*");
    }
  }
  void visitStringConcat(StringConcat* curr) {
    printMedium(o, "string.concat");
  }
  void visitStringEq(StringEq* curr) {
    switch (curr->op) {
      case StringEqEqual:
        printMedium(o, "string.eq");
        break;
      case StringEqCompare:
        printMedium(o, "string.compare");
        break;
      default:
        WASM_UNREACHABLE("invalid string.eq*");
    }
  }
  void visitStringWTF16Get(StringWTF16Get* curr) {
    printMedium(o, "stringview_wtf16.get_codeunit");
  }
  void visitStringSliceWTF(StringSliceWTF* curr) {
    printMedium(o, "stringview_wtf16.slice");
  }
  void visitContNew(ContNew* curr) {
    assert(curr->type.isContinuation());
    printMedium(o, "cont.new ");
    printHeapType(curr->type.getHeapType());
  }
  void visitContBind(ContBind* curr) {
    assert(curr->cont->type.isContinuation() && curr->type.isContinuation());
    printMedium(o, "cont.bind ");
    printHeapType(curr->cont->type.getHeapType());
    o << ' ';
    printHeapType(curr->type.getHeapType());
  }
  void visitSuspend(Suspend* curr) {
    printMedium(o, "suspend ");
    curr->tag.print(o);
  }
  template<typename ResumeType>
  static void handleResumeTable(std::ostream& o, ResumeType* curr) {
    static_assert(std::is_base_of<ResumeType, Resume>::value ||
                  std::is_base_of<ResumeType, ResumeThrow>::value);
    for (Index i = 0; i < curr->handlerTags.size(); i++) {
      o << " (";
      printMedium(o, "on ");
      curr->handlerTags[i].print(o);
      o << ' ';
      if (curr->handlerBlocks[i].isNull()) {
        o << "switch";
      } else {
        curr->handlerBlocks[i].print(o);
      }
      o << ')';
    }
  }
  void visitResume(Resume* curr) {
    assert(curr->cont->type.isContinuation());
    printMedium(o, "resume");

    o << ' ';
    printHeapType(curr->cont->type.getHeapType());

    handleResumeTable(o, curr);
  }
  void visitResumeThrow(ResumeThrow* curr) {
    assert(curr->cont->type.isContinuation());
    printMedium(o, "resume_throw");

    o << ' ';
    printHeapType(curr->cont->type.getHeapType());
    o << ' ';
    curr->tag.print(o);

    handleResumeTable(o, curr);
  }
  void visitStackSwitch(StackSwitch* curr) {
    assert(curr->cont->type.isContinuation());
    printMedium(o, "switch");

    o << ' ';
    printHeapType(curr->cont->type.getHeapType());
    o << ' ';
    curr->tag.print(o);
  }
};

void PrintSExpression::setModule(Module* module) {
  currModule = module;
  if (module) {
    heapTypes = ModuleUtils::getOptimizedIndexedHeapTypes(*module).types;
    for (auto type : heapTypes) {
      if (type.isSignature()) {
        signatureTypes.insert({type.getSignature(), type});
      }
    }
  } else {
    heapTypes = {};
    signatureTypes = {};
  }
  // Reset the type printer for this module's types (or absence thereof).
  typePrinter.~TypePrinter();
  new (&typePrinter) TypePrinter(*this, heapTypes);
}

std::ostream& PrintSExpression::printPrefixedTypes(const char* prefix,
                                                   Type type) {
  o << '(' << prefix;
  if (type == Type::none) {
    return o << ')';
  }
  if (type.isTuple()) {
    // Tuple types are not printed in parens, we can just emit them one after
    // the other in the same list as the "result".
    for (auto t : type) {
      o << ' ';
      printType(t);
    }
  } else {
    o << ' ';
    printType(type);
  }
  o << ')';
  return o;
}

void PrintSExpression::printDebugLocation(
  const std::optional<Function::DebugLocation>& location) {
  if (minify) {
    return;
  }
  // Do not skip repeated debug info in full mode, for less-confusing debugging:
  // full mode prints out everything in the most verbose manner.
  if (lastPrintedLocation == location && indent > lastPrintIndent && !full) {
    return;
  }
  lastPrintedLocation = location;
  lastPrintIndent = indent;
  if (!location) {
    o << ";;@\n";
  } else {
    auto fileName = currModule->debugInfoFileNames[location->fileIndex];
    o << ";;@ " << fileName << ":" << location->lineNumber << ":"
      << location->columnNumber;

    if (location->symbolNameIndex) {
      auto symbolName =
        currModule->debugInfoSymbolNames[*(location->symbolNameIndex)];
      o << ":" << symbolName;
    }

    o << '\n';
  }
  doIndent(o, indent);
}

void PrintSExpression::printDebugLocation(Expression* curr) {
  if (currFunction) {
    // show an annotation, if there is one
    auto& debugLocations = currFunction->debugLocations;
    auto iter = debugLocations.find(curr);
    if (iter != debugLocations.end()) {
      printDebugLocation(iter->second);
    } else {
      printDebugLocation(std::nullopt);
    }
    // show a binary position, if there is one
    if (debugInfo) {
      auto iter = currFunction->expressionLocations.find(curr);
      if (iter != currFunction->expressionLocations.end()) {
        Colors::grey(o);
        o << ";; code offset: 0x" << std::hex << iter->second.start << std::dec
          << '\n';
        restoreNormalColor(o);
        doIndent(o, indent);
      }
    }
  }
}

void PrintSExpression::printDebugDelimiterLocation(Expression* curr, Index i) {
  if (currFunction && debugInfo) {
    auto iter = currFunction->delimiterLocations.find(curr);
    if (iter != currFunction->delimiterLocations.end()) {
      auto& locations = iter->second;
      Colors::grey(o);
      o << ";; code offset: 0x" << std::hex << locations[i] << std::dec << '\n';
      restoreNormalColor(o);
      doIndent(o, indent);
    }
  }
}

void PrintSExpression::printExpressionContents(Expression* curr) {
  PrintExpressionContents(*this).visit(curr);
}

void PrintSExpression::incIndent() {
  if (minify) {
    return;
  }
  o << '\n';
  indent++;
}

void PrintSExpression::decIndent() {
  if (!minify) {
    assert(indent > 0);
    indent--;
    doIndent(o, indent);
  }
  o << ')';
}

void PrintSExpression::printFullLine(Expression* expression) {
  if (!minify) {
    doIndent(o, indent);
  }
  visit(expression);
  if (full) {
    o << " (; ";
    printTypeOrName(expression->type, o, currModule);
    o << " ;)";
  }
  o << maybeNewLine;
}

void PrintSExpression::maybePrintImplicitBlock(Expression* curr) {
  auto block = curr->dynCast<Block>();
  if (!full && block && block->name.isNull()) {
    for (auto expression : block->list) {
      printFullLine(expression);
    }
  } else {
    printFullLine(curr);
  }
}

void PrintSExpression::visitExpression(Expression* curr) {
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

void PrintSExpression::visitBlock(Block* curr) {
  // special-case Block, because Block nesting (in their first element) can be
  // incredibly deep
  std::vector<Block*> stack;
  while (1) {
    if (stack.size() > 0) {
      doIndent(o, indent);
      printDebugLocation(curr);
    }
    stack.push_back(curr);
    o << '(';
    printExpressionContents(curr);
    if (full) {
      o << " (; ";
      printTypeOrName(curr->type, o, currModule);
      o << " ;)";
    }
    incIndent();
    if (curr->list.size() > 0 && curr->list[0]->is<Block>()) {
      // recurse into the first element
      curr = curr->list[0]->cast<Block>();
      continue;
    } else {
      break; // that's all we can recurse, start to unwind
    }
  }

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
    controlFlowDepth--;
  }
  decIndent();
  if (full) {
    o << " ;; end block";
    if (curr->name.is()) {
      o << ' ' << curr->name;
    }
  }
}

void PrintSExpression::visitIf(If* curr) {
  controlFlowDepth++;
  o << '(';
  printExpressionContents(curr);
  incIndent();
  printFullLine(curr->condition);
  doIndent(o, indent);
  o << "(then";
  incIndent();
  maybePrintImplicitBlock(curr->ifTrue);
  decIndent();
  o << maybeNewLine;
  if (curr->ifFalse) {
    doIndent(o, indent);
    o << "(else";
    incIndent();
    // Note: debug info here is not used as LLVM does not emit ifs, and since
    // LLVM is the main source of DWARF, effectively we never encounter ifs
    // with DWARF.
    printDebugDelimiterLocation(curr, BinaryLocations::Else);
    maybePrintImplicitBlock(curr->ifFalse);
    decIndent();
    o << maybeNewLine;
  }
  decIndent();
  if (full) {
    o << " ;; end if";
  }
  controlFlowDepth--;
}

void PrintSExpression::visitLoop(Loop* curr) {
  controlFlowDepth++;
  o << '(';
  printExpressionContents(curr);
  incIndent();
  maybePrintImplicitBlock(curr->body);
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
// try-delegate is written in the folded format as
// (try
//  (do
//    ...
//  )
//  (delegate $label)
// )
// When the 'delegate' delegates to the caller, we write the argument as an
// immediate.
void PrintSExpression::visitTry(Try* curr) {
  controlFlowDepth++;
  o << '(';
  printExpressionContents(curr);
  incIndent();
  doIndent(o, indent);
  o << '(';
  printMedium(o, "do");
  incIndent();
  maybePrintImplicitBlock(curr->body);
  decIndent();
  o << "\n";
  for (size_t i = 0; i < curr->catchTags.size(); i++) {
    doIndent(o, indent);
    printDebugDelimiterLocation(curr, i);
    o << '(';
    printMedium(o, "catch ");
    curr->catchTags[i].print(o);
    incIndent();
    maybePrintImplicitBlock(curr->catchBodies[i]);
    decIndent();
    o << "\n";
  }
  if (curr->hasCatchAll()) {
    doIndent(o, indent);
    printDebugDelimiterLocation(curr, curr->catchTags.size());
    o << '(';
    printMedium(o, "catch_all");
    incIndent();
    maybePrintImplicitBlock(curr->catchBodies.back());
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
      curr->delegateTarget.print(o);
    }
    o << ")\n";
  }
  decIndent();
  if (full) {
    o << " ;; end try";
  }
}

void PrintSExpression::visitTryTable(TryTable* curr) {
  controlFlowDepth++;
  o << '(';
  printExpressionContents(curr);
  incIndent();
  maybePrintImplicitBlock(curr->body);
  decIndent();
  if (full) {
    o << " ;; end try_table";
  }
  controlFlowDepth--;
}

bool PrintSExpression::maybePrintUnreachableReplacement(Expression* curr,
                                                        Type type) {
  // When we cannot print an instruction because the child from which it's
  // supposed to get a type immediate is unreachable, then we print a
  // semantically-equivalent block that drops each of the children and ends in
  // an unreachable.
  if (type != Type::unreachable) {
    return false;
  }

  // Emit a block with drops of the children.
  o << "(block";
  if (!minify) {
    o << " ;; (replaces unreachable " << getExpressionName(curr)
      << " we can't emit)";
  }
  incIndent();
  for (auto* child : ChildIterator(curr)) {
    Drop drop;
    drop.value = child;
    printFullLine(&drop);
  }
  Unreachable unreachable;
  printFullLine(&unreachable);
  decIndent();
  return true;
}

bool PrintSExpression::maybePrintUnreachableOrNullReplacement(Expression* curr,
                                                              Type type) {
  if (type.isNull()) {
    type = Type::unreachable;
  }
  return maybePrintUnreachableReplacement(curr, type);
}

static bool requiresExplicitFuncType(HeapType type) {
  // When the `(type $f)` in a function's typeuse is omitted, the typeuse
  // matches or declares an MVP function type. When the intended type is not an
  // MVP function type, we therefore need the explicit `(type $f)`.
  return type.isOpen() || type.isShared() || type.getRecGroup().size() > 1;
}

void PrintSExpression::handleSignature(Function* curr,
                                       bool printImplicitNames) {
  o << '(';
  printMajor(o, "func ");
  curr->name.print(o);
  if ((currModule && currModule->features.hasGC()) ||
      requiresExplicitFuncType(curr->type)) {
    o << " (type ";
    printHeapType(curr->type) << ')';
  }
  bool inParam = false;
  Index i = 0;
  for (const auto& param : curr->getParams()) {
    auto hasName = printImplicitNames || curr->hasLocalName(i);
    if (hasName && inParam) {
      o << ')' << maybeSpace;
      inParam = false;
    } else if (inParam) {
      o << ' ';
    } else {
      o << maybeSpace;
    }
    if (!inParam) {
      o << '(';
      printMinor(o, "param ");
      inParam = true;
    }
    if (hasName) {
      printLocal(i, currFunction, o);
      o << ' ';
    }
    printType(param);
    if (hasName) {
      o << ')';
      inParam = false;
    }
    ++i;
  }
  if (inParam) {
    o << ')';
  }
  if (curr->getResults() != Type::none) {
    o << maybeSpace;
    printResultType(curr->getResults());
  }
}

void PrintSExpression::visitExport(Export* curr) {
  o << '(';
  printMedium(o, "export ");
  std::stringstream escaped;
  String::printEscaped(escaped, curr->name.str);
  printText(o, escaped.str(), false) << " (";
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
    case ExternalKind::Tag:
      o << "tag";
      break;
    case ExternalKind::Invalid:
      WASM_UNREACHABLE("invalid ExternalKind");
  }
  o << ' ';
  curr->value.print(o) << "))";
}

void PrintSExpression::emitImportHeader(Importable* curr) {
  printMedium(o, "import ");
  std::stringstream escapedModule, escapedBase;
  String::printEscaped(escapedModule, curr->module.str);
  String::printEscaped(escapedBase, curr->base.str);
  printText(o, escapedModule.str(), false) << ' ';
  printText(o, escapedBase.str(), false) << ' ';
}

void PrintSExpression::visitGlobal(Global* curr) {
  if (curr->imported()) {
    visitImportedGlobal(curr);
  } else {
    visitDefinedGlobal(curr);
  }
}

void PrintSExpression::emitGlobalType(Global* curr) {
  if (curr->mutable_) {
    o << "(mut ";
    printType(curr->type) << ')';
  } else {
    printType(curr->type);
  }
}

void PrintSExpression::visitImportedGlobal(Global* curr) {
  doIndent(o, indent);
  o << '(';
  emitImportHeader(curr);
  o << "(global ";
  curr->name.print(o) << ' ';
  emitGlobalType(curr);
  o << "))" << maybeNewLine;
}

void PrintSExpression::visitDefinedGlobal(Global* curr) {
  doIndent(o, indent);
  o << '(';
  printMedium(o, "global ");
  curr->name.print(o) << ' ';
  emitGlobalType(curr);
  o << ' ';
  visit(curr->init);
  o << ')';
  o << maybeNewLine;
}

void PrintSExpression::visitFunction(Function* curr) {
  if (curr->imported()) {
    visitImportedFunction(curr);
  } else if (curr->body == nullptr) {
    // We are in the middle of parsing the module and have not parsed this
    // function's code yet. Skip it.
  } else {
    visitDefinedFunction(curr);
  }
}

void PrintSExpression::visitImportedFunction(Function* curr) {
  doIndent(o, indent);
  currFunction = curr;
  lastPrintedLocation = std::nullopt;
  o << '(';
  emitImportHeader(curr);
  handleSignature(curr);
  o << "))";
  o << maybeNewLine;
}

void PrintSExpression::visitDefinedFunction(Function* curr) {
  doIndent(o, indent);
  currFunction = curr;
  lastPrintedLocation = std::nullopt;
  lastPrintIndent = 0;
  if (currFunction->prologLocation) {
    printDebugLocation(*currFunction->prologLocation);
  }
  handleSignature(curr, true);
  incIndent();
  for (size_t i = curr->getVarIndexBase(); i < curr->getNumLocals(); i++) {
    doIndent(o, indent);
    o << '(';
    printMinor(o, "local ");
    printLocal(i, currFunction, o) << ' ';
    printType(curr->getLocalType(i)) << ')';
    o << maybeNewLine;
  }
  // Print the body.
  StackIR* stackIR = nullptr;
  if (moduleStackIR) {
    stackIR = moduleStackIR->getStackIROrNull(curr);
  }
  if (stackIR) {
    printStackIR(stackIR, *this);
  } else {
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
  }
  if (currFunction->epilogLocation) {
    // Print last debug location: mix of decIndent and printDebugLocation
    // logic.
    doIndent(o, indent);
    if (!minify) {
      indent--;
    }
    printDebugLocation(*currFunction->epilogLocation);
    o << ')';
  } else {
    decIndent();
  }
  o << maybeNewLine;
}

void PrintSExpression::visitTag(Tag* curr) {
  if (curr->imported()) {
    visitImportedTag(curr);
  } else {
    visitDefinedTag(curr);
  }
}

void PrintSExpression::visitImportedTag(Tag* curr) {
  doIndent(o, indent);
  o << '(';
  emitImportHeader(curr);
  o << "(tag ";
  curr->name.print(o);
  if (curr->sig.params != Type::none) {
    o << maybeSpace;
    printParamType(curr->sig.params);
  }
  if (curr->sig.results != Type::none) {
    o << maybeSpace;
    printResultType(curr->sig.results);
  }
  o << "))";
  o << maybeNewLine;
}

void PrintSExpression::visitDefinedTag(Tag* curr) {
  doIndent(o, indent);
  o << '(';
  printMedium(o, "tag ");
  curr->name.print(o);
  if (curr->sig.params != Type::none) {
    o << maybeSpace;
    printParamType(curr->sig.params);
  }
  if (curr->sig.results != Type::none) {
    o << maybeSpace;
    printResultType(curr->sig.results);
  }
  o << ")" << maybeNewLine;
}

void PrintSExpression::printTableHeader(Table* curr) {
  o << '(';
  printMedium(o, "table") << ' ';
  curr->name.print(o) << ' ';
  if (curr->is64()) {
    o << "i64 ";
  }
  o << curr->initial;
  if (curr->hasMax()) {
    o << ' ' << curr->max;
  }
  o << ' ';
  printType(curr->type) << ')';
}

void PrintSExpression::visitTable(Table* curr) {
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

void PrintSExpression::visitElementSegment(ElementSegment* curr) {
  bool usesExpressions = TableUtils::usesExpressions(curr, currModule);
  auto printElemType = [&]() {
    if (!usesExpressions) {
      o << "func";
    } else {
      printType(curr->type);
    }
  };

  doIndent(o, indent);
  o << '(';
  printMedium(o, "elem ");
  curr->name.print(o);

  if (curr->table.is()) {
    if (usesExpressions || currModule->tables.size() > 1) {
      // tableuse
      o << " (table ";
      curr->table.print(o);
      o << ")";
    }

    o << ' ';
    bool needExplicitOffset = Measurer{}.measure(curr->offset) > 1;
    if (needExplicitOffset) {
      o << "(offset ";
    }
    visit(curr->offset);
    if (needExplicitOffset) {
      o << ')';
    }

    if (usesExpressions || currModule->tables.size() > 1) {
      o << ' ';
      printElemType();
    }
  } else {
    o << ' ';
    printElemType();
  }

  if (!usesExpressions) {
    for (auto* entry : curr->data) {
      auto* refFunc = entry->cast<RefFunc>();
      o << ' ';
      refFunc->func.print(o);
    }
  } else {
    for (auto* entry : curr->data) {
      o << " (item ";
      visit(entry);
      o << ')';
    }
  }
  o << ')' << maybeNewLine;
}

void PrintSExpression::printMemoryHeader(Memory* curr) {
  o << '(';
  printMedium(o, "memory") << ' ';
  curr->name.print(o) << ' ';
  if (curr->is64()) {
    o << "i64 ";
  }
  o << curr->initial;
  if (curr->hasMax()) {
    o << ' ' << curr->max;
  }
  if (curr->shared) {
    printMedium(o, " shared");
  }
  o << ")";
}

void PrintSExpression::visitMemory(Memory* curr) {
  if (curr->imported()) {
    doIndent(o, indent);
    o << '(';
    emitImportHeader(curr);
    printMemoryHeader(curr);
    o << ')' << maybeNewLine;
  } else {
    doIndent(o, indent);
    printMemoryHeader(curr);
    o << '\n';
  }
}

void PrintSExpression::visitDataSegment(DataSegment* curr) {
  if (!curr->isPassive && !curr->offset) {
    // This data segment must have been created from the datacount section but
    // not parsed yet. Skip it.
    return;
  }
  doIndent(o, indent);
  o << '(';
  printMajor(o, "data ");
  curr->name.print(o);
  o << ' ';
  if (!curr->isPassive) {
    assert(!currModule || currModule->memories.size() > 0);
    if (!currModule || curr->memory != currModule->memories[0]->name) {
      o << "(memory ";
      curr->memory.print(o);
      o << ") ";
    }
    bool needExplicitOffset = Measurer{}.measure(curr->offset) > 1;
    if (needExplicitOffset) {
      o << "(offset ";
    }
    visit(curr->offset);
    if (needExplicitOffset) {
      o << ")";
    }
    o << ' ';
  }
  String::printEscaped(o, {curr->data.data(), curr->data.size()});
  o << ')' << maybeNewLine;
}

void PrintSExpression::printDylinkSection(
  const std::unique_ptr<DylinkSection>& dylinkSection) {
  doIndent(o, indent) << ";; dylink section\n";
  doIndent(o, indent) << ";;   memorysize: " << dylinkSection->memorySize
                      << '\n';
  doIndent(o, indent) << ";;   memoryalignment: "
                      << dylinkSection->memoryAlignment << '\n';
  doIndent(o, indent) << ";;   tablesize: " << dylinkSection->tableSize << '\n';
  doIndent(o, indent) << ";;   tablealignment: "
                      << dylinkSection->tableAlignment << '\n';
  for (auto& neededDynlib : dylinkSection->neededDynlibs) {
    doIndent(o, indent) << ";;   needed dynlib: " << neededDynlib << '\n';
  }
  if (dylinkSection->tail.size()) {
    doIndent(o, indent) << ";;   extra dylink data, size "
                        << dylinkSection->tail.size() << "\n";
  }
}

void PrintSExpression::visitModule(Module* curr) {
  setModule(curr);
  o << '(';
  printMajor(o, "module");
  if (curr->name.is()) {
    o << ' ';
    curr->name.print(o);
  }
  incIndent();

  // Use the same type order as the binary output would even though there is
  // no code size benefit in the text format.
  std::optional<RecGroup> currGroup;
  bool nontrivialGroup = false;
  auto finishGroup = [&]() {
    if (nontrivialGroup) {
      decIndent();
      o << maybeNewLine;
    }
  };
  for (auto type : heapTypes) {
    RecGroup newGroup = type.getRecGroup();
    if (!currGroup || *currGroup != newGroup) {
      if (currGroup) {
        finishGroup();
      }
      currGroup = newGroup;
      nontrivialGroup = currGroup->size() > 1;
      if (nontrivialGroup) {
        doIndent(o, indent);
        o << "(rec";
        incIndent();
      }
    }
    doIndent(o, indent);
    o << typePrinter(type) << maybeNewLine;
  }
  finishGroup();

  ModuleUtils::iterImportedMemories(
    *curr, [&](Memory* memory) { visitMemory(memory); });
  ModuleUtils::iterImportedTables(*curr,
                                  [&](Table* table) { visitTable(table); });
  ModuleUtils::iterImportedGlobals(
    *curr, [&](Global* global) { visitGlobal(global); });
  ModuleUtils::iterImportedFunctions(
    *curr, [&](Function* func) { visitFunction(func); });
  ModuleUtils::iterImportedTags(*curr, [&](Tag* tag) { visitTag(tag); });
  ModuleUtils::iterDefinedGlobals(*curr,
                                  [&](Global* global) { visitGlobal(global); });
  ModuleUtils::iterDefinedMemories(
    *curr, [&](Memory* memory) { visitMemory(memory); });
  for (auto& segment : curr->dataSegments) {
    visitDataSegment(segment.get());
  }
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
      o << ' ';
      name.print(o);
    }
    o << ')' << maybeNewLine;
  }
  ModuleUtils::iterDefinedTags(*curr, [&](Tag* tag) { visitTag(tag); });
  for (auto& child : curr->exports) {
    doIndent(o, indent);
    visitExport(child.get());
    o << maybeNewLine;
  }
  if (curr->start.is()) {
    doIndent(o, indent);
    o << '(';
    printMedium(o, "start") << ' ';
    curr->start.print(o) << ')';
    o << maybeNewLine;
  }
  ModuleUtils::iterDefinedFunctions(
    *curr, [&](Function* func) { visitFunction(func); });
  if (curr->dylinkSection) {
    printDylinkSection(curr->dylinkSection);
  }
  for (auto& section : curr->customSections) {
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
  setModule(nullptr);
}

// Prints out a module
class Printer : public Pass {
protected:
  std::ostream& o;

public:
  Printer() : o(std::cout) {}
  Printer(std::ostream* o) : o(*o) {}

  bool modifiesBinaryenIR() override { return false; }

  void run(Module* module) override {
    PrintSExpression print(o);
    print.setDebugInfo(getPassOptions().debugInfo);
    print.visitModule(module);
  }
};

Pass* createPrinterPass() { return new Printer(); }

// Prints out a minified module

class MinifiedPrinter : public Printer {
public:
  MinifiedPrinter() = default;
  MinifiedPrinter(std::ostream* o) : Printer(o) {}

  void run(Module* module) override {
    PrintSExpression print(o);
    print.setMinify(true);
    print.setDebugInfo(getPassOptions().debugInfo);
    print.visitModule(module);
  }
};

Pass* createMinifiedPrinterPass() { return new MinifiedPrinter(); }

// Prints out a module withough elision, i.e., the full ast

class FullPrinter : public Printer {
public:
  FullPrinter() = default;
  FullPrinter(std::ostream* o) : Printer(o) {}

  void run(Module* module) override {
    PrintSExpression print(o);
    print.setFull(true);
    print.setDebugInfo(getPassOptions().debugInfo);
    print.currModule = module;
    print.visitModule(module);
  }
};

Pass* createFullPrinterPass() { return new FullPrinter(); }

// Print Stack IR (if present)

class PrintStackIR : public Printer {
public:
  PrintStackIR() = default;
  PrintStackIR(std::ostream* o) : Printer(o) {}

  void run(Module* module) override {
    PrintSExpression print(o);
    print.setDebugInfo(getPassOptions().debugInfo);
    print.currModule = module;
    print.generateStackIR(getPassOptions());
    print.visitModule(module);
  }
};

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
  }
  print.visit(expression);
  if (full || isFullForced()) {
    o << " (; ";
    printTypeOrName(expression->type, o, wasm);
    o << " ;)";
  }
  return o;
}

static std::ostream&
printStackInst(StackInst* inst, std::ostream& o, Function* func) {
  PrintSExpression printer(o);
  switch (inst->op) {
    case StackInst::Basic:
    case StackInst::BlockBegin:
    case StackInst::IfBegin:
    case StackInst::LoopBegin:
    case StackInst::TryBegin: {
      PrintExpressionContents(printer).visit(inst->origin);
      break;
    }
    case StackInst::BlockEnd:
    case StackInst::IfEnd:
    case StackInst::LoopEnd:
    case StackInst::TryEnd: {
      printMedium(o, "end");
      o << " ;; type: ";
      printer.printType(inst->type);
      break;
    }
    case StackInst::IfElse: {
      printMedium(o, "else");
      break;
    }
    case StackInst::Catch: {
      // Because StackInst does not have info on which catch within a try this
      // is, we can't print the tag name.
      printMedium(o, "catch");
      break;
    }
    case StackInst::CatchAll: {
      printMedium(o, "catch_all");
      break;
    }
    case StackInst::Delegate: {
      printMedium(o, "delegate ");
      inst->origin->cast<Try>()->delegateTarget.print(o);
      break;
    }
    default:
      WASM_UNREACHABLE("unexpeted op");
  }
  return o;
}

static std::ostream& printStackIR(StackIR* ir, PrintSExpression& printer) {
  std::ostream& o = printer.o;
  size_t indent = printer.currFunction ? 2 : 0;
  auto doIndent = [&]() { o << std::string(indent, ' '); };

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

        PrintExpressionContents(printer).visit(inst->origin);
        break;
      }
      case StackInst::TryBegin:
        catchIndexStack.push_back(0);
        [[fallthrough]];
      case StackInst::BlockBegin:
      case StackInst::IfBegin:
      case StackInst::LoopBegin:
      case StackInst::TryTableBegin: {
        controlFlowDepth++;
        doIndent();
        PrintExpressionContents(printer).visit(inst->origin);
        indent++;
        break;
      }
      case StackInst::TryEnd:
        catchIndexStack.pop_back();
        [[fallthrough]];
      case StackInst::BlockEnd:
      case StackInst::IfEnd:
      case StackInst::LoopEnd:
      case StackInst::TryTableEnd: {
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
        curr->catchTags[catchIndexStack.back()++].print(o);
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
          curr->delegateTarget.print(o);
        }
        break;
      }
      default:
        WASM_UNREACHABLE("unexpeted op");
    }
    o << '\n';
  }
  assert(controlFlowDepth == 0);
  return o;
}

std::ostream&
printStackIR(std::ostream& o, Module* module, const PassOptions& options) {
  wasm::PassRunner runner(module, options);
  runner.add(std::make_unique<PrintStackIR>(&o));
  runner.run();
  return o;
}

} // namespace wasm

namespace std {

std::ostream& operator<<(std::ostream& o, wasm::Module& module) {
  wasm::PassRunner runner(&module);
  wasm::Printer printer(&o);
  // Do not use runner.run(), since that will cause an infinite recursion in
  // BINARYEN_PASS_DEBUG=3, which prints modules (using this function) as part
  // of running passes.
  printer.setPassRunner(&runner);
  printer.run(&module);
  return o;
}

std::ostream& operator<<(std::ostream& o, wasm::Function& func) {
  wasm::PrintSExpression print(o);
  print.setMinify(false);
  print.setDebugInfo(false);
  print.visitFunction(&func);
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

std::ostream& operator<<(std::ostream& o, wasm::ShallowExpression expression) {
  wasm::PrintSExpression printer(o);
  printer.setModule(expression.module);
  wasm::PrintExpressionContents(printer).visit(expression.expr);
  return o;
}

std::ostream& operator<<(std::ostream& o, wasm::StackInst& inst) {
  return wasm::printStackInst(&inst, o);
}

std::ostream& operator<<(std::ostream& o, wasm::ModuleType pair) {
  wasm::printTypeOrName(pair.second, o, &pair.first);
  return o;
}

} // namespace std
