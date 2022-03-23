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

//
// Remove params from signature/function types where possible.
//
// This differs from DeadArgumentElimination in that DAE will look at each
// function by itself, and cannot handle indirectly-called functions. This pass
// looks at each heap type at a time, and if all functions with a heap type do
// not use a particular param, will remove the param.
//

#include "ir/find_all.h"
#include "ir/function-utils.h"
#include "ir/lubs.h"
#include "ir/module-utils.h"
#include "ir/type-updating.h"
#include "pass.h"
#include "support/sorted_vector.h"
#include "wasm-type.h"
#include "wasm.h"

using namespace std;

namespace wasm {

namespace {

struct SignaturePruning : public Pass {
  // Maps each heap type to the possible pruned heap type. We will fill this during analysis and then use it while doing
  // an update of the types. If a type has no improvement that we can find, it
  // will not appear in this map.
  std::unordered_map<HeapType, Signature> newSignatures;

  void run(PassRunner* runner, Module* module) override {
    if (getTypeSystem() != TypeSystem::Nominal) {
      Fatal() << "SignaturePruning requires nominal typing";
    }

    if (!module->tables.empty()) {
      // When there are tables we must also take their types into account, which
      // would require us to take call_indirect, element segments, etc. into
      // account. For now, do nothing if there are tables.
      // TODO
      return;
    }

    // First, find all the information we need. Start by collecting inside each
    // function in parallel.

    struct Info {
      std::vector<Call*> calls;
      std::vector<CallRef*> callRefs;

      std::unordered_set<Index> usedParams;
    };

    ModuleUtils::ParallelFunctionAnalysis<Info> analysis(
      *module, [&](Function* func, Info& info) {
        if (func->imported()) {
          return;
        }
        info.calls = std::move(FindAll<Call>(func->body).list);
        info.callRefs = std::move(FindAll<CallRef>(func->body).list);
        info.usedParams = FunctionUtils::getUsedParams(func);
      });

    // A map of types to all the information combined over all the functions
    // with that type.
    std::unordered_map<HeapType, Info> allInfo;

    // Map heap types to all functions with that type.
    std::unordered_map<HeapType, std::vector<Function*>> sigFuncs;

    // Combine all the information we gathered into that map.
    for (auto& [func, info] : analysis.map) {
      // For direct calls, add each call to the type of the function being
      // called.
      for (auto* call : info.calls) {
        allInfo[module->getFunction(call->target)->type].calls.push_back(call);
      }

      // For indirect calls, add each call_ref to the type the call_ref uses.
      for (auto* callRef : info.callRefs) {
        auto calledType = callRef->target->type;
        if (calledType != Type::unreachable) {
          allInfo[calledType.getHeapType()].callRefs.push_back(callRef);
        }
      }

      // A parameter used in this function is used in the heap type - just one
      // function is enough to prevent the parameter from being removed.
      auto allUsedParams = allInfo[func->type].usedParams;
      for (auto index : info.usedParams) {
        allUsedParams.insert(index);
      }

      sigFuncs[func->type].push_back(func);
    }

    // Find parameters to prune.
    std::unordered_set<HeapType> seen;
    for (auto& func : module->functions) {
      auto type = func->type;
      if (!seen.insert(type).second) {
        continue;
      }

      auto sig = type.getSignature();
      auto& info = allInfo[type];
      auto numParams = sig.params.size();
      if (info.usedParams.size() == numParams) {
        // All parameters are used, give up on this one.
        continue;
      }

      // We found possible work! Find the specific params that are unused try to
      // prune them.
      SortedVector unusedParams;
      for (Index i = 0; i < numParams; i++) {
        if (info.usedParams.count(i) == 0) {
          unusedParams.insert(i);
        }
      }
      if (!FunctionUtils::removeParameters(
            sigFuncs[type], unusedParams, info.calls, info.callRefs, module, runner)) {
        continue;
      }

      // Success! Update the types.
      std::vector<Type> newParams;
      for (Index i = 0; i < numParams; i++) {
        if (info.usedParams.count(i)) {
          newParams.push_back(func->getLocalType(i));
        }
      }
      newSignatures[type] = Signature(Type(newParams), sig.results);
    }

    if (newSignatures.empty()) {
      // We found nothing to optimize.
      return;
    }

    // Rewrite the types.
    // TODO: shared util with SignatureRefining
    class TypeRewriter : public GlobalTypeRewriter {
      SignaturePruning& parent;

    public:
      TypeRewriter(Module& wasm, SignaturePruning& parent)
        : GlobalTypeRewriter(wasm), parent(parent) {}

      void modifySignature(HeapType oldSignatureType, Signature& sig) override {
        auto iter = parent.newSignatures.find(oldSignatureType);
        if (iter != parent.newSignatures.end()) {
          sig.params = getTempType(iter->second.params);
          sig.results = getTempType(iter->second.results);
        }
      }
    };
    TypeRewriter(*module, *this).update();
  }
};

} // anonymous namespace

Pass* createSignaturePruningPass() { return new SignaturePruning(); }

} // namespace wasm
