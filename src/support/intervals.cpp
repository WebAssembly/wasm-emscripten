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

#include <assert.h>

#include "intervals.h"
#include "support/index.h"
#include <algorithm>

using namespace wasm;

std::set<Interval>
IntervalProcessor::getOverlaps(std::vector<Interval>& intervals) {
  std::sort(intervals.begin(), intervals.end(), [](Interval a, Interval b) {
    return a.start < b.end;
  });

  std::set<Interval> overlaps;
  auto& firstInterval = intervals[0];
  // Look for overlapping intervals
  for (Index i = 1; i < intervals.size(); i++) {
    auto& nextInterval = intervals[i];
    if (firstInterval.end < nextInterval.start) {
      firstInterval = nextInterval;
      continue;
    }

    // Keep the interval with the higher weight
    if (nextInterval.weight > firstInterval.weight) {
      overlaps.insert(firstInterval);
      firstInterval = nextInterval;
    } else {
      overlaps.insert(nextInterval);
    }
  }

  return overlaps;
}
