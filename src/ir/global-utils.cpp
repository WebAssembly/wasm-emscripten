/*
 * Copyright 2024 WebAssembly Community Group participants
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

#include "ir/global-utils.h"
#include "ir/find_all.h"
#include "wasm.h"

namespace wasm::GlobalUtils {

namespace {

// We'll count uses in parallel.
using AtomicNameCountMap = std::unordered_map<Name, std::atomic<Index>>;

struct UseCountScanner : public WalkerPass<PostWalker<UseCountScanner>> {
  bool isFunctionParallel() override { return true; }

  bool modifiesBinaryenIR() override { return false; }

  UseCountScanner(AtomicNameCountMap& counts) : counts(counts) {}

  std::unique_ptr<Pass> create() override {
    return std::make_unique<UseCountScanner>(counts);
  }

  void visitGlobalGet(GlobalGet* curr) {
    // We can't add a new element to the map in parallel.
    assert(counts.count(curr->name) > 0);
    counts[curr->name]++;
  }
  void visitGlobalSet(GlobalSet* curr) {
    assert(counts.count(curr->name) > 0);
    counts[curr->name]++;
  }

private:
  AtomicNameCountMap& counts;
};

} // namespace

UseCounter::UseCounter(Module& wasm) {
  auto& globals = wasm.globals;

  AtomicNameCountMap atomicCounts;
  // Fill in info, as we'll operate on it in parallel.
  for (auto& global : globals) {
    atomicCounts[global->name];
  }

  // Count.
  UseCountScanner scanner(atomicCounts);
  PassRunner runner(&wasm);
  scanner.run(&runner, &wasm);
  scanner.runOnModuleCode(&runner, &wasm);

  // Copy over to the final non-atomic storage.
  for (auto& [k, v] : atomicCounts) {
    globalUses[k] = v;
  }
}

} // namespace wasm::GlobalUtils
