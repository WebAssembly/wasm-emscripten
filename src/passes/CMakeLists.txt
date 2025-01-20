file(READ wasm-intrinsics.wat WASM_INTRINSICS_WAT HEX)
set(CMAKE_CONFIGURE_DEPENDS wasm-intrinsics.wat)

string(REGEX MATCHALL "([A-Fa-f0-9][A-Fa-f0-9])" SEPARATED_HEX ${WASM_INTRINSICS_WAT})

foreach (hex IN LISTS SEPARATED_HEX)
  string(APPEND WASM_INTRINSICS_EMBED "0x${hex},")
endforeach ()
string(APPEND WASM_INTRINSICS_EMBED "0x00")

configure_file(WasmIntrinsics.cpp.in WasmIntrinsics.cpp @ONLY)

FILE(GLOB passes_HEADERS *.h)

set(passes_SOURCES
  param-utils.cpp
  pass.cpp
  test_passes.cpp
  AbstractTypeRefining.cpp
  AlignmentLowering.cpp
  Asyncify.cpp
  AvoidReinterprets.cpp
  CoalesceLocals.cpp
  CodePushing.cpp
  CodeFolding.cpp
  ConstantFieldPropagation.cpp
  ConstHoisting.cpp
  DataFlowOpts.cpp
  DeadArgumentElimination.cpp
  DeadCodeElimination.cpp
  DeAlign.cpp
  DebugLocationPropagation.cpp
  DeNaN.cpp
  Directize.cpp
  DuplicateImportElimination.cpp
  DuplicateFunctionElimination.cpp
  DWARF.cpp
  EncloseWorld.cpp
  ExtractFunction.cpp
  Flatten.cpp
  FuncCastEmulation.cpp
  GenerateDynCalls.cpp
  GlobalEffects.cpp
  GlobalRefining.cpp
  GlobalStructInference.cpp
  GlobalTypeOptimization.cpp
  GUFA.cpp
  hash-stringify-walker.cpp
  Outlining.cpp
  Heap2Local.cpp
  HeapStoreOptimization.cpp
  I64ToI32Lowering.cpp
  Inlining.cpp
  InstrumentLocals.cpp
  InstrumentMemory.cpp
  Intrinsics.cpp
  J2CLItableMerging.cpp
  J2CLOpts.cpp
  JSPI.cpp
  LegalizeJSInterface.cpp
  LimitSegments.cpp
  LLVMMemoryCopyFillLowering.cpp
  LocalCSE.cpp
  LocalSubtyping.cpp
  LogExecution.cpp
  LoopInvariantCodeMotion.cpp
  Memory64Lowering.cpp
  MemoryPacking.cpp
  MergeBlocks.cpp
  MergeSimilarFunctions.cpp
  MergeLocals.cpp
  Metrics.cpp
  MinifyImportsAndExports.cpp
  MinimizeRecGroups.cpp
  Monomorphize.cpp
  MultiMemoryLowering.cpp
  NameList.cpp
  NameTypes.cpp
  NoInline.cpp
  LLVMNontrappingFPToIntLowering.cpp
  OnceReduction.cpp
  OptimizeAddedConstants.cpp
  OptimizeCasts.cpp
  OptimizeInstructions.cpp
  OptimizeForJS.cpp
  PickLoadSigns.cpp
  Poppify.cpp
  PostEmscripten.cpp
  Precompute.cpp
  Print.cpp
  PrintCallGraph.cpp
  PrintFeatures.cpp
  PrintFunctionMap.cpp
  RoundTrip.cpp
  SetGlobals.cpp
  SignaturePruning.cpp
  SignatureRefining.cpp
  SignExtLowering.cpp
  StringLowering.cpp
  Strip.cpp
  StripTargetFeatures.cpp
  TraceCalls.cpp
  RedundantSetElimination.cpp
  RemoveImports.cpp
  RemoveMemoryInit.cpp
  RemoveNonJSOps.cpp
  RemoveUnusedBrs.cpp
  RemoveUnusedNames.cpp
  RemoveUnusedModuleElements.cpp
  RemoveUnusedTypes.cpp
  ReorderFunctions.cpp
  ReorderGlobals.cpp
  ReorderLocals.cpp
  ReReloop.cpp
  TrapMode.cpp
  TypeGeneralizing.cpp
  TypeRefining.cpp
  TypeMerging.cpp
  TypeSSA.cpp
  SafeHeap.cpp
  SeparateDataSegments.cpp
  SimplifyGlobals.cpp
  SimplifyLocals.cpp
  Souperify.cpp
  SpillPointers.cpp
  StackCheck.cpp
  StripEH.cpp
  SSAify.cpp
  TupleOptimization.cpp
  TranslateEH.cpp
  TypeFinalizing.cpp
  Unsubtyping.cpp
  Untee.cpp
  Vacuum.cpp
  AddThrows.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/WasmIntrinsics.cpp
  ${passes_HEADERS}
)
# The below condition is intended for removal once the suffix_tree and
# suffix_tree_node source files no longer depend on LLVM code in the
# third_party folder
if(EMSCRIPTEN)
  list(REMOVE_ITEM passes_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/stringify-walker.h)
  list(REMOVE_ITEM passes_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/stringify-walker-impl.h)
  list(REMOVE_ITEM passes_SOURCES "hash-stringify-walker.cpp")
  list(REMOVE_ITEM passes_SOURCES "Outlining.cpp")
endif()
add_library(passes OBJECT ${passes_SOURCES})
