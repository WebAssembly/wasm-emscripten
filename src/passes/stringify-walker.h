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

#ifndef wasm_passes_stringify_walker_h
#define wasm_passes_stringify_walker_h

#include "ir/iteration.h"
#include "ir/module-utils.h"
#include "ir/utils.h"
#include "wasm-traversal.h"
#include <queue>

namespace wasm {

/*
 * This walker does a normal postorder traversal except that it defers
 * traversing the contents of control flow structures it encounters. This
 * effectively un-nests control flow.
 *
 * For example, the below (contrived) wat:
 * 1 : (block
 * 2 :   (if
 * 3 :     (i32.const 0)
 * 4 :     (then (return (i32.const 1)))
 * 5 :     (else (return (i32.const 0)))))
 * 6 :   (drop
 * 7 :     (i32.add
 * 8 :       (i32.const 20)
 * 9 :       (i32.const 10)))
 * 10:   (if
 * 11:     (i32.const 1)
 * 12:     (then (return (i32.const 2)))
 * 14: )
 * Would have its expressions visited in the following order (based on line
 * number):
 * 1, 3, 2, 8, 9, 7, 6, 11, 10, 4, 5, 12
 *
 * Of note:
 *   - The visits to if-True on line 4 and if-False on line 5 are deferred until
 *     after the rest of the siblings of the if expression on line 2 are visited
 *   - The if-condition (i32.const 0) on line 3 is visited before the if
 *     expression on line 2. Similarly, the if-condition (i32.const 1) on line
 *     11 is visited before the if expression on line 10.
 *   - The add (line 7) binary operator's left and right children (lines 8 - 9)
 *     are visited first as they need to be on the stack before the add
 *     operation is executed
 */

template<typename SubType>
struct StringifyWalker
  : public PostWalker<SubType, UnifiedExpressionVisitor<SubType>> {

  using Super = PostWalker<SubType, UnifiedExpressionVisitor<SubType>>;

  std::queue<Expression**> controlFlowQueue;

  /*
   * To initiate the walk, subclasses should call walkModule with a pointer to
   * the wasm module.
   *
   * Member functions addUniqueSymbol and visitExpression are provided as
   * extension points for subclasses. These functions will be called at
   * appropriate points during the walk and should be implemented by subclasses.
   */
  void visitExpression(Expression* curr);
  void addUniqueSymbol();

  void doWalkModule(Module* module);
  void doWalkFunction(Function* func);
  void walk(Expression* curr);
  static void scan(SubType* self, Expression** currp);
  static void doVisitExpression(SubType* self, Expression** currp);

private:
  void dequeueControlFlow();
};

} // namespace wasm

#include "stringify-walker-impl.h"

namespace wasm {

// This custom hasher conforms to std::hash<Key>. Its purpose is to provide
// a custom hash for if expressions, so the if-condition of the if expression is
// not included in the hash for the if expression. This is needed because in the
// binary format, the if-condition comes before and is consumed by the if. To
// match the binary format, we hash the if condition before and separately from
// the rest of the if expression.
struct StringifyHasher {
  size_t operator()(Expression* curr) const;
};

// This custom equator conforms to std::equal_to<Key>. Similar to
// StringifyHasher, it's purpose is to not include the if-condition when
// evaluating the equality of two if expressions.
struct StringifyEquator {
  bool operator()(Expression* lhs, Expression* rhs) const;
};

struct HashStringifyWalker : public StringifyWalker<HashStringifyWalker> {
  // After calling walkModule, this vector contains the result of encoding a
  // wasm module as a string of uint64_t values. Each value represents either an
  // Expression or a separator to mark the end of control flow.
  std::vector<unsigned> hashString;
  // A monotonic counter used to ensure that unique expressions in the
  // module are assigned a unique value in the hashString
  unsigned nextVal = 0;
  // A monotonic counter used to ensure that each separator in the
  // module is assigned a unique value in the hashString
  signed nextSeparatorVal = -1;
  // Contains a mapping of expression pointer to value to ensure we
  // use the same value for matching expressions. A custom hasher and
  // equator is provided in order to separate out evaluation of the if-condition
  // when evaluating if expressions.
  std::unordered_map<Expression*, unsigned, StringifyHasher, StringifyEquator>
    exprToCounter;

  void addUniqueSymbol();
  void visitExpression(Expression* curr);
};

} // namespace wasm

#endif // wasm_passes_stringify_walker_h
