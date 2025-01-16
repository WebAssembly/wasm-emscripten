#include "pass.h"
#include "wasm-builder.h"
#include "wasm.h"

namespace wasm {

struct AddThrowsPass : WalkerPass<PostWalker<AddThrowsPass>> {

  void visitBinary(Binary* curr) {
    switch (curr->op) {
      case DivSInt32:
      case DivUInt32:
      case RemSInt32:
      case RemUInt32:
      case DivSInt64:
      case DivUInt64:
      case RemSInt64:
      case RemUInt64: {
        Builder builder(*getModule());
        auto type = curr->right->type;
        auto eqz = type == Type::i32 ? EqZInt32 : EqZInt64;
        auto scratch = builder.addVar(getFunction(), type);
        curr->right = builder.makeIf(
          builder.makeUnary(eqz,
                            builder.makeLocalTee(scratch, curr->right, type)),
          builder.makeThrow("DivByZero", {}),
          builder.makeLocalGet(scratch, type),
          type);
        return;
      }
      default:
        return;
    }
  }

  template<typename T> std::optional<Index> handleRef(T* curr) {
    Builder builder(*getModule());
    auto type = curr->ref->type;
    if (type.isNonNullable()) {
      return std::nullopt;
    }
    auto scratch = builder.addVar(getFunction(), type);
    curr->ref = builder.makeIf(
      builder.makeRefIsNull(builder.makeLocalTee(scratch, curr->ref, type)),
      builder.makeThrow("NullDeref", {}),
      builder.makeLocalGet(scratch, type),
      type);
    return scratch;
  }

  template<typename T>
  void handleIndex(T* curr, std::optional<Index> refLocal) {
    Builder builder(*getModule());
    if (!refLocal) {
      refLocal = builder.addVar(getFunction(), curr->ref->type);
      curr->ref = builder.makeLocalTee(*refLocal, curr->ref, curr->ref->type);
    }
    auto scratch = builder.addVar(getFunction(), Type::i32);
    curr->index = builder.makeIf(
      builder.makeBinary(
        GeUInt32,
        builder.makeLocalTee(scratch, curr->index, Type::i32),
        builder.makeArrayLen(builder.makeLocalGet(*refLocal, curr->ref->type))),
      builder.makeThrow("IndexOOB", {}),
      builder.makeLocalGet(scratch, Type::i32));
  }

  void visitStructGet(StructGet* curr) { handleRef(curr); }

  void visitStructSet(StructSet* curr) { handleRef(curr); }

  void visitArrayGet(ArrayGet* curr) {
    auto ref = handleRef(curr);
    handleIndex(curr, ref);
  }

  void visitArraySet(ArraySet* curr) {
    auto ref = handleRef(curr);
    handleIndex(curr, ref);
  }

  void run(Module* wasm) override {
    Builder builder(*wasm);
    wasm->addTag(builder.makeTag("DivByZero", Signature{}));
    wasm->addTag(builder.makeTag("NullDeref", Signature{}));
    wasm->addTag(builder.makeTag("IndexOOB", Signature{}));
    WalkerPass<PostWalker<AddThrowsPass>>::run(wasm);
  }
};

Pass* createAddThrowsPass() { return new AddThrowsPass(); }

} // namespace wasm
