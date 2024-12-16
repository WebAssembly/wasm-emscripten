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

#include "stringify-walker.h"
#include "support/intervals.h"

namespace wasm {

size_t StringifyHasher::operator()(Expression* curr) const {
  if (Properties::isControlFlowStructure(curr)) {
    if (auto* iff = curr->dynCast<If>()) {
      size_t digest = wasm::hash(iff->_id);
      rehash(digest, ExpressionAnalyzer::hash(iff->ifTrue));
      if (iff->ifFalse) {
        rehash(digest, ExpressionAnalyzer::hash(iff->ifFalse));
      }
      return digest;
    }

    return ExpressionAnalyzer::hash(curr);
  }

  return ExpressionAnalyzer::shallowHash(curr);
}

bool StringifyEquator::operator()(Expression* lhs, Expression* rhs) const {
  if (Properties::isControlFlowStructure(lhs) &&
      Properties::isControlFlowStructure(rhs)) {
    auto* iffl = lhs->dynCast<If>();
    auto* iffr = rhs->dynCast<If>();

    if (iffl && iffr) {
      return ExpressionAnalyzer::equal(iffl->ifTrue, iffr->ifTrue) &&
             ExpressionAnalyzer::equal(iffl->ifFalse, iffr->ifFalse);
    }

    return ExpressionAnalyzer::equal(lhs, rhs);
  }

  return ExpressionAnalyzer::shallowEqual(lhs, rhs);
}

void HashStringifyWalker::addUniqueSymbol(SeparatorReason reason) {
  // Use a negative value to distinguish symbols for separators from symbols
  // for Expressions
  assert((uint32_t)nextSeparatorVal >= nextVal);
  if (auto funcStart = reason.getFuncStart()) {
    idxToFuncName.insert({hashString.size(), funcStart->func->name});
  }
  hashString.push_back((uint32_t)nextSeparatorVal);
  nextSeparatorVal--;
  exprs.push_back(nullptr);
}

void HashStringifyWalker::visitExpression(Expression* curr) {
  auto [it, inserted] = exprToCounter.insert({curr, nextVal});
  hashString.push_back(it->second);
  exprs.push_back(curr);
  if (inserted) {
    nextVal++;
  }
}

std::pair<uint32_t, Name>
HashStringifyWalker::makeRelative(uint32_t idx) const {
  // The upper_bound function returns an iterator to the first value in the set
  // that is true for idx < value. We subtract one from this returned value to
  // tell us which function actually contains the the idx.
  auto [funcIdx, func] = *--idxToFuncName.upper_bound(idx);
  return {idx - funcIdx, func};
}

std::vector<SuffixTree::RepeatedSubstring>
StringifyProcessor::repeatSubstrings(std::vector<uint32_t>& hashString) {
  SuffixTree st(hashString);
  std::vector<SuffixTree::RepeatedSubstring> substrings(st.begin(), st.end());
  for (auto& substring : substrings) {
    // Sort by increasing start index to ensure determinism.
    std::sort(substring.StartIndices.begin(), substring.StartIndices.end());
  }
  // Substrings are sorted so that the longest substring that repeats the most
  // times is ordered first. This is done so that we can assume the most
  // worthwhile substrings to outline come first.
  std::sort(
    substrings.begin(),
    substrings.end(),
    [](SuffixTree::RepeatedSubstring a, SuffixTree::RepeatedSubstring b) {
      size_t aWeight = a.Length * a.StartIndices.size();
      size_t bWeight = b.Length * b.StartIndices.size();
      if (aWeight == bWeight) {
        return a.StartIndices[0] < b.StartIndices[0];
      }
      return aWeight > bWeight;
    });
  return substrings;
}

// Deduplicate substrings by iterating through the list of substrings, keeping
// only those whose list of end indices is disjoint from the set of end indices
// for all substrings kept so far. Substrings that are contained within other
// substrings will always share an end index with those other substrings. Note
// that this deduplication may be over-aggressive, since it will remove
// substrings that are contained within any previous substring, even if they
// have many other occurrences that are not inside other substrings. Part of the
// reason dedupe can be so aggressive is an assumption 1) that the input
// substrings have been sorted so that the longest substrings with the most
// repeats come first and 2) these are more worthwhile to keep than subsequent
// substrings of substrings, even if they appear more times.
std::vector<SuffixTree::RepeatedSubstring> StringifyProcessor::dedupe(
  const std::vector<SuffixTree::RepeatedSubstring>& substrings) {
  std::unordered_set<uint32_t> seen;
  std::vector<SuffixTree::RepeatedSubstring> result;
  for (auto substring : substrings) {
    std::vector<uint32_t> idxToInsert;
    bool seenEndIdx = false;
    for (auto startIdx : substring.StartIndices) {
      // We are using the end index to ensure that each repeated substring
      // reported by the SuffixTree is unique. This is because LLVM's SuffixTree
      // reports back repeat sequences that are substrings of longer repeat
      // sequences with the same endIdx, and we generally prefer to outline
      // longer repeat sequences.
      uint32_t endIdx = substring.Length + startIdx;
      if (seen.count(endIdx)) {
        seenEndIdx = true;
        break;
      }
      idxToInsert.push_back(endIdx);
    }
    if (!seenEndIdx) {
      seen.insert(idxToInsert.begin(), idxToInsert.end());
      result.push_back(substring);
    }
  }

  return result;
}

std::vector<SuffixTree::RepeatedSubstring> StringifyProcessor::removeOverlaps(
  const std::vector<SuffixTree::RepeatedSubstring>& substrings) {
  std::vector<Interval> intervals;
  std::unordered_map<Interval, unsigned> intervalMap;

  // Construct intervals
  for (Index i = 0; i < substrings.size(); i++) {
    auto substring = substrings[i];
    for (auto startIdx : substring.StartIndices) {
      auto interval =
        Interval(startIdx,
                 startIdx + substring.Length - 1,
                 substring.Length * substring.StartIndices.size());
      intervals.push_back(interval);
      intervalMap[std::move(interval)] = i;
    }
  }

  // Get the overlapping intervals
  std::set<Interval> overlaps = IntervalProcessor::getOverlaps(intervals);
  std::set<unsigned> doNotInclude;
  for (auto& interval : overlaps) {
    doNotInclude.insert(intervalMap[interval]);
  }

  // Add the substrings that don't overlap to the result
  std::vector<SuffixTree::RepeatedSubstring> result;
  for (Index i = 0; i < substrings.size(); i++) {
    if (doNotInclude.find(i) != doNotInclude.end()) {
      continue;
    }

    auto substring = substrings[i];
    result.push_back(substring);
  }

  return result;
}

std::vector<SuffixTree::RepeatedSubstring> StringifyProcessor::filter(
  const std::vector<SuffixTree::RepeatedSubstring>& substrings,
  const std::vector<Expression*>& exprs,
  std::function<bool(const Expression*)> condition) {

  struct FilterStringifyWalker : public StringifyWalker<FilterStringifyWalker> {
    bool hasFilterValue = false;
    std::function<bool(const Expression*)> condition;

    FilterStringifyWalker(std::function<bool(const Expression*)> condition)
      : condition(condition){};

    void walk(Expression* curr) {
      hasFilterValue = false;
      Super::walk(curr);
    }

    void addUniqueSymbol(SeparatorReason reason) {}

    void visitExpression(Expression* curr) {
      if (condition(curr)) {
        hasFilterValue = true;
      }
    }
  };

  FilterStringifyWalker walker(condition);

  std::vector<SuffixTree::RepeatedSubstring> result;
  for (auto substring : substrings) {
    bool hasFilterValue = false;
    for (auto idx = substring.StartIndices[0],
              endIdx = substring.StartIndices[0] + substring.Length;
         idx < endIdx;
         idx++) {
      Expression* curr = exprs[idx];
      if (Properties::isControlFlowStructure(curr)) {
        walker.walk(curr);
        if (walker.hasFilterValue) {
          hasFilterValue = true;
          break;
        }
      }
      if (condition(curr)) {
        hasFilterValue = true;
        break;
      }
    }
    if (!hasFilterValue) {
      result.push_back(substring);
    }
  }

  return result;
}

std::vector<SuffixTree::RepeatedSubstring> StringifyProcessor::filterLocalSets(
  const std::vector<SuffixTree::RepeatedSubstring>& substrings,
  const std::vector<Expression*>& exprs) {
  return StringifyProcessor::filter(
    substrings, exprs, [](const Expression* curr) {
      return curr->is<LocalSet>();
    });
}

std::vector<SuffixTree::RepeatedSubstring> StringifyProcessor::filterLocalGets(
  const std::vector<SuffixTree::RepeatedSubstring>& substrings,
  const std::vector<Expression*>& exprs) {
  return StringifyProcessor::filter(
    substrings, exprs, [](const Expression* curr) {
      return curr->is<LocalGet>();
    });
}

std::vector<SuffixTree::RepeatedSubstring> StringifyProcessor::filterBranches(
  const std::vector<SuffixTree::RepeatedSubstring>& substrings,
  const std::vector<Expression*>& exprs) {
  return StringifyProcessor::filter(
    substrings, exprs, [](const Expression* curr) {
      return Properties::isBranch(curr) || curr->is<Return>();
    });
}

} // namespace wasm
