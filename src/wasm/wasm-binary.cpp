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

#include <algorithm>
#include <fstream>
#include <iomanip>

#include "ir/eh-utils.h"
#include "ir/module-utils.h"
#include "ir/names.h"
#include "ir/table-utils.h"
#include "ir/type-updating.h"
#include "pass.h"
#include "support/bits.h"
#include "support/debug.h"
#include "support/stdckdint.h"
#include "support/string.h"
#include "wasm-binary.h"
#include "wasm-debug.h"
#include "wasm-limits.h"
#include "wasm-stack.h"

#define DEBUG_TYPE "binary"

namespace wasm {

void WasmBinaryWriter::prepare() {
  // Collect function types and their frequencies. Collect information in each
  // function in parallel, then merge.
  indexedTypes = ModuleUtils::getOptimizedIndexedHeapTypes(*wasm);
  for (Index i = 0, size = indexedTypes.types.size(); i < size; ++i) {
    if (indexedTypes.types[i].isSignature()) {
      signatureIndexes.insert({indexedTypes.types[i].getSignature(), i});
    }
  }
  importInfo = std::make_unique<ImportInfo>(*wasm);
}

void WasmBinaryWriter::write() {
  writeHeader();

  writeDylinkSection();

  initializeDebugInfo();
  if (sourceMap) {
    writeSourceMapProlog();
  }

  writeTypes();
  writeImports();
  writeFunctionSignatures();
  writeTableDeclarations();
  writeMemories();
  writeTags();
  if (wasm->features.hasStrings()) {
    writeStrings();
  }
  writeGlobals();
  writeExports();
  writeStart();
  writeElementSegments();
  writeDataCount();
  writeFunctions();
  writeDataSegments();
  if (debugInfo || emitModuleName) {
    writeNames();
  }
  if (sourceMap && !sourceMapUrl.empty()) {
    writeSourceMapUrl();
  }
  if (symbolMap.size() > 0) {
    writeSymbolMap();
  }

  if (sourceMap) {
    writeSourceMapEpilog();
  }

#ifdef BUILD_LLVM_DWARF
  // Update DWARF user sections after writing the data they refer to
  // (function bodies), and before writing the user sections themselves.
  if (Debug::hasDWARFSections(*wasm)) {
    Debug::writeDWARFSections(*wasm, binaryLocations);
  }
#endif

  writeLateCustomSections();
  writeFeaturesSection();
}

void WasmBinaryWriter::writeHeader() {
  o << int32_t(BinaryConsts::Magic); // magic number \0asm
  o << int32_t(BinaryConsts::Version);
}

int32_t WasmBinaryWriter::writeU32LEBPlaceholder() {
  int32_t ret = o.size();
  o << int32_t(0);
  o << int8_t(0);
  return ret;
}

void WasmBinaryWriter::writeResizableLimits(
  Address initial, Address maximum, bool hasMaximum, bool shared, bool is64) {
  uint32_t flags = (hasMaximum ? (uint32_t)BinaryConsts::HasMaximum : 0U) |
                   (shared ? (uint32_t)BinaryConsts::IsShared : 0U) |
                   (is64 ? (uint32_t)BinaryConsts::Is64 : 0U);
  o << U32LEB(flags);
  if (is64) {
    o << U64LEB(initial);
    if (hasMaximum) {
      o << U64LEB(maximum);
    }
  } else {
    o << U32LEB(initial);
    if (hasMaximum) {
      o << U32LEB(maximum);
    }
  }
}

template<typename T> int32_t WasmBinaryWriter::startSection(T code) {
  o << uint8_t(code);
  if (sourceMap) {
    sourceMapLocationsSizeAtSectionStart = sourceMapLocations.size();
  }
  binaryLocationsSizeAtSectionStart = binaryLocations.expressions.size();
  return writeU32LEBPlaceholder(); // section size to be filled in later
}

void WasmBinaryWriter::finishSection(int32_t start) {
  // section size does not include the reserved bytes of the size field itself
  int32_t size = o.size() - start - MaxLEB32Bytes;
  auto sizeFieldSize = o.writeAt(start, U32LEB(size));
  // We can move things back if the actual LEB for the size doesn't use the
  // maximum 5 bytes. In that case we need to adjust offsets after we move
  // things backwards.
  auto adjustmentForLEBShrinking = MaxLEB32Bytes - sizeFieldSize;
  if (adjustmentForLEBShrinking) {
    // we can save some room, nice
    assert(sizeFieldSize < MaxLEB32Bytes);
    std::move(&o[start] + MaxLEB32Bytes,
              &o[start] + MaxLEB32Bytes + size,
              &o[start] + sizeFieldSize);
    o.resize(o.size() - adjustmentForLEBShrinking);
    if (sourceMap) {
      for (auto i = sourceMapLocationsSizeAtSectionStart;
           i < sourceMapLocations.size();
           ++i) {
        sourceMapLocations[i].first -= adjustmentForLEBShrinking;
      }
    }
  }

  if (binaryLocationsSizeAtSectionStart != binaryLocations.expressions.size()) {
    // We added the binary locations, adjust them: they must be relative
    // to the code section.
    assert(binaryLocationsSizeAtSectionStart == 0);
    // The section type byte is right before the LEB for the size; we want
    // offsets that are relative to the body, which is after that section type
    // byte and the the size LEB.
    auto body = start + sizeFieldSize;
    // Offsets are relative to the body of the code section: after the
    // section type byte and the size.
    // Everything was moved by the adjustment, track that. After this,
    // we are at the right absolute address.
    // We are relative to the section start.
    auto totalAdjustment = adjustmentForLEBShrinking + body;
    for (auto& [_, locations] : binaryLocations.expressions) {
      locations.start -= totalAdjustment;
      locations.end -= totalAdjustment;
    }
    for (auto& [_, locations] : binaryLocations.functions) {
      locations.start -= totalAdjustment;
      locations.declarations -= totalAdjustment;
      locations.end -= totalAdjustment;
    }
    for (auto& [_, locations] : binaryLocations.delimiters) {
      for (auto& item : locations) {
        item -= totalAdjustment;
      }
    }
  }
}

int32_t WasmBinaryWriter::startSubsection(
  BinaryConsts::CustomSections::Subsection code) {
  return startSection(code);
}

void WasmBinaryWriter::finishSubsection(int32_t start) { finishSection(start); }

void WasmBinaryWriter::writeStart() {
  if (!wasm->start.is()) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Start);
  o << U32LEB(getFunctionIndex(wasm->start.str));
  finishSection(start);
}

void WasmBinaryWriter::writeMemories() {
  if (importInfo->getNumDefinedMemories() == 0) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Memory);
  auto num = importInfo->getNumDefinedMemories();
  o << U32LEB(num);
  ModuleUtils::iterDefinedMemories(*wasm, [&](Memory* memory) {
    writeResizableLimits(memory->initial,
                         memory->max,
                         memory->hasMax(),
                         memory->shared,
                         memory->is64());
  });
  finishSection(start);
}

void WasmBinaryWriter::writeTypes() {
  if (indexedTypes.types.size() == 0) {
    return;
  }
  // Count the number of recursion groups, which is the number of elements in
  // the type section.
  size_t numGroups = 0;
  {
    std::optional<RecGroup> lastGroup;
    for (auto type : indexedTypes.types) {
      auto currGroup = type.getRecGroup();
      numGroups += lastGroup != currGroup;
      lastGroup = currGroup;
    }
  }

  // As a temporary measure, detect which types have subtypes and always use
  // `sub` or `sub final` for these types. The standard says that types without
  // `sub` or `sub final` are final, but we currently treat them as non-final.
  // To avoid unsafe ambiguity, only use the short form for types that it would
  // be safe to treat as final, i.e. types without subtypes.
  std::vector<bool> hasSubtypes(indexedTypes.types.size());
  for (auto type : indexedTypes.types) {
    if (auto super = type.getDeclaredSuperType()) {
      hasSubtypes[indexedTypes.indices[*super]] = true;
    }
  }

  auto start = startSection(BinaryConsts::Section::Type);
  o << U32LEB(numGroups);
  std::optional<RecGroup> lastGroup = std::nullopt;
  for (Index i = 0; i < indexedTypes.types.size(); ++i) {
    auto type = indexedTypes.types[i];
    // Check whether we need to start a new recursion group. Recursion groups of
    // size 1 are implicit, so only emit a group header for larger groups.
    auto currGroup = type.getRecGroup();
    if (lastGroup != currGroup && currGroup.size() > 1) {
      o << uint8_t(BinaryConsts::EncodedType::Rec) << U32LEB(currGroup.size());
    }
    lastGroup = currGroup;
    // Emit the type definition.
    auto super = type.getDeclaredSuperType();
    if (super || type.isOpen()) {
      if (type.isOpen()) {
        o << uint8_t(BinaryConsts::EncodedType::Sub);
      } else {
        o << uint8_t(BinaryConsts::EncodedType::SubFinal);
      }
      if (super) {
        o << U32LEB(1);
        writeHeapType(*super);
      } else {
        o << U32LEB(0);
      }
    }
    if (type.isShared()) {
      o << uint8_t(BinaryConsts::EncodedType::SharedDef);
    }
    switch (type.getKind()) {
      case HeapTypeKind::Func: {
        o << uint8_t(BinaryConsts::EncodedType::Func);
        auto sig = type.getSignature();
        for (auto& sigType : {sig.params, sig.results}) {
          o << U32LEB(sigType.size());
          for (const auto& type : sigType) {
            writeType(type);
          }
        }
        break;
      }
      case HeapTypeKind::Struct: {
        o << uint8_t(BinaryConsts::EncodedType::Struct);
        auto fields = type.getStruct().fields;
        o << U32LEB(fields.size());
        for (const auto& field : fields) {
          writeField(field);
        }
        break;
      }
      case HeapTypeKind::Array:
        o << uint8_t(BinaryConsts::EncodedType::Array);
        writeField(type.getArray().element);
        break;
      case HeapTypeKind::Cont:
        o << uint8_t(BinaryConsts::EncodedType::Cont);
        writeHeapType(type.getContinuation().type);
        break;
      case HeapTypeKind::Basic:
        WASM_UNREACHABLE("unexpected kind");
    }
  }
  finishSection(start);
}

void WasmBinaryWriter::writeImports() {
  auto num = importInfo->getNumImports();
  if (num == 0) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Import);
  o << U32LEB(num);
  auto writeImportHeader = [&](Importable* import) {
    writeInlineString(import->module.str);
    writeInlineString(import->base.str);
  };
  ModuleUtils::iterImportedFunctions(*wasm, [&](Function* func) {
    writeImportHeader(func);
    o << U32LEB(int32_t(ExternalKind::Function));
    o << U32LEB(getTypeIndex(func->type));
  });
  ModuleUtils::iterImportedGlobals(*wasm, [&](Global* global) {
    writeImportHeader(global);
    o << U32LEB(int32_t(ExternalKind::Global));
    writeType(global->type);
    o << U32LEB(global->mutable_);
  });
  ModuleUtils::iterImportedTags(*wasm, [&](Tag* tag) {
    writeImportHeader(tag);
    o << U32LEB(int32_t(ExternalKind::Tag));
    o << uint8_t(0); // Reserved 'attribute' field. Always 0.
    o << U32LEB(getTypeIndex(tag->sig));
  });
  ModuleUtils::iterImportedMemories(*wasm, [&](Memory* memory) {
    writeImportHeader(memory);
    o << U32LEB(int32_t(ExternalKind::Memory));
    writeResizableLimits(memory->initial,
                         memory->max,
                         memory->hasMax(),
                         memory->shared,
                         memory->is64());
  });
  ModuleUtils::iterImportedTables(*wasm, [&](Table* table) {
    writeImportHeader(table);
    o << U32LEB(int32_t(ExternalKind::Table));
    writeType(table->type);
    writeResizableLimits(table->initial,
                         table->max,
                         table->hasMax(),
                         /*shared=*/false,
                         table->is64());
  });
  finishSection(start);
}

void WasmBinaryWriter::writeFunctionSignatures() {
  if (importInfo->getNumDefinedFunctions() == 0) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Function);
  o << U32LEB(importInfo->getNumDefinedFunctions());
  ModuleUtils::iterDefinedFunctions(
    *wasm, [&](Function* func) { o << U32LEB(getTypeIndex(func->type)); });
  finishSection(start);
}

void WasmBinaryWriter::writeExpression(Expression* curr) {
  BinaryenIRToBinaryWriter(*this, o).visit(curr);
}

void WasmBinaryWriter::writeFunctions() {
  if (importInfo->getNumDefinedFunctions() == 0) {
    return;
  }

  std::optional<ModuleStackIR> moduleStackIR;
  if (options.generateStackIR) {
    moduleStackIR.emplace(*wasm, options);
  }

  auto sectionStart = startSection(BinaryConsts::Section::Code);
  o << U32LEB(importInfo->getNumDefinedFunctions());
  bool DWARF = Debug::hasDWARFSections(*getModule());
  ModuleUtils::iterDefinedFunctions(*wasm, [&](Function* func) {
    assert(binaryLocationTrackedExpressionsForFunc.empty());
    // Do not smear any debug location from the previous function.
    writeNoDebugLocation();
    size_t sourceMapLocationsSizeAtFunctionStart = sourceMapLocations.size();
    size_t sizePos = writeU32LEBPlaceholder();
    size_t start = o.size();
    // Emit Stack IR if present.
    StackIR* stackIR = nullptr;
    if (moduleStackIR) {
      stackIR = moduleStackIR->getStackIROrNull(func);
    }
    if (stackIR) {
      StackIRToBinaryWriter writer(*this, o, func, *stackIR, sourceMap, DWARF);
      writer.write();
      if (debugInfo) {
        funcMappedLocals[func->name] = std::move(writer.getMappedLocals());
      }
    } else {
      BinaryenIRToBinaryWriter writer(*this, o, func, sourceMap, DWARF);
      writer.write();
      if (debugInfo) {
        funcMappedLocals[func->name] = std::move(writer.getMappedLocals());
      }
    }
    size_t size = o.size() - start;
    assert(size <= std::numeric_limits<uint32_t>::max());
    auto sizeFieldSize = o.writeAt(sizePos, U32LEB(size));
    // We can move things back if the actual LEB for the size doesn't use the
    // maximum 5 bytes. In that case we need to adjust offsets after we move
    // things backwards.
    auto adjustmentForLEBShrinking = MaxLEB32Bytes - sizeFieldSize;
    if (adjustmentForLEBShrinking) {
      // we can save some room, nice
      assert(sizeFieldSize < MaxLEB32Bytes);
      std::move(&o[start], &o[start] + size, &o[sizePos] + sizeFieldSize);
      o.resize(o.size() - adjustmentForLEBShrinking);
      if (sourceMap) {
        for (auto i = sourceMapLocationsSizeAtFunctionStart;
             i < sourceMapLocations.size();
             ++i) {
          sourceMapLocations[i].first -= adjustmentForLEBShrinking;
        }
      }
      for (auto* curr : binaryLocationTrackedExpressionsForFunc) {
        // We added the binary locations, adjust them: they must be relative
        // to the code section.
        auto& span = binaryLocations.expressions[curr];
        span.start -= adjustmentForLEBShrinking;
        span.end -= adjustmentForLEBShrinking;
        auto iter = binaryLocations.delimiters.find(curr);
        if (iter != binaryLocations.delimiters.end()) {
          for (auto& item : iter->second) {
            item -= adjustmentForLEBShrinking;
          }
        }
      }
    }
    if (!binaryLocationTrackedExpressionsForFunc.empty()) {
      binaryLocations.functions[func] = BinaryLocations::FunctionLocations{
        BinaryLocation(sizePos),
        BinaryLocation(start - adjustmentForLEBShrinking),
        BinaryLocation(o.size())};
    }
    tableOfContents.functionBodies.emplace_back(
      func->name, sizePos + sizeFieldSize, size);
    binaryLocationTrackedExpressionsForFunc.clear();

    if (func->getParams().size() > WebLimitations::MaxFunctionParams) {
      std::cerr << "Some VMs may not accept this binary because it has a large "
                << "number of parameters in function " << func->name << ".\n";
    }
    if (func->getNumLocals() > WebLimitations::MaxFunctionLocals) {
      std::cerr << "Some VMs may not accept this binary because it has a large "
                << "number of locals in function " << func->name << ".\n";
    }
  });
  finishSection(sectionStart);
}

void WasmBinaryWriter::writeStrings() {
  assert(wasm->features.hasStrings());

  // Scan the entire wasm to find the relevant strings.
  // To find all the string literals we must scan all the code.
  using StringSet = std::unordered_set<Name>;

  struct StringWalker : public PostWalker<StringWalker> {
    StringSet& strings;

    StringWalker(StringSet& strings) : strings(strings) {}

    void visitStringConst(StringConst* curr) { strings.insert(curr->string); }
  };

  ModuleUtils::ParallelFunctionAnalysis<StringSet> analysis(
    *wasm, [&](Function* func, StringSet& strings) {
      if (!func->imported()) {
        StringWalker(strings).walk(func->body);
      }
    });

  // Also walk the global module code (for simplicity, also add it to the
  // function map, using a "function" key of nullptr).
  auto& globalStrings = analysis.map[nullptr];
  StringWalker(globalStrings).walkModuleCode(wasm);

  // Generate the indexes from the combined set of necessary strings,
  // which we sort for determinism.
  StringSet allStrings;
  for (auto& [func, strings] : analysis.map) {
    for (auto& string : strings) {
      allStrings.insert(string);
    }
  }
  std::vector<Name> sorted;
  for (auto& string : allStrings) {
    sorted.push_back(string);
  }
  std::sort(sorted.begin(), sorted.end());
  for (Index i = 0; i < sorted.size(); i++) {
    stringIndexes[sorted[i]] = i;
  }

  auto num = sorted.size();
  if (num == 0) {
    return;
  }

  auto start = startSection(BinaryConsts::Section::Strings);

  // Placeholder for future use in the spec.
  o << U32LEB(0);

  // The number of strings and then their contents.
  o << U32LEB(num);
  for (auto& string : sorted) {
    // Re-encode from WTF-16 to WTF-8.
    std::stringstream wtf8;
    [[maybe_unused]] bool valid = String::convertWTF16ToWTF8(wtf8, string.str);
    assert(valid);
    // TODO: Use wtf8.view() once we have C++20.
    writeInlineString(wtf8.str());
  }

  finishSection(start);
}

void WasmBinaryWriter::writeGlobals() {
  if (importInfo->getNumDefinedGlobals() == 0) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Global);
  // Count and emit the total number of globals after tuple globals have been
  // expanded into their constituent parts.
  Index num = 0;
  ModuleUtils::iterDefinedGlobals(
    *wasm, [&num](Global* global) { num += global->type.size(); });
  o << U32LEB(num);
  ModuleUtils::iterDefinedGlobals(*wasm, [&](Global* global) {
    size_t i = 0;
    for (const auto& t : global->type) {
      writeType(t);
      o << U32LEB(global->mutable_);
      if (global->type.size() == 1) {
        writeExpression(global->init);
      } else if (auto* make = global->init->dynCast<TupleMake>()) {
        // Emit the proper lane for this global.
        writeExpression(make->operands[i]);
      } else {
        // For now tuple globals must contain tuple.make. We could perhaps
        // support more operations, like global.get, but the code would need to
        // look something like this:
        //
        //   auto parentIndex = getGlobalIndex(get->name);
        //   o << int8_t(BinaryConsts::GlobalGet) << U32LEB(parentIndex + i);
        //
        // That is, we must emit the instruction here, and not defer to
        // writeExpression, as writeExpression writes an entire expression at a
        // time (and not just one of the lanes). As emitting an instruction here
        // is less clean, and there is no important use case for global.get of
        // one tuple global to another, we disallow this.
        WASM_UNREACHABLE("unsupported tuple global operation");
      }
      o << int8_t(BinaryConsts::End);
      ++i;
    }
  });
  finishSection(start);
}

void WasmBinaryWriter::writeExports() {
  if (wasm->exports.size() == 0) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Export);
  o << U32LEB(wasm->exports.size());
  for (auto& curr : wasm->exports) {
    writeInlineString(curr->name.str);
    o << U32LEB(int32_t(curr->kind));
    switch (curr->kind) {
      case ExternalKind::Function:
        o << U32LEB(getFunctionIndex(curr->value));
        break;
      case ExternalKind::Table:
        o << U32LEB(getTableIndex(curr->value));
        break;
      case ExternalKind::Memory:
        o << U32LEB(getMemoryIndex(curr->value));
        break;
      case ExternalKind::Global:
        o << U32LEB(getGlobalIndex(curr->value));
        break;
      case ExternalKind::Tag:
        o << U32LEB(getTagIndex(curr->value));
        break;
      default:
        WASM_UNREACHABLE("unexpected extern kind");
    }
  }
  finishSection(start);
}

void WasmBinaryWriter::writeDataCount() {
  if (!wasm->features.hasBulkMemory() || !wasm->dataSegments.size()) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::DataCount);
  o << U32LEB(wasm->dataSegments.size());
  finishSection(start);
}

void WasmBinaryWriter::writeDataSegments() {
  if (wasm->dataSegments.size() == 0) {
    return;
  }
  if (wasm->dataSegments.size() > WebLimitations::MaxDataSegments) {
    std::cerr << "Some VMs may not accept this binary because it has a large "
              << "number of data segments. Run the limit-segments pass to "
              << "merge segments.\n";
  }
  auto start = startSection(BinaryConsts::Section::Data);
  o << U32LEB(wasm->dataSegments.size());
  for (auto& segment : wasm->dataSegments) {
    uint32_t flags = 0;
    Index memoryIndex = 0;
    if (segment->isPassive) {
      flags |= BinaryConsts::IsPassive;
    } else {
      memoryIndex = getMemoryIndex(segment->memory);
      if (memoryIndex) {
        flags |= BinaryConsts::HasIndex;
      }
    }
    o << U32LEB(flags);
    if (!segment->isPassive) {
      if (memoryIndex) {
        o << U32LEB(memoryIndex);
      }
      writeExpression(segment->offset);
      o << int8_t(BinaryConsts::End);
    }
    writeInlineBuffer(segment->data.data(), segment->data.size());
  }
  finishSection(start);
}

