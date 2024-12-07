/*
 * Copyright 2017 WebAssembly Community Group participants
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

#include <mutex>
#include <set>
#include <sstream>
#include <unordered_set>

#include "ir/eh-utils.h"
#include "ir/features.h"
#include "ir/find_all.h"
#include "ir/gc-type-utils.h"
#include "ir/global-utils.h"
#include "ir/intrinsics.h"
#include "ir/local-graph.h"
#include "ir/local-structural-dominance.h"
#include "ir/module-utils.h"
#include "ir/stack-utils.h"
#include "ir/utils.h"
#include "support/colors.h"
#include "wasm-validator.h"
#include "wasm.h"

namespace wasm {

// Print anything that can be streamed to an ostream
template<typename T,
         typename std::enable_if<!std::is_base_of<
           Expression,
           typename std::remove_pointer<T>::type>::value>::type* = nullptr>
inline std::ostream&
printModuleComponent(T curr, std::ostream& stream, Module& wasm) {
  stream << curr << std::endl;
  return stream;
}

// Extra overload for Expressions, to print their contents.
inline std::ostream&
printModuleComponent(Expression* curr, std::ostream& stream, Module& wasm) {
  if (curr) {
    stream << ModuleExpression(wasm, curr) << '\n';
  }
  return stream;
}

// For parallel validation, we have a helper struct for coordination
struct ValidationInfo {
  Module& wasm;

  bool validateWeb;
  bool validateGlobally;
  bool quiet;

  std::atomic<bool> valid;

  // a stream of error test for each function. we print in the right order at
  // the end, for deterministic output
  // note errors are rare/unexpected, so it's ok to use a slow mutex here
  std::mutex mutex;
  std::unordered_map<Function*, std::unique_ptr<std::ostringstream>> outputs;

  ValidationInfo(Module& wasm) : wasm(wasm) { valid.store(true); }

  std::ostringstream& getStream(Function* func) {
    std::unique_lock<std::mutex> lock(mutex);
    auto iter = outputs.find(func);
    if (iter != outputs.end()) {
      return *(iter->second.get());
    }
    auto& ret = outputs[func] = std::make_unique<std::ostringstream>();
    return *ret.get();
  }

  // printing and error handling support

  template<typename T, typename S>
  std::ostream& fail(S text, T curr, Function* func) {
    valid.store(false);
    auto& stream = getStream(func);
    if (quiet) {
      return stream;
    }
    auto& ret = printFailureHeader(func);
    ret << text << ", on \n";
    return printModuleComponent(curr, ret, wasm);
  }

  std::ostream& printFailureHeader(Function* func) {
    auto& stream = getStream(func);
    if (quiet) {
      return stream;
    }
    Colors::red(stream);
    if (func) {
      stream << "[wasm-validator error in function ";
      Colors::green(stream);
      stream << func->name;
      Colors::red(stream);
      stream << "] ";
    } else {
      stream << "[wasm-validator error in module] ";
    }
    Colors::normal(stream);
    return stream;
  }

  // Checking utilities.

  // Returns whether the result was in fact true.
  template<typename T>
  bool shouldBeTrue(bool result,
                    T curr,
                    const char* text,
                    Function* func = nullptr) {
    if (!result) {
      fail("unexpected false: " + std::string(text), curr, func);
      return false;
    }
    return true;
  }

  // Returns whether the result was in fact false.
  template<typename T>
  bool shouldBeFalse(bool result,
                     T curr,
                     const char* text,
                     Function* func = nullptr) {
    if (result) {
      fail("unexpected true: " + std::string(text), curr, func);
      return false;
    }
    return true;
  }

  template<typename T, typename S>
  bool shouldBeEqual(
    S left, S right, T curr, const char* text, Function* func = nullptr) {
    if (left != right) {
      std::ostringstream ss;
      ss << left << " != " << right << ": " << text;
      fail(ss.str(), curr, func);
      return false;
    }
    return true;
  }

  template<typename T, typename S>
  bool shouldBeEqualOrFirstIsUnreachable(
    S left, S right, T curr, const char* text, Function* func = nullptr) {
    if (left != Type::unreachable && left != right) {
      std::ostringstream ss;
      ss << left << " != " << right << ": " << text;
      fail(ss.str(), curr, func);
      return false;
    }
    return true;
  }

  template<typename T, typename S>
  bool shouldBeUnequal(
    S left, S right, T curr, const char* text, Function* func = nullptr) {
    if (left == right) {
      std::ostringstream ss;
      ss << left << " == " << right << ": " << text;
      fail(ss.str(), curr, func);
      return false;
    }
    return true;
  }

  void shouldBeIntOrUnreachable(Type ty,
                                Expression* curr,
                                const char* text,
                                Function* func = nullptr) {
    switch (ty.getBasic()) {
      case Type::i32:
      case Type::i64:
      case Type::unreachable: {
        break;
      }
      default:
        fail(text, curr, func);
    }
  }

  // Type 'left' should be a subtype of 'right'.
  bool shouldBeSubType(Type left,
                       Type right,
                       Expression* curr,
                       const char* text,
                       Function* func = nullptr) {
    if (Type::isSubType(left, right)) {
      return true;
    }
    fail(text, curr, func);
    return false;
  }

  bool shouldBeSubTypeIgnoringShared(Type left,
                                     Type right,
                                     Expression* curr,
                                     const char* text,
                                     Function* func = nullptr) {
    assert(right.isRef() && right.getHeapType().isBasic());
    auto share = left.isRef() ? left.getHeapType().getShared() : Unshared;
    auto ht = right.getHeapType();
    auto matchedRight = Type(ht.getBasic(share), right.getNullability());
    return shouldBeSubType(left, matchedRight, curr, text, func);
  }
};

std::string getMissingFeaturesList(Module& wasm, FeatureSet feats) {
  std::stringstream ss;
  bool first = true;
  ss << '[';
  (feats - wasm.features).iterFeatures([&](FeatureSet feat) {
    if (first) {
      first = false;
    } else {
      ss << " ";
    }
    ss << "--enable-" << feat.toString();
  });
  ss << ']';
  return ss.str();
}

struct FunctionValidator : public WalkerPass<PostWalker<FunctionValidator>> {
  bool isFunctionParallel() override { return true; }

  std::unique_ptr<Pass> create() override {
    return std::make_unique<FunctionValidator>(*getModule(), &info);
  }

  bool modifiesBinaryenIR() override { return false; }

  ValidationInfo& info;

  FunctionValidator(Module& wasm, ValidationInfo* info) : info(*info) {
    setModule(&wasm);
  }

  // Validate the entire module.
  void validate(PassRunner* runner) { run(runner, getModule()); }

  // Validate a specific expression.
  void validate(Expression* curr) { walk(curr); }

  // Validate a function.
  void validate(Function* func) { walkFunction(func); }

  std::unordered_map<Name, std::unordered_set<Type>> breakTypes;
  std::unordered_set<Name> delegateTargetNames;
  std::unordered_set<Name> rethrowTargetNames;

  // Binaryen IR requires that label names must be unique - IR generators must
  // ensure that
  std::unordered_set<Name> labelNames;

  void noteLabelName(Name name);

public:
  // visitors

  void validatePoppyExpression(Expression* curr);

  static void visitPoppyExpression(FunctionValidator* self,
                                   Expression** currp) {
    self->validatePoppyExpression(*currp);
  }

  static void visitPreBlock(FunctionValidator* self, Expression** currp) {
    auto* curr = (*currp)->cast<Block>();
    if (curr->name.is()) {
      self->breakTypes[curr->name];
    }
  }

  void visitBlock(Block* curr);
  void validateNormalBlockElements(Block* curr);
  void validatePoppyBlockElements(Block* curr);

  static void visitPreLoop(FunctionValidator* self, Expression** currp) {
    auto* curr = (*currp)->cast<Loop>();
    if (curr->name.is()) {
      self->breakTypes[curr->name];
    }
  }

  void visitLoop(Loop* curr);
  void visitIf(If* curr);

  static void visitPreTry(FunctionValidator* self, Expression** currp) {
    auto* curr = (*currp)->cast<Try>();
    if (curr->name.is()) {
      self->delegateTargetNames.insert(curr->name);
    }
  }

  // We remove try's label before proceeding to verify catch bodies because the
  // following is a validation failure:
  // (try $l0
  //   (do ... )
  //   (catch $e
  //     (try
  //       (do ...)
  //       (delegate $l0) ;; validation failure
  //     )
  //   )
  // )
  // Unlike branches, if delegate's target 'catch' is located above the
  // delegate, it is a validation failure.
  static void visitPreCatch(FunctionValidator* self, Expression** currp) {
    auto* curr = (*currp)->cast<Try>();
    if (curr->name.is()) {
      self->delegateTargetNames.erase(curr->name);
      self->rethrowTargetNames.insert(curr->name);
    }
  }

  // override scan to add a pre and a post check task to all nodes
  static void scan(FunctionValidator* self, Expression** currp) {
    auto* curr = *currp;
    // Treat 'Try' specially because we need to run visitPreCatch between the
    // try body and catch bodies
    if (curr->is<Try>()) {
      self->pushTask(doVisitTry, currp);
      auto& list = curr->cast<Try>()->catchBodies;
      for (int i = int(list.size()) - 1; i >= 0; i--) {
        self->pushTask(scan, &list[i]);
      }
      self->pushTask(visitPreCatch, currp);
      self->pushTask(scan, &curr->cast<Try>()->body);
      self->pushTask(visitPreTry, currp);
      return;
    }

    PostWalker<FunctionValidator>::scan(self, currp);

    if (curr->is<Block>()) {
      self->pushTask(visitPreBlock, currp);
    }
    if (curr->is<Loop>()) {
      self->pushTask(visitPreLoop, currp);
    }
    if (auto* func = self->getFunction()) {
      if (func->profile == IRProfile::Poppy) {
        self->pushTask(visitPoppyExpression, currp);
      }
    }

    // Also verify that only allowed expressions end up in the situation where
    // the expression has type unreachable but there is no unreachable child.
    // For example a Call with no unreachable child cannot be unreachable, but a
    // Break can be.
    if (curr->type == Type::unreachable) {
      switch (curr->_id) {
        case Expression::BreakId: {
          // If there is a condition, that is already validated fully in
          // visitBreak(). If there isn't a condition, then this is allowed to
          // be unreachable even without an unreachable child. Either way, we
          // can leave.
          return;
        }
        case Expression::SwitchId:
        case Expression::ReturnId:
        case Expression::UnreachableId:
        case Expression::ThrowId:
        case Expression::RethrowId:
        case Expression::ThrowRefId: {
          // These can all be unreachable without an unreachable child.
          return;
        }
        case Expression::CallId: {
          if (curr->cast<Call>()->isReturn) {
            return;
          }
          break;
        }
        case Expression::CallIndirectId: {
          if (curr->cast<CallIndirect>()->isReturn) {
            return;
          }
          break;
        }
        case Expression::CallRefId: {
          if (curr->cast<CallRef>()->isReturn) {
            return;
          }
          break;
        }
        default: {
          break;
        }
      }

      // If we reach here, then we must have an unreachable child.
      bool hasUnreachableChild = false;
      for (auto* child : ChildIterator(curr)) {
        if (child->type == Type::unreachable) {
          hasUnreachableChild = true;
          break;
        }
      }
      self->shouldBeTrue(hasUnreachableChild,
                         curr,
                         "unreachable instruction must have unreachable child");
    }
  }

  void noteBreak(Name name, Expression* value, Expression* curr);
  void noteBreak(Name name, Type valueType, Expression* curr);
  void visitBreak(Break* curr);
  void visitSwitch(Switch* curr);
  void visitCall(Call* curr);
  void visitCallIndirect(CallIndirect* curr);
  void visitConst(Const* curr);
  void visitLocalGet(LocalGet* curr);
  void visitLocalSet(LocalSet* curr);
  void visitGlobalGet(GlobalGet* curr);
  void visitGlobalSet(GlobalSet* curr);
  void visitLoad(Load* curr);
  void visitStore(Store* curr);
  void visitAtomicRMW(AtomicRMW* curr);
  void visitAtomicCmpxchg(AtomicCmpxchg* curr);
  void visitAtomicWait(AtomicWait* curr);
  void visitAtomicNotify(AtomicNotify* curr);
  void visitAtomicFence(AtomicFence* curr);
  void visitSIMDExtract(SIMDExtract* curr);
  void visitSIMDReplace(SIMDReplace* curr);
  void visitSIMDShuffle(SIMDShuffle* curr);
  void visitSIMDTernary(SIMDTernary* curr);
  void visitSIMDShift(SIMDShift* curr);
  void visitSIMDLoad(SIMDLoad* curr);
  void visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr);
  void visitMemoryInit(MemoryInit* curr);
  void visitDataDrop(DataDrop* curr);
  void visitMemoryCopy(MemoryCopy* curr);
  void visitMemoryFill(MemoryFill* curr);
  void visitBinary(Binary* curr);
  void visitUnary(Unary* curr);
  void visitSelect(Select* curr);
  void visitDrop(Drop* curr);
  void visitReturn(Return* curr);
  void visitMemorySize(MemorySize* curr);
  void visitMemoryGrow(MemoryGrow* curr);
  void visitRefNull(RefNull* curr);
  void visitRefIsNull(RefIsNull* curr);
  void visitRefAs(RefAs* curr);
  void visitRefFunc(RefFunc* curr);
  void visitRefEq(RefEq* curr);
  void visitTableGet(TableGet* curr);
  void visitTableSet(TableSet* curr);
  void visitTableSize(TableSize* curr);
  void visitTableGrow(TableGrow* curr);
  void visitTableFill(TableFill* curr);
  void visitTableCopy(TableCopy* curr);
  void visitTableInit(TableInit* curr);
  void noteDelegate(Name name, Expression* curr);
  void noteRethrow(Name name, Expression* curr);
  void visitTry(Try* curr);
  void visitTryTable(TryTable* curr);
  void visitThrow(Throw* curr);
  void visitRethrow(Rethrow* curr);
  void visitThrowRef(ThrowRef* curr);
  void visitTupleMake(TupleMake* curr);
  void visitTupleExtract(TupleExtract* curr);
  void visitCallRef(CallRef* curr);
  void visitRefI31(RefI31* curr);
  void visitI31Get(I31Get* curr);
  void visitRefTest(RefTest* curr);
  void visitRefCast(RefCast* curr);
  void visitBrOn(BrOn* curr);
  void visitStructNew(StructNew* curr);
  void visitStructGet(StructGet* curr);
  void visitStructSet(StructSet* curr);
  void visitArrayNew(ArrayNew* curr);
  template<typename ArrayNew> void visitArrayNew(ArrayNew* curr);
  void visitArrayNewData(ArrayNewData* curr);
  void visitArrayNewElem(ArrayNewElem* curr);
  void visitArrayNewFixed(ArrayNewFixed* curr);
  void visitArrayGet(ArrayGet* curr);
  void visitArraySet(ArraySet* curr);
  void visitArrayLen(ArrayLen* curr);
  void visitArrayCopy(ArrayCopy* curr);
  void visitArrayFill(ArrayFill* curr);
  template<typename ArrayInit> void visitArrayInit(ArrayInit* curr);
  void visitArrayInitData(ArrayInitData* curr);
  void visitArrayInitElem(ArrayInitElem* curr);
  void visitStringNew(StringNew* curr);
  void visitStringConst(StringConst* curr);
  void visitStringMeasure(StringMeasure* curr);
  void visitStringEncode(StringEncode* curr);
  void visitStringConcat(StringConcat* curr);
  void visitStringEq(StringEq* curr);
  void visitStringWTF16Get(StringWTF16Get* curr);
  void visitStringSliceWTF(StringSliceWTF* curr);
  void visitContNew(ContNew* curr);
  void visitContBind(ContBind* curr);
  void visitSuspend(Suspend* curr);
  void visitResume(Resume* curr);
  void visitResumeThrow(ResumeThrow* curr);
  void visitStackSwitch(StackSwitch* curr);

  void visitFunction(Function* curr);

  // helpers
private:
  std::ostream& getStream() { return info.getStream(getFunction()); }

  template<typename T>
  bool shouldBeTrue(bool result, T curr, const char* text) {
    return info.shouldBeTrue(result, curr, text, getFunction());
  }
  template<typename T>
  bool shouldBeFalse(bool result, T curr, const char* text) {
    return info.shouldBeFalse(result, curr, text, getFunction());
  }

  template<typename T, typename S>
  bool shouldBeEqual(S left, S right, T curr, const char* text) {
    return info.shouldBeEqual(left, right, curr, text, getFunction());
  }

  template<typename T, typename S>
  bool
  shouldBeEqualOrFirstIsUnreachable(S left, S right, T curr, const char* text) {
    return info.shouldBeEqualOrFirstIsUnreachable(
      left, right, curr, text, getFunction());
  }

  template<typename T, typename S>
  bool shouldBeUnequal(S left, S right, T curr, const char* text) {
    return info.shouldBeUnequal(left, right, curr, text, getFunction());
  }

  void shouldBeIntOrUnreachable(Type ty, Expression* curr, const char* text) {
    return info.shouldBeIntOrUnreachable(ty, curr, text, getFunction());
  }

  bool
  shouldBeSubType(Type left, Type right, Expression* curr, const char* text) {
    return info.shouldBeSubType(left, right, curr, text, getFunction());
  }

  bool shouldBeSubTypeIgnoringShared(Type left,
                                     Type right,
                                     Expression* curr,
                                     const char* text) {
    return info.shouldBeSubTypeIgnoringShared(left, right, curr, text);
  }

  void validateOffset(Address offset, Memory* mem, Expression* curr);
  void validateAlignment(
    size_t align, Type type, Index bytes, bool isAtomic, Expression* curr);
  void validateMemBytes(uint8_t bytes, Type type, Expression* curr);

  template<typename T> void validateReturnCall(T* curr) {
    shouldBeTrue(!curr->isReturn || getModule()->features.hasTailCall(),
                 curr,
                 "return_call* requires tail calls [--enable-tail-call]");
  }

  // |printable| is the expression to print in case of an error. That may differ
  // from |curr| which we are validating.
  template<typename T>
  void validateCallParamsAndResult(T* curr,
                                   HeapType sigType,
                                   Expression* printable) {
    if (!shouldBeTrue(sigType.isSignature(),
                      printable,
                      "Heap type must be a signature type")) {
      return;
    }
    auto sig = sigType.getSignature();
    if (!shouldBeTrue(curr->operands.size() == sig.params.size(),
                      printable,
                      "call* param number must match")) {
      return;
    }
    size_t i = 0;
    for (const auto& param : sig.params) {
      if (!shouldBeSubType(curr->operands[i]->type,
                           param,
                           printable,
                           "call param types must match") &&
          !info.quiet) {
        getStream() << "(on argument " << i << ")\n";
      }
      ++i;
    }
    if (curr->isReturn) {
      shouldBeEqual(curr->type,
                    Type(Type::unreachable),
                    printable,
                    "return_call* should have unreachable type");
      auto* func = getFunction();
      if (!shouldBeTrue(!!func, curr, "function not defined")) {
        return;
      }
      shouldBeSubType(
        sig.results,
        func->getResults(),
        printable,
        "return_call* callee return type must match caller return type");
    } else {
      shouldBeEqualOrFirstIsUnreachable(
        curr->type,
        sig.results,
        printable,
        "call* type must match callee return type");
    }
  }

  // In the common case, we use |curr| as |printable|.
  template<typename T>
  void validateCallParamsAndResult(T* curr, HeapType sigType) {
    validateCallParamsAndResult(curr, sigType, curr);
  }
};

void FunctionValidator::noteLabelName(Name name) {
  if (!name.is()) {
    return;
  }
  auto [_, inserted] = labelNames.insert(name);
  shouldBeTrue(
    inserted,
    name,
    "names in Binaryen IR must be unique - IR generators must ensure that");
}

void FunctionValidator::validatePoppyExpression(Expression* curr) {
  if (curr->type == Type::unreachable) {
    shouldBeTrue(StackUtils::mayBeUnreachable(curr),
                 curr,
                 "Only control flow structures and unreachable polymorphic"
                 " instructions may be unreachable in Poppy IR");
  }
  if (Properties::isControlFlowStructure(curr)) {
    // Check that control flow children (except If conditions) are blocks
    if (auto* if_ = curr->dynCast<If>()) {
      shouldBeTrue(
        if_->condition->is<Pop>(), curr, "Expected condition to be a Pop");
      shouldBeTrue(if_->ifTrue->is<Block>(),
                   curr,
                   "Expected control flow child to be a block");
      shouldBeTrue(!if_->ifFalse || if_->ifFalse->is<Block>(),
                   curr,
                   "Expected control flow child to be a block");
    } else if (!curr->is<Block>()) {
      for (auto* child : ChildIterator(curr)) {
        shouldBeTrue(child->is<Block>(),
                     curr,
                     "Expected control flow child to be a block");
      }
    }
  } else {
    // Check that all children are Pops
    for (auto* child : ChildIterator(curr)) {
      shouldBeTrue(child->is<Pop>(), curr, "Unexpected non-Pop child");
    }
  }
}

void FunctionValidator::visitBlock(Block* curr) {
  if (!getModule()->features.hasMultivalue()) {
    shouldBeTrue(
      !curr->type.isTuple(),
      curr,
      "Multivalue block type require multivalue [--enable-multivalue]");
  }
  // if we are break'ed to, then the value must be right for us
  if (curr->name.is()) {
    noteLabelName(curr->name);
    auto iter = breakTypes.find(curr->name);
    assert(iter != breakTypes.end()); // we set it ourselves
    for (Type breakType : iter->second) {
      if (breakType == Type::none && curr->type == Type::unreachable) {
        // We allow empty breaks to unreachable blocks.
        continue;
      }

      shouldBeSubType(breakType,
                      curr->type,
                      curr,
                      "break type must be a subtype of the target block type");
    }
    breakTypes.erase(iter);
  }

  auto* func = getFunction();
  if (!shouldBeTrue(!!func, curr, "function not defined")) {
    return;
  }
  switch (func->profile) {
    case IRProfile::Normal:
      validateNormalBlockElements(curr);
      break;
    case IRProfile::Poppy:
      validatePoppyBlockElements(curr);
      break;
  }
}

void FunctionValidator::validateNormalBlockElements(Block* curr) {
  if (curr->list.size() > 1) {
    for (Index i = 0; i < curr->list.size() - 1; i++) {
      if (!shouldBeTrue(
            !curr->list[i]->type.isConcrete(),
            curr,
            "non-final block elements returning a value must be dropped") &&
          !info.quiet) {
        getStream() << "(on index " << i << ":\n"
                    << curr->list[i] << "\n), type: " << curr->list[i]->type
                    << "\n";
      }
    }
  }
  if (curr->list.size() > 0) {
    auto backType = curr->list.back()->type;
    if (!curr->type.isConcrete()) {
      shouldBeFalse(backType.isConcrete(),
                    curr,
                    "if block is not returning a value, final element should "
                    "not flow out a value");
    } else {
      if (backType.isConcrete()) {
        shouldBeSubType(
          backType,
          curr->type,
          curr,
          "block with value and last element with value must match types");
      } else {
        shouldBeUnequal(
          backType,
          Type(Type::none),
          curr,
          "block with value must not have last element that is none");
      }
    }
  }
  if (curr->type.isConcrete()) {
    shouldBeTrue(
      curr->list.size() > 0, curr, "block with a value must not be empty");
  }
}

void FunctionValidator::validatePoppyBlockElements(Block* curr) {
  StackSignature blockSig;
  for (size_t i = 0; i < curr->list.size(); ++i) {
    Expression* expr = curr->list[i];
    if (!shouldBeTrue(
          !expr->is<Pop>(), expr, "Unexpected top-level pop in block")) {
      return;
    }
    StackSignature sig(expr);
    if (!shouldBeTrue(blockSig.composes(sig),
                      curr,
                      "block element has incompatible type") &&
        !info.quiet) {
      getStream() << "(on index " << i << ":\n"
                  << expr << "\n), required: " << sig.params << ", available: ";
      if (blockSig.kind == StackSignature::Polymorphic) {
        getStream() << "polymorphic, ";
      }
      getStream() << blockSig.results << "\n";
      return;
    }
    blockSig += sig;
  }
  if (curr->type == Type::unreachable) {
    shouldBeTrue(blockSig.kind == StackSignature::Polymorphic,
                 curr,
                 "unreachable block should have unreachable element");
  } else {
    if (!shouldBeTrue(
          StackSignature::isSubType(
            blockSig,
            StackSignature(Type::none, curr->type, StackSignature::Fixed)),
          curr,
          "block contents should satisfy block type") &&
        !info.quiet) {
      getStream() << "contents: " << blockSig.results
                  << (blockSig.kind == StackSignature::Polymorphic
                        ? " [polymorphic]"
                        : "")
                  << "\n"
                  << "expected: " << curr->type << "\n";
    }
  }
}

void FunctionValidator::visitLoop(Loop* curr) {
  if (curr->name.is()) {
    noteLabelName(curr->name);
    auto iter = breakTypes.find(curr->name);
    assert(iter != breakTypes.end()); // we set it ourselves
    for (Type breakType : iter->second) {
      shouldBeEqual(breakType,
                    Type(Type::none),
                    curr,
                    "breaks to a loop cannot pass a value");
    }
    breakTypes.erase(iter);
  }
  if (curr->type == Type::none) {
    shouldBeFalse(curr->body->type.isConcrete(),
                  curr,
                  "bad body for a loop that has no value");
  }

  // When there are multiple instructions within a loop, they are wrapped in a
  // Block internally, so visitBlock can take care of verification. Here we
  // check cases when there is only one instruction in a Loop.
  if (!curr->body->is<Block>()) {
    if (!curr->type.isConcrete()) {
      shouldBeFalse(curr->body->type.isConcrete(),
                    curr,
                    "if loop is not returning a value, final element should "
                    "not flow out a value");
    } else {
      shouldBeSubType(curr->body->type,
                      curr->type,
                      curr,
                      "loop with value and body must match types");
    }
  }
}

void FunctionValidator::visitIf(If* curr) {
  shouldBeTrue(curr->condition->type == Type::unreachable ||
                 curr->condition->type == Type::i32,
               curr,
               "if condition must be valid");
  if (!curr->ifFalse) {
    shouldBeFalse(curr->ifTrue->type.isConcrete(),
                  curr,
                  "if without else must not return a value in body");
    if (curr->condition->type != Type::unreachable) {
      shouldBeEqual(curr->type,
                    Type(Type::none),
                    curr,
                    "if without else and reachable condition must be none");
    }
  } else {
    if (curr->type != Type::unreachable) {
      shouldBeSubType(curr->ifTrue->type,
                      curr->type,
                      curr,
                      "returning if-else's true must have right type");
      shouldBeSubType(curr->ifFalse->type,
                      curr->type,
                      curr,
                      "returning if-else's false must have right type");
    } else {
      if (curr->condition->type == Type::unreachable) {
        shouldBeTrue(
          curr->ifTrue->type == Type::unreachable ||
            curr->ifFalse->type == Type::unreachable ||
            (curr->ifTrue->type == Type::none &&
             curr->ifFalse->type == Type::none) ||
            Type::hasLeastUpperBound(curr->ifTrue->type, curr->ifFalse->type),
          curr,
          "arms of unreachable if-else must have compatible types");
      } else {
        shouldBeEqual(curr->ifTrue->type,
                      Type(Type::unreachable),
                      curr,
                      "unreachable if-else must have unreachable true");
        shouldBeEqual(curr->ifFalse->type,
                      Type(Type::unreachable),
                      curr,
                      "unreachable if-else must have unreachable false");
      }
    }
  }
}

void FunctionValidator::noteBreak(Name name,
                                  Expression* value,
                                  Expression* curr) {
  if (value) {
    shouldBeUnequal(
      value->type, Type(Type::none), curr, "breaks must have a valid value");
  }
  noteBreak(name, value ? value->type : Type::none, curr);
}

void FunctionValidator::noteBreak(Name name, Type valueType, Expression* curr) {
  auto iter = breakTypes.find(name);
  if (!shouldBeTrue(
        iter != breakTypes.end(), curr, "all break targets must be valid")) {
    return;
  }
  iter->second.insert(valueType);
}

void FunctionValidator::visitBreak(Break* curr) {
  noteBreak(curr->name, curr->value, curr);
  if (curr->value) {
    shouldBeTrue(curr->value->type != Type::none,
                 curr,
                 "break value must not have none type");
  }
  if (curr->condition) {
    shouldBeTrue(curr->condition->type == Type::unreachable ||
                   curr->condition->type == Type::i32,
                 curr,
                 "break condition must be i32");
  }
}

void FunctionValidator::visitSwitch(Switch* curr) {
  for (auto& target : curr->targets) {
    noteBreak(target, curr->value, curr);
  }
  noteBreak(curr->default_, curr->value, curr);
  shouldBeTrue(curr->condition->type == Type::unreachable ||
                 curr->condition->type == Type::i32,
               curr,
               "br_table condition must be i32");
}

void FunctionValidator::visitCall(Call* curr) {
  validateReturnCall(curr);
  if (!info.validateGlobally) {
    return;
  }
  auto* target = getModule()->getFunctionOrNull(curr->target);
  if (!shouldBeTrue(!!target, curr, "call target must exist")) {
    return;
  }
  validateCallParamsAndResult(curr, target->type);

  if (Intrinsics(*getModule()).isCallWithoutEffects(curr)) {
    // call.without.effects has the specific form of the last argument being a
    // function reference, which will be called with all the previous arguments.
    // The type must be consistent with that. This, for example, is not:
    //
    //  (call $call.without.effects
    //    (i32.const 1)
    //    (.. some function reference that expects an f64 param and not i32 ..)
    //  )
    if (shouldBeTrue(!curr->operands.empty(),
                     curr,
                     "call.without.effects must have a target operand")) {
      auto* target = curr->operands.back();
      // Validate only in the case that the target is a function. If it isn't,
      // it might be unreachable (which is fine, and we can ignore this), or if
      // the call.without.effects import doesn't have a function as the last
      // parameter, then validateImports() will handle that later (and it's
      // better to emit a single error there than one per callsite here).
      if (target->type.isFunction()) {
        // Copy the original call and remove the reference. It must then match
        // the expected signature.
        struct Copy {
          std::vector<Expression*> operands;
          bool isReturn;
          Type type;
        } copy;
        for (Index i = 0; i < curr->operands.size() - 1; i++) {
          copy.operands.push_back(curr->operands[i]);
        }
        copy.isReturn = curr->isReturn;
        copy.type = curr->type;
        validateCallParamsAndResult(&copy, target->type.getHeapType(), curr);
      }
    }
  }
}

void FunctionValidator::visitCallIndirect(CallIndirect* curr) {
  validateReturnCall(curr);

  if (curr->target->type != Type::unreachable) {
    auto* table = getModule()->getTableOrNull(curr->table);
    if (shouldBeTrue(!!table, curr, "call-indirect table must exist")) {
      shouldBeEqualOrFirstIsUnreachable(
        curr->target->type,
        table->addressType,
        curr,
        "call-indirect call target must match the table index type");
      shouldBeTrue(!!table, curr, "call-indirect table must exist");
      shouldBeTrue(table->type.isFunction(),
                   curr,
                   "call-indirect table must be of function type.");
    }
  }

  validateCallParamsAndResult(curr, curr->heapType);
}

void FunctionValidator::visitConst(Const* curr) {
  shouldBeTrue(curr->type.getFeatures() <= getModule()->features,
               curr,
               "all used features should be allowed");
}

void FunctionValidator::visitLocalGet(LocalGet* curr) {
  shouldBeTrue(curr->type.isConcrete(),
               curr,
               "local.get must have a valid type - check what you provided "
               "when you constructed the node");
  if (shouldBeTrue(curr->index < getFunction()->getNumLocals(),
                   curr,
                   "local.get index must be small enough")) {
    shouldBeTrue(curr->type == getFunction()->getLocalType(curr->index),
                 curr,
                 "local.get must have proper type");
  }
}

void FunctionValidator::visitLocalSet(LocalSet* curr) {
  if (shouldBeTrue(curr->index < getFunction()->getNumLocals(),
                   curr,
                   "local.set index must be small enough")) {
    if (curr->value->type != Type::unreachable) {
      if (curr->type != Type::none) { // tee is ok anyhow
        shouldBeEqual(getFunction()->getLocalType(curr->index),
                      curr->type,
                      curr,
                      "local.set type must be correct");
      }
      shouldBeSubType(curr->value->type,
                      getFunction()->getLocalType(curr->index),
                      curr,
                      "local.set's value type must be correct");
    }
  }
}

void FunctionValidator::visitGlobalGet(GlobalGet* curr) {
  if (!info.validateGlobally) {
    return;
  }
  auto* global = getModule()->getGlobalOrNull(curr->name);
  if (shouldBeTrue(global, curr, "global.get name must be valid")) {
    shouldBeEqual(
      curr->type, global->type, curr, "global.get must have right type");
  }
}

void FunctionValidator::visitGlobalSet(GlobalSet* curr) {
  if (!info.validateGlobally) {
    return;
  }
  auto* global = getModule()->getGlobalOrNull(curr->name);
  if (shouldBeTrue(global,
                   curr,
                   "global.set name must be valid (and not an import; imports "
                   "can't be modified)")) {
    shouldBeTrue(global->mutable_, curr, "global.set global must be mutable");
    shouldBeSubType(curr->value->type,
                    global->type,
                    curr,
                    "global.set value must have right type");
  }
}

void FunctionValidator::visitLoad(Load* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.load memory must exist");
  if (curr->isAtomic) {
    shouldBeTrue(getModule()->features.hasAtomics(),
                 curr,
                 "Atomic operations require threads [--enable-threads]");
    shouldBeTrue(curr->type == Type::i32 || curr->type == Type::i64 ||
                   curr->type == Type::unreachable,
                 curr,
                 "Atomic load should be i32 or i64");
  }
  if (curr->type == Type::v128) {
    shouldBeTrue(getModule()->features.hasSIMD(),
                 curr,
                 "SIMD operations require SIMD [--enable-simd]");
  }
  validateMemBytes(curr->bytes, curr->type, curr);
  validateOffset(curr->offset, memory, curr);
  validateAlignment(curr->align, curr->type, curr->bytes, curr->isAtomic, curr);
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "load pointer type must match memory index type");
  if (curr->isAtomic) {
    shouldBeFalse(curr->signed_, curr, "atomic loads must be unsigned");
    shouldBeIntOrUnreachable(
      curr->type, curr, "atomic loads must be of integers");
  }
}

void FunctionValidator::visitStore(Store* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.store memory must exist");
  if (curr->isAtomic) {
    shouldBeTrue(getModule()->features.hasAtomics(),
                 curr,
                 "Atomic operations require threads [--enable-threads]");
    shouldBeTrue(curr->valueType == Type::i32 || curr->valueType == Type::i64 ||
                   curr->valueType == Type::unreachable,
                 curr,
                 "Atomic store should be i32 or i64");
  }
  if (curr->valueType == Type::v128) {
    shouldBeTrue(getModule()->features.hasSIMD(),
                 curr,
                 "SIMD operations require SIMD [--enable-simd]");
  }
  validateMemBytes(curr->bytes, curr->valueType, curr);
  validateOffset(curr->offset, memory, curr);
  validateAlignment(
    curr->align, curr->valueType, curr->bytes, curr->isAtomic, curr);
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "store pointer must match memory index type");
  shouldBeUnequal(curr->value->type,
                  Type(Type::none),
                  curr,
                  "store value type must not be none");
  shouldBeEqualOrFirstIsUnreachable(
    curr->value->type, curr->valueType, curr, "store value type must match");
  if (curr->isAtomic) {
    shouldBeIntOrUnreachable(
      curr->valueType, curr, "atomic stores must be of integers");
  }
}

void FunctionValidator::visitAtomicRMW(AtomicRMW* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.atomicRMW memory must exist");
  shouldBeTrue(getModule()->features.hasAtomics(),
               curr,
               "Atomic operations require threads [--enable-threads]");
  validateMemBytes(curr->bytes, curr->type, curr);
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "AtomicRMW pointer type must match memory index type");
  shouldBeEqualOrFirstIsUnreachable(curr->type,
                                    curr->value->type,
                                    curr,
                                    "AtomicRMW result type must match operand");
  shouldBeIntOrUnreachable(
    curr->type, curr, "Atomic operations are only valid on int types");
}

void FunctionValidator::visitAtomicCmpxchg(AtomicCmpxchg* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.atomicCmpxchg memory must exist");
  shouldBeTrue(getModule()->features.hasAtomics(),
               curr,
               "Atomic operations require threads [--enable-threads]");
  validateMemBytes(curr->bytes, curr->type, curr);
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "cmpxchg pointer must match memory index type");
  if (curr->expected->type != Type::unreachable &&
      curr->replacement->type != Type::unreachable) {
    shouldBeEqual(curr->expected->type,
                  curr->replacement->type,
                  curr,
                  "cmpxchg operand types must match");
  }
  shouldBeEqualOrFirstIsUnreachable(curr->type,
                                    curr->expected->type,
                                    curr,
                                    "Cmpxchg result type must match expected");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type,
    curr->replacement->type,
    curr,
    "Cmpxchg result type must match replacement");
  shouldBeIntOrUnreachable(curr->expected->type,
                           curr,
                           "Atomic operations are only valid on int types");
}

void FunctionValidator::visitAtomicWait(AtomicWait* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.atomicWait memory must exist");
  shouldBeTrue(getModule()->features.hasAtomics(),
               curr,
               "Atomic operations require threads [--enable-threads]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::i32), curr, "AtomicWait must have type i32");
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "AtomicWait pointer must match memory index type");
  shouldBeIntOrUnreachable(
    curr->expected->type, curr, "AtomicWait expected type must be int");
  shouldBeEqualOrFirstIsUnreachable(
    curr->expected->type,
    curr->expectedType,
    curr,
    "AtomicWait expected type must match operand");
  shouldBeEqualOrFirstIsUnreachable(curr->timeout->type,
                                    Type(Type::i64),
                                    curr,
                                    "AtomicWait timeout type must be i64");
}

void FunctionValidator::visitAtomicNotify(AtomicNotify* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.atomicNotify memory must exist");
  shouldBeTrue(getModule()->features.hasAtomics(),
               curr,
               "Atomic operations require threads [--enable-threads]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::i32), curr, "AtomicNotify must have type i32");
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "AtomicNotify pointer must match memory index type");
  shouldBeEqualOrFirstIsUnreachable(
    curr->notifyCount->type,
    Type(Type::i32),
    curr,
    "AtomicNotify notifyCount type must be i32");
}

void FunctionValidator::visitAtomicFence(AtomicFence* curr) {
  shouldBeTrue(getModule()->features.hasAtomics(),
               curr,
               "Atomic operations require threads [--enable-threads]");
  shouldBeTrue(curr->order == 0,
               curr,
               "Currently only sequentially consistent atomics are supported, "
               "so AtomicFence's order should be 0");
}

void FunctionValidator::visitSIMDExtract(SIMDExtract* curr) {
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  shouldBeEqualOrFirstIsUnreachable(curr->vec->type,
                                    Type(Type::v128),
                                    curr,
                                    "extract_lane must operate on a v128");
  Type lane_t = Type::none;
  size_t lanes = 0;
  switch (curr->op) {
    case ExtractLaneSVecI8x16:
    case ExtractLaneUVecI8x16:
      lane_t = Type::i32;
      lanes = 16;
      break;
    case ExtractLaneSVecI16x8:
    case ExtractLaneUVecI16x8:
      lane_t = Type::i32;
      lanes = 8;
      break;
    case ExtractLaneVecI32x4:
      lane_t = Type::i32;
      lanes = 4;
      break;
    case ExtractLaneVecI64x2:
      lane_t = Type::i64;
      lanes = 2;
      break;
    case ExtractLaneVecF16x8:
      shouldBeTrue(getModule()->features.hasFP16(),
                   curr,
                   "FP16 operations require FP16 [--enable-fp16]");
      lane_t = Type::f32;
      lanes = 8;
      break;
    case ExtractLaneVecF32x4:
      lane_t = Type::f32;
      lanes = 4;
      break;
    case ExtractLaneVecF64x2:
      lane_t = Type::f64;
      lanes = 2;
      break;
  }
  shouldBeEqualOrFirstIsUnreachable(
    curr->type,
    lane_t,
    curr,
    "extract_lane must have same type as vector lane");
  shouldBeTrue(curr->index < lanes, curr, "invalid lane index");
}

void FunctionValidator::visitSIMDReplace(SIMDReplace* curr) {
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::v128), curr, "replace_lane must have type v128");
  shouldBeEqualOrFirstIsUnreachable(curr->vec->type,
                                    Type(Type::v128),
                                    curr,
                                    "replace_lane must operate on a v128");
  Type lane_t = Type::none;
  size_t lanes = 0;
  switch (curr->op) {
    case ReplaceLaneVecI8x16:
      lane_t = Type::i32;
      lanes = 16;
      break;
    case ReplaceLaneVecI16x8:
      lane_t = Type::i32;
      lanes = 8;
      break;
    case ReplaceLaneVecI32x4:
      lane_t = Type::i32;
      lanes = 4;
      break;
    case ReplaceLaneVecI64x2:
      lane_t = Type::i64;
      lanes = 2;
      break;
    case ReplaceLaneVecF16x8:
      shouldBeTrue(getModule()->features.hasFP16(),
                   curr,
                   "FP16 operations require FP16 [--enable-fp16]");
      lane_t = Type::f32;
      lanes = 8;
      break;
    case ReplaceLaneVecF32x4:
      lane_t = Type::f32;
      lanes = 4;
      break;
    case ReplaceLaneVecF64x2:
      lane_t = Type::f64;
      lanes = 2;
      break;
  }
  shouldBeEqualOrFirstIsUnreachable(
    curr->value->type, lane_t, curr, "unexpected value type");
  shouldBeTrue(curr->index < lanes, curr, "invalid lane index");
}

void FunctionValidator::visitSIMDShuffle(SIMDShuffle* curr) {
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::v128), curr, "i8x16.shuffle must have type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->left->type, Type(Type::v128), curr, "expected operand of type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->right->type, Type(Type::v128), curr, "expected operand of type v128");
  for (uint8_t index : curr->mask) {
    shouldBeTrue(index < 32, curr, "Invalid lane index in mask");
  }
}

void FunctionValidator::visitSIMDTernary(SIMDTernary* curr) {
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::v128), curr, "SIMD ternary must have type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->a->type, Type(Type::v128), curr, "expected operand of type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->b->type, Type(Type::v128), curr, "expected operand of type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->c->type, Type(Type::v128), curr, "expected operand of type v128");
}

void FunctionValidator::visitSIMDShift(SIMDShift* curr) {
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::v128), curr, "vector shift must have type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->vec->type, Type(Type::v128), curr, "expected operand of type v128");
  shouldBeEqualOrFirstIsUnreachable(curr->shift->type,
                                    Type(Type::i32),
                                    curr,
                                    "expected shift amount to have type i32");
}

void FunctionValidator::visitSIMDLoad(SIMDLoad* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.SIMDLoad memory must exist");
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::v128), curr, "load_splat must have type v128");
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "load_splat address must match memory index type");
  Type memAlignType = Type::none;
  switch (curr->op) {
    case Load8SplatVec128:
    case Load16SplatVec128:
    case Load32SplatVec128:
    case Load32ZeroVec128:
      memAlignType = Type::i32;
      break;
    case Load64SplatVec128:
    case Load8x8SVec128:
    case Load8x8UVec128:
    case Load16x4SVec128:
    case Load16x4UVec128:
    case Load32x2SVec128:
    case Load32x2UVec128:
    case Load64ZeroVec128:
      memAlignType = Type::i64;
      break;
  }
  Index bytes = curr->getMemBytes();
  validateOffset(curr->offset, memory, curr);
  validateAlignment(curr->align, memAlignType, bytes, /*isAtomic=*/false, curr);
}

