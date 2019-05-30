/*
 * Copyright 2019 WebAssembly Community Group participants
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

#ifndef wasm_ir_stackification_h
#define wasm_ir_stackification_h

#include "wasm.h"

namespace wasm {

namespace Stackification {

// Stackifies the given expression in place then returns it for convenience.
Expression* stackify(Module&, Expression*&);

// Unstackifies the given expression, replacing push/pop pairs with
// local.get/local.set pairs.
// TODO: For multivalue, ensure that all remaining pops are children of
// local.set instructions and all remaining pushes directly provide multivalue
// return values.
Expression* unstackify(Module&, Expression*&);
void verifyUnstackified(Function* func);

} // namespace Stackification

} // namespace wasm

#endif // wasm_ir_stackification_h
