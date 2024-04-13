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

#include <cassert>

#include "ir/names.h"
#include "ir/properties.h"
#include "ir/utils.h"
#include "wasm-ir-builder.h"

#define IR_BUILDER_DEBUG 0

#if IR_BUILDER_DEBUG
#define DBG(statement) statement
#else
#define DBG(statement)
#endif

using namespace std::string_literals;

namespace wasm {

namespace {

Result<> validateTypeAnnotation(HeapType type, Expression* child) {
  if (child->type == Type::unreachable) {
    return Ok{};
  }
  if (!child->type.isRef() ||
      !HeapType::isSubType(child->type.getHeapType(), type)) {
    return Err{"invalid reference type on stack"};
  }
  return Ok{};
}

} // anonymous namespace

Result<Index> IRBuilder::addScratchLocal(Type type) {
  if (!func) {
    return Err{"scratch local required, but there is no function context"};
  }
  Name name = Names::getValidLocalName(*func, "scratch");
  return Builder::addVar(func, name, type);
}

MaybeResult<IRBuilder::HoistedVal> IRBuilder::hoistLastValue() {
  auto& stack = getScope().exprStack;
  int index = stack.size() - 1;
  for (; index >= 0; --index) {
    if (stack[index]->type != Type::none) {
      break;
    }
  }
  if (index < 0) {
    // There is no value-producing or unreachable expression.
    return {};
  }
  if (unsigned(index) == stack.size() - 1) {
    // Value-producing expression already on top of the stack.
    return HoistedVal{Index(index), nullptr};
  }
  auto*& expr = stack[index];
  auto type = expr->type;
  if (type == Type::unreachable) {
    // Make sure the top of the stack also has an unreachable expression.
    if (stack.back()->type != Type::unreachable) {
      push(builder.makeUnreachable());
    }
    return HoistedVal{Index(index), nullptr};
  }
  // Hoist with a scratch local.
  auto scratchIdx = addScratchLocal(type);
  CHECK_ERR(scratchIdx);
  expr = builder.makeLocalSet(*scratchIdx, expr);
  auto* get = builder.makeLocalGet(*scratchIdx, type);
  push(get);
  return HoistedVal{Index(index), get};
}

Result<> IRBuilder::packageHoistedValue(const HoistedVal& hoisted,
                                        size_t sizeHint) {
  auto& scope = getScope();
  assert(!scope.exprStack.empty());

  auto packageAsBlock = [&](Type type) {
    // Create a block containing the producer of the hoisted value, the final
    // get of the hoisted value, and everything in between.
    std::vector<Expression*> exprs(scope.exprStack.begin() + hoisted.valIndex,
                                   scope.exprStack.end());
    auto* block = builder.makeBlock(exprs, type);
    scope.exprStack.resize(hoisted.valIndex);
    push(block);
  };

  auto type = scope.exprStack.back()->type;

  if (type.size() == sizeHint || type.size() <= 1) {
    if (hoisted.get) {
      packageAsBlock(type);
    }
    return Ok{};
  }

  // We need to break up the hoisted tuple. Create and push an expression
  // setting the tuple to a local and returning its first element, then push
  // additional gets of each of its subsequent elements. Reuse the scratch local
  // we used for hoisting, if it exists.
  Index scratchIdx;
  if (hoisted.get) {
    // Update the get on top of the stack to just return the first element.
    scope.exprStack.back() = builder.makeTupleExtract(hoisted.get, 0);
    packageAsBlock(type[0]);
    scratchIdx = hoisted.get->index;
  } else {
    auto scratch = addScratchLocal(type);
    CHECK_ERR(scratch);
    scope.exprStack.back() = builder.makeTupleExtract(
      builder.makeLocalTee(*scratch, scope.exprStack.back(), type), 0);
    scratchIdx = *scratch;
  }
  for (Index i = 1, size = type.size(); i < size; ++i) {
    push(builder.makeTupleExtract(builder.makeLocalGet(scratchIdx, type), i));
  }
  return Ok{};
}

void IRBuilder::push(Expression* expr) {
  auto& scope = getScope();
  if (expr->type == Type::unreachable) {
    // We want to avoid popping back past this most recent unreachable
    // instruction. Drop all prior instructions so they won't be consumed by
    // later instructions but will still be emitted for their side effects, if
    // any.
    for (auto& expr : scope.exprStack) {
      expr = builder.dropIfConcretelyTyped(expr);
    }
    scope.unreachable = true;
  }
  scope.exprStack.push_back(expr);

  applyDebugLoc(expr);

  DBG(std::cerr << "After pushing " << ShallowExpression{expr} << ":\n");
  DBG(dump());
}

Result<Expression*> IRBuilder::pop(size_t size) {
  assert(size >= 1);
  auto& scope = getScope();

  // Find the suffix of expressions that do not produce values.
  auto hoisted = hoistLastValue();
  CHECK_ERR(hoisted);
  if (!hoisted) {
    // There are no expressions that produce values.
    if (scope.unreachable) {
      return builder.makeUnreachable();
    }
    return Err{"popping from empty stack"};
  }

  CHECK_ERR(packageHoistedValue(*hoisted, size));

  auto* ret = scope.exprStack.back();
  // If the top value has the correct size, we can pop it and be done.
  // Unreachable values satisfy any size.
  if (ret->type.size() == size || ret->type == Type::unreachable) {
    scope.exprStack.pop_back();
    return ret;
  }

  // The last value-producing expression did not produce exactly the right
  // number of values, so we need to construct a tuple piecewise instead.
  assert(size > 1);
  std::vector<Expression*> elems;
  elems.resize(size);
  for (int i = size - 1; i >= 0; --i) {
    auto elem = pop();
    CHECK_ERR(elem);
    elems[i] = *elem;
  }
  return builder.makeTupleMake(elems);
}

Result<Expression*> IRBuilder::build() {
  if (scopeStack.empty()) {
    return builder.makeNop();
  }
  if (scopeStack.size() > 1 || !scopeStack.back().isNone()) {
    return Err{"unfinished block context"};
  }
  if (scopeStack.back().exprStack.size() > 1) {
    return Err{"unused expressions without block context"};
  }
  assert(scopeStack.back().exprStack.size() == 1);
  auto* expr = scopeStack.back().exprStack.back();
  scopeStack.clear();
  labelDepths.clear();
  return expr;
}

void IRBuilder::setDebugLocation(const Function::DebugLocation& loc) {
  DBG(std::cerr << "setting debugloc " << loc.fileIndex << ":" << loc.lineNumber
                << ":" << loc.columnNumber << "\n";);
  debugLoc = loc;
}

void IRBuilder::applyDebugLoc(Expression* expr) {
  if (debugLoc) {
    if (func) {
      DBG(std::cerr << "applying debugloc " << debugLoc->fileIndex << ":"
                    << debugLoc->lineNumber << ":" << debugLoc->columnNumber
                    << " to expression " << ShallowExpression{expr} << "\n");
      func->debugLocations[expr] = *debugLoc;
    }
    debugLoc.reset();
  }
}

void IRBuilder::dump() {
#if IR_BUILDER_DEBUG
  std::cerr << "Scope stack";
  if (func) {
    std::cerr << " in function $" << func->name;
  }
  std::cerr << ":\n";

  for (auto& scope : scopeStack) {
    std::cerr << "  scope ";
    if (scope.isNone()) {
      std::cerr << "none";
    } else if (auto* f = scope.getFunction()) {
      std::cerr << "func " << f->name;
    } else if (scope.getBlock()) {
      std::cerr << "block";
    } else if (scope.getIf()) {
      std::cerr << "if";
    } else if (scope.getElse()) {
      std::cerr << "else";
    } else if (scope.getLoop()) {
      std::cerr << "loop";
    } else if (auto* tryy = scope.getTry()) {
      std::cerr << "try";
      if (tryy->name) {
        std::cerr << " " << tryy->name;
      }
    } else if (auto* tryy = scope.getCatch()) {
      std::cerr << "catch";
      if (tryy->name) {
        std::cerr << " " << tryy->name;
      }
    } else if (auto* tryy = scope.getCatchAll()) {
      std::cerr << "catch_all";
      if (tryy->name) {
        std::cerr << " " << tryy->name;
      }
    } else {
      WASM_UNREACHABLE("unexpected scope");
    }

    if (auto name = scope.getOriginalLabel()) {
      std::cerr << " (original label: " << name << ")";
    }

    if (scope.label) {
      std::cerr << " (label: " << scope.label << ")";
    }

    if (scope.unreachable) {
      std::cerr << " (unreachable)";
    }

    std::cerr << ":\n";

    for (auto* expr : scope.exprStack) {
      std::cerr << "    " << ShallowExpression{expr} << "\n";
    }
  }
#endif // IR_BUILDER_DEBUG
}

Result<> IRBuilder::visit(Expression* curr) {
  // Call either `visitExpression` or an expression-specific override.
  auto val = UnifiedExpressionVisitor<IRBuilder, Result<>>::visit(curr);
  CHECK_ERR(val);
  if (auto* block = curr->dynCast<Block>()) {
    block->finalize(block->type);
  } else {
    // TODO: Call more efficient versions of finalize() that take the known type
    // for other kinds of nodes as well, as done above.
    ReFinalizeNode{}.visit(curr);
  }
  push(curr);
  return Ok{};
}

// Handle the common case of instructions with a constant number of children
// uniformly.
Result<> IRBuilder::visitExpression(Expression* curr) {
  if (Properties::isControlFlowStructure(curr)) {
    // Control flow structures (besides `if`, handled separately) do not consume
    // stack values.
    return Ok{};
  }

#define DELEGATE_ID curr->_id
#define DELEGATE_START(id) [[maybe_unused]] auto* expr = curr->cast<id>();
#define DELEGATE_GET_FIELD(id, field) expr->field
#define DELEGATE_FIELD_CHILD(id, field)                                        \
  auto field = pop();                                                          \
  CHECK_ERR(field);                                                            \
  expr->field = *field;
#define DELEGATE_FIELD_SCOPE_NAME_DEF(id, field)                               \
  if (labelDepths.count(expr->field)) {                                        \
    return Err{"repeated label"};                                              \
  }
#define DELEGATE_END(id)

#define DELEGATE_FIELD_OPTIONAL_CHILD(id, field)                               \
  WASM_UNREACHABLE("should have called visit" #id " because " #id              \
                   " has optional child " #field);
#define DELEGATE_FIELD_CHILD_VECTOR(id, field)                                 \
  WASM_UNREACHABLE("should have called visit" #id " because " #id              \
                   " has child vector " #field);

#define DELEGATE_FIELD_INT(id, field)
#define DELEGATE_FIELD_LITERAL(id, field)
#define DELEGATE_FIELD_NAME(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_USE(id, field)

#define DELEGATE_FIELD_TYPE(id, field)
#define DELEGATE_FIELD_HEAPTYPE(id, field)
#define DELEGATE_FIELD_ADDRESS(id, field)

#include "wasm-delegations-fields.def"

  return Ok{};
}

Result<> IRBuilder::visitDrop(Drop* curr, std::optional<uint32_t> arity) {
  // Multivalue drops must remain multivalue drops.
  if (!arity) {
    arity = curr->value->type.size();
  }
  if (*arity >= 2) {
    auto val = pop(*arity);
    CHECK_ERR(val);
    curr->value = *val;
    return Ok{};
  }
  return visitExpression(curr);
}

Result<> IRBuilder::visitIf(If* curr) {
  // Only the condition is popped from the stack. The ifTrue and ifFalse are
  // self-contained so we do not modify them.
  auto cond = pop();
  CHECK_ERR(cond);
  curr->condition = *cond;
  return Ok{};
}

Result<> IRBuilder::visitReturn(Return* curr) {
  if (!func) {
    return Err{"cannot return outside of a function"};
  }
  size_t n = func->getResults().size();
  if (n == 0) {
    curr->value = nullptr;
  } else {
    auto val = pop(n);
    CHECK_ERR(val);
    curr->value = *val;
  }
  return Ok{};
}

Result<> IRBuilder::visitStructNew(StructNew* curr) {
  for (size_t i = 0, n = curr->operands.size(); i < n; ++i) {
    auto val = pop();
    CHECK_ERR(val);
    curr->operands[n - 1 - i] = *val;
  }
  return Ok{};
}

Result<> IRBuilder::visitArrayNew(ArrayNew* curr) {
  auto size = pop();
  CHECK_ERR(size);
  curr->size = *size;
  if (!curr->isWithDefault()) {
    auto init = pop();
    CHECK_ERR(init);
    curr->init = *init;
  }
  return Ok{};
}

Result<> IRBuilder::visitArrayNewFixed(ArrayNewFixed* curr) {
  for (size_t i = 0, size = curr->values.size(); i < size; ++i) {
    auto val = pop();
    CHECK_ERR(val);
    curr->values[size - i - 1] = *val;
  }
  return Ok{};
}

Result<Expression*> IRBuilder::getBranchValue(Expression* curr,
                                              Name labelName,
                                              std::optional<Index> label) {
  // As new branch instructions are added, one of the existing branch visit*
  // functions is likely to be copied, along with its call to getBranchValue().
  // This assert serves as a reminder to also add an implementation of
  // visit*WithType() for new branch instructions.
  assert(curr->is<Break>() || curr->is<Switch>());
  if (!label) {
    auto index = getLabelIndex(labelName);
    CHECK_ERR(index);
    label = *index;
  }
  auto scope = getScope(*label);
  CHECK_ERR(scope);
  // Loops would receive their input type rather than their output type, if we
  // supported that.
  size_t numValues = (*scope)->getLoop() ? 0 : (*scope)->getResultType().size();
  return numValues == 0 ? nullptr : pop(numValues);
}

Result<> IRBuilder::visitBreak(Break* curr, std::optional<Index> label) {
  if (curr->condition) {
    auto cond = pop();
    CHECK_ERR(cond);
    curr->condition = *cond;
  }
  auto value = getBranchValue(curr, curr->name, label);
  CHECK_ERR(value);
  curr->value = *value;
  return Ok{};
}

Result<> IRBuilder::visitBreakWithType(Break* curr, Type type) {
  if (curr->condition) {
    auto cond = pop();
    CHECK_ERR(cond);
    curr->condition = *cond;
  }
  if (type == Type::none) {
    curr->value = nullptr;
  } else {
    auto value = pop(type.size());
    CHECK_ERR(value)
    curr->value = *value;
  }
  curr->finalize();
  push(curr);
  return Ok{};
}

Result<> IRBuilder::visitSwitch(Switch* curr,
                                std::optional<Index> defaultLabel) {
  auto cond = pop();
  CHECK_ERR(cond);
  curr->condition = *cond;
  auto value = getBranchValue(curr, curr->default_, defaultLabel);
  CHECK_ERR(value);
  curr->value = *value;
  return Ok{};
}

Result<> IRBuilder::visitSwitchWithType(Switch* curr, Type type) {
  auto cond = pop();
  CHECK_ERR(cond);
  curr->condition = *cond;
  if (type == Type::none) {
    curr->value = nullptr;
  } else {
    auto value = pop(type.size());
    CHECK_ERR(value)
    curr->value = *value;
  }
  curr->finalize();
  push(curr);
  return Ok{};
}

Result<> IRBuilder::visitCall(Call* curr) {
  auto numArgs = wasm.getFunction(curr->target)->getNumParams();
  curr->operands.resize(numArgs);
  for (size_t i = 0; i < numArgs; ++i) {
    auto arg = pop();
    CHECK_ERR(arg);
    curr->operands[numArgs - 1 - i] = *arg;
  }
  return Ok{};
}

Result<> IRBuilder::visitCallIndirect(CallIndirect* curr) {
  auto target = pop();
  CHECK_ERR(target);
  curr->target = *target;
  auto numArgs = curr->heapType.getSignature().params.size();
  curr->operands.resize(numArgs);
  for (size_t i = 0; i < numArgs; ++i) {
    auto arg = pop();
    CHECK_ERR(arg);
    curr->operands[numArgs - 1 - i] = *arg;
  }
  return Ok{};
}

Result<> IRBuilder::visitCallRef(CallRef* curr) {
  auto target = pop();
  CHECK_ERR(target);
  curr->target = *target;
  for (size_t i = 0, numArgs = curr->operands.size(); i < numArgs; ++i) {
    auto arg = pop();
    CHECK_ERR(arg);
    curr->operands[numArgs - 1 - i] = *arg;
  }
  return Ok{};
}

Result<> IRBuilder::visitLocalSet(LocalSet* curr) {
  auto type = func->getLocalType(curr->index);
  auto val = pop(type.size());
  CHECK_ERR(val);
  curr->value = *val;
  return Ok{};
}

Result<> IRBuilder::visitGlobalSet(GlobalSet* curr) {
  auto type = wasm.getGlobal(curr->name)->type;
  auto val = pop(type.size());
  CHECK_ERR(val);
  curr->value = *val;
  return Ok{};
}
Result<> IRBuilder::visitThrow(Throw* curr) {
  auto numArgs = wasm.getTag(curr->tag)->sig.params.size();
  curr->operands.resize(numArgs);
  for (size_t i = 0; i < numArgs; ++i) {
    auto arg = pop();
    CHECK_ERR(arg);
    curr->operands[numArgs - 1 - i] = *arg;
  }
  return Ok{};
}

Result<> IRBuilder::visitStringNew(StringNew* curr) {
  switch (curr->op) {
    case StringNewUTF8:
    case StringNewWTF8:
    case StringNewLossyUTF8:
    case StringNewWTF16: {
      auto len = pop();
      CHECK_ERR(len);
      curr->length = *len;
      break;
    }
    case StringNewUTF8Array:
    case StringNewWTF8Array:
    case StringNewLossyUTF8Array:
    case StringNewWTF16Array: {
      auto end = pop();
      CHECK_ERR(end);
      curr->end = *end;
      auto start = pop();
      CHECK_ERR(start);
      curr->start = *start;
      break;
    }
    case StringNewFromCodePoint:
      break;
  }
  auto ptr = pop();
  CHECK_ERR(ptr);
  curr->ptr = *ptr;
  return Ok{};
}

Result<> IRBuilder::visitStringEncode(StringEncode* curr) {
  switch (curr->op) {
    case StringEncodeUTF8Array:
    case StringEncodeLossyUTF8Array:
    case StringEncodeWTF8Array:
    case StringEncodeWTF16Array: {
      auto start = pop();
      CHECK_ERR(start);
      curr->start = *start;
    }
      [[fallthrough]];
    case StringEncodeUTF8:
    case StringEncodeLossyUTF8:
    case StringEncodeWTF8:
    case StringEncodeWTF16: {
      auto ptr = pop();
      CHECK_ERR(ptr);
      curr->ptr = *ptr;
      auto ref = pop();
      CHECK_ERR(ref);
      curr->ref = *ref;
      return Ok{};
    }
  }
  WASM_UNREACHABLE("unexpected op");
}

Result<> IRBuilder::visitContBind(ContBind* curr) {
  auto cont = pop();
  CHECK_ERR(cont);
  curr->cont = *cont;

  size_t paramsBefore =
    curr->contTypeBefore.getContinuation().type.getSignature().params.size();
  size_t paramsAfter =
    curr->contTypeAfter.getContinuation().type.getSignature().params.size();
  if (paramsBefore < paramsAfter) {
    return Err{"incompatible continuation types in cont.bind: source type " +
               curr->contTypeBefore.toString() +
               " has fewer parameters than destination " +
               curr->contTypeAfter.toString()};
  }
  size_t numArgs = paramsBefore - paramsAfter;

  curr->operands.resize(numArgs);
  for (size_t i = 0; i < numArgs; ++i) {
    auto val = pop();
    CHECK_ERR(val);
    curr->operands[numArgs - i - 1] = *val;
  }
  return Ok{};
}

Result<> IRBuilder::visitResume(Resume* curr) {
  auto cont = pop();
  CHECK_ERR(cont);
  curr->cont = *cont;

  auto sig = curr->contType.getContinuation().type.getSignature();
  auto size = sig.params.size();
  curr->operands.resize(size);
  for (size_t i = 0; i < size; ++i) {
    auto val = pop();
    CHECK_ERR(val);
    curr->operands[size - i - 1] = *val;
  }
  return Ok{};
}

Result<> IRBuilder::visitSuspend(Suspend* curr) {
  auto tag = wasm.getTag(curr->tag);
  auto sig = tag->sig;
  auto size = sig.params.size();
  curr->operands.resize(size);
  for (size_t i = 0; i < size; ++i) {
    auto val = pop();
    CHECK_ERR(val);
    curr->operands[size - i - 1] = *val;
  }
  return Ok{};
}

Result<> IRBuilder::visitTupleMake(TupleMake* curr) {
  assert(curr->operands.size() >= 2);
  for (size_t i = 0, size = curr->operands.size(); i < size; ++i) {
    auto elem = pop();
    CHECK_ERR(elem);
    curr->operands[size - 1 - i] = *elem;
  }
  return Ok{};
}

Result<> IRBuilder::visitTupleExtract(TupleExtract* curr,
                                      std::optional<uint32_t> arity) {
  if (!arity) {
    if (curr->tuple->type == Type::unreachable) {
      // Fallback to an arbitrary valid arity.
      arity = 2;
    } else {
      arity = curr->tuple->type.size();
    }
  }
  assert(*arity >= 2);
  auto tuple = pop(*arity);
  CHECK_ERR(tuple);
  curr->tuple = *tuple;
  return Ok{};
}

Result<> IRBuilder::visitPop(Pop*) {
  // Do not actually push this pop onto the stack since we generate our own pops
  // as necessary when visiting the beginnings of try blocks.
  return Ok{};
}

Result<> IRBuilder::visitFunctionStart(Function* func) {
  if (!scopeStack.empty()) {
    return Err{"unexpected start of function"};
  }
  if (debugLoc) {
    func->prologLocation.insert(*debugLoc);
    debugLoc.reset();
  }
  scopeStack.push_back(ScopeCtx::makeFunc(func));
  this->func = func;
  return Ok{};
}

Result<> IRBuilder::visitBlockStart(Block* curr) {
  applyDebugLoc(curr);
  pushScope(ScopeCtx::makeBlock(curr));
  return Ok{};
}

Result<> IRBuilder::visitIfStart(If* iff, Name label) {
  applyDebugLoc(iff);
  auto cond = pop();
  CHECK_ERR(cond);
  iff->condition = *cond;
  pushScope(ScopeCtx::makeIf(iff, label));
  return Ok{};
}

Result<> IRBuilder::visitLoopStart(Loop* loop) {
  applyDebugLoc(loop);
  pushScope(ScopeCtx::makeLoop(loop));
  return Ok{};
}

Result<> IRBuilder::visitTryStart(Try* tryy, Name label) {
  applyDebugLoc(tryy);
  // The delegate label will be regenerated if we need it. See
  // `getDelegateLabelName` for details.
  tryy->name = Name();
  pushScope(ScopeCtx::makeTry(tryy, label));
  return Ok{};
}

Result<> IRBuilder::visitTryTableStart(TryTable* trytable, Name label) {
  applyDebugLoc(trytable);
  pushScope(ScopeCtx::makeTryTable(trytable, label));
  return Ok{};
}

Result<Expression*> IRBuilder::finishScope(Block* block) {
  if (debugLoc) {
    DBG(std::cerr << "discarding debugloc " << debugLoc->fileIndex << ":"
                  << debugLoc->lineNumber << ":" << debugLoc->columnNumber
                  << "\n");
  }
  debugLoc.reset();

  if (scopeStack.empty() || scopeStack.back().isNone()) {
    return Err{"unexpected end of scope"};
  }

  auto& scope = scopeStack.back();
  auto type = scope.getResultType();
  if (type.isTuple()) {
    if (scope.unreachable) {
      // We may not have enough concrete values on the stack to construct the
      // full tuple, and if we tried to fill out the beginning of a tuple.make
      // with additional popped `unreachable`s, that could cause a trap to
      // happen before important side effects. Instead, just drop everything on
      // the stack and finish with a single unreachable.
      //
      // TODO: Validate that the available expressions are a correct suffix of
      // the expected type, since this will no longer be caught by normal
      // validation?
      for (auto& expr : scope.exprStack) {
        expr = builder.dropIfConcretelyTyped(expr);
      }
      if (scope.exprStack.back()->type != Type::unreachable) {
        scope.exprStack.push_back(builder.makeUnreachable());
      }
    } else {
      auto hoisted = hoistLastValue();
      CHECK_ERR(hoisted);
      if (!hoisted) {
        return Err{"popping from empty stack"};
      }
      auto hoistedType = scope.exprStack.back()->type;
      if (hoistedType.size() != type.size()) {
        // We cannot propagate the hoisted value directly because it does not
        // have the correct number of elements. Break it up if necessary and
        // construct our returned tuple from parts.
        CHECK_ERR(packageHoistedValue(*hoisted));
        std::vector<Expression*> elems(type.size());
        for (size_t i = 0; i < elems.size(); ++i) {
          auto elem = pop();
          CHECK_ERR(elem);
          elems[elems.size() - 1 - i] = *elem;
        }
        scope.exprStack.push_back(builder.makeTupleMake(std::move(elems)));
      }
    }
  } else if (type.isConcrete()) {
    // If the value is buried in none-typed expressions, we have to bring it to
    // the top.
    auto hoisted = hoistLastValue();
    CHECK_ERR(hoisted);
    if (!hoisted) {
      return Err{"popping from empty stack"};
    }
  }

  Expression* ret = nullptr;
  if (scope.exprStack.size() == 0) {
    // No expressions for this scope, but we need something. If we were given a
    // block, we can empty it out and return it, but otherwise we need a nop.
    if (block) {
      block->list.clear();
      ret = block;
    } else {
      ret = builder.makeNop();
    }
  } else if (scope.exprStack.size() == 1) {
    // We can put our single expression directly into the surrounding scope.
    if (block) {
      block->list.resize(1);
      block->list[0] = scope.exprStack.back();
      ret = block;
    } else {
      ret = scope.exprStack.back();
    }
  } else {
    // More than one expression, so we need a block. Allocate one if we weren't
    // already given one.
    if (block) {
      block->list.set(scope.exprStack);
    } else {
      block = builder.makeBlock(scope.exprStack, type);
    }
    ret = block;
  }

  // If this scope had a label, remove it from the context.
  if (auto label = scope.getOriginalLabel()) {
    labelDepths.at(label).pop_back();
  }

  scopeStack.pop_back();
  return ret;
}

Result<> IRBuilder::visitElse() {
  auto& scope = getScope();
  auto* iff = scope.getIf();
  if (!iff) {
    return Err{"unexpected else"};
  }
  auto originalLabel = scope.getOriginalLabel();
  auto label = scope.label;
  auto expr = finishScope();
  CHECK_ERR(expr);
  iff->ifTrue = *expr;
  pushScope(ScopeCtx::makeElse(iff, originalLabel, label));
  return Ok{};
}

Result<> IRBuilder::visitCatch(Name tag) {
  auto& scope = getScope();
  bool wasTry = true;
  auto* tryy = scope.getTry();
  if (!tryy) {
    wasTry = false;
    tryy = scope.getCatch();
  }
  if (!tryy) {
    return Err{"unexpected catch"};
  }
  auto originalLabel = scope.getOriginalLabel();
  auto label = scope.label;
  auto expr = finishScope();
  size_t index = 0;
  CHECK_ERR(expr);
  if (wasTry) {
    tryy->body = *expr;
  } else {
    index = scope.getIndex();
    if (index > tryy->catchBodies.size()) {
      tryy->catchBodies.resize(index + 1);
    }
    tryy->catchBodies[index] = *expr;
  }
  if (index > tryy->catchTags.size()) {
    tryy->catchTags.resize(index + 1);
  }
  tryy->catchTags[index] = tag;
  pushScope(ScopeCtx::makeCatch(tryy, originalLabel, index, label));
  // Push a pop for the exception payload.
  auto params = wasm.getTag(tag)->sig.params;
  if (params != Type::none) {
    push(builder.makePop(params));
  }
  return Ok{};
}

Result<> IRBuilder::visitCatchAll() {
  auto& scope = getScope();
  bool wasTry = true;
  auto* tryy = scope.getTry();
  if (!tryy) {
    wasTry = false;
    tryy = scope.getCatch();
  }
  if (!tryy) {
    return Err{"unexpected catch"};
  }
  auto originalLabel = scope.getOriginalLabel();
  auto label = scope.label;
  auto expr = finishScope();
  CHECK_ERR(expr);
  if (wasTry) {
    tryy->body = *expr;
  } else {
    size_t lastTagIdx = tryy->catchTags.size();
    if (lastTagIdx > tryy->catchBodies.size()) {
      tryy->catchBodies.resize(lastTagIdx + 1);
    }
    tryy->catchBodies[lastTagIdx] = *expr;
  }
  pushScope(ScopeCtx::makeCatchAll(tryy, originalLabel, label));
  return Ok{};
}

Result<Name> IRBuilder::getDelegateLabelName(Index label) {
  if (label >= scopeStack.size()) {
    return Err{"invalid label: " + std::to_string(label)};
  }
  auto& scope = scopeStack[scopeStack.size() - label - 1];
  auto* delegateTry = scope.getTry();
  if (!delegateTry) {
    delegateTry = scope.getCatch();
  }
  if (!delegateTry) {
    delegateTry = scope.getCatchAll();
  }
  if (!delegateTry) {
    return Err{"expected try scope at label " + std::to_string(label)};
  }
  // Only delegate and rethrow can reference the try name in Binaryen IR, so
  // trys might need two labels: one for delegate/rethrow and one for all
  // other control flow. These labels must be different to satisfy the
  // Binaryen validator. To keep this complexity contained within the
  // handling of trys and delegates, pretend there is just the single normal
  // label and add a prefix to it to generate the delegate label.
  auto delegateName =
    Name(std::string("__delegate__") + getLabelName(label)->toString());
  delegateTry->name = delegateName;
  return delegateName;
}

Result<> IRBuilder::visitDelegate(Index label) {
  auto& scope = getScope();
  auto* tryy = scope.getTry();
  if (!tryy) {
    return Err{"unexpected delegate"};
  }
  // In Binaryen IR, delegates can only target try or function scopes directly.
  // Search upward to find the nearest enclosing try or function scope. Since
  // the given label is relative the parent scope of the try, start by adjusting
  // it to be relative to the try scope.
  ++label;
  for (size_t size = scopeStack.size(); label < size; ++label) {
    auto& delegateScope = scopeStack[size - label - 1];
    if (delegateScope.getTry()) {
      auto delegateName = getDelegateLabelName(label);
      CHECK_ERR(delegateName);
      tryy->delegateTarget = *delegateName;
      break;
    } else if (delegateScope.getFunction()) {
      tryy->delegateTarget = DELEGATE_CALLER_TARGET;
      break;
    }
  }
  if (label == scopeStack.size()) {
    return Err{"unexpected delegate"};
  }
  // Delegate ends the try.
  return visitEnd();
}

Result<> IRBuilder::visitEnd() {
  auto scope = getScope();
  if (scope.isNone()) {
    return Err{"unexpected end"};
  }
  if (auto* func = scope.getFunction(); func && debugLoc) {
    func->epilogLocation.insert(*debugLoc);
    debugLoc.reset();
  }
  auto expr = finishScope(scope.getBlock());
  CHECK_ERR(expr);

  // If the scope expression cannot be directly labeled, we may need to wrap it
  // in a block. It's possible that the scope expression becomes typed
  // unreachable when it is finalized, but if the wrapper block is targeted by
  // any branches, the target block needs to have the original non-unreachable
  // type of the scope expression.
  auto originalScopeType = scope.getResultType();
  auto maybeWrapForLabel = [&](Expression* curr) -> Expression* {
    if (scope.label) {
      return builder.makeBlock(scope.label,
                               {curr},
                               scope.labelUsed ? originalScopeType
                                               : scope.getResultType());
    }
    return curr;
  };

  if (auto* func = scope.getFunction()) {
    func->body = maybeWrapForLabel(*expr);
    labelDepths.clear();
  } else if (auto* block = scope.getBlock()) {
    assert(*expr == block);
    block->name = scope.label;
    // TODO: Track branches so we can know whether this block is a target and
    // finalize more efficiently.
    block->finalize(block->type);
    push(block);
  } else if (auto* loop = scope.getLoop()) {
    loop->body = *expr;
    loop->name = scope.label;
    loop->finalize(loop->type);
    push(loop);
  } else if (auto* iff = scope.getIf()) {
    iff->ifTrue = *expr;
    iff->ifFalse = nullptr;
    iff->finalize(iff->type);
    push(maybeWrapForLabel(iff));
  } else if (auto* iff = scope.getElse()) {
    iff->ifFalse = *expr;
    iff->finalize(iff->type);
    push(maybeWrapForLabel(iff));
  } else if (auto* tryy = scope.getTry()) {
    tryy->body = *expr;
    tryy->finalize(tryy->type);
    push(maybeWrapForLabel(tryy));
  } else if (Try * tryy; (tryy = scope.getCatch())) {
    auto index = scope.getIndex();
    if (index > tryy->catchBodies.size()) {
      tryy->catchBodies.resize(index + 1);
    }
    tryy->catchBodies[index] = *expr;
    tryy->finalize(tryy->type);
    push(maybeWrapForLabel(tryy));
  } else if (Try * tryy; (tryy = scope.getCatchAll())) {
    auto index = tryy->catchTags.size();
    if (index > tryy->catchBodies.size()) {
      tryy->catchBodies.resize(index + 1);
    }
    tryy->catchBodies[index] = *expr;
    tryy->finalize(tryy->type);
    push(maybeWrapForLabel(tryy));
  } else if (auto* trytable = scope.getTryTable()) {
    trytable->body = *expr;
    trytable->finalize(trytable->type, &wasm);
    push(maybeWrapForLabel(trytable));
  } else {
    WASM_UNREACHABLE("unexpected scope kind");
  }
  return Ok{};
}

Result<Index> IRBuilder::getLabelIndex(Name label, bool inDelegate) {
  auto it = labelDepths.find(label);
  if (it == labelDepths.end() || it->second.empty()) {
    return Err{"unexpected label '"s + label.toString() + "'"};
  }
  auto index = scopeStack.size() - it->second.back();
  if (inDelegate) {
    if (index == 0) {
      // The real label we're referencing, if it exists, has been shadowed by
      // the `try`. Get the previous label with this name instead. For example:
      //
      // block $l
      //  try $l
      //  delegate $l
      // end
      //
      // The `delegate $l` should target the block, not the try, even though a
      // normal branch to $l in the try's scope would target the try.
      if (it->second.size() <= 1) {
        return Err{"unexpected self-referencing label '"s + label.toString() +
                   "'"};
      }
      index = scopeStack.size() - it->second[it->second.size() - 2];
      assert(index != 0);
    }
    // Adjust the index to be relative to the try.
    --index;
  }
  return index;
}

Result<Name> IRBuilder::getLabelName(Index label) {
  auto scope = getScope(label);
  CHECK_ERR(scope);
  auto& scopeLabel = (*scope)->label;
  if (!scopeLabel) {
    // The scope does not already have a name, so we need to create one.
    scopeLabel = makeFresh("label");
  }
  (*scope)->labelUsed = true;
  return scopeLabel;
}

Result<> IRBuilder::makeNop() {
  push(builder.makeNop());
  return Ok{};
}

Result<> IRBuilder::makeBlock(Name label, Type type) {
  auto* block = wasm.allocator.alloc<Block>();
  block->name = label;
  block->type = type;
  return visitBlockStart(block);
}

Result<> IRBuilder::makeIf(Name label, Type type) {
  auto* iff = wasm.allocator.alloc<If>();
  iff->type = type;
  return visitIfStart(iff, label);
}

Result<> IRBuilder::makeLoop(Name label, Type type) {
  auto* loop = wasm.allocator.alloc<Loop>();
  loop->name = label;
  loop->type = type;
  return visitLoopStart(loop);
}

Result<> IRBuilder::makeBreak(Index label, bool isConditional) {
  auto name = getLabelName(label);
  CHECK_ERR(name);
  Break curr;
  curr.name = *name;
  // Use a dummy condition value if we need to pop a condition.
  curr.condition = isConditional ? &curr : nullptr;
  CHECK_ERR(visitBreak(&curr, label));
  push(builder.makeBreak(curr.name, curr.value, curr.condition));
  return Ok{};
}

Result<> IRBuilder::makeSwitch(const std::vector<Index>& labels,
                               Index defaultLabel) {
  std::vector<Name> names;
  names.reserve(labels.size());
  for (auto label : labels) {
    auto name = getLabelName(label);
    CHECK_ERR(name);
    names.push_back(*name);
  }
  auto defaultName = getLabelName(defaultLabel);
  CHECK_ERR(defaultName);
  Switch curr(wasm.allocator);
  CHECK_ERR(visitSwitch(&curr, defaultLabel));
  push(builder.makeSwitch(names, *defaultName, curr.condition, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeCall(Name func, bool isReturn) {
  Call curr(wasm.allocator);
  curr.target = func;
  CHECK_ERR(visitCall(&curr));
  auto type = wasm.getFunction(func)->getResults();
  push(builder.makeCall(curr.target, curr.operands, type, isReturn));
  return Ok{};
}

Result<> IRBuilder::makeCallIndirect(Name table, HeapType type, bool isReturn) {
  CallIndirect curr(wasm.allocator);
  curr.heapType = type;
  CHECK_ERR(visitCallIndirect(&curr));
  push(builder.makeCallIndirect(
    table, curr.target, curr.operands, type, isReturn));
  return Ok{};
}

Result<> IRBuilder::makeLocalGet(Index local) {
  push(builder.makeLocalGet(local, func->getLocalType(local)));
  return Ok{};
}

Result<> IRBuilder::makeLocalSet(Index local) {
  LocalSet curr;
  curr.index = local;
  CHECK_ERR(visitLocalSet(&curr));
  push(builder.makeLocalSet(local, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeLocalTee(Index local) {
  LocalSet curr;
  curr.index = local;
  CHECK_ERR(visitLocalSet(&curr));
  push(builder.makeLocalTee(local, curr.value, func->getLocalType(local)));
  return Ok{};
}

Result<> IRBuilder::makeGlobalGet(Name global) {
  push(builder.makeGlobalGet(global, wasm.getGlobal(global)->type));
  return Ok{};
}

Result<> IRBuilder::makeGlobalSet(Name global) {
  GlobalSet curr;
  curr.name = global;
  CHECK_ERR(visitGlobalSet(&curr));
  push(builder.makeGlobalSet(global, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeLoad(unsigned bytes,
                             bool signed_,
                             Address offset,
                             unsigned align,
                             Type type,
                             Name mem) {
  Load curr;
  CHECK_ERR(visitLoad(&curr));
  push(builder.makeLoad(bytes, signed_, offset, align, curr.ptr, type, mem));
  return Ok{};
}

Result<> IRBuilder::makeStore(
  unsigned bytes, Address offset, unsigned align, Type type, Name mem) {
  Store curr;
  CHECK_ERR(visitStore(&curr));
  push(
    builder.makeStore(bytes, offset, align, curr.ptr, curr.value, type, mem));
  return Ok{};
}

Result<>
IRBuilder::makeAtomicLoad(unsigned bytes, Address offset, Type type, Name mem) {
  Load curr;
  CHECK_ERR(visitLoad(&curr));
  push(builder.makeAtomicLoad(bytes, offset, curr.ptr, type, mem));
  return Ok{};
}

Result<> IRBuilder::makeAtomicStore(unsigned bytes,
                                    Address offset,
                                    Type type,
                                    Name mem) {
  Store curr;
  CHECK_ERR(visitStore(&curr));
  push(builder.makeAtomicStore(bytes, offset, curr.ptr, curr.value, type, mem));
  return Ok{};
}

Result<> IRBuilder::makeAtomicRMW(
  AtomicRMWOp op, unsigned bytes, Address offset, Type type, Name mem) {
  AtomicRMW curr;
  CHECK_ERR(visitAtomicRMW(&curr));
  push(
    builder.makeAtomicRMW(op, bytes, offset, curr.ptr, curr.value, type, mem));
  return Ok{};
}

Result<> IRBuilder::makeAtomicCmpxchg(unsigned bytes,
                                      Address offset,
                                      Type type,
                                      Name mem) {
  AtomicCmpxchg curr;
  CHECK_ERR(visitAtomicCmpxchg(&curr));
  push(builder.makeAtomicCmpxchg(
    bytes, offset, curr.ptr, curr.expected, curr.replacement, type, mem));
  return Ok{};
}

Result<> IRBuilder::makeAtomicWait(Type type, Address offset, Name mem) {
  AtomicWait curr;
  CHECK_ERR(visitAtomicWait(&curr));
  push(builder.makeAtomicWait(
    curr.ptr, curr.expected, curr.timeout, type, offset, mem));
  return Ok{};
}

Result<> IRBuilder::makeAtomicNotify(Address offset, Name mem) {
  AtomicNotify curr;
  CHECK_ERR(visitAtomicNotify(&curr));
  push(builder.makeAtomicNotify(curr.ptr, curr.notifyCount, offset, mem));
  return Ok{};
}

Result<> IRBuilder::makeAtomicFence() {
  push(builder.makeAtomicFence());
  return Ok{};
}

Result<> IRBuilder::makeSIMDExtract(SIMDExtractOp op, uint8_t lane) {
  SIMDExtract curr;
  CHECK_ERR(visitSIMDExtract(&curr));
  push(builder.makeSIMDExtract(op, curr.vec, lane));
  return Ok{};
}

Result<> IRBuilder::makeSIMDReplace(SIMDReplaceOp op, uint8_t lane) {
  SIMDReplace curr;
  CHECK_ERR(visitSIMDReplace(&curr));
  push(builder.makeSIMDReplace(op, curr.vec, lane, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeSIMDShuffle(const std::array<uint8_t, 16>& lanes) {
  SIMDShuffle curr;
  CHECK_ERR(visitSIMDShuffle(&curr));
  push(builder.makeSIMDShuffle(curr.left, curr.right, lanes));
  return Ok{};
}

Result<> IRBuilder::makeSIMDTernary(SIMDTernaryOp op) {
  SIMDTernary curr;
  CHECK_ERR(visitSIMDTernary(&curr));
  push(builder.makeSIMDTernary(op, curr.a, curr.b, curr.c));
  return Ok{};
}

Result<> IRBuilder::makeSIMDShift(SIMDShiftOp op) {
  SIMDShift curr;
  CHECK_ERR(visitSIMDShift(&curr));
  push(builder.makeSIMDShift(op, curr.vec, curr.shift));
  return Ok{};
}

Result<> IRBuilder::makeSIMDLoad(SIMDLoadOp op,
                                 Address offset,
                                 unsigned align,
                                 Name mem) {
  SIMDLoad curr;
  CHECK_ERR(visitSIMDLoad(&curr));
  push(builder.makeSIMDLoad(op, offset, align, curr.ptr, mem));
  return Ok{};
}

Result<> IRBuilder::makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp op,
                                          Address offset,
                                          unsigned align,
                                          uint8_t lane,
                                          Name mem) {
  SIMDLoadStoreLane curr;
  CHECK_ERR(visitSIMDLoadStoreLane(&curr));
  push(builder.makeSIMDLoadStoreLane(
    op, offset, align, lane, curr.ptr, curr.vec, mem));
  return Ok{};
}

Result<> IRBuilder::makeMemoryInit(Name data, Name mem) {
  MemoryInit curr;
  CHECK_ERR(visitMemoryInit(&curr));
  push(builder.makeMemoryInit(data, curr.dest, curr.offset, curr.size, mem));
  return Ok{};
}

Result<> IRBuilder::makeDataDrop(Name data) {
  push(builder.makeDataDrop(data));
  return Ok{};
}

Result<> IRBuilder::makeMemoryCopy(Name destMem, Name srcMem) {
  MemoryCopy curr;
  CHECK_ERR(visitMemoryCopy(&curr));
  push(
    builder.makeMemoryCopy(curr.dest, curr.source, curr.size, destMem, srcMem));
  return Ok{};
}

Result<> IRBuilder::makeMemoryFill(Name mem) {
  MemoryFill curr;
  CHECK_ERR(visitMemoryFill(&curr));
  push(builder.makeMemoryFill(curr.dest, curr.value, curr.size, mem));
  return Ok{};
}

Result<> IRBuilder::makeConst(Literal val) {
  push(builder.makeConst(val));
  return Ok{};
}

Result<> IRBuilder::makeUnary(UnaryOp op) {
  Unary curr;
  CHECK_ERR(visitUnary(&curr));
  push(builder.makeUnary(op, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeBinary(BinaryOp op) {
  Binary curr;
  CHECK_ERR(visitBinary(&curr));
  push(builder.makeBinary(op, curr.left, curr.right));
  return Ok{};
}

Result<> IRBuilder::makeSelect(std::optional<Type> type) {
  Select curr;
  CHECK_ERR(visitSelect(&curr));
  auto* built =
    type ? builder.makeSelect(curr.condition, curr.ifTrue, curr.ifFalse, *type)
         : builder.makeSelect(curr.condition, curr.ifTrue, curr.ifFalse);
  if (type && !Type::isSubType(built->type, *type)) {
    return Err{"select type does not match expected type"};
  }
  push(built);
  return Ok{};
}

Result<> IRBuilder::makeDrop() {
  Drop curr;
  CHECK_ERR(visitDrop(&curr, 1));
  push(builder.makeDrop(curr.value));
  return Ok{};
}

Result<> IRBuilder::makeReturn() {
  Return curr;
  CHECK_ERR(visitReturn(&curr));
  push(builder.makeReturn(curr.value));
  return Ok{};
}

Result<> IRBuilder::makeMemorySize(Name mem) {
  push(builder.makeMemorySize(mem));
  return Ok{};
}

Result<> IRBuilder::makeMemoryGrow(Name mem) {
  MemoryGrow curr;
  CHECK_ERR(visitMemoryGrow(&curr));
  push(builder.makeMemoryGrow(curr.delta, mem));
  return Ok{};
}

Result<> IRBuilder::makeUnreachable() {
  push(builder.makeUnreachable());
  return Ok{};
}

Result<> IRBuilder::makePop(Type type) {
  // We don't actually want to create a new Pop expression here because we
  // already create them automatically when starting a legacy catch block that
  // needs one. Just verify that the Pop we are being asked to make is the same
  // type as the Pop we have already made.
  auto& scope = getScope();
  if (!scope.getCatch() || scope.exprStack.size() != 1 ||
      !scope.exprStack[0]->is<Pop>()) {
    return Err{
      "pop instructions may only appear at the beginning of catch blocks"};
  }
  auto expectedType = scope.exprStack[0]->type;
  if (!Type::isSubType(expectedType, type)) {
    return Err{std::string("Expected pop of type ") + expectedType.toString()};
  }
  return Ok{};
}

Result<> IRBuilder::makeRefNull(HeapType type) {
  push(builder.makeRefNull(type));
  return Ok{};
}

Result<> IRBuilder::makeRefIsNull() {
  RefIsNull curr;
  CHECK_ERR(visitRefIsNull(&curr));
  push(builder.makeRefIsNull(curr.value));
  return Ok{};
}

Result<> IRBuilder::makeRefFunc(Name func) {
  push(builder.makeRefFunc(func, wasm.getFunction(func)->type));
  return Ok{};
}

Result<> IRBuilder::makeRefEq() {
  RefEq curr;
  CHECK_ERR(visitRefEq(&curr));
  push(builder.makeRefEq(curr.left, curr.right));
  return Ok{};
}

Result<> IRBuilder::makeTableGet(Name table) {
  TableGet curr;
  CHECK_ERR(visitTableGet(&curr));
  auto type = wasm.getTable(table)->type;
  push(builder.makeTableGet(table, curr.index, type));
  return Ok{};
}

Result<> IRBuilder::makeTableSet(Name table) {
  TableSet curr;
  CHECK_ERR(visitTableSet(&curr));
  push(builder.makeTableSet(table, curr.index, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeTableSize(Name table) {
  push(builder.makeTableSize(table));
  return Ok{};
}

Result<> IRBuilder::makeTableGrow(Name table) {
  TableGrow curr;
  CHECK_ERR(visitTableGrow(&curr));
  push(builder.makeTableGrow(table, curr.value, curr.delta));
  return Ok{};
}

Result<> IRBuilder::makeTableFill(Name table) {
  TableFill curr;
  CHECK_ERR(visitTableFill(&curr));
  push(builder.makeTableFill(table, curr.dest, curr.value, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeTableCopy(Name destTable, Name srcTable) {
  TableCopy curr;
  CHECK_ERR(visitTableCopy(&curr));
  push(builder.makeTableCopy(
    curr.dest, curr.source, curr.size, destTable, srcTable));
  return Ok{};
}

Result<> IRBuilder::makeTry(Name label, Type type) {
  auto* tryy = wasm.allocator.alloc<Try>();
  tryy->type = type;
  return visitTryStart(tryy, label);
}

Result<> IRBuilder::makeTryTable(Name label,
                                 Type type,
                                 const std::vector<Name>& tags,
                                 const std::vector<Index>& labels,
                                 const std::vector<bool>& isRefs) {
  auto* trytable = wasm.allocator.alloc<TryTable>();
  trytable->type = type;
  trytable->catchTags.set(tags);
  trytable->catchRefs.set(isRefs);
  trytable->catchDests.reserve(labels.size());
  for (auto label : labels) {
    auto name = getLabelName(label);
    CHECK_ERR(name);
    trytable->catchDests.push_back(*name);
  }
  return visitTryTableStart(trytable, label);
}

Result<> IRBuilder::makeThrow(Name tag) {
  Throw curr(wasm.allocator);
  curr.tag = tag;
  CHECK_ERR(visitThrow(&curr));
  push(builder.makeThrow(tag, curr.operands));
  return Ok{};
}

Result<> IRBuilder::makeRethrow(Index label) {
  // Rethrow references `Try` labels directly, just like `delegate`.
  auto name = getDelegateLabelName(label);
  CHECK_ERR(name);
  push(builder.makeRethrow(*name));
  return Ok{};
}

Result<> IRBuilder::makeThrowRef() {
  ThrowRef curr;
  CHECK_ERR(visitThrowRef(&curr));
  push(builder.makeThrowRef(curr.exnref));
  return Ok{};
}

Result<> IRBuilder::makeTupleMake(uint32_t arity) {
  if (arity < 2) {
    return Err{"tuple arity must be at least 2"};
  }
  TupleMake curr(wasm.allocator);
  curr.operands.resize(arity);
  CHECK_ERR(visitTupleMake(&curr));
  push(builder.makeTupleMake(curr.operands));
  return Ok{};
}

Result<> IRBuilder::makeTupleExtract(uint32_t arity, uint32_t index) {
  if (index >= arity) {
    return Err{"tuple index out of bounds"};
  }
  if (arity < 2) {
    return Err{"tuple arity must be at least 2"};
  }
  TupleExtract curr;
  CHECK_ERR(visitTupleExtract(&curr, arity));
  push(builder.makeTupleExtract(curr.tuple, index));
  return Ok{};
}

Result<> IRBuilder::makeTupleDrop(uint32_t arity) {
  if (arity < 2) {
    return Err{"tuple arity must be at least 2"};
  }
  Drop curr;
  CHECK_ERR(visitDrop(&curr, arity));
  push(builder.makeDrop(curr.value));
  return Ok{};
}

Result<> IRBuilder::makeRefI31() {
  RefI31 curr;
  CHECK_ERR(visitRefI31(&curr));
  push(builder.makeRefI31(curr.value));
  return Ok{};
}

Result<> IRBuilder::makeI31Get(bool signed_) {
  I31Get curr;
  CHECK_ERR(visitI31Get(&curr));
  push(builder.makeI31Get(curr.i31, signed_));
  return Ok{};
}

Result<> IRBuilder::makeCallRef(HeapType type, bool isReturn) {
  CallRef curr(wasm.allocator);
  if (!type.isSignature()) {
    return Err{"expected function type"};
  }
  auto sig = type.getSignature();
  curr.operands.resize(type.getSignature().params.size());
  CHECK_ERR(visitCallRef(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.target));
  push(builder.makeCallRef(curr.target, curr.operands, sig.results, isReturn));
  return Ok{};
}

Result<> IRBuilder::makeRefTest(Type type) {
  RefTest curr;
  CHECK_ERR(visitRefTest(&curr));
  push(builder.makeRefTest(curr.ref, type));
  return Ok{};
}

Result<> IRBuilder::makeRefCast(Type type) {
  RefCast curr;
  CHECK_ERR(visitRefCast(&curr));
  push(builder.makeRefCast(curr.ref, type));
  return Ok{};
}

Result<> IRBuilder::makeBrOn(Index label, BrOnOp op, Type in, Type out) {
  BrOn curr;
  CHECK_ERR(visitBrOn(&curr));
  if (out != Type::none) {
    if (!Type::isSubType(out, in)) {
      return Err{"output type is not a subtype of the input type"};
    }
    if (!Type::isSubType(curr.ref->type, in)) {
      return Err{"expected input to match input type annotation"};
    }
  }
  auto name = getLabelName(label);
  CHECK_ERR(name);
  push(builder.makeBrOn(op, *name, curr.ref, out));
  return Ok{};
}

Result<> IRBuilder::makeStructNew(HeapType type) {
  StructNew curr(wasm.allocator);
  // Differentiate from struct.new_default with a non-empty expression list.
  curr.operands.resize(type.getStruct().fields.size());
  CHECK_ERR(visitStructNew(&curr));
  push(builder.makeStructNew(type, std::move(curr.operands)));
  return Ok{};
}

Result<> IRBuilder::makeStructNewDefault(HeapType type) {
  push(builder.makeStructNew(type, {}));
  return Ok{};
}

Result<> IRBuilder::makeStructGet(HeapType type, Index field, bool signed_) {
  const auto& fields = type.getStruct().fields;
  StructGet curr;
  CHECK_ERR(visitStructGet(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeStructGet(field, curr.ref, fields[field].type, signed_));
  return Ok{};
}

Result<> IRBuilder::makeStructSet(HeapType type, Index field) {
  StructSet curr;
  CHECK_ERR(visitStructSet(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeStructSet(field, curr.ref, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeArrayNew(HeapType type) {
  ArrayNew curr;
  // Differentiate from array.new_default with dummy initializer.
  curr.init = (Expression*)0x01;
  CHECK_ERR(visitArrayNew(&curr));
  push(builder.makeArrayNew(type, curr.size, curr.init));
  return Ok{};
}

Result<> IRBuilder::makeArrayNewDefault(HeapType type) {
  ArrayNew curr;
  CHECK_ERR(visitArrayNew(&curr));
  push(builder.makeArrayNew(type, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeArrayNewData(HeapType type, Name data) {
  ArrayNewData curr;
  CHECK_ERR(visitArrayNewData(&curr));
  push(builder.makeArrayNewData(type, data, curr.offset, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeArrayNewElem(HeapType type, Name elem) {
  ArrayNewElem curr;
  CHECK_ERR(visitArrayNewElem(&curr));
  push(builder.makeArrayNewElem(type, elem, curr.offset, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeArrayNewFixed(HeapType type, uint32_t arity) {
  ArrayNewFixed curr(wasm.allocator);
  curr.values.resize(arity);
  CHECK_ERR(visitArrayNewFixed(&curr));
  push(builder.makeArrayNewFixed(type, curr.values));
  return Ok{};
}

Result<> IRBuilder::makeArrayGet(HeapType type, bool signed_) {
  ArrayGet curr;
  CHECK_ERR(visitArrayGet(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeArrayGet(
    curr.ref, curr.index, type.getArray().element.type, signed_));
  return Ok{};
}

Result<> IRBuilder::makeArraySet(HeapType type) {
  ArraySet curr;
  CHECK_ERR(visitArraySet(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeArraySet(curr.ref, curr.index, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeArrayLen() {
  ArrayLen curr;
  CHECK_ERR(visitArrayLen(&curr));
  push(builder.makeArrayLen(curr.ref));
  return Ok{};
}

Result<> IRBuilder::makeArrayCopy(HeapType destType, HeapType srcType) {
  ArrayCopy curr;
  CHECK_ERR(visitArrayCopy(&curr));
  CHECK_ERR(validateTypeAnnotation(destType, curr.destRef));
  CHECK_ERR(validateTypeAnnotation(srcType, curr.srcRef));
  push(builder.makeArrayCopy(
    curr.destRef, curr.destIndex, curr.srcRef, curr.srcIndex, curr.length));
  return Ok{};
}

Result<> IRBuilder::makeArrayFill(HeapType type) {
  ArrayFill curr;
  CHECK_ERR(visitArrayFill(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeArrayFill(curr.ref, curr.index, curr.value, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeArrayInitData(HeapType type, Name data) {
  ArrayInitData curr;
  CHECK_ERR(visitArrayInitData(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeArrayInitData(
    data, curr.ref, curr.index, curr.offset, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeArrayInitElem(HeapType type, Name elem) {
  ArrayInitElem curr;
  CHECK_ERR(visitArrayInitElem(&curr));
  CHECK_ERR(validateTypeAnnotation(type, curr.ref));
  push(builder.makeArrayInitElem(
    elem, curr.ref, curr.index, curr.offset, curr.size));
  return Ok{};
}

Result<> IRBuilder::makeRefAs(RefAsOp op) {
  RefAs curr;
  CHECK_ERR(visitRefAs(&curr));
  push(builder.makeRefAs(op, curr.value));
  return Ok{};
}

Result<> IRBuilder::makeStringNew(StringNewOp op, bool try_, Name mem) {
  StringNew curr;
  curr.op = op;
  CHECK_ERR(visitStringNew(&curr));
  // TODO: Store the memory in the IR.
  switch (op) {
    case StringNewUTF8:
    case StringNewWTF8:
    case StringNewLossyUTF8:
    case StringNewWTF16:
      push(builder.makeStringNew(op, curr.ptr, curr.length, try_));
      return Ok{};
    case StringNewUTF8Array:
    case StringNewWTF8Array:
    case StringNewLossyUTF8Array:
    case StringNewWTF16Array:
      push(builder.makeStringNew(op, curr.ptr, curr.start, curr.end, try_));
      return Ok{};
    case StringNewFromCodePoint:
      push(builder.makeStringNew(op, curr.ptr, nullptr, try_));
      return Ok{};
  }
  WASM_UNREACHABLE("unexpected op");
}

Result<> IRBuilder::makeStringConst(Name string) {
  push(builder.makeStringConst(string));
  return Ok{};
}

Result<> IRBuilder::makeStringMeasure(StringMeasureOp op) {
  StringMeasure curr;
  CHECK_ERR(visitStringMeasure(&curr));
  push(builder.makeStringMeasure(op, curr.ref));
  return Ok{};
}

Result<> IRBuilder::makeStringEncode(StringEncodeOp op, Name mem) {
  StringEncode curr;
  curr.op = op;
  CHECK_ERR(visitStringEncode(&curr));
  // TODO: Store the memory in the IR.
  push(builder.makeStringEncode(op, curr.ref, curr.ptr, curr.start));
  return Ok{};
}

Result<> IRBuilder::makeStringConcat() {
  StringConcat curr;
  CHECK_ERR(visitStringConcat(&curr));
  push(builder.makeStringConcat(curr.left, curr.right));
  return Ok{};
}

Result<> IRBuilder::makeStringEq(StringEqOp op) {
  StringEq curr;
  CHECK_ERR(visitStringEq(&curr));
  push(builder.makeStringEq(op, curr.left, curr.right));
  return Ok{};
}

Result<> IRBuilder::makeStringAs(StringAsOp op) {
  StringAs curr;
  CHECK_ERR(visitStringAs(&curr));
  push(builder.makeStringAs(op, curr.ref));
  return Ok{};
}

Result<> IRBuilder::makeStringWTF8Advance() {
  StringWTF8Advance curr;
  CHECK_ERR(visitStringWTF8Advance(&curr));
  push(builder.makeStringWTF8Advance(curr.ref, curr.pos, curr.bytes));
  return Ok{};
}

Result<> IRBuilder::makeStringWTF16Get() {
  StringWTF16Get curr;
  CHECK_ERR(visitStringWTF16Get(&curr));
  push(builder.makeStringWTF16Get(curr.ref, curr.pos));
  return Ok{};
}

Result<> IRBuilder::makeStringIterNext() {
  StringIterNext curr;
  CHECK_ERR(visitStringIterNext(&curr));
  push(builder.makeStringIterNext(curr.ref));
  return Ok{};
}

Result<> IRBuilder::makeStringIterMove(StringIterMoveOp op) {
  StringIterMove curr;
  CHECK_ERR(visitStringIterMove(&curr));
  push(builder.makeStringIterMove(op, curr.ref, curr.num));
  return Ok{};
}

Result<> IRBuilder::makeStringSliceWTF(StringSliceWTFOp op) {
  StringSliceWTF curr;
  CHECK_ERR(visitStringSliceWTF(&curr));
  push(builder.makeStringSliceWTF(op, curr.ref, curr.start, curr.end));
  return Ok{};
}

Result<> IRBuilder::makeStringSliceIter() {
  StringSliceIter curr;
  CHECK_ERR(visitStringSliceIter(&curr));
  push(builder.makeStringSliceIter(curr.ref, curr.num));
  return Ok{};
}

Result<> IRBuilder::makeContBind(HeapType contTypeBefore,
                                 HeapType contTypeAfter) {
  if (!contTypeBefore.isContinuation() || !contTypeAfter.isContinuation()) {
    return Err{"expected continuation types"};
  }
  ContBind curr(wasm.allocator);
  curr.contTypeBefore = contTypeBefore;
  curr.contTypeAfter = contTypeAfter;
  CHECK_ERR(visitContBind(&curr));

  std::vector<Expression*> operands(curr.operands.begin(), curr.operands.end());
  push(
    builder.makeContBind(contTypeBefore, contTypeAfter, operands, curr.cont));
  return Ok{};
}

Result<> IRBuilder::makeContNew(HeapType ct) {
  if (!ct.isContinuation()) {
    return Err{"expected continuation type"};
  }
  ContNew curr;
  CHECK_ERR(visitContNew(&curr));

  push(builder.makeContNew(ct, curr.func));
  return Ok{};
}

Result<> IRBuilder::makeResume(HeapType ct,
                               const std::vector<Name>& tags,
                               const std::vector<Index>& labels) {
  if (!ct.isContinuation()) {
    return Err{"expected continuation type"};
  }
  Resume curr(wasm.allocator);
  curr.contType = ct;
  CHECK_ERR(visitResume(&curr));

  std::vector<Name> labelNames;
  labelNames.reserve(labels.size());
  for (auto label : labels) {
    auto name = getLabelName(label);
    CHECK_ERR(name);
    labelNames.push_back(*name);
  }
  std::vector<Expression*> operands(curr.operands.begin(), curr.operands.end());
  push(builder.makeResume(ct, tags, labelNames, operands, curr.cont));
  return Ok{};
}

Result<> IRBuilder::makeSuspend(Name tag) {
  Suspend curr(wasm.allocator);
  curr.tag = tag;
  CHECK_ERR(visitSuspend(&curr));

  std::vector<Expression*> operands(curr.operands.begin(), curr.operands.end());
  push(builder.makeSuspend(tag, operands));
  return Ok{};
}

} // namespace wasm