void FunctionValidator::visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.SIMDLoadStoreLane memory must exist");
  shouldBeTrue(getModule()->features.hasSIMD(),
               curr,
               "SIMD operations require SIMD [--enable-simd]");
  if (curr->isLoad()) {
    shouldBeEqualOrFirstIsUnreachable(
      curr->type, Type(Type::v128), curr, "loadX_lane must have type v128");
  } else {
    shouldBeEqualOrFirstIsUnreachable(
      curr->type, Type(Type::none), curr, "storeX_lane must have type none");
  }
  shouldBeEqualOrFirstIsUnreachable(
    curr->ptr->type,
    memory->addressType,
    curr,
    "loadX_lane or storeX_lane address must match memory index type");
  shouldBeEqualOrFirstIsUnreachable(
    curr->vec->type,
    Type(Type::v128),
    curr,
    "loadX_lane or storeX_lane vector argument must have type v128");
  size_t lanes;
  Type memAlignType = Type::none;
  switch (curr->op) {
    case Load8LaneVec128:
    case Store8LaneVec128:
      lanes = 16;
      memAlignType = Type::i32;
      break;
    case Load16LaneVec128:
    case Store16LaneVec128:
      lanes = 8;
      memAlignType = Type::i32;
      break;
    case Load32LaneVec128:
    case Store32LaneVec128:
      lanes = 4;
      memAlignType = Type::i32;
      break;
    case Load64LaneVec128:
    case Store64LaneVec128:
      lanes = 2;
      memAlignType = Type::i64;
      break;
    default:
      WASM_UNREACHABLE("Unexpected SIMDLoadStoreLane op");
  }
  Index bytes = curr->getMemBytes();
  validateOffset(curr->offset, memory, curr);
  validateAlignment(curr->align, memAlignType, bytes, /*isAtomic=*/false, curr);
  shouldBeTrue(curr->index < lanes, curr, "invalid lane index");
}

