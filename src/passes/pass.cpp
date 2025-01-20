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

#include <chrono>
#include <sstream>

#ifdef __linux__
#include <unistd.h>
#endif

#include "ir/hashed.h"
#include "ir/module-utils.h"
#include "ir/type-updating.h"
#include "pass.h"
#include "passes/passes.h"
#include "support/colors.h"
#include "wasm-debug.h"
#include "wasm-io.h"
#include "wasm-validator.h"

namespace wasm {

// PassRegistry

PassRegistry::PassRegistry() { registerPasses(); }

static PassRegistry singleton;

PassRegistry* PassRegistry::get() { return &singleton; }

void PassRegistry::registerPass(const char* name,
                                const char* description,
                                Creator create) {
  assert(passInfos.find(name) == passInfos.end());
  passInfos[name] = PassInfo(description, create);
}

void PassRegistry::registerTestPass(const char* name,
                                    const char* description,
                                    Creator create) {
  assert(passInfos.find(name) == passInfos.end());
  passInfos[name] = PassInfo(description, create, true);
}

std::unique_ptr<Pass> PassRegistry::createPass(std::string name) {
  if (passInfos.find(name) == passInfos.end()) {
    Fatal() << "Could not find pass: " << name << "\n";
  }
  std::unique_ptr<Pass> ret;
  ret.reset(passInfos[name].create());
  ret->name = name;
  return ret;
}

std::vector<std::string> PassRegistry::getRegisteredNames() {
  std::vector<std::string> ret;
  for (auto& [name, _] : passInfos) {
    ret.push_back(name);
  }
  return ret;
}

bool PassRegistry::containsPass(const std::string& name) {
  return passInfos.count(name) > 0;
}

std::string PassRegistry::getPassDescription(std::string name) {
  assert(passInfos.find(name) != passInfos.end());
  return passInfos[name].description;
}

bool PassRegistry::isPassHidden(std::string name) {
  assert(passInfos.find(name) != passInfos.end());
  return passInfos[name].hidden;
}

// PassRunner

void PassRegistry::registerPasses() {
  registerPass("alignment-lowering",
               "lower unaligned loads and stores to smaller aligned ones",
               createAlignmentLoweringPass);
  registerPass("asyncify",
               "async/await style transform, allowing pausing and resuming",
               createAsyncifyPass);
  registerPass("avoid-reinterprets",
               "Tries to avoid reinterpret operations via more loads",
               createAvoidReinterpretsPass);
  registerPass(
    "dae", "removes arguments to calls in an lto-like manner", createDAEPass);
  registerPass("dae-optimizing",
               "removes arguments to calls in an lto-like manner, and "
               "optimizes where we removed",
               createDAEOptimizingPass);
  registerPass("abstract-type-refining",
               "refine and merge abstract (never-created) types",
               createAbstractTypeRefiningPass);
  registerPass("coalesce-locals",
               "reduce # of locals by coalescing",
               createCoalesceLocalsPass);
  registerPass("coalesce-locals-learning",
               "reduce # of locals by coalescing and learning",
               createCoalesceLocalsWithLearningPass);
  registerPass("code-pushing",
               "push code forward, potentially making it not always execute",
               createCodePushingPass);
  registerPass(
    "code-folding", "fold code, merging duplicates", createCodeFoldingPass);
  registerPass("const-hoisting",
               "hoist repeated constants to a local",
               createConstHoistingPass);
  registerPass("cfp",
               "propagate constant struct field values",
               createConstantFieldPropagationPass);
  registerPass("cfp-reftest",
               "propagate constant struct field values, using ref.test",
               createConstantFieldPropagationRefTestPass);
  registerPass(
    "dce", "removes unreachable code", createDeadCodeEliminationPass);
  registerPass("dealign",
               "forces all loads and stores to have alignment 1",
               createDeAlignPass);
  registerPass(
    "propagate-debug-locs",
    "propagate debug location from parents or previous siblings to child nodes",
    createDebugLocationPropagationPass);
  registerPass("denan",
               "instrument the wasm to convert NaNs into 0 at runtime",
               createDeNaNPass);
  registerPass(
    "directize", "turns indirect calls into direct ones", createDirectizePass);
  registerPass("discard-global-effects",
               "discards global effect info",
               createDiscardGlobalEffectsPass);
  registerPass(
    "dfo", "optimizes using the DataFlow SSA IR", createDataFlowOptsPass);
  registerPass("dwarfdump",
               "dump DWARF debug info sections from the read binary",
               createDWARFDumpPass);
  registerPass("duplicate-import-elimination",
               "removes duplicate imports",
               createDuplicateImportEliminationPass);
  registerPass("duplicate-function-elimination",
               "removes duplicate functions",
               createDuplicateFunctionEliminationPass);
  registerPass("emit-target-features",
               "emit the target features section in the output",
               createEmitTargetFeaturesPass);
  registerPass("enclose-world",
               "modify the wasm (destructively) for closed-world",
               createEncloseWorldPass);
  registerPass("extract-function",
               "leaves just one function (useful for debugging)",
               createExtractFunctionPass);
  registerPass("extract-function-index",
               "leaves just one function selected by index",
               createExtractFunctionIndexPass);
  registerPass(
    "flatten", "flattens out code, removing nesting", createFlattenPass);
  registerPass("fpcast-emu",
               "emulates function pointer casts, allowing incorrect indirect "
               "calls to (sometimes) work",
               createFuncCastEmulationPass);
  registerPass(
    "func-metrics", "reports function metrics", createFunctionMetricsPass);
  registerPass("generate-dyncalls",
               "generate dynCall fuctions used by emscripten ABI",
               createGenerateDynCallsPass);
  registerPass(
    "generate-i64-dyncalls",
    "generate dynCall functions used by emscripten ABI, but only for "
    "functions with i64 in their signature (which cannot be invoked "
    "via the wasm table without JavaScript BigInt support).",
    createGenerateI64DynCallsPass);
  registerPass("generate-global-effects",
               "generate global effect info (helps later passes)",
               createGenerateGlobalEffectsPass);
  registerPass(
    "global-refining", "refine the types of globals", createGlobalRefiningPass);
  registerPass(
    "gsi", "globally optimize struct values", createGlobalStructInferencePass);
  registerPass(
    "gto", "globally optimize GC types", createGlobalTypeOptimizationPass);
  registerPass("gufa",
               "Grand Unified Flow Analysis: optimize the entire program using "
               "information about what content can actually appear in each "
               "location",
               createGUFAPass);
  registerPass("gufa-cast-all",
               "GUFA plus add casts for all inferences",
               createGUFACastAllPass);
  registerPass("gufa-optimizing",
               "GUFA plus local optimizations in functions we modified",
               createGUFAOptimizingPass);
  registerPass(
    "optimize-j2cl", "optimizes J2CL specific constructs.", createJ2CLOptsPass);
  registerPass(
    "merge-j2cl-itables",
    "Merges itable structures into vtables to make types more compact",
    createJ2CLItableMergingPass);
  registerPass("type-refining",
               "apply more specific subtypes to type fields where possible",
               createTypeRefiningPass);
  registerPass(
    "heap2local", "replace GC allocations with locals", createHeap2LocalPass);
  registerPass("heap-store-optimization",
               "optimize heap (GC) stores",
               createHeapStoreOptimizationPass);
  registerPass(
    "inline-main", "inline __original_main into main", createInlineMainPass);
  registerPass("inlining",
               "inline functions (you probably want inlining-optimizing)",
               createInliningPass);
  registerPass("inlining-optimizing",
               "inline functions and optimizes where we inlined",
               createInliningOptimizingPass);
  registerPass("intrinsic-lowering",
               "lower away binaryen intrinsics",
               createIntrinsicLoweringPass);
  registerPass("jspi",
               "wrap imports and exports for JavaScript promise integration",
               createJSPIPass);
  registerPass("legalize-js-interface",
               "legalizes i64 types on the import/export boundary",
               createLegalizeJSInterfacePass);
  registerPass("legalize-and-prune-js-interface",
               "legalizes the import/export boundary and prunes when needed",
               createLegalizeAndPruneJSInterfacePass);
  registerPass("local-cse",
               "common subexpression elimination inside basic blocks",
               createLocalCSEPass);
  registerPass("local-subtyping",
               "apply more specific subtypes to locals where possible",
               createLocalSubtypingPass);
  registerPass("log-execution",
               "instrument the build with logging of where execution goes",
               createLogExecutionPass);
  registerPass("i64-to-i32-lowering",
               "lower all uses of i64s to use i32s instead",
               createI64ToI32LoweringPass);
  registerPass(
    "trace-calls",
    "instrument the build with code to intercept specific function calls",
    createTraceCallsPass);
  registerPass(
    "instrument-locals",
    "instrument the build with code to intercept all loads and stores",
    createInstrumentLocalsPass);
  registerPass(
    "instrument-memory",
    "instrument the build with code to intercept all loads and stores",
    createInstrumentMemoryPass);
  registerPass(
    "licm", "loop invariant code motion", createLoopInvariantCodeMotionPass);
  registerPass("limit-segments",
               "attempt to merge segments to fit within web limits",
               createLimitSegmentsPass);
  registerPass("memory64-lowering",
               "lower loads and stores to a 64-bit memory to instead use a "
               "32-bit one",
               createMemory64LoweringPass);
  registerPass("table64-lowering",
               "alias for memory64-lowering",
               createMemory64LoweringPass);
  registerPass("llvm-memory-copy-fill-lowering",
               "Lower memory.copy and memory.fill to wasm mvp and disable "
               "the bulk-memory feature.",
               createLLVMMemoryCopyFillLoweringPass);
  registerPass("memory-packing",
               "packs memory into separate segments, skipping zeros",
               createMemoryPackingPass);
  registerPass(
    "merge-blocks", "merges blocks to their parents", createMergeBlocksPass);
  registerPass("merge-similar-functions",
               "merges similar functions when benefical",
               createMergeSimilarFunctionsPass);
  registerPass(
    "merge-locals", "merges locals when beneficial", createMergeLocalsPass);
  registerPass("metrics",
               "reports metrics (with an optional title, --metrics[=TITLE])",
               createMetricsPass);
  registerPass("minify-imports",
               "minifies import names (only those, and not export names), and "
               "emits a mapping to the minified ones",
               createMinifyImportsPass);
  registerPass("minify-imports-and-exports",
               "minifies both import and export names, and emits a mapping to "
               "the minified ones",
               createMinifyImportsAndExportsPass);
  registerPass("minify-imports-and-exports-and-modules",
               "minifies both import and export names, and emits a mapping to "
               "the minified ones, and minifies the modules as well",
               createMinifyImportsAndExportsAndModulesPass);
  registerPass("minimize-rec-groups",
               "Split types into minimal recursion groups",
               createMinimizeRecGroupsPass);
  registerPass("mod-asyncify-always-and-only-unwind",
               "apply the assumption that asyncify imports always unwind, "
               "and we never rewind",
               createModAsyncifyAlwaysOnlyUnwindPass);
  registerPass("mod-asyncify-never-unwind",
               "apply the assumption that asyncify never unwinds",
               createModAsyncifyNeverUnwindPass);
  registerPass("monomorphize",
               "creates specialized versions of functions",
               createMonomorphizePass);
  registerPass("monomorphize-always",
               "creates specialized versions of functions (even if unhelpful)",
               createMonomorphizeAlwaysPass);
  registerPass("multi-memory-lowering",
               "combines multiple memories into a single memory",
               createMultiMemoryLoweringPass);
  registerPass(
    "multi-memory-lowering-with-bounds-checks",
    "combines multiple memories into a single memory, trapping if the read or "
    "write is larger than the length of the memory's data",
    createMultiMemoryLoweringWithBoundsChecksPass);
  registerPass("nm", "name list", createNameListPass);
  registerPass("name-types", "(re)name all heap types", createNameTypesPass);
  registerPass("no-inline", "mark functions as no-inline", createNoInlinePass);
  registerPass("no-full-inline",
               "mark functions as no-inline (for full inlining only)",
               createNoFullInlinePass);
  registerPass("no-partial-inline",
               "mark functions as no-inline (for partial inlining only)",
               createNoPartialInlinePass);
  registerPass("llvm-nontrapping-fptoint-lowering",
               "lower nontrapping float-to-int operations to wasm mvp and "
               "disable the nontrapping fptoint feature",
               createLLVMNonTrappingFPToIntLoweringPass);
  registerPass("once-reduction",
               "reduces calls to code that only runs once",
               createOnceReductionPass);
  registerPass("optimize-added-constants",
               "optimizes added constants into load/store offsets",
               createOptimizeAddedConstantsPass);
  registerPass("optimize-added-constants-propagate",
               "optimizes added constants into load/store offsets, propagating "
               "them across locals too",
               createOptimizeAddedConstantsPropagatePass);
  registerPass(
    "optimize-casts", "eliminate and reuse casts", createOptimizeCastsPass);
  registerPass("optimize-instructions",
               "optimizes instruction combinations",
               createOptimizeInstructionsPass);
// Outlining currently relies on LLVM's SuffixTree, which we can't rely upon
// when building Binaryen for Emscripten.
#ifndef SKIP_OUTLINING
  registerPass("outlining", "outline instructions", createOutliningPass);
#endif
  registerPass("pick-load-signs",
               "pick load signs based on their uses",
               createPickLoadSignsPass);
  registerPass(
    "poppify", "Tranform Binaryen IR into Poppy IR", createPoppifyPass);
  registerPass("post-emscripten",
               "miscellaneous optimizations for Emscripten-generated code",
               createPostEmscriptenPass);
  registerPass("optimize-for-js",
               "early optimize of the instruction combinations for js",
               createOptimizeForJSPass);
  registerPass("precompute",
               "computes compile-time evaluatable expressions",
               createPrecomputePass);
  registerPass("precompute-propagate",
               "computes compile-time evaluatable expressions and propagates "
               "them through locals",
               createPrecomputePropagatePass);
  registerPass("print", "print in s-expression format", createPrinterPass);
  registerPass("print-minified",
               "print in minified s-expression format",
               createMinifiedPrinterPass);
  registerPass("print-features",
               "print options for enabled features",
               createPrintFeaturesPass);
  registerPass(
    "print-full", "print in full s-expression format", createFullPrinterPass);
  registerPass(
    "print-call-graph", "print call graph", createPrintCallGraphPass);

  // Register PrintFunctionMap using its normal name.
  registerPass("print-function-map",
               "print a map of function indexes to names",
               createPrintFunctionMapPass);
  // Also register it as "symbolmap" so that  wasm-opt --symbolmap=foo  is the
  // same as  wasm-as --symbolmap=foo  even though the latter is not a pass
  // (wasm-as cannot run arbitrary passes).
  // TODO: switch emscripten to this name, then remove the old one
  registerPass(
    "symbolmap", "(alias for print-function-map)", createPrintFunctionMapPass);

  registerPass("propagate-globals-globally",
               "propagate global values to other globals (useful for tests)",
               createPropagateGlobalsGloballyPass);
  registerPass("remove-non-js-ops",
               "removes operations incompatible with js",
               createRemoveNonJSOpsPass);
  registerPass("remove-imports",
               "removes imports and replaces them with nops",
               createRemoveImportsPass);
  registerPass("remove-memory-init",
               "removes memory initialization",
               createRemoveMemoryInitPass);
  registerPass("remove-memory",
               "removes memory init (legacy name)",
               createRemoveMemoryInitPass);
  registerPass("remove-unused-brs",
               "removes breaks from locations that are not needed",
               createRemoveUnusedBrsPass);
  registerPass("remove-unused-module-elements",
               "removes unused module elements",
               createRemoveUnusedModuleElementsPass);
  registerPass("remove-unused-nonfunction-module-elements",
               "removes unused module elements that are not functions",
               createRemoveUnusedNonFunctionModuleElementsPass);
  registerPass("remove-unused-names",
               "removes names from locations that are never branched to",
               createRemoveUnusedNamesPass);
  registerPass("remove-unused-types",
               "remove unused private GC types",
               createRemoveUnusedTypesPass);
  registerPass("reorder-functions-by-name",
               "sorts functions by name (useful for debugging)",
               createReorderFunctionsByNamePass);
  registerPass("reorder-functions",
               "sorts functions by access frequency",
               createReorderFunctionsPass);
  registerPass("reorder-globals",
               "sorts globals by access frequency",
               createReorderGlobalsPass);
  registerTestPass("reorder-globals-always",
                   "sorts globals by access frequency (even if there are few)",
                   createReorderGlobalsAlwaysPass);
  registerPass("reorder-locals",
               "sorts locals by access frequency",
               createReorderLocalsPass);
  registerPass("rereloop",
               "re-optimize control flow using the relooper algorithm",
               createReReloopPass);
  registerPass(
    "rse", "remove redundant local.sets", createRedundantSetEliminationPass);
  registerPass("roundtrip",
               "write the module to binary, then read it",
               createRoundTripPass);
  registerPass("safe-heap",
               "instrument loads and stores to check for invalid behavior",
               createSafeHeapPass);
  registerPass("set-globals",
               "sets specified globals to specified values",
               createSetGlobalsPass);
  registerPass("separate-data-segments",
               "write data segments to a file and strip them from the module",
               createSeparateDataSegmentsPass);
  registerPass("signature-pruning",
               "remove params from function signature types where possible",
               createSignaturePruningPass);
  registerPass("signature-refining",
               "apply more specific subtypes to signature types where possible",
               createSignatureRefiningPass);
  registerPass("signext-lowering",
               "lower sign-ext operations to wasm mvp and disable the sign "
               "extension feature",
               createSignExtLoweringPass);
  registerPass("simplify-globals",
               "miscellaneous globals-related optimizations",
               createSimplifyGlobalsPass);
  registerPass("simplify-globals-optimizing",
               "miscellaneous globals-related optimizations, and optimizes "
               "where we replaced global.gets with constants",
               createSimplifyGlobalsOptimizingPass);
  registerPass("simplify-locals",
               "miscellaneous locals-related optimizations",
               createSimplifyLocalsPass);
  registerPass("simplify-locals-nonesting",
               "miscellaneous locals-related optimizations (no nesting at all; "
               "preserves flatness)",
               createSimplifyLocalsNoNestingPass);
  registerPass("simplify-locals-notee",
               "miscellaneous locals-related optimizations (no tees)",
               createSimplifyLocalsNoTeePass);
  registerPass("simplify-locals-nostructure",
               "miscellaneous locals-related optimizations (no structure)",
               createSimplifyLocalsNoStructurePass);
  registerPass(
    "simplify-locals-notee-nostructure",
    "miscellaneous locals-related optimizations (no tees or structure)",
    createSimplifyLocalsNoTeeNoStructurePass);
  registerPass("souperify", "emit Souper IR in text form", createSouperifyPass);
  registerPass("souperify-single-use",
               "emit Souper IR in text form (single-use nodes only)",
               createSouperifySingleUsePass);
  registerPass("spill-pointers",
               "spill pointers to the C stack (useful for Boehm-style GC)",
               createSpillPointersPass);
  registerPass("stub-unsupported-js",
               "stub out unsupported JS operations",
               createStubUnsupportedJSOpsPass);
  registerPass("ssa",
               "ssa-ify variables so that they have a single assignment",
               createSSAifyPass);
  registerPass(
    "ssa-nomerge",
    "ssa-ify variables so that they have a single assignment, ignoring merges",
    createSSAifyNoMergePass);
  registerPass("string-gathering",
               "gathers wasm strings to globals",
               createStringGatheringPass);
  registerPass("string-lowering",
               "lowers wasm strings and operations to imports",
               createStringLoweringPass);
  registerPass(
    "string-lowering-magic-imports",
    "same as string-lowering, but encodes well-formed strings as magic imports",
    createStringLoweringMagicImportPass);
  registerPass("string-lowering-magic-imports-assert",
               "same as string-lowering-magic-imports, but raise a fatal error "
               "if there are invalid strings",
               createStringLoweringMagicImportAssertPass);
  registerPass(
    "strip", "deprecated; same as strip-debug", createStripDebugPass);
  registerPass("stack-check",
               "enforce limits on llvm's __stack_pointer global",
               createStackCheckPass);
  registerPass("strip-debug",
               "strip debug info (including the names section)",
               createStripDebugPass);
  registerPass("strip-dwarf", "strip dwarf debug info", createStripDWARFPass);
  registerPass("strip-producers",
               "strip the wasm producers section",
               createStripProducersPass);
  registerPass("strip-eh", "strip EH instructions", createStripEHPass);
  registerPass("strip-target-features",
               "strip the wasm target features section",
               createStripTargetFeaturesPass);
  registerPass("translate-to-new-eh",
               "deprecated; same as translate-to-exnref",
               createTranslateToExnrefPass);
  registerPass("translate-to-exnref",
               "translate old Phase 3 EH instructions to new ones with exnref",
               createTranslateToExnrefPass);
  registerPass("trap-mode-clamp",
               "replace trapping operations with clamping semantics",
               createTrapModeClamp);
  registerPass("trap-mode-js",
               "replace trapping operations with js semantics",
               createTrapModeJS);
  registerPass("tuple-optimization",
               "optimize trivial tuples away",
               createTupleOptimizationPass);
  registerPass("type-finalizing",
               "mark all leaf types as final",
               createTypeFinalizingPass);
  registerPass("type-merging",
               "merge types to their supertypes where possible",
               createTypeMergingPass);
  registerPass("type-ssa",
               "create new nominal types to help other optimizations",
               createTypeSSAPass);
  registerPass("type-unfinalizing",
               "mark all types as non-final (open)",
               createTypeUnFinalizingPass);
  registerPass("unsubtyping",
               "removes unnecessary subtyping relationships",
               createUnsubtypingPass);
  registerPass("untee",
               "removes local.tees, replacing them with sets and gets",
               createUnteePass);
  registerPass("vacuum", "removes obviously unneeded code", createVacuumPass);
  registerPass(
    "add-throws",
    "add throws for divs by zero, null dereferences, and array OOB access",
    createAddThrowsPass);
  // registerPass(
  //   "lower-i64", "lowers i64 into pairs of i32s", createLowerInt64Pass);

  // Register passes used for internal testing. These don't show up in --help.
  registerTestPass("catch-pop-fixup",
                   "fixup nested pops within catches",
                   createCatchPopFixupPass);
  registerTestPass("experimental-type-generalizing",
                   "generalize types (not yet sound)",
                   createTypeGeneralizingPass);
}

void PassRunner::addIfNoDWARFIssues(std::string passName) {
  auto pass = PassRegistry::get()->createPass(passName);
  if (!pass->invalidatesDWARF() || !shouldPreserveDWARF()) {
    doAdd(std::move(pass));
  }
}

void PassRunner::addDefaultOptimizationPasses() {
  addDefaultGlobalOptimizationPrePasses();
  addDefaultFunctionOptimizationPasses();
  addDefaultGlobalOptimizationPostPasses();
}

void PassRunner::addDefaultFunctionOptimizationPasses() {
  // All the additions here are optional if DWARF must be preserved. That is,
  // when DWARF is relevant we run fewer optimizations.
  // FIXME: support DWARF in all of them.

  // Untangling to semi-ssa form is helpful (but best to ignore merges
  // so as to not introduce new copies).
  if (options.optimizeLevel >= 3 || options.shrinkLevel >= 1) {
    addIfNoDWARFIssues("ssa-nomerge");
  }
  // if we are willing to work very very hard, flatten the IR and do opts
  // that depend on flat IR
  if (options.optimizeLevel >= 4) {
    addIfNoDWARFIssues("flatten");
    // LocalCSE is particularly useful after flatten (see comment in the pass
    // itself), but we must simplify locals a little first (as flatten adds many
    // new and redundant ones, which make things seem different if we do not
    // run some amount of simplify-locals first).
    addIfNoDWARFIssues("simplify-locals-notee-nostructure");
    addIfNoDWARFIssues("local-cse");
    // TODO: add rereloop etc. here
  }
  addIfNoDWARFIssues("dce");
  addIfNoDWARFIssues("remove-unused-names");
  addIfNoDWARFIssues("remove-unused-brs");
  addIfNoDWARFIssues("remove-unused-names");
  addIfNoDWARFIssues("optimize-instructions");
  if (wasm->features.hasGC()) {
    addIfNoDWARFIssues("heap-store-optimization");
  }
  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("pick-load-signs");
  }
  // early propagation
  if (options.optimizeLevel >= 3 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("precompute-propagate");
  } else {
    addIfNoDWARFIssues("precompute");
  }
  if (options.lowMemoryUnused) {
    if (options.optimizeLevel >= 3 || options.shrinkLevel >= 1) {
      addIfNoDWARFIssues("optimize-added-constants-propagate");
    } else {
      addIfNoDWARFIssues("optimize-added-constants");
    }
  }
  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("code-pushing");
  }
  if (wasm->features.hasMultivalue()) {
    // Optimize tuples before local opts (as splitting tuples can help local
    // opts), but also not too early, as we want to be after
    // optimize-instructions at least (which can remove tuple-related things).
    addIfNoDWARFIssues("tuple-optimization");
  }
  // don't create if/block return values yet, as coalesce can remove copies that
  // that could inhibit
  addIfNoDWARFIssues("simplify-locals-nostructure");
  addIfNoDWARFIssues("vacuum"); // previous pass creates garbage
  addIfNoDWARFIssues("reorder-locals");
  // simplify-locals opens opportunities for optimizations
  addIfNoDWARFIssues("remove-unused-brs");
  if (options.optimizeLevel > 1 && wasm->features.hasGC()) {
    addIfNoDWARFIssues("heap2local");
  }
  // if we are willing to work hard, also optimize copies before coalescing
  if (options.optimizeLevel >= 3 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("merge-locals"); // very slow on e.g. sqlite
  }
  if (options.optimizeLevel > 1 && wasm->features.hasGC()) {
    addIfNoDWARFIssues("optimize-casts");
    // Coalescing may prevent subtyping (as a coalesced local must have the
    // supertype of all those combined into it), so subtype first.
    // TODO: when optimizing for size, maybe the order should reverse?
    addIfNoDWARFIssues("local-subtyping");
  }
  addIfNoDWARFIssues("coalesce-locals");
  if (options.optimizeLevel >= 3 || options.shrinkLevel >= 1) {
    addIfNoDWARFIssues("local-cse");
  }
  addIfNoDWARFIssues("simplify-locals");
  addIfNoDWARFIssues("vacuum");
  addIfNoDWARFIssues("reorder-locals");
  addIfNoDWARFIssues("coalesce-locals");
  addIfNoDWARFIssues("reorder-locals");
  addIfNoDWARFIssues("vacuum");
  if (options.optimizeLevel >= 3 || options.shrinkLevel >= 1) {
    addIfNoDWARFIssues("code-folding");
  }
  addIfNoDWARFIssues("merge-blocks"); // makes remove-unused-brs more effective
  addIfNoDWARFIssues(
    "remove-unused-brs"); // coalesce-locals opens opportunities
  addIfNoDWARFIssues(
    "remove-unused-names");           // remove-unused-brs opens opportunities
  addIfNoDWARFIssues("merge-blocks"); // clean up remove-unused-brs new blocks
  // late propagation
  if (options.optimizeLevel >= 3 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("precompute-propagate");
  } else {
    addIfNoDWARFIssues("precompute");
  }
  addIfNoDWARFIssues("optimize-instructions");
  if (wasm->features.hasGC()) {
    addIfNoDWARFIssues("heap-store-optimization");
  }
  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 1) {
    addIfNoDWARFIssues(
      "rse"); // after all coalesce-locals, and before a final vacuum
  }
  addIfNoDWARFIssues("vacuum"); // just to be safe
}

