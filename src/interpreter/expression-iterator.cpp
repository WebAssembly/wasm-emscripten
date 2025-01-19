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

#include <algorithm>

#include "expression-iterator.h"
#include "wasm-traversal.h"

namespace wasm::interpreter {

ExpressionIterator::ExpressionIterator(Expression* root) {
  // TODO: Visit loops at their beginnings instead of their ends.
  struct Collector
    : PostWalker<Collector, UnifiedExpressionVisitor<Collector>> {
    std::vector<Expression*>& exprs;
    Collector(std::vector<Expression*>& exprs) : exprs(exprs) {}
    void visitExpression(Expression* curr) { exprs.push_back(curr); }
  } collector(exprs);
  collector.walk(root);

  // Reverse the expressions so we can pop from the back to advance the
  // iterator.
  std::reverse(exprs.begin(), exprs.end());
}

} // namespace wasm::interpreter