void FunctionValidator::visitMemoryInit(MemoryInit* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(
    getModule()->features.hasBulkMemory(),
    curr,
    "Bulk memory operations require bulk memory [--enable-bulk-memory]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::none), curr, "memory.init must have type none");
  shouldBeEqualOrFirstIsUnreachable(
    curr->dest->type,
    memory->addressType,
    curr,
    "memory.init dest must match memory index type");
  shouldBeEqualOrFirstIsUnreachable(curr->offset->type,
                                    Type(Type::i32),
                                    curr,
                                    "memory.init offset must be an i32");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type, Type(Type::i32), curr, "memory.init size must be an i32");
  if (!shouldBeTrue(!!memory, curr, "memory.init memory must exist")) {
    return;
  }
  shouldBeTrue(getModule()->getDataSegmentOrNull(curr->segment),
               curr,
               "memory.init segment should exist");
}

void FunctionValidator::visitDataDrop(DataDrop* curr) {
  shouldBeTrue(
    getModule()->features.hasBulkMemory(),
    curr,
    "Bulk memory operations require bulk memory [--enable-bulk-memory]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::none), curr, "data.drop must have type none");
  shouldBeTrue(getModule()->getDataSegmentOrNull(curr->segment),
               curr,
               "data.drop segment should exist");
}

void FunctionValidator::visitMemoryCopy(MemoryCopy* curr) {
  shouldBeTrue(getModule()->features.hasBulkMemoryOpt(),
               curr,
               "memory.copy operations require bulk memory operations "
               "[--enable-bulk-memory-opt]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::none), curr, "memory.copy must have type none");
  auto* destMemory = getModule()->getMemoryOrNull(curr->destMemory);
  shouldBeTrue(!!destMemory, curr, "memory.copy destMemory must exist");
  auto* sourceMemory = getModule()->getMemoryOrNull(curr->sourceMemory);
  shouldBeTrue(!!sourceMemory, curr, "memory.copy sourceMemory must exist");
  shouldBeEqualOrFirstIsUnreachable(
    curr->dest->type,
    destMemory->addressType,
    curr,
    "memory.copy dest must match destMemory index type");
  shouldBeEqualOrFirstIsUnreachable(
    curr->source->type,
    sourceMemory->addressType,
    curr,
    "memory.copy source must match sourceMemory index type");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type,
    destMemory->addressType,
    curr,
    "memory.copy size must match destMemory index type");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type,
    sourceMemory->addressType,
    curr,
    "memory.copy size must match destMemory index type");
}

void FunctionValidator::visitMemoryFill(MemoryFill* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(
    getModule()->features.hasBulkMemoryOpt(),
    curr,
    "memory.fill operations require bulk memory [--enable-bulk-memory-opt]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::none), curr, "memory.fill must have type none");
  shouldBeEqualOrFirstIsUnreachable(
    curr->dest->type,
    memory->addressType,
    curr,
    "memory.fill dest must match memory index type");
  shouldBeEqualOrFirstIsUnreachable(curr->value->type,
                                    Type(Type::i32),
                                    curr,
                                    "memory.fill value must be an i32");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type,
    memory->addressType,
    curr,
    "memory.fill size must match memory index type");
  shouldBeTrue(!!memory, curr, "memory.fill memory must exist");
}

void FunctionValidator::validateMemBytes(uint8_t bytes,
                                         Type type,
                                         Expression* curr) {
  switch (type.getBasic()) {
    case Type::i32:
      shouldBeTrue(bytes == 1 || bytes == 2 || bytes == 4,
                   curr,
                   "expected i32 operation to touch 1, 2, or 4 bytes");
      break;
    case Type::i64:
      shouldBeTrue(bytes == 1 || bytes == 2 || bytes == 4 || bytes == 8,
                   curr,
                   "expected i64 operation to touch 1, 2, 4, or 8 bytes");
      break;
    case Type::f32:
      shouldBeTrue(bytes == 2 || bytes == 4,
                   curr,
                   "expected f32 operation to touch 2 or 4 bytes");
      break;
    case Type::f64:
      shouldBeEqual(
        bytes, uint8_t(8), curr, "expected f64 operation to touch 8 bytes");
      break;
    case Type::v128:
      shouldBeEqual(
        bytes, uint8_t(16), curr, "expected v128 operation to touch 16 bytes");
      break;
    case Type::unreachable:
      break;
    case Type::none:
      WASM_UNREACHABLE("unexpected type");
  }
}