void PassRunner::addDefaultGlobalOptimizationPrePasses() {
  // Removing duplicate functions is fast and saves work later.
  addIfNoDWARFIssues("duplicate-function-elimination");
  // Do a global cleanup before anything heavy, as it is fairly fast and can
  // save a lot of work if there is a significant amount of dead code.
  if (options.optimizeLevel >= 2) {
    addIfNoDWARFIssues("remove-unused-module-elements");
  }
  addIfNoDWARFIssues("memory-packing");
  if (options.optimizeLevel >= 2) {
    addIfNoDWARFIssues("once-reduction");
  }
  if (wasm->features.hasGC() && options.optimizeLevel >= 2) {
    if (options.closedWorld) {
      addIfNoDWARFIssues("type-refining");
      addIfNoDWARFIssues("signature-pruning");
      addIfNoDWARFIssues("signature-refining");
    }
    addIfNoDWARFIssues("global-refining");
    // Global type optimization can remove fields that are not needed, which can
    // remove ref.funcs that were once assigned to vtables but are no longer
    // needed, which can allow more code to be removed globally. After those,
    // constant field propagation can be more effective.
    if (options.closedWorld) {
      addIfNoDWARFIssues("gto");
    }
    addIfNoDWARFIssues("remove-unused-module-elements");
    if (options.closedWorld) {
      addIfNoDWARFIssues("remove-unused-types");
      addIfNoDWARFIssues("cfp");
      addIfNoDWARFIssues("gsi");
      addIfNoDWARFIssues("abstract-type-refining");
    }
  }
  // TODO: generate-global-effects here, right before function passes, then
  //       discard in addDefaultGlobalOptimizationPostPasses? the benefit seems
  //       quite minor so far, except perhaps when using call.without.effects
  //       which can lead to more opportunities for global effects to matter.
}

