/*
 * Copyright 2021 WebAssembly Community Group participants
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

#ifndef wasm_ir_replacer_h
#define wasm_ir_replacer_h

#include <wasm-traversal.h>
#include <wasm.h>

namespace wasm {

// A map of replacements to perform on expressions. After filling the map,
// simply do a walk(..) and they will be replaced.
//
// This is useful for a general replacement of expressions. Some passes can
// store pointers to expressions and update only those, but then there is the
// nesting problem,
//
//  (foo
//   (bar
//    (baz
//
// If we replace bar with something then the pointer to baz may change. Doing
// the replacements in the order of a walk, as is done here, is always safe.
struct ExpressionReplacer
  : PostWalker<ExpressionReplacer,
               UnifiedExpressionVisitor<ExpressionReplacer>> {
  std::unordered_map<Expression*, Expression*> replacements;

  void visitExpression(Expression* curr) {
    auto iter = replacements.find(curr);
    if (iter != replacements.end()) {
      replaceCurrent(iter->second);
    }
  }
};

} // namespace wasm

#endif // wasm_ir_replacer_h