void FunctionValidator::visitBinary(Binary* curr) {
  if (curr->left->type != Type::unreachable &&
      curr->right->type != Type::unreachable) {
    shouldBeEqual(curr->left->type,
                  curr->right->type,
                  curr,
                  "binary child types must be equal");
  }
  switch (curr->op) {
    case AddInt32:
    case SubInt32:
    case MulInt32:
    case DivSInt32:
    case DivUInt32:
    case RemSInt32:
    case RemUInt32:
    case AndInt32:
    case OrInt32:
    case XorInt32:
    case ShlInt32:
    case ShrUInt32:
    case ShrSInt32:
    case RotLInt32:
    case RotRInt32:
    case EqInt32:
    case NeInt32:
    case LtSInt32:
    case LtUInt32:
    case LeSInt32:
    case LeUInt32:
    case GtSInt32:
    case GtUInt32:
    case GeSInt32:
    case GeUInt32: {
      shouldBeEqualOrFirstIsUnreachable(
        curr->left->type, Type(Type::i32), curr, "i32 op");
      break;
    }
    case AddInt64:
    case SubInt64:
    case MulInt64:
    case DivSInt64:
    case DivUInt64:
    case RemSInt64:
    case RemUInt64:
    case AndInt64:
    case OrInt64:
    case XorInt64:
    case ShlInt64:
    case ShrUInt64:
    case ShrSInt64:
    case RotLInt64:
    case RotRInt64:
    case EqInt64:
    case NeInt64:
    case LtSInt64:
    case LtUInt64:
    case LeSInt64:
    case LeUInt64:
    case GtSInt64:
    case GtUInt64:
    case GeSInt64:
    case GeUInt64: {
      shouldBeEqualOrFirstIsUnreachable(
        curr->left->type, Type(Type::i64), curr, "i64 op");
      break;
    }
    case AddFloat32:
    case SubFloat32:
    case MulFloat32:
    case DivFloat32:
    case CopySignFloat32:
    case MinFloat32:
    case MaxFloat32:
    case EqFloat32:
    case NeFloat32:
    case LtFloat32:
    case LeFloat32:
    case GtFloat32:
    case GeFloat32: {
      shouldBeEqualOrFirstIsUnreachable(
        curr->left->type, Type(Type::f32), curr, "f32 op");
      break;
    }
    case AddFloat64:
    case SubFloat64:
    case MulFloat64:
    case DivFloat64:
    case CopySignFloat64:
    case MinFloat64:
    case MaxFloat64:
    case EqFloat64:
    case NeFloat64:
    case LtFloat64:
    case LeFloat64:
    case GtFloat64:
    case GeFloat64: {
      shouldBeEqualOrFirstIsUnreachable(
        curr->left->type, Type(Type::f64), curr, "f64 op");
      break;
    }
    case EqVecF16x8:
    case NeVecF16x8:
    case LtVecF16x8:
    case LeVecF16x8:
    case GtVecF16x8:
    case GeVecF16x8:
    case AddVecF16x8:
    case SubVecF16x8:
    case MulVecF16x8:
    case DivVecF16x8:
    case MinVecF16x8:
    case MaxVecF16x8:
    case PMinVecF16x8:
    case PMaxVecF16x8:
      shouldBeTrue(getModule()->features.hasFP16(),
                   curr,
                   "FP16 operations require FP16 [--enable-fp16]");
      [[fallthrough]];
    case EqVecI8x16:
    case NeVecI8x16:
    case LtSVecI8x16:
    case LtUVecI8x16:
    case LeSVecI8x16:
    case LeUVecI8x16:
    case GtSVecI8x16:
    case GtUVecI8x16:
    case GeSVecI8x16:
    case GeUVecI8x16:
    case EqVecI16x8:
    case NeVecI16x8:
    case LtSVecI16x8:
    case LtUVecI16x8:
    case LeSVecI16x8:
    case LeUVecI16x8:
    case GtSVecI16x8:
    case GtUVecI16x8:
    case GeSVecI16x8:
    case GeUVecI16x8:
    case EqVecI32x4:
    case NeVecI32x4:
    case LtSVecI32x4:
    case LtUVecI32x4:
    case LeSVecI32x4:
    case LeUVecI32x4:
    case GtSVecI32x4:
    case GtUVecI32x4:
    case GeSVecI32x4:
    case GeUVecI32x4:
    case EqVecI64x2:
    case NeVecI64x2:
    case LtSVecI64x2:
    case LeSVecI64x2:
    case GtSVecI64x2:
    case GeSVecI64x2:
    case EqVecF32x4:
    case NeVecF32x4:
    case LtVecF32x4:
    case LeVecF32x4:
    case GtVecF32x4:
    case GeVecF32x4:
    case EqVecF64x2:
    case NeVecF64x2:
    case LtVecF64x2:
    case LeVecF64x2:
    case GtVecF64x2:
    case GeVecF64x2:
    case AndVec128:
    case OrVec128:
    case XorVec128:
    case AndNotVec128:
    case AddVecI8x16:
    case AddSatSVecI8x16:
    case AddSatUVecI8x16:
    case SubVecI8x16:
    case SubSatSVecI8x16:
    case SubSatUVecI8x16:
    case MinSVecI8x16:
    case MinUVecI8x16:
    case MaxSVecI8x16:
    case MaxUVecI8x16:
    case AvgrUVecI8x16:
    case Q15MulrSatSVecI16x8:
    case ExtMulLowSVecI16x8:
    case ExtMulHighSVecI16x8:
    case ExtMulLowUVecI16x8:
    case ExtMulHighUVecI16x8:
    case AddVecI16x8:
    case AddSatSVecI16x8:
    case AddSatUVecI16x8:
    case SubVecI16x8:
    case SubSatSVecI16x8:
    case SubSatUVecI16x8:
    case MulVecI16x8:
    case MinSVecI16x8:
    case MinUVecI16x8:
    case MaxSVecI16x8:
    case MaxUVecI16x8:
    case AvgrUVecI16x8:
    case AddVecI32x4:
    case SubVecI32x4:
    case MulVecI32x4:
    case MinSVecI32x4:
    case MinUVecI32x4:
    case MaxSVecI32x4:
    case MaxUVecI32x4:
    case DotSVecI16x8ToVecI32x4:
    case ExtMulLowSVecI32x4:
    case ExtMulHighSVecI32x4:
    case ExtMulLowUVecI32x4:
    case ExtMulHighUVecI32x4:
    case AddVecI64x2:
    case SubVecI64x2:
    case MulVecI64x2:
    case ExtMulLowSVecI64x2:
    case ExtMulHighSVecI64x2:
    case ExtMulLowUVecI64x2:
    case ExtMulHighUVecI64x2:
    case AddVecF32x4:
    case SubVecF32x4:
    case MulVecF32x4:
    case DivVecF32x4:
    case MinVecF32x4:
    case MaxVecF32x4:
    case PMinVecF32x4:
    case PMaxVecF32x4:
    case RelaxedMinVecF32x4:
    case RelaxedMaxVecF32x4:
    case AddVecF64x2:
    case SubVecF64x2:
    case MulVecF64x2:
    case DivVecF64x2:
    case MinVecF64x2:
    case MaxVecF64x2:
    case PMinVecF64x2:
    case PMaxVecF64x2:
    case RelaxedMinVecF64x2:
    case RelaxedMaxVecF64x2:
    case NarrowSVecI16x8ToVecI8x16:
    case NarrowUVecI16x8ToVecI8x16:
    case NarrowSVecI32x4ToVecI16x8:
    case NarrowUVecI32x4ToVecI16x8:
    case SwizzleVecI8x16:
    case RelaxedSwizzleVecI8x16:
    case RelaxedQ15MulrSVecI16x8:
    case DotI8x16I7x16SToVecI16x8: {
      shouldBeEqualOrFirstIsUnreachable(
        curr->left->type, Type(Type::v128), curr, "v128 op");
      shouldBeEqualOrFirstIsUnreachable(
        curr->right->type, Type(Type::v128), curr, "v128 op");
      break;
    }
    case InvalidBinary:
      WASM_UNREACHABLE("invliad binary op");
  }
  shouldBeTrue(Features::get(curr->op) <= getModule()->features,
               curr,
               "all used features should be allowed");
}

void FunctionValidator::visitUnary(Unary* curr) {
  shouldBeUnequal(curr->value->type,
                  Type(Type::none),
                  curr,
                  "unaries must not receive a none as their input");
  if (curr->value->type == Type::unreachable) {
    return; // nothing to check
  }
  switch (curr->op) {
    case ClzInt32:
    case CtzInt32:
    case PopcntInt32: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i32),
                    curr,
                    "i32 unary value type must be correct");
      break;
    }
    case ClzInt64:
    case CtzInt64:
    case PopcntInt64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i64),
                    curr,
                    "i64 unary value type must be correct");
      break;
    }
    case NegFloat32:
    case AbsFloat32:
    case CeilFloat32:
    case FloorFloat32:
    case TruncFloat32:
    case NearestFloat32:
    case SqrtFloat32: {
      shouldBeEqual(curr->value->type,
                    Type(Type::f32),
                    curr,
                    "f32 unary value type must be correct");
      break;
    }
    case NegFloat64:
    case AbsFloat64:
    case CeilFloat64:
    case FloorFloat64:
    case TruncFloat64:
    case NearestFloat64:
    case SqrtFloat64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::f64),
                    curr,
                    "f64 unary value type must be correct");
      break;
    }
    case EqZInt32: {
      shouldBeTrue(
        curr->value->type == Type::i32, curr, "i32.eqz input must be i32");
      break;
    }
    case EqZInt64: {
      shouldBeTrue(curr->value->type == Type(Type::i64),
                   curr,
                   "i64.eqz input must be i64");
      break;
    }
    case ExtendSInt32:
    case ExtendUInt32:
    case ExtendS8Int32:
    case ExtendS16Int32: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i32),
                    curr,
                    "extend type must be correct");
      break;
    }
    case ExtendS8Int64:
    case ExtendS16Int64:
    case ExtendS32Int64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i64),
                    curr,
                    "extend type must be correct");
      break;
    }
    case WrapInt64: {
      shouldBeEqual(
        curr->value->type, Type(Type::i64), curr, "wrap type must be correct");
      break;
    }
    case TruncSFloat32ToInt32:
    case TruncSFloat32ToInt64:
    case TruncUFloat32ToInt32:
    case TruncUFloat32ToInt64: {
      shouldBeEqual(
        curr->value->type, Type(Type::f32), curr, "trunc type must be correct");
      break;
    }
    case TruncSatSFloat32ToInt32:
    case TruncSatSFloat32ToInt64:
    case TruncSatUFloat32ToInt32:
    case TruncSatUFloat32ToInt64: {
      shouldBeEqual(
        curr->value->type, Type(Type::f32), curr, "trunc type must be correct");
      break;
    }
    case TruncSFloat64ToInt32:
    case TruncSFloat64ToInt64:
    case TruncUFloat64ToInt32:
    case TruncUFloat64ToInt64: {
      shouldBeEqual(
        curr->value->type, Type(Type::f64), curr, "trunc type must be correct");
      break;
    }
    case TruncSatSFloat64ToInt32:
    case TruncSatSFloat64ToInt64:
    case TruncSatUFloat64ToInt32:
    case TruncSatUFloat64ToInt64: {
      shouldBeEqual(
        curr->value->type, Type(Type::f64), curr, "trunc type must be correct");
      break;
    }
    case ReinterpretFloat32: {
      shouldBeEqual(curr->value->type,
                    Type(Type::f32),
                    curr,
                    "reinterpret/f32 type must be correct");
      break;
    }
    case ReinterpretFloat64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::f64),
                    curr,
                    "reinterpret/f64 type must be correct");
      break;
    }
    case ConvertUInt32ToFloat32:
    case ConvertUInt32ToFloat64:
    case ConvertSInt32ToFloat32:
    case ConvertSInt32ToFloat64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i32),
                    curr,
                    "convert type must be correct");
      break;
    }
    case ConvertUInt64ToFloat32:
    case ConvertUInt64ToFloat64:
    case ConvertSInt64ToFloat32:
    case ConvertSInt64ToFloat64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i64),
                    curr,
                    "convert type must be correct");
      break;
    }
    case PromoteFloat32: {
      shouldBeEqual(curr->value->type,
                    Type(Type::f32),
                    curr,
                    "promote type must be correct");
      break;
    }
    case DemoteFloat64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::f64),
                    curr,
                    "demote type must be correct");
      break;
    }
    case ReinterpretInt32: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i32),
                    curr,
                    "reinterpret/i32 type must be correct");
      break;
    }
    case ReinterpretInt64: {
      shouldBeEqual(curr->value->type,
                    Type(Type::i64),
                    curr,
                    "reinterpret/i64 type must be correct");
      break;
    }
    case SplatVecI8x16:
    case SplatVecI16x8:
    case SplatVecI32x4:
      shouldBeEqual(
        curr->type, Type(Type::v128), curr, "expected splat to have v128 type");
      shouldBeEqual(
        curr->value->type, Type(Type::i32), curr, "expected i32 splat value");
      break;
    case SplatVecI64x2:
      shouldBeEqual(
        curr->type, Type(Type::v128), curr, "expected splat to have v128 type");
      shouldBeEqual(
        curr->value->type, Type(Type::i64), curr, "expected i64 splat value");
      break;
    case SplatVecF16x8:
      shouldBeTrue(getModule()->features.hasFP16(),
                   curr,
                   "FP16 operations require FP16 [--enable-fp16]");
      [[fallthrough]];
    case SplatVecF32x4:
      shouldBeEqual(
        curr->type, Type(Type::v128), curr, "expected splat to have v128 type");
      shouldBeEqual(
        curr->value->type, Type(Type::f32), curr, "expected f32 splat value");
      break;
    case SplatVecF64x2:
      shouldBeEqual(
        curr->type, Type(Type::v128), curr, "expected splat to have v128 type");
      shouldBeEqual(
        curr->value->type, Type(Type::f64), curr, "expected f64 splat value");
      break;
    case AbsVecF16x8:
    case NegVecF16x8:
    case SqrtVecF16x8:
    case CeilVecF16x8:
    case FloorVecF16x8:
    case TruncVecF16x8:
    case NearestVecF16x8:
      shouldBeTrue(getModule()->features.hasFP16(),
                   curr,
                   "FP16 operations require FP16 [--enable-fp16]");
      [[fallthrough]];
    case NotVec128:
    case PopcntVecI8x16:
    case AbsVecI8x16:
    case AbsVecI16x8:
    case AbsVecI32x4:
    case AbsVecI64x2:
    case NegVecI8x16:
    case NegVecI16x8:
    case NegVecI32x4:
    case NegVecI64x2:
    case AbsVecF32x4:
    case NegVecF32x4:
    case SqrtVecF32x4:
    case CeilVecF32x4:
    case FloorVecF32x4:
    case TruncVecF32x4:
    case NearestVecF32x4:
    case AbsVecF64x2:
    case NegVecF64x2:
    case SqrtVecF64x2:
    case CeilVecF64x2:
    case FloorVecF64x2:
    case TruncVecF64x2:
    case NearestVecF64x2:
    case ExtAddPairwiseSVecI8x16ToI16x8:
    case ExtAddPairwiseUVecI8x16ToI16x8:
    case ExtAddPairwiseSVecI16x8ToI32x4:
    case ExtAddPairwiseUVecI16x8ToI32x4:
    case TruncSatSVecF32x4ToVecI32x4:
    case TruncSatUVecF32x4ToVecI32x4:
    case ConvertSVecI32x4ToVecF32x4:
    case ConvertUVecI32x4ToVecF32x4:
    case ExtendLowSVecI8x16ToVecI16x8:
    case ExtendHighSVecI8x16ToVecI16x8:
    case ExtendLowUVecI8x16ToVecI16x8:
    case ExtendHighUVecI8x16ToVecI16x8:
    case ExtendLowSVecI16x8ToVecI32x4:
    case ExtendHighSVecI16x8ToVecI32x4:
    case ExtendLowUVecI16x8ToVecI32x4:
    case ExtendHighUVecI16x8ToVecI32x4:
    case ExtendLowSVecI32x4ToVecI64x2:
    case ExtendHighSVecI32x4ToVecI64x2:
    case ExtendLowUVecI32x4ToVecI64x2:
    case ExtendHighUVecI32x4ToVecI64x2:
    case ConvertLowSVecI32x4ToVecF64x2:
    case ConvertLowUVecI32x4ToVecF64x2:
    case TruncSatZeroSVecF64x2ToVecI32x4:
    case TruncSatZeroUVecF64x2ToVecI32x4:
    case DemoteZeroVecF64x2ToVecF32x4:
    case PromoteLowVecF32x4ToVecF64x2:
    case RelaxedTruncSVecF32x4ToVecI32x4:
    case RelaxedTruncUVecF32x4ToVecI32x4:
    case RelaxedTruncZeroSVecF64x2ToVecI32x4:
    case RelaxedTruncZeroUVecF64x2ToVecI32x4:
    case TruncSatSVecF16x8ToVecI16x8:
    case TruncSatUVecF16x8ToVecI16x8:
    case ConvertSVecI16x8ToVecF16x8:
    case ConvertUVecI16x8ToVecF16x8:
      shouldBeEqual(curr->type, Type(Type::v128), curr, "expected v128 type");
      shouldBeEqual(
        curr->value->type, Type(Type::v128), curr, "expected v128 operand");
      break;
    case AnyTrueVec128:
    case AllTrueVecI8x16:
    case AllTrueVecI16x8:
    case AllTrueVecI32x4:
    case AllTrueVecI64x2:
    case BitmaskVecI8x16:
    case BitmaskVecI16x8:
    case BitmaskVecI32x4:
    case BitmaskVecI64x2:
      shouldBeEqual(curr->type, Type(Type::i32), curr, "expected i32 type");
      shouldBeEqual(
        curr->value->type, Type(Type::v128), curr, "expected v128 operand");
      break;
    case InvalidUnary:
      WASM_UNREACHABLE("invalid unary op");
  }
  shouldBeTrue(Features::get(curr->op) <= getModule()->features,
               curr,
               "all used features should be allowed");
}

