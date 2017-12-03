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

//
// Performs DCE on a graph containing a wasm module. The caller provides
// the graph, this tool fills in the wasm module's parts. It can then
// remove exports that are unused, for example, which is impossible
// otherwise (since we wouldn't know if the outside needs them).
//
// TODO: Currently all functions in the table are considered roots,
//       as the outside may call them. In the future we probably want
//       to refine that.

#include <memory>

#include "pass.h"
#include "support/command-line.h"
#include "support/file.h"
#include "support/colors.h"
#include "wasm-io.h"
#include "wasm-builder.h"
#include "ir/import-utils.h"

#include "emscripten-optimizer/simple_ast.h"

using namespace wasm;

// Generic reachability graph of abstract nodes

struct DCENode {
  Name name;
  std::vector<Name> reaches; // the other nodes this one can reach
  DCENode() {}
  DCENode(Name name) : name(name) {}
};

// A meta DCE graph with wasm integration
struct MetaDCEGraph {
  std::unordered_map<Name, DCENode> nodes;
  std::unordered_set<Name> roots;

  std::unordered_map<Name, Name> importToDCENode; // import internal name => DCE name
  std::unordered_map<Name, Name> exportToDCENode; // export exported name => DCE name
  std::unordered_map<Name, Name> functionToDCENode; // function name => DCE name
  std::unordered_map<Name, Name> globalToDCENode; // global name => DCE name

  std::unordered_map<Name, Name> DCENodeToImport; // reverse maps
  std::unordered_map<Name, Name> DCENodeToExport;
  std::unordered_map<Name, Name> DCENodeToFunction;
  std::unordered_map<Name, Name> DCENodeToGlobal;

  Module& wasm;

  MetaDCEGraph(Module& wasm) : wasm(wasm) {}

  // populate the graph with info from the wasm, integrating with potentially-existing
  // nodes for imports and exports that the graph may already contain.
  void scanWebAssembly() {
    // Add an entry for everything we might need ahead of time, so parallel work
    // does not alter parent state, just adds to things pointed by it, independently
    // (each thread will add for one function, etc.)
    for (auto& func : wasm.functions) {
      auto dceName = getName("func", func->name.str);
      DCENodeToFunction[dceName] = func->name;
      functionToDCENode[func->name] = dceName;
      nodes[dceName] = DCENode(dceName);
    }
    for (auto& global : wasm.globals) {
      auto dceName = getName("global", global->name.str);
      DCENodeToGlobal[dceName] = global->name;
      globalToDCENode[global->name] = dceName;
      nodes[dceName] = DCENode(dceName);
    }
    for (auto& imp : wasm.imports) {
      if (importToDCENode.find(imp->name) == importToDCENode.end()) {
        auto dceName = getName("import", imp->name.str);
        DCENodeToImport[dceName] = imp->name;
        importToDCENode[imp->name] = dceName;
        nodes[dceName] = DCENode(dceName);
      }
    }
    for (auto& exp : wasm.exports) {
      if (exportToDCENode.find(exp->name) == exportToDCENode.end()) {
        auto dceName = getName("export", exp->name.str);
        DCENodeToExport[dceName] = exp->name;
        exportToDCENode[exp->name] = dceName;
        nodes[dceName] = DCENode(dceName);
      }
      // we can also link the export to the thing being exported
      auto& node = nodes[exportToDCENode[exp->name]];
      if (exp->kind == ExternalKind::Function) {
        if (wasm.getFunctionOrNull(exp->value)) {
          node.reaches.push_back(functionToDCENode[exp->value]);
        } else {
          node.reaches.push_back(importToDCENode[exp->value]);
        }
      } else if (exp->kind == ExternalKind::Global) {
        if (wasm.getGlobalOrNull(exp->value)) {
          node.reaches.push_back(globalToDCENode[exp->value]);
        } else {
          node.reaches.push_back(importToDCENode[exp->value]);
        }
      }
    }

    // A parallel scanner for function bodies
    struct Scanner : public WalkerPass<PostWalker<Scanner>> {
      bool isFunctionParallel() override { return true; }

      Scanner(MetaDCEGraph* parent) : parent(parent) {}

      Scanner* create() override {
        return new Scanner(parent);
      }

      void visitCall(Call* curr) {
        parent->nodes[parent->functionToDCENode[getFunction()->name]].reaches.push_back(
          parent->functionToDCENode[curr->target]
        );
      }
      void visitCallImport(CallImport* curr) {
        assert(parent->functionToDCENode.count(getFunction()->name) > 0);
        parent->nodes[parent->functionToDCENode[getFunction()->name]].reaches.push_back(
          parent->importToDCENode[curr->target]
        );
      }
      void visitGetGlobal(GetGlobal* curr) {
        handleGlobal(curr->name);
      }
      void visitSetGlobal(SetGlobal* curr) {
        handleGlobal(curr->name);
      }

    private:
      MetaDCEGraph* parent;

      void handleGlobal(Name name) {
        if (getModule()->getGlobalOrNull(name)) return;
        // it's an import
        parent->nodes[parent->functionToDCENode[getFunction()->name]].reaches.push_back(
          parent->importToDCENode[name]
        );
      }
    };

    PassRunner runner(&wasm);
    runner.setIsNested(true);
    runner.add<Scanner>(this);
    runner.run();

    // also scan segment offsets
    Scanner scanner(this);
    scanner.setModule(&wasm);
    for (auto& segment : wasm.table.segments) {
      scanner.walk(segment.offset);
      // TODO: currently, all functions in the table are roots, but we
      //       should add an option to refine that
      for (auto& name : segment.data) {
        roots.insert(functionToDCENode[name]);
      }
    }
    for (auto& segment : wasm.memory.segments) {
      scanner.walk(segment.offset);
    }
  }

private:
  // gets a unique name for the graph
  Name getName(std::string prefix1, std::string prefix2) {
    while (1) {
      auto curr = Name(prefix1 + '$' + prefix2 + '$' + std::to_string(nameIndex++));
      if (nodes.find(curr) == nodes.end()) {
        return curr;
      }
    }
  }