uint32_t WasmBinaryWriter::getFunctionIndex(Name name) const {
  auto it = indexes.functionIndexes.find(name);
  assert(it != indexes.functionIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getTableIndex(Name name) const {
  auto it = indexes.tableIndexes.find(name);
  assert(it != indexes.tableIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getMemoryIndex(Name name) const {
  auto it = indexes.memoryIndexes.find(name);
  assert(it != indexes.memoryIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getGlobalIndex(Name name) const {
  auto it = indexes.globalIndexes.find(name);
  assert(it != indexes.globalIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getTagIndex(Name name) const {
  auto it = indexes.tagIndexes.find(name);
  assert(it != indexes.tagIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getDataSegmentIndex(Name name) const {
  auto it = indexes.dataIndexes.find(name);
  assert(it != indexes.dataIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getElementSegmentIndex(Name name) const {
  auto it = indexes.elemIndexes.find(name);
  assert(it != indexes.elemIndexes.end());
  return it->second;
}

uint32_t WasmBinaryWriter::getTypeIndex(HeapType type) const {
  auto it = indexedTypes.indices.find(type);
#ifndef NDEBUG
  if (it == indexedTypes.indices.end()) {
    std::cout << "Missing type: " << type << '\n';
    assert(0);
  }
#endif
  return it->second;
}

uint32_t WasmBinaryWriter::getSignatureIndex(Signature sig) const {
  auto it = signatureIndexes.find(sig);
#ifndef NDEBUG
  if (it == signatureIndexes.end()) {
    std::cout << "Missing signature: " << sig << '\n';
    assert(0);
  }
#endif
  return it->second;
}

uint32_t WasmBinaryWriter::getStringIndex(Name string) const {
  auto it = stringIndexes.find(string);
  assert(it != stringIndexes.end());
  return it->second;
}

void WasmBinaryWriter::writeTableDeclarations() {
  if (importInfo->getNumDefinedTables() == 0) {
    // std::cerr << std::endl << "(WasmBinaryWriter::writeTableDeclarations) No
    // defined tables found. skipping" << std::endl;
    return;
  }
  auto start = startSection(BinaryConsts::Section::Table);
  auto num = importInfo->getNumDefinedTables();
  o << U32LEB(num);
  ModuleUtils::iterDefinedTables(*wasm, [&](Table* table) {
    writeType(table->type);
    writeResizableLimits(table->initial,
                         table->max,
                         table->hasMax(),
                         /*shared=*/false,
                         table->is64());
  });
  finishSection(start);
}

void WasmBinaryWriter::writeElementSegments() {
  size_t elemCount = wasm->elementSegments.size();
  auto needingElemDecl = TableUtils::getFunctionsNeedingElemDeclare(*wasm);
  if (!needingElemDecl.empty()) {
    elemCount++;
  }
  if (elemCount == 0) {
    return;
  }

  auto start = startSection(BinaryConsts::Section::Element);
  o << U32LEB(elemCount);

  Type funcref = Type(HeapType::func, Nullable);
  for (auto& segment : wasm->elementSegments) {
    Index tableIdx = 0;

    bool isPassive = segment->table.isNull();
    // If the segment is MVP, we can use the shorter form.
    bool usesExpressions = TableUtils::usesExpressions(segment.get(), wasm);

    // The table index can and should be elided for active segments of table 0
    // when table 0 has type funcref. This was the only type of segment
    // supported by the MVP, which also did not support table indices in the
    // segment encoding.
    bool hasTableIndex = false;
    if (!isPassive) {
      tableIdx = getTableIndex(segment->table);
      hasTableIndex =
        tableIdx > 0 || wasm->getTable(segment->table)->type != funcref;
    }

    uint32_t flags = 0;
    if (usesExpressions) {
      flags |= BinaryConsts::UsesExpressions;
    }
    if (isPassive) {
      flags |= BinaryConsts::IsPassive;
    } else if (hasTableIndex) {
      flags |= BinaryConsts::HasIndex;
    }

    o << U32LEB(flags);
    if (!isPassive) {
      if (hasTableIndex) {
        o << U32LEB(tableIdx);
      }
      writeExpression(segment->offset);
      o << int8_t(BinaryConsts::End);
    }

    if (isPassive || hasTableIndex) {
      if (usesExpressions) {
        // elemType
        writeType(segment->type);
      } else {
        // MVP elemKind of funcref
        o << U32LEB(0);
      }
    }
    o << U32LEB(segment->data.size());
    if (usesExpressions) {
      for (auto* item : segment->data) {
        writeExpression(item);
        o << int8_t(BinaryConsts::End);
      }
    } else {
      for (auto& item : segment->data) {
        // We've ensured that all items are ref.func.
        auto& name = item->cast<RefFunc>()->func;
        o << U32LEB(getFunctionIndex(name));
      }
    }
  }

  if (!needingElemDecl.empty()) {
    o << U32LEB(BinaryConsts::IsPassive | BinaryConsts::IsDeclarative);
    o << U32LEB(0); // type (indicating funcref)
    o << U32LEB(needingElemDecl.size());
    for (auto name : needingElemDecl) {
      o << U32LEB(indexes.functionIndexes[name]);
    }
  }

  finishSection(start);
}

void WasmBinaryWriter::writeTags() {
  if (importInfo->getNumDefinedTags() == 0) {
    return;
  }
  auto start = startSection(BinaryConsts::Section::Tag);
  auto num = importInfo->getNumDefinedTags();
  o << U32LEB(num);
  ModuleUtils::iterDefinedTags(*wasm, [&](Tag* tag) {
    o << uint8_t(0); // Reserved 'attribute' field. Always 0.
    o << U32LEB(getTypeIndex(tag->sig));
  });

  finishSection(start);
}

void WasmBinaryWriter::writeNames() {
  auto start = startSection(BinaryConsts::Section::Custom);
  writeInlineString(BinaryConsts::CustomSections::Name);

  // module name
  if (emitModuleName && wasm->name.is()) {
    auto substart =
      startSubsection(BinaryConsts::CustomSections::Subsection::NameModule);
    writeEscapedName(wasm->name.str);
    finishSubsection(substart);
  }

  if (!debugInfo) {
    // We were only writing the module name.
    finishSection(start);
    return;
  }

  // function names
  {
    std::vector<std::pair<Index, Function*>> functionsWithNames;
    Index checked = 0;
    auto check = [&](Function* curr) {
      if (curr->hasExplicitName) {
        functionsWithNames.push_back({checked, curr});
      }
      checked++;
    };
    ModuleUtils::iterImportedFunctions(*wasm, check);
    ModuleUtils::iterDefinedFunctions(*wasm, check);
    assert(checked == indexes.functionIndexes.size());
    if (functionsWithNames.size() > 0) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameFunction);
      o << U32LEB(functionsWithNames.size());
      for (auto& [index, global] : functionsWithNames) {
        o << U32LEB(index);
        writeEscapedName(global->name.str);
      }
      finishSubsection(substart);
    }
  }

  // local names
  {
    // Find all functions with at least one local name and only emit the
    // subsection if there is at least one.
    std::vector<std::pair<Index, Function*>> functionsWithLocalNames;
    Index checked = 0;
    auto check = [&](Function* curr) {
      auto numLocals = curr->getNumLocals();
      for (Index i = 0; i < numLocals; ++i) {
        if (curr->hasLocalName(i)) {
          functionsWithLocalNames.push_back({checked, curr});
          break;
        }
      }
      checked++;
    };
    ModuleUtils::iterImportedFunctions(*wasm, check);
    ModuleUtils::iterDefinedFunctions(*wasm, check);
    assert(checked == indexes.functionIndexes.size());
    if (functionsWithLocalNames.size() > 0) {
      // Otherwise emit those functions but only include locals with a name.
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameLocal);
      o << U32LEB(functionsWithLocalNames.size());
      Index emitted = 0;
      for (auto& [index, func] : functionsWithLocalNames) {
        // Pairs of (local index in IR, name).
        std::vector<std::pair<Index, Name>> localsWithNames;
        auto numLocals = func->getNumLocals();
        for (Index indexInFunc = 0; indexInFunc < numLocals; ++indexInFunc) {
          if (func->hasLocalName(indexInFunc)) {
            Index indexInBinary;
            auto iter = funcMappedLocals.find(func->name);
            if (iter != funcMappedLocals.end()) {
              // TODO: handle multivalue
              indexInBinary = iter->second[{indexInFunc, 0}];
            } else {
              // No data on funcMappedLocals. That is only possible if we are an
              // imported function, where there are no locals to map, and in
              // that case the index is unchanged anyhow: parameters always have
              // the same index, they are not mapped in any way.
              assert(func->imported());
              indexInBinary = indexInFunc;
            }
            localsWithNames.push_back(
              {indexInBinary, func->getLocalName(indexInFunc)});
          }
        }
        assert(localsWithNames.size());
        std::sort(localsWithNames.begin(), localsWithNames.end());
        o << U32LEB(index);
        o << U32LEB(localsWithNames.size());
        for (auto& [indexInBinary, name] : localsWithNames) {
          o << U32LEB(indexInBinary);
          writeEscapedName(name.str);
        }
        emitted++;
      }
      assert(emitted == functionsWithLocalNames.size());
      finishSubsection(substart);
    }
  }

  // type names
  {
    std::vector<HeapType> namedTypes;
    for (auto type : indexedTypes.types) {
      if (wasm->typeNames.count(type) && wasm->typeNames[type].name.is()) {
        namedTypes.push_back(type);
      }
    }
    if (!namedTypes.empty()) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameType);
      o << U32LEB(namedTypes.size());
      for (auto type : namedTypes) {
        o << U32LEB(indexedTypes.indices[type]);
        writeEscapedName(wasm->typeNames[type].name.str);
      }
      finishSubsection(substart);
    }
  }

  // table names
  {
    std::vector<std::pair<Index, Table*>> tablesWithNames;
    Index checked = 0;
    auto check = [&](Table* curr) {
      if (curr->hasExplicitName) {
        tablesWithNames.push_back({checked, curr});
      }
      checked++;
    };
    ModuleUtils::iterImportedTables(*wasm, check);
    ModuleUtils::iterDefinedTables(*wasm, check);
    assert(checked == indexes.tableIndexes.size());

    if (tablesWithNames.size() > 0) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameTable);
      o << U32LEB(tablesWithNames.size());

      for (auto& [index, table] : tablesWithNames) {
        o << U32LEB(index);
        writeEscapedName(table->name.str);
      }

      finishSubsection(substart);
    }
  }

  // memory names
  {
    std::vector<std::pair<Index, Memory*>> memoriesWithNames;
    Index checked = 0;
    auto check = [&](Memory* curr) {
      if (curr->hasExplicitName) {
        memoriesWithNames.push_back({checked, curr});
      }
      checked++;
    };
    ModuleUtils::iterImportedMemories(*wasm, check);
    ModuleUtils::iterDefinedMemories(*wasm, check);
    assert(checked == indexes.memoryIndexes.size());
    if (memoriesWithNames.size() > 0) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameMemory);
      o << U32LEB(memoriesWithNames.size());
      for (auto& [index, memory] : memoriesWithNames) {
        o << U32LEB(index);
        writeEscapedName(memory->name.str);
      }
      finishSubsection(substart);
    }
  }

  // global names
  {
    std::vector<std::pair<Index, Global*>> globalsWithNames;
    Index checked = 0;
    auto check = [&](Global* curr) {
      if (curr->hasExplicitName) {
        globalsWithNames.push_back({checked, curr});
      }
      checked++;
    };
    ModuleUtils::iterImportedGlobals(*wasm, check);
    ModuleUtils::iterDefinedGlobals(*wasm, check);
    assert(checked == indexes.globalIndexes.size());
    if (globalsWithNames.size() > 0) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameGlobal);
      o << U32LEB(globalsWithNames.size());
      for (auto& [index, global] : globalsWithNames) {
        o << U32LEB(index);
        writeEscapedName(global->name.str);
      }
      finishSubsection(substart);
    }
  }

  // elem segment names
  {
    std::vector<std::pair<Index, ElementSegment*>> elemsWithNames;
    Index checked = 0;
    for (auto& curr : wasm->elementSegments) {
      if (curr->hasExplicitName) {
        elemsWithNames.push_back({checked, curr.get()});
      }
      checked++;
    }
    assert(checked == indexes.elemIndexes.size());

    if (elemsWithNames.size() > 0) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameElem);
      o << U32LEB(elemsWithNames.size());

      for (auto& [index, elem] : elemsWithNames) {
        o << U32LEB(index);
        writeEscapedName(elem->name.str);
      }

      finishSubsection(substart);
    }
  }

  // data segment names
  {
    Index count = 0;
    for (auto& seg : wasm->dataSegments) {
      if (seg->hasExplicitName) {
        count++;
      }
    }

    if (count) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameData);
      o << U32LEB(count);
      for (Index i = 0; i < wasm->dataSegments.size(); i++) {
        auto& seg = wasm->dataSegments[i];
        if (seg->hasExplicitName) {
          o << U32LEB(i);
          writeEscapedName(seg->name.str);
        }
      }
      finishSubsection(substart);
    }
  }

  // TODO: label, type, and element names
  // see: https://github.com/WebAssembly/extended-name-section

  // GC field names
  if (wasm->features.hasGC()) {
    std::vector<HeapType> relevantTypes;
    for (auto& type : indexedTypes.types) {
      if (type.isStruct() && wasm->typeNames.count(type) &&
          !wasm->typeNames[type].fieldNames.empty()) {
        relevantTypes.push_back(type);
      }
    }
    if (!relevantTypes.empty()) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameField);
      o << U32LEB(relevantTypes.size());
      for (Index i = 0; i < relevantTypes.size(); i++) {
        auto type = relevantTypes[i];
        o << U32LEB(indexedTypes.indices[type]);
        std::unordered_map<Index, Name>& fieldNames =
          wasm->typeNames.at(type).fieldNames;
        o << U32LEB(fieldNames.size());
        for (auto& [index, name] : fieldNames) {
          o << U32LEB(index);
          writeEscapedName(name.str);
        }
      }
      finishSubsection(substart);
    }
  }

  // tag names
  if (!wasm->tags.empty()) {
    Index count = 0;
    for (auto& tag : wasm->tags) {
      if (tag->hasExplicitName) {
        count++;
      }
    }

    if (count) {
      auto substart =
        startSubsection(BinaryConsts::CustomSections::Subsection::NameTag);
      o << U32LEB(count);
      for (Index i = 0; i < wasm->tags.size(); i++) {
        auto& tag = wasm->tags[i];
        if (tag->hasExplicitName) {
          o << U32LEB(i);
          writeEscapedName(tag->name.str);
        }
      }
      finishSubsection(substart);
    }
  }

  finishSection(start);
}

void WasmBinaryWriter::writeSourceMapUrl() {
  auto start = startSection(BinaryConsts::Section::Custom);
  writeInlineString(BinaryConsts::CustomSections::SourceMapUrl);
  writeInlineString(sourceMapUrl.c_str());
  finishSection(start);
}

void WasmBinaryWriter::writeSymbolMap() {
  std::ofstream file(symbolMap);
  auto write = [&](Function* func) {
    file << getFunctionIndex(func->name) << ":" << func->name.str << std::endl;
  };
  ModuleUtils::iterImportedFunctions(*wasm, write);
  ModuleUtils::iterDefinedFunctions(*wasm, write);
  file.close();
}

void WasmBinaryWriter::initializeDebugInfo() {
  lastDebugLocation = {0, /* lineNumber = */ 1, 0, std::nullopt};
}

void WasmBinaryWriter::writeSourceMapProlog() {
  *sourceMap << "{\"version\":3,";

  for (const auto& section : wasm->customSections) {
    if (section.name == BinaryConsts::CustomSections::BuildId) {
      U32LEB ret;
      size_t pos = 0;
      ret.read([&]() { return section.data[pos++]; });

      if (section.data.size() != pos + ret.value) {
        std::cerr
          << "warning: build id section with an incorrect size detected!\n";
        break;
      }

      *sourceMap << "\"debugId\":\"";
      for (size_t i = pos; i < section.data.size(); i++) {
        *sourceMap << std::setfill('0') << std::setw(2) << std::hex
                   << static_cast<int>(static_cast<uint8_t>(section.data[i]));
      }
      *sourceMap << "\",";
      break;
    }
  }

  *sourceMap << "\"sources\":[";
  for (size_t i = 0; i < wasm->debugInfoFileNames.size(); i++) {
    if (i > 0) {
      *sourceMap << ",";
    }
    // TODO respect JSON string encoding, e.g. quotes and control chars.
    *sourceMap << "\"" << wasm->debugInfoFileNames[i] << "\"";
  }
  *sourceMap << "],\"names\":[";

  for (size_t i = 0; i < wasm->debugInfoSymbolNames.size(); i++) {
    if (i > 0) {
      *sourceMap << ",";
    }
    // TODO respect JSON string encoding, e.g. quotes and control chars.
    *sourceMap << "\"" << wasm->debugInfoSymbolNames[i] << "\"";
  }

  *sourceMap << "],\"mappings\":\"";
}

static void writeBase64VLQ(std::ostream& out, int32_t n) {
  uint32_t value = n >= 0 ? n << 1 : ((-n) << 1) | 1;
  while (1) {
    uint32_t digit = value & 0x1F;
    value >>= 5;
    if (!value) {
      // last VLQ digit -- base64 codes 'A'..'Z', 'a'..'f'
      out << char(digit < 26 ? 'A' + digit : 'a' + digit - 26);
      break;
    }
    // more VLG digit will follow -- add continuation bit (0x20),
    // base64 codes 'g'..'z', '0'..'9', '+', '/'
    out << char(digit < 20
                  ? 'g' + digit
                  : digit < 30 ? '0' + digit - 20 : digit == 30 ? '+' : '/');
  }
}

void WasmBinaryWriter::writeSourceMapEpilog() {
  // write source map entries
  size_t lastOffset = 0;
  BinaryLocation lastFileIndex = 0;
  BinaryLocation lastLineNumber = 1;
  BinaryLocation lastColumnNumber = 0;
  BinaryLocation lastSymbolNameIndex = 0;
  for (const auto& [offset, loc] : sourceMapLocations) {
    if (lastOffset > 0) {
      *sourceMap << ",";
    }
    writeBase64VLQ(*sourceMap, int32_t(offset - lastOffset));
    lastOffset = offset;
    if (loc) {
      writeBase64VLQ(*sourceMap, int32_t(loc->fileIndex - lastFileIndex));
      lastFileIndex = loc->fileIndex;

      writeBase64VLQ(*sourceMap, int32_t(loc->lineNumber - lastLineNumber));
      lastLineNumber = loc->lineNumber;

      writeBase64VLQ(*sourceMap, int32_t(loc->columnNumber - lastColumnNumber));
      lastColumnNumber = loc->columnNumber;

      if (loc->symbolNameIndex) {
        writeBase64VLQ(*sourceMap,
                       int32_t(*loc->symbolNameIndex - lastSymbolNameIndex));
        lastSymbolNameIndex = *loc->symbolNameIndex;
      }
    }
  }
  *sourceMap << "\"}";
}

void WasmBinaryWriter::writeLateCustomSections() {
  for (auto& section : wasm->customSections) {
    if (section.name != BinaryConsts::CustomSections::Dylink) {
      writeCustomSection(section);
    }
  }
}

void WasmBinaryWriter::writeCustomSection(const CustomSection& section) {
  auto start = startSection(BinaryConsts::Custom);
  writeInlineString(section.name.c_str());
  for (size_t i = 0; i < section.data.size(); i++) {
    o << uint8_t(section.data[i]);
  }
  finishSection(start);
}

