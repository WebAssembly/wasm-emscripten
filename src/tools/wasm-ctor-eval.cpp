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
// Loads wasm plus a list of functions that are global ctors, i.e.,
// are to be executed. It then executes as many of them as it can,
// applying their changes to memory as needed, then writes it. In
// other words, this executes code at compile time to speed up
// startup later.
//

#include <memory>

#include "asmjs/shared-constants.h"
#include "ir/global-utils.h"
#include "ir/import-utils.h"
#include "ir/literal-utils.h"
#include "ir/memory-utils.h"
#include "ir/module-utils.h"
#include "pass.h"
#include "support/colors.h"
#include "support/file.h"
#include "tool-options.h"
#include "wasm-builder.h"
#include "wasm-interpreter.h"
#include "wasm-io.h"
#include "wasm-validator.h"

using namespace wasm;

struct FailToEvalException {
  std::string why;
  FailToEvalException(std::string why) : why(why) {}
};

// We do not have access to imported globals
class EvallingGlobalManager {
  // values of globals
  std::map<Name, Literals> globals;

  // globals that are dangerous to modify in the module
  std::set<Name> dangerousGlobals;

  // whether we are done adding new globals
  bool sealed = false;

public:
  void addDangerous(Name name) { dangerousGlobals.insert(name); }

  void seal() { sealed = true; }

  // for equality purposes, we just care about the globals
  // and whether they have changed
  bool operator==(const EvallingGlobalManager& other) {
    return globals == other.globals;
  }
  bool operator!=(const EvallingGlobalManager& other) {
    return !(*this == other);
  }

  Literals& operator[](Name name) {
    if (dangerousGlobals.count(name) > 0) {
      std::string extra;
      if (name == "___dso_handle") {
        extra = "\nrecommendation: build with -s NO_EXIT_RUNTIME=1 so that "
                "calls to atexit that use ___dso_handle are not emitted";
      }
      throw FailToEvalException(
        std::string(
          "tried to access a dangerous (import-initialized) global: ") +
        name.str + extra);
    }
    return globals[name];
  }

  struct Iterator {
    Name first;
    Literals second;
    bool found;

    Iterator() : found(false) {}
    Iterator(Name name, Literals value)
      : first(name), second(value), found(true) {}

    bool operator==(const Iterator& other) {
      return first == other.first && second == other.second &&
             found == other.found;
    }
    bool operator!=(const Iterator& other) { return !(*this == other); }
  };

  Iterator find(Name name) {
    if (globals.find(name) == globals.end()) {
      return end();
    }
    return Iterator(name, globals[name]);
  }

  Iterator end() { return Iterator(); }
};

// Use a ridiculously large stack size.
static Index STACK_SIZE = 32 * 1024 * 1024;

// Start the stack at a ridiculously large location, and do so in
// a way that works regardless if the stack goes up or down.
static Index STACK_START = 1024 * 1024 * 1024 + STACK_SIZE;

// Bound the stack location in both directions, so we have bounds
// that do not depend on the direction it grows.
static Index STACK_LOWER_LIMIT = STACK_START - STACK_SIZE;
static Index STACK_UPPER_LIMIT = STACK_START + STACK_SIZE;