void PassRunner::add(std::string passName, std::optional<std::string> passArg) {
  auto pass = PassRegistry::get()->createPass(passName);
  if (passArg) {
    pass->setPassArg(*passArg);
  }

  doAdd(std::move(pass));
}

void PassRunner::addDefaultGlobalOptimizationPostPasses() {
  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 1) {
    addIfNoDWARFIssues("dae-optimizing");
  }
  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("inlining-optimizing");
  }

  // Optimizations show more functions as duplicate, so run this here in Post.
  addIfNoDWARFIssues("duplicate-function-elimination");
  addIfNoDWARFIssues("duplicate-import-elimination");

  // perform after the number of functions is reduced by inlining-optimizing
  if (options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("merge-similar-functions");
  }

  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 2) {
    addIfNoDWARFIssues("simplify-globals-optimizing");
  } else {
    addIfNoDWARFIssues("simplify-globals");
  }
  addIfNoDWARFIssues("remove-unused-module-elements");
  if (options.optimizeLevel >= 2 && wasm->features.hasStrings()) {
    // Gather strings to globals right before reorder-globals, which will then
    // sort them properly.
    addIfNoDWARFIssues("string-gathering");
  }
  if (options.optimizeLevel >= 2 || options.shrinkLevel >= 1) {
    addIfNoDWARFIssues("reorder-globals");
  }
  // may allow more inlining/dae/etc., need --converge for that
  addIfNoDWARFIssues("directize");
}

