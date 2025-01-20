/*
 * Copyright 2022 WebAssembly Community Group participants
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

#include "module-utils.h"
#include "ir/debuginfo.h"
#include "ir/intrinsics.h"
#include "ir/manipulation.h"
#include "ir/properties.h"
#include "support/insert_ordered.h"
#include "support/topological_sort.h"

namespace wasm::ModuleUtils {

// Update the file name indices when moving a set of debug locations from one
// module to another.
static void updateLocation(std::optional<Function::DebugLocation>& location,
                           std::vector<Index>& fileIndexMap) {
  if (location) {
    location->fileIndex = fileIndexMap[location->fileIndex];
  }
}

// Update the symbol name indices when moving a set of debug locations from one
// module to another.
static void updateSymbol(std::optional<Function::DebugLocation>& location,
                         std::vector<Index>& symbolIndexMap) {
  if (location && location->symbolNameIndex) {
    location->symbolNameIndex = symbolIndexMap[*location->symbolNameIndex];
  }
}

// Copies a function into a module. If newName is provided it is used as the
// name of the function (otherwise the original name is copied). If fileIndexMap
// is specified, it is used to rename source map filename indices when copying
// the function from one module to another one. If symbolNameIndexMap is
// specified, it is used to rename source map symbol name indices when copying
// the function from one module to another one.
Function* copyFunction(Function* func,
                       Module& out,
                       Name newName,
                       std::optional<std::vector<Index>> fileIndexMap,
                       std::optional<std::vector<Index>> symbolNameIndexMap) {
  auto ret = copyFunctionWithoutAdd(
    func, out, newName, fileIndexMap, symbolNameIndexMap);
  return out.addFunction(std::move(ret));
}

std::unique_ptr<Function>
copyFunctionWithoutAdd(Function* func,
                       Module& out,
                       Name newName,
                       std::optional<std::vector<Index>> fileIndexMap,
                       std::optional<std::vector<Index>> symbolNameIndexMap) {
  auto ret = std::make_unique<Function>();
  ret->name = newName.is() ? newName : func->name;
  ret->hasExplicitName = func->hasExplicitName;
  ret->type = func->type;
  ret->vars = func->vars;
  ret->localNames = func->localNames;
  ret->localIndices = func->localIndices;
  ret->body = ExpressionManipulator::copy(func->body, out);
  debuginfo::copyBetweenFunctions(func->body, ret->body, func, ret.get());
  ret->prologLocation = func->prologLocation;
  ret->epilogLocation = func->epilogLocation;
  // Update file indices if needed
  if (fileIndexMap) {
    for (auto& iter : ret->debugLocations) {
      if (iter.second) {
        iter.second->fileIndex = (*fileIndexMap)[iter.second->fileIndex];
      }
    }
    updateLocation(ret->prologLocation, *fileIndexMap);
    updateLocation(ret->epilogLocation, *fileIndexMap);
  }
  if (symbolNameIndexMap) {
    for (auto& iter : ret->debugLocations) {
      if (iter.second) {
        if (iter.second->symbolNameIndex.has_value()) {
          iter.second->symbolNameIndex =
            (*symbolNameIndexMap)[*(iter.second->symbolNameIndex)];
        }
      }
      updateSymbol(ret->prologLocation, *symbolNameIndexMap);
      updateSymbol(ret->epilogLocation, *symbolNameIndexMap);
    }
  }
  ret->module = func->module;
  ret->base = func->base;
  ret->noFullInline = func->noFullInline;
  ret->noPartialInline = func->noPartialInline;
  return ret;
}

Global* copyGlobal(Global* global, Module& out) {
  auto* ret = new Global();
  ret->name = global->name;
  ret->hasExplicitName = global->hasExplicitName;
  ret->type = global->type;
  ret->mutable_ = global->mutable_;
  ret->module = global->module;
  ret->base = global->base;
  if (global->imported()) {
    ret->init = nullptr;
  } else {
    ret->init = ExpressionManipulator::copy(global->init, out);
  }
  out.addGlobal(ret);
  return ret;
}

Tag* copyTag(Tag* tag, Module& out) {
  auto* ret = new Tag();
  ret->name = tag->name;
  ret->hasExplicitName = tag->hasExplicitName;
  ret->type = tag->type;
  ret->module = tag->module;
  ret->base = tag->base;
  out.addTag(ret);
  return ret;
}

ElementSegment* copyElementSegment(const ElementSegment* segment, Module& out) {
  auto copy = [&](std::unique_ptr<ElementSegment>&& ret) {
    ret->name = segment->name;
    ret->hasExplicitName = segment->hasExplicitName;
    ret->type = segment->type;
    ret->data.reserve(segment->data.size());
    for (auto* item : segment->data) {
      ret->data.push_back(ExpressionManipulator::copy(item, out));
    }

    return out.addElementSegment(std::move(ret));
  };

  if (segment->table.isNull()) {
    return copy(std::make_unique<ElementSegment>());
  } else {
    auto offset = ExpressionManipulator::copy(segment->offset, out);
    return copy(std::make_unique<ElementSegment>(segment->table, offset));
  }
}

Table* copyTable(const Table* table, Module& out) {
  auto ret = std::make_unique<Table>();
  ret->name = table->name;
  ret->hasExplicitName = table->hasExplicitName;
  ret->type = table->type;
  ret->module = table->module;
  ret->base = table->base;

  ret->initial = table->initial;
  ret->max = table->max;
  ret->addressType = table->addressType;

  return out.addTable(std::move(ret));
}

Memory* copyMemory(const Memory* memory, Module& out) {
  auto ret = Builder::makeMemory(memory->name);
  ret->hasExplicitName = memory->hasExplicitName;
  ret->initial = memory->initial;
  ret->max = memory->max;
  ret->shared = memory->shared;
  ret->addressType = memory->addressType;
  ret->module = memory->module;
  ret->base = memory->base;

  return out.addMemory(std::move(ret));
}

DataSegment* copyDataSegment(const DataSegment* segment, Module& out) {
  auto ret = Builder::makeDataSegment();
  ret->name = segment->name;
  ret->hasExplicitName = segment->hasExplicitName;
  ret->memory = segment->memory;
  ret->isPassive = segment->isPassive;
  if (!segment->isPassive) {
    auto offset = ExpressionManipulator::copy(segment->offset, out);
    ret->offset = offset;
  }
  ret->data = segment->data;

  return out.addDataSegment(std::move(ret));
}

// Copies named toplevel module items (things of kind ModuleItemKind). See
// copyModule() for something that also copies exports, the start function, etc.
void copyModuleItems(const Module& in, Module& out) {
  // If the source module has some debug information, we first compute how
  // to map file name indices from this modules to file name indices in
  // the target module.
  std::optional<std::vector<Index>> fileIndexMap;
  if (!in.debugInfoFileNames.empty()) {
    std::unordered_map<std::string, Index> debugInfoFileIndices;
    for (Index i = 0; i < out.debugInfoFileNames.size(); i++) {
      debugInfoFileIndices[out.debugInfoFileNames[i]] = i;
    }
    fileIndexMap.emplace();
    for (Index i = 0; i < in.debugInfoFileNames.size(); i++) {
      std::string file = in.debugInfoFileNames[i];
      auto iter = debugInfoFileIndices.find(file);
      if (iter == debugInfoFileIndices.end()) {
        Index index = out.debugInfoFileNames.size();
        out.debugInfoFileNames.push_back(file);
        debugInfoFileIndices[file] = index;
      }
      fileIndexMap->push_back(debugInfoFileIndices[file]);
    }
  }

  std::optional<std::vector<Index>> symbolNameIndexMap;
  if (!in.debugInfoSymbolNames.empty()) {
    std::unordered_map<std::string, Index> debugInfoSymbolNameIndices;
    for (Index i = 0; i < out.debugInfoSymbolNames.size(); i++) {
      debugInfoSymbolNameIndices[out.debugInfoSymbolNames[i]] = i;
    }
    symbolNameIndexMap.emplace();
    for (Index i = 0; i < in.debugInfoSymbolNames.size(); i++) {
      std::string file = in.debugInfoSymbolNames[i];
      auto iter = debugInfoSymbolNameIndices.find(file);
      if (iter == debugInfoSymbolNameIndices.end()) {
        Index index = out.debugInfoSymbolNames.size();
        out.debugInfoSymbolNames.push_back(file);
        debugInfoSymbolNameIndices[file] = index;
      }
      symbolNameIndexMap->push_back(debugInfoSymbolNameIndices[file]);
    }
  }

  for (auto& curr : in.functions) {
    copyFunction(curr.get(), out, Name(), fileIndexMap, symbolNameIndexMap);
  }
  for (auto& curr : in.globals) {
    copyGlobal(curr.get(), out);
  }
  for (auto& curr : in.tags) {
    copyTag(curr.get(), out);
  }
  for (auto& curr : in.elementSegments) {
    copyElementSegment(curr.get(), out);
  }
  for (auto& curr : in.tables) {
    copyTable(curr.get(), out);
  }
  for (auto& curr : in.memories) {
    copyMemory(curr.get(), out);
  }
  for (auto& curr : in.dataSegments) {
    copyDataSegment(curr.get(), out);
  }

  for (auto& [type, names] : in.typeNames) {
    if (!out.typeNames.count(type)) {
      out.typeNames[type] = names;
    }
  }
}

// TODO: merge this with copyModuleItems, and add options for copying
// exports and other things that are currently different between them,
// if we still need those differences.
void copyModule(const Module& in, Module& out) {
  // we use names throughout, not raw pointers, so simple copying is fine
  // for everything *but* expressions
  for (auto& curr : in.exports) {
    out.addExport(std::make_unique<Export>(*curr));
  }
  copyModuleItems(in, out);
  out.start = in.start;
  out.customSections = in.customSections;
  out.debugInfoFileNames = in.debugInfoFileNames;
  out.debugInfoSymbolNames = in.debugInfoSymbolNames;
  out.features = in.features;
}

void clearModule(Module& wasm) {
  wasm.~Module();
  new (&wasm) Module;
}

// Renaming

// Rename functions along with all their uses.
// Note that for this to work the functions themselves don't necessarily need
// to exist.  For example, it is possible to remove a given function and then
// call this to redirect all of its uses.
template<typename T> void renameFunctions(Module& wasm, T& map) {
  // Update the function itself.
  for (auto& [oldName, newName] : map) {
    if (Function* func = wasm.getFunctionOrNull(oldName)) {
      assert(!wasm.getFunctionOrNull(newName) || func->name == newName);
      func->name = newName;
    }
  }
  wasm.updateMaps();

  // Update all references to it.
  struct Updater : public WalkerPass<PostWalker<Updater>> {
    bool isFunctionParallel() override { return true; }

    T& map;

    void maybeUpdate(Name& name) {
      if (auto iter = map.find(name); iter != map.end()) {
        name = iter->second;
      }
    }

    Updater(T& map) : map(map) {}

    std::unique_ptr<Pass> create() override {
      return std::make_unique<Updater>(map);
    }

    void visitCall(Call* curr) { maybeUpdate(curr->target); }

    void visitRefFunc(RefFunc* curr) { maybeUpdate(curr->func); }
  };

  Updater updater(map);
  updater.maybeUpdate(wasm.start);
  PassRunner runner(&wasm);
  updater.run(&runner, &wasm);
  updater.runOnModuleCode(&runner, &wasm);
}

void renameFunction(Module& wasm, Name oldName, Name newName) {
  std::map<Name, Name> map;
  map[oldName] = newName;
  renameFunctions(wasm, map);
}

namespace {

// Helper for collecting HeapTypes and their frequencies.
struct TypeInfos {
  InsertOrderedMap<HeapType, HeapTypeInfo> info;

  // Multivalue control flow structures need a function type, but the identity
  // of the function type (i.e. what recursion group it is in or whether it is
  // final) doesn't matter. Save them for the end to see if we can re-use an
  // existing function type with the necessary signature.
  InsertOrderedMap<Signature, size_t> controlFlowSignatures;

  void note(HeapType type) {
    if (!type.isBasic()) {
      ++info[type].useCount;
    }
  }
  void note(Type type) {
    for (HeapType ht : type.getHeapTypeChildren()) {
      note(ht);
    }
  }
  // Ensure a type is included without increasing its count.
  void include(HeapType type) {
    if (!type.isBasic()) {
      info[type];
    }
  }
  void include(Type type) {
    for (HeapType ht : type.getHeapTypeChildren()) {
      include(ht);
    }
  }
  void noteControlFlow(Signature sig) {
    // TODO: support control flow input parameters.
    assert(sig.params.size() == 0);
    if (sig.results.isTuple()) {
      // We have to use a function type.
      ++controlFlowSignatures[sig];
    } else if (sig.results != Type::none) {
      // The result type can be emitted directly instead of using a function
      // type.
      note(sig.results);
    }
  }
  bool contains(HeapType type) { return info.count(type); }
};

struct CodeScanner
  : PostWalker<CodeScanner, UnifiedExpressionVisitor<CodeScanner>> {
  TypeInfos& info;
  TypeInclusion inclusion;

  CodeScanner(Module& wasm, TypeInfos& info, TypeInclusion inclusion)
    : info(info), inclusion(inclusion) {
    setModule(&wasm);
  }

  void visitExpression(Expression* curr) {
    if (auto* call = curr->dynCast<CallIndirect>()) {
      info.note(call->heapType);
    } else if (auto* call = curr->dynCast<CallRef>()) {
      info.note(call->target->type);
    } else if (curr->is<RefNull>()) {
      info.note(curr->type);
    } else if (curr->is<Select>() && curr->type.isRef()) {
      // This select will be annotated in the binary, so note it.
      info.note(curr->type);
    } else if (curr->is<StructNew>()) {
      info.note(curr->type);
    } else if (curr->is<ArrayNew>()) {
      info.note(curr->type);
    } else if (curr->is<ArrayNewData>()) {
      info.note(curr->type);
    } else if (curr->is<ArrayNewElem>()) {
      info.note(curr->type);
    } else if (curr->is<ArrayNewFixed>()) {
      info.note(curr->type);
    } else if (auto* copy = curr->dynCast<ArrayCopy>()) {
      info.note(copy->destRef->type);
      info.note(copy->srcRef->type);
    } else if (auto* fill = curr->dynCast<ArrayFill>()) {
      info.note(fill->ref->type);
    } else if (auto* init = curr->dynCast<ArrayInitData>()) {
      info.note(init->ref->type);
    } else if (auto* init = curr->dynCast<ArrayInitElem>()) {
      info.note(init->ref->type);
    } else if (auto* cast = curr->dynCast<RefCast>()) {
      info.note(cast->type);
    } else if (auto* cast = curr->dynCast<RefTest>()) {
      info.note(cast->castType);
    } else if (auto* cast = curr->dynCast<BrOn>()) {
      if (cast->op == BrOnCast || cast->op == BrOnCastFail) {
        info.note(cast->ref->type);
        info.note(cast->castType);
      }
    } else if (auto* get = curr->dynCast<StructGet>()) {
      info.note(get->ref->type);
    } else if (auto* set = curr->dynCast<StructSet>()) {
      info.note(set->ref->type);
    } else if (auto* get = curr->dynCast<ArrayGet>()) {
      info.note(get->ref->type);
    } else if (auto* set = curr->dynCast<ArraySet>()) {
      info.note(set->ref->type);
    } else if (auto* contBind = curr->dynCast<ContBind>()) {
      info.note(contBind->contTypeBefore);
      info.note(contBind->contTypeAfter);
    } else if (auto* contNew = curr->dynCast<ContNew>()) {
      info.note(contNew->contType);
    } else if (auto* resume = curr->dynCast<Resume>()) {
      info.note(resume->contType);
    } else if (Properties::isControlFlowStructure(curr)) {
      info.noteControlFlow(Signature(Type::none, curr->type));
    }
  }
};

void classifyTypeVisibility(Module& wasm,
                            InsertOrderedMap<HeapType, HeapTypeInfo>& types);

} // anonymous namespace

InsertOrderedMap<HeapType, HeapTypeInfo> collectHeapTypeInfo(
  Module& wasm, TypeInclusion inclusion, VisibilityHandling visibility) {
  // Collect module-level info.
  TypeInfos info;
  CodeScanner(wasm, info, inclusion).walkModuleCode(&wasm);
  for (auto& curr : wasm.globals) {
    info.note(curr->type);
  }
  for (auto& curr : wasm.tags) {
    info.note(curr->type);
  }
  for (auto& curr : wasm.tables) {
    info.note(curr->type);
  }
  for (auto& curr : wasm.elementSegments) {
    info.note(curr->type);
  }

  // Collect info from functions in parallel.
  ModuleUtils::ParallelFunctionAnalysis<TypeInfos, Immutable, InsertOrderedMap>
    analysis(wasm, [&](Function* func, TypeInfos& info) {
      info.note(func->type);
      for (auto type : func->vars) {
        info.note(type);
      }
      // Don't just use `func->imported()` here because we also might be
      // printing an error message on a partially parsed module whose declared
      // function bodies have not all been parsed yet.
      if (func->body) {
        CodeScanner(wasm, info, inclusion).walk(func->body);
      }
    });

  // Combine the function info with the module info.
  for (auto& [_, functionInfo] : analysis.map) {
    for (auto& [type, typeInfo] : functionInfo.info) {
      info.info[type].useCount += typeInfo.useCount;
    }
    for (auto& [sig, count] : functionInfo.controlFlowSignatures) {
      info.controlFlowSignatures[sig] += count;
    }
  }

  // Recursively traverse each reference type, which may have a child type that
  // is itself a reference type. This reflects an appearance in the binary
  // format that is in the type section itself. As we do this we may find more
  // and more types, as nested children of previous ones. Each such type will
  // appear in the type section once, so we just need to visit it once. Also
  // track which recursion groups we've already processed to avoid quadratic
  // behavior when there is a single large group.
  // TODO: Use a vector here, since we never try to add the same type twice.
  UniqueNonrepeatingDeferredQueue<HeapType> newTypes;
  std::unordered_map<Signature, HeapType> seenSigs;
  auto noteNewType = [&](HeapType type) {
    newTypes.push(type);
    if (type.isSignature()) {
      seenSigs.insert({type.getSignature(), type});
    }
  };
  for (auto& [type, _] : info.info) {
    noteNewType(type);
  }
  auto controlFlowIt = info.controlFlowSignatures.begin();
  std::unordered_set<RecGroup> includedGroups;
  while (!newTypes.empty()) {
    while (!newTypes.empty()) {
      auto ht = newTypes.pop();
      for (HeapType child : ht.getReferencedHeapTypes()) {
        if (!child.isBasic()) {
          if (!info.contains(child)) {
            noteNewType(child);
          }
          info.note(child);
        }
      }

      // Make sure we've noted the complete recursion group of each type as
      // well.
      if (inclusion != TypeInclusion::UsedIRTypes) {
        auto recGroup = ht.getRecGroup();
        if (includedGroups.insert(recGroup).second) {
          for (auto type : recGroup) {
            if (!info.contains(type)) {
              noteNewType(type);
              info.include(type);
            }
          }
        }
      }
    }

    // We've found all the types there are to find without considering more
    // control flow types. Consider one more control flow type and repeat.
    for (; controlFlowIt != info.controlFlowSignatures.end(); ++controlFlowIt) {
      auto& [sig, count] = *controlFlowIt;
      if (auto it = seenSigs.find(sig); it != seenSigs.end()) {
        info.info[it->second].useCount += count;
      } else {
        // We've never seen this signature before, so add a type for it.
        HeapType type(sig);
        noteNewType(type);
        info.info[type].useCount += count;
        break;
      }
    }
  }

  if (visibility == VisibilityHandling::FindVisibility) {
    classifyTypeVisibility(wasm, info.info);
  }

  return std::move(info.info);
}

namespace {

void classifyTypeVisibility(Module& wasm,
                            InsertOrderedMap<HeapType, HeapTypeInfo>& types) {
  // We will need to traverse the types used by public types and mark them
  // public as well.
  std::vector<HeapType> workList;
  std::unordered_set<RecGroup> publicGroups;

  auto notePublic = [&](HeapType type) {
    if (type.isBasic()) {
      return;
    }
    auto group = type.getRecGroup();
    if (!publicGroups.insert(group).second) {
      // The groups in this type have already been marked public.
      return;
    }
    for (auto member : type.getRecGroup()) {
      if (auto it = types.find(member); it != types.end()) {
        it->second.visibility = Visibility::Public;
      }
      workList.push_back(member);
    }
  };

  ModuleUtils::iterImportedTags(wasm, [&](Tag* tag) { notePublic(tag->type); });
  ModuleUtils::iterImportedTables(wasm, [&](Table* table) {
    assert(table->type.isRef());
    notePublic(table->type.getHeapType());
  });
  ModuleUtils::iterImportedGlobals(wasm, [&](Global* global) {
    if (global->type.isRef()) {
      notePublic(global->type.getHeapType());
    }
  });
  ModuleUtils::iterImportedFunctions(wasm, [&](Function* func) {
    // We can ignore call.without.effects, which is implemented as an import but
    // functionally is a call within the module.
    if (!Intrinsics(wasm).isCallWithoutEffects(func)) {
      notePublic(func->type);
    }
  });
  for (auto& ex : wasm.exports) {
    switch (ex->kind) {
      case ExternalKind::Function: {
        auto* func = wasm.getFunction(ex->value);
        notePublic(func->type);
        continue;
      }
      case ExternalKind::Table: {
        auto* table = wasm.getTable(ex->value);
        assert(table->type.isRef());
        notePublic(table->type.getHeapType());
        continue;
      }
      case ExternalKind::Memory:
        // Never a reference type.
        continue;
      case ExternalKind::Global: {
        auto* global = wasm.getGlobal(ex->value);
        if (global->type.isRef()) {
          notePublic(global->type.getHeapType());
        }
        continue;
      }
      case ExternalKind::Tag:
        notePublic(wasm.getTag(ex->value)->type);
        continue;
      case ExternalKind::Invalid:
        break;
    }
    WASM_UNREACHABLE("unexpected export kind");
  }

  // Ignorable public types are public.
  for (auto type : getIgnorablePublicTypes()) {
    notePublic(type);
  }

  // Find all the other public types reachable from directly publicized types.
  while (!workList.empty()) {
    auto curr = workList.back();
    workList.pop_back();
    for (auto t : curr.getReferencedHeapTypes()) {
      notePublic(t);
    }
  }

  for (auto& [_, info] : types) {
    if (info.visibility != Visibility::Public) {
      info.visibility = Visibility::Private;
    }
  }

  // TODO: In an open world, we need to consider subtypes of public types public
  // as well, or potentially even consider all types to be public unless
  // otherwise annotated.
}

void setIndices(IndexedHeapTypes& indexedTypes) {
  for (Index i = 0; i < indexedTypes.types.size(); i++) {
    indexedTypes.indices[indexedTypes.types[i]] = i;
  }
}

} // anonymous namespace

std::vector<HeapType> collectHeapTypes(Module& wasm) {
  auto info = collectHeapTypeInfo(wasm);
  std::vector<HeapType> types;
  types.reserve(info.size());
  for (auto& [type, _] : info) {
    types.push_back(type);
  }
  return types;
}

std::vector<HeapType> getPublicHeapTypes(Module& wasm) {
  auto info = collectHeapTypeInfo(
    wasm, TypeInclusion::BinaryTypes, VisibilityHandling::FindVisibility);
  std::vector<HeapType> types;
  types.reserve(info.size());
  for (auto& [type, typeInfo] : info) {
    if (typeInfo.visibility == Visibility::Public) {
      types.push_back(type);
    }
  }
  return types;
}

std::vector<HeapType> getPrivateHeapTypes(Module& wasm) {
  auto info = collectHeapTypeInfo(
    wasm, TypeInclusion::UsedIRTypes, VisibilityHandling::FindVisibility);
  std::vector<HeapType> types;
  types.reserve(info.size());
  for (auto& [type, typeInfo] : info) {
    if (typeInfo.visibility == Visibility::Private) {
      types.push_back(type);
    }
  }
  return types;
}

IndexedHeapTypes getOptimizedIndexedHeapTypes(Module& wasm) {
  auto counts = collectHeapTypeInfo(wasm, TypeInclusion::BinaryTypes);

  // Collect the rec groups.
  std::unordered_map<RecGroup, size_t> groupIndices;
  std::vector<RecGroup> groups;
  for (auto& [type, _] : counts) {
    auto group = type.getRecGroup();
    if (groupIndices.insert({group, groups.size()}).second) {
      groups.push_back(group);
    }
  }

  // Collect the total use counts for each group.
  std::vector<size_t> groupCounts;
  groupCounts.reserve(groups.size());
  for (auto group : groups) {
    size_t count = 0;
    for (auto type : group) {
      count += counts.at(type).useCount;
    }
    groupCounts.push_back(count);
  }

  // Collect the reverse dependencies of each group.
  std::vector<std::unordered_set<size_t>> depSets(groups.size());
  for (size_t i = 0; i < groups.size(); ++i) {
    for (auto type : groups[i]) {
      for (auto child : type.getReferencedHeapTypes()) {
        if (child.isBasic()) {
          continue;
        }
        auto childGroup = child.getRecGroup();
        if (childGroup == groups[i]) {
          continue;
        }
        depSets[groupIndices.at(childGroup)].insert(i);
      }
    }
  }
  TopologicalSort::Graph deps;
  deps.reserve(groups.size());
  for (size_t i = 0; i < groups.size(); ++i) {
    deps.emplace_back(depSets[i].begin(), depSets[i].end());
  }

  // Experimentally determined to be pretty good for a variety of programs in
  // different languages.
  constexpr double childFactor = 0.25;

  // Each rec group's weight, adjusted for its size and incorporating the weight
  // of its users.
  std::vector<double> weights(groups.size());
  for (size_t i = 0; i < groups.size(); ++i) {
    weights[i] = double(groupCounts[i]) / groups[i].size();
  }
  auto sorted = TopologicalSort::sort(deps);
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    for (auto user : deps[*it]) {
      weights[*it] += childFactor * weights[user];
    }
  }

  // If we've preserved the input type order on the module, we have to respect
  // that first. Use the index of the first type from each group. In principle
  // we could try to do something more robust like take the minimum index of all
  // the types in the group, but if the groups haven't been preserved, then we
  // won't be able to perfectly preserve the order anyway.
  std::vector<std::optional<Index>> groupTypeIndices;
  if (wasm.typeIndices.empty()) {
    groupTypeIndices.resize(groups.size());
  } else {
    groupTypeIndices.reserve(groups.size());
    for (auto group : groups) {
      groupTypeIndices.emplace_back();
      if (auto it = wasm.typeIndices.find(group[0]);
          it != wasm.typeIndices.end()) {
        groupTypeIndices.back() = it->second;
      }
    }
  }

  auto order = TopologicalSort::minSort(deps, [&](size_t a, size_t b) {
    auto indexA = groupTypeIndices[a];
    auto indexB = groupTypeIndices[b];
    // Groups with indices must be sorted before groups without indices to
    // ensure transitivity of this comparison relation.
    if (indexA.has_value() != indexB.has_value()) {
      return indexA.has_value();
    }
    // Sort by preserved index if we can.
    if (indexA && *indexA != *indexB) {
      return *indexA < *indexB;
    }
    // Otherwise sort by weight and break ties by the arbitrary deterministic
    // order in which we've collected types.
    auto weightA = weights[a];
    auto weightB = weights[b];
    if (weightA != weightB) {
      return weightA > weightB;
    }
    return a < b;
  });

  IndexedHeapTypes indexedTypes;
  indexedTypes.types.reserve(counts.size());

  for (auto groupIndex : order) {
    for (auto type : groups[groupIndex]) {
      indexedTypes.types.push_back(type);
    }
  }
  setIndices(indexedTypes);
  return indexedTypes;
}

} // namespace wasm::ModuleUtils