class EvallingModuleInstance
  : public ModuleInstanceBase<EvallingGlobalManager, EvallingModuleInstance> {
public:
  EvallingModuleInstance(Module& wasm,
                         ExternalInterface* externalInterface,
                         std::map<Name, std::shared_ptr<EvallingModuleInstance>>
                           linkedInstances_ = {})
    : ModuleInstanceBase(wasm, externalInterface, linkedInstances_) {
    // if any global in the module has a non-const constructor, it is using a
    // global import, which we don't have, and is illegal to use
    ModuleUtils::iterDefinedGlobals(wasm, [&](Global* global) {
      if (!global->init->is<Const>()) {
        // some constants are ok to use
        if (auto* get = global->init->dynCast<GlobalGet>()) {
          auto name = get->name;
          auto* import = wasm.getGlobal(name);
          if (import->module == Name(ENV) &&
              (import->base ==
                 STACKTOP || // stack constants are special, we handle them
               import->base == STACK_MAX)) {
            return; // this is fine
          }
        }
        // this global is dangerously initialized by an import, so if it is
        // used, we must fail
        globals.addDangerous(global->name);
      }
    });
  }

  std::vector<char> stack;

  // create C stack space for us to use. We do *NOT* care about their contents,
  // assuming the stack top was unwound. the memory may have been modified,
  // but it should not be read afterwards, doing so would be undefined behavior
  void setupEnvironment() {
    // prepare scratch memory
    stack.resize(2 * STACK_SIZE);
    // tell the module to accept writes up to the stack end
    auto total = STACK_START + STACK_SIZE;
    memorySize = total / Memory::kPageSize;
  }
};

// Build an artificial `env` module based on a module's imports, so that the
// interpreter can use correct object instances. It initializes usable global
// imports, and fills the rest with fake values since those are dangerous to
// use. we will fail if dangerous globals are used.
std::unique_ptr<Module> buildEnvModule(Module& wasm) {
  auto env = std::make_unique<Module>();
  env->name = "env";

  // create empty functions with similar signature
  ModuleUtils::iterImportedFunctions(wasm, [&](Function* func) {
    if (func->module == "env") {
      Builder builder(*env);
      auto* copied = ModuleUtils::copyFunction(func, *env);
      copied->module = Name();
      copied->base = Name();
      copied->body = builder.makeUnreachable();
      env->addExport(
        builder.makeExport(func->base, copied->name, ExternalKind::Function));
    }
  });

  // create tables with similar initial and max values
  ModuleUtils::iterImportedTables(wasm, [&](Table* table) {
    if (table->module == "env") {
      auto* copied = ModuleUtils::copyTable(table, *env);
      copied->module = Name();
      copied->base = Name();
      env->addExport(Builder(*env).makeExport(
        table->base, copied->name, ExternalKind::Table));
    }
  });

  ModuleUtils::iterImportedGlobals(wasm, [&](Global* global) {
    if (global->module == "env") {
      auto* copied = ModuleUtils::copyGlobal(global, *env);
      copied->module = Name();
      copied->base = Name();

      Builder builder(*env);
      if (global->base == STACKTOP || global->base == STACK_MAX) {
        copied->init = builder.makeConst(STACK_START);
      } else {
        copied->init = builder.makeConst(Literal::makeZero(global->type));
      }
      env->addExport(
        builder.makeExport(global->base, copied->name, ExternalKind::Global));
    }
  });

  // create an exported memory with the same initial and max size
  ModuleUtils::iterImportedMemories(wasm, [&](Memory* memory) {
    if (memory->module == "env") {
      env->memory.name = wasm.memory.name;
      env->memory.exists = true;
      env->memory.initial = memory->initial;
      env->memory.max = memory->max;
      env->memory.shared = memory->shared;
      env->memory.indexType = memory->indexType;
      env->addExport(Builder(*env).makeExport(
        wasm.memory.base, wasm.memory.name, ExternalKind::Memory));
    }
  });

  return env;
}

struct CtorEvalExternalInterface : EvallingModuleInstance::ExternalInterface {
  Module* wasm;
  EvallingModuleInstance* instance;
  std::map<Name, std::shared_ptr<EvallingModuleInstance>> linkedInstances;

  CtorEvalExternalInterface(
    std::map<Name, std::shared_ptr<EvallingModuleInstance>> linkedInstances_ =
      {}) {
    linkedInstances.swap(linkedInstances_);
  }

  void init(Module& wasm_, EvallingModuleInstance& instance_) override {
    wasm = &wasm_;
    instance = &instance_;
  }