static void dumpWasm(Name name, Module* wasm, const PassOptions& options) {
  static int counter = 0;
  std::string numstr = std::to_string(counter++);
  while (numstr.size() < 3) {
    numstr = '0' + numstr;
  }
  auto fullName = std::string("byn-");
#ifdef __linux__
  // TODO: use _getpid() on windows, elsewhere?
  fullName += std::to_string(getpid()) + '-';
#endif
  fullName += numstr + "-" + name.toString();
  Colors::setEnabled(false);
  ModuleWriter writer(options);
  writer.setDebugInfo(true);
  writer.writeBinary(*wasm, fullName + ".wasm");
}

void PassRunner::run() {
  static const int passDebug = getPassDebug();
  // Emit logging information when asked for. At passDebug level 1+ we log
  // the main passes, while in 2 we also log nested ones. Note that for
  // nested ones we can only emit their name - we can't validate, or save the
  // file, or print, as the wasm may be in an intermediate state that is not
  // valid.
  if (options.debug || (passDebug == 2 || (passDebug && !isNested))) {
    // for debug logging purposes, run each pass in full before running the
    // other
    auto totalTime = std::chrono::duration<double>(0);
    auto what = isNested ? "nested passes" : "passes";
    std::cerr << "[PassRunner] running " << what << std::endl;
    size_t padding = 0;
    for (auto& pass : passes) {
      padding = std::max(padding, pass->name.size());
    }
    if (passDebug >= 3 && !isNested) {
      dumpWasm("before", wasm, options);
    }
    for (auto& pass : passes) {
      // ignoring the time, save a printout of the module before, in case this
      // pass breaks it, so we can print the before and after
      std::stringstream moduleBefore;
      if (passDebug == 2 && !isNested) {
        moduleBefore << *wasm << '\n';
      }
      // prepare to run
      std::cerr << "[PassRunner]   running pass: " << pass->name << "... ";
      for (size_t i = 0; i < padding - pass->name.size(); i++) {
        std::cerr << ' ';
      }
      auto before = std::chrono::steady_clock::now();
      if (pass->isFunctionParallel()) {
        // function-parallel passes should get a new instance per function
        ModuleUtils::iterDefinedFunctions(
          *wasm, [&](Function* func) { runPassOnFunction(pass.get(), func); });
      } else {
        runPass(pass.get());
      }
      auto after = std::chrono::steady_clock::now();
      std::chrono::duration<double> diff = after - before;
      std::cerr << diff.count() << " seconds." << std::endl;
      totalTime += diff;
      if (options.validate && !isNested) {
        // validate, ignoring the time
        std::cerr << "[PassRunner]   (validating)\n";
        if (!WasmValidator().validate(*wasm, options)) {
          std::cout << *wasm << '\n';
          if (passDebug >= 2) {
            Fatal() << "Last pass (" << pass->name
                    << ") broke validation. Here is the module before: \n"
                    << moduleBefore.str() << "\n";
          } else {
            Fatal() << "Last pass (" << pass->name
                    << ") broke validation. Run with BINARYEN_PASS_DEBUG=2 "
                       "in the env to see the earlier state, or 3 to dump "
                       "byn-* files for each pass\n";
          }
        }
      }
      if (passDebug >= 3) {
        dumpWasm(pass->name, wasm, options);
      }
    }
    std::cerr << "[PassRunner] " << what << " took " << totalTime.count()
              << " seconds." << std::endl;
    if (options.validate && !isNested) {
      std::cerr << "[PassRunner] (final validation)\n";
      if (!WasmValidator().validate(*wasm, options)) {
        std::cout << *wasm << '\n';
        Fatal() << "final module does not validate\n";
      }
    }
  } else {
    // non-debug normal mode, run them in an optimal manner - for locality it is
    // better to run as many passes as possible on a single function before
    // moving to the next
    std::vector<Pass*> stack;
    auto flush = [&]() {
      if (stack.size() > 0) {
        // run the stack of passes on all the functions, in parallel
        size_t num = ThreadPool::get()->size();
        std::vector<std::function<ThreadWorkState()>> doWorkers;
        std::atomic<size_t> nextFunction;
        nextFunction.store(0);
        size_t numFunctions = wasm->functions.size();
        for (size_t i = 0; i < num; i++) {
          doWorkers.push_back([&]() {
            auto index = nextFunction.fetch_add(1);
            // get the next task, if there is one
            if (index >= numFunctions) {
              return ThreadWorkState::Finished; // nothing left
            }
            Function* func = this->wasm->functions[index].get();
            if (!func->imported()) {
              // do the current task: run all passes on this function
              for (auto* pass : stack) {
                runPassOnFunction(pass, func);
              }
            }
            if (index + 1 == numFunctions) {
              return ThreadWorkState::Finished; // we did the last one
            }
            return ThreadWorkState::More;
          });
        }
        ThreadPool::get()->work(doWorkers);
      }
      stack.clear();
    };
    for (auto& pass : passes) {
      if (pass->isFunctionParallel()) {
        stack.push_back(pass.get());
      } else {
        flush();
        runPass(pass.get());
      }
    }
    flush();
  }
}

