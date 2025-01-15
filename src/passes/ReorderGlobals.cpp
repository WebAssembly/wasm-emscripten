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
// Sorts globals by their static use count. This helps reduce the size of wasm
// binaries because fewer bytes are needed to encode references to frequently
// used globals.
//
// This pass also sorts by dependencies, as each global can only appear after
// those it refers to. Other passes can use this internally to fix the ordering
// after they add new globals.
//
// The "always" variant of this pass will always sort globals, even if there are
// so few they all fit in a single LEB. In "always" mode we sort regardless and
// we measure size by considering each subsequent index to have a higher cost.
// That is, in reality the size of all globals up to 128 is a single byte, and
// then the LEB grows to 2, while in "always" mode we increase the size by 1/128
// for each global in a smooth manner. "Always" is mainly useful for testing.
//

#include "memory"

#include "ir/find_all.h"
#include "ir/global-utils.h"
#include "pass.h"
#include "support/topological_sort.h"
#include "wasm.h"

namespace wasm {

struct ReorderGlobals : public Pass {
  bool always;

  ReorderGlobals(bool always) : always(always) {}

  // For efficiency we will use global indices rather than names. That is, we
  // use the index of the global in the original ordering to identify each
  // global. A different ordering is then a vector of old indices, saying where
  // each element comes from, which is logically a mapping between indices.
  using IndexIndexMap = std::vector<Index>;

  // We will also track counts of uses for each global. We use floating-point
  // values here since while the initial counts are integers, we will be
  // considering fractional sums of them later.
  using IndexCountMap = std::vector<double>;

  void run(Module* module) override {
    auto& globals = module->globals;

    if (globals.size() < 128 && !always) {
      // The module has so few globals that they all fit in a single-byte U32LEB
      // value, so no reordering we can do can actually decrease code size. Note
      // that this is the common case with wasm MVP modules where the only
      // globals are typically the stack pointer and perhaps a handful of others
      // (however, features like wasm GC there may be a great many globals).
      // TODO: we still need to sort here to fix dependencies sometimes
      return;
    }

    GlobalUtils::UseCounter uses(*module);

    // Convert names to indices.
    std::unordered_map<Name, Index> originalIndices;
    for (Index i = 0; i < globals.size(); i++) {
      originalIndices[globals[i]->name] = i;
    }
    IndexCountMap counts(globals.size());
    for (auto& [name, count] : uses.globalUses) {
      counts[originalIndices[name]] = count;
    }

    // We must take into account dependencies, so that globals appear before
    // their users in other globals:
    //
    //   (global $a i32 (i32.const 10))
    //   (global $b i32 (global.get $a)) ;; $b depends on $a; $a must be first
    //
    // To do so we construct a map from each global to those that depends on it.
    std::vector<std::unordered_set<Index>> dependentSets(globals.size());
    for (Index i = 0; i < globals.size(); i++) {
      auto& global = globals[i];
      if (!global->imported()) {
        for (auto* get : FindAll<GlobalGet>(global->init).list) {
          auto getIndex = originalIndices[get->name];
          dependentSets[getIndex].insert(i);
        }
      }
    }
    TopologicalSort::Graph deps;
    deps.reserve(globals.size());
    for (Index i = 0; i < globals.size(); ++i) {
      deps.emplace_back(dependentSets[i].begin(), dependentSets[i].end());
    }

    // Compute various sorting options. All the options use a variation of the
    // algorithm in doSort() below, see there for more details; the only
    // difference between the sorts is in the use counts we provide it.
    struct SortAndSize {
      IndexIndexMap sort;
      double size;
      SortAndSize(IndexIndexMap&& sort, double size)
        : sort(std::move(sort)), size(size) {}
    };
    std::vector<SortAndSize> options;
    auto addOption = [&](const IndexCountMap& customCounts) {
      // Compute the sort using custom counts that guide us how to order.
      auto sort = doSort(customCounts, deps, module);
      // Compute the size using the true counts.
      auto size = computeSize(sort, counts);
      options.emplace_back(std::move(sort), size);
    };

    // Compute the closest thing we can to the original, unoptimized sort, by
    // setting all counts to 0 there, so it only takes into account dependencies
    // and the original ordering and nothing else.
    //
    // We put this sort first because if they all end up with equal size we
    // prefer it (as it avoids pointless changes).
    IndexCountMap zeroes(globals.size());
    addOption(zeroes);

    // A simple sort that uses the counts. As the algorithm in doSort() is
    // greedy (see below), this is a pure greedy sort (at each point it time it
    // picks the global with the highest count that it can).
    addOption(counts);

    // We can make the sort less greedy by adding to each global's count some of
    // the count of its children. Then we'd still be picking in a greedy manner
    // but our choices would be influenced by the bigger picture of what can be
    // unlocked by emitting a particular global (it may have a low count itself,
    // but allow emitting something that depends on it that has a high count).
    // We try two variations of this, one which is a simple sum (add the
    // dependent's counts to the global's) and one which exponentially decreases
    // them (that is, we add a small multiple of the dependent's counts, which
    // may help as the dependents also depend on other things potentially, so a
    // simple sum may be too naive).
    double const EXPONENTIAL_FACTOR = 0.095;
    IndexCountMap sumCounts(globals.size()), exponentialCounts(globals.size());

    auto sorted = TopologicalSort::sort(deps);
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
      auto global = *it;
      // We can compute this global's count as in the sorted order all the
      // values it cares about are resolved. Start with the self-count, then
      // add the deps.
      sumCounts[global] = exponentialCounts[global] = counts[global];
      for (auto dep : deps[global]) {
        sumCounts[global] += sumCounts[dep];
        exponentialCounts[global] +=
          EXPONENTIAL_FACTOR * exponentialCounts[dep];
      }
    }