  void importGlobals(EvallingGlobalManager& globals, Module& wasm_) override {
    ModuleUtils::iterImportedGlobals(wasm_, [&](Global* global) {
      auto it = linkedInstances.find(global->module);
      if (it != linkedInstances.end()) {
        auto* inst = it->second.get();
        auto* globalExport = inst->wasm.getExportOrNull(global->base);
        if (!globalExport) {
          throw FailToEvalException(std::string("importGlobals: ") +
                                    global->module.str + "." +
                                    global->base.str);
        }
        globals[global->name] = inst->globals[globalExport->value];
      } else {
        throw FailToEvalException(std::string("importGlobals: ") +
                                  global->module.str + "." + global->base.str);
      }
    });
  }

  Literals callImport(Function* import, LiteralList& arguments) override {
    std::string extra;
    if (import->module == ENV && import->base == "___cxa_atexit") {
      extra = "\nrecommendation: build with -s NO_EXIT_RUNTIME=1 so that calls "
              "to atexit are not emitted";
    }
    throw FailToEvalException(std::string("call import: ") +
                              import->module.str + "." + import->base.str +
                              extra);
  }

  Literals callTable(Name tableName,
                     Index index,
                     Signature sig,
                     LiteralList& arguments,
                     Type result,
                     EvallingModuleInstance& instance) override {

    std::unordered_map<wasm::Name, std::vector<wasm::Name>>::iterator it;

    auto* table = wasm->getTableOrNull(tableName);
    if (!table) {
      throw FailToEvalException("callTable on non-existing table");
    }

    // we assume the table is not modified (hmm)
    // look through the segments, try to find the function
    for (auto& segment : wasm->elementSegments) {
      if (segment->table != tableName) {
        continue;
      }

      Index start;
      // look for the index in this segment. if it has a constant offset, we
      // look in the proper range. if it instead gets a global, we rely on the
      // fact that when not dynamically linking then the table is loaded at
      // offset 0.
      if (auto* c = segment->offset->dynCast<Const>()) {
        start = c->value.getInteger();
      } else if (segment->offset->is<GlobalGet>()) {
        start = 0;
      } else {
        // wasm spec only allows const and global.get there
        WASM_UNREACHABLE("invalid expr type");
      }
      auto end = start + segment->data.size();
      if (start <= index && index < end) {
        auto entry = segment->data[index - start];
        if (auto* get = entry->dynCast<RefFunc>()) {
          auto name = get->func;
          // if this is one of our functions, we can call it; if it was
          // imported, fail
          auto* func = wasm->getFunction(name);
          if (func->sig != sig) {
            throw FailToEvalException(
              std::string("callTable signature mismatch: ") + name.str);
          }
          if (!func->imported()) {
            return instance.callFunctionInternal(name, arguments);
          } else {
            throw FailToEvalException(
              std::string("callTable on imported function: ") + name.str);
          }
        } else {
          throw FailToEvalException(
            std::string("callTable on uninitialized entry"));
        }
      }
    }
    throw FailToEvalException(
      std::string("callTable on index not found in static segments: ") +
      std::to_string(index));
  }

  int8_t load8s(Address addr) override { return doLoad<int8_t>(addr); }
  uint8_t load8u(Address addr) override { return doLoad<uint8_t>(addr); }
  int16_t load16s(Address addr) override { return doLoad<int16_t>(addr); }
  uint16_t load16u(Address addr) override { return doLoad<uint16_t>(addr); }
  int32_t load32s(Address addr) override { return doLoad<int32_t>(addr); }
  uint32_t load32u(Address addr) override { return doLoad<uint32_t>(addr); }
  int64_t load64s(Address addr) override { return doLoad<int64_t>(addr); }
  uint64_t load64u(Address addr) override { return doLoad<uint64_t>(addr); }

  void store8(Address addr, int8_t value) override {
    doStore<int8_t>(addr, value);
  }
  void store16(Address addr, int16_t value) override {
    doStore<int16_t>(addr, value);
  }
  void store32(Address addr, int32_t value) override {
    doStore<int32_t>(addr, value);
  }
  void store64(Address addr, int64_t value) override {
    doStore<int64_t>(addr, value);
  }

