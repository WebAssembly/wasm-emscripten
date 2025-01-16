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
// wasm-emscripten-finalize console tool
// Performs Emscripten-specific transforms on .wasm files
//

#include <exception>

#include "abi/js.h"
#include "ir/trapping.h"
#include "support/colors.h"
#include "support/debug.h"
#include "support/file.h"
#include "tool-options.h"
#include "wasm-binary.h"
#include "wasm-emscripten.h"
#include "wasm-io.h"
#include "wasm-validator.h"

#define DEBUG_TYPE "emscripten"

using namespace wasm;

int main(int argc, const char* argv[]) {
  std::string infile;
  std::string outfile;
  std::string inputSourceMapFilename;
  std::string outputSourceMapFilename;
  std::string outputSourceMapUrl;
  bool emitBinary = true;
  bool debugInfo = false;
  bool DWARF = false;
  bool sideModule = false;
  bool legalizeJavaScriptFFI = true;
  bool bigInt = false;
  bool checkStackOverflow = false;
  bool standaloneWasm = false;
  // TODO: remove after https://github.com/WebAssembly/binaryen/issues/3043
  bool minimizeWasmChanges = false;
  bool noDynCalls = false;
  bool onlyI64DynCalls = false;
  bool memoryPacking = false;

  const std::string WasmEmscriptenFinalizeOption =
    "wasm-emscripten-finalize options";

  ToolOptions options("wasm-emscripten-finalize",
                      "Performs Emscripten-specific transforms on .wasm files");
  options
    .add("--output",
         "-o",
         "Output file",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::One,
         [&outfile](Options*, const std::string& argument) {
           outfile = argument;
           Colors::setEnabled(false);
         })
    .add("--debuginfo",
         "-g",
         "Emit names section in wasm binary (or full debuginfo in wast)",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&debugInfo](Options*, const std::string&) { debugInfo = true; })
    .add("--dwarf",
         "",
         "Update DWARF debug info",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&DWARF](Options*, const std::string&) { DWARF = true; })
    .add("--emit-text",
         "-S",
         "Emit text instead of binary for the output file. "
         "In this mode if no output file is specified, we write to stdout.",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&emitBinary](Options*, const std::string&) { emitBinary = false; })
    .add("--side-module",
         "",
         "Input is an emscripten side module",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&sideModule](Options* o, const std::string& argument) {
           sideModule = true;
         })
    .add("--input-source-map",
         "-ism",
         "Consume source map from the specified file",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::One,
         [&inputSourceMapFilename](Options* o, const std::string& argument) {
           inputSourceMapFilename = argument;
         })
    .add("--no-legalize-javascript-ffi",
         "-nj",
         "Do not fully legalize (i64->i32, "
         "f32->f64) the imports and exports for interfacing with JS",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&legalizeJavaScriptFFI](Options* o, const std::string&) {
           legalizeJavaScriptFFI = false;
         })
    .add("--bigint",
         "-bi",
         "Assume JS will use wasm/JS BigInt integration, so wasm i64s will "
         "turn into JS BigInts, and there is no need for any legalization at "
         "all (not even minimal legalization of dynCalls)",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&bigInt](Options* o, const std::string&) { bigInt = true; })
    .add("--output-source-map",
         "-osm",
         "Emit source map to the specified file",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::One,
         [&outputSourceMapFilename](Options* o, const std::string& argument) {
           outputSourceMapFilename = argument;
         })
    .add("--output-source-map-url",
         "-osu",
         "Emit specified string as source map URL",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::One,
         [&outputSourceMapUrl](Options* o, const std::string& argument) {
           outputSourceMapUrl = argument;
         })
    .add("--check-stack-overflow",
         "",
         "Check for stack overflows every time the stack is extended",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&checkStackOverflow](Options* o, const std::string&) {
           checkStackOverflow = true;
         })
    .add("--standalone-wasm",
         "",
         "Emit a wasm file that does not depend on JS, as much as possible,"
         " using wasi and other standard conventions etc. where possible",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&standaloneWasm](Options* o, const std::string&) {
           standaloneWasm = true;
         })
    .add("--minimize-wasm-changes",
         "",
         "Modify the wasm as little as possible. This is useful during "
         "development as we reduce the number of changes to the wasm, as it "
         "lets emscripten control how much modifications to do.",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&minimizeWasmChanges](Options* o, const std::string&) {
           minimizeWasmChanges = true;
         })
    .add("--no-dyncalls",
         "",
         "",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&noDynCalls](Options* o, const std::string&) { noDynCalls = true; })
    .add("--dyncalls-i64",
         "",
         "",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&onlyI64DynCalls](Options* o, const std::string&) {
           onlyI64DynCalls = true;
         })
    .add("--pack-memory",
         "",
         "Pack memory segments before writing out memory file",
         WasmEmscriptenFinalizeOption,
         Options::Arguments::Zero,
         [&memoryPacking](Options*, const std::string&) {
           memoryPacking = true;
         })
    .add_positional("INFILE",
                    Options::Arguments::One,
                    [&infile](Options* o, const std::string& argument) {
                      infile = argument;
                    });
  options.parse(argc, argv);

  if (infile == "") {
    Fatal() << "Need to specify an infile\n";
  }

  // We will write the modified wasm if the user asked us to, either by
  // specifying an output file or requesting text output (which goes to stdout
  // by default).
  auto writeOutput = outfile.size() > 0 || !emitBinary;

  Module wasm;
  options.applyOptionsBeforeParse(wasm);
  ModuleReader reader;
  // If we are not writing output then we definitely don't need to read debug
  // info. However, if we emit output then definitely load the names section so
  // that we roundtrip names properly.
  reader.setDebugInfo(writeOutput);
  reader.setDWARF(DWARF && writeOutput);
  if (!writeOutput) {
    // If we are not writing the output then all we are doing is simple parsing
    // of metadata from global parts of the wasm such as imports and exports. In
    // that case, it is unnecessary to parse function contents which are the
    // great bulk of the work, and we can skip all that.
    // Note that the one case we do need function bodies for, pthreads + EM_ASM
    // parsing, requires special handling. The start function has code that we
    // parse in order to find the EM_ASMs, and for that reason the binary reader
    // will still parse the start function even in this mode.
    reader.setSkipFunctionBodies(true);
  }
  try {
    reader.read(infile, wasm, inputSourceMapFilename);
  } catch (ParseException& p) {
    p.dump(std::cerr);
    std::cerr << '\n';
    Fatal() << "error in parsing input";
  } catch (MapParseException& p) {
    p.dump(std::cerr);
    std::cerr << '\n';
    Fatal() << "error in parsing wasm source map";
  }

  options.applyOptionsAfterParse(wasm);

  BYN_TRACE_WITH_TYPE("emscripten-dump", "Module before:\n");
  BYN_DEBUG_WITH_TYPE("emscripten-dump", std::cerr << &wasm);

  EmscriptenGlueGenerator generator(wasm);
  generator.standalone = standaloneWasm;
  generator.sideModule = sideModule;
  generator.minimizeWasmChanges = minimizeWasmChanges;
  generator.onlyI64DynCalls = onlyI64DynCalls;
  generator.noDynCalls = noDynCalls;

  PassRunner passRunner(&wasm, options.passOptions);
  passRunner.setDebug(options.debug);
  passRunner.setDebugInfo(debugInfo);

  if (checkStackOverflow) {
    if (!standaloneWasm) {
      // In standalone mode we don't set a handler at all.. which means
      // just trap on overflow.
      passRunner.options.arguments["stack-check-handler"] =
        "__handle_stack_overflow";
    }
    passRunner.add("stack-check");
  }

  if (!noDynCalls && !standaloneWasm) {
    // If not standalone wasm then JS is relevant and we need dynCalls.
    if (onlyI64DynCalls) {
      passRunner.add("generate-i64-dyncalls");
    } else {
      passRunner.add("generate-dyncalls");
    }
  }

  // Legalize the wasm, if BigInts don't make that moot.
  if (!bigInt && legalizeJavaScriptFFI) {
    passRunner.add("legalize-js-interface");
  }

  // If DWARF is unused, strip it out. This avoids us keeping it alive
  // until wasm-opt strips it later.
  if (!DWARF) {
    passRunner.add("strip-dwarf");
  }

  // If we are going to write out the memory to file, add
  // `--zero-filled-memory` was passed then run the memory packing
  // pass now, before we write out the file.
  // TODO(sbc): Move the writing of the memory file to a later phase
  // and remove this option.
  if (!dataSegmentFile.empty() && memoryPacking) {
    passRunner.options.zeroFilledMemory = true;
    passRunner.add("memory-packing");
  }

  passRunner.run();

  BYN_TRACE_WITH_TYPE("emscripten-dump", "Module after:\n");
  BYN_DEBUG_WITH_TYPE("emscripten-dump", std::cerr << wasm << '\n');

  if (writeOutput) {
    Output output(outfile, emitBinary ? Flags::Binary : Flags::Text);
    ModuleWriter writer(options.passOptions);
    writer.setDebugInfo(debugInfo);
    // writer.setSymbolMap(symbolMap);
    writer.setBinary(emitBinary);
    if (outputSourceMapFilename.size()) {
      writer.setSourceMapFilename(outputSourceMapFilename);
      writer.setSourceMapUrl(outputSourceMapUrl);
    }
    writer.write(wasm, output);
  }
  return 0;
}