  Index nameIndex = 0;

  std::unordered_set<Name> reached;

public:
  // Perform the DCE: simple marking from the roots
  void deadCodeElimination() {
    std::vector<Name> queue;
    for (auto root : roots) {
      reached.insert(root);
      queue.push_back(root);
    }
    while (queue.size() > 0) {
      auto name = queue.back();
      queue.pop_back();
      auto& node = nodes[name];
      for (auto target : node.reaches) {
        if (reached.find(target) == reached.end()) {
          reached.insert(target);
          queue.push_back(target);
        }
      }
    }
  }

  // Apply to the wasm
  void apply() {
    // Remove the unused exports
    std::vector<Name> toRemove;
    for (auto& exp : wasm.exports) {
      auto name = exp->name;
      auto dceName = exportToDCENode[name];
      if (reached.find(dceName) == reached.end()) {
        toRemove.push_back(name);
      }
    }
    for (auto name : toRemove) {
      wasm.removeExport(name);
    }
    // Now they are gone, standard optimization passes can do the rest!
    PassRunner passRunner(&wasm);
    passRunner.add("remove-unused-module-elements");
    passRunner.run();
  }

  // Print out everything we found is not used, and so can be
  // removed, including on the outside
  void printAllUnused() {
    std::set<std::string> unused;
    for (auto& pair : nodes) {
      auto name = pair.first;
      if (reached.find(name) == reached.end()) {
        unused.insert(name.str);
      }
    }
    for (auto& name : unused) {
      std::cout << "unused: " << name << '\n';
    }
  }

