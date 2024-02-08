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

// Helpers for handling a generic range of values

#ifndef wasm_support_intervals_h
#define wasm_support_intervals_h

#include <set>
#include <vector>

namespace wasm {

// The weight determines the value of the
// interval when comparing against another interval, higher is better.
struct Interval {
  unsigned start;
  unsigned end;
  unsigned weight;
  Interval(unsigned start, unsigned end, unsigned weight)
    : start(start), end(end), weight(weight) {}

  bool operator<(const Interval& other) const {
    return start < other.start && weight < other.weight;
  }

  bool operator==(const Interval& other) const {
    return start == other.start && end == other.end && weight == other.weight;
  }
};

struct IntervalProcessor {
  // Given a vector of intervals, returns a new vector. To resolve overlapping
  // intervals, the interval with the highest weight is kept.
  static std::set<Interval> getOverlaps(std::vector<Interval>&);
};

} // namespace wasm

namespace std {

template<> struct hash<wasm::Interval> {
  size_t operator()(const wasm::Interval& i) const {
    return std::hash<unsigned>{}(i.start) + std::hash<unsigned>{}(i.end) +
           std::hash<unsigned>{}(i.weight);
  }
};

} // namespace std

#endif // wasm_suport_intervals