void PassRunner::runOnFunction(Function* func) {
  if (options.debug) {
    std::cerr << "[PassRunner] running passes on function " << func->name
              << std::endl;
  }
  for (auto& pass : passes) {
    runPassOnFunction(pass.get(), func);
  }
}

void PassRunner::doAdd(std::unique_ptr<Pass> pass) {
  if (pass->invalidatesDWARF() && shouldPreserveDWARF()) {
    std::cerr << "warning: running pass '" << pass->name
              << "' which is not fully compatible with DWARF\n";
  }
  if (passRemovesDebugInfo(pass->name)) {
    addedPassesRemovedDWARF = true;
  }
  passes.emplace_back(std::move(pass));
}

void PassRunner::clear() { passes.clear(); }

void PassRunner::runPass(Pass* pass) {
  assert(!pass->isFunctionParallel());

  if (options.passesToSkip.count(pass->name)) {
    return;
  }

  // Passes can only be run once and we deliberately do not clear the pass
  // runner after running the pass, so there must not already be a runner here.
  assert(!pass->getPassRunner());
  pass->setPassRunner(this);
  pass->run(wasm);
  handleAfterEffects(pass);
}

void PassRunner::runPassOnFunction(Pass* pass, Function* func) {
  assert(pass->isFunctionParallel());

  if (options.passesToSkip.count(pass->name)) {
    return;
  }

  auto passDebug = getPassDebug();

  // Add extra validation logic in pass-debug mode 2. The main logic in
  // PassRunner::run will work at the module level, and here for a function-
  // parallel pass we can do the same at the function level: we can print the
  // function before the pass, run the pass on the function, and then if it
  // fails to validate we can show an error and print the state right before the
  // pass broke it.
  //
  // Skip nameless passes for this. Anything without a name is an internal
  // component of some larger pass, and information about it won't be very
  // useful - leave it to the entire module to fail validation in that case.
  bool extraFunctionValidation =
    passDebug == 2 && options.validate && !pass->name.empty();
  std::stringstream bodyBefore;
  if (extraFunctionValidation) {
    bodyBefore << *func->body << '\n';
  }

  // Function-parallel passes get a new instance per function
  auto instance = pass->create();
  instance->setPassRunner(this);
  instance->runOnFunction(wasm, func);
  handleAfterEffects(pass, func);

  if (extraFunctionValidation) {
    if (!WasmValidator().validate(func, *wasm, WasmValidator::Minimal)) {
      Fatal() << "Last nested function-parallel pass (" << pass->name
              << ") broke validation of function " << func->name
              << ". Here is the function body before:\n"
              << bodyBefore.str() << "\n\nAnd here it is now:\n"
              << *func->body << '\n';
    }
  }
}

