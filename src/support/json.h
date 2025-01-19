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

// An arena-free version of emscripten-optimizer/simple_ast.h's JSON
// class TODO: use this instead of that

#ifndef wasm_support_json_h
#define wasm_support_json_h

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "support/istring.h"
#include "support/safe_integer.h"
#include "support/utilities.h"

namespace json {

using IString = wasm::IString;

// Main value type
struct Value {
  struct Ref : public std::shared_ptr<Value> {
    Ref() = default;
    Ref(Value* value) : std::shared_ptr<Value>(value) {}

    Ref& operator[](size_t x) { return (*this->get())[x]; }
    Ref& operator[](IString x) { return (*this->get())[x]; }
  };

  template<typename T> static Ref make(T t) { return Ref(new Value(t)); }

  enum Type {
    String = 0,
    Number = 1,
    Array = 2,
    Null = 3,
    Bool = 4,
    Object = 5,
  };

  Type type = Null;

  using ArrayStorage = std::vector<Ref>;
  using ObjectStorage = std::unordered_map<IString, Ref>;

  // MSVC does not allow unrestricted unions:
  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2544.pdf
#ifdef _MSC_VER
  IString str;
#endif
  union { // TODO: optimize
#ifndef _MSC_VER
    IString str;
#endif
    double num = 0;
    ArrayStorage* arr; // manually allocated/freed
    bool boo;
    ObjectStorage* obj; // manually allocated/freed
    Ref ref;
  };

  // constructors all copy their input
  Value() {}
  explicit Value(const char* s) : type(Null) { setString(s); }
  explicit Value(double n) : type(Null) { setNumber(n); }
  explicit Value(ArrayStorage& a) : type(Null) {
    setArray();
    *arr = a;
  }
  // no bool constructor - would endanger the double one (int might convert the
  // wrong way)

  ~Value() { free(); }

  void free() {
    if (type == Array) {
      delete arr;
      arr = nullptr;
    } else if (type == Object) {
      delete obj;
      obj = nullptr;
    }
    type = Null;
    num = 0;
  }

  Value& setString(const char* s) {
    free();
    type = String;
    str = s;
    return *this;
  }
  Value& setString(const IString& s) {
    free();
    type = String;
    str = s;
    return *this;
  }
  Value& setNumber(double n) {
    free();
    type = Number;
    num = n;
    return *this;
  }
  Value& setArray(ArrayStorage& a) {
    free();
    type = Array;
    arr = new ArrayStorage;
    *arr = a;
    return *this;
  }
  Value& setArray(size_t size_hint = 0) {
    free();
    type = Array;
    arr = new ArrayStorage;
    arr->reserve(size_hint);
    return *this;
  }
  Value& setNull() {
    free();
    type = Null;
    return *this;
  }
  Value&
  setBool(bool b) { // Bool in the name, as otherwise might overload over int
    free();
    type = Bool;
    boo = b;
    return *this;
  }
  Value& setObject() {
    free();
    type = Object;
    obj = new ObjectStorage();
    return *this;
  }

  bool isString() { return type == String; }
  bool isNumber() { return type == Number; }
  bool isArray() { return type == Array; }
  bool isNull() { return type == Null; }
  bool isBool() { return type == Bool; }
  bool isObject() { return type == Object; }

  bool isBool(bool b) {
    return type == Bool && b == boo;
  } // avoid overloading == as it might overload over int

  const char* getCString() {
    assert(isString());
    return str.str.data();
  }
  IString& getIString() {
    assert(isString());
    return str;
  }
  double& getNumber() {
    assert(isNumber());
    return num;
  }
  ArrayStorage& getArray() {
    assert(isArray());
    return *arr;
  }
  bool& getBool() {
    assert(isBool());
    return boo;
  }

  int32_t getInteger() { // convenience function to get a known integer
    assert(wasm::isInteger(getNumber()));
    int32_t ret = getNumber();
    assert(double(ret) == getNumber()); // no loss in conversion
    return ret;
  }

  Value& operator=(const Value& other) {
    free();
    switch (other.type) {
      case String:
        setString(other.str);
        break;
      case Number:
        setNumber(other.num);
        break;
      case Array:
        setArray(*other.arr);
        break;
      case Null:
        setNull();
        break;
      case Bool:
        setBool(other.boo);
        break;
      default:
        abort(); // TODO
    }
    return *this;
  }

