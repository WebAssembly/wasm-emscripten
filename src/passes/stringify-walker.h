#ifndef passes_stringify_walker_module_h
#define passes_stringify_walker_module_h

#include <queue>
#include "wasm-traversal.h"

namespace wasm {

template<typename SubType>
struct StringifyWalker
  : public PostWalker<SubType,
                      UnifiedExpressionVisitor<SubType>> {

  Module *wasm;
  std::queue<Expression**> queue;

  static void walkModule(SubType* self, Module* module);
  static void scan(SubType* self, Expression** currp);
  static void visitControlFlow(SubType* self, Expression** currp);
  void visitExpression(Expression* curr);
  // casts itself to subType and then calls visitExpression on that.

private:
  static void handler(SubType* self, Expression**);
  static void scanChildren(SubType* self, Expression** currp);
};

struct HashStringifyWalker
: public StringifyWalker<HashStringifyWalker> {

  void walkModule(Module *module);
  static void functionDidBegin(HashStringifyWalker* self);
  void visitExpression(Expression* curr);
  static void visitControlFlow(HashStringifyWalker* self, Expression** currp);

 private:
  std::vector<uint64_t> string;
  uint64_t monotonic = 0;
  // Change key to Expression
  // [[maybe_unused]] std::unordered_map<Expression *, uint64_t> exprToCounter;
  [[maybe_unused]] std::unordered_map<uint64_t, uint64_t> exprToCounter;

  void appendGloballyUniqueChar();
  void appendExpressionHash(Expression* curr, uint64_t hash);
};

struct TestStringifyWalker
: public StringifyWalker<TestStringifyWalker> {

  void walkModule(Module *module);
  static void functionDidBegin(TestStringifyWalker* self);
  void visitExpression(Expression* curr);
  static void visitControlFlow(TestStringifyWalker* self, Expression** currp);
  void print(std::ostream& os);
};

} // namespace wasm

#endif // passes_stringify_walker_module_h