  // called during initialization, but we don't keep track of a table
  void tableStore(Name tableName, Index addr, const Literal& value) override {}
  const Literal& tableLoad(Name tableName, Index addr) override {
    throw FailToEvalException("table load");
  }
  Index growTable(Name tableName, Index delta, Literal initialValue) override {
    throw FailToEvalException("grow table");
  }
  virtual Index tableSize(Name tableName) override {
    auto* table = wasm->getTableOrNull(tableName);
    if (!table) {
      throw FailToEvalException("tableSize on non-existing table");
    }
    return table->initial;
  }
  bool growMemory(Address /*oldSize*/, Address newSize) override {
    throw FailToEvalException("grow memory");
  }

  void trap(const char* why) override {
    throw FailToEvalException(std::string("trap: ") + why);
  }

  void hostLimit(const char* why) override {
    throw FailToEvalException(std::string("trap: ") + why);
  }

  void throwException(const WasmException& exn) override {
    std::stringstream ss;
    ss << "exception thrown: " << exn;
    throw FailToEvalException(ss.str());
  }

private:
  // TODO: handle unaligned too, see shell-interface

  template<typename T> T* getMemory(Address address) {
    // if memory is on the stack, use the stack
    if (address >= STACK_LOWER_LIMIT) {
      if (address >= STACK_UPPER_LIMIT) {
        throw FailToEvalException("stack usage too high");
      }
      Address relative = address - STACK_LOWER_LIMIT;
      // in range, all is good, use the stack
      return (T*)(&instance->stack[relative]);
    }

    // otherwise, this must be in the singleton segment. resize as needed
    if (wasm->memory.segments.size() == 0) {
      std::vector<char> temp;
      Builder builder(*wasm);
      wasm->memory.segments.push_back(
        Memory::Segment(builder.makeConst(int32_t(0)), temp));
    }
    // memory should already have been flattened
    assert(wasm->memory.segments[0].offset->cast<Const>()->value.getInteger() ==
           0);
    auto max = address + sizeof(T);
    auto& data = wasm->memory.segments[0].data;
    if (max > data.size()) {
      data.resize(max);
    }
    return (T*)(&data[address]);
  }

  template<typename T> void doStore(Address address, T value) {
    // do a memcpy to avoid undefined behavior if unaligned
    memcpy(getMemory<T>(address), &value, sizeof(T));
  }

  template<typename T> T doLoad(Address address) {
    // do a memcpy to avoid undefined behavior if unaligned
    T ret;
    memcpy(&ret, getMemory<T>(address), sizeof(T));
    return ret;
  }
};

void evalCtors(Module& wasm, std::vector<std::string> ctors) {
  // build and link the env module
  auto envModule = buildEnvModule(wasm);
  CtorEvalExternalInterface envInterface;
  auto envInstance =
    std::make_shared<EvallingModuleInstance>(*envModule, &envInterface);
  envInstance->setupEnvironment();

  std::map<Name, std::shared_ptr<EvallingModuleInstance>> linkedInstances;
  linkedInstances["env"] = envInstance;

  CtorEvalExternalInterface interface(linkedInstances);
  try {
    // flatten memory, so we do not depend on the layout of data segments
    if (!MemoryUtils::flatten(wasm.memory)) {
      Fatal() << "  ...stopping since could not flatten memory\n";
    }

    // create an instance for evalling
    EvallingModuleInstance instance(wasm, &interface, linkedInstances);
    // set up the stack area and other environment details
    instance.setupEnvironment();
    // we should not add new globals from here on; as a result, using
    // an imported global will fail, as it is missing and so looks new
    instance.globals.seal();
    // go one by one, in order, until we fail
    // TODO: if we knew priorities, we could reorder?
    for (auto& ctor : ctors) {
      std::cerr << "trying to eval " << ctor << '\n';
      // snapshot memory, as either the entire function is done, or none
      auto memoryBefore = wasm.memory;
      // snapshot globals (note that STACKTOP might be modified, but should
      // be returned, so that works out)
      auto globalsBefore = instance.globals;
      Export* ex = wasm.getExportOrNull(ctor);
      if (!ex) {
        Fatal() << "export not found: " << ctor;
      }
      try {
        instance.callFunction(ex->value, LiteralList());
      } catch (FailToEvalException& fail) {
        // that's it, we failed, so stop here, cleaning up partial
        // memory changes first
        std::cerr << "  ...stopping since could not eval: " << fail.why << "\n";
        wasm.memory = memoryBefore;
        return;
      }
      if (instance.globals != globalsBefore) {
        std::cerr << "  ...stopping since globals modified\n";
        wasm.memory = memoryBefore;
        return;
      }
      std::cerr << "  ...success on " << ctor << ".\n";
      // success, the entire function was evalled!
      // we can nop the function (which may be used elsewhere)
      // and remove the export
      auto* exp = wasm.getExport(ctor);
      auto* func = wasm.getFunction(exp->value);
      func->body = wasm.allocator.alloc<Nop>();
      wasm.removeExport(exp->name);
    }
  } catch (FailToEvalException& fail) {
    // that's it, we failed to even create the instance
    std::cerr << "  ...stopping since could not create module instance: "
              << fail.why << "\n";
    return;
  }
}