  bool operator==(const Value& other) {
    if (type != other.type) {
      return false;
    }
    switch (other.type) {
      case String:
        return str == other.str;
      case Number:
        return num == other.num;
      case Array:
        return this == &other; // if you want a deep compare, use deepCompare
      case Null:
        break;
      case Bool:
        return boo == other.boo;
      case Object:
        return this == &other; // if you want a deep compare, use deepCompare
      default:
        abort();
    }
    return true;
  }

  char* parse(char* curr) {
#define is_json_space(x)                                                       \
  (x == 32 || x == 9 || x == 10 ||                                             \
   x == 13) /* space, tab, linefeed/newline, or return */
#define skip()                                                                 \
  {                                                                            \
    while (*curr && is_json_space(*curr))                                      \
      curr++;                                                                  \
  }
#define skip_escaped_characters(ptr)                                           \
  while (*ptr && *ptr != '"') {                                                \
    if (*ptr == '\\' && *(ptr + 1)) {                                          \
      ptr++;                                                                   \
    }                                                                          \
    ptr++;                                                                     \
  }
    skip();
    if (*curr == '"') {
      // String
      curr++;
      char* close = curr;
      skip_escaped_characters(close);
      if (!(*close == '"')) {
        wasm::Fatal() << "Assertion failed (close == '\"') ";
      }
      *close = 0; // end this string, and reuse it straight from the input
      setString(curr);
      curr = close + 1;
    } else if (*curr == '[') {
      // Array
      curr++;
      skip();
      setArray();
      while (*curr != ']') {
        Ref temp = Ref(new Value());
        arr->push_back(temp);
        curr = temp->parse(curr);
        skip();
        if (*curr == ']') {
          break;
        }
        assert(*curr == ',');
        curr++;
        skip();
      }
      curr++;
    } else if (*curr == 'n') {
      // Null
      assert(strncmp(curr, "null", 4) == 0);
      setNull();
      curr += 4;
    } else if (*curr == 't') {
      // Bool true
      assert(strncmp(curr, "true", 4) == 0);
      setBool(true);
      curr += 4;
    } else if (*curr == 'f') {
      // Bool false
      assert(strncmp(curr, "false", 5) == 0);
      setBool(false);
      curr += 5;
    } else if (*curr == '{') {
      // Object
      curr++;
      skip();
      setObject();
      while (*curr != '}') {
        if (*curr == '"') {
          curr++;
          char* close = curr;
          skip_escaped_characters(close);
          if (!(*close == '"')) {
            wasm::Fatal() << "Assertion failed (close == '\"') ";
          }
          *close = 0; // end this string, and reuse it straight from the input
          IString key(curr);
          curr = close + 1;
          skip();
          assert(*curr == ':');
          curr++;
          skip();
          Ref value = Ref(new Value());
          curr = value->parse(curr);
          (*obj)[key] = value;
        } else {
          // Unquoted key
          char* start = curr;
          while (*curr && *curr != ':' && !is_json_space(*curr)) {
            curr++;
          }
          assert(*curr == ':');
          IString key(std::string(start, curr - start).c_str());
          curr++;
          skip();
          Ref value = Ref(new Value());
          curr = value->parse(curr);
          (*obj)[key] = value;
        }
        skip();
        if (*curr == '}') {
          break;
        }
        assert(*curr == ',');
        curr++;
        skip();
      }
      curr++;
    } else {
      // Number
      char* after;
      setNumber(strtod(curr, &after));
      curr = after;
    }
    return curr;
  }

  void stringify(std::ostream& os, bool pretty = false);

  // String operations

  // Number operations

  // Array operations

  size_t size() {
    assert(isArray());
    return arr->size();
  }

  void setSize(size_t size) {
    assert(isArray());
    auto old = arr->size();
    if (old != size) {
      arr->resize(size);
    }
    if (old < size) {
      for (auto i = old; i < size; i++) {
        (*arr)[i] = Ref(new Value());
      }
    }
  }

  Ref& operator[](unsigned x) {
    assert(isArray());
    return (*arr)[x];
  }

  Value& push_back(Ref r) {
    assert(isArray());
    arr->push_back(r);
    return *this;
  }
  Ref pop_back() {
    assert(isArray());
    Ref ret = arr->back();
    arr->pop_back();
    return ret;
  }

  Ref back() {
    assert(isArray());
    if (arr->size() == 0) {
      return nullptr;
    }
    return arr->back();
  }

  // Null operations

  // Bool operations

  // Object operations

  Ref& operator[](IString x) {
    assert(isObject());
    return (*obj)[x];
  }

  bool has(IString x) {
    assert(isObject());
    return obj->count(x) > 0;
  }
};

using Ref = Value::Ref;

} // namespace json

#endif // wasm_support_json_h