  // A debug utility, prints out the graph
  void dump() {
    std::cout << "=== graph ===\n";
    for (auto root : roots) {
      std::cout << "root: " << root.str << '\n';
    }
    for (auto& pair : nodes) {
      auto name = pair.first;
      auto& node = pair.second;
      std::cout << "node: " << name.str << '\n';
      if (DCENodeToImport.find(name) != DCENodeToImport.end()) {
        auto* imp = wasm.getImport(DCENodeToImport[name]);
        std::cout << "  is import " << DCENodeToImport[name] << ", " << imp->module.str << '.' << imp->base.str << '\n';
      }
      if (DCENodeToExport.find(name) != DCENodeToExport.end()) {
        std::cout << "  is export " << DCENodeToExport[name].str << ", " << wasm.getExport(DCENodeToExport[name])->value << '\n';
      }
      if (DCENodeToFunction.find(name) != DCENodeToFunction.end()) {
        std::cout << "  is function " << DCENodeToFunction[name] << '\n';
      }
      if (DCENodeToGlobal.find(name) != DCENodeToGlobal.end()) {
        std::cout << "  is function " << DCENodeToGlobal[name] << '\n';
      }
      for (auto target : node.reaches) {
        std::cout << "  reaches: " << target.str << '\n';
      }
    }
    std::cout << "=============\n";
  }
};

//
// main
//