void WasmBinaryWriter::writeFeaturesSection() {
  if (!wasm->hasFeaturesSection || wasm->features.isMVP()) {
    return;
  }

  // TODO(tlively): unify feature names with rest of toolchain and use
  // FeatureSet::toString()
  auto toString = [](FeatureSet::Feature f) {
    switch (f) {
      case FeatureSet::Atomics:
        return BinaryConsts::CustomSections::AtomicsFeature;
      case FeatureSet::MutableGlobals:
        return BinaryConsts::CustomSections::MutableGlobalsFeature;
      case FeatureSet::TruncSat:
        return BinaryConsts::CustomSections::TruncSatFeature;
      case FeatureSet::SIMD:
        return BinaryConsts::CustomSections::SIMD128Feature;
      case FeatureSet::BulkMemory:
        return BinaryConsts::CustomSections::BulkMemoryFeature;
      case FeatureSet::SignExt:
        return BinaryConsts::CustomSections::SignExtFeature;
      case FeatureSet::ExceptionHandling:
        return BinaryConsts::CustomSections::ExceptionHandlingFeature;
      case FeatureSet::TailCall:
        return BinaryConsts::CustomSections::TailCallFeature;
      case FeatureSet::ReferenceTypes:
        return BinaryConsts::CustomSections::ReferenceTypesFeature;
      case FeatureSet::Multivalue:
        return BinaryConsts::CustomSections::MultivalueFeature;
      case FeatureSet::GC:
        return BinaryConsts::CustomSections::GCFeature;
      case FeatureSet::Memory64:
        return BinaryConsts::CustomSections::Memory64Feature;
      case FeatureSet::RelaxedSIMD:
        return BinaryConsts::CustomSections::RelaxedSIMDFeature;
      case FeatureSet::ExtendedConst:
        return BinaryConsts::CustomSections::ExtendedConstFeature;
      case FeatureSet::Strings:
        return BinaryConsts::CustomSections::StringsFeature;
      case FeatureSet::MultiMemory:
        return BinaryConsts::CustomSections::MultiMemoryFeature;
      case FeatureSet::StackSwitching:
        return BinaryConsts::CustomSections::StackSwitchingFeature;
      case FeatureSet::SharedEverything:
        return BinaryConsts::CustomSections::SharedEverythingFeature;
      case FeatureSet::FP16:
        return BinaryConsts::CustomSections::FP16Feature;
      case FeatureSet::BulkMemoryOpt:
        return BinaryConsts::CustomSections::BulkMemoryOptFeature;
      case FeatureSet::CallIndirectOverlong:
        return BinaryConsts::CustomSections::CallIndirectOverlongFeature;
      case FeatureSet::None:
      case FeatureSet::Default:
      case FeatureSet::All:
        break;
    }
    WASM_UNREACHABLE("unexpected feature flag");
  };

  std::vector<const char*> features;
  wasm->features.iterFeatures(
    [&](FeatureSet::Feature f) { features.push_back(toString(f)); });

  auto start = startSection(BinaryConsts::Custom);
  writeInlineString(BinaryConsts::CustomSections::TargetFeatures);
  o << U32LEB(features.size());
  for (auto& f : features) {
    o << uint8_t(BinaryConsts::FeatureUsed);
    writeInlineString(f);
  }
  finishSection(start);
}

void WasmBinaryWriter::writeLegacyDylinkSection() {
  if (!wasm->dylinkSection) {
    return;
  }

  auto start = startSection(BinaryConsts::Custom);
  writeInlineString(BinaryConsts::CustomSections::Dylink);
  o << U32LEB(wasm->dylinkSection->memorySize);
  o << U32LEB(wasm->dylinkSection->memoryAlignment);
  o << U32LEB(wasm->dylinkSection->tableSize);
  o << U32LEB(wasm->dylinkSection->tableAlignment);
  o << U32LEB(wasm->dylinkSection->neededDynlibs.size());
  for (auto& neededDynlib : wasm->dylinkSection->neededDynlibs) {
    writeInlineString(neededDynlib.str);
  }
  finishSection(start);
}

void WasmBinaryWriter::writeDylinkSection() {
  if (!wasm->dylinkSection) {
    return;
  }

  if (wasm->dylinkSection->isLegacy) {
    writeLegacyDylinkSection();
    return;
  }

  auto start = startSection(BinaryConsts::Custom);
  writeInlineString(BinaryConsts::CustomSections::Dylink0);

  auto substart =
    startSubsection(BinaryConsts::CustomSections::Subsection::DylinkMemInfo);
  o << U32LEB(wasm->dylinkSection->memorySize);
  o << U32LEB(wasm->dylinkSection->memoryAlignment);
  o << U32LEB(wasm->dylinkSection->tableSize);
  o << U32LEB(wasm->dylinkSection->tableAlignment);
  finishSubsection(substart);

  if (wasm->dylinkSection->neededDynlibs.size()) {
    substart =
      startSubsection(BinaryConsts::CustomSections::Subsection::DylinkNeeded);
    o << U32LEB(wasm->dylinkSection->neededDynlibs.size());
    for (auto& neededDynlib : wasm->dylinkSection->neededDynlibs) {
      writeInlineString(neededDynlib.str);
    }
    finishSubsection(substart);
  }

  writeData(wasm->dylinkSection->tail.data(), wasm->dylinkSection->tail.size());
  finishSection(start);
}

void WasmBinaryWriter::writeDebugLocation(const Function::DebugLocation& loc) {
  if (loc == lastDebugLocation) {
    return;
  }
  auto offset = o.size();
  sourceMapLocations.emplace_back(offset, &loc);
  lastDebugLocation = loc;
}

void WasmBinaryWriter::writeNoDebugLocation() {
  // Emit an indication that there is no debug location there (so that
  // we do not get "smeared" with debug info from anything before or
  // after us).
  //
  // We don't need to write repeated "no debug info" indications, as a
  // single one is enough to make it clear that the debug information
  // before us is valid no longer. We also don't need to write one if
  // there is nothing before us.
  if (!sourceMapLocations.empty() &&
      sourceMapLocations.back().second != nullptr) {
    sourceMapLocations.emplace_back(o.size(), nullptr);

    // Initialize the state of debug info to indicate there is no current
    // debug info relevant. This sets |lastDebugLocation| to a dummy value,
    // so that later places with debug info can see that they differ from
    // it (without this, if we had some debug info, then a nullptr for none,
    // and then the same debug info, we could get confused).
    initializeDebugInfo();
  }
}

void WasmBinaryWriter::writeDebugLocation(Expression* curr, Function* func) {
  if (sourceMap) {
    auto& debugLocations = func->debugLocations;
    auto iter = debugLocations.find(curr);
    if (iter != debugLocations.end() && iter->second) {
      // There is debug information here, write it out.
      writeDebugLocation(*(iter->second));
    } else {
      // This expression has no debug location.
      writeNoDebugLocation();
    }
  }
  // If this is an instruction in a function, and if the original wasm had
  // binary locations tracked, then track it in the output as well.
  if (func && !func->expressionLocations.empty()) {
    binaryLocations.expressions[curr] =
      BinaryLocations::Span{BinaryLocation(o.size()), 0};
    binaryLocationTrackedExpressionsForFunc.push_back(curr);
  }
}

void WasmBinaryWriter::writeDebugLocationEnd(Expression* curr, Function* func) {
  if (func && !func->expressionLocations.empty()) {
    auto& span = binaryLocations.expressions.at(curr);
    span.end = o.size();
  }
}

void WasmBinaryWriter::writeExtraDebugLocation(Expression* curr,
                                               Function* func,
                                               size_t id) {
  if (func && !func->expressionLocations.empty()) {
    binaryLocations.delimiters[curr][id] = o.size();
  }
}

void WasmBinaryWriter::writeData(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    o << int8_t(data[i]);
  }
}

void WasmBinaryWriter::writeInlineString(std::string_view name) {
  o << U32LEB(name.size());
  writeData(name.data(), name.size());
}

static bool isHexDigit(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
         (ch >= 'A' && ch <= 'F');
}

static int decodeHexNibble(char ch) {
  return ch <= '9' ? ch & 15 : (ch & 15) + 9;
}

void WasmBinaryWriter::writeEscapedName(std::string_view name) {
  if (name.find('\\') == std::string_view::npos) {
    writeInlineString(name);
    return;
  }
  // decode escaped by escapeName (see below) function names
  std::string unescaped;
  for (size_t i = 0; i < name.size();) {
    char ch = name[i++];
    // support only `\xx` escapes; ignore invalid or unsupported escapes
    if (ch != '\\' || i + 1 >= name.size() || !isHexDigit(name[i]) ||
        !isHexDigit(name[i + 1])) {
      unescaped.push_back(ch);
      continue;
    }
    unescaped.push_back(
      char((decodeHexNibble(name[i]) << 4) | decodeHexNibble(name[i + 1])));
    i += 2;
  }
  writeInlineString({unescaped.data(), unescaped.size()});
}

void WasmBinaryWriter::writeInlineBuffer(const char* data, size_t size) {
  o << U32LEB(size);
  writeData(data, size);
}

void WasmBinaryWriter::writeType(Type type) {
  if (type.isRef()) {
    // The only reference types allowed without GC are funcref, externref, and
    // exnref. We internally use more refined versions of those types, but we
    // cannot emit those without GC.
    if (!wasm->features.hasGC()) {
      auto ht = type.getHeapType();
      if (ht.isMaybeShared(HeapType::string)) {
        // Do not overgeneralize stringref to anyref. We have tests that when a
        // stringref is expected, we actually get a stringref. If we see a
        // string, the stringref feature must be enabled.
        type = Type(HeapTypes::string.getBasic(ht.getShared()), Nullable);
      } else {
        // Only the top type (func, extern, exn) is available, and only the
        // nullable version.
        type = Type(type.getHeapType().getTop(), Nullable);
      }
    }
    auto heapType = type.getHeapType();
    if (type.isNullable() && heapType.isBasic() && !heapType.isShared()) {
      switch (heapType.getBasic(Unshared)) {
        case HeapType::ext:
          o << S32LEB(BinaryConsts::EncodedType::externref);
          return;
        case HeapType::any:
          o << S32LEB(BinaryConsts::EncodedType::anyref);
          return;
        case HeapType::func:
          o << S32LEB(BinaryConsts::EncodedType::funcref);
          return;
        case HeapType::cont:
          o << S32LEB(BinaryConsts::EncodedType::contref);
          return;
        case HeapType::eq:
          o << S32LEB(BinaryConsts::EncodedType::eqref);
          return;
        case HeapType::i31:
          o << S32LEB(BinaryConsts::EncodedType::i31ref);
          return;
        case HeapType::struct_:
          o << S32LEB(BinaryConsts::EncodedType::structref);
          return;
        case HeapType::array:
          o << S32LEB(BinaryConsts::EncodedType::arrayref);
          return;
        case HeapType::exn:
          o << S32LEB(BinaryConsts::EncodedType::exnref);
          return;
        case HeapType::string:
          o << S32LEB(BinaryConsts::EncodedType::stringref);
          return;
        case HeapType::none:
          o << S32LEB(BinaryConsts::EncodedType::nullref);
          return;
        case HeapType::noext:
          o << S32LEB(BinaryConsts::EncodedType::nullexternref);
          return;
        case HeapType::nofunc:
          o << S32LEB(BinaryConsts::EncodedType::nullfuncref);
          return;
        case HeapType::noexn:
          o << S32LEB(BinaryConsts::EncodedType::nullexnref);
          return;
        case HeapType::nocont:
          o << S32LEB(BinaryConsts::EncodedType::nullcontref);
          return;
      }
    }
    if (type.isNullable()) {
      o << S32LEB(BinaryConsts::EncodedType::nullable);
    } else {
      o << S32LEB(BinaryConsts::EncodedType::nonnullable);
    }
    writeHeapType(type.getHeapType());
    return;
  }
  int ret = 0;
  TODO_SINGLE_COMPOUND(type);
  switch (type.getBasic()) {
    // None only used for block signatures. TODO: Separate out?
    case Type::none:
      ret = BinaryConsts::EncodedType::Empty;
      break;
    case Type::i32:
      ret = BinaryConsts::EncodedType::i32;
      break;
    case Type::i64:
      ret = BinaryConsts::EncodedType::i64;
      break;
    case Type::f32:
      ret = BinaryConsts::EncodedType::f32;
      break;
    case Type::f64:
      ret = BinaryConsts::EncodedType::f64;
      break;
    case Type::v128:
      ret = BinaryConsts::EncodedType::v128;
      break;
    default:
      WASM_UNREACHABLE("unexpected type");
  }
  o << S32LEB(ret);
}

void WasmBinaryWriter::writeHeapType(HeapType type) {
  // ref.null always has a bottom heap type in Binaryen IR, but those types are
  // only actually valid with GC. Otherwise, emit the corresponding valid top
  // types instead.
  if (!wasm->features.hasGC()) {
    type = type.getTop();
  }

  if (!type.isBasic()) {
    o << S64LEB(getTypeIndex(type)); // TODO: Actually s33
    return;
  }

  int ret = 0;
  if (type.isShared()) {
    o << S32LEB(BinaryConsts::EncodedType::Shared);
  }
  switch (type.getBasic(Unshared)) {
    case HeapType::ext:
      ret = BinaryConsts::EncodedHeapType::ext;
      break;
    case HeapType::func:
      ret = BinaryConsts::EncodedHeapType::func;
      break;
    case HeapType::cont:
      ret = BinaryConsts::EncodedHeapType::cont;
      break;
    case HeapType::any:
      ret = BinaryConsts::EncodedHeapType::any;
      break;
    case HeapType::eq:
      ret = BinaryConsts::EncodedHeapType::eq;
      break;
    case HeapType::i31:
      ret = BinaryConsts::EncodedHeapType::i31;
      break;
    case HeapType::struct_:
      ret = BinaryConsts::EncodedHeapType::struct_;
      break;
    case HeapType::array:
      ret = BinaryConsts::EncodedHeapType::array;
      break;
    case HeapType::exn:
      ret = BinaryConsts::EncodedHeapType::exn;
      break;
    case HeapType::string:
      ret = BinaryConsts::EncodedHeapType::string;
      break;
    case HeapType::none:
      ret = BinaryConsts::EncodedHeapType::none;
      break;
    case HeapType::noext:
      ret = BinaryConsts::EncodedHeapType::noext;
      break;
    case HeapType::nofunc:
      ret = BinaryConsts::EncodedHeapType::nofunc;
      break;
    case HeapType::noexn:
      ret = BinaryConsts::EncodedHeapType::noexn;
      break;
    case HeapType::nocont:
      ret = BinaryConsts::EncodedHeapType::nocont;
      break;
  }
  o << S64LEB(ret); // TODO: Actually s33
}

void WasmBinaryWriter::writeIndexedHeapType(HeapType type) {
  o << U32LEB(getTypeIndex(type));
}

void WasmBinaryWriter::writeField(const Field& field) {
  if (field.type == Type::i32 && field.packedType != Field::not_packed) {
    if (field.packedType == Field::i8) {
      o << S32LEB(BinaryConsts::EncodedType::i8);
    } else if (field.packedType == Field::i16) {
      o << S32LEB(BinaryConsts::EncodedType::i16);
    } else {
      WASM_UNREACHABLE("invalid packed type");
    }
  } else {
    writeType(field.type);
  }
  o << U32LEB(field.mutable_);
}

// reader

WasmBinaryReader::WasmBinaryReader(Module& wasm,
                                   FeatureSet features,
                                   const std::vector<char>& input,
                                   const std::vector<char>& sourceMap)
  : wasm(wasm), allocator(wasm.allocator), input(input), builder(wasm),
    sourceMapReader(sourceMap) {
  wasm.features = features;
}

bool WasmBinaryReader::hasDWARFSections() {
  assert(pos == 0);
  getInt32(); // magic
  getInt32(); // version
  bool has = false;
  while (more()) {
    uint8_t sectionCode = getInt8();
    uint32_t payloadLen = getU32LEB();
    if (uint64_t(pos) + uint64_t(payloadLen) > input.size()) {
      throwError("Section extends beyond end of input");
    }
    auto oldPos = pos;
    if (sectionCode == BinaryConsts::Section::Custom) {
      auto sectionName = getInlineString();
      if (Debug::isDWARFSection(sectionName)) {
        has = true;
        break;
      }
    }
    pos = oldPos + payloadLen;
  }
  pos = 0;
  return has;
}

void WasmBinaryReader::read() {
  if (DWARF) {
    // In order to update dwarf, we must store info about each IR node's
    // binary position. This has noticeable memory overhead, so we don't do it
    // by default: the user must request it by setting "DWARF", and even if so
    // we scan ahead to see that there actually *are* DWARF sections, so that
    // we don't do unnecessary work.
    if (!hasDWARFSections()) {
      DWARF = false;
    }
  }

  // Skip ahead and read the name section so we know what names to use when we
  // construct module elements.
  if (debugInfo) {
    findAndReadNames();
  }

  readHeader();
  sourceMapReader.readHeader(wasm);

  // Read sections until the end
  while (more()) {
    uint8_t sectionCode = getInt8();
    uint32_t payloadLen = getU32LEB();
    if (uint64_t(pos) + uint64_t(payloadLen) > input.size()) {
      throwError("Section extends beyond end of input");
    }

    auto oldPos = pos;

    // Note the section in the list of seen sections, as almost no sections can
    // appear more than once, and verify those that shouldn't do not.
    if (sectionCode != BinaryConsts::Section::Custom &&
        !seenSections.insert(sectionCode).second) {
      throwError("section seen more than once: " + std::to_string(sectionCode));
    }

    switch (sectionCode) {
      case BinaryConsts::Section::Start:
        readStart();
        break;
      case BinaryConsts::Section::Memory:
        readMemories();
        break;
      case BinaryConsts::Section::Type:
        readTypes();
        break;
      case BinaryConsts::Section::Import:
        readImports();
        break;
      case BinaryConsts::Section::Function:
        readFunctionSignatures();
        break;
      case BinaryConsts::Section::Code:
        if (DWARF) {
          codeSectionLocation = pos;
        }
        readFunctions();
        break;
      case BinaryConsts::Section::Export:
        readExports();
        break;
      case BinaryConsts::Section::Element:
        readElementSegments();
        break;
      case BinaryConsts::Section::Strings:
        readStrings();
        break;
      case BinaryConsts::Section::Global:
        readGlobals();
        break;
      case BinaryConsts::Section::Data:
        readDataSegments();
        break;
      case BinaryConsts::Section::DataCount:
        readDataSegmentCount();
        break;
      case BinaryConsts::Section::Table:
        readTableDeclarations();
        break;
      case BinaryConsts::Section::Tag:
        readTags();
        break;
      case BinaryConsts::Section::Custom: {
        readCustomSection(payloadLen);
        if (pos > oldPos + payloadLen) {
          throwError("bad user section size, started at " +
                     std::to_string(oldPos) + " plus payload " +
                     std::to_string(payloadLen) +
                     " not being equal to new position " + std::to_string(pos));
        }
        pos = oldPos + payloadLen;
        break;
      }
      default:
        throwError(std::string("unrecognized section ID: ") +
                   std::to_string(sectionCode));
    }

    // make sure we advanced exactly past this section
    if (pos != oldPos + payloadLen) {
      throwError("bad section size, started at " + std::to_string(oldPos) +
                 " plus payload " + std::to_string(payloadLen) +
                 " not being equal to new position " + std::to_string(pos));
    }
  }

  validateBinary();
}

void WasmBinaryReader::readCustomSection(size_t payloadLen) {
  auto oldPos = pos;
  Name sectionName = getInlineString();
  size_t read = pos - oldPos;
  if (read > payloadLen) {
    throwError("bad user section size");
  }
  payloadLen -= read;
  if (sectionName.equals(BinaryConsts::CustomSections::Name)) {
    // We already read the name section before anything else.
    pos += payloadLen;
  } else if (sectionName.equals(BinaryConsts::CustomSections::TargetFeatures)) {
    readFeatures(payloadLen);
  } else if (sectionName.equals(BinaryConsts::CustomSections::Dylink)) {
    readDylink(payloadLen);
  } else if (sectionName.equals(BinaryConsts::CustomSections::Dylink0)) {
    readDylink0(payloadLen);
  } else {
    // an unfamiliar custom section
    if (sectionName.equals(BinaryConsts::CustomSections::Linking)) {
      std::cerr
        << "warning: linking section is present, so this is not a standard "
           "wasm file - binaryen cannot handle this properly!\n";
    }
    wasm.customSections.resize(wasm.customSections.size() + 1);
    auto& section = wasm.customSections.back();
    section.name = sectionName.str;
    auto data = getByteView(payloadLen);
    section.data = {data.begin(), data.end()};
  }
}

std::string_view WasmBinaryReader::getByteView(size_t size) {
  if (size > input.size() || pos > input.size() - size) {
    throwError("unexpected end of input");
  }
  pos += size;
  return {input.data() + (pos - size), size};
}

uint8_t WasmBinaryReader::getInt8() {
  if (!more()) {
    throwError("unexpected end of input");
  }
  return input[pos++];
}

uint16_t WasmBinaryReader::getInt16() {
  auto ret = uint16_t(getInt8());
  ret |= uint16_t(getInt8()) << 8;
  return ret;
}

uint32_t WasmBinaryReader::getInt32() {
  auto ret = uint32_t(getInt16());
  ret |= uint32_t(getInt16()) << 16;
  return ret;
}

uint64_t WasmBinaryReader::getInt64() {
  auto ret = uint64_t(getInt32());
  ret |= uint64_t(getInt32()) << 32;
  return ret;
}

uint8_t WasmBinaryReader::getLaneIndex(size_t lanes) {
  auto ret = getInt8();
  if (ret >= lanes) {
    throwError("Illegal lane index");
  }
  return ret;
}

Literal WasmBinaryReader::getFloat32Literal() {
  auto ret = Literal(getInt32());
  ret = ret.castToF32();
  return ret;
}

Literal WasmBinaryReader::getFloat64Literal() {
  auto ret = Literal(getInt64());
  ret = ret.castToF64();
  return ret;
}

Literal WasmBinaryReader::getVec128Literal() {
  std::array<uint8_t, 16> bytes;
  for (auto i = 0; i < 16; ++i) {
    bytes[i] = getInt8();
  }
  auto ret = Literal(bytes.data());
  return ret;
}

uint32_t WasmBinaryReader::getU32LEB() {
  U32LEB ret;
  ret.read([&]() { return getInt8(); });
  return ret.value;
}

uint64_t WasmBinaryReader::getU64LEB() {
  U64LEB ret;
  ret.read([&]() { return getInt8(); });
  return ret.value;
}

int32_t WasmBinaryReader::getS32LEB() {
  S32LEB ret;
  ret.read([&]() { return (int8_t)getInt8(); });
  return ret.value;
}

int64_t WasmBinaryReader::getS64LEB() {
  S64LEB ret;
  ret.read([&]() { return (int8_t)getInt8(); });
  return ret.value;
}

