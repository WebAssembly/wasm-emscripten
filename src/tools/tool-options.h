/*
 * Copyright 2018 WebAssembly Community Group participants
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

#ifndef wasm_tools_tool_options_h
#define wasm_tools_tool_options_h

#include "ir/module-utils.h"
#include "pass.h"
#include "support/command-line.h"

//
// Shared options for commandline tools
//

namespace wasm {

struct ToolOptions : public Options {
  PassOptions passOptions;

  bool quiet = false;
  bool preserveTypeOrder = false;
  IRProfile profile = IRProfile::Normal;

  constexpr static const char* ToolOptionsCategory = "Tool options";

  ToolOptions(const std::string& command, const std::string& description)
    : Options(command, description) {
    (*this)
      .add("--mvp-features",
           "-mvp",
           "Disable all non-MVP features",
           ToolOptionsCategory,
           Arguments::Zero,
           [this](Options*, const std::string&) {
             enabledFeatures.setMVP();
             disabledFeatures.setAll();
           })
      .add("--all-features",
           "-all",
           "Enable all features",
           ToolOptionsCategory,
           Arguments::Zero,
           [this](Options*, const std::string&) {
             enabledFeatures.setAll();
             disabledFeatures.setMVP();
           })
      .add("--detect-features",
           "",
           "(deprecated - this flag does nothing)",
           ToolOptionsCategory,
           Arguments::Zero,
           [](Options*, const std::string&) {})
      .add("--quiet",
           "-q",
           "Emit less verbose output and hide trivial warnings.",
           ToolOptionsCategory,
           Arguments::Zero,
           [this](Options*, const std::string&) { quiet = true; })
      .add(
        "--experimental-poppy",
        "",
        "Parse wast files as Poppy IR for testing purposes.",
        ToolOptionsCategory,
        Arguments::Zero,
        [this](Options*, const std::string&) { profile = IRProfile::Poppy; });
    (*this)
      .addFeature(FeatureSet::SignExt, "sign extension operations")
      .addFeature(FeatureSet::Atomics, "atomic operations")
      .addFeature(FeatureSet::MutableGlobals, "mutable globals")
      .addFeature(FeatureSet::TruncSat, "nontrapping float-to-int operations")
      .addFeature(FeatureSet::SIMD, "SIMD operations and types")
      .addFeature(FeatureSet::BulkMemory,
                  "bulk memory operations",
                  FeatureSet(FeatureSet::BulkMemoryOpt))
      .addFeature(FeatureSet::BulkMemoryOpt,
                  "memory.copy and memory.fill",
                  FeatureSet::None,
                  FeatureSet(FeatureSet::BulkMemory))
      .addFeature(FeatureSet::CallIndirectOverlong,
                  "LEB encoding of call-indirect (Ignored for compatibility as "
                  "it has no effect on Binaryen)")
      .addFeature(FeatureSet::ExceptionHandling,
                  "exception handling operations")
      .addFeature(FeatureSet::TailCall, "tail call operations")
      .addFeature(FeatureSet::ReferenceTypes, "reference types")
      .addFeature(FeatureSet::Multivalue, "multivalue functions")
      .addFeature(FeatureSet::GC, "garbage collection")
      .addFeature(FeatureSet::Memory64, "memory64")
      .addFeature(FeatureSet::RelaxedSIMD, "relaxed SIMD")
      .addFeature(FeatureSet::ExtendedConst, "extended const expressions")
      .addFeature(FeatureSet::Strings, "strings")
      .addFeature(FeatureSet::MultiMemory, "multimemory")
      .addFeature(FeatureSet::StackSwitching, "stack switching")
      .addFeature(FeatureSet::SharedEverything, "shared-everything threads")
      .addFeature(FeatureSet::FP16, "float 16 operations")
      .add("--enable-typed-function-references",
           "",
           "Deprecated compatibility flag",
           ToolOptionsCategory,
           Options::Arguments::Zero,
           [](Options* o, const std::string& argument) {
             std::cerr
               << "Warning: Typed function references have been made part of "
                  "GC and --enable-typed-function-references is deprecated\n";
           })
      .add("--disable-typed-function-references",
           "",
           "Deprecated compatibility flag",
           ToolOptionsCategory,
           Options::Arguments::Zero,
           [](Options* o, const std::string& argument) {
             std::cerr
               << "Warning: Typed function references have been made part of "
                  "GC and --disable-typed-function-references is deprecated\n";
           })
      .add("--no-validation",
           "-n",
           "Disables validation, assumes inputs are correct",
           ToolOptionsCategory,
           Options::Arguments::Zero,
           [this](Options* o, const std::string& argument) {
             passOptions.validate = false;
           })
      .add("--pass-arg",
           "-pa",
           "An argument passed along to optimization passes being run. Must be "
           "in the form KEY@VALUE.  If KEY is the name of a pass then it "
           "applies to the closest instance of that pass before us. If KEY is "
           "not the name of a pass then it is a global option that applies to "
           "all pass instances that read it.",
           ToolOptionsCategory,
           Options::Arguments::N,
           [this](Options*, const std::string& argument) {
             std::string key, value;
             auto colon = argument.find('@');
             if (colon == std::string::npos) {
               key = argument;
               value = "1";
             } else {
               key = argument.substr(0, colon);
               value = argument.substr(colon + 1);
             }

             addPassArg(key, value);
           })
      .add(
        "--closed-world",
        "-cw",
        "Assume code outside of the module does not inspect or interact with "
        "GC and function references, even if they are passed out. The outside "
        "may hold on to them and pass them back in, but not inspect their "
        "contents or call them.",
        ToolOptionsCategory,
        Options::Arguments::Zero,
        [this](Options*, const std::string&) {
          passOptions.closedWorld = true;
        })
      .add(
        "--preserve-type-order",
        "",
        "Preserve the order of types from the input (useful for debugging and "
        "testing)",
        ToolOptionsCategory,
        Options::Arguments::Zero,
        [&](Options* o, const std::string& arguments) {
          preserveTypeOrder = true;
        })
      .add("--generate-stack-ir",
           "",
           "generate StackIR during writing",
           ToolOptionsCategory,
           Options::Arguments::Zero,
           [&](Options* o, const std::string& arguments) {
             passOptions.generateStackIR = true;
           })
      .add("--optimize-stack-ir",
           "",
           "optimize StackIR during writing",
           ToolOptionsCategory,
           Options::Arguments::Zero,
           [&](Options* o, const std::string& arguments) {
             // Also generate StackIR, to have something to optimize.
             passOptions.generateStackIR = true;
             passOptions.optimizeStackIR = true;
           })
      .add("--print-stack-ir",
           "",
           "print StackIR during writing",
           ToolOptionsCategory,
           Options::Arguments::Zero,
           [&](Options* o, const std::string& arguments) {
             // Also generate StackIR, to have something to print.
             passOptions.generateStackIR = true;
             passOptions.printStackIR = &std::cout;
           });
  }

  ToolOptions& addFeature(FeatureSet::Feature feature,
                          const std::string& description,
                          FeatureSet impliedEnable = FeatureSet::None,
                          FeatureSet impliedDisable = FeatureSet::None) {
    (*this)
      .add(std::string("--enable-") + FeatureSet::toString(feature),
           "",
           std::string("Enable ") + description,
           ToolOptionsCategory,
           Arguments::Zero,
           [this, feature, impliedEnable](Options*, const std::string&) {
             enabledFeatures.set(feature | impliedEnable, true);
             disabledFeatures.set(feature | impliedEnable, false);
           })

      .add(std::string("--disable-") + FeatureSet::toString(feature),
           "",
           std::string("Disable ") + description,
           ToolOptionsCategory,
           Arguments::Zero,
           [this, feature, impliedDisable](Options*, const std::string&) {
             enabledFeatures.set(feature | impliedDisable, false);
             disabledFeatures.set(feature | impliedDisable, true);
           });
    return *this;
  }

  void applyOptionsBeforeParse(Module& module) const {
    module.features.enable(enabledFeatures);
    module.features.disable(disabledFeatures);
  }

  void applyOptionsAfterParse(Module& module) const {
    if (!preserveTypeOrder) {
      module.typeIndices.clear();
    }
  }

  virtual void addPassArg(const std::string& key, const std::string& value) {
    passOptions.arguments[key] = value;
  }

  virtual ~ToolOptions() = default;

private:
  FeatureSet enabledFeatures = FeatureSet::Default;
  FeatureSet disabledFeatures = FeatureSet::None;
};

} // namespace wasm

#endif
