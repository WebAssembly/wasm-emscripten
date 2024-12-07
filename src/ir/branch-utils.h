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

#ifndef wasm_ir_branch_h
#define wasm_ir_branch_h

#include "ir/iteration.h"
#include "wasm-traversal.h"
#include "wasm.h"

namespace wasm::BranchUtils {

// Some branches are obviously not actually reachable (e.g. (br $out
// (unreachable)))

inline bool isBranchReachable(Expression* expr) {
  // If any child is unreachable, the branch is not taken. Note that expr itself
  // may be unreachable regardless (as in the case of a simple Break with no
  // condition, which is still taken).
  for (auto child : ChildIterator(expr)) {
    if (child->type == Type::unreachable) {
      return false;
    }
  }
  return true;
}

// Perform a generic operation on uses of scope names (branch + delegate
// targets) in an expression. The provided function receives a Name& which it
// can modify if it needs to.
template<typename T> void operateOnScopeNameUses(Expression* expr, T func) {
#define DELEGATE_ID expr->_id

#define DELEGATE_START(id) [[maybe_unused]] auto* cast = expr->cast<id>();

#define DELEGATE_GET_FIELD(id, field) cast->field

#define DELEGATE_FIELD_SCOPE_NAME_USE(id, field) func(cast->field);

#define DELEGATE_FIELD_CHILD(id, field)
#define DELEGATE_FIELD_INT(id, field)
#define DELEGATE_FIELD_LITERAL(id, field)
#define DELEGATE_FIELD_NAME(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_DEF(id, field)
#define DELEGATE_FIELD_TYPE(id, field)
#define DELEGATE_FIELD_HEAPTYPE(id, field)
#define DELEGATE_FIELD_ADDRESS(id, field)

#include "wasm-delegations-fields.def"
}

// Similar to operateOnScopeNameUses, but also passes in the type that is sent
// if the branch is taken. The type is none if there is no value.
template<typename T>
void operateOnScopeNameUsesAndSentTypes(Expression* expr, T func) {
  operateOnScopeNameUses(expr, [&](Name& name) {
    // There isn't a delegate mechanism for getting a sent value, so do a direct
    // if-else chain. This will need to be updated with new br variants.
    if (auto* br = expr->dynCast<Break>()) {
      func(name, br->value ? br->value->type : Type::none);
    } else if (auto* sw = expr->dynCast<Switch>()) {
      func(name, sw->value ? sw->value->type : Type::none);
    } else if (auto* br = expr->dynCast<BrOn>()) {
      func(name, br->getSentType());
    } else if (auto* tt = expr->dynCast<TryTable>()) {
      for (Index i = 0; i < tt->catchTags.size(); i++) {
        auto dest = tt->catchDests[i];
        if (dest == name) {
          func(name, tt->sentTypes[i]);
        }
      }
    } else if (auto* r = expr->dynCast<Resume>()) {
      for (Index i = 0; i < r->handlerTags.size(); i++) {
        auto dest = r->handlerTags[i];
        if (!dest.isNull() && dest == name) {
          func(name, r->sentTypes[i]);
        }
      }
    } else if (auto* r = expr->dynCast<ResumeThrow>()) {
      for (Index i = 0; i < r->handlerTags.size(); i++) {
        auto dest = r->handlerTags[i];
        if (!dest.isNull() && dest == name) {
          func(name, r->sentTypes[i]);
        }
      }
    } else {
      assert(expr->is<Try>() || expr->is<Rethrow>()); // delegate or rethrow
    }
  });
}

// Similar to operateOnScopeNameUses, but also passes in the expression that is
// sent if the branch is taken. nullptr is given if there is no value or there
// is a value but it is not known statically.
template<typename T>
void operateOnScopeNameUsesAndSentValues(Expression* expr, T func) {
  operateOnScopeNameUses(expr, [&](Name& name) {
    // There isn't a delegate mechanism for getting a sent value, so do a direct
    // if-else chain. This will need to be updated with new br variants.
    if (auto* br = expr->dynCast<Break>()) {
      func(name, br->value);
    } else if (auto* sw = expr->dynCast<Switch>()) {
      func(name, sw->value);
    } else if (auto* br = expr->dynCast<BrOn>()) {
      // A value may not be sent (e.g. BrOnNull does *not* send a null).
      func(name, br->getSentType() != Type::none ? br->ref : nullptr);
    } else if (expr->is<TryTable>()) {
      // The values are supplied by throwing instructions, so we are unable to
      // know what they will be here.
      func(name, nullptr);
    } else if (expr->is<Resume>()) {
      // The values are supplied by suspend instructions executed while running
      // the continuation, so we are unable to know what they will be here.
      func(name, nullptr);
    } else if (expr->is<ResumeThrow>()) {
      func(name, nullptr);
    } else {
      assert(expr->is<Try>() || expr->is<Rethrow>()); // delegate or rethrow
    }
  });
}

// Perform a generic operation on definitions of scope names in an expression.
// The provided function receives a Name& which it can modify if it needs to.
template<typename T> void operateOnScopeNameDefs(Expression* expr, T func) {
#define DELEGATE_ID expr->_id

#define DELEGATE_START(id) [[maybe_unused]] auto* cast = expr->cast<id>();

#define DELEGATE_GET_FIELD(id, field) cast->field

#define DELEGATE_FIELD_SCOPE_NAME_DEF(id, field) func(cast->field);

#define DELEGATE_FIELD_CHILD(id, field)
#define DELEGATE_FIELD_INT(id, field)
#define DELEGATE_FIELD_LITERAL(id, field)
#define DELEGATE_FIELD_NAME(id, field)
#define DELEGATE_FIELD_TYPE(id, field)
#define DELEGATE_FIELD_HEAPTYPE(id, field)
#define DELEGATE_FIELD_ADDRESS(id, field)
#define DELEGATE_FIELD_SCOPE_NAME_USE(id, field)

#include "wasm-delegations-fields.def"
}

using NameSet = std::set<Name>;

inline NameSet getUniqueTargets(Expression* expr) {
  NameSet ret;
  operateOnScopeNameUses(expr, [&](Name& name) { ret.insert(name); });
  return ret;
}

// If we branch to 'from', change that to 'to' instead.
inline bool replacePossibleTarget(Expression* branch, Name from, Name to) {
  bool worked = false;
  operateOnScopeNameUses(branch, [&](Name& name) {
    if (name == from) {
      name = to;
      worked = true;
    }
  });
  return worked;
}

// Replace all delegate/rethrow targets within the given AST.
inline void replaceExceptionTargets(Expression* ast, Name from, Name to) {
  struct Replacer
    : public PostWalker<Replacer, UnifiedExpressionVisitor<Replacer>> {
    Name from, to;
    Replacer(Name from, Name to) : from(from), to(to) {}
    void visitExpression(Expression* curr) {
      if (curr->is<Try>() || curr->is<Rethrow>()) {
        operateOnScopeNameUses(curr, [&](Name& name) {
          if (name == from) {
            name = to;
          }
        });
      }
    }
  };
  Replacer replacer(from, to);
  replacer.walk(ast);
}

// Replace all branch targets within the given AST.
inline void replaceBranchTargets(Expression* ast, Name from, Name to) {
  struct Replacer
    : public PostWalker<Replacer, UnifiedExpressionVisitor<Replacer>> {
    Name from, to;
    Replacer(Name from, Name to) : from(from), to(to) {}
    void visitExpression(Expression* curr) {
      if (Properties::isBranch(curr)) {
        operateOnScopeNameUses(curr, [&](Name& name) {
          if (name == from) {
            name = to;
          }
        });
      }
    }
  };
  Replacer replacer(from, to);
  replacer.walk(ast);
}

// Returns the set of targets to which we branch that are
// outside of an expression.
inline NameSet getExitingBranches(Expression* ast) {
  struct Scanner
    : public PostWalker<Scanner, UnifiedExpressionVisitor<Scanner>> {
    NameSet targets;

    void visitExpression(Expression* curr) {
      operateOnScopeNameDefs(curr, [&](Name& name) {
        if (name.is()) {
          targets.erase(name);
        }
      });
      operateOnScopeNameUses(curr, [&](Name& name) { targets.insert(name); });
    }
  };
  Scanner scanner;
  scanner.walk(ast);
  // anything not erased is a branch out
  return scanner.targets;
}

// returns the list of all branch targets in a node

inline NameSet getBranchTargets(Expression* ast) {
  struct Scanner
    : public PostWalker<Scanner, UnifiedExpressionVisitor<Scanner>> {
    NameSet targets;

    void visitExpression(Expression* curr) {
      operateOnScopeNameDefs(curr, [&](Name& name) {
        if (name.is()) {
          targets.insert(name);
        }
      });
    }
  };
  Scanner scanner;
  scanner.walk(ast);
  return scanner.targets;
}

// Check if an expression defines a particular name as a branch target anywhere
// inside it.
inline bool hasBranchTarget(Expression* ast, Name target) {
  if (!target.is()) {
    return false;
  }

  struct Scanner
    : public PostWalker<Scanner, UnifiedExpressionVisitor<Scanner>> {
    Name target;
    bool has = false;

    void visitExpression(Expression* curr) {
      operateOnScopeNameDefs(curr, [&](Name& name) {
        if (name == target) {
          has = true;
        }
      });
    }
  };
  Scanner scanner;
  scanner.target = target;
  scanner.walk(ast);
  return scanner.has;
}

// Get the name of the branch target that is defined in the expression, or an
// empty name if there is none.
inline Name getDefinedName(Expression* curr) {
  Name ret;
  operateOnScopeNameDefs(curr, [&](Name& name) { ret = name; });
  return ret;
}

// Return the value sent by a branch instruction, or nullptr if there is none.
inline Expression* getSentValue(Expression* curr) {
  Expression* ret = nullptr;
  operateOnScopeNameUsesAndSentValues(
    curr, [&](Name name, Expression* value) { ret = value; });
  return ret;
}

// Finds if there are branches targeting a name. Note that since names are
// unique in our IR, we just need to look for the name, and do not need
// to analyze scoping.
struct BranchSeeker
  : public PostWalker<BranchSeeker, UnifiedExpressionVisitor<BranchSeeker>> {
  Name target;

  Index found = 0;

  std::unordered_set<Type> types;

  BranchSeeker(Name target) : target(target) {}

  void noteFound(Type newType) {
    found++;
    types.insert(newType);
  }

  void visitExpression(Expression* curr) {
    operateOnScopeNameUsesAndSentTypes(curr, [&](Name& name, Type type) {
      if (name == target) {
        noteFound(type);
      }
    });
  }

  static bool has(Expression* tree, Name target) {
    if (!target.is()) {
      return false;
    }
    BranchSeeker seeker(target);
    seeker.walk(tree);
    return seeker.found > 0;
  }

  static Index count(Expression* tree, Name target) {
    if (!target.is()) {
      return 0;
    }
    BranchSeeker seeker(target);
    seeker.walk(tree);
    return seeker.found;
  }
};

// Accumulates all the branches in an entire tree.
struct BranchAccumulator
  : public PostWalker<BranchAccumulator,
                      UnifiedExpressionVisitor<BranchAccumulator>> {
  NameSet branches;

  void visitExpression(Expression* curr) {
    auto selfBranches = getUniqueTargets(curr);
    branches.insert(selfBranches.begin(), selfBranches.end());
  }

  static NameSet get(Expression* tree) {
    BranchAccumulator accumulator;
    accumulator.walk(tree);
    return accumulator.branches;
  }
};

// A helper structure for the common case of post-walking some IR while querying
// whether a branch is present. We can cache results for children in order to
// avoid quadratic time searches.
// We assume that a node will be scanned *once* here. That means that if we
// scan a node, we can discard all information for its children. This avoids
// linearly increasing memory usage over time.
class BranchSeekerCache {
  // Maps all the branches present in an expression and all its nested children.
  std::unordered_map<Expression*, NameSet> branches;

public:
  const NameSet& getBranches(Expression* curr) {
    auto iter = branches.find(curr);
    if (iter != branches.end()) {
      return iter->second;
    }
    NameSet currBranches;
    auto add = [&](NameSet& moreBranches) {
      // Make sure to do a fast swap for the first set of branches to arrive.
      // This helps the case of the first child being a block with a very large
      // set of names.
      if (currBranches.empty()) {
        currBranches.swap(moreBranches);
      } else {
        currBranches.insert(moreBranches.begin(), moreBranches.end());
      }
    };
    // Add from the children, which are hopefully cached.
    for (auto child : ChildIterator(curr)) {
      auto iter = branches.find(child);
      if (iter != branches.end()) {
        add(iter->second);
        // We are scanning the parent, which means we assume the child will
        // never be visited again.
        branches.erase(iter);
      } else {
        // The child was not cached. Scan it manually.
        BranchAccumulator childBranches;
        childBranches.walk(child);
        add(childBranches.branches);
        // Don't bother caching anything - we are scanning the parent, so the
        // child will presumably not be scanned again.
      }
    }
    // Finish with the parent's own branches.
    auto selfBranches = getUniqueTargets(curr);
    add(selfBranches);
    return branches[curr] = std::move(currBranches);
  }