//
// main
//

int main(int argc, const char* argv[]) {
  Name entry;
  std::vector<std::string> passes;
  bool emitBinary = true;
  bool debugInfo = false;
  std::string ctorsString;

  ToolOptions options("wasm-ctor-eval",
                      "Execute C++ global constructors ahead of time");
  options
    .add("--output",
         "-o",
         "Output file (stdout if not specified)",
         Options::Arguments::One,
         [](Options* o, const std::string& argument) {
           o->extra["output"] = argument;
           Colors::setEnabled(false);
         })
    .add("--emit-text",
         "-S",
         "Emit text instead of binary for the output file",
         Options::Arguments::Zero,
         [&](Options* o, const std::string& argument) { emitBinary = false; })
    .add("--debuginfo",
         "-g",
         "Emit names section and debug info",
         Options::Arguments::Zero,
         [&](Options* o, const std::string& arguments) { debugInfo = true; })
    .add(
      "--ctors",
      "-c",
      "Comma-separated list of global constructor functions to evaluate",
      Options::Arguments::One,
      [&](Options* o, const std::string& argument) { ctorsString = argument; })
    .add_positional("INFILE",
                    Options::Arguments::One,
                    [](Options* o, const std::string& argument) {
                      o->extra["infile"] = argument;
                    });
  options.parse(argc, argv);

  auto input(read_file<std::string>(options.extra["infile"], Flags::Text));

  Module wasm;

  {
    if (options.debug) {
      std::cerr << "reading...\n";
    }
    ModuleReader reader;
    try {
      reader.read(options.extra["infile"], wasm);
    } catch (ParseException& p) {
      p.dump(std::cerr);
      Fatal() << "error in parsing input";
    }
  }

  options.applyFeatures(wasm);

  if (!WasmValidator().validate(wasm)) {
    std::cout << wasm << '\n';
    Fatal() << "error in validating input";
  }

  // get list of ctors, and eval them
  std::vector<std::string> ctors;
  std::istringstream stream(ctorsString);
  std::string temp;
  while (std::getline(stream, temp, ',')) {
    ctors.push_back(temp);
  }
  evalCtors(wasm, ctors);

  // Do some useful optimizations after the evalling
  {
    PassRunner passRunner(&wasm);
    passRunner.add("memory-packing"); // we flattened it, so re-optimize
    passRunner.add("remove-unused-names");
    passRunner.add("dce");
    passRunner.add("merge-blocks");
    passRunner.add("vacuum");
    passRunner.add("remove-unused-module-elements");
    passRunner.run();
  }

  if (options.extra.count("output") > 0) {
    if (options.debug) {
      std::cerr << "writing..." << std::endl;
    }
    ModuleWriter writer;
    writer.setBinary(emitBinary);
    writer.setDebugInfo(debugInfo);
    writer.write(wasm, options.extra["output"]);
  }
}