void FunctionValidator::visitSelect(Select* curr) {
  shouldBeUnequal(
    curr->ifFalse->type, Type(Type::none), curr, "select right must be valid");
  shouldBeUnequal(
    curr->type, Type(Type::none), curr, "select type must be valid");
  shouldBeTrue(curr->condition->type == Type::unreachable ||
                 curr->condition->type == Type::i32,
               curr,
               "select condition must be valid");
  if (curr->ifTrue->type != Type::unreachable) {
    shouldBeFalse(
      curr->ifTrue->type.isTuple(), curr, "select value may not be a tuple");
  }
  if (curr->ifFalse->type != Type::unreachable) {
    shouldBeFalse(
      curr->ifFalse->type.isTuple(), curr, "select value may not be a tuple");
  }
  if (curr->type != Type::unreachable) {
    shouldBeTrue(Type::isSubType(curr->ifTrue->type, curr->type),
                 curr,
                 "select's left expression must be subtype of select's type");
    shouldBeTrue(Type::isSubType(curr->ifFalse->type, curr->type),
                 curr,
                 "select's right expression must be subtype of select's type");
  }
}

void FunctionValidator::visitDrop(Drop* curr) {
  shouldBeTrue(curr->value->type.isConcrete() ||
                 curr->value->type == Type::unreachable,
               curr,
               "can only drop a valid value");
  if (curr->value->type.isTuple()) {
    shouldBeTrue(getModule()->features.hasMultivalue(),
                 curr,
                 "Tuples drops are not allowed unless multivalue is enabled");
  }
}

void FunctionValidator::visitReturn(Return* curr) {
  auto* func = getFunction();
  if (!shouldBeTrue(!!func, curr, "return must be within a function")) {
    return;
  }
  auto results = func->getResults();
  if (results.isConcrete()) {
    if (!shouldBeTrue(
          curr->value, curr, "concrete return should have a value")) {
      return;
    }
    shouldBeSubType(
      curr->value->type,
      results,
      curr,
      "return value should be a subtype of the function result type");
  } else {
    shouldBeTrue(!curr->value, curr, "return should not have a value");
  }
}

void FunctionValidator::visitMemorySize(MemorySize* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.size memory must exist");
}

void FunctionValidator::visitMemoryGrow(MemoryGrow* curr) {
  auto* memory = getModule()->getMemoryOrNull(curr->memory);
  shouldBeTrue(!!memory, curr, "memory.grow memory must exist");
  shouldBeEqualOrFirstIsUnreachable(curr->delta->type,
                                    memory->addressType,
                                    curr,
                                    "memory.grow must match memory index type");
}

void FunctionValidator::visitRefNull(RefNull* curr) {
  // If we are not in a function, this is a global location like a table. We
  // allow RefNull there as we represent tables that way regardless of what
  // features are enabled.
  auto feats = curr->type.getFeatures();
  if (!shouldBeTrue(!getFunction() || feats <= getModule()->features,
                    curr,
                    "ref.null requires additional features")) {
    getStream() << getMissingFeaturesList(*getModule(), feats) << '\n';
  }
  if (!shouldBeTrue(
        curr->type.isNullable(), curr, "ref.null types must be nullable")) {
    return;
  }
  shouldBeTrue(
    curr->type.isNull(), curr, "ref.null must have a bottom heap type");
}

void FunctionValidator::visitRefIsNull(RefIsNull* curr) {
  shouldBeTrue(
    getModule()->features.hasReferenceTypes(),
    curr,
    "ref.is_null requires reference-types [--enable-reference-types]");
  shouldBeTrue(curr->value->type == Type::unreachable ||
                 curr->value->type.isRef(),
               curr->value,
               "ref.is_null's argument should be a reference type");
}

void FunctionValidator::visitRefAs(RefAs* curr) {
  if (curr->value->type != Type::unreachable &&
      !shouldBeTrue(
        curr->value->type.isRef(), curr, "ref.as value must be reference")) {
    return;
  }
  switch (curr->op) {
    case RefAsNonNull: {
      shouldBeTrue(
        getModule()->features.hasReferenceTypes(),
        curr,
        "ref.as requires reference-types [--enable-reference-types]");
      break;
    }
    case AnyConvertExtern: {
      shouldBeTrue(getModule()->features.hasGC(),
                   curr,
                   "any.convert_extern requries GC [--enable-gc]");
      if (curr->type == Type::unreachable) {
        return;
      }
      shouldBeSubTypeIgnoringShared(
        curr->value->type,
        Type(HeapType::ext, Nullable),
        curr->value,
        "any.convert_extern value should be an externref");
      break;
    }
    case ExternConvertAny: {
      shouldBeTrue(getModule()->features.hasGC(),
                   curr,
                   "extern.convert_any requries GC [--enable-gc]");
      if (curr->type == Type::unreachable) {
        return;
      }
      shouldBeSubTypeIgnoringShared(
        curr->value->type,
        Type(HeapType::any, Nullable),
        curr->value,
        "extern.convert_any value should be an anyref");
      break;
    }
  }
}

void FunctionValidator::visitRefFunc(RefFunc* curr) {
  // If we are not in a function, this is a global location like a table. We
  // allow RefFunc there as we represent tables that way regardless of what
  // features are enabled.
  shouldBeTrue(!getFunction() || getModule()->features.hasReferenceTypes(),
               curr,
               "ref.func requires reference-types [--enable-reference-types]");
  if (!info.validateGlobally) {
    return;
  }
  auto* func = getModule()->getFunctionOrNull(curr->func);
  shouldBeTrue(!!func, curr, "function argument of ref.func must exist");
  shouldBeTrue(curr->type.isFunction(),
               curr,
               "ref.func must have a function reference type");
  shouldBeTrue(
    !curr->type.isNullable(), curr, "ref.func must have non-nullable type");
  // TODO: verify it also has a typed function references type, and the right
  // one,
  //   curr->type.getHeapType().getSignature()
  // That is blocked on having the ability to create signature types in the C
  // API (for now those users create the type with funcref). This also needs to
  // be fixed in LegalizeJSInterface and FuncCastEmulation and other places that
  // update function types.
  // TODO: check for non-nullability
}

void FunctionValidator::visitRefEq(RefEq* curr) {
  Type eqref = Type(HeapType::eq, Nullable);
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "ref.eq requires gc [--enable-gc]");
  shouldBeSubTypeIgnoringShared(
    curr->left->type,
    eqref,
    curr->left,
    "ref.eq's left argument should be a subtype of eqref");
  shouldBeSubTypeIgnoringShared(
    curr->right->type,
    eqref,
    curr->right,
    "ref.eq's right argument should be a subtype of eqref");
  if (curr->left->type.isRef() && curr->right->type.isRef()) {
    shouldBeEqual(curr->left->type.getHeapType().getShared(),
                  curr->right->type.getHeapType().getShared(),
                  curr,
                  "ref.eq operands must have the same shareability");
  }
}

void FunctionValidator::visitTableGet(TableGet* curr) {
  shouldBeTrue(getModule()->features.hasReferenceTypes(),
               curr,
               "table.get requires reference types [--enable-reference-types]");
  auto* table = getModule()->getTableOrNull(curr->table);
  if (shouldBeTrue(!!table, curr, "table.get table must exist")) {
    if (curr->type != Type::unreachable) {
      shouldBeEqual(curr->type,
                    table->type,
                    curr,
                    "table.get must have same type as table.");
    }
    shouldBeEqualOrFirstIsUnreachable(
      curr->index->type,
      table->addressType,
      curr,
      "table.get index must match the table index type.");
  }
}

void FunctionValidator::visitTableSet(TableSet* curr) {
  shouldBeTrue(getModule()->features.hasReferenceTypes(),
               curr,
               "table.set requires reference types [--enable-reference-types]");
  auto* table = getModule()->getTableOrNull(curr->table);
  if (shouldBeTrue(!!table, curr, "table.set table must exist")) {
    if (curr->type != Type::unreachable) {
      shouldBeSubType(curr->value->type,
                      table->type,
                      curr,
                      "table.set value must have right type");
    }
    shouldBeEqualOrFirstIsUnreachable(
      curr->index->type,
      table->addressType,
      curr,
      "table.set index must match the table index type.");
  }
}

void FunctionValidator::visitTableSize(TableSize* curr) {
  shouldBeTrue(
    getModule()->features.hasReferenceTypes(),
    curr,
    "table.size requires reference types [--enable-reference-types]");
  auto* table = getModule()->getTableOrNull(curr->table);
  shouldBeTrue(!!table, curr, "table.size table must exist");
}

void FunctionValidator::visitTableGrow(TableGrow* curr) {
  shouldBeTrue(
    getModule()->features.hasReferenceTypes(),
    curr,
    "table.grow requires reference types [--enable-reference-types]");
  auto* table = getModule()->getTableOrNull(curr->table);
  if (shouldBeTrue(!!table, curr, "table.grow table must exist") &&
      curr->type != Type::unreachable) {
    shouldBeSubType(curr->value->type,
                    table->type,
                    curr,
                    "table.grow value must have right type");
    shouldBeEqual(curr->delta->type,
                  table->addressType,
                  curr,
                  "table.grow must match table index type");
  }
}

void FunctionValidator::visitTableFill(TableFill* curr) {
  shouldBeTrue(getModule()->features.hasBulkMemory() &&
                 getModule()->features.hasReferenceTypes(),
               curr,
               "table.fill requires bulk-memory [--enable-bulk-memory] and "
               "reference-types [--enable-reference-types]");
  auto* table = getModule()->getTableOrNull(curr->table);
  if (shouldBeTrue(!!table, curr, "table.fill table must exist")) {
    shouldBeSubType(curr->value->type,
                    table->type,
                    curr,
                    "table.fill value must have right type");
    shouldBeEqualOrFirstIsUnreachable(
      curr->dest->type,
      table->addressType,
      curr,
      "table.fill dest must match table index type");
    shouldBeEqualOrFirstIsUnreachable(
      curr->size->type,
      table->addressType,
      curr,
      "table.fill size must match table index type");
  }
}

void FunctionValidator::visitTableCopy(TableCopy* curr) {
  shouldBeTrue(getModule()->features.hasBulkMemory(),
               curr,
               "table.copy requires bulk-memory [--enable-bulk-memory]");
  auto* sourceTable = getModule()->getTableOrNull(curr->sourceTable);
  auto* destTable = getModule()->getTableOrNull(curr->destTable);
  if (shouldBeTrue(!!sourceTable, curr, "table.copy source table must exist") &&
      shouldBeTrue(!!destTable, curr, "table.copy dest table must exist")) {
    shouldBeSubType(sourceTable->type,
                    destTable->type,
                    curr,
                    "table.copy source must have right type for dest");
  }
  shouldBeEqualOrFirstIsUnreachable(curr->dest->type,
                                    destTable->addressType,
                                    curr,
                                    "table.copy dest must be valid");
  shouldBeEqualOrFirstIsUnreachable(curr->source->type,
                                    sourceTable->addressType,
                                    curr,
                                    "table.copy source must be valid");
  Type sizeType =
    sourceTable->is64() && destTable->is64() ? Type::i64 : Type::i32;
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type, sizeType, curr, "table.copy size must be valid");
}

void FunctionValidator::visitTableInit(TableInit* curr) {
  shouldBeTrue(getModule()->features.hasBulkMemory(),
               curr,
               "table.init requires bulk-memory [--enable-bulk-memory]");
  auto* segment = getModule()->getElementSegment(curr->segment);
  auto* table = getModule()->getTableOrNull(curr->table);
  if (shouldBeTrue(!!segment, curr, "table.init segment must exist") &&
      shouldBeTrue(!!table, curr, "table.init table must exist")) {
    shouldBeSubType(segment->type,
                    table->type,
                    curr,
                    "table.init source must have right type for dest");
  }
  shouldBeEqualOrFirstIsUnreachable(curr->dest->type,
                                    table->addressType,
                                    curr,
                                    "table.init dest must be valid");
  shouldBeEqualOrFirstIsUnreachable(curr->offset->type,
                                    Type(Type::i32),
                                    curr,
                                    "table.init offset must be valid");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type, Type(Type::i32), curr, "table.init size must be valid");
}

void FunctionValidator::noteDelegate(Name name, Expression* curr) {
  if (name != DELEGATE_CALLER_TARGET) {
    shouldBeTrue(delegateTargetNames.count(name) != 0,
                 curr,
                 "all delegate targets must be valid");
  }
}

void FunctionValidator::noteRethrow(Name name, Expression* curr) {
  shouldBeTrue(rethrowTargetNames.count(name) != 0,
               curr,
               "all rethrow targets must be valid");
}

void FunctionValidator::visitTry(Try* curr) {
  shouldBeTrue(getModule()->features.hasExceptionHandling(),
               curr,
               "try requires exception-handling [--enable-exception-handling]");
  if (curr->name.is()) {
    noteLabelName(curr->name);
  }
  if (curr->type != Type::unreachable) {
    shouldBeSubType(curr->body->type,
                    curr->type,
                    curr->body,
                    "try's type does not match try body's type");
    for (auto catchBody : curr->catchBodies) {
      shouldBeSubType(catchBody->type,
                      curr->type,
                      catchBody,
                      "try's type does not match catch's body type");
    }
  } else {
    shouldBeEqual(curr->body->type,
                  Type(Type::unreachable),
                  curr,
                  "unreachable try-catch must have unreachable try body");
    for (auto catchBody : curr->catchBodies) {
      shouldBeEqual(catchBody->type,
                    Type(Type::unreachable),
                    curr,
                    "unreachable try-catch must have unreachable catch body");
    }
  }
  shouldBeTrue(curr->catchBodies.size() - curr->catchTags.size() <= 1,
               curr,
               "the number of catch blocks and tags do not match");

  shouldBeFalse(curr->isCatch() && curr->isDelegate(),
                curr,
                "try cannot have both catch and delegate at the same time");

  for (Index i = 0; i < curr->catchTags.size(); i++) {
    Name tagName = curr->catchTags[i];
    auto* tag = getModule()->getTagOrNull(tagName);
    if (!shouldBeTrue(tag != nullptr, curr, "")) {
      getStream() << "tag name is invalid: " << tagName << "\n";
    } else if (!shouldBeEqual(tag->sig.results, Type(Type::none), curr, "")) {
      getStream()
        << "catch's tag (" << tagName
        << ") has result values, which is not allowed for exception handling";
    } else {
      auto* catchBody = curr->catchBodies[i];
      auto pops = EHUtils::findPops(catchBody);
      if (tag->sig.params == Type::none) {
        if (!shouldBeTrue(pops.empty(), curr, "")) {
          getStream() << "catch's tag (" << tagName
                      << ") doesn't have any params, but there are pops";
        }
      } else {
        if (shouldBeTrue(pops.size() == 1, curr, "")) {
          auto* pop = *pops.begin();
          if (!shouldBeSubType(tag->sig.params, pop->type, curr, "")) {
            getStream()
              << "catch's tag (" << tagName
              << ")'s pop doesn't have the same type as the tag's params";
          }
          if (!shouldBeTrue(
                EHUtils::containsValidDanglingPop(catchBody), curr, "")) {
            getStream() << "catch's body (" << tagName
                        << ")'s pop's location is not valid";
          }
        } else {
          getStream() << "catch's tag (" << tagName
                      << ") has params, so there should be a single pop within "
                         "the catch body";
        }
      }
    }
  }

  if (curr->hasCatchAll()) {
    auto* catchAllBody = curr->catchBodies.back();
    shouldBeTrue(EHUtils::findPops(catchAllBody).empty(),
                 curr,
                 "catch_all's body should not have pops");
  }

  if (curr->isDelegate()) {
    noteDelegate(curr->delegateTarget, curr);
  }

  rethrowTargetNames.erase(curr->name);
}