  bool hasBranch(Expression* curr, Name target) {
    bool result = getBranches(curr).count(target);
#ifdef BRANCH_UTILS_DEBUG
    assert(bresult == BranchSeeker::has(curr, target));
#endif
    return result;
  }
};

// Stores information about branch targets, specifically, finding them by their
// name, and finding the branches to them.
struct BranchTargets {
  BranchTargets(Expression* expr) { inner.walk(expr); }

  // Gets the expression that defines this branch target, i.e., where we branch
  // to if we branch to that name.
  Expression* getTarget(Name name) const {
    auto iter = inner.targets.find(name);
    assert(iter != inner.targets.end());
    return iter->second;
  }

  // Gets the expressions branching to a target.
  std::unordered_set<Expression*> getBranches(Name name) const {
    auto iter = inner.branches.find(name);
    if (iter != inner.branches.end()) {
      return iter->second;
    }
    return {};
  }

private:
  struct Inner : public PostWalker<Inner, UnifiedExpressionVisitor<Inner>> {
    void visitExpression(Expression* curr) {
      operateOnScopeNameDefs(curr, [&](Name name) {
        if (name.is()) {
          targets[name] = curr;
        }
      });
      operateOnScopeNameUses(curr, [&](Name& name) {
        if (name.is()) {
          branches[name].insert(curr);
        }
      });
    }

    std::map<Name, Expression*> targets;
    std::map<Name, std::unordered_set<Expression*>> branches;
  } inner;
};

} // namespace wasm::BranchUtils

#endif // wasm_ir_branch_h
