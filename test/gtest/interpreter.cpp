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

// TODO: Replace this test file with spec tests as soon as possible.

#include "interpreter/interpreter.h"
#include "literal.h"
#include "wasm-ir-builder.h"
#include "wasm.h"

#include "gtest/gtest.h"

using namespace wasm;

TEST(InterpreterTest, Add) {
  Module wasm;
  IRBuilder builder(wasm);

  ASSERT_FALSE(builder.makeConst(Literal(uint32_t(1))).getErr());
  ASSERT_FALSE(builder.makeConst(Literal(uint32_t(2))).getErr());
  ASSERT_FALSE(builder.makeBinary(AddInt32).getErr());

  auto expr = builder.build();
  ASSERT_FALSE(expr.getErr());

  auto results = Interpreter{}.run(*expr);
  std::vector<Literal> expected{Literal(uint32_t(3))};

  EXPECT_EQ(results, expected);
}