void FunctionValidator::visitTryTable(TryTable* curr) {
  shouldBeTrue(
    getModule()->features.hasExceptionHandling(),
    curr,
    "try_table requires exception-handling [--enable-exception-handling]");
  if (curr->type != Type::unreachable) {
    shouldBeSubType(curr->body->type,
                    curr->type,
                    curr->body,
                    "try_table's type does not match try_table body's type");
  }

  shouldBeEqual(curr->catchTags.size(),
                curr->catchDests.size(),
                curr,
                "the number of catch tags and catch destinations do not match");
  shouldBeEqual(curr->catchTags.size(),
                curr->catchRefs.size(),
                curr,
                "the number of catch tags and catch refs do not match");
  shouldBeEqual(curr->catchTags.size(),
                curr->sentTypes.size(),
                curr,
                "the number of catch tags and sent types do not match");

  const char* invalidSentTypeMsg = "invalid catch sent type information";
  Type exnref = Type(HeapType::exn, NonNullable);
  for (Index i = 0; i < curr->catchTags.size(); i++) {
    auto sentType = curr->sentTypes[i];
    size_t tagTypeSize;

    Name tagName = curr->catchTags[i];
    if (!tagName) { // catch_all or catch_all_ref
      tagTypeSize = 0;
    } else { // catch or catch_ref
      // Check tag validity
      auto* tag = getModule()->getTagOrNull(tagName);
      if (!shouldBeTrue(tag != nullptr, curr, "")) {
        getStream() << "catch's tag name is invalid: " << tagName << "\n";
      } else if (!shouldBeEqual(tag->sig.results, Type(Type::none), curr, "")) {
        getStream()
          << "catch's tag (" << tagName
          << ") has result values, which is not allowed for exception handling";
      }

      // tagType and sentType should be the same (except for the possible exnref
      // at the end of sentType)
      auto tagType = tag->sig.params;
      tagTypeSize = tagType.size();
      for (Index j = 0; j < tagType.size(); j++) {
        shouldBeEqual(tagType[j], sentType[j], curr, invalidSentTypeMsg);
      }
    }

    // If this is catch_ref or catch_all_ref, sentType.size() should be
    // tagType.size() + 1 because there is an exrnef tacked at the end. If
    // this is catch/catch_all, the two sizes should be the same.
    if (curr->catchRefs[i]) {
      if (shouldBeTrue(
            sentType.size() == tagTypeSize + 1, curr, invalidSentTypeMsg)) {
        shouldBeEqual(
          sentType[sentType.size() - 1], exnref, curr, invalidSentTypeMsg);
      }
    } else {
      shouldBeTrue(sentType.size() == tagTypeSize, curr, invalidSentTypeMsg);
    }

    // Note catch destinations with sent types
    noteBreak(curr->catchDests[i], curr->sentTypes[i], curr);
  }
}

void FunctionValidator::visitThrow(Throw* curr) {
  shouldBeTrue(
    getModule()->features.hasExceptionHandling(),
    curr,
    "throw requires exception-handling [--enable-exception-handling]");
  shouldBeEqual(curr->type,
                Type(Type::unreachable),
                curr,
                "throw's type must be unreachable");
  if (!info.validateGlobally) {
    return;
  }
  auto* tag = getModule()->getTagOrNull(curr->tag);
  if (!shouldBeTrue(!!tag, curr, "throw's tag must exist")) {
    return;
  }
  shouldBeEqual(
    tag->sig.results,
    Type(Type::none),
    curr,
    "tags with result types must not be used for exception handling");
  if (!shouldBeEqual(curr->operands.size(),
                     tag->sig.params.size(),
                     curr,
                     "tag's param numbers must match")) {
    return;
  }
  size_t i = 0;
  for (const auto& param : tag->sig.params) {
    if (!shouldBeSubType(curr->operands[i]->type,
                         param,
                         curr->operands[i],
                         "tag param types must match") &&
        !info.quiet) {
      getStream() << "(on argument " << i << ")\n";
    }
    ++i;
  }
}

void FunctionValidator::visitRethrow(Rethrow* curr) {
  shouldBeTrue(
    getModule()->features.hasExceptionHandling(),
    curr,
    "rethrow requires exception-handling [--enable-exception-handling]");
  shouldBeEqual(curr->type,
                Type(Type::unreachable),
                curr,
                "rethrow's type must be unreachable");
  noteRethrow(curr->target, curr);
}

void FunctionValidator::visitTupleMake(TupleMake* curr) {
  shouldBeTrue(getModule()->features.hasMultivalue(),
               curr,
               "Tuples are not allowed unless multivalue is enabled");
  shouldBeTrue(
    curr->operands.size() > 1, curr, "tuple.make must have multiple operands");
  std::vector<Type> types;
  for (auto* op : curr->operands) {
    if (op->type == Type::unreachable) {
      shouldBeTrue(
        curr->type == Type::unreachable,
        curr,
        "If tuple.make has an unreachable operand, it must be unreachable");
      return;
    }
    types.push_back(op->type);
  }
  shouldBeSubType(Type(types),
                  curr->type,
                  curr,
                  "Type of tuple.make does not match types of its operands");
}

void FunctionValidator::visitThrowRef(ThrowRef* curr) {
  Type exnref = Type(HeapType::exn, Nullable);
  shouldBeSubType(curr->exnref->type,
                  exnref,
                  curr,
                  "throw_ref's argument should be a subtype of exnref");
}

void FunctionValidator::visitTupleExtract(TupleExtract* curr) {
  shouldBeTrue(getModule()->features.hasMultivalue(),
               curr,
               "Tuples are not allowed unless multivalue is enabled");
  if (curr->tuple->type == Type::unreachable) {
    shouldBeTrue(
      curr->type == Type::unreachable,
      curr,
      "If tuple.extract has an unreachable operand, it must be unreachable");
  } else {
    bool inBounds = curr->index < curr->tuple->type.size();
    shouldBeTrue(inBounds, curr, "tuple.extract index out of bounds");
    if (inBounds) {
      shouldBeSubType(
        curr->tuple->type[curr->index],
        curr->type,
        curr,
        "tuple.extract type does not match the type of the extracted element");
    }
  }
}

void FunctionValidator::visitCallRef(CallRef* curr) {
  validateReturnCall(curr);
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "call_ref requires gc [--enable-gc]");
  if (curr->target->type == Type::unreachable ||
      (curr->target->type.isRef() &&
       curr->target->type.getHeapType().isMaybeShared(HeapType::nofunc))) {
    return;
  }
  if (shouldBeTrue(curr->target->type.isFunction(),
                   curr,
                   "call_ref target must be a function reference")) {
    validateCallParamsAndResult(curr, curr->target->type.getHeapType());
  }
}

void FunctionValidator::visitRefI31(RefI31* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "ref.i31 requires gc [--enable-gc]");
  if (curr->type.isRef() && curr->type.getHeapType().isShared()) {
    shouldBeTrue(
      getModule()->features.hasSharedEverything(),
      curr,
      "ref.i31_shared requires shared-everything [--enable-shared-everything]");
  }
  shouldBeSubType(curr->value->type,
                  Type::i32,
                  curr->value,
                  "ref.i31's argument should be i32");
}

void FunctionValidator::visitI31Get(I31Get* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "i31.get_s/u requires gc [--enable-gc]");
  shouldBeSubTypeIgnoringShared(curr->i31->type,
                                Type(HeapType::i31, Nullable),
                                curr->i31,
                                "i31.get_s/u's argument should be i31ref");
}

void FunctionValidator::visitRefTest(RefTest* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "ref.test requires gc [--enable-gc]");
  if (curr->ref->type == Type::unreachable) {
    return;
  }
  if (!shouldBeTrue(
        curr->ref->type.isRef(), curr, "ref.test ref must have ref type")) {
    return;
  }
  if (!shouldBeTrue(
        curr->castType.isRef(), curr, "ref.test target must have ref type")) {
    return;
  }
  shouldBeEqual(
    curr->castType.getHeapType().getBottom(),
    curr->ref->type.getHeapType().getBottom(),
    curr,
    "ref.test target type and ref type must have a common supertype");
}

void FunctionValidator::visitRefCast(RefCast* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "ref.cast requires gc [--enable-gc]");
  if (curr->ref->type == Type::unreachable) {
    return;
  }
  if (!shouldBeTrue(
        curr->ref->type.isRef(), curr, "ref.cast ref must have ref type")) {
    return;
  }
  // If the cast is unreachable but not the ref (we ruled out the former
  // earlier), then the cast is unreachable because the cast type had no
  // common supertype with the ref, which is invalid. This is the same as the
  // check below us, but we must do it first (as getHeapType fails otherwise).
  if (!shouldBeUnequal(
        curr->type,
        Type(Type::unreachable),
        curr,
        "ref.cast target type and ref type must have a common supertype")) {
    return;
  }
  // Also error (more generically) on i32 and anything else invalid here.
  if (!shouldBeTrue(curr->type.isRef(), curr, "ref.cast must have ref type")) {
    return;
  }
  shouldBeEqual(
    curr->type.getHeapType().getBottom(),
    curr->ref->type.getHeapType().getBottom(),
    curr,
    "ref.cast target type and ref type must have a common supertype");

  // We should never have a nullable cast of a non-nullable reference, since
  // that unnecessarily loses type information.
  shouldBeTrue(curr->ref->type.isNullable() || curr->type.isNonNullable(),
               curr,
               "ref.cast null of non-nullable references are not allowed");
}

void FunctionValidator::visitBrOn(BrOn* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "br_on_cast requires gc [--enable-gc]");
  if (curr->ref->type == Type::unreachable) {
    return;
  }
  if (!shouldBeTrue(
        curr->ref->type.isRef(), curr, "br_on_cast ref must have ref type")) {
    return;
  }
  if (curr->op == BrOnCast || curr->op == BrOnCastFail) {
    if (!shouldBeTrue(curr->castType.isRef(),
                      curr,
                      "br_on_cast must have reference cast type")) {
      return;
    }
    shouldBeEqual(
      curr->castType.getHeapType().getBottom(),
      curr->ref->type.getHeapType().getBottom(),
      curr,
      "br_on_cast* target type and ref type must have a common supertype");
    shouldBeSubType(
      curr->castType,
      curr->ref->type,
      curr,
      "br_on_cast* target type must be a subtype of its input type");
  } else {
    shouldBeEqual(curr->castType,
                  Type(Type::none),
                  curr,
                  "non-cast br_on* must not set intendedType field");
  }
  noteBreak(curr->name, curr->getSentType(), curr);
}

void FunctionValidator::visitStructNew(StructNew* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "struct.new requires gc [--enable-gc]");
  if (curr->type == Type::unreachable) {
    return;
  }
  auto heapType = curr->type.getHeapType();
  if (!shouldBeTrue(
        heapType.isStruct(), curr, "struct.new heap type must be struct")) {
    return;
  }
  const auto& fields = heapType.getStruct().fields;
  if (curr->isWithDefault()) {
    shouldBeTrue(curr->operands.empty(),
                 curr,
                 "struct.new_with_default should have no operands");
    // All the fields must be defaultable.
    for (const auto& field : fields) {
      shouldBeTrue(field.type.isDefaultable(),
                   field,
                   "struct.new_with_default value type must be defaultable");
    }
  } else {
    if (shouldBeEqual(curr->operands.size(),
                      fields.size(),
                      curr,
                      "struct.new must have the right number of operands")) {
      // All the fields must have the proper type.
      for (Index i = 0; i < fields.size(); i++) {
        if (!Type::isSubType(curr->operands[i]->type, fields[i].type)) {
          info.fail("struct.new operand " + std::to_string(i) +
                      " must have proper type",
                    curr,
                    getFunction());
        }
      }
    }
  }
}

void FunctionValidator::visitStructGet(StructGet* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "struct.get requires gc [--enable-gc]");
  if (curr->type == Type::unreachable || curr->ref->type.isNull()) {
    return;
  }
  if (!shouldBeTrue(curr->ref->type.isStruct(),
                    curr->ref,
                    "struct.get ref must be a struct")) {
    return;
  }
  const auto& fields = curr->ref->type.getHeapType().getStruct().fields;
  shouldBeTrue(curr->index < fields.size(), curr, "bad struct.get field");
  auto field = fields[curr->index];
  // If the type is not packed, it must be marked internally as unsigned, by
  // convention.
  if (field.type != Type::i32 || field.packedType == Field::not_packed) {
    shouldBeFalse(curr->signed_, curr, "non-packed get cannot be signed");
  }
  if (curr->ref->type == Type::unreachable) {
    return;
  }
  shouldBeEqual(
    curr->type, field.type, curr, "struct.get must have the proper type");
}

void FunctionValidator::visitStructSet(StructSet* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "struct.set requires gc [--enable-gc]");
  if (curr->ref->type == Type::unreachable) {
    return;
  }
  if (!shouldBeTrue(curr->ref->type.isRef(),
                    curr->ref,
                    "struct.set ref must be a reference type")) {
    return;
  }
  auto type = curr->ref->type.getHeapType();
  if (type.isMaybeShared(HeapType::none)) {
    return;
  }
  if (!shouldBeTrue(
        type.isStruct(), curr->ref, "struct.set ref must be a struct")) {
    return;
  }
  const auto& fields = type.getStruct().fields;
  shouldBeTrue(curr->index < fields.size(), curr, "bad struct.get field");
  auto& field = fields[curr->index];
  shouldBeSubType(curr->value->type,
                  field.type,
                  curr,
                  "struct.set must have the proper type");
  shouldBeEqual(
    field.mutable_, Mutable, curr, "struct.set field must be mutable");
}

void FunctionValidator::visitArrayNew(ArrayNew* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "array.new requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type, Type(Type::i32), curr, "array.new size must be an i32");
  if (curr->type == Type::unreachable) {
    return;
  }
  auto heapType = curr->type.getHeapType();
  if (!shouldBeTrue(
        heapType.isArray(), curr, "array.new heap type must be array")) {
    return;
  }
  const auto& element = heapType.getArray().element;
  if (curr->isWithDefault()) {
    shouldBeTrue(
      !curr->init, curr, "array.new_with_default should have no init");
    // The element must be defaultable.
    shouldBeTrue(element.type.isDefaultable(),
                 element,
                 "array.new_with_default value type must be defaultable");
  } else {
    shouldBeTrue(!!curr->init, curr, "array.new should have an init");
    // The inits must have the proper type.
    shouldBeSubType(curr->init->type,
                    element.type,
                    curr,
                    "array.new init must have proper type");
  }
}

template<typename ArrayNew>
void FunctionValidator::visitArrayNew(ArrayNew* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "array.new_{data, elem} requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->offset->type,
    Type(Type::i32),
    curr,
    "array.new_{data, elem} offset must be an i32");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type,
    Type(Type::i32),
    curr,
    "array.new_{data, elem} size must be an i32");
  if (curr->type == Type::unreachable) {
    return;
  }
  if (!shouldBeTrue(
        curr->type.isRef(),
        curr,
        "array.new_{data, elem} type should be an array reference")) {
    return;
  }
  auto heapType = curr->type.getHeapType();
  if (!shouldBeTrue(
        heapType.isArray(),
        curr,
        "array.new_{data, elem} type should be an array reference")) {
    return;
  }
}

void FunctionValidator::visitArrayNewData(ArrayNewData* curr) {
  visitArrayNew(curr);

  shouldBeTrue(
    getModule()->features.hasBulkMemory(),
    curr,
    "Data segment operations require bulk memory [--enable-bulk-memory]");
  if (!shouldBeTrue(getModule()->getDataSegment(curr->segment),
                    curr,
                    "array.new_data segment should exist")) {
    return;
  }

  auto field = GCTypeUtils::getField(curr->type);
  if (!field) {
    // A bottom type, or unreachable.
    return;
  }
  shouldBeTrue(field->type.isNumber(),
               curr,
               "array.new_data result element type should be numeric");
}

void FunctionValidator::visitArrayNewElem(ArrayNewElem* curr) {
  visitArrayNew(curr);

  if (!shouldBeTrue(getModule()->getElementSegment(curr->segment),
                    curr,
                    "array.new_elem segment should exist")) {
    return;
  }

  auto field = GCTypeUtils::getField(curr->type);
  if (!field) {
    // A bottom type, or unreachable.
    return;
  }
  shouldBeSubType(getModule()->getElementSegment(curr->segment)->type,
                  field->type,
                  curr,
                  "array.new_elem segment type should be a subtype of the "
                  "result element type");
}

void FunctionValidator::visitArrayNewFixed(ArrayNewFixed* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "array.init requires gc [--enable-gc]");
  if (curr->type == Type::unreachable) {
    return;
  }
  auto heapType = curr->type.getHeapType();
  if (!shouldBeTrue(
        heapType.isArray(), curr, "array.init heap type must be array")) {
    return;
  }
  const auto& element = heapType.getArray().element;
  for (auto* value : curr->values) {
    shouldBeSubType(value->type,
                    element.type,
                    curr,
                    "array.init value must have proper type");
  }
}

void FunctionValidator::visitArrayGet(ArrayGet* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "array.get requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->index->type, Type(Type::i32), curr, "array.get index must be an i32");
  const char* mustBeArray =
    "array.get target should be a specific array reference";
  if (curr->type == Type::unreachable ||
      !shouldBeTrue(curr->ref->type.isRef(), curr, mustBeArray) ||
      curr->ref->type.getHeapType().isBottom() ||
      !shouldBeTrue(curr->ref->type.isArray(), curr, mustBeArray)) {
    return;
  }
  auto heapType = curr->ref->type.getHeapType();
  const auto& element = heapType.getArray().element;
  // If the type is not packed, it must be marked internally as unsigned, by
  // convention.
  if (element.type != Type::i32 || element.packedType == Field::not_packed) {
    shouldBeFalse(curr->signed_, curr, "non-packed get cannot be signed");
  }
  shouldBeEqual(
    curr->type, element.type, curr, "array.get must have the proper type");
}

