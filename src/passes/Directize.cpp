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

//
// Turn indirect calls into direct calls. This is possible if we know
// the table cannot change, and if we see a constant argument for the
// indirect call's index.
//
// If called with
//
//   --directize-initial-tables-immutable
//
// then the initial tables' contents are assumed to be immutable. That is, if
// a table looks like [a, b, c] in the wasm, and we see a call to index 1, we
// will assume it must call b. It is possible that the table is appended to, but
// in this mode we assume the initial contents are not overwritten.
//

#include <unordered_map>

#include "call-utils.h"
#include "ir/table-utils.h"
#include "ir/type-updating.h"
#include "ir/utils.h"
#include "pass.h"
#include "wasm-builder.h"
#include "wasm-traversal.h"
#include "wasm.h"

namespace wasm {

namespace {

struct TableInfo {
  // Whether the table may be modifed at runtime, either because it is imported
  // or exported, or table.set operations exist for it in the code.
  bool mayBeModified = false;

  // Whether we can assume that the initial contents are immutable. See the
  // toplevel comment.
  bool initialContentsImmutable = false;

  std::unique_ptr<TableUtils::FlatTable> flatTable;

  bool canOptimize() const {
    // We can optimize if:
    //  * Either the table can't be modified at all, or it can be modified but
    //    the initial contents are immutable (so we can optimize them).
    //  * The table is flat.
    return (!mayBeModified || initialContentsImmutable) &&
            flatTable->valid;
  }
};

using TableInfoMap = std::unordered_map<Name, TableInfo>;

struct FunctionDirectizer : public WalkerPass<PostWalker<FunctionDirectizer>> {
  bool isFunctionParallel() override { return true; }

  Pass* create() override { return new FunctionDirectizer(tables); }

  FunctionDirectizer(const TableInfoMap& tables) : tables(tables) {}

  void visitCallIndirect(CallIndirect* curr) {
    auto& table = tables.at(curr->table);
    if (!table.canOptimize()) {
      return;
    }
    auto& flatTable = *table.flatTable;

    // If the target is constant, we can emit a direct call.
    if (curr->target->is<Const>()) {
      std::vector<Expression*> operands(curr->operands.begin(),
                                        curr->operands.end());
      replaceCurrent(makeDirectCall(operands, curr->target, flatTable, curr));
      return;
    }

    // Emit direct calls for things like a select over constants.
    if (auto* calls = CallUtils::convertToDirectCalls(
          curr,
          [&](Expression* target) {
            return getTargetInfo(target, flatTable, curr);
          },
          *getFunction(),
          *getModule())) {
      replaceCurrent(calls);
      // Note that types may have changed, as the utility here can add locals
      // which require fixups if they are non-nullable, for example.
      changedTypes = true;
      return;
    }
  }

  void doWalkFunction(Function* func) {
    WalkerPass<PostWalker<FunctionDirectizer>>::doWalkFunction(func);
    if (changedTypes) {
      ReFinalize().walkFunctionInModule(func, getModule());
      TypeUpdating::handleNonDefaultableLocals(func, *getModule());
    }
  }

private:
  const TableInfoMap& tables;

  bool changedTypes = false;

  // Given an expression that we will use as the target of an indirect call,
  // analyze it and return one of the results of CallUtils::IndirectCallInfo,
  // that is, whether we know a direct call target, or we know it will trap, or
  // if we know nothing.
  CallUtils::IndirectCallInfo
  getTargetInfo(Expression* target,
                const TableUtils::FlatTable& flatTable,
                CallIndirect* original) {
    auto* c = target->dynCast<Const>();
    if (!c) {
      return CallUtils::Unknown{};
    }

    Index index = c->value.geti32();

    // If the index is invalid, or the type is wrong, then this will trap.
    if (index >= flatTable.names.size()) {
      return CallUtils::Trap{};
    }
    auto name = flatTable.names[index];
    if (!name.is()) {
      return CallUtils::Trap{};
    }
    auto* func = getModule()->getFunction(name);
    if (original->heapType != func->type) {
      return CallUtils::Trap{};
    }
    return CallUtils::Known{name};
  }

  // Create a direct call for a given list of operands, an expression which is
  // known to contain a constant indicating the table offset, and the relevant
  // table. If we can see that the call will trap, instead return an
  // unreachable.
  Expression* makeDirectCall(const std::vector<Expression*>& operands,
                             Expression* c,
                             const TableUtils::FlatTable& flatTable,
                             CallIndirect* original) {
    // If the index is invalid, or the type is wrong, we can
    // emit an unreachable here, since in Binaryen it is ok to
    // reorder/replace traps when optimizing (but never to
    // remove them, at least not by default).
    auto info = getTargetInfo(c, flatTable, original);
    if (std::get_if<CallUtils::Trap>(&info)) {
      return replaceWithUnreachable(operands);
    }
    assert(std::get_if<CallUtils::Known>(&info));
    auto name = std::get_if<CallUtils::Known>(&info)->target;

    // Everything looks good!
    return Builder(*getModule())
      .makeCall(name, operands, original->type, original->isReturn);
  }

  Expression* replaceWithUnreachable(const std::vector<Expression*>& operands) {
    // Emitting an unreachable means we must update parent types.
    changedTypes = true;

    Builder builder(*getModule());
    std::vector<Expression*> newOperands;
    for (auto* operand : operands) {
      newOperands.push_back(builder.makeDrop(operand));
    }
    return builder.makeSequence(builder.makeBlock(newOperands),
                                builder.makeUnreachable());
  }
};

struct Directize : public Pass {
  void run(PassRunner* runner, Module* module) override {
    if (module->tables.empty()) {
      return;
    }

    // TODO: consider a per-table option here
    auto initialContentsImmutable =
      runner->options.getArgumentOrDefault(
        "directize-initial-contents-immutable", "") == "";

    // Set up the initial info.
    TableInfoMap tables;
    for (auto& table : module->tables) {
      tables[table->name].initialContentsImmutable = initialContentsImmutable;
      tables[table->name].flatTable =
        std::make_unique<TableUtils::FlatTable>(*module, *table);
    }

    // Next, look at the imports and exports.

    for (auto& table : module->tables) {
      if (table->imported()) {
        tables[table->name].mayBeModified = true;
      }
    }

    for (auto& ex : module->exports) {
      if (ex->kind == ExternalKind::Table) {
        tables[ex->value].mayBeModified = true;
      }
    }

    // This may already be enough information to know that we can't optimize
    // anything. If so, skip scanning all the module contents.
    auto canOptimize = [&]() {
      for (auto& [_, info] : tables) {
        if (info.canOptimize()) {
          return true;
        }
      }
      return false;
    };

    if (!canOptimize()) {
      return;
    }

    // Find which tables have sets.

    using TablesWithSet = std::unordered_set<Name>;

    ModuleUtils::ParallelFunctionAnalysis<TablesWithSet> analysis(
      *module, [&](Function* func, TablesWithSet& tablesWithSet) {
        if (func->imported()) {
          return;
        }
        for (auto* set : FindAll<TableSet>(func->body).list) {
          tablesWithSet.insert(set->table);
        }
      });

    for (auto& [_, names] : analysis.map) {
      for (auto name : names) {
        tables[name].mayBeModified = true;
      }
    }

    // Perhaps the new information about tables with sets shows we cannot
    // optimize.
    if (!canOptimize()) {
      return;
    }

    // We can optimize!
    FunctionDirectizer(tables).run(runner, module);
  }
};

} // anonymous namespace

Pass* createDirectizePass() { return new Directize(); }

} // namespace wasm
