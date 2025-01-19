/*
 * Copyright 2025 WebAssembly Community Group participants
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

#ifndef interpreter_expression_iterator_h
#define interpreter_expression_iterator_h

#include <cassert>
#include <iterator>
#include <vector>

#include "wasm.h"

namespace wasm::interpreter {

// TODO: This is a quick and dirty hack. We should implement a proper iterator
// in ir/iteration.h that keeps only a vector of (Expression*, index) pairs to
// find the current location in the epxression tree.
struct ExpressionIterator {
  using difference_type = std::ptrdiff_t;
  using value_type = Expression*;
  using pointer = Expression**;
  using reference = Expression*&;
  using iterator_category = std::input_iterator_tag;

  // The list of remaining instructions in reverse order so we can pop from the
  // back to advance the iterator.
  std::vector<Expression*> exprs;

  ExpressionIterator(Expression* root);

  ExpressionIterator() = default;
  ExpressionIterator(const ExpressionIterator&) = default;
  ExpressionIterator(ExpressionIterator&&) = default;
  ExpressionIterator& operator=(const ExpressionIterator&) = default;
  ExpressionIterator& operator=(ExpressionIterator&&) = default;

  operator bool() { return exprs.size(); }

  Expression* operator*() {
    assert(exprs.size());
    return exprs.back();
  }

  ExpressionIterator& operator++() {
    assert(exprs.size());
    exprs.pop_back();
    return *this;
  }

  bool operator==(const ExpressionIterator& other) {
    return exprs.size() == other.exprs.size();
  }

  bool operator!=(const ExpressionIterator& other) { return !(*this == other); }
};

} // namespace wasm::interpreter

#endif // interpreter_expression_iterator_h
