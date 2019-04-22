/*
 * Copyright 2015 WebAssembly Community Group participants
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

#include "simple_ast.h"

namespace cashew {

// Ref methods

Ref& Ref::operator[](unsigned x) { return (*get())[x]; }

Ref& Ref::operator[](IString x) { return (*get())[x]; }

bool Ref::operator==(const char* str) {
  return get()->isString() && !strcmp(get()->str.str, str);
}

bool Ref::operator!=(const char* str) {
  return get()->isString() ? !!strcmp(get()->str.str, str) : true;
}

bool Ref::operator==(const IString& str) {
  return get()->isString() && get()->str == str;
}

bool Ref::operator!=(const IString& str) {
  return get()->isString() && get()->str != str;
}

bool Ref::operator==(Ref other) { return **this == *other; }

bool Ref::operator!() { return !get() || get()->isNull(); }

// Arena

GlobalMixedArena arena;

// Value

Value& Value::setAssign(Ref target, Ref value) {
  asAssign()->target() = target;
  asAssign()->value() = value;
  return *this;
}

Value& Value::setAssignName(IString target, Ref value) {
  asAssignName()->target() = target;
  asAssignName()->value() = value;
  return *this;
}

Assign* Value::asAssign() {
  assert(isAssign());
  return static_cast<Assign*>(this);
}

AssignName* Value::asAssignName() {
  assert(isAssignName());
  return static_cast<AssignName*>(this);
}

void Value::stringify(std::ostream& os, bool pretty) {
  static int indent = 0;
#define indentify()                                                            \
  {                                                                            \
    for (int i_ = 0; i_ < indent; i_++)                                        \
      os << "  ";                                                              \
  }
  switch (type) {
    case String: {
      if (str.str) {
        os << '"' << str.str << '"';
      } else {
        os << "\"(null)\"";
      }
      break;
    }
    case Number: {
      // doubles can have 17 digits of precision
      os << std::setprecision(17) << num;
      break;
    }
    case Array: {
      if (arr->size() == 0) {
        os << "[]";
        break;
      }
      os << '[';
      if (pretty) {
        os << std::endl;
        indent++;
      }
      for (size_t i = 0; i < arr->size(); i++) {
        if (i > 0) {
          if (pretty)
            os << "," << std::endl;
          else
            os << ", ";
        }
        indentify();
        (*arr)[i]->stringify(os, pretty);
      }
      if (pretty) {
        os << std::endl;
        indent--;
      }
      indentify();
      os << ']';
      break;
    }
    case Null: {
      os << "null";
      break;
    }
    case Bool: {
      os << (boo ? "true" : "false");
      break;
    }
    case Object: {
      os << '{';
      if (pretty) {
        os << std::endl;
        indent++;
      }
      bool first = true;
      for (auto i : *obj) {
        if (first) {
          first = false;
        } else {
          os << ", ";
          if (pretty)
            os << std::endl;
        }
        indentify();
        os << '"' << i.first.c_str() << "\": ";
        i.second->stringify(os, pretty);
      }
      if (pretty) {
        os << std::endl;
        indent--;
      }
      indentify();
      os << '}';
      break;
    }
    case Assign_: {
      os << "[";
      ref->stringify(os, pretty);
      os << ", ";
      asAssign()->value()->stringify(os, pretty);
      os << "]";
      break;
    }
    case AssignName_: {
      os << "[\"" << asAssignName()->target().str << "\"";
      os << ", ";
      asAssignName()->value()->stringify(os, pretty);
      os << "]";
      break;
    }
  }
}

// dump

void dump(const char* str, Ref node, bool pretty) {
  std::cerr << str << ": ";
  if (!!node)
    node->stringify(std::cerr, pretty);
  else
    std::cerr << "(nullptr)";
  std::cerr << std::endl;
}

// Traversals

struct TraverseInfo {
  TraverseInfo() = default;
  TraverseInfo(Ref node) : node(node) {
    assert(node.get());
    if (node->isArray()) {
      for (size_t i = 0; i < node->size(); i++) {
        maybeAdd(node[i]);
      }
    } else if (node->isAssign()) {
      auto assign = node->asAssign();
      maybeAdd(assign->target());
      maybeAdd(assign->value());
    } else if (node->isAssignName()) {
      auto assign = node->asAssignName();
      maybeAdd(assign->value());
    } else {
      // no children
    }
  }
  Ref node;
  size_t index = -1;
  std::vector<Ref> children;

private:
  void maybeAdd(Ref child) {
    if (child.get()) {
      children.push_back(child);
    }
  }
};

// Traverse, calling visit after the children
void traversePost(Ref node, std::function<void(Ref)> visit) {
  std::vector<TraverseInfo> stack;
  stack.push_back(TraverseInfo(node));
  while (!stack.empty()) {
    TraverseInfo& back = stack.back();
    if (back.index == size_t(-1)) {
      // This is the first time we see this. Push its children.
      back.index = 0;
      for (auto child : back.children) {
        stack.emplace_back(child);
      }
      continue;
    }
    if (back.index < back.children.size()) {
      // Visit this child.
      back.index++;
      visit(back.children[back.index - 1]);
      continue;
    }
    assert(back.index == back.children.size());
    // Time to visit the node itself
    auto node = back.node;
    stack.pop_back();
    visit(node);
  }
}

} // namespace cashew