    addOption(sumCounts);
    addOption(exponentialCounts);

    // Pick the best out of all the options we computed.
    IndexIndexMap* best = nullptr;
    double bestSize;
    for (auto& [sort, size] : options) {
      if (!best || size < bestSize) {
        best = &sort;
        bestSize = size;
      }
    }

    // Apply the indices we computed.
    auto old = std::move(globals);
    globals.resize(old.size());
    for (Index i = 0; i < old.size(); i++) {
      globals[i] = std::move(old[(*best)[i]]);
    }
    module->updateMaps();
  }

  IndexIndexMap doSort(const IndexCountMap& counts,
                       const TopologicalSort::Graph& deps,
                       Module* module) {
    // To sort the globals we do a simple greedy approach of always picking the
    // global with the highest count at every point in time, subject to the
    // constraint that we can only emit globals that have all of their
    // dependencies already emitted.
    //
    // The greedy approach here may also be suboptimal, however. Consider that
    // we might see that the best available global is $a, but if we instead
    // selected some other global $b, that would allow us to select a third
    // global $c that depends on $b, and $c might have a much higher use count
    // than $a. For that reason we try several variations of this with different
    // counts, see earlier.

    // Now use that optimal order to create an ordered graph that includes the
    // dependencies. The final order will be the minimum topological sort of
    // this graph.
    return TopologicalSort::minSort(deps, [&](Index a, Index b) {
      // Imports always go first. The binary writer takes care of this itself
      // anyhow, but it is better to do it here in the IR so we can actually
      // see what the final layout will be.
      auto aImported = module->globals[a]->imported();
      auto bImported = module->globals[b]->imported();
      if (aImported != bImported) {
        return aImported;
      }

      // Sort by the counts. Higher counts come first.
      auto aCount = counts[a];
      auto bCount = counts[b];
      if (aCount != bCount) {
        return aCount > bCount;
      }

      // Break ties using the original order, which means just using the
      // indices.
      return a < b;
    });
  }

  // Given an indexing of the globals and the counts of how many times each is
  // used, estimate the size of relevant parts of the wasm binary (that is, of
  // LEBs in global.gets).
  double computeSize(IndexIndexMap& indices, IndexCountMap& counts) {
    if (always) {
      // In this mode we gradually increase the cost of later globals, in an
      // unrealistic but smooth manner.
      double total = 0;
      for (Index i = 0; i < indices.size(); i++) {
        // Multiply the count for this global by a smoothed LEB factor, which
        // starts at 1 (for 1 byte) at index 0, and then increases linearly with
        // i, so that after 128 globals we reach 2 (which is the true index at
        // which the LEB size normally jumps from 1 to 2), and so forth.
        total += counts[indices[i]] * (1.0 + (i / 128.0));
      }
      return total;
    }

    // The total size we are computing.
    double total = 0;
    // Track the size in bits and the next index at which the size increases. At
    // the first iteration we'll compute the size of the LEB for index 0, and so
    // forth.
    Index sizeInBits = 0;
    Index nextSizeIncrease = 0;
    for (Index i = 0; i < indices.size(); i++) {
      if (i == nextSizeIncrease) {
        sizeInBits++;
        // At the current size we have 7 * sizeInBits bits to use.  For example,
        // at index 0 the size is 1 and we'll compute 128 here, and indeed after
        // emitting 128 globals (0,..,127) the 129th (at index 128) requires a
        // larger LEB.
        nextSizeIncrease = 1 << (7 * sizeInBits);
      }
      total += counts[indices[i]] * sizeInBits;
    }
    return total;
  }
};

Pass* createReorderGlobalsPass() { return new ReorderGlobals(false); }

Pass* createReorderGlobalsAlwaysPass() { return new ReorderGlobals(true); }

} // namespace wasm