bool WasmBinaryReader::getBasicType(int32_t code, Type& out) {
  switch (code) {
    case BinaryConsts::EncodedType::i32:
      out = Type::i32;
      return true;
    case BinaryConsts::EncodedType::i64:
      out = Type::i64;
      return true;
    case BinaryConsts::EncodedType::f32:
      out = Type::f32;
      return true;
    case BinaryConsts::EncodedType::f64:
      out = Type::f64;
      return true;
    case BinaryConsts::EncodedType::v128:
      out = Type::v128;
      return true;
    case BinaryConsts::EncodedType::funcref:
      out = Type(HeapType::func, Nullable);
      return true;
    case BinaryConsts::EncodedType::contref:
      out = Type(HeapType::cont, Nullable);
      return true;
    case BinaryConsts::EncodedType::externref:
      out = Type(HeapType::ext, Nullable);
      return true;
    case BinaryConsts::EncodedType::anyref:
      out = Type(HeapType::any, Nullable);
      return true;
    case BinaryConsts::EncodedType::eqref:
      out = Type(HeapType::eq, Nullable);
      return true;
    case BinaryConsts::EncodedType::i31ref:
      out = Type(HeapType::i31, Nullable);
      return true;
    case BinaryConsts::EncodedType::structref:
      out = Type(HeapType::struct_, Nullable);
      return true;
    case BinaryConsts::EncodedType::arrayref:
      out = Type(HeapType::array, Nullable);
      return true;
    case BinaryConsts::EncodedType::exnref:
      out = Type(HeapType::exn, Nullable);
      return true;
    case BinaryConsts::EncodedType::stringref:
      out = Type(HeapType::string, Nullable);
      return true;
    case BinaryConsts::EncodedType::nullref:
      out = Type(HeapType::none, Nullable);
      return true;
    case BinaryConsts::EncodedType::nullexternref:
      out = Type(HeapType::noext, Nullable);
      return true;
    case BinaryConsts::EncodedType::nullfuncref:
      out = Type(HeapType::nofunc, Nullable);
      return true;
    case BinaryConsts::EncodedType::nullexnref:
      out = Type(HeapType::noexn, Nullable);
      return true;
    case BinaryConsts::EncodedType::nullcontref:
      out = Type(HeapType::nocont, Nullable);
      return true;
    default:
      return false;
  }
}

bool WasmBinaryReader::getBasicHeapType(int64_t code, HeapType& out) {
  switch (code) {
    case BinaryConsts::EncodedHeapType::func:
      out = HeapType::func;
      return true;
    case BinaryConsts::EncodedHeapType::cont:
      out = HeapType::cont;
      return true;
    case BinaryConsts::EncodedHeapType::ext:
      out = HeapType::ext;
      return true;
    case BinaryConsts::EncodedHeapType::any:
      out = HeapType::any;
      return true;
    case BinaryConsts::EncodedHeapType::eq:
      out = HeapType::eq;
      return true;
    case BinaryConsts::EncodedHeapType::i31:
      out = HeapType::i31;
      return true;
    case BinaryConsts::EncodedHeapType::struct_:
      out = HeapType::struct_;
      return true;
    case BinaryConsts::EncodedHeapType::array:
      out = HeapType::array;
      return true;
    case BinaryConsts::EncodedHeapType::exn:
      out = HeapType::exn;
      return true;
    case BinaryConsts::EncodedHeapType::string:
      out = HeapType::string;
      return true;
    case BinaryConsts::EncodedHeapType::none:
      out = HeapType::none;
      return true;
    case BinaryConsts::EncodedHeapType::noext:
      out = HeapType::noext;
      return true;
    case BinaryConsts::EncodedHeapType::nofunc:
      out = HeapType::nofunc;
      return true;
    case BinaryConsts::EncodedHeapType::noexn:
      out = HeapType::noexn;
      return true;
    case BinaryConsts::EncodedHeapType::nocont:
      out = HeapType::nocont;
      return true;
    default:
      return false;
  }
}

Type WasmBinaryReader::getType(int initial) {
  // Single value types are negative; signature indices are non-negative
  if (initial >= 0) {
    // TODO: Handle block input types properly.
    auto sig = getSignatureByTypeIndex(initial);
    if (sig.params != Type::none) {
      throwError("control flow inputs are not supported yet");
    }
    return sig.results;
  }
  Type type;
  if (getBasicType(initial, type)) {
    return type;
  }
  switch (initial) {
    // None only used for block signatures. TODO: Separate out?
    case BinaryConsts::EncodedType::Empty:
      return Type::none;
    case BinaryConsts::EncodedType::nullable:
      return Type(getHeapType(), Nullable);
    case BinaryConsts::EncodedType::nonnullable:
      return Type(getHeapType(), NonNullable);
    default:
      throwError("invalid wasm type: " + std::to_string(initial));
  }
  WASM_UNREACHABLE("unexpected type");
}

Type WasmBinaryReader::getType() { return getType(getS32LEB()); }

HeapType WasmBinaryReader::getHeapType() {
  auto type = getS64LEB(); // TODO: Actually s33
  // Single heap types are negative; heap type indices are non-negative
  if (type >= 0) {
    if (size_t(type) >= types.size()) {
      throwError("invalid signature index: " + std::to_string(type));
    }
    return types[type];
  }
  auto share = Unshared;
  if (type == BinaryConsts::EncodedType::Shared) {
    share = Shared;
    type = getS64LEB(); // TODO: Actually s33
  }
  HeapType ht;
  if (getBasicHeapType(type, ht)) {
    return ht.getBasic(share);
  } else {
    throwError("invalid wasm heap type: " + std::to_string(type));
  }
  WASM_UNREACHABLE("unexpected type");
}

HeapType WasmBinaryReader::getIndexedHeapType() {
  auto index = getU32LEB();
  if (index >= types.size()) {
    throwError("invalid heap type index: " + std::to_string(index));
  }
  return types[index];
}

Type WasmBinaryReader::getConcreteType() {
  auto type = getType();
  if (!type.isConcrete()) {
    throwError("non-concrete type when one expected");
  }
  return type;
}

Name WasmBinaryReader::getInlineString(bool requireValid) {
  auto len = getU32LEB();
  auto data = getByteView(len);
  if (requireValid && !String::isUTF8(data)) {
    throwError("invalid UTF-8 string");
  }
  return Name(data);
}

void WasmBinaryReader::verifyInt8(int8_t x) {
  int8_t y = getInt8();
  if (x != y) {
    throwError("surprising value");
  }
}

void WasmBinaryReader::verifyInt16(int16_t x) {
  int16_t y = getInt16();
  if (x != y) {
    throwError("surprising value");
  }
}

void WasmBinaryReader::verifyInt32(int32_t x) {
  int32_t y = getInt32();
  if (x != y) {
    throwError("surprising value");
  }
}

void WasmBinaryReader::verifyInt64(int64_t x) {
  int64_t y = getInt64();
  if (x != y) {
    throwError("surprising value");
  }
}

void WasmBinaryReader::readHeader() {
  verifyInt32(BinaryConsts::Magic);
  auto version = getInt32();
  if (version != BinaryConsts::Version) {
    if (version == 0x1000d) {
      throwError("this looks like a wasm component, which Binaryen does not "
                 "support yet (see "
                 "https://github.com/WebAssembly/binaryen/issues/6728)");
    }
    throwError("invalid version");
  }
}

void WasmBinaryReader::readStart() {
  startIndex = getU32LEB();
  wasm.start = getFunctionName(startIndex);
}

static Name makeName(std::string prefix, size_t counter) {
  return Name(prefix + std::to_string(counter));
}

// Look up a name from the names section or use a validated version of the
// provided name. Return the name and whether it is explicit in the input.
static std::pair<Name, bool>
getOrMakeName(const std::unordered_map<Index, Name>& nameMap,
              Index i,
              Name name,
              std::unordered_set<Name>& usedNames) {
  if (auto it = nameMap.find(i); it != nameMap.end()) {
    return {it->second, true};
  } else {
    auto valid = Names::getValidNameGivenExisting(name, usedNames);
    usedNames.insert(valid);
    return {valid, false};
  }
}