void FunctionValidator::visitArraySet(ArraySet* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "array.set requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->index->type, Type(Type::i32), curr, "array.set index must be an i32");
  if (curr->type == Type::unreachable) {
    return;
  }
  const char* mustBeArray = "array.set target should be an array reference";
  if (curr->type == Type::unreachable ||
      !shouldBeTrue(curr->ref->type.isRef(), curr, mustBeArray) ||
      curr->ref->type.getHeapType().isBottom() ||
      !shouldBeTrue(curr->ref->type.isArray(), curr, mustBeArray)) {
    return;
  }
  const auto& element = curr->ref->type.getHeapType().getArray().element;
  shouldBeSubType(curr->value->type,
                  element.type,
                  curr,
                  "array.set must have the proper type");
  shouldBeTrue(element.mutable_, curr, "array.set type must be mutable");
}

void FunctionValidator::visitArrayLen(ArrayLen* curr) {
  shouldBeTrue(
    getModule()->features.hasGC(), curr, "array.len requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(
    curr->type, Type(Type::i32), curr, "array.len result must be an i32");
  shouldBeSubTypeIgnoringShared(
    curr->ref->type,
    Type(HeapType::array, Nullable),
    curr,
    "array.len argument must be an array reference");
}

void FunctionValidator::visitArrayCopy(ArrayCopy* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "array.copy requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(curr->srcIndex->type,
                                    Type(Type::i32),
                                    curr,
                                    "array.copy src index must be an i32");
  shouldBeEqualOrFirstIsUnreachable(curr->destIndex->type,
                                    Type(Type::i32),
                                    curr,
                                    "array.copy dest index must be an i32");
  if (curr->type == Type::unreachable) {
    return;
  }
  if (!shouldBeTrue(curr->srcRef->type.isRef(),
                    curr,
                    "array.copy source should be a reference")) {
    return;
  }
  if (!shouldBeTrue(curr->destRef->type.isRef(),
                    curr,
                    "array.copy destination should be a reference")) {
    return;
  }
  auto srcHeapType = curr->srcRef->type.getHeapType();
  auto destHeapType = curr->destRef->type.getHeapType();
  // Normally both types need to be references to specific arrays, but if either
  // of the types are bottom, we don't further constrain the other at all
  // because this will be emitted as an unreachable.
  if (srcHeapType.isBottom() || destHeapType.isBottom()) {
    return;
  }
  if (!shouldBeTrue(srcHeapType.isArray(),
                    curr,
                    "array.copy source should be an array reference")) {
    return;
  }
  if (!shouldBeTrue(destHeapType.isArray(),
                    curr,
                    "array.copy destination should be an array reference")) {
    return;
  }
  const auto& srcElement = srcHeapType.getArray().element;
  const auto& destElement = destHeapType.getArray().element;
  shouldBeSubType(srcElement.type,
                  destElement.type,
                  curr,
                  "array.copy must have the proper types");
  shouldBeEqual(srcElement.packedType,
                destElement.packedType,
                curr,
                "array.copy types must match");
  shouldBeTrue(
    destElement.mutable_, curr, "array.copy destination must be mutable");
}

void FunctionValidator::visitArrayFill(ArrayFill* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "array.fill requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(curr->index->type,
                                    Type(Type::i32),
                                    curr,
                                    "array.fill index must be an i32");
  shouldBeEqualOrFirstIsUnreachable(
    curr->size->type, Type(Type::i32), curr, "array.fill size must be an i32");
  const char* mustBeArray =
    "array.fill destination should be an array reference";
  if (curr->type == Type::unreachable ||
      !shouldBeTrue(curr->ref->type.isRef(), curr, mustBeArray) ||
      curr->ref->type.getHeapType().isBottom() ||
      !shouldBeTrue(curr->ref->type.isArray(), curr, mustBeArray)) {
    return;
  }
  auto heapType = curr->ref->type.getHeapType();
  auto element = heapType.getArray().element;
  shouldBeSubType(curr->value->type,
                  element.type,
                  curr,
                  "array.fill value must match destination element type");
  shouldBeTrue(
    element.mutable_, curr, "array.fill destination must be mutable");
}

template<typename ArrayInit>
void FunctionValidator::visitArrayInit(ArrayInit* curr) {
  shouldBeTrue(getModule()->features.hasGC(),
               curr,
               "array.init_* requires gc [--enable-gc]");
  shouldBeEqualOrFirstIsUnreachable(curr->index->type,
                                    Type(Type::i32),
                                    curr,
                                    "array.init_* index must be an i32");
  shouldBeEqualOrFirstIsUnreachable(curr->offset->type,
                                    Type(Type::i32),
                                    curr,
                                    "array.init_* offset must be an i32");
  shouldBeEqualOrFirstIsUnreachable(curr->size->type,
                                    Type(Type::i32),
                                    curr,
                                    "array.init_* size must be an i32");
  const char* mustBeArray =
    "array.init_* destination must be an array reference";
  if (curr->type == Type::unreachable ||
      !shouldBeTrue(curr->ref->type.isRef(), curr, mustBeArray) ||
      curr->ref->type.getHeapType().isBottom() ||
      !shouldBeTrue(curr->ref->type.isArray(), curr, mustBeArray)) {
    return;
  }
  auto heapType = curr->ref->type.getHeapType();
  auto element = heapType.getArray().element;
  shouldBeTrue(
    element.mutable_, curr, "array.init_* destination must be mutable");
}

void FunctionValidator::visitArrayInitData(ArrayInitData* curr) {
  visitArrayInit(curr);

  shouldBeTrue(
    getModule()->features.hasBulkMemory(),
    curr,
    "Data segment operations require bulk memory [--enable-bulk-memory]");
  shouldBeTrue(getModule()->getDataSegmentOrNull(curr->segment),
               curr,
               "array.init_data segment must exist");

  auto field = GCTypeUtils::getField(curr->ref->type);
  if (!field) {
    // A bottom type, or unreachable.
    return;
  }
  shouldBeTrue(field->type.isNumber(),
               curr,
               "array.init_data destination must be numeric");
}

void FunctionValidator::visitArrayInitElem(ArrayInitElem* curr) {
  visitArrayInit(curr);

  auto* seg = getModule()->getElementSegmentOrNull(curr->segment);
  if (!shouldBeTrue(seg, curr, "array.init_elem segment must exist")) {
    return;
  }

  auto field = GCTypeUtils::getField(curr->ref->type);
  if (!field) {
    // A bottom type, or unreachable.
    return;
  }

  shouldBeSubType(seg->type,
                  field->type,
                  curr,
                  "array.init_elem segment type must match destination type");
}

void FunctionValidator::visitStringNew(StringNew* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");

  switch (curr->op) {
    case StringNewLossyUTF8Array:
    case StringNewWTF16Array: {
      auto refType = curr->ref->type;
      if (refType == Type::unreachable) {
        return;
      }
      if (!shouldBeTrue(
            refType.isRef(), curr, "string.new input must have array type")) {
        return;
      }
      auto heapType = refType.getHeapType();
      if (!shouldBeTrue(heapType.isBottom() || heapType.isArray(),
                        curr,
                        "string.new input must have array type")) {
        return;
      }
      shouldBeEqualOrFirstIsUnreachable(curr->start->type,
                                        Type(Type::i32),
                                        curr,
                                        "string.new start must be i32");
      shouldBeEqualOrFirstIsUnreachable(
        curr->end->type, Type(Type::i32), curr, "string.new end must be i32");
      return;
    }
    case StringNewFromCodePoint:
      shouldBeEqualOrFirstIsUnreachable(
        curr->ref->type,
        Type(Type::i32),
        curr,
        "string.from_code_point code point must be i32");
      shouldBeTrue(
        !curr->start, curr, "string.from_code_point should not have start");
      shouldBeTrue(
        !curr->end, curr, "string.from_code_point should not have end");
      return;
  }
  WASM_UNREACHABLE("unexpected op");
}

