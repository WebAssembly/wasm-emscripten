/*
 * Copyright 2016 WebAssembly Community Group participants
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
// Prints the call graph in .dot format. You can use http://www.graphviz.org/ to
// view .dot files.
//

#include <iomanip>
#include <memory>
#include <sstream>

#include "ir/element-utils.h"
#include "ir/module-utils.h"
#include "ir/utils.h"
#include "pass.h"
#include "wasm.h"

namespace wasm {

struct PrintCallGraph : public Pass {
  bool modifiesBinaryenIR() override { return false; }

  void run(Module* module) override {
    Name caller = getPassOptions().getArgumentOrDefault(
      "callgraph-entry",
      "");

    struct GraphCreator : public PostWalker<GraphCreator> {
      Module* module;
      Function* currFunction;
      std::set<Name> visitedTargets; // Used to avoid printing duplicate edges.
      std::vector<Function*> allIndirectTargets;
      std::multimap<Name, Name> fullGraph; // First param is the caller/parent, second is the callee/child
      std::stringstream callGraphStream;

      GraphCreator(Module* module, Name caller) : module(module) {
        // Walk function bodies.
        ModuleUtils::iterDefinedFunctions(*module, [&](Function* curr) {
          currFunction = curr;
          visitedTargets.clear();
          walk(curr->body);
        });

        constructCallgraph(caller);
      }

      void constructCallgraph(Name caller) {
        if (caller.toString().empty()) {
          // construct callgraph for the entire module
          for (const auto& graph : fullGraph) {
            callGraphStream << "  \"" << graph.first << "\" -> \"" << graph.second
                      << "\"; // call\n";
          }
        } else {
          // construct callgraph for a given entry point
          std::set<Name> next{caller};
          visitedTargets.clear();
          
          while (!next.empty()) {
            std::set<Name> new_next;
            for (auto curFunc : next) {
              if (visitedTargets.count(curFunc)) {
                continue;
              }
              visitedTargets.insert(curFunc);

              auto count = fullGraph.count(curFunc);
              auto range = fullGraph.equal_range(curFunc);
              for (auto iter = range.first; iter != range.second; ++iter) {
                Name callee = iter->second;
                new_next.insert(callee);

                callGraphStream << "  \"" << curFunc.toString() << "\" -> \"" << callee.toString()
                    << "\"; // call\n";
              }
            }
            next = std::move(new_next);
          }
        }
      }

      void visitCall(Call* curr) {
        auto* target = module->getFunction(curr->target);
        if (!visitedTargets.emplace(target->name).second) {
          return;
        }
        fullGraph.emplace(currFunction->name, target->name);
      }
    };
    GraphCreator graphCreator(module, caller);

    std::ostream& o = std::cout;
     o << "digraph call {\n"
          "  rankdir = LR;\n"
          "  subgraph cluster_key {\n"
          "    node [shape=box, fontname=courier, fontsize=10];\n"
          "    edge [fontname=courier, fontsize=10];\n"
          "    label = \"Key\";\n"
          "    \"Import\" [style=\"filled\", fillcolor=\"turquoise\"];\n"
          "    \"Export\" [style=\"filled\", fillcolor=\"gray\"];\n"
          "    \"Indirect Target\" [style=\"filled, rounded\", "
          "fillcolor=\"white\"];\n"
          "    \"A\" -> \"B\" [style=\"filled, rounded\", label = \"Direct "
          "Call\"];\n"
          "  }\n\n"
          "  node [shape=box, fontname=courier, fontsize=10];\n";

    // Defined functions
     ModuleUtils::iterDefinedFunctions(*module, [&](Function* curr) {
      if (!caller.toString().empty() &&
          graphCreator.visitedTargets.count(curr->name) == 0) {
         return;
       }
       std::cout << "  \"" << curr->name
                 << "\" [style=\"filled\", fillcolor=\"white\"];\n";
     });

    // Imported functions
     ModuleUtils::iterImportedFunctions(*module, [&](Function* curr) {
       if (!caller.toString().empty() &&
           graphCreator.visitedTargets.count(curr->name) == 0) {
         return;
       }
         o << "  \"" << curr->name
           << "\" [style=\"filled\", fillcolor=\"turquoise\"];\n";
     });

    // Exports
     for (auto& curr : module->exports) {
      if (curr->kind == ExternalKind::Function) {
        Function* func = module->getFunction(curr->value);
        if (!caller.toString().empty() &&
            graphCreator.visitedTargets.count(func->name) == 0) {
          continue;
        }
        o << "  \"" << func->name
          << "\" [style=\"filled\", fillcolor=\"gray\"];\n";
      }
     }

    // Indirect Targets
     ElementUtils::iterAllElementFunctionNames(module, [&](Name& name) {
       auto* func = module->getFunction(name);
       if (!caller.toString().empty() &&
           graphCreator.visitedTargets.count(func->name) == 0) {
         return;
       }
       o << "  \"" << func->name << "\" [style=\"filled, rounded\"];\n";
     });

    o << graphCreator.callGraphStream.rdbuf();

    o << "}\n";
  }
};

Pass* createPrintCallGraphPass() { return new PrintCallGraph(); }

} // namespace wasm