void WasmBinaryReader::readMemories() {
  auto num = getU32LEB();
  auto numImports = wasm.memories.size();
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : memoryNames) {
    if (index >= num + numImports) {
      std::cerr << "warning: memory index out of bounds in name section: "
                << name << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  for (size_t i = 0; i < num; i++) {
    auto [name, isExplicit] =
      getOrMakeName(memoryNames, numImports + i, makeName("", i), usedNames);
    auto memory = Builder::makeMemory(name);
    memory->hasExplicitName = isExplicit;
    getResizableLimits(memory->initial,
                       memory->max,
                       memory->shared,
                       memory->addressType,
                       Memory::kUnlimitedSize);
    wasm.addMemory(std::move(memory));
  }
}

void WasmBinaryReader::readTypes() {
  TypeBuilder builder(getU32LEB());

  auto readHeapType = [&]() -> HeapType {
    int64_t htCode = getS64LEB(); // TODO: Actually s33
    auto share = Unshared;
    if (htCode == BinaryConsts::EncodedType::Shared) {
      share = Shared;
      htCode = getS64LEB(); // TODO: Actually s33
    }
    HeapType ht;
    if (getBasicHeapType(htCode, ht)) {
      return ht.getBasic(share);
    }
    if (size_t(htCode) >= builder.size()) {
      throwError("invalid type index: " + std::to_string(htCode));
    }
    return builder.getTempHeapType(size_t(htCode));
  };
  auto makeType = [&](int32_t typeCode) {
    Type type;
    if (getBasicType(typeCode, type)) {
      return type;
    }

    switch (typeCode) {
      case BinaryConsts::EncodedType::nullable:
      case BinaryConsts::EncodedType::nonnullable: {
        auto nullability = typeCode == BinaryConsts::EncodedType::nullable
                             ? Nullable
                             : NonNullable;

        HeapType ht = readHeapType();
        if (ht.isBasic()) {
          return Type(ht, nullability);
        }

        return builder.getTempRefType(ht, nullability);
      }
      default:
        throwError("unexpected type index: " + std::to_string(typeCode));
    }
    WASM_UNREACHABLE("unexpected type");
  };
  auto readType = [&]() { return makeType(getS32LEB()); };

  auto readSignatureDef = [&]() {
    std::vector<Type> params;
    std::vector<Type> results;
    size_t numParams = getU32LEB();
    for (size_t j = 0; j < numParams; j++) {
      params.push_back(readType());
    }
    auto numResults = getU32LEB();
    for (size_t j = 0; j < numResults; j++) {
      results.push_back(readType());
    }
    return Signature(builder.getTempTupleType(params),
                     builder.getTempTupleType(results));
  };

  auto readContinuationDef = [&]() {
    HeapType ht = readHeapType();
    if (!ht.isSignature()) {
      throw ParseException("cont types must be built from function types");
    }
    return Continuation(ht);
  };

  auto readMutability = [&]() {
    switch (getU32LEB()) {
      case 0:
        return Immutable;
      case 1:
        return Mutable;
      default:
        throw ParseException("Expected 0 or 1 for mutability");
    }
  };

  auto readFieldDef = [&]() {
    // The value may be a general wasm type, or one of the types only possible
    // in a field.
    auto typeCode = getS32LEB();
    if (typeCode == BinaryConsts::EncodedType::i8) {
      auto mutable_ = readMutability();
      return Field(Field::i8, mutable_);
    }
    if (typeCode == BinaryConsts::EncodedType::i16) {
      auto mutable_ = readMutability();
      return Field(Field::i16, mutable_);
    }
    // It's a regular wasm value.
    auto type = makeType(typeCode);
    auto mutable_ = readMutability();
    return Field(type, mutable_);
  };

  auto readStructDef = [&]() {
    FieldList fields;
    size_t numFields = getU32LEB();
    for (size_t j = 0; j < numFields; j++) {
      fields.push_back(readFieldDef());
    }
    return Struct(std::move(fields));
  };

  for (size_t i = 0; i < builder.size(); i++) {
    auto form = getInt8();
    if (form == BinaryConsts::EncodedType::Rec) {
      uint32_t groupSize = getU32LEB();
      if (groupSize == 0u) {
        // TODO: Support groups of size zero by shrinking the builder.
        throwError("Recursion groups of size zero not supported");
      }
      // The group counts as one element in the type section, so we have to
      // allocate space for the extra types.
      builder.grow(groupSize - 1);
      builder.createRecGroup(i, groupSize);
      form = getInt8();
    }
    std::optional<uint32_t> superIndex;
    if (form == BinaryConsts::EncodedType::Sub ||
        form == BinaryConsts::EncodedType::SubFinal) {
      if (form == BinaryConsts::EncodedType::Sub) {
        builder[i].setOpen();
      }
      uint32_t supers = getU32LEB();
      if (supers > 0) {
        if (supers != 1) {
          throwError("Invalid type definition with " + std::to_string(supers) +
                     " supertypes");
        }
        superIndex = getU32LEB();
      }
      form = getInt8();
    }
    if (form == BinaryConsts::SharedDef) {
      builder[i].setShared();
      form = getInt8();
    }
    if (form == BinaryConsts::EncodedType::Func) {
      builder[i] = readSignatureDef();
    } else if (form == BinaryConsts::EncodedType::Cont) {
      builder[i] = readContinuationDef();
    } else if (form == BinaryConsts::EncodedType::Struct) {
      builder[i] = readStructDef();
    } else if (form == BinaryConsts::EncodedType::Array) {
      builder[i] = Array(readFieldDef());
    } else {
      throwError("Bad type form " + std::to_string(form));
    }
    if (superIndex) {
      if (*superIndex > builder.size()) {
        throwError("Out of bounds supertype index " +
                   std::to_string(*superIndex));
      }
      builder[i].subTypeOf(builder[*superIndex]);
    }
  }

  auto result = builder.build();
  if (auto* err = result.getError()) {
    Fatal() << "Invalid type: " << err->reason << " at index " << err->index;
  }
  types = std::move(*result);

  // Record the type indices.
  for (Index i = 0; i < types.size(); ++i) {
    wasm.typeIndices.insert({types[i], i});
  }

  // Assign names from the names section.
  for (auto& [index, name] : typeNames) {
    if (index >= types.size()) {
      std::cerr << "warning: type index out of bounds in name section: " << name
                << " at index " << index << '\n';
      continue;
    }
    wasm.typeNames[types[index]].name = name;
  }
  for (auto& [index, fields] : fieldNames) {
    if (index >= types.size()) {
      std::cerr
        << "warning: type index out of bounds in name section: fields at index "
        << index << '\n';
      continue;
    }
    if (!types[index].isStruct()) {
      std::cerr << "warning: field names applied to non-struct type at index "
                << index << '\n';
      continue;
    }
    auto& names = wasm.typeNames[types[index]].fieldNames;
    for (auto& [field, name] : fields) {
      if (field >= types[index].getStruct().fields.size()) {
        std::cerr << "warning: field index out of bounds in name section: "
                  << name << " at index " << field << " in type " << index
                  << '\n';
        continue;
      }
      names[field] = name;
    }
  }
}

Name WasmBinaryReader::getFunctionName(Index index) {
  if (index >= wasm.functions.size()) {
    throwError("invalid function index");
  }
  return wasm.functions[index]->name;
}

Name WasmBinaryReader::getTableName(Index index) {
  if (index >= wasm.tables.size()) {
    throwError("invalid table index");
  }
  return wasm.tables[index]->name;
}

Name WasmBinaryReader::getMemoryName(Index index) {
  if (index >= wasm.memories.size()) {
    throwError("invalid memory index");
  }
  return wasm.memories[index]->name;
}

Name WasmBinaryReader::getGlobalName(Index index) {
  if (index >= wasm.globals.size()) {
    throwError("invalid global index");
  }
  return wasm.globals[index]->name;
}

Table* WasmBinaryReader::getTable(Index index) {
  if (index < wasm.tables.size()) {
    return wasm.tables[index].get();
  }
  throwError("Table index out of range.");
}

Name WasmBinaryReader::getTagName(Index index) {
  if (index >= wasm.tags.size()) {
    throwError("invalid tag index");
  }
  return wasm.tags[index]->name;
}

Name WasmBinaryReader::getDataName(Index index) {
  if (index >= wasm.dataSegments.size()) {
    throwError("invalid data segment index");
  }
  return wasm.dataSegments[index]->name;
}

Name WasmBinaryReader::getElemName(Index index) {
  if (index >= wasm.elementSegments.size()) {
    throwError("invalid element segment index");
  }
  return wasm.elementSegments[index]->name;
}

Memory* WasmBinaryReader::getMemory(Index index) {
  if (index < wasm.memories.size()) {
    return wasm.memories[index].get();
  }
  throwError("Memory index out of range.");
}

void WasmBinaryReader::getResizableLimits(Address& initial,
                                          Address& max,
                                          bool& shared,
                                          Type& addressType,
                                          Address defaultIfNoMax) {
  auto flags = getU32LEB();
  bool hasMax = (flags & BinaryConsts::HasMaximum) != 0;
  bool isShared = (flags & BinaryConsts::IsShared) != 0;
  bool is64 = (flags & BinaryConsts::Is64) != 0;
  initial = is64 ? getU64LEB() : getU32LEB();
  if (isShared && !hasMax) {
    throwError("shared memory must have max size");
  }
  shared = isShared;
  addressType = is64 ? Type::i64 : Type::i32;
  if (hasMax) {
    max = is64 ? getU64LEB() : getU32LEB();
  } else {
    max = defaultIfNoMax;
  }
}

void WasmBinaryReader::readImports() {
  size_t num = getU32LEB();
  Builder builder(wasm);
  std::unordered_set<Name> usedFunctionNames, usedTableNames, usedMemoryNames,
    usedGlobalNames, usedTagNames;
  for (size_t i = 0; i < num; i++) {
    auto module = getInlineString();
    auto base = getInlineString();
    auto kind = (ExternalKind)getU32LEB();
    // We set a unique prefix for the name based on the kind. This ensures no
    // collisions between them, which can't occur here (due to the index i) but
    // could occur later due to the names section.
    switch (kind) {
      case ExternalKind::Function: {
        auto [name, isExplicit] =
          getOrMakeName(functionNames,
                        wasm.functions.size(),
                        makeName("fimport$", wasm.functions.size()),
                        usedFunctionNames);
        auto index = getU32LEB();
        functionTypes.push_back(getTypeByIndex(index));
        auto type = getTypeByIndex(index);
        if (!type.isSignature()) {
          throwError(std::string("Imported function ") + module.toString() +
                     '.' + base.toString() +
                     "'s type must be a signature. Given: " + type.toString());
        }
        auto curr = builder.makeFunction(name, type, {});
        curr->hasExplicitName = isExplicit;
        curr->module = module;
        curr->base = base;
        setLocalNames(*curr, wasm.functions.size());
        wasm.addFunction(std::move(curr));
        break;
      }
      case ExternalKind::Table: {
        auto [name, isExplicit] =
          getOrMakeName(tableNames,
                        wasm.tables.size(),
                        makeName("timport$", wasm.tables.size()),
                        usedTableNames);
        auto table = builder.makeTable(name);
        table->hasExplicitName = isExplicit;
        table->module = module;
        table->base = base;
        table->type = getType();

        bool is_shared;
        getResizableLimits(table->initial,
                           table->max,
                           is_shared,
                           table->addressType,
                           Table::kUnlimitedSize);
        if (is_shared) {
          throwError("Tables may not be shared");
        }
        wasm.addTable(std::move(table));
        break;
      }
      case ExternalKind::Memory: {
        auto [name, isExplicit] =
          getOrMakeName(memoryNames,
                        wasm.memories.size(),
                        makeName("mimport$", wasm.memories.size()),
                        usedMemoryNames);
        auto memory = builder.makeMemory(name);
        memory->hasExplicitName = isExplicit;
        memory->module = module;
        memory->base = base;
        getResizableLimits(memory->initial,
                           memory->max,
                           memory->shared,
                           memory->addressType,
                           Memory::kUnlimitedSize);
        wasm.addMemory(std::move(memory));
        break;
      }
      case ExternalKind::Global: {
        auto [name, isExplicit] =
          getOrMakeName(globalNames,
                        wasm.globals.size(),
                        makeName("gimport$", wasm.globals.size()),
                        usedGlobalNames);
        auto type = getConcreteType();
        auto mutable_ = getU32LEB();
        if (mutable_ & ~1) {
          throwError("Global mutability must be 0 or 1");
        }
        auto curr =
          builder.makeGlobal(name,
                             type,
                             nullptr,
                             mutable_ ? Builder::Mutable : Builder::Immutable);
        curr->hasExplicitName = isExplicit;
        curr->module = module;
        curr->base = base;
        wasm.addGlobal(std::move(curr));
        break;
      }
      case ExternalKind::Tag: {
        auto [name, isExplicit] =
          getOrMakeName(tagNames,
                        wasm.tags.size(),
                        makeName("eimport$", wasm.tags.size()),
                        usedTagNames);
        getInt8(); // Reserved 'attribute' field
        auto index = getU32LEB();
        auto curr = builder.makeTag(name, getSignatureByTypeIndex(index));
        curr->hasExplicitName = isExplicit;
        curr->module = module;
        curr->base = base;
        wasm.addTag(std::move(curr));
        break;
      }
      default: {
        throwError("bad import kind");
      }
    }
  }
  numFuncImports = wasm.functions.size();
}

void WasmBinaryReader::setLocalNames(Function& func, Index i) {
  if (auto it = localNames.find(i); it != localNames.end()) {
    for (auto& [local, name] : it->second) {
      if (local >= func.getNumLocals()) {
        std::cerr << "warning: local index out of bounds in name section: "
                  << name << " at index " << local << " in function " << i
                  << '\n';
        continue;
      }
      func.setLocalName(local, name);
    }
  }
}

void WasmBinaryReader::readFunctionSignatures() {
  size_t num = getU32LEB();
  auto numImports = wasm.functions.size();
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : functionNames) {
    if (index >= num + numImports) {
      std::cerr << "warning: function index out of bounds in name section: "
                << name << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  // Also check that the function indices in the local names subsection are
  // in-bounds, even though we don't use them here.
  for (auto& [index, locals] : localNames) {
    if (index >= num + numImports) {
      std::cerr << "warning: function index out of bounds in name section: "
                   "locals at index "
                << index << '\n';
    }
  }
  for (size_t i = 0; i < num; i++) {
    auto [name, isExplicit] =
      getOrMakeName(functionNames, numImports + i, makeName("", i), usedNames);
    auto index = getU32LEB();
    HeapType type = getTypeByIndex(index);
    functionTypes.push_back(type);
    // Check that the type is a signature.
    getSignatureByTypeIndex(index);
    auto func = Builder(wasm).makeFunction(name, type, {}, nullptr);
    func->hasExplicitName = isExplicit;
    wasm.addFunction(std::move(func));
  }
}

HeapType WasmBinaryReader::getTypeByIndex(Index index) {
  if (index >= types.size()) {
    throwError("invalid type index " + std::to_string(index) + " / " +
               std::to_string(types.size()));
  }
  return types[index];
}

HeapType WasmBinaryReader::getTypeByFunctionIndex(Index index) {
  if (index >= functionTypes.size()) {
    throwError("invalid function index");
  }
  return functionTypes[index];
}

Signature WasmBinaryReader::getSignatureByTypeIndex(Index index) {
  auto heapType = getTypeByIndex(index);
  if (!heapType.isSignature()) {
    throwError("invalid signature type " + heapType.toString());
  }
  return heapType.getSignature();
}

Signature WasmBinaryReader::getSignatureByFunctionIndex(Index index) {
  auto heapType = getTypeByFunctionIndex(index);
  if (!heapType.isSignature()) {
    throwError("invalid signature type " + heapType.toString());
  }
  return heapType.getSignature();
}

void WasmBinaryReader::readFunctions() {
  numFuncBodies = getU32LEB();
  if (numFuncBodies + numFuncImports != wasm.functions.size()) {
    throwError("invalid function section size, must equal types");
  }
  if (DWARF) {
    builder.setBinaryLocation(&pos, codeSectionLocation);
  }
  for (size_t i = 0; i < numFuncBodies; i++) {
    auto sizePos = pos;
    size_t size = getU32LEB();
    if (size == 0) {
      throwError("empty function size");
    }
    Index endOfFunction = pos + size;

    auto& func = wasm.functions[numFuncImports + i];
    currFunction = func.get();

    if (DWARF) {
      func->funcLocation = BinaryLocations::FunctionLocations{
        BinaryLocation(sizePos - codeSectionLocation),
        BinaryLocation(pos - codeSectionLocation),
        BinaryLocation(pos - codeSectionLocation + size)};
    }

    func->prologLocation = sourceMapReader.readDebugLocationAt(pos);

    readVars();
    setLocalNames(*func, numFuncImports + i);
    {
      // Process the function body. Even if we are skipping function bodies we
      // need to not skip the start function. That contains important code for
      // wasm-emscripten-finalize in the form of pthread-related segment
      // initializations. As this is just one function, it doesn't add
      // significant time, so the optimization of skipping bodies is still very
      // useful.
      auto currFunctionIndex = wasm.functions.size();
      bool isStart = startIndex == currFunctionIndex;
      if (skipFunctionBodies && !isStart) {
        // When skipping the function body we need to put something valid in
        // their place so we validate. An unreachable is always acceptable
        // there.
        func->body = Builder(wasm).makeUnreachable();
        // Skip reading the contents.
        pos = endOfFunction;
      } else {
        auto start = builder.visitFunctionStart(func.get());
        if (auto* err = start.getErr()) {
          throwError(err->msg);
        }
        while (pos < endOfFunction) {
          auto inst = readInst();
          if (auto* err = inst.getErr()) {
            throwError(err->msg);
          }
        }
        if (pos != endOfFunction) {
          throwError("function overflowed its bounds");
        }
        if (!builder.empty()) {
          throwError("expected function end");
        }
      }
    }

    sourceMapReader.finishFunction();
    TypeUpdating::handleNonDefaultableLocals(func.get(), wasm);
    currFunction = nullptr;
  }
}

void WasmBinaryReader::readVars() {
  uint32_t totalVars = 0;
  size_t numLocalTypes = getU32LEB();
  // Use a SmallVector as in the common (MVP) case there are only 4 possible
  // types.
  SmallVector<std::pair<uint32_t, Type>, 4> decodedVars;
  decodedVars.reserve(numLocalTypes);
  for (size_t t = 0; t < numLocalTypes; t++) {
    auto num = getU32LEB();
    if (std::ckd_add(&totalVars, totalVars, num)) {
      throwError("unaddressable number of locals");
    }
    auto type = getConcreteType();
    decodedVars.emplace_back(num, type);
  }
  currFunction->vars.reserve(totalVars);
  for (auto [num, type] : decodedVars) {
    while (num > 0) {
      currFunction->vars.push_back(type);
      num--;
    }
  }
}

Result<> WasmBinaryReader::readInst() {
  if (auto loc = sourceMapReader.readDebugLocationAt(pos)) {
    builder.setDebugLocation(loc);
  }
  uint8_t code = getInt8();
  switch (code) {
    case BinaryConsts::Block:
      return builder.makeBlock(Name(), getType());
    case BinaryConsts::If:
      return builder.makeIf(Name(), getType());
    case BinaryConsts::Loop:
      return builder.makeLoop(Name(), getType());
    case BinaryConsts::Br:
      return builder.makeBreak(getU32LEB(), false);
    case BinaryConsts::BrIf:
      return builder.makeBreak(getU32LEB(), true);
    case BinaryConsts::BrTable: {
      auto numTargets = getU32LEB();
      std::vector<Index> labels(numTargets);
      for (Index i = 0; i < numTargets; ++i) {
        labels[i] = getU32LEB();
      }
      return builder.makeSwitch(labels, getU32LEB());
    }
    case BinaryConsts::CallFunction:
    case BinaryConsts::RetCallFunction:
      return builder.makeCall(getFunctionName(getU32LEB()),
                              code == BinaryConsts::RetCallFunction);
    case BinaryConsts::CallIndirect:
    case BinaryConsts::RetCallIndirect: {
      auto type = getIndexedHeapType();
      auto table = getTableName(getU32LEB());
      return builder.makeCallIndirect(
        table, type, code == BinaryConsts::RetCallIndirect);
    }
    case BinaryConsts::LocalGet:
      return builder.makeLocalGet(getU32LEB());
    case BinaryConsts::LocalSet:
      return builder.makeLocalSet(getU32LEB());
    case BinaryConsts::LocalTee:
      return builder.makeLocalTee(getU32LEB());
    case BinaryConsts::GlobalGet:
      return builder.makeGlobalGet(getGlobalName(getU32LEB()));
    case BinaryConsts::GlobalSet:
      return builder.makeGlobalSet(getGlobalName(getU32LEB()));
    case BinaryConsts::Select:
      return builder.makeSelect(std::nullopt);
    case BinaryConsts::SelectWithType: {
      auto numTypes = getU32LEB();
      std::vector<Type> types;
      for (Index i = 0; i < numTypes; ++i) {
        auto t = getType();
        if (!t.isConcrete()) {
          return Err{"bad select type"};
        }
        types.push_back(t);
      }
      return builder.makeSelect(Type(types));
    }
    case BinaryConsts::Return:
      return builder.makeReturn();
    case BinaryConsts::Nop:
      return builder.makeNop();
    case BinaryConsts::Unreachable:
      return builder.makeUnreachable();
    case BinaryConsts::Drop:
      return builder.makeDrop();
    case BinaryConsts::End:
      return builder.visitEnd();
    case BinaryConsts::Else:
      return builder.visitElse();
    case BinaryConsts::Catch_Legacy:
      return builder.visitCatch(getTagName(getU32LEB()));
    case BinaryConsts::CatchAll_Legacy:
      return builder.visitCatchAll();
    case BinaryConsts::Delegate:
      return builder.visitDelegate(getU32LEB());
    case BinaryConsts::RefNull:
      return builder.makeRefNull(getHeapType());
    case BinaryConsts::RefIsNull:
      return builder.makeRefIsNull();
    case BinaryConsts::RefFunc:
      return builder.makeRefFunc(getFunctionName(getU32LEB()));
    case BinaryConsts::RefEq:
      return builder.makeRefEq();
    case BinaryConsts::RefAsNonNull:
      return builder.makeRefAs(RefAsNonNull);
    case BinaryConsts::BrOnNull:
      return builder.makeBrOn(getU32LEB(), BrOnNull);
    case BinaryConsts::BrOnNonNull:
      return builder.makeBrOn(getU32LEB(), BrOnNonNull);
    case BinaryConsts::TableGet:
      return builder.makeTableGet(getTableName(getU32LEB()));
    case BinaryConsts::TableSet:
      return builder.makeTableSet(getTableName(getU32LEB()));
    case BinaryConsts::Try:
      return builder.makeTry(Name(), getType());
    case BinaryConsts::TryTable: {
      auto type = getType();
      std::vector<Name> tags;
      std::vector<Index> labels;
      std::vector<bool> isRefs;
      auto numHandlers = getU32LEB();
      for (Index i = 0; i < numHandlers; ++i) {
        uint8_t code = getInt8();
        if (code == BinaryConsts::Catch || code == BinaryConsts::CatchRef) {
          tags.push_back(getTagName(getU32LEB()));
        } else {
          tags.push_back(Name());
        }
        labels.push_back(getU32LEB());
        isRefs.push_back(code == BinaryConsts::CatchRef ||
                         code == BinaryConsts::CatchAllRef);
      }
      return builder.makeTryTable(Name(), type, tags, labels, isRefs);
    }
    case BinaryConsts::Throw:
      return builder.makeThrow(getTagName(getU32LEB()));
    case BinaryConsts::Rethrow:
      return builder.makeRethrow(getU32LEB());
    case BinaryConsts::ThrowRef:
      return builder.makeThrowRef();
    case BinaryConsts::MemorySize:
      return builder.makeMemorySize(getMemoryName(getU32LEB()));
    case BinaryConsts::MemoryGrow:
      return builder.makeMemoryGrow(getMemoryName(getU32LEB()));
    case BinaryConsts::CallRef:
    case BinaryConsts::RetCallRef:
      return builder.makeCallRef(getIndexedHeapType(),
                                 code == BinaryConsts::RetCallRef);
    case BinaryConsts::ContNew:
      return builder.makeContNew(getIndexedHeapType());
    case BinaryConsts::ContBind: {
      auto before = getIndexedHeapType();
      auto after = getIndexedHeapType();
      return builder.makeContBind(before, after);
    }
    case BinaryConsts::Suspend:
      return builder.makeSuspend(getTagName(getU32LEB()));
    case BinaryConsts::Resume: {
      auto type = getIndexedHeapType();
      auto numHandlers = getU32LEB();
      std::vector<Name> tags;
      std::vector<std::optional<Index>> labels;
      tags.reserve(numHandlers);
      labels.reserve(numHandlers);
      for (Index i = 0; i < numHandlers; ++i) {
        uint8_t code = getInt8();
        if (code == BinaryConsts::OnLabel) {
          tags.push_back(getTagName(getU32LEB()));
          labels.push_back(std::optional<Index>{getU32LEB()});
        } else if (code == BinaryConsts::OnSwitch) {
          tags.push_back(getTagName(getU32LEB()));
          labels.push_back(std::nullopt);
        } else {
          return Err{"ON opcode expected"};
        }
      }
      return builder.makeResume(type, tags, labels);
    }
    case BinaryConsts::ResumeThrow: {
      auto type = getIndexedHeapType();
      auto tag = getTagName(getU32LEB());
      auto numHandlers = getU32LEB();
      std::vector<Name> tags;
      std::vector<std::optional<Index>> labels;
      tags.reserve(numHandlers);
      labels.reserve(numHandlers);
      for (Index i = 0; i < numHandlers; ++i) {
        uint8_t code = getInt8();
        if (code == BinaryConsts::OnLabel) {
          tags.push_back(getTagName(getU32LEB()));
          labels.push_back(std::optional<Index>{getU32LEB()});
        } else if (code == BinaryConsts::OnSwitch) {
          tags.push_back(getTagName(getU32LEB()));
          labels.push_back(std::nullopt);
        } else {
          return Err{"ON opcode expected"};
        }
      }
      return builder.makeResumeThrow(type, tag, tags, labels);
    }
    case BinaryConsts::Switch: {
      auto type = getIndexedHeapType();
      auto tag = getTagName(getU32LEB());
      return builder.makeStackSwitch(type, tag);
    }

#define BINARY_INT(code)                                                       \
  case BinaryConsts::I32##code:                                                \
    return builder.makeBinary(code##Int32);                                    \
  case BinaryConsts::I64##code:                                                \
    return builder.makeBinary(code##Int64);
#define BINARY_FLOAT(code)                                                     \
  case BinaryConsts::F32##code:                                                \
    return builder.makeBinary(code##Float32);                                  \
  case BinaryConsts::F64##code:                                                \
    return builder.makeBinary(code##Float64);
#define BINARY_NUM(code)                                                       \
  BINARY_INT(code)                                                             \
  BINARY_FLOAT(code)

      BINARY_NUM(Add);
      BINARY_NUM(Sub);
      BINARY_NUM(Mul);
      BINARY_INT(DivS);
      BINARY_INT(DivU);
      BINARY_INT(RemS);
      BINARY_INT(RemU);
      BINARY_INT(And);
      BINARY_INT(Or);
      BINARY_INT(Xor);
      BINARY_INT(Shl);
      BINARY_INT(ShrU);
      BINARY_INT(ShrS);
      BINARY_INT(RotL);
      BINARY_INT(RotR);
      BINARY_FLOAT(Div);
      BINARY_FLOAT(CopySign);
      BINARY_FLOAT(Min);
      BINARY_FLOAT(Max);
      BINARY_NUM(Eq);
      BINARY_NUM(Ne);
      BINARY_INT(LtS);
      BINARY_INT(LtU);
      BINARY_INT(LeS);
      BINARY_INT(LeU);
      BINARY_INT(GtS);
      BINARY_INT(GtU);
      BINARY_INT(GeS);
      BINARY_INT(GeU);
      BINARY_FLOAT(Lt);
      BINARY_FLOAT(Le);
      BINARY_FLOAT(Gt);
      BINARY_FLOAT(Ge);

#define UNARY_INT(code)                                                        \
  case BinaryConsts::I32##code:                                                \
    return builder.makeUnary(code##Int32);                                     \
  case BinaryConsts::I64##code:                                                \
    return builder.makeUnary(code##Int64);
#define UNARY_FLOAT(code)                                                      \
  case BinaryConsts::F32##code:                                                \
    return builder.makeUnary(code##Float32);                                   \
  case BinaryConsts::F64##code:                                                \
    return builder.makeUnary(code##Float64);

      UNARY_INT(Clz);
      UNARY_INT(Ctz);
      UNARY_INT(Popcnt);
      UNARY_INT(EqZ);
      UNARY_FLOAT(Neg);
      UNARY_FLOAT(Abs);
      UNARY_FLOAT(Ceil);
      UNARY_FLOAT(Floor);
      UNARY_FLOAT(Nearest);
      UNARY_FLOAT(Sqrt);

    case BinaryConsts::F32UConvertI32:
      return builder.makeUnary(ConvertUInt32ToFloat32);
    case BinaryConsts::F64UConvertI32:
      return builder.makeUnary(ConvertUInt32ToFloat64);
    case BinaryConsts::F32SConvertI32:
      return builder.makeUnary(ConvertSInt32ToFloat32);
    case BinaryConsts::F64SConvertI32:
      return builder.makeUnary(ConvertSInt32ToFloat64);
    case BinaryConsts::F32UConvertI64:
      return builder.makeUnary(ConvertUInt64ToFloat32);
    case BinaryConsts::F64UConvertI64:
      return builder.makeUnary(ConvertUInt64ToFloat64);
    case BinaryConsts::F32SConvertI64:
      return builder.makeUnary(ConvertSInt64ToFloat32);
    case BinaryConsts::F64SConvertI64:
      return builder.makeUnary(ConvertSInt64ToFloat64);
    case BinaryConsts::I64SExtendI32:
      return builder.makeUnary(ExtendSInt32);
    case BinaryConsts::I64UExtendI32:
      return builder.makeUnary(ExtendUInt32);
    case BinaryConsts::I32WrapI64:
      return builder.makeUnary(WrapInt64);
    case BinaryConsts::I32UTruncF32:
      return builder.makeUnary(TruncUFloat32ToInt32);
    case BinaryConsts::I32UTruncF64:
      return builder.makeUnary(TruncUFloat64ToInt32);
    case BinaryConsts::I32STruncF32:
      return builder.makeUnary(TruncSFloat32ToInt32);
    case BinaryConsts::I32STruncF64:
      return builder.makeUnary(TruncSFloat64ToInt32);
    case BinaryConsts::I64UTruncF32:
      return builder.makeUnary(TruncUFloat32ToInt64);
    case BinaryConsts::I64UTruncF64:
      return builder.makeUnary(TruncUFloat64ToInt64);
    case BinaryConsts::I64STruncF32:
      return builder.makeUnary(TruncSFloat32ToInt64);
    case BinaryConsts::I64STruncF64:
      return builder.makeUnary(TruncSFloat64ToInt64);
    case BinaryConsts::F32Trunc:
      return builder.makeUnary(TruncFloat32);
    case BinaryConsts::F64Trunc:
      return builder.makeUnary(TruncFloat64);
    case BinaryConsts::F32DemoteI64:
      return builder.makeUnary(DemoteFloat64);
    case BinaryConsts::F64PromoteF32:
      return builder.makeUnary(PromoteFloat32);
    case BinaryConsts::I32ReinterpretF32:
      return builder.makeUnary(ReinterpretFloat32);
    case BinaryConsts::I64ReinterpretF64:
      return builder.makeUnary(ReinterpretFloat64);
    case BinaryConsts::F32ReinterpretI32:
      return builder.makeUnary(ReinterpretInt32);
    case BinaryConsts::F64ReinterpretI64:
      return builder.makeUnary(ReinterpretInt64);
    case BinaryConsts::I32ExtendS8:
      return builder.makeUnary(ExtendS8Int32);
    case BinaryConsts::I32ExtendS16:
      return builder.makeUnary(ExtendS16Int32);
    case BinaryConsts::I64ExtendS8:
      return builder.makeUnary(ExtendS8Int64);
    case BinaryConsts::I64ExtendS16:
      return builder.makeUnary(ExtendS16Int64);
    case BinaryConsts::I64ExtendS32:
      return builder.makeUnary(ExtendS32Int64);
    case BinaryConsts::I32Const:
      return builder.makeConst(Literal(getS32LEB()));
    case BinaryConsts::I64Const:
      return builder.makeConst(Literal(getS64LEB()));
    case BinaryConsts::F32Const:
      return builder.makeConst(getFloat32Literal());
    case BinaryConsts::F64Const:
      return builder.makeConst(getFloat64Literal());
    case BinaryConsts::I32LoadMem8S: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(1, true, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I32LoadMem8U: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(1, false, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I32LoadMem16S: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(2, true, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I32LoadMem16U: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(2, false, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I32LoadMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(4, false, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I64LoadMem8S: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(1, true, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64LoadMem8U: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(1, false, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64LoadMem16S: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(2, true, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64LoadMem16U: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(2, false, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64LoadMem32S: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(4, true, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64LoadMem32U: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(4, false, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64LoadMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(8, false, offset, align, Type::i64, mem);
    }
    case BinaryConsts::F32LoadMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(4, false, offset, align, Type::f32, mem);
    }
    case BinaryConsts::F64LoadMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeLoad(8, false, offset, align, Type::f64, mem);
    }
    case BinaryConsts::I32StoreMem8: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(1, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I32StoreMem16: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(2, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I32StoreMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(4, offset, align, Type::i32, mem);
    }
    case BinaryConsts::I64StoreMem8: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(1, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64StoreMem16: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(2, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64StoreMem32: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(4, offset, align, Type::i64, mem);
    }
    case BinaryConsts::I64StoreMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(8, offset, align, Type::i64, mem);
    }
    case BinaryConsts::F32StoreMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(4, offset, align, Type::f32, mem);
    }
    case BinaryConsts::F64StoreMem: {
      auto [mem, align, offset] = getMemarg();
      return builder.makeStore(8, offset, align, Type::f64, mem);
    }
    case BinaryConsts::AtomicPrefix: {
      auto op = getU32LEB();
      switch (op) {
        case BinaryConsts::I32AtomicLoad8U: {
          // TODO: pass align through for validation.
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(1, offset, Type::i32, mem);
        }
        case BinaryConsts::I32AtomicLoad16U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(2, offset, Type::i32, mem);
        }
        case BinaryConsts::I32AtomicLoad: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(4, offset, Type::i32, mem);
        }
        case BinaryConsts::I64AtomicLoad8U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(1, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicLoad16U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(2, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicLoad32U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(4, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicLoad: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicLoad(8, offset, Type::i64, mem);
        }
        case BinaryConsts::I32AtomicStore8: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(1, offset, Type::i32, mem);
        }
        case BinaryConsts::I32AtomicStore16: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(2, offset, Type::i32, mem);
        }
        case BinaryConsts::I32AtomicStore: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(4, offset, Type::i32, mem);
        }
        case BinaryConsts::I64AtomicStore8: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(1, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicStore16: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(2, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicStore32: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(4, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicStore: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicStore(8, offset, Type::i64, mem);
        }

#define RMW(op)                                                                \
  case BinaryConsts::I32AtomicRMW##op: {                                       \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 4, offset, Type::i32, mem);          \
  }                                                                            \
  case BinaryConsts::I32AtomicRMW##op##8U: {                                   \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 1, offset, Type::i32, mem);          \
  }                                                                            \
  case BinaryConsts::I32AtomicRMW##op##16U: {                                  \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 2, offset, Type::i32, mem);          \
  }                                                                            \
  case BinaryConsts::I64AtomicRMW##op: {                                       \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 8, offset, Type::i64, mem);          \
  }                                                                            \
  case BinaryConsts::I64AtomicRMW##op##8U: {                                   \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 1, offset, Type::i64, mem);          \
  }                                                                            \
  case BinaryConsts::I64AtomicRMW##op##16U: {                                  \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 2, offset, Type::i64, mem);          \
  }                                                                            \
  case BinaryConsts::I64AtomicRMW##op##32U: {                                  \
    auto [mem, align, offset] = getMemarg();                                   \
    return builder.makeAtomicRMW(RMW##op, 4, offset, Type::i64, mem);          \
  }

          RMW(Add);
          RMW(Sub);
          RMW(And);
          RMW(Or);
          RMW(Xor);
          RMW(Xchg);

        case BinaryConsts::I32AtomicCmpxchg: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(4, offset, Type::i32, mem);
        }
        case BinaryConsts::I32AtomicCmpxchg8U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(1, offset, Type::i32, mem);
        }
        case BinaryConsts::I32AtomicCmpxchg16U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(2, offset, Type::i32, mem);
        }
        case BinaryConsts::I64AtomicCmpxchg: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(8, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicCmpxchg8U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(1, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicCmpxchg16U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(2, offset, Type::i64, mem);
        }
        case BinaryConsts::I64AtomicCmpxchg32U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicCmpxchg(4, offset, Type::i64, mem);
        }
        case BinaryConsts::I32AtomicWait: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicWait(Type::i32, offset, mem);
        }
        case BinaryConsts::I64AtomicWait: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicWait(Type::i64, offset, mem);
        }
        case BinaryConsts::AtomicNotify: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeAtomicNotify(offset, mem);
        }
        case BinaryConsts::AtomicFence:
          if (getInt8() != 0) {
            return Err{"expected 0x00 byte immediate on atomic.fence"};
          }
          return builder.makeAtomicFence();
      }
      return Err{"unknown atomic operation"};
    }
    case BinaryConsts::MiscPrefix: {
      auto op = getU32LEB();
      switch (op) {
        case BinaryConsts::I32STruncSatF32:
          return builder.makeUnary(TruncSatSFloat32ToInt32);
        case BinaryConsts::I32UTruncSatF32:
          return builder.makeUnary(TruncSatUFloat32ToInt32);
        case BinaryConsts::I32STruncSatF64:
          return builder.makeUnary(TruncSatSFloat64ToInt32);
        case BinaryConsts::I32UTruncSatF64:
          return builder.makeUnary(TruncSatUFloat64ToInt32);
        case BinaryConsts::I64STruncSatF32:
          return builder.makeUnary(TruncSatSFloat32ToInt64);
        case BinaryConsts::I64UTruncSatF32:
          return builder.makeUnary(TruncSatUFloat32ToInt64);
        case BinaryConsts::I64STruncSatF64:
          return builder.makeUnary(TruncSatSFloat64ToInt64);
        case BinaryConsts::I64UTruncSatF64:
          return builder.makeUnary(TruncSatUFloat64ToInt64);
        case BinaryConsts::MemoryInit: {
          auto data = getDataName(getU32LEB());
          auto mem = getMemoryName(getU32LEB());
          return builder.makeMemoryInit(data, mem);
        }
        case BinaryConsts::DataDrop:
          return builder.makeDataDrop(getDataName(getU32LEB()));
        case BinaryConsts::MemoryCopy: {
          auto dest = getMemoryName(getU32LEB());
          auto src = getMemoryName(getU32LEB());
          return builder.makeMemoryCopy(dest, src);
        }
        case BinaryConsts::MemoryFill:
          return builder.makeMemoryFill(getMemoryName(getU32LEB()));
        case BinaryConsts::TableSize:
          return builder.makeTableSize(getTableName(getU32LEB()));
        case BinaryConsts::TableGrow:
          return builder.makeTableGrow(getTableName(getU32LEB()));
        case BinaryConsts::TableFill:
          return builder.makeTableFill(getTableName(getU32LEB()));
        case BinaryConsts::TableCopy: {
          auto dest = getTableName(getU32LEB());
          auto src = getTableName(getU32LEB());
          return builder.makeTableCopy(dest, src);
        }
        case BinaryConsts::TableInit: {
          auto elem = getElemName(getU32LEB());
          auto table = getTableName(getU32LEB());
          return builder.makeTableInit(elem, table);
        }
        case BinaryConsts::F32_F16LoadMem: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeLoad(2, false, offset, align, Type::f32, mem);
        }
        case BinaryConsts::F32_F16StoreMem: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeStore(2, offset, align, Type::f32, mem);
        }
      }
      return Err{"unknown misc operation"};
    }
    case BinaryConsts::SIMDPrefix: {
      auto op = getU32LEB();
      switch (op) {
        case BinaryConsts::I8x16Eq:
          return builder.makeBinary(EqVecI8x16);
        case BinaryConsts::I8x16Ne:
          return builder.makeBinary(NeVecI8x16);
        case BinaryConsts::I8x16LtS:
          return builder.makeBinary(LtSVecI8x16);
        case BinaryConsts::I8x16LtU:
          return builder.makeBinary(LtUVecI8x16);
        case BinaryConsts::I8x16GtS:
          return builder.makeBinary(GtSVecI8x16);
        case BinaryConsts::I8x16GtU:
          return builder.makeBinary(GtUVecI8x16);
        case BinaryConsts::I8x16LeS:
          return builder.makeBinary(LeSVecI8x16);
        case BinaryConsts::I8x16LeU:
          return builder.makeBinary(LeUVecI8x16);
        case BinaryConsts::I8x16GeS:
          return builder.makeBinary(GeSVecI8x16);
        case BinaryConsts::I8x16GeU:
          return builder.makeBinary(GeUVecI8x16);
        case BinaryConsts::I16x8Eq:
          return builder.makeBinary(EqVecI16x8);
        case BinaryConsts::I16x8Ne:
          return builder.makeBinary(NeVecI16x8);
        case BinaryConsts::I16x8LtS:
          return builder.makeBinary(LtSVecI16x8);
        case BinaryConsts::I16x8LtU:
          return builder.makeBinary(LtUVecI16x8);
        case BinaryConsts::I16x8GtS:
          return builder.makeBinary(GtSVecI16x8);
        case BinaryConsts::I16x8GtU:
          return builder.makeBinary(GtUVecI16x8);
        case BinaryConsts::I16x8LeS:
          return builder.makeBinary(LeSVecI16x8);
        case BinaryConsts::I16x8LeU:
          return builder.makeBinary(LeUVecI16x8);
        case BinaryConsts::I16x8GeS:
          return builder.makeBinary(GeSVecI16x8);
        case BinaryConsts::I16x8GeU:
          return builder.makeBinary(GeUVecI16x8);
        case BinaryConsts::I32x4Eq:
          return builder.makeBinary(EqVecI32x4);
        case BinaryConsts::I32x4Ne:
          return builder.makeBinary(NeVecI32x4);
        case BinaryConsts::I32x4LtS:
          return builder.makeBinary(LtSVecI32x4);
        case BinaryConsts::I32x4LtU:
          return builder.makeBinary(LtUVecI32x4);
        case BinaryConsts::I32x4GtS:
          return builder.makeBinary(GtSVecI32x4);
        case BinaryConsts::I32x4GtU:
          return builder.makeBinary(GtUVecI32x4);
        case BinaryConsts::I32x4LeS:
          return builder.makeBinary(LeSVecI32x4);
        case BinaryConsts::I32x4LeU:
          return builder.makeBinary(LeUVecI32x4);
        case BinaryConsts::I32x4GeS:
          return builder.makeBinary(GeSVecI32x4);
        case BinaryConsts::I32x4GeU:
          return builder.makeBinary(GeUVecI32x4);
        case BinaryConsts::I64x2Eq:
          return builder.makeBinary(EqVecI64x2);
        case BinaryConsts::I64x2Ne:
          return builder.makeBinary(NeVecI64x2);
        case BinaryConsts::I64x2LtS:
          return builder.makeBinary(LtSVecI64x2);
        case BinaryConsts::I64x2GtS:
          return builder.makeBinary(GtSVecI64x2);
        case BinaryConsts::I64x2LeS:
          return builder.makeBinary(LeSVecI64x2);
        case BinaryConsts::I64x2GeS:
          return builder.makeBinary(GeSVecI64x2);
        case BinaryConsts::F16x8Eq:
          return builder.makeBinary(EqVecF16x8);
        case BinaryConsts::F16x8Ne:
          return builder.makeBinary(NeVecF16x8);
        case BinaryConsts::F16x8Lt:
          return builder.makeBinary(LtVecF16x8);
        case BinaryConsts::F16x8Gt:
          return builder.makeBinary(GtVecF16x8);
        case BinaryConsts::F16x8Le:
          return builder.makeBinary(LeVecF16x8);
        case BinaryConsts::F16x8Ge:
          return builder.makeBinary(GeVecF16x8);
        case BinaryConsts::F32x4Eq:
          return builder.makeBinary(EqVecF32x4);
        case BinaryConsts::F32x4Ne:
          return builder.makeBinary(NeVecF32x4);
        case BinaryConsts::F32x4Lt:
          return builder.makeBinary(LtVecF32x4);
        case BinaryConsts::F32x4Gt:
          return builder.makeBinary(GtVecF32x4);
        case BinaryConsts::F32x4Le:
          return builder.makeBinary(LeVecF32x4);
        case BinaryConsts::F32x4Ge:
          return builder.makeBinary(GeVecF32x4);
        case BinaryConsts::F64x2Eq:
          return builder.makeBinary(EqVecF64x2);
        case BinaryConsts::F64x2Ne:
          return builder.makeBinary(NeVecF64x2);
        case BinaryConsts::F64x2Lt:
          return builder.makeBinary(LtVecF64x2);
        case BinaryConsts::F64x2Gt:
          return builder.makeBinary(GtVecF64x2);
        case BinaryConsts::F64x2Le:
          return builder.makeBinary(LeVecF64x2);
        case BinaryConsts::F64x2Ge:
          return builder.makeBinary(GeVecF64x2);
        case BinaryConsts::V128And:
          return builder.makeBinary(AndVec128);
        case BinaryConsts::V128Or:
          return builder.makeBinary(OrVec128);
        case BinaryConsts::V128Xor:
          return builder.makeBinary(XorVec128);
        case BinaryConsts::V128Andnot:
          return builder.makeBinary(AndNotVec128);
        case BinaryConsts::I8x16Add:
          return builder.makeBinary(AddVecI8x16);
        case BinaryConsts::I8x16AddSatS:
          return builder.makeBinary(AddSatSVecI8x16);
        case BinaryConsts::I8x16AddSatU:
          return builder.makeBinary(AddSatUVecI8x16);
        case BinaryConsts::I8x16Sub:
          return builder.makeBinary(SubVecI8x16);
        case BinaryConsts::I8x16SubSatS:
          return builder.makeBinary(SubSatSVecI8x16);
        case BinaryConsts::I8x16SubSatU:
          return builder.makeBinary(SubSatUVecI8x16);
        case BinaryConsts::I8x16MinS:
          return builder.makeBinary(MinSVecI8x16);
        case BinaryConsts::I8x16MinU:
          return builder.makeBinary(MinUVecI8x16);
        case BinaryConsts::I8x16MaxS:
          return builder.makeBinary(MaxSVecI8x16);
        case BinaryConsts::I8x16MaxU:
          return builder.makeBinary(MaxUVecI8x16);
        case BinaryConsts::I8x16AvgrU:
          return builder.makeBinary(AvgrUVecI8x16);
        case BinaryConsts::I16x8Add:
          return builder.makeBinary(AddVecI16x8);
        case BinaryConsts::I16x8AddSatS:
          return builder.makeBinary(AddSatSVecI16x8);
        case BinaryConsts::I16x8AddSatU:
          return builder.makeBinary(AddSatUVecI16x8);
        case BinaryConsts::I16x8Sub:
          return builder.makeBinary(SubVecI16x8);
        case BinaryConsts::I16x8SubSatS:
          return builder.makeBinary(SubSatSVecI16x8);
        case BinaryConsts::I16x8SubSatU:
          return builder.makeBinary(SubSatUVecI16x8);
        case BinaryConsts::I16x8Mul:
          return builder.makeBinary(MulVecI16x8);
        case BinaryConsts::I16x8MinS:
          return builder.makeBinary(MinSVecI16x8);
        case BinaryConsts::I16x8MinU:
          return builder.makeBinary(MinUVecI16x8);
        case BinaryConsts::I16x8MaxS:
          return builder.makeBinary(MaxSVecI16x8);
        case BinaryConsts::I16x8MaxU:
          return builder.makeBinary(MaxUVecI16x8);
        case BinaryConsts::I16x8AvgrU:
          return builder.makeBinary(AvgrUVecI16x8);
        case BinaryConsts::I16x8Q15MulrSatS:
          return builder.makeBinary(Q15MulrSatSVecI16x8);
        case BinaryConsts::I16x8ExtmulLowI8x16S:
          return builder.makeBinary(ExtMulLowSVecI16x8);
        case BinaryConsts::I16x8ExtmulHighI8x16S:
          return builder.makeBinary(ExtMulHighSVecI16x8);
        case BinaryConsts::I16x8ExtmulLowI8x16U:
          return builder.makeBinary(ExtMulLowUVecI16x8);
        case BinaryConsts::I16x8ExtmulHighI8x16U:
          return builder.makeBinary(ExtMulHighUVecI16x8);
        case BinaryConsts::I32x4Add:
          return builder.makeBinary(AddVecI32x4);
        case BinaryConsts::I32x4Sub:
          return builder.makeBinary(SubVecI32x4);
        case BinaryConsts::I32x4Mul:
          return builder.makeBinary(MulVecI32x4);
        case BinaryConsts::I32x4MinS:
          return builder.makeBinary(MinSVecI32x4);
        case BinaryConsts::I32x4MinU:
          return builder.makeBinary(MinUVecI32x4);
        case BinaryConsts::I32x4MaxS:
          return builder.makeBinary(MaxSVecI32x4);
        case BinaryConsts::I32x4MaxU:
          return builder.makeBinary(MaxUVecI32x4);
        case BinaryConsts::I32x4DotI16x8S:
          return builder.makeBinary(DotSVecI16x8ToVecI32x4);
        case BinaryConsts::I32x4ExtmulLowI16x8S:
          return builder.makeBinary(ExtMulLowSVecI32x4);
        case BinaryConsts::I32x4ExtmulHighI16x8S:
          return builder.makeBinary(ExtMulHighSVecI32x4);
        case BinaryConsts::I32x4ExtmulLowI16x8U:
          return builder.makeBinary(ExtMulLowUVecI32x4);
        case BinaryConsts::I32x4ExtmulHighI16x8U:
          return builder.makeBinary(ExtMulHighUVecI32x4);
        case BinaryConsts::I64x2Add:
          return builder.makeBinary(AddVecI64x2);
        case BinaryConsts::I64x2Sub:
          return builder.makeBinary(SubVecI64x2);
        case BinaryConsts::I64x2Mul:
          return builder.makeBinary(MulVecI64x2);
        case BinaryConsts::I64x2ExtmulLowI32x4S:
          return builder.makeBinary(ExtMulLowSVecI64x2);
        case BinaryConsts::I64x2ExtmulHighI32x4S:
          return builder.makeBinary(ExtMulHighSVecI64x2);
        case BinaryConsts::I64x2ExtmulLowI32x4U:
          return builder.makeBinary(ExtMulLowUVecI64x2);
        case BinaryConsts::I64x2ExtmulHighI32x4U:
          return builder.makeBinary(ExtMulHighUVecI64x2);
        case BinaryConsts::F16x8Add:
          return builder.makeBinary(AddVecF16x8);
        case BinaryConsts::F16x8Sub:
          return builder.makeBinary(SubVecF16x8);
        case BinaryConsts::F16x8Mul:
          return builder.makeBinary(MulVecF16x8);
        case BinaryConsts::F16x8Div:
          return builder.makeBinary(DivVecF16x8);
        case BinaryConsts::F16x8Min:
          return builder.makeBinary(MinVecF16x8);
        case BinaryConsts::F16x8Max:
          return builder.makeBinary(MaxVecF16x8);
        case BinaryConsts::F16x8Pmin:
          return builder.makeBinary(PMinVecF16x8);
        case BinaryConsts::F16x8Pmax:
          return builder.makeBinary(PMaxVecF16x8);
        case BinaryConsts::F32x4Add:
          return builder.makeBinary(AddVecF32x4);
        case BinaryConsts::F32x4Sub:
          return builder.makeBinary(SubVecF32x4);
        case BinaryConsts::F32x4Mul:
          return builder.makeBinary(MulVecF32x4);
        case BinaryConsts::F32x4Div:
          return builder.makeBinary(DivVecF32x4);
        case BinaryConsts::F32x4Min:
          return builder.makeBinary(MinVecF32x4);
        case BinaryConsts::F32x4Max:
          return builder.makeBinary(MaxVecF32x4);
        case BinaryConsts::F32x4Pmin:
          return builder.makeBinary(PMinVecF32x4);
        case BinaryConsts::F32x4Pmax:
          return builder.makeBinary(PMaxVecF32x4);
        case BinaryConsts::F64x2Add:
          return builder.makeBinary(AddVecF64x2);
        case BinaryConsts::F64x2Sub:
          return builder.makeBinary(SubVecF64x2);
        case BinaryConsts::F64x2Mul:
          return builder.makeBinary(MulVecF64x2);
        case BinaryConsts::F64x2Div:
          return builder.makeBinary(DivVecF64x2);
        case BinaryConsts::F64x2Min:
          return builder.makeBinary(MinVecF64x2);
        case BinaryConsts::F64x2Max:
          return builder.makeBinary(MaxVecF64x2);
        case BinaryConsts::F64x2Pmin:
          return builder.makeBinary(PMinVecF64x2);
        case BinaryConsts::F64x2Pmax:
          return builder.makeBinary(PMaxVecF64x2);
        case BinaryConsts::I8x16NarrowI16x8S:
          return builder.makeBinary(NarrowSVecI16x8ToVecI8x16);
        case BinaryConsts::I8x16NarrowI16x8U:
          return builder.makeBinary(NarrowUVecI16x8ToVecI8x16);
        case BinaryConsts::I16x8NarrowI32x4S:
          return builder.makeBinary(NarrowSVecI32x4ToVecI16x8);
        case BinaryConsts::I16x8NarrowI32x4U:
          return builder.makeBinary(NarrowUVecI32x4ToVecI16x8);
        case BinaryConsts::I8x16Swizzle:
          return builder.makeBinary(SwizzleVecI8x16);
        case BinaryConsts::I8x16RelaxedSwizzle:
          return builder.makeBinary(RelaxedSwizzleVecI8x16);
        case BinaryConsts::F32x4RelaxedMin:
          return builder.makeBinary(RelaxedMinVecF32x4);
        case BinaryConsts::F32x4RelaxedMax:
          return builder.makeBinary(RelaxedMaxVecF32x4);
        case BinaryConsts::F64x2RelaxedMin:
          return builder.makeBinary(RelaxedMinVecF64x2);
        case BinaryConsts::F64x2RelaxedMax:
          return builder.makeBinary(RelaxedMaxVecF64x2);
        case BinaryConsts::I16x8RelaxedQ15MulrS:
          return builder.makeBinary(RelaxedQ15MulrSVecI16x8);
        case BinaryConsts::I16x8DotI8x16I7x16S:
          return builder.makeBinary(DotI8x16I7x16SToVecI16x8);
        case BinaryConsts::I8x16Splat:
          return builder.makeUnary(SplatVecI8x16);
        case BinaryConsts::I16x8Splat:
          return builder.makeUnary(SplatVecI16x8);
        case BinaryConsts::I32x4Splat:
          return builder.makeUnary(SplatVecI32x4);
        case BinaryConsts::I64x2Splat:
          return builder.makeUnary(SplatVecI64x2);
        case BinaryConsts::F16x8Splat:
          return builder.makeUnary(SplatVecF16x8);
        case BinaryConsts::F32x4Splat:
          return builder.makeUnary(SplatVecF32x4);
        case BinaryConsts::F64x2Splat:
          return builder.makeUnary(SplatVecF64x2);
        case BinaryConsts::V128Not:
          return builder.makeUnary(NotVec128);
        case BinaryConsts::V128AnyTrue:
          return builder.makeUnary(AnyTrueVec128);
        case BinaryConsts::I8x16Popcnt:
          return builder.makeUnary(PopcntVecI8x16);
        case BinaryConsts::I8x16Abs:
          return builder.makeUnary(AbsVecI8x16);
        case BinaryConsts::I8x16Neg:
          return builder.makeUnary(NegVecI8x16);
        case BinaryConsts::I8x16AllTrue:
          return builder.makeUnary(AllTrueVecI8x16);
        case BinaryConsts::I8x16Bitmask:
          return builder.makeUnary(BitmaskVecI8x16);
        case BinaryConsts::I16x8Abs:
          return builder.makeUnary(AbsVecI16x8);
        case BinaryConsts::I16x8Neg:
          return builder.makeUnary(NegVecI16x8);
        case BinaryConsts::I16x8AllTrue:
          return builder.makeUnary(AllTrueVecI16x8);
        case BinaryConsts::I16x8Bitmask:
          return builder.makeUnary(BitmaskVecI16x8);
        case BinaryConsts::I32x4Abs:
          return builder.makeUnary(AbsVecI32x4);
        case BinaryConsts::I32x4Neg:
          return builder.makeUnary(NegVecI32x4);
        case BinaryConsts::I32x4AllTrue:
          return builder.makeUnary(AllTrueVecI32x4);
        case BinaryConsts::I32x4Bitmask:
          return builder.makeUnary(BitmaskVecI32x4);
        case BinaryConsts::I64x2Abs:
          return builder.makeUnary(AbsVecI64x2);
        case BinaryConsts::I64x2Neg:
          return builder.makeUnary(NegVecI64x2);
        case BinaryConsts::I64x2AllTrue:
          return builder.makeUnary(AllTrueVecI64x2);
        case BinaryConsts::I64x2Bitmask:
          return builder.makeUnary(BitmaskVecI64x2);
        case BinaryConsts::F16x8Abs:
          return builder.makeUnary(AbsVecF16x8);
        case BinaryConsts::F16x8Neg:
          return builder.makeUnary(NegVecF16x8);
        case BinaryConsts::F16x8Sqrt:
          return builder.makeUnary(SqrtVecF16x8);
        case BinaryConsts::F16x8Ceil:
          return builder.makeUnary(CeilVecF16x8);
        case BinaryConsts::F16x8Floor:
          return builder.makeUnary(FloorVecF16x8);
        case BinaryConsts::F16x8Trunc:
          return builder.makeUnary(TruncVecF16x8);
        case BinaryConsts::F16x8Nearest:
          return builder.makeUnary(NearestVecF16x8);
        case BinaryConsts::F32x4Abs:
          return builder.makeUnary(AbsVecF32x4);
        case BinaryConsts::F32x4Neg:
          return builder.makeUnary(NegVecF32x4);
        case BinaryConsts::F32x4Sqrt:
          return builder.makeUnary(SqrtVecF32x4);
        case BinaryConsts::F32x4Ceil:
          return builder.makeUnary(CeilVecF32x4);
        case BinaryConsts::F32x4Floor:
          return builder.makeUnary(FloorVecF32x4);
        case BinaryConsts::F32x4Trunc:
          return builder.makeUnary(TruncVecF32x4);
        case BinaryConsts::F32x4Nearest:
          return builder.makeUnary(NearestVecF32x4);
        case BinaryConsts::F64x2Abs:
          return builder.makeUnary(AbsVecF64x2);
        case BinaryConsts::F64x2Neg:
          return builder.makeUnary(NegVecF64x2);
        case BinaryConsts::F64x2Sqrt:
          return builder.makeUnary(SqrtVecF64x2);
        case BinaryConsts::F64x2Ceil:
          return builder.makeUnary(CeilVecF64x2);
        case BinaryConsts::F64x2Floor:
          return builder.makeUnary(FloorVecF64x2);
        case BinaryConsts::F64x2Trunc:
          return builder.makeUnary(TruncVecF64x2);
        case BinaryConsts::F64x2Nearest:
          return builder.makeUnary(NearestVecF64x2);
        case BinaryConsts::I16x8ExtaddPairwiseI8x16S:
          return builder.makeUnary(ExtAddPairwiseSVecI8x16ToI16x8);
        case BinaryConsts::I16x8ExtaddPairwiseI8x16U:
          return builder.makeUnary(ExtAddPairwiseUVecI8x16ToI16x8);
        case BinaryConsts::I32x4ExtaddPairwiseI16x8S:
          return builder.makeUnary(ExtAddPairwiseSVecI16x8ToI32x4);
        case BinaryConsts::I32x4ExtaddPairwiseI16x8U:
          return builder.makeUnary(ExtAddPairwiseUVecI16x8ToI32x4);
        case BinaryConsts::I32x4TruncSatF32x4S:
          return builder.makeUnary(TruncSatSVecF32x4ToVecI32x4);
        case BinaryConsts::I32x4TruncSatF32x4U:
          return builder.makeUnary(TruncSatUVecF32x4ToVecI32x4);
        case BinaryConsts::F32x4ConvertI32x4S:
          return builder.makeUnary(ConvertSVecI32x4ToVecF32x4);
        case BinaryConsts::F32x4ConvertI32x4U:
          return builder.makeUnary(ConvertUVecI32x4ToVecF32x4);
        case BinaryConsts::I16x8ExtendLowI8x16S:
          return builder.makeUnary(ExtendLowSVecI8x16ToVecI16x8);
        case BinaryConsts::I16x8ExtendHighI8x16S:
          return builder.makeUnary(ExtendHighSVecI8x16ToVecI16x8);
        case BinaryConsts::I16x8ExtendLowI8x16U:
          return builder.makeUnary(ExtendLowUVecI8x16ToVecI16x8);
        case BinaryConsts::I16x8ExtendHighI8x16U:
          return builder.makeUnary(ExtendHighUVecI8x16ToVecI16x8);
        case BinaryConsts::I32x4ExtendLowI16x8S:
          return builder.makeUnary(ExtendLowSVecI16x8ToVecI32x4);
        case BinaryConsts::I32x4ExtendHighI16x8S:
          return builder.makeUnary(ExtendHighSVecI16x8ToVecI32x4);
        case BinaryConsts::I32x4ExtendLowI16x8U:
          return builder.makeUnary(ExtendLowUVecI16x8ToVecI32x4);
        case BinaryConsts::I32x4ExtendHighI16x8U:
          return builder.makeUnary(ExtendHighUVecI16x8ToVecI32x4);
        case BinaryConsts::I64x2ExtendLowI32x4S:
          return builder.makeUnary(ExtendLowSVecI32x4ToVecI64x2);
        case BinaryConsts::I64x2ExtendHighI32x4S:
          return builder.makeUnary(ExtendHighSVecI32x4ToVecI64x2);
        case BinaryConsts::I64x2ExtendLowI32x4U:
          return builder.makeUnary(ExtendLowUVecI32x4ToVecI64x2);
        case BinaryConsts::I64x2ExtendHighI32x4U:
          return builder.makeUnary(ExtendHighUVecI32x4ToVecI64x2);
        case BinaryConsts::F64x2ConvertLowI32x4S:
          return builder.makeUnary(ConvertLowSVecI32x4ToVecF64x2);
        case BinaryConsts::F64x2ConvertLowI32x4U:
          return builder.makeUnary(ConvertLowUVecI32x4ToVecF64x2);
        case BinaryConsts::I32x4TruncSatF64x2SZero:
          return builder.makeUnary(TruncSatZeroSVecF64x2ToVecI32x4);
        case BinaryConsts::I32x4TruncSatF64x2UZero:
          return builder.makeUnary(TruncSatZeroUVecF64x2ToVecI32x4);
        case BinaryConsts::F32x4DemoteF64x2Zero:
          return builder.makeUnary(DemoteZeroVecF64x2ToVecF32x4);
        case BinaryConsts::F64x2PromoteLowF32x4:
          return builder.makeUnary(PromoteLowVecF32x4ToVecF64x2);
        case BinaryConsts::I32x4RelaxedTruncF32x4S:
          return builder.makeUnary(RelaxedTruncSVecF32x4ToVecI32x4);
        case BinaryConsts::I32x4RelaxedTruncF32x4U:
          return builder.makeUnary(RelaxedTruncUVecF32x4ToVecI32x4);
        case BinaryConsts::I32x4RelaxedTruncF64x2SZero:
          return builder.makeUnary(RelaxedTruncZeroSVecF64x2ToVecI32x4);
        case BinaryConsts::I32x4RelaxedTruncF64x2UZero:
          return builder.makeUnary(RelaxedTruncZeroUVecF64x2ToVecI32x4);
        case BinaryConsts::I16x8TruncSatF16x8S:
          return builder.makeUnary(TruncSatSVecF16x8ToVecI16x8);
        case BinaryConsts::I16x8TruncSatF16x8U:
          return builder.makeUnary(TruncSatUVecF16x8ToVecI16x8);
        case BinaryConsts::F16x8ConvertI16x8S:
          return builder.makeUnary(ConvertSVecI16x8ToVecF16x8);
        case BinaryConsts::F16x8ConvertI16x8U:
          return builder.makeUnary(ConvertUVecI16x8ToVecF16x8);
        case BinaryConsts::I8x16ExtractLaneS:
          return builder.makeSIMDExtract(ExtractLaneSVecI8x16,
                                         getLaneIndex(16));
        case BinaryConsts::I8x16ExtractLaneU:
          return builder.makeSIMDExtract(ExtractLaneUVecI8x16,
                                         getLaneIndex(16));
        case BinaryConsts::I16x8ExtractLaneS:
          return builder.makeSIMDExtract(ExtractLaneSVecI16x8, getLaneIndex(8));
        case BinaryConsts::I16x8ExtractLaneU:
          return builder.makeSIMDExtract(ExtractLaneUVecI16x8, getLaneIndex(8));
        case BinaryConsts::I32x4ExtractLane:
          return builder.makeSIMDExtract(ExtractLaneVecI32x4, getLaneIndex(4));
        case BinaryConsts::I64x2ExtractLane:
          return builder.makeSIMDExtract(ExtractLaneVecI64x2, getLaneIndex(2));
        case BinaryConsts::F16x8ExtractLane:
          return builder.makeSIMDExtract(ExtractLaneVecF16x8, getLaneIndex(8));
        case BinaryConsts::F32x4ExtractLane:
          return builder.makeSIMDExtract(ExtractLaneVecF32x4, getLaneIndex(4));
        case BinaryConsts::F64x2ExtractLane:
          return builder.makeSIMDExtract(ExtractLaneVecF64x2, getLaneIndex(2));
        case BinaryConsts::I8x16ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecI8x16, getLaneIndex(16));
        case BinaryConsts::I16x8ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecI16x8, getLaneIndex(8));
        case BinaryConsts::I32x4ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecI32x4, getLaneIndex(4));
        case BinaryConsts::I64x2ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecI64x2, getLaneIndex(2));
        case BinaryConsts::F16x8ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecF16x8, getLaneIndex(8));
        case BinaryConsts::F32x4ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecF32x4, getLaneIndex(4));
        case BinaryConsts::F64x2ReplaceLane:
          return builder.makeSIMDReplace(ReplaceLaneVecF64x2, getLaneIndex(2));
        case BinaryConsts::I8x16Shuffle: {
          std::array<uint8_t, 16> lanes;
          for (Index i = 0; i < 16; ++i) {
            lanes[i] = getLaneIndex(32);
          }
          return builder.makeSIMDShuffle(lanes);
        }
        case BinaryConsts::V128Bitselect:
          return builder.makeSIMDTernary(Bitselect);
        case BinaryConsts::I8x16Laneselect:
          return builder.makeSIMDTernary(LaneselectI8x16);
        case BinaryConsts::I16x8Laneselect:
          return builder.makeSIMDTernary(LaneselectI16x8);
        case BinaryConsts::I32x4Laneselect:
          return builder.makeSIMDTernary(LaneselectI32x4);
        case BinaryConsts::I64x2Laneselect:
          return builder.makeSIMDTernary(LaneselectI64x2);
        case BinaryConsts::F16x8RelaxedMadd:
          return builder.makeSIMDTernary(RelaxedMaddVecF16x8);
        case BinaryConsts::F16x8RelaxedNmadd:
          return builder.makeSIMDTernary(RelaxedNmaddVecF16x8);
        case BinaryConsts::F32x4RelaxedMadd:
          return builder.makeSIMDTernary(RelaxedMaddVecF32x4);
        case BinaryConsts::F32x4RelaxedNmadd:
          return builder.makeSIMDTernary(RelaxedNmaddVecF32x4);
        case BinaryConsts::F64x2RelaxedMadd:
          return builder.makeSIMDTernary(RelaxedMaddVecF64x2);
        case BinaryConsts::F64x2RelaxedNmadd:
          return builder.makeSIMDTernary(RelaxedNmaddVecF64x2);
        case BinaryConsts::I32x4DotI8x16I7x16AddS:
          return builder.makeSIMDTernary(DotI8x16I7x16AddSToVecI32x4);
        case BinaryConsts::I8x16Shl:
          return builder.makeSIMDShift(ShlVecI8x16);
        case BinaryConsts::I8x16ShrS:
          return builder.makeSIMDShift(ShrSVecI8x16);
        case BinaryConsts::I8x16ShrU:
          return builder.makeSIMDShift(ShrUVecI8x16);
        case BinaryConsts::I16x8Shl:
          return builder.makeSIMDShift(ShlVecI16x8);
        case BinaryConsts::I16x8ShrS:
          return builder.makeSIMDShift(ShrSVecI16x8);
        case BinaryConsts::I16x8ShrU:
          return builder.makeSIMDShift(ShrUVecI16x8);
        case BinaryConsts::I32x4Shl:
          return builder.makeSIMDShift(ShlVecI32x4);
        case BinaryConsts::I32x4ShrS:
          return builder.makeSIMDShift(ShrSVecI32x4);
        case BinaryConsts::I32x4ShrU:
          return builder.makeSIMDShift(ShrUVecI32x4);
        case BinaryConsts::I64x2Shl:
          return builder.makeSIMDShift(ShlVecI64x2);
        case BinaryConsts::I64x2ShrS:
          return builder.makeSIMDShift(ShrSVecI64x2);
        case BinaryConsts::I64x2ShrU:
          return builder.makeSIMDShift(ShrUVecI64x2);
        case BinaryConsts::V128Const:
          return builder.makeConst(getVec128Literal());
        case BinaryConsts::V128Store: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeStore(16, offset, align, Type::v128, mem);
        }
        case BinaryConsts::V128Load: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeLoad(16, false, offset, align, Type::v128, mem);
        }
        case BinaryConsts::V128Load8Splat: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load8SplatVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load16Splat: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load16SplatVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load32Splat: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load32SplatVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load64Splat: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load64SplatVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load8x8S: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load8x8SVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load8x8U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load8x8UVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load16x4S: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load16x4SVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load16x4U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load16x4UVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load32x2S: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load32x2SVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load32x2U: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load32x2UVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load32Zero: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load32ZeroVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load64Zero: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoad(Load64ZeroVec128, offset, align, mem);
        }
        case BinaryConsts::V128Load8Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Load8LaneVec128, offset, align, getLaneIndex(16), mem);
        }
        case BinaryConsts::V128Load16Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Load16LaneVec128, offset, align, getLaneIndex(8), mem);
        }
        case BinaryConsts::V128Load32Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Load32LaneVec128, offset, align, getLaneIndex(4), mem);
        }
        case BinaryConsts::V128Load64Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Load64LaneVec128, offset, align, getLaneIndex(2), mem);
        }
        case BinaryConsts::V128Store8Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Store8LaneVec128, offset, align, getLaneIndex(16), mem);
        }
        case BinaryConsts::V128Store16Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Store16LaneVec128, offset, align, getLaneIndex(8), mem);
        }
        case BinaryConsts::V128Store32Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Store32LaneVec128, offset, align, getLaneIndex(4), mem);
        }
        case BinaryConsts::V128Store64Lane: {
          auto [mem, align, offset] = getMemarg();
          return builder.makeSIMDLoadStoreLane(
            Store64LaneVec128, offset, align, getLaneIndex(2), mem);
        }
      }
      return Err{"unknown SIMD operation"};
    }
    case BinaryConsts::GCPrefix: {
      auto op = getU32LEB();
      switch (op) {
        case BinaryConsts::RefI31:
          return builder.makeRefI31(Unshared);
        case BinaryConsts::RefI31Shared:
          return builder.makeRefI31(Shared);
        case BinaryConsts::I31GetS:
          return builder.makeI31Get(true);
        case BinaryConsts::I31GetU:
          return builder.makeI31Get(false);
        case BinaryConsts::RefTest:
          return builder.makeRefTest(Type(getHeapType(), NonNullable));
        case BinaryConsts::RefTestNull:
          return builder.makeRefTest(Type(getHeapType(), Nullable));
        case BinaryConsts::RefCast:
          return builder.makeRefCast(Type(getHeapType(), NonNullable));
        case BinaryConsts::RefCastNull:
          return builder.makeRefCast(Type(getHeapType(), Nullable));
        case BinaryConsts::BrOnCast:
        case BinaryConsts::BrOnCastFail: {
          auto flags = getInt8();
          auto label = getU32LEB();
          auto in = Type(getHeapType(), (flags & 1) ? Nullable : NonNullable);
          auto cast = Type(getHeapType(), (flags & 2) ? Nullable : NonNullable);
          auto kind = op == BinaryConsts::BrOnCast ? BrOnCast : BrOnCastFail;
          return builder.makeBrOn(label, kind, in, cast);
        }
        case BinaryConsts::StructNew:
          return builder.makeStructNew(getIndexedHeapType());
        case BinaryConsts::StructNewDefault:
          return builder.makeStructNewDefault(getIndexedHeapType());
        case BinaryConsts::StructGet:
        case BinaryConsts::StructGetS:
        case BinaryConsts::StructGetU: {
          auto type = getIndexedHeapType();
          auto field = getU32LEB();
          return builder.makeStructGet(
            type, field, op == BinaryConsts::StructGetS);
        }
        case BinaryConsts::StructSet: {
          auto type = getIndexedHeapType();
          auto field = getU32LEB();
          return builder.makeStructSet(type, field);
        }
        case BinaryConsts::ArrayNew:
          return builder.makeArrayNew(getIndexedHeapType());
        case BinaryConsts::ArrayNewDefault:
          return builder.makeArrayNewDefault(getIndexedHeapType());
        case BinaryConsts::ArrayNewFixed: {
          auto type = getIndexedHeapType();
          auto arity = getU32LEB();
          return builder.makeArrayNewFixed(type, arity);
        }
        case BinaryConsts::ArrayNewData: {
          auto type = getIndexedHeapType();
          auto data = getDataName(getU32LEB());
          return builder.makeArrayNewData(type, data);
        }
        case BinaryConsts::ArrayNewElem: {
          auto type = getIndexedHeapType();
          auto elem = getElemName(getU32LEB());
          return builder.makeArrayNewElem(type, elem);
        }
        case BinaryConsts::ArrayGet:
        case BinaryConsts::ArrayGetU:
          return builder.makeArrayGet(getIndexedHeapType(), false);
        case BinaryConsts::ArrayGetS:
          return builder.makeArrayGet(getIndexedHeapType(), true);
        case BinaryConsts::ArraySet:
          return builder.makeArraySet(getIndexedHeapType());
        case BinaryConsts::ArrayLen:
          return builder.makeArrayLen();
        case BinaryConsts::ArrayCopy: {
          auto dest = getIndexedHeapType();
          auto src = getIndexedHeapType();
          return builder.makeArrayCopy(dest, src);
        }
        case BinaryConsts::ArrayFill:
          return builder.makeArrayFill(getIndexedHeapType());
        case BinaryConsts::ArrayInitData: {
          auto type = getIndexedHeapType();
          auto data = getDataName(getU32LEB());
          return builder.makeArrayInitData(type, data);
        }
        case BinaryConsts::ArrayInitElem: {
          auto type = getIndexedHeapType();
          auto elem = getElemName(getU32LEB());
          return builder.makeArrayInitElem(type, elem);
        }
        case BinaryConsts::StringNewLossyUTF8Array:
          return builder.makeStringNew(StringNewLossyUTF8Array);
        case BinaryConsts::StringNewWTF16Array:
          return builder.makeStringNew(StringNewWTF16Array);
        case BinaryConsts::StringFromCodePoint:
          return builder.makeStringNew(StringNewFromCodePoint);
        case BinaryConsts::StringAsWTF16:
          // This turns into nothing because we do not represent stringviews in
          // the IR.
          return Ok{};
        case BinaryConsts::StringConst:
          return builder.makeStringConst(getIndexedString());
        case BinaryConsts::StringMeasureUTF8:
          return builder.makeStringMeasure(StringMeasureUTF8);
        case BinaryConsts::StringMeasureWTF16:
          return builder.makeStringMeasure(StringMeasureWTF16);
        case BinaryConsts::StringEncodeLossyUTF8Array:
          return builder.makeStringEncode(StringEncodeLossyUTF8Array);
        case BinaryConsts::StringEncodeWTF16Array:
          return builder.makeStringEncode(StringEncodeWTF16Array);
        case BinaryConsts::StringConcat:
          return builder.makeStringConcat();
        case BinaryConsts::StringEq:
          return builder.makeStringEq(StringEqEqual);
        case BinaryConsts::StringCompare:
          return builder.makeStringEq(StringEqCompare);
        case BinaryConsts::StringViewWTF16GetCodePoint:
          return builder.makeStringWTF16Get();
        case BinaryConsts::StringViewWTF16Slice:
          return builder.makeStringSliceWTF();
        case BinaryConsts::AnyConvertExtern:
          return builder.makeRefAs(AnyConvertExtern);
        case BinaryConsts::ExternConvertAny:
          return builder.makeRefAs(ExternConvertAny);
      }
      return Err{"unknown GC operation"};
    }
  }
  return Err{"unknown operation"};
}