void FunctionValidator::visitStringConst(StringConst* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitStringMeasure(StringMeasure* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitStringEncode(StringEncode* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitStringConcat(StringConcat* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitStringEq(StringEq* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitStringWTF16Get(StringWTF16Get* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitStringSliceWTF(StringSliceWTF* curr) {
  shouldBeTrue(!getModule() || getModule()->features.hasStrings(),
               curr,
               "string operations require reference-types [--enable-strings]");
}

void FunctionValidator::visitContNew(ContNew* curr) {
  // TODO implement actual type-checking
  shouldBeTrue(!getModule() || getModule()->features.hasStackSwitching(),
               curr,
               "cont.new requires stack-switching [--enable-stack-switching]");

  shouldBeTrue(
    (curr->type.isContinuation() &&
     curr->type.getHeapType().getContinuation().type.isSignature()) ||
      curr->type == Type::unreachable,
    curr,
    "cont.new must be annotated with a continuation type");
}

void FunctionValidator::visitContBind(ContBind* curr) {
  // TODO implement actual type-checking
  shouldBeTrue(!getModule() || getModule()->features.hasStackSwitching(),
               curr,
               "cont.bind requires stack-switching [--enable-stack-switching]");

  shouldBeTrue(
    (curr->cont->type.isContinuation() &&
     curr->cont->type.getHeapType().getContinuation().type.isSignature()) ||
      curr->cont->type == Type::unreachable,
    curr,
    "the first type annotation on cont.bind must be a continuation type");

  shouldBeTrue(
    (curr->type.isContinuation() &&
     curr->type.getHeapType().getContinuation().type.isSignature()) ||
      curr->type == Type::unreachable,
    curr,
    "the second type annotation on cont.bind must be a continuation type");
}

void FunctionValidator::visitSuspend(Suspend* curr) {
  // TODO implement actual type-checking
  shouldBeTrue(!getModule() || getModule()->features.hasStackSwitching(),
               curr,
               "suspend requires stack-switching [--enable-stack-switching]");
}

void FunctionValidator::visitResume(Resume* curr) {
  // TODO implement actual type-checking
  shouldBeTrue(!getModule() || getModule()->features.hasStackSwitching(),
               curr,
               "resume requires stack-switching [--enable-stack-switching]");

  shouldBeTrue(
    curr->sentTypes.size() == curr->handlerBlocks.size(),
    curr,
    "sentTypes cache in resume instruction has not been initialized");

  shouldBeTrue(
    (curr->cont->type.isContinuation() &&
     curr->cont->type.getHeapType().getContinuation().type.isSignature()) ||
      curr->type == Type::unreachable,
    curr,
    "resume must be annotated with a continuation type");
}

void FunctionValidator::visitResumeThrow(ResumeThrow* curr) {
  // TODO implement actual type-checking
  shouldBeTrue(
    !getModule() || (getModule()->features.hasExceptionHandling() &&
                     getModule()->features.hasStackSwitching()),
    curr,
    "resume_throw requires exception handling [--enable-exception-handling] "
    "and stack-switching [--enable-stack-switching]");

  shouldBeTrue(
    curr->sentTypes.size() == curr->handlerBlocks.size(),
    curr,
    "sentTypes cache in resume_throw instruction has not been initialized");

  shouldBeTrue(
    (curr->cont->type.isContinuation() &&
     curr->cont->type.getHeapType().getContinuation().type.isSignature()) ||
      curr->type == Type::unreachable,
    curr,
    "resume_throw must be annotated with a continuation type");

  auto* tag = getModule()->getTagOrNull(curr->tag);
  if (!shouldBeTrue(!!tag, curr, "resume_throw must be annotated with a tag")) {
    return;
  }
}

void FunctionValidator::visitStackSwitch(StackSwitch* curr) {
  // TODO implement actual type-checking
  shouldBeTrue(!getModule() || getModule()->features.hasStackSwitching(),
               curr,
               "switch requires stack-switching [--enable-stack-switching]");

  shouldBeTrue(
    (curr->cont->type.isContinuation() &&
     curr->cont->type.getHeapType().getContinuation().type.isSignature()) ||
      curr->type == Type::unreachable,
    curr,
    "switch must be annotated with a continuation type");

  auto* tag = getModule()->getTagOrNull(curr->tag);
  if (!shouldBeTrue(!!tag, curr, "switch must be annotated with a tag")) {
    return;
  }
}

void FunctionValidator::visitFunction(Function* curr) {
  FeatureSet features;
  // Check for things like having a rec group with GC enabled. The type we're
  // checking is a reference type even if this an MVP function type, so ignore
  // the reference types feature here.
  features |= (curr->type.getFeatures() & ~FeatureSet::ReferenceTypes);
  for (const auto& param : curr->getParams()) {
    features |= param.getFeatures();
    shouldBeTrue(param.isConcrete(), curr, "params must be concretely typed");
  }
  for (const auto& result : curr->getResults()) {
    features |= result.getFeatures();
    shouldBeTrue(result.isConcrete(), curr, "results must be concretely typed");
  }
  for (const auto& var : curr->vars) {
    features |= var.getFeatures();
  }
  shouldBeTrue(features <= getModule()->features,
               curr->name,
               "all used types should be allowed");

  // validate optional local names
  std::unordered_set<Name> seen;
  for (auto& pair : curr->localNames) {
    Name name = pair.second;
    shouldBeTrue(seen.insert(name).second, name, "local names must be unique");
  }

  if (curr->body) {
    if (curr->getResults().isTuple()) {
      shouldBeTrue(getModule()->features.hasMultivalue(),
                   curr->body,
                   "Multivalue function results (multivalue is not enabled)");
    }
    if (curr->profile == IRProfile::Poppy) {
      shouldBeTrue(
        curr->body->is<Block>(), curr->body, "Function body must be a block");
    }
    // if function has no result, it is ignored
    // if body is unreachable, it might be e.g. a return
    shouldBeSubType(curr->body->type,
                    curr->getResults(),
                    curr->body,
                    "function body type must match, if function returns");

    if (getModule()->features.hasGC()) {
      // If we have non-nullable locals, verify that local.get are valid.
      LocalStructuralDominance info(curr, *getModule());
      for (auto index : info.nonDominatingIndices) {
        auto localType = curr->getLocalType(index);
        for (auto type : localType) {
          shouldBeTrue(!type.isNonNullable(),
                       index,
                       "non-nullable local's sets must dominate gets");
        }
      }
    }

    // Assert that we finished with a clean state after processing the body's
    // expressions, and reset the state for next time. Note that we use some of
    // this state in the above validations, so this must appear last.
    assert(breakTypes.empty());
    assert(delegateTargetNames.empty());
    assert(rethrowTargetNames.empty());
    labelNames.clear();
  }
}

void FunctionValidator::validateOffset(Address offset,
                                       Memory* mem,
                                       Expression* curr) {
  shouldBeTrue(mem->is64() || offset <= std::numeric_limits<uint32_t>::max(),
               curr,
               "offset must be u32");
}

void FunctionValidator::validateAlignment(
  size_t align, Type type, Index bytes, bool isAtomic, Expression* curr) {
  if (isAtomic) {
    shouldBeEqual(align,
                  (size_t)bytes,
                  curr,
                  "atomic accesses must have natural alignment");
    return;
  }
  switch (align) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
      break;
    default: {
      info.fail("bad alignment: " + std::to_string(align), curr, getFunction());
      break;
    }
  }
  shouldBeTrue(align <= bytes, curr, "alignment must not exceed natural");
  TODO_SINGLE_COMPOUND(type);
  switch (type.getBasic()) {
    case Type::i32:
    case Type::f32: {
      shouldBeTrue(align <= 4, curr, "alignment must not exceed natural");
      break;
    }
    case Type::i64:
    case Type::f64: {
      shouldBeTrue(align <= 8, curr, "alignment must not exceed natural");
      break;
    }
    case Type::v128:
    case Type::unreachable:
      break;
    case Type::none:
      WASM_UNREACHABLE("invalid type");
  }
}

static void validateBinaryenIR(Module& wasm, ValidationInfo& info) {
  struct BinaryenIRValidator
    : public PostWalker<BinaryenIRValidator,
                        UnifiedExpressionVisitor<BinaryenIRValidator>> {
    ValidationInfo& info;

    std::unordered_set<Expression*> seen;

    BinaryenIRValidator(ValidationInfo& info) : info(info) {}

    void visitExpression(Expression* curr) {
      auto scope = getFunction() ? getFunction()->name : Name("(global scope)");
      // check if a node type is 'stale', i.e., we forgot to finalize() the
      // node.
      auto oldType = curr->type;
      ReFinalizeNode().visit(curr);
      auto newType = curr->type;
      // It's ok for control flow structures to be further refinable, but all
      // other instructions must have the most-precise possible types.
      if (oldType != newType && !(Properties::isControlFlowStructure(curr) &&
                                  Type::isSubType(newType, oldType))) {
        std::ostringstream ss;
        ss << "stale type found in " << scope << " on " << curr
           << "\n(marked as " << oldType << ", should be " << newType << ")\n";
        info.fail(ss.str(), curr, getFunction());
      }
      curr->type = oldType;
      // check if a node is a duplicate - expressions must not be seen more than
      // once
      if (!seen.insert(curr).second) {
        std::ostringstream ss;
        ss << "expression seen more than once in the tree in " << scope
           << " on " << curr << '\n';
        info.fail(ss.str(), curr, getFunction());
      }
    }
  };
  BinaryenIRValidator binaryenIRValidator(info);
  binaryenIRValidator.walkModule(&wasm);
}

// Main validator class

static void validateImports(Module& module, ValidationInfo& info) {
  ModuleUtils::iterImportedFunctions(module, [&](Function* curr) {
    if (curr->getResults().isTuple()) {
      info.shouldBeTrue(module.features.hasMultivalue(),
                        curr->name,
                        "Imported multivalue function requires multivalue "
                        "[--enable-multivalue]");
    }
    if (info.validateWeb) {
      for (const auto& param : curr->getParams()) {
        info.shouldBeUnequal(param,
                             Type(Type::i64),
                             curr->name,
                             "Imported function must not have i64 parameters");
      }
      for (const auto& result : curr->getResults()) {
        info.shouldBeUnequal(result,
                             Type(Type::i64),
                             curr->name,
                             "Imported function must not have i64 results");
      }
    }

    if (Intrinsics(module).isCallWithoutEffects(curr)) {
      auto lastParam = curr->getParams();
      if (lastParam.isTuple()) {
        lastParam = lastParam.getTuple().back();
      }
      info.shouldBeTrue(lastParam.isFunction(),
                        curr->name,
                        "call.if.used's last param must be a function");
    }
  });
  ModuleUtils::iterImportedGlobals(module, [&](Global* curr) {
    if (!module.features.hasMutableGlobals()) {
      info.shouldBeFalse(curr->mutable_,
                         curr->name,
                         "Imported mutable global requires mutable-globals "
                         "[--enable-mutable-globals]");
    }
    info.shouldBeFalse(
      curr->type.isTuple(), curr->name, "Imported global cannot be tuple");
  });
}

static void validateExports(Module& module, ValidationInfo& info) {
  for (auto& curr : module.exports) {
    if (curr->kind == ExternalKind::Function) {
      if (info.validateWeb) {
        Function* f = module.getFunction(curr->value);
        for (const auto& param : f->getParams()) {
          info.shouldBeUnequal(
            param,
            Type(Type::i64),
            f->name,
            "Exported function must not have i64 parameters");
        }
        for (const auto& result : f->getResults()) {
          info.shouldBeUnequal(result,
                               Type(Type::i64),
                               f->name,
                               "Exported function must not have i64 results");
        }
      }
    } else if (curr->kind == ExternalKind::Global) {
      if (Global* g = module.getGlobalOrNull(curr->value)) {
        if (!module.features.hasMutableGlobals()) {
          info.shouldBeFalse(g->mutable_,
                             g->name,
                             "Exported mutable global requires mutable-globals "
                             "[--enable-mutable-globals]");
        }
        info.shouldBeFalse(
          g->type.isTuple(), g->name, "Exported global cannot be tuple");
      }
    }
  }
  std::unordered_set<Name> exportNames;
  for (auto& exp : module.exports) {
    Name name = exp->value;
    if (exp->kind == ExternalKind::Function) {
      info.shouldBeTrue(module.getFunctionOrNull(name),
                        name,
                        "module function exports must be found");
    } else if (exp->kind == ExternalKind::Global) {
      info.shouldBeTrue(module.getGlobalOrNull(name),
                        name,
                        "module global exports must be found");
    } else if (exp->kind == ExternalKind::Table) {
      info.shouldBeTrue(module.getTableOrNull(name),
                        name,
                        "module table exports must be found");
    } else if (exp->kind == ExternalKind::Memory) {
      info.shouldBeTrue(module.getMemoryOrNull(name),
                        name,
                        "module memory exports must be found");
    } else if (exp->kind == ExternalKind::Tag) {
      info.shouldBeTrue(
        module.getTagOrNull(name), name, "module tag exports must be found");
    } else {
      WASM_UNREACHABLE("invalid ExternalKind");
    }
    Name exportName = exp->name;
    info.shouldBeFalse(exportNames.count(exportName) > 0,
                       exportName,
                       "module exports must be unique");
    exportNames.insert(exportName);
  }
}

static void validateGlobals(Module& module, ValidationInfo& info) {
  std::unordered_set<Global*> seen;
  ModuleUtils::iterDefinedGlobals(module, [&](Global* curr) {
    info.shouldBeTrue(curr->type.getFeatures() <= module.features,
                      curr->name,
                      "all used types should be allowed");
    info.shouldBeTrue(
      curr->init != nullptr, curr->name, "global init must be non-null");
    assert(curr->init);
    info.shouldBeTrue(GlobalUtils::canInitializeGlobal(module, curr->init),
                      curr->name,
                      "global init must be constant");

    if (!info.shouldBeSubType(curr->init->type,
                              curr->type,
                              curr->init,
                              "global init must have correct type") &&
        !info.quiet) {
      info.getStream(nullptr) << "(on global " << curr->name << ")\n";
    }
    FunctionValidator(module, &info).validate(curr->init);
    // If GC is enabled (which means globals can refer to other non-imported
    // globals), check that globals only refer to preceeding globals.
    if (module.features.hasGC() && curr->init) {
      for (auto* get : FindAll<GlobalGet>(curr->init).list) {
        auto* global = module.getGlobalOrNull(get->name);
        info.shouldBeTrue(
          global && (seen.count(global) || global->imported()),
          curr->init,
          "global initializer should only refer to previous globals");
      }
      seen.insert(curr);
    }
  });

  // Check that globals have allowed types.
  for (auto& g : module.globals) {
    auto globalFeats = g->type.getFeatures();
    if (!info.shouldBeTrue(globalFeats <= module.features, g->name, "")) {
      info.getStream(nullptr)
        << "global type requires additional features "
        << getMissingFeaturesList(module, globalFeats) << '\n';
    }
  }
}

static void validateMemories(Module& module, ValidationInfo& info) {
  if (module.memories.size() > 1) {
    info.shouldBeTrue(
      module.features.hasMultiMemory(),
      "memory",
      "multiple memories require multimemory [--enable-multimemory]");
  }
  for (auto& memory : module.memories) {
    if (memory->hasMax()) {
      info.shouldBeFalse(
        memory->initial > memory->max, "memory", "memory max >= initial");
    }
    if (memory->is64()) {
      info.shouldBeTrue(module.features.hasMemory64(),
                        "memory",
                        "64-bit memories require memory64 [--enable-memory64]");
    } else {
      info.shouldBeTrue(memory->initial <= Memory::kMaxSize32,
                        "memory",
                        "initial memory must be <= 4GB");
      info.shouldBeTrue(!memory->hasMax() || memory->max <= Memory::kMaxSize32,
                        "memory",
                        "max memory must be <= 4GB, or unlimited");
    }
    info.shouldBeTrue(!memory->shared || memory->hasMax(),
                      "memory",
                      "shared memory must have max size");
    if (memory->shared) {
      info.shouldBeTrue(module.features.hasAtomics(),
                        "memory",
                        "shared memory requires threads [--enable-threads]");
    }
  }
}

static void validateDataSegments(Module& module, ValidationInfo& info) {
  for (auto& segment : module.dataSegments) {
    if (segment->isPassive) {
      info.shouldBeTrue(
        module.features.hasBulkMemory(),
        segment->offset,
        "nonzero segment flags require bulk memory [--enable-bulk-memory]");
      info.shouldBeEqual(segment->offset,
                         (Expression*)nullptr,
                         segment->offset,
                         "passive segment should not have an offset");
    } else {
      auto memory = module.getMemoryOrNull(segment->memory);
      if (!info.shouldBeTrue(memory != nullptr,
                             "segment",
                             "active segment must have a valid memory name")) {
        continue;
      }
      info.shouldBeEqual(segment->offset->type,
                         memory->addressType,
                         segment->offset,
                         "segment offset must match memory index type");
      info.shouldBeTrue(
        Properties::isValidConstantExpression(module, segment->offset),
        segment->offset,
        "memory segment offset must be constant");
      FunctionValidator(module, &info).validate(segment->offset);
    }
  }
}

static void validateTables(Module& module, ValidationInfo& info) {
  FunctionValidator validator(module, &info);

  if (!module.features.hasReferenceTypes()) {
    info.shouldBeTrue(module.tables.size() <= 1,
                      "table",
                      "Only 1 table definition allowed in MVP (requires "
                      "--enable-reference-types)");
    if (!module.tables.empty()) {
      auto& table = module.tables.front();
      info.shouldBeTrue(table->type == Type(HeapType::func, Nullable),
                        "table",
                        "Only funcref is valid for table type (when reference "
                        "types are disabled)");
      for (auto& segment : module.elementSegments) {
        info.shouldBeTrue(segment->table == table->name,
                          "elem",
                          "all element segments should refer to a single table "
                          "in MVP.");
        for (auto* expr : segment->data) {
          info.shouldBeTrue(
            expr->is<RefFunc>(),
            expr,
            "all table elements must be non-null funcrefs in MVP.");
          validator.validate(expr);
        }
      }
    }
  }

  auto funcref = Type(HeapType::func, Nullable);
  for (auto& table : module.tables) {
    info.shouldBeTrue(table->initial <= table->max,
                      "table",
                      "size minimum must not be greater than maximum");
    info.shouldBeTrue(
      table->type.isNullable(),
      "table",
      "Non-nullable reference types are not yet supported for tables");
    auto typeFeats = table->type.getFeatures();
    if (!info.shouldBeTrue(table->type == funcref ||
                             typeFeats <= module.features,
                           "table",
                           "table type requires additional features")) {
      info.getStream(nullptr)
        << getMissingFeaturesList(module, typeFeats) << '\n';
    }
    if (table->is64()) {
      info.shouldBeTrue(module.features.hasMemory64(),
                        "memory",
                        "64-bit tables require memory64 [--enable-memory64]");
    }
  }

  for (auto& segment : module.elementSegments) {
    info.shouldBeTrue(segment->type.isRef(),
                      "elem",
                      "element segment type must be of reference type.");
    info.shouldBeTrue(
      segment->type.isNullable(),
      "elem",
      "Non-nullable reference types are not yet supported for tables");
    auto typeFeats = segment->type.getFeatures();
    if (!info.shouldBeTrue(
          segment->type == funcref || typeFeats <= module.features,
          "elem",
          "element segment type requires additional features")) {
      info.getStream(nullptr)
        << getMissingFeaturesList(module, typeFeats) << '\n';
    }

    bool isPassive = !segment->table.is();
    if (isPassive) {
      info.shouldBeTrue(
        !segment->offset, "elem", "passive segment should not have an offset");
    } else {
      auto table = module.getTableOrNull(segment->table);
      info.shouldBeTrue(table != nullptr,
                        "elem",
                        "element segment must have a valid table name");
      info.shouldBeTrue(
        !!segment->offset, "elem", "table segment offset must have an offset");
      info.shouldBeEqual(segment->offset->type,
                         table->addressType,
                         segment->offset,
                         "element segment offset must match table index type");
      info.shouldBeTrue(
        Properties::isValidConstantExpression(module, segment->offset),
        segment->offset,
        "table segment offset must be constant");
      info.shouldBeTrue(
        Type::isSubType(segment->type, table->type),
        "elem",
        "element segment type must be a subtype of the table type");
      validator.validate(segment->offset);
    }
    for (auto* expr : segment->data) {
      info.shouldBeTrue(Properties::isValidConstantExpression(module, expr),
                        expr,
                        "element must be a constant expression");
      info.shouldBeSubType(expr->type,
                           segment->type,
                           expr,
                           "element must be a subtype of the segment type");
      validator.validate(expr);
    }
  }
}

static void validateTags(Module& module, ValidationInfo& info) {
  if (!module.tags.empty()) {
    info.shouldBeTrue(
      module.features.hasExceptionHandling(),
      module.tags[0]->name,
      "Tags require exception-handling [--enable-exception-handling]");
  }
  for (auto& curr : module.tags) {
    if (curr->sig.results != Type(Type::none)) {
      info.shouldBeTrue(module.features.hasStackSwitching(),
                        curr->name,
                        "Tags with result types require stack switching "
                        "feature [--enable-stack-switching]");
    }
    if (curr->sig.params.isTuple()) {
      info.shouldBeTrue(
        module.features.hasMultivalue(),
        curr->name,
        "Multivalue tag type requires multivalue [--enable-multivalue]");
    }
    FeatureSet features;
    for (const auto& param : curr->sig.params) {
      features |= param.getFeatures();
      info.shouldBeTrue(param.isConcrete(),
                        curr->name,
                        "Values in a tag should have concrete types");
    }
    info.shouldBeTrue(features <= module.features,
                      curr->name,
                      "all param types in tags should be allowed");
  }
}

static void validateStart(Module& module, ValidationInfo& info) {
  // start
  if (module.start.is()) {
    auto func = module.getFunctionOrNull(module.start);
    if (info.shouldBeTrue(
          func != nullptr, module.start, "start must be found")) {
      info.shouldBeTrue(func->getParams() == Type::none,
                        module.start,
                        "start must have 0 params");
      info.shouldBeTrue(func->getResults() == Type::none,
                        module.start,
                        "start must not return a value");
    }
  }
}

namespace {
template<typename T, typename U>
void validateModuleMap(Module& module,
                       ValidationInfo& info,
                       T& list,
                       U getter,
                       const std::string& kind) {
  // Given a list of module elements (like exports or globals), see that we can
  // get the items using the getter (getExportorNull, etc.). The getter uses the
  // lookup map internally, so this validates that they contain all items in
  // the list.
  for (auto& item : list) {
    auto* ptr = (module.*getter)(item->name);
    if (!ptr) {
      info.fail(kind + " must be found (use updateMaps)", item->name, nullptr);
    } else {
      info.shouldBeEqual(item->name,
                         ptr->name,
                         item->name,
                         "getter must return the correct item");
    }
  }

  // TODO: Also check there is nothing extraneous in the map, but that would
  //       require inspecting private fields of Module.
}
} // anonymous namespace

static void validateModuleMaps(Module& module, ValidationInfo& info) {
  // Module maps should be up to date.
  validateModuleMap(
    module, info, module.exports, &Module::getExportOrNull, "Export");
  validateModuleMap(
    module, info, module.functions, &Module::getFunctionOrNull, "Function");
  validateModuleMap(
    module, info, module.globals, &Module::getGlobalOrNull, "Global");
  validateModuleMap(module, info, module.tags, &Module::getTagOrNull, "Tag");
  validateModuleMap(module,
                    info,
                    module.elementSegments,
                    &Module::getElementSegmentOrNull,
                    "ElementSegment");
  validateModuleMap(
    module, info, module.memories, &Module::getMemoryOrNull, "Memory");
  validateModuleMap(module,
                    info,
                    module.dataSegments,
                    &Module::getDataSegmentOrNull,
                    "DataSegment");
  validateModuleMap(
    module, info, module.tables, &Module::getTableOrNull, "Table");
}

static void validateFeatures(Module& module, ValidationInfo& info) {
  if (module.features.hasGC()) {
    info.shouldBeTrue(module.features.hasReferenceTypes(),
                      module.features,
                      "--enable-gc requires --enable-reference-types");
  }
}

// TODO: If we want the validator to be part of libwasm rather than libpasses,
// then Using PassRunner::getPassDebug causes a circular dependence. We should
// fix that, perhaps by moving some of the pass infrastructure into libsupport.
bool WasmValidator::validate(Module& module, Flags flags) {
  ValidationInfo info(module);
  info.validateWeb = (flags & Web) != 0;
  info.validateGlobally = (flags & Globally) != 0;
  info.quiet = (flags & Quiet) != 0;

  // Parallel function validation.
  PassRunner runner(&module);
  FunctionValidator functionValidator(module, &info);
  functionValidator.validate(&runner);

  // Also validate imports, which were not covered in the parallel traversal
  // since it is a function-parallel operation.
  for (auto& func : module.functions) {
    if (func->imported()) {
      functionValidator.visitFunction(func.get());
    }
  }

  // Validate globally.
  if (info.validateGlobally) {
    validateImports(module, info);
    validateExports(module, info);
    validateGlobals(module, info);
    validateMemories(module, info);
    validateDataSegments(module, info);
    validateTables(module, info);
    validateTags(module, info);
    validateStart(module, info);
    validateModuleMaps(module, info);
    validateFeatures(module, info);
  }

  // Validate additional internal IR details when in pass-debug mode.
  if (PassRunner::getPassDebug()) {
    validateBinaryenIR(module, info);
  }

  // Print all the data.
  if (!info.valid.load() && !info.quiet) {
    for (auto& func : module.functions) {
      std::cerr << info.getStream(func.get()).str();
    }
    std::cerr << info.getStream(nullptr).str();
  }
  return info.valid.load();
}

bool WasmValidator::validate(Module& module, const PassOptions& options) {
  return validate(module, options.validateGlobally ? Globally : Minimal);
}

bool WasmValidator::validate(Function* func, Module& module, Flags flags) {
  ValidationInfo info(module);
  info.validateWeb = (flags & Web) != 0;
  info.validateGlobally = (flags & Globally) != 0;
  info.quiet = (flags & Quiet) != 0;
  FunctionValidator(module, &info).validate(func);
  // print all the data
  if (!info.valid.load() && !info.quiet) {
    std::cerr << info.getStream(func).str();
    std::cerr << info.getStream(nullptr).str();
  }
  return info.valid.load();
}

} // namespace wasm
