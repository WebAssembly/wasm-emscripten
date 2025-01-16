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

#ifndef wasm_passes_h
#define wasm_passes_h

namespace wasm {

class Pass;

// Normal passes:
Pass* createAbstractTypeRefiningPass();
Pass* createAlignmentLoweringPass();
Pass* createAsyncifyPass();
Pass* createAvoidReinterpretsPass();
Pass* createCoalesceLocalsPass();
Pass* createCoalesceLocalsWithLearningPass();
Pass* createCodeFoldingPass();
Pass* createCodePushingPass();
Pass* createConstHoistingPass();
Pass* createConstantFieldPropagationPass();
Pass* createConstantFieldPropagationRefTestPass();
Pass* createDAEPass();
Pass* createDAEOptimizingPass();
Pass* createDataFlowOptsPass();
Pass* createDeadCodeEliminationPass();
Pass* createDeNaNPass();
Pass* createDeAlignPass();
Pass* createDebugLocationPropagationPass();
Pass* createDirectizePass();
Pass* createDiscardGlobalEffectsPass();
Pass* createDWARFDumpPass();
Pass* createDuplicateImportEliminationPass();
Pass* createDuplicateFunctionEliminationPass();
Pass* createEmitTargetFeaturesPass();
Pass* createEncloseWorldPass();
Pass* createExtractFunctionPass();
Pass* createExtractFunctionIndexPass();
Pass* createFlattenPass();
Pass* createFuncCastEmulationPass();
Pass* createFullPrinterPass();
Pass* createFunctionMetricsPass();
Pass* createGenerateDynCallsPass();
Pass* createGenerateI64DynCallsPass();
Pass* createGenerateGlobalEffectsPass();
Pass* createGlobalRefiningPass();
Pass* createGlobalStructInferencePass();
Pass* createGlobalTypeOptimizationPass();
Pass* createGUFAPass();
Pass* createGUFACastAllPass();
Pass* createGUFAOptimizingPass();
Pass* createHeap2LocalPass();
Pass* createHeapStoreOptimizationPass();
Pass* createI64ToI32LoweringPass();
Pass* createInlineMainPass();
Pass* createInliningPass();
Pass* createInliningOptimizingPass();
Pass* createJ2CLItableMergingPass();
Pass* createJSPIPass();
Pass* createJ2CLOptsPass();
Pass* createLegalizeAndPruneJSInterfacePass();
Pass* createLegalizeJSInterfacePass();
Pass* createLimitSegmentsPass();
Pass* createLocalCSEPass();
Pass* createLocalSubtypingPass();
Pass* createLogExecutionPass();
Pass* createIntrinsicLoweringPass();
Pass* createTraceCallsPass();
Pass* createInstrumentLocalsPass();
Pass* createInstrumentMemoryPass();
Pass* createLLVMMemoryCopyFillLoweringPass();
Pass* createLoopInvariantCodeMotionPass();
Pass* createMemory64LoweringPass();
Pass* createMemoryPackingPass();
Pass* createMergeBlocksPass();
Pass* createMergeSimilarFunctionsPass();
Pass* createMergeLocalsPass();
Pass* createMinifiedPrinterPass();
Pass* createMinifyImportsPass();
Pass* createMinifyImportsAndExportsPass();
Pass* createMinifyImportsAndExportsAndModulesPass();
Pass* createMinimizeRecGroupsPass();
Pass* createMetricsPass();
Pass* createMonomorphizePass();
Pass* createMonomorphizeAlwaysPass();
Pass* createMultiMemoryLoweringPass();
Pass* createMultiMemoryLoweringWithBoundsChecksPass();
Pass* createNameListPass();
Pass* createNameTypesPass();
Pass* createNoInlinePass();
Pass* createNoFullInlinePass();
Pass* createNoPartialInlinePass();
Pass* createOnceReductionPass();
Pass* createOptimizeAddedConstantsPass();
Pass* createOptimizeAddedConstantsPropagatePass();
Pass* createOptimizeInstructionsPass();
Pass* createOptimizeCastsPass();
Pass* createOptimizeForJSPass();
// Outlining currently relies on LLVM's SuffixTree, which we can't rely upon
// when building Binaryen for Emscripten.
#ifdef __EMSCRIPTEN__
#define SKIP_OUTLINING
#endif
#ifndef SKIP_OUTLINING
Pass* createOutliningPass();
#endif
Pass* createPickLoadSignsPass();
Pass* createModAsyncifyAlwaysOnlyUnwindPass();
Pass* createModAsyncifyNeverUnwindPass();
Pass* createLLVMNonTrappingFPToIntLoweringPass();
Pass* createPoppifyPass();
Pass* createPostEmscriptenPass();
Pass* createPrecomputePass();
Pass* createPrecomputePropagatePass();
Pass* createPrinterPass();
Pass* createPrintCallGraphPass();
Pass* createPrintFeaturesPass();
Pass* createPrintFunctionMapPass();
Pass* createPropagateGlobalsGloballyPass();
Pass* createRemoveNonJSOpsPass();
Pass* createRemoveImportsPass();
Pass* createRemoveMemoryInitPass();
Pass* createRemoveUnusedBrsPass();
Pass* createRemoveUnusedModuleElementsPass();
Pass* createRemoveUnusedNonFunctionModuleElementsPass();
Pass* createRemoveUnusedNamesPass();
Pass* createRemoveUnusedTypesPass();
Pass* createReorderFunctionsByNamePass();
Pass* createReorderFunctionsPass();
Pass* createReorderGlobalsPass();
Pass* createReorderGlobalsAlwaysPass();
Pass* createReorderLocalsPass();
Pass* createReReloopPass();
Pass* createRedundantSetEliminationPass();
Pass* createRoundTripPass();
Pass* createSafeHeapPass();
Pass* createSetGlobalsPass();
Pass* createSeparateDataSegmentsPass();
Pass* createSignaturePruningPass();
Pass* createSignatureRefiningPass();
Pass* createSignExtLoweringPass();
Pass* createSimplifyLocalsPass();
Pass* createSimplifyGlobalsPass();
Pass* createSimplifyGlobalsOptimizingPass();
Pass* createSimplifyLocalsNoNestingPass();
Pass* createSimplifyLocalsNoTeePass();
Pass* createSimplifyLocalsNoStructurePass();
Pass* createSimplifyLocalsNoTeeNoStructurePass();
Pass* createStackCheckPass();
Pass* createStringGatheringPass();
Pass* createStringLoweringPass();
Pass* createStringLoweringMagicImportPass();
Pass* createStringLoweringMagicImportAssertPass();
Pass* createStripDebugPass();
Pass* createStripDWARFPass();
Pass* createStripProducersPass();
Pass* createStripTargetFeaturesPass();
Pass* createSouperifyPass();
Pass* createSouperifySingleUsePass();
Pass* createSpillPointersPass();
Pass* createStripEHPass();
Pass* createStubUnsupportedJSOpsPass();
Pass* createSSAifyPass();
Pass* createSSAifyNoMergePass();
Pass* createTable64LoweringPass();
Pass* createTranslateToExnrefPass();
Pass* createTrapModeClamp();
Pass* createTrapModeJS();
Pass* createTupleOptimizationPass();
Pass* createTypeGeneralizingPass();
Pass* createTypeRefiningPass();
Pass* createTypeFinalizingPass();
Pass* createTypeMergingPass();
Pass* createTypeSSAPass();
Pass* createTypeUnFinalizingPass();
Pass* createUnsubtypingPass();
Pass* createUnteePass();
Pass* createVacuumPass();

Pass* createAddThrowsPass();

// Test passes:
Pass* createCatchPopFixupPass();

} // namespace wasm

#endif