void WasmBinaryReader::readExports() {
  size_t num = getU32LEB();
  std::unordered_set<Name> names;
  for (size_t i = 0; i < num; i++) {
    auto curr = std::make_unique<Export>();
    curr->name = getInlineString();
    if (!names.emplace(curr->name).second) {
      throwError("duplicate export name");
    }
    curr->kind = (ExternalKind)getU32LEB();
    auto* ex = wasm.addExport(std::move(curr));
    auto index = getU32LEB();
    switch (ex->kind) {
      case ExternalKind::Function:
        ex->value = getFunctionName(index);
        continue;
      case ExternalKind::Table:
        ex->value = getTableName(index);
        continue;
      case ExternalKind::Memory:
        ex->value = getMemoryName(index);
        continue;
      case ExternalKind::Global:
        ex->value = getGlobalName(index);
        continue;
      case ExternalKind::Tag:
        ex->value = getTagName(index);
        continue;
      case ExternalKind::Invalid:
        break;
    }
    throwError("invalid export kind");
  }
}

Expression* WasmBinaryReader::readExpression() {
  assert(builder.empty());
  while (input[pos] != BinaryConsts::End) {
    auto inst = readInst();
    if (auto* err = inst.getErr()) {
      throwError(err->msg);
    }
  }
  ++pos;
  auto expr = builder.build();
  if (auto* err = expr.getErr()) {
    throwError(err->msg);
  }
  return *expr;
}

