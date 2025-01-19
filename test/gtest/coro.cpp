#include <coroutine>
#include <utility>

#include "support/utilities.h"
#include "wasm-builder.h"
#include "wasm.h"
#include "gtest/gtest.h"

using namespace wasm;

template<typename T> struct Generator {
  struct promise_type;

  std::coroutine_handle<promise_type> handle;

  ~Generator() { handle.destroy(); }

  struct promise_type {
    std::optional<T> value;

    Generator<T> get_return_object() {
      return Generator<T>{
        std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    template<std::convertible_to<T> From>
    std::suspend_always yield_value(From&& from) {
      value = std::forward<From>(from);
      return {};
    }

    void return_void() {}

    void unhandled_exception() { WASM_UNREACHABLE("unhandled exception"); }
  };

  std::optional<T> next() {
    if (!handle.done()) {
      handle.resume();
    }
    auto ret = std::move(handle.promise().value);
    handle.promise().value.reset();
    return ret;
  }

  struct Iter {
    using value_type = T;
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;

    Generator<T>* generator = nullptr;
    std::optional<T> curr = std::nullopt;

    const T& operator*() const { return *curr; }

    const T* operator->() const { return &*curr; }

    Iter& operator++() {
      curr = generator->next();
      return *this;
    }

    Iter operator++(int) {
      auto it = *this;
      ++(*this);
      return it;
    }

    bool operator==(const Iter& other) const { return !curr && !other.curr; }
  };

  static_assert(std::input_iterator<Iter>);

  Iter begin() { return ++Iter{this}; }

  Iter end() { return Iter{this}; }
};

Generator<Expression**> walkExpressionPtrs(Expression*& curr) {
  struct Task {
    Expression** currp;
    bool done;
  };
  std::vector<Task> workStack = {{&curr, false}};
  while (!workStack.empty()) {
    auto task = workStack.back();
    workStack.pop_back();

    if (task.done) {
      co_yield task.currp;
      continue;
    }

    workStack.push_back({task.currp, true});

    Expression* curr = *task.currp;

#define DELEGATE_ID curr->_id

#define DELEGATE_START(id) [[maybe_unused]] auto* expr = curr->cast<id>();

#define DELEGATE_GET_FIELD(id, field) expr->field

#define DELEGATE_FIELD_CHILD(id, field)                                        \
  workStack.push_back({&expr->field, false})

#define DELEGATE_FIELD_OPTIONAL_CHILD(id, field)                               \
  if (expr->field) {                                                           \
    workStack.push_back({&expr->field, false});                                \
  }

#define DELEGATE_FIELD_INT(id, field)
#define DELEGATE_FIELD_INT_ARRAY(id, field)
#define DELEGATE_FIELD_LITERAL(id, field)
#define DELEGATE_FIELD_NAME(id, field)
#define DELEGATE_FIELD_NAME_VECTOR(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_DEF(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_USE(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_USE_VECTOR(id, field)
#define DELEGATE_FIELD_TYPE(id, field)
#define DELEGATE_FIELD_HEAPTYPE(id, field)
#define DELEGATE_FIELD_ADDRESS(id, field)

#include "wasm-delegations-fields.def"
  }
}

Generator<Expression*> walkExpression(Expression* curr) {
  for (auto& expr : walkExpressionPtrs(curr)) {
    co_yield *expr;
  }
}

TEST(Coro, Traversal) {
  Module wasm;
  Builder builder(wasm);

  Expression* expr = builder.makeBinary(
    MulInt32,
    builder.makeBinary(
      AddInt32, builder.makeConst(int32_t(0)), builder.makeConst(int32_t(1))),
    builder.makeBinary(
      SubInt32, builder.makeConst(int32_t(2)), builder.makeConst(int32_t(3))));

  Binary* mul = expr->cast<Binary>();
  Binary* add = mul->left->cast<Binary>();
  Binary* sub = mul->right->cast<Binary>();

  {
    auto walker = walkExpressionPtrs(expr);
    ASSERT_EQ(walker.next(), std::optional{&add->left});
    ASSERT_EQ(walker.next(), std::optional{&add->right});
    ASSERT_EQ(walker.next(), std::optional{&mul->left});
    ASSERT_EQ(walker.next(), std::optional{&sub->left});
    ASSERT_EQ(walker.next(), std::optional{&sub->right});
    ASSERT_EQ(walker.next(), std::optional{&mul->right});
    ASSERT_EQ(walker.next(), std::optional{&expr});
    ASSERT_EQ(walker.next(), std::nullopt);
  }

  {
    auto walker = walkExpression(expr);
    ASSERT_EQ(walker.next(), std::optional{add->left});
    ASSERT_EQ(walker.next(), std::optional{add->right});
    ASSERT_EQ(walker.next(), std::optional{add});
    ASSERT_EQ(walker.next(), std::optional{sub->left});
    ASSERT_EQ(walker.next(), std::optional{sub->right});
    ASSERT_EQ(walker.next(), std::optional{sub});
    ASSERT_EQ(walker.next(), std::optional{mul});
  }
}