void PassRunner::handleAfterEffects(Pass* pass, Function* func) {
  if (!pass->modifiesBinaryenIR()) {
    return;
  }

  // Binaryen IR is modified, so we may have work here.

  if (!func) {
    // If no function is provided, then this is not a function-parallel pass,
    // and it may have operated on any of the functions in theory, so run on
    // them all.
    assert(!pass->isFunctionParallel());
    for (auto& func : wasm->functions) {
      handleAfterEffects(pass, func.get());
    }
    return;
  }

  if (pass->requiresNonNullableLocalFixups()) {
    TypeUpdating::handleNonDefaultableLocals(func, *wasm);
  }

  if (options.funcEffectsMap && pass->addsEffects()) {
    // Effects were added, so discard any computed effects for this function.
    options.funcEffectsMap->erase(func->name);
  }
}

int PassRunner::getPassDebug() {
  static const int passDebug =
    getenv("BINARYEN_PASS_DEBUG") ? atoi(getenv("BINARYEN_PASS_DEBUG")) : 0;
  return passDebug;
}

bool PassRunner::passRemovesDebugInfo(const std::string& name) {
  return name == "strip" || name == "strip-debug" || name == "strip-dwarf";
}

bool PassRunner::shouldPreserveDWARF() {
  // Check if the debugging subsystem wants to preserve DWARF.
  if (!Debug::shouldPreserveDWARF(options, *wasm)) {
    return false;
  }

  // We may need DWARF. Check if one of our previous passes would remove it
  // anyhow, in which case, there is nothing to preserve.
  if (addedPassesRemovedDWARF) {
    return false;
  }

  return true;
}

bool Pass::hasArgument(const std::string& key) {
  // An argument with the name of the pass is stored on the instance. Other
  // arguments are in the global storage.
  return key == name ? passArg.has_value() : getPassOptions().hasArgument(key);
}

std::string Pass::getArgument(const std::string& key,
                              const std::string& errorTextIfMissing) {
  if (!hasArgument(key)) {
    Fatal() << errorTextIfMissing;
  }

  return (key == name) ? *passArg
                       : getPassOptions().getArgument(key, errorTextIfMissing);
}

std::string Pass::getArgumentOrDefault(const std::string& key,
                                       const std::string& defaultValue) {
  if (key == name) {
    return passArg.value_or(defaultValue);
  }

  return getPassOptions().getArgumentOrDefault(key, defaultValue);
}

} // namespace wasm