void WasmBinaryReader::readStrings() {
  auto reserved = getU32LEB();
  if (reserved != 0) {
    throwError("unexpected reserved value in strings");
  }
  size_t num = getU32LEB();
  for (size_t i = 0; i < num; i++) {
    auto string = getInlineString(false);
    // Re-encode from WTF-8 to WTF-16.
    std::stringstream wtf16;
    if (!String::convertWTF8ToWTF16(wtf16, string.str)) {
      throwError("invalid string constant");
    }
    // TODO: Use wtf16.view() once we have C++20.
    strings.push_back(wtf16.str());
  }
}

Name WasmBinaryReader::getIndexedString() {
  auto index = getU32LEB();
  if (index >= strings.size()) {
    throwError("bad string index");
  }
  return strings[index];
}

void WasmBinaryReader::readGlobals() {
  size_t num = getU32LEB();
  auto numImports = wasm.globals.size();
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : globalNames) {
    if (index >= num + numImports) {
      std::cerr << "warning: global index out of bounds in name section: "
                << name << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  for (size_t i = 0; i < num; i++) {
    auto [name, isExplicit] = getOrMakeName(
      globalNames, numImports + i, makeName("global$", i), usedNames);
    auto type = getConcreteType();
    auto mutable_ = getU32LEB();
    if (mutable_ & ~1) {
      throwError("Global mutability must be 0 or 1");
    }
    auto* init = readExpression();
    auto global = Builder::makeGlobal(
      name, type, init, mutable_ ? Builder::Mutable : Builder::Immutable);
    global->hasExplicitName = isExplicit;
    wasm.addGlobal(std::move(global));
  }
}

void WasmBinaryReader::validateBinary() {
  if (hasDataCount && wasm.dataSegments.size() != dataCount) {
    throwError("Number of segments does not agree with DataCount section");
  }

  if (functionTypes.size() != numFuncImports + numFuncBodies) {
    throwError("function and code sections have inconsistent lengths");
  }
}

void WasmBinaryReader::createDataSegments(Index count) {
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : dataNames) {
    if (index >= count) {
      std::cerr << "warning: data index out of bounds in name section: " << name
                << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  for (size_t i = 0; i < count; ++i) {
    auto [name, isExplicit] =
      getOrMakeName(dataNames, i, makeName("", i), usedNames);
    auto curr = Builder::makeDataSegment(name);
    curr->hasExplicitName = isExplicit;
    wasm.addDataSegment(std::move(curr));
  }
}

void WasmBinaryReader::readDataSegmentCount() {
  hasDataCount = true;
  dataCount = getU32LEB();
  // Eagerly create the data segments so they are available during parsing of
  // the code section.
  createDataSegments(dataCount);
}

void WasmBinaryReader::readDataSegments() {
  auto num = getU32LEB();
  if (hasDataCount) {
    if (num != dataCount) {
      throwError("data count and data sections disagree on size");
    }
  } else {
    // We haven't already created the data segments, so create them now.
    createDataSegments(num);
  }
  assert(wasm.dataSegments.size() == num);
  for (size_t i = 0; i < num; i++) {
    auto& curr = wasm.dataSegments[i];
    uint32_t flags = getU32LEB();
    if (flags > 2) {
      throwError("bad segment flags, must be 0, 1, or 2, not " +
                 std::to_string(flags));
    }
    curr->isPassive = flags & BinaryConsts::IsPassive;
    if (curr->isPassive) {
      curr->memory = Name();
      curr->offset = nullptr;
    } else {
      Index memIdx = 0;
      if (flags & BinaryConsts::HasIndex) {
        memIdx = getU32LEB();
      }
      curr->memory = getMemoryName(memIdx);
      curr->offset = readExpression();
    }
    auto size = getU32LEB();
    auto data = getByteView(size);
    curr->data = {data.begin(), data.end()};
  }
}

void WasmBinaryReader::readTableDeclarations() {
  auto num = getU32LEB();
  auto numImports = wasm.tables.size();
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : tableNames) {
    if (index >= num + numImports) {
      std::cerr << "warning: table index out of bounds in name section: "
                << name << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  for (size_t i = 0; i < num; i++) {
    auto [name, isExplicit] =
      getOrMakeName(tableNames, numImports + i, makeName("", i), usedNames);
    auto elemType = getType();
    if (!elemType.isRef()) {
      throwError("Table type must be a reference type");
    }
    auto table = Builder::makeTable(name, elemType);
    table->hasExplicitName = isExplicit;
    bool is_shared;
    getResizableLimits(table->initial,
                       table->max,
                       is_shared,
                       table->addressType,
                       Table::kUnlimitedSize);
    if (is_shared) {
      throwError("Tables may not be shared");
    }
    wasm.addTable(std::move(table));
  }
}

void WasmBinaryReader::readElementSegments() {
  auto num = getU32LEB();
  if (num >= Table::kMaxSize) {
    throwError("Too many segments");
  }
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : elemNames) {
    if (index >= num) {
      std::cerr << "warning: elem index out of bounds in name section: " << name
                << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  for (size_t i = 0; i < num; i++) {
    auto [name, isExplicit] =
      getOrMakeName(elemNames, i, makeName("", i), usedNames);
    auto flags = getU32LEB();
    bool isPassive = (flags & BinaryConsts::IsPassive) != 0;
    bool hasTableIdx = !isPassive && ((flags & BinaryConsts::HasIndex) != 0);
    bool isDeclarative =
      isPassive && ((flags & BinaryConsts::IsDeclarative) != 0);
    bool usesExpressions = (flags & BinaryConsts::UsesExpressions) != 0;

    if (isDeclarative) {
      // Declared segments are needed in wasm text and binary, but not in
      // Binaryen IR; skip over the segment
      [[maybe_unused]] auto type = getU32LEB();
      auto num = getU32LEB();
      for (Index i = 0; i < num; i++) {
        if (usesExpressions) {
          readExpression();
        } else {
          getU32LEB();
        }
      }
      continue;
    }

    auto segment = std::make_unique<ElementSegment>();
    segment->setName(name, isExplicit);

    if (!isPassive) {
      Index tableIdx = 0;
      if (hasTableIdx) {
        tableIdx = getU32LEB();
      }

      if (tableIdx >= wasm.tables.size()) {
        throwError("Table index out of range.");
      }
      auto* table = wasm.tables[tableIdx].get();
      segment->table = table->name;
      segment->offset = readExpression();
    }

    if (isPassive || hasTableIdx) {
      if (usesExpressions) {
        segment->type = getType();
      } else {
        auto elemKind = getU32LEB();
        if (elemKind != 0x0) {
          throwError("Invalid kind (!= funcref(0)) since !usesExpressions.");
        }
      }
    }

    auto& segmentData = segment->data;
    auto size = getU32LEB();
    if (usesExpressions) {
      for (Index j = 0; j < size; j++) {
        segmentData.push_back(readExpression());
      }
    } else {
      for (Index j = 0; j < size; j++) {
        Index index = getU32LEB();
        auto sig = getTypeByFunctionIndex(index);
        auto* refFunc = Builder(wasm).makeRefFunc(getFunctionName(index), sig);
        segmentData.push_back(refFunc);
      }
    }
    wasm.addElementSegment(std::move(segment));
  }
}

void WasmBinaryReader::readTags() {
  size_t num = getU32LEB();
  auto numImports = wasm.tags.size();
  std::unordered_set<Name> usedNames;
  for (auto& [index, name] : tagNames) {
    if (index >= num + numImports) {
      std::cerr << "warning: tag index out of bounds in name section: " << name
                << " at index " << index << '\n';
    }
    usedNames.insert(name);
  }
  for (size_t i = 0; i < num; i++) {
    getInt8(); // Reserved 'attribute' field
    auto [name, isExplicit] =
      getOrMakeName(tagNames, numImports + i, makeName("tag$", i), usedNames);
    auto typeIndex = getU32LEB();
    auto tag = Builder::makeTag(name, getSignatureByTypeIndex(typeIndex));
    tag->hasExplicitName = isExplicit;
    wasm.addTag(std::move(tag));
  }
}

static bool isIdChar(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
         (ch >= 'a' && ch <= 'z') || ch == '!' || ch == '#' || ch == '$' ||
         ch == '%' || ch == '&' || ch == '\'' || ch == '*' || ch == '+' ||
         ch == '-' || ch == '.' || ch == '/' || ch == ':' || ch == '<' ||
         ch == '=' || ch == '>' || ch == '?' || ch == '@' || ch == '^' ||
         ch == '_' || ch == '`' || ch == '|' || ch == '~';
}

static char formatNibble(int nibble) {
  return nibble < 10 ? '0' + nibble : 'a' - 10 + nibble;
}

Name WasmBinaryReader::escape(Name name) {
  bool allIdChars = true;
  for (char c : name.str) {
    if (!(allIdChars = isIdChar(c))) {
      break;
    }
  }
  if (allIdChars) {
    return name;
  }
  // encode name, if at least one non-idchar (per WebAssembly spec) was found
  std::string escaped;
  for (char c : name.str) {
    if (isIdChar(c)) {
      escaped.push_back(c);
      continue;
    }
    // replace non-idchar with `\xx` escape
    escaped.push_back('\\');
    escaped.push_back(formatNibble(c >> 4));
    escaped.push_back(formatNibble(c & 15));
  }
  return escaped;
}

namespace {

// Performs necessary processing of names from the name section before using
// them. Specifically it escapes and deduplicates them.
class NameProcessor {
public:
  // Returns a unique, escaped name. Notes that name for the items to follow to
  // keep them unique as well.
  Name process(Name name) {
    return deduplicate(WasmBinaryReader::escape(name));
  }

private:
  std::unordered_set<Name> usedNames;

  Name deduplicate(Name base) {
    auto name = Names::getValidNameGivenExisting(base, usedNames);
    usedNames.insert(name);
    return name;
  }
};

} // anonymous namespace

void WasmBinaryReader::findAndReadNames() {
  // Find the names section. Skip the magic and version.
  assert(pos == 0);
  getInt32();
  getInt32();
  Index payloadLen, sectionPos;
  bool found = false;
  while (more()) {
    uint8_t sectionCode = getInt8();
    payloadLen = getU32LEB();
    sectionPos = pos;
    if (sectionCode == BinaryConsts::Section::Custom) {
      auto sectionName = getInlineString();
      if (sectionName.equals(BinaryConsts::CustomSections::Name)) {
        found = true;
        break;
      }
    }
    pos = sectionPos + payloadLen;
  }
  if (!found) {
    // No names section to read.
    pos = 0;
    return;
  }

  // Read the names.
  uint32_t lastType = 0;
  while (pos < sectionPos + payloadLen) {
    auto nameType = getU32LEB();
    if (lastType && nameType <= lastType) {
      std::cerr << "warning: out-of-order name subsection: " << nameType
                << std::endl;
    }
    lastType = nameType;
    auto subsectionSize = getU32LEB();
    auto subsectionPos = pos;
    using Subsection = BinaryConsts::CustomSections::Subsection;
    if (nameType == Subsection::NameModule) {
      wasm.name = getInlineString();
    } else if (nameType == Subsection::NameFunction) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        functionNames[index] = name;
      }
    } else if (nameType == Subsection::NameLocal) {
      auto numFuncs = getU32LEB();
      for (size_t i = 0; i < numFuncs; i++) {
        auto funcIndex = getU32LEB();
        auto numLocals = getU32LEB();
        NameProcessor processor;
        for (size_t j = 0; j < numLocals; j++) {
          auto localIndex = getU32LEB();
          auto rawName = getInlineString();
          auto name = processor.process(rawName);
          localNames[funcIndex][localIndex] = name;
        }
      }
    } else if (nameType == Subsection::NameType) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        typeNames[index] = name;
      }
    } else if (nameType == Subsection::NameTable) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        tableNames[index] = name;
      }
    } else if (nameType == Subsection::NameElem) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        elemNames[index] = name;
      }
    } else if (nameType == Subsection::NameMemory) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        memoryNames[index] = name;
      }
    } else if (nameType == Subsection::NameData) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        dataNames[index] = name;
      }
    } else if (nameType == Subsection::NameGlobal) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        globalNames[index] = name;
      }
    } else if (nameType == Subsection::NameField) {
      auto numTypes = getU32LEB();
      for (size_t i = 0; i < numTypes; i++) {
        auto typeIndex = getU32LEB();
        auto numFields = getU32LEB();
        NameProcessor processor;
        for (size_t i = 0; i < numFields; i++) {
          auto fieldIndex = getU32LEB();
          auto rawName = getInlineString();
          auto name = processor.process(rawName);
          fieldNames[typeIndex][fieldIndex] = name;
        }
      }
    } else if (nameType == Subsection::NameTag) {
      auto num = getU32LEB();
      NameProcessor processor;
      for (size_t i = 0; i < num; i++) {
        auto index = getU32LEB();
        auto rawName = getInlineString();
        auto name = processor.process(rawName);
        tagNames[index] = name;
      }
    } else {
      std::cerr << "warning: unknown name subsection with id "
                << std::to_string(nameType) << " at " << pos << std::endl;
      pos = subsectionPos + subsectionSize;
    }
    if (pos != subsectionPos + subsectionSize) {
      throwError("bad names subsection position change");
    }
  }
  if (pos != sectionPos + payloadLen) {
    throwError("bad names section position change");
  }

  // Reset the position; we were just reading ahead.
  pos = 0;
}