int main(int argc, const char* argv[]) {
  Name entry;
  std::vector<std::string> passes;
  bool emitBinary = true;
  bool debugInfo = false;
  std::string graphFile;

  Options options("wasm-metadce", "This tool performs dead code elimination (DCE) on a larger space "
                                  "that the wasm module is just a part of. For example, if you have "
                                  "JS and wasm that are connected, this can DCE the combined graph. "
                                  "By doing so, it is able to eliminate wasm module exports, which "
                                  "otherwise regular optimizations cannot.\n\n"
                                  "This tool receives a representation of the reachability graph "
                                  "that the wasm module resides in, which contains abstract nodes "
                                  "and connections showing what they reach. Some of those nodes "
                                  "can represent the wasm module's imports and exports. The tool "
                                  "then completes the graph by adding the internal parts of the "
                                  "module, and does DCE on the entire thing.\n\n"
                                  "This tool will output a wasm module with dead code eliminated, "
                                  "and metadata describing the things in the rest of the graph "
                                  "that can be eliminated as well.\n\n"
                                  "The graph description file should represent the graph in the following "
                                  "JSON notation:\n\n"
                                  "  [\n"
                                  "    {\n"
                                  "      \"name\": \"entity1\",\n"
                                  "      \"reaches\": [\"entity2, \"entity3\"],\n"
                                  "      \"root\": true\n"
                                  "    },\n"
                                  "    {\n"
                                  "      \"name\": \"entity2\",\n"
                                  "      \"reaches\": [\"entity1, \"entity4\"]\n"
                                  "    },\n"
                                  "    {\n"
                                  "      \"name\": \"entity3\",\n"
                                  "      \"reaches\": [\"entity1\"],\n"
                                  "      \"export\": \"export1\"\n"
                                  "    },\n"
                                  "    {\n"
                                  "      \"name\": \"entity4\",\n"
                                  "      \"import\": [\"module\", \"import1\"]\n"
                                  "    },\n"
                                  "  ]\n\n"
                                  "Each entity has a name and an optional list of the other "
                                  "entities it reaches. It can also be marked as a root, "
                                  "export (with the export string), or import (with the "
                                  "module and import strings). DCE then computes what is "
                                  "reachable from the roots.");

  options
      .add("--output", "-o", "Output file (stdout if not specified)",
           Options::Arguments::One,
           [](Options* o, const std::string& argument) {
             o->extra["output"] = argument;
             Colors::disable();
           })
      .add("--emit-text", "-S", "Emit text instead of binary for the output file",
           Options::Arguments::Zero,
           [&](Options *o, const std::string &argument) { emitBinary = false; })
      .add("--debuginfo", "-g", "Emit names section and debug info",
           Options::Arguments::Zero,
           [&](Options *o, const std::string &arguments) { debugInfo = true; })
      .add("--graph-file", "-f", "Filename of the graph description file",
           Options::Arguments::One,
           [&](Options* o, const std::string& argument) {
             graphFile = argument;
           })
      .add_positional("INFILE", Options::Arguments::One,
                      [](Options* o, const std::string& argument) {
                        o->extra["infile"] = argument;
                      });
  options.parse(argc, argv);

  if (graphFile.size() == 0) {
    Fatal() << "no graph file provided.";
  }

  auto input(read_file<std::string>(options.extra["infile"], Flags::Text, Flags::Release));

  Module wasm;

  {
    if (options.debug) std::cerr << "reading...\n";
    ModuleReader reader;
    reader.setDebug(options.debug);

    try {
      reader.read(options.extra["infile"], wasm);
    } catch (ParseException& p) {
      p.dump(std::cerr);
      Fatal() << "error in parsing wasm input";
    }
  }

  auto graphInput(read_file<std::string>(graphFile, Flags::Text, Flags::Release));
  auto* copy = strdup(graphInput.c_str());
  cashew::Value json;
  json.parse(copy);

  // parse the JSON into our graph, doing all the JSON parsing here, leaving
  // the abstract computation for the class itself
  const cashew::IString NAME("name"),
                        REACHES("reaches"),
                        ROOT("root"),
                        EXPORT("export"),
                        IMPORT("import");

  MetaDCEGraph graph(wasm);

  if (!json.isArray()) {
    Fatal() << "input graph must be a JSON array of nodes. see --help for the form";
  }
  auto size = json.size();
  for (size_t i = 0; i < size; i++) {
    cashew::Ref ref = json[i];
    if (!ref->isObject()) {
      Fatal() << "nodes in input graph must be JSON objects. see --help for the form";
    }
    if (!ref->has(NAME)) {
      Fatal() << "nodes in input graph must have a name. see --help for the form";
    }
    DCENode node(ref[NAME]->getIString());
    if (ref->has(REACHES)) {
      cashew::Ref reaches = ref[REACHES];
      if (!reaches->isArray()) {
        Fatal() << "node.reaches must be an array. see --help for the form";
      }
      auto size = reaches->size();
      for (size_t j = 0; j < size; j++) {
        cashew::Ref name = reaches[j];
        if (!name->isString()) {
          Fatal() << "node.reaches items must be strings. see --help for the form";
        }
        node.reaches.push_back(name->getIString());
      }
    }
    if (ref->has(ROOT)) {
      cashew::Ref root = ref[ROOT];
      if (!root->isBool() || !root->getBool()) {
        Fatal() << "node.root, if it exists, must be true. see --help for the form";
      }
      graph.roots.insert(node.name);
    }
    if (ref->has(EXPORT)) {
      cashew::Ref exp = ref[EXPORT];
      if (!exp->isString()) {
        Fatal() << "node.export, if it exists, must be a string. see --help for the form";
      }
      graph.exportToDCENode[exp->getIString()] = node.name;
      graph.DCENodeToExport[node.name] = exp->getIString();
    }
    if (ref->has(IMPORT)) {
      cashew::Ref imp = ref[IMPORT];
      if (!imp->isArray() || imp->size() != 2 || !imp[0]->isString() || !imp[1]->isString()) {
        Fatal() << "node.import, if it exists, must be an array of two strings. see --help for the form";
      }
      auto importName = ImportUtils::getImport(wasm, imp[0]->getIString(), imp[1]->getIString())->name;
      graph.importToDCENode[importName] = node.name;
      graph.DCENodeToImport[node.name] = importName;
    }
    // TODO: optimize this copy with a clever move
    graph.nodes[node.name] = node;
  }

  // The external graph is now populated. Scan the module
  graph.scanWebAssembly();

  // Perform the DCE
  graph.deadCodeElimination();

  // Apply to the wasm
  graph.apply();

  if (options.extra.count("output") > 0) {
    ModuleWriter writer;
    writer.setBinary(emitBinary);
    writer.setDebugInfo(debugInfo);
    writer.write(wasm, options.extra["output"]);
  }

  // Print out everything that we found is removable, the outside might use that
  graph.printAllUnused();

  // Clean up
  free(copy);
}
