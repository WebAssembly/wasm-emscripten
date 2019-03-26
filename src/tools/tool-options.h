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

#include "support/command-line.h"
#include "pass.h"

//
// Shared optimization options for commandline tools
//

namespace wasm {

struct ToolOptions : public Options {
  PassOptions passOptions;
  bool explicitFeatures = false;

  ToolOptions(const std::string& command, const std::string& description)
      : Options(command, description) {
    (*this)
        .add("--mvp-features", "-mvp", "Disable all non-MVP features",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features = FeatureSet::MVP;
               explicitFeatures = true;
             })
        .add("--all-features", "-all", "Enable all features (default)",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features = FeatureSet::All;
               explicitFeatures = true;
             })
        .add("--enable-sign-ext", "", "Enable sign extension operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setSignExt();
               explicitFeatures = true;
             })
        .add("--disable-sign-ext", "", "Disable sign extension operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setSignExt(false);
               explicitFeatures = true;
             })
        .add("--enable-threads", "", "Enable atomic operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setAtomics();
               explicitFeatures = true;
             })
        .add("--disable-threads", "", "Disable atomic operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setAtomics(false);
               explicitFeatures = true;
             })
        .add("--enable-mutable-globals", "", "Enable mutable globals",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setMutableGlobals();
               explicitFeatures = true;
             })
        .add("--disable-mutable-globals", "", "Disable mutable globals",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setMutableGlobals(false);
               explicitFeatures = true;
             })
        .add("--enable-nontrapping-float-to-int", "",
             "Enable nontrapping float-to-int operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setTruncSat();
               explicitFeatures = true;
             })
        .add("--disable-nontrapping-float-to-int", "",
             "Disable nontrapping float-to-int operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setTruncSat(false);
               explicitFeatures = true;
             })
        .add("--enable-simd", "",
             "Enable SIMD operations and types",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setSIMD();
               explicitFeatures = true;
             })
        .add("--disable-simd", "",
             "Disable SIMD operations and types",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setSIMD(false);
               explicitFeatures = true;
             })
        .add("--enable-bulk-memory", "",
             "Enable bulk memory operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setBulkMemory();
               explicitFeatures = true;
             })
        .add("--disable-bulk-memory", "",
             "Disable bulk memory operations",
             Options::Arguments::Zero,
             [this](Options *o, const std::string& arguments) {
               passOptions.features.setBulkMemory(false);
               explicitFeatures = true;
             })
        .add("--no-validation", "-n",
             "Disables validation, assumes inputs are correct",
             Options::Arguments::Zero,
             [this](Options* o, const std::string& argument) {
               passOptions.validate = false;
             });
  }
};

} // namespace wasm