void WasmBinaryReader::readFeatures(size_t payloadLen) {
  wasm.hasFeaturesSection = true;

  auto sectionPos = pos;
  size_t numFeatures = getU32LEB();
  for (size_t i = 0; i < numFeatures; ++i) {
    uint8_t prefix = getInt8();

    bool disallowed = prefix == BinaryConsts::FeatureDisallowed;
    bool used = prefix == BinaryConsts::FeatureUsed;

    if (!disallowed && !used) {
      throwError("Unrecognized feature policy prefix");
    }

    Name name = getInlineString();
    if (pos > sectionPos + payloadLen) {
      throwError("ill-formed string extends beyond section");
    }

    FeatureSet feature;
    if (name == BinaryConsts::CustomSections::AtomicsFeature) {
      feature = FeatureSet::Atomics;
    } else if (name == BinaryConsts::CustomSections::BulkMemoryFeature) {
      feature = FeatureSet::BulkMemory;
      if (used) {
        // For backward compatibility, enable this dependent feature.
        feature |= FeatureSet::BulkMemoryOpt;
      }
    } else if (name == BinaryConsts::CustomSections::BulkMemoryOptFeature) {
      feature = FeatureSet::BulkMemoryOpt;
    } else if (name == BinaryConsts::CustomSections::ExceptionHandlingFeature) {
      feature = FeatureSet::ExceptionHandling;
    } else if (name == BinaryConsts::CustomSections::MutableGlobalsFeature) {
      feature = FeatureSet::MutableGlobals;
    } else if (name == BinaryConsts::CustomSections::TruncSatFeature) {
      feature = FeatureSet::TruncSat;
    } else if (name == BinaryConsts::CustomSections::SignExtFeature) {
      feature = FeatureSet::SignExt;
    } else if (name == BinaryConsts::CustomSections::SIMD128Feature) {
      feature = FeatureSet::SIMD;
    } else if (name == BinaryConsts::CustomSections::TailCallFeature) {
      feature = FeatureSet::TailCall;
    } else if (name == BinaryConsts::CustomSections::ReferenceTypesFeature) {
      feature = FeatureSet::ReferenceTypes;
    } else if (name == BinaryConsts::CustomSections::MultivalueFeature) {
      feature = FeatureSet::Multivalue;
    } else if (name == BinaryConsts::CustomSections::GCFeature) {
      feature = FeatureSet::GC;
    } else if (name == BinaryConsts::CustomSections::Memory64Feature) {
      feature = FeatureSet::Memory64;
    } else if (name == BinaryConsts::CustomSections::RelaxedSIMDFeature) {
      feature = FeatureSet::RelaxedSIMD;
    } else if (name == BinaryConsts::CustomSections::ExtendedConstFeature) {
      feature = FeatureSet::ExtendedConst;
    } else if (name == BinaryConsts::CustomSections::StringsFeature) {
      feature = FeatureSet::Strings;
    } else if (name == BinaryConsts::CustomSections::MultiMemoryFeature) {
      feature = FeatureSet::MultiMemory;
    } else if (name == BinaryConsts::CustomSections::StackSwitchingFeature) {
      feature = FeatureSet::StackSwitching;
    } else if (name == BinaryConsts::CustomSections::SharedEverythingFeature) {
      feature = FeatureSet::SharedEverything;
    } else if (name == BinaryConsts::CustomSections::FP16Feature) {
      feature = FeatureSet::FP16;
    } else {
      // Silently ignore unknown features (this may be and old binaryen running
      // on a new wasm).
    }

    if (disallowed && wasm.features.has(feature)) {
      std::cerr
        << "warning: feature " << feature.toString()
        << " was enabled by the user, but disallowed in the features section.";
    }
    if (used) {
      wasm.features.enable(feature);
    }
  }
  if (pos != sectionPos + payloadLen) {
    throwError("bad features section size");
  }
}

void WasmBinaryReader::readDylink(size_t payloadLen) {
  wasm.dylinkSection = std::make_unique<DylinkSection>();

  auto sectionPos = pos;

  wasm.dylinkSection->isLegacy = true;
  wasm.dylinkSection->memorySize = getU32LEB();
  wasm.dylinkSection->memoryAlignment = getU32LEB();
  wasm.dylinkSection->tableSize = getU32LEB();
  wasm.dylinkSection->tableAlignment = getU32LEB();

  size_t numNeededDynlibs = getU32LEB();
  for (size_t i = 0; i < numNeededDynlibs; ++i) {
    wasm.dylinkSection->neededDynlibs.push_back(getInlineString());
  }

  if (pos != sectionPos + payloadLen) {
    throwError("bad dylink section size");
  }
}

void WasmBinaryReader::readDylink0(size_t payloadLen) {
  auto sectionPos = pos;
  uint32_t lastType = 0;

  wasm.dylinkSection = std::make_unique<DylinkSection>();
  while (pos < sectionPos + payloadLen) {
    auto oldPos = pos;
    auto dylinkType = getU32LEB();
    if (lastType && dylinkType <= lastType) {
      std::cerr << "warning: out-of-order dylink.0 subsection: " << dylinkType
                << std::endl;
    }
    lastType = dylinkType;
    auto subsectionSize = getU32LEB();
    auto subsectionPos = pos;
    if (dylinkType == BinaryConsts::CustomSections::Subsection::DylinkMemInfo) {
      wasm.dylinkSection->memorySize = getU32LEB();
      wasm.dylinkSection->memoryAlignment = getU32LEB();
      wasm.dylinkSection->tableSize = getU32LEB();
      wasm.dylinkSection->tableAlignment = getU32LEB();
    } else if (dylinkType ==
               BinaryConsts::CustomSections::Subsection::DylinkNeeded) {
      size_t numNeededDynlibs = getU32LEB();
      for (size_t i = 0; i < numNeededDynlibs; ++i) {
        wasm.dylinkSection->neededDynlibs.push_back(getInlineString());
      }
    } else {
      // Unknown subsection.  Stop parsing now and store the rest of
      // the section verbatim.
      pos = oldPos;
      size_t remaining = (sectionPos + payloadLen) - pos;
      auto tail = getByteView(remaining);
      wasm.dylinkSection->tail = {tail.begin(), tail.end()};
      break;
    }
    if (pos != subsectionPos + subsectionSize) {
      throwError("bad dylink.0 subsection position change");
    }
  }
}

Index WasmBinaryReader::readMemoryAccess(Address& alignment, Address& offset) {
  auto rawAlignment = getU32LEB();
  bool hasMemIdx = false;
  Index memIdx = 0;
  // Check bit 6 in the alignment to know whether a memory index is present per:
  // https://github.com/WebAssembly/multi-memory/blob/main/proposals/multi-memory/Overview.md
  if (rawAlignment & (1 << (6))) {
    hasMemIdx = true;
    // Clear the bit before we parse alignment
    rawAlignment = rawAlignment & ~(1 << 6);
  }

  if (rawAlignment > 8) {
    throwError("Alignment must be of a reasonable size");
  }

  alignment = Bits::pow2(rawAlignment);
  if (hasMemIdx) {
    memIdx = getU32LEB();
  }
  if (memIdx >= wasm.memories.size()) {
    throwError("Memory index out of range while reading memory alignment.");
  }
  auto* memory = wasm.memories[memIdx].get();
  offset = memory->addressType == Type::i32 ? getU32LEB() : getU64LEB();

  return memIdx;
}

// TODO: make this the only version
std::tuple<Name, Address, Address> WasmBinaryReader::getMemarg() {
  Address alignment, offset;
  auto memIdx = readMemoryAccess(alignment, offset);
  return {getMemoryName(memIdx), alignment, offset};
}

} // namespace wasm
