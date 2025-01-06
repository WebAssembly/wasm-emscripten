Changelog
=========

This document describes changes between tagged Binaryen versions.

To browse or download snapshots of old tagged versions, visit
https://github.com/WebAssembly/binaryen/releases.

Not all changes are documented here. In particular, new features, user-oriented
fixes, options, command-line parameters, usage changes, deprecations,
significant internal modifications and optimizations etc. generally deserve a
mention. To examine the full set of changes between versions, visit the link to
full changeset diff at the end of each section.

Current Trunk
-------------

 - Binaryen.js now builds with BigInt support. The i64.const and f64.const_bits
   functions now take BigInt parameters rather than pairs of numbers.

v121
----

 - BinaryenSelect no longer takes a type parameter. (#7097)
 - AutoDrop APIs have been removed. (#7106)
 - bulk-memory-opt and call-indirect-overlong features are added for parity with
   LLVM. (#7139)
 - WasmGC optimizations now run significantly faster and scale better with
   available threads. (#7142)
 - Binaryen now supports parsing control flow structures with parameter types by
   lowering them away in the parsers. (#7149)

v120
----

 - Remove closed world validation checks. These checks were causing more harm
   than good. All valid code will now validate with `--closed-world` (but also
   it now provides fewer warnings to users that enable closed world on code
   which does not conform to the requirements of that mode, which can lead to
   changes in runtime behavior; for the long-term plans, see #6965). (#7019)
 - Many compile time speedups were implemented (2x overall improvement), see
   https://github.com/WebAssembly/binaryen/issues/4165#issuecomment-2372548271
 - Several `exnref` (newest version of Wasm EH) optimizations: #7013, #6996,
   #6997, #6983, #6980
 - Source Maps: Support 5 segment mappings. (#6795)
 - [wasm-split] Add a multi-split mode. (#6943)
 - Add a `--preserve-type-order` option that minimizes text format changes in
   type ordering. (#6916)
 - Add a J2CL specific pass that moves itable entries to vtables. (#6888)

v119
----

 - Passes can now receive individual pass arguments, that is --foo=A --foo=B for
   a pass foo will run the pass twice (which was possible before) and will now
   run it first with argument A and second with B. --pass-arg=foo@BAR will now
   apply to the most recent --foo pass on the commandline, if foo is a pass
   (while global pass arguments - that are not the name of a pass - remain, as
   before, global for all passes). (#6687)
 - The Metrics pass now takes an optional argument to use as the title,
   `--metrics=text` will show that text before the metrics. Each instance of
   Metrics can have unique text, `--metrics=before -O3 --metrics=after`. (#6792)
 - Add C and JS APIs to control more pass options (trapsNeverHappen,
   closedWorld, generateStackIR, optimizeStackIR, and the list of skipped
   passes). (#6713)
 - A C APIs for getting/setting the type of Functions (#6721).
 - Allow using `--skip-pass` on the commandline multiple times (#6714).
 - The instructions relaxed_fma and relaxed_fnma have been renamed to
   relaxed_madd and relaxed_nmadd.
 - Add a new `--heap-store-optimization` pass. (#6882)
 - Add a pass for minimizing recursion groups. (#6832)


v118
----

 - StackIR is now handled entirely during binary writing (#6568). This is
   mostly not noticeable, except that:
   - Text output no longer notes `(; has Stack IR ;)` (as Stack IR only exists
     during binary writing).
   - `--generate-stack-ir`, `--optimize-stack-ir`, and `--print-stack-ir` are
     now flags and not passes. That means the order of operations may seem
     different, as they apply during binary writing (or, if no binary is written
     but we were still asked to print StackIR, `wasm-opt` does it at the very
     end).
   - Whether to generate, optimize, and print StackIR is now noted as part of
     the PassOptions. As a result `BinaryenModulePrintStackIR` and similar APIs
     do not receive an `optimize` flag, as they read the PassOption
     `optimizeStackIR` instead.
 - The new, standards-compliant text parser is now the default.
 - Source map comments on `else` branches must now be placed above the
   instruction inside the `else` branch rather than on the `else` branch itself.
 - Source map locations from instructions are no longer automatically propagated
   to function epilogues.
 - Add a new `BinaryenModuleReadWithFeatures` function to the C API that allows
   to configure which features to enable in the parser. (#6380)
 - The build-time option to use legacy WasmGC opcodes is removed. (#5874)
 - The strings in `string.const` instructions must now be valid WTF-8.
 - The `TraverseCalls` flag for `ExpressionRunner` is removed.
 - C API: Support adding data segments individually (#6346)
 - Add sourcemap support to wasm-metadce and wasm-merge (#6372).
 - Fix semantics of return calls (#6448, #6451, #6464, #6470, #6474).
 - Add table64 lowering pass (#6595).
 - Add TraceCalls instrumentation pass (#6619).

v117
----

 - Add a WebAssembly build to release (#6351)
 - Add Linux aarch64 build to release (#6334).
 - The text format for tuple instructions now requires immediates. For example,
   to make a tuple of two elements, use `tuple.make 2` (#6169) (#6172) (#6170).
 - The text format for `if` expressions now requires `then` and `else` to
   introduce the two branch arms, matching the spec (#6201).
 - Fuzzer: Remove --emit-js-shell logic and reuse fuzz_shell.js instead (#6310).
 - [EH] Add --experimental-new-eh option to wasm-opt (#6270) (#6210).
 - Add StringLowering pass, from stringref to imported-strings (#6271).
 - C API: Add BinaryenFunctionAppendVar (#6213).
 - Add J2CL optimization pass (#6151).
 - Add no-inline IR annotation, and passes to set it based on function name
   (#6146).
 - C API: Add BinaryenTableGetType and BinaryenTableSetType (#6137).
 - Add an Unsubtyping optimization (#5982).
 - Compute full transitive closure in GlobalEffects (#5992).
 - Add passes to finalize or unfinalize types (#5944).
 - Add a tuple optimization pass (#5937).

v116
----

 - "I31New" changed to "RefI31" everywhere it appears in the C API and similarly
   "i31.new" has been replaced with "ref.i31" in the JS API and in printed
   output (#5930, #3931).
 - The standard WasmGC opcodes are now on by default (#5873).

v115
----

- The nonstandard, experimental gc-nn-locals feature has been removed now that
  standard non-nullable locals are supported. (#5080)
- Print all debug annotations when `BINARYEN_PRINT_FULL`. (#5904)
- Use the standard text format for WasmGC now that things are stable there.
  (#5894, #5889, #5852, #5801)
- Rename multimemory flag to `--multimemory` to match its naming in LLVM.
  (#5890)
- Allow opting into standard GC encodings at compile time. (#5868)
- Remove legacy WasmGC instructions. (#5861)
- GUFA: Infer using TrapsNeverHappen. (#5850)
- Add a pass to reorder functions by name, which can be useful for debugging
  (e.g. comparisons after optimizations), `--reorder-functions-by-name`. (#5811)
- C API: Add `BinaryenAddFunctionWithHeapType` which is like
  `BinaryenAddFunction` but takes a heap type. The old function is kept for
  backwards compatibility and as a convenience. (#5829)
- Work on new optimization framework using lattices. (#5784, #5794, #5800,
  #5817, #5831, #5849)
- Heap types are now final by default and openness must be opted into both in
  the text and binary formats as well as in the TypeBuilder API. (#5918)

v114
----

- Fix a bug where e.g. -O3 -Oz ran the -O3 with the opt levels of -Oz, which
  could inhibit inlining, for example. While this is a bugfix, it affects how
  commandline options are interpreted, so if you depended on the old behavior
  this may be a breaking change. That is, the old behavior made -O3 -Oz run the
  first -O3 with -Oz's opt levels, and the new behavior is to run -O3 with the
  proper (-O3) opt levels. This is a followup to #5333 from a previous release.
  (#5787)
- Add pass to remove Exception Handling instructions (#5770).


v113
----

- Add a `wasm-merge` tool. This is a full rewrite of the previous `wasm-merge`
  tool that was removed from the tree in the past. The new version is much
  simpler after recent improvements to multi-memory and multi-table. The
  rewrite was motivated by new use cases for merging modules in the context of
  WasmGC.
- Some C and JS API functions now refer to data and element segments by name
  instead of index.
- The `--nominal` and `--hybrid` command line options and related API functions
  have been removed. The only supported type system is now the standard
  isorecursive (i.e. hybrid) type system. (#5672)
- Add a "mayNotReturn" effect (#5711).
- Disable the memory64 feature in Memory64Lowering.cpp (#5679).
- Disable sign extension in SignExtLowering.cpp (#5676).

v112
----

- Add AbstractTypeRefining pass (#5461)
- Add a mechanism to skip a pass by name (#5448)
- Add TypeMerging pass (#5321)
- Add TypeSSA pass  (#5299)
- Optimization sequences like `-O3 -Os` now do the expected thing and run `-O3`
  followed by `-Os`. Previously the last of them set the defaults that were used
  by all executions, so `-O3 -Os` was equivalent to `-Os -Os`. (There is no
  change to the default optimization level that other passes can see. For
  example, `--precompute-propagate -O2 -O1` will run `--precompute-propagate`
  at opt level `1`, as the global default is set to `2` and then overridden to
  `1`. The only change is that the passes run by `-O2` will actually run `-O2`
  now, while before they'd use the global default which made them do `-O1`.)
- Add `--closed-world` flag. This enables more optimizations in GC mode as it
  lets us assume that we can change types inside the module.
- The isorecursive WasmGC type system (i.e. --hybrid) is now the default to
  match the spec and the old default equirecursive (i.e. --structural) system
  has been removed.
- `ref.is_func`, `ref.is_data`, and `ref.is_i31` have been removed from the C
  and JS APIs and `RefIs` has been replaced with `RefIsNull`.
- Types `Data` and `Dataref` have been replaced with types `Struct` and
  `Structref` in the C and JS APIs.
* `BinaryenStringNew` now takes an additional last argument, `try_`, indicating
  whether the instruction is one of `string.new_utf8_try` respectively
  `string.new_utf8_array_try`.
* `BinaryenStringEq` now takes an additional second argument, `op`, that is
  either `BinaryenStringEqEqual()` if the instruction is `string.eq` or
  `BinaryenStringEqCompare()` if the instruction is `string.compare`.

v111
----

- Add extra `memory64` argument for `BinaryenSetMemory` and new
  `BinaryenMemoryIs64` C-API method to determine 64-bit memory. (#4963)
- `TypeBuilderSetSubType` now takes a supertype as the second argument.
- `call_ref` now takes a mandatory signature type immediate.
- If `THROW_ON_FATAL` is defined at compile-time, then fatal errors will throw a
  `std::runtime_error` instead of terminating the process. This may be used by
  embedders of Binaryen to recover from errors.
- Implemented bottom heap types: `none`, `nofunc`, and `noextern`. RefNull
  expressions and null `Literal`s must now have type `nullref`, `nullfuncref`,
  or `nullexternref`.
- The C-API's `BinaryenTypeI31ref` and `BinaryenTypeDataref` now return nullable
  types.
- The `sign-extension` and `mutable-globals` features are now both enabled by
  default in all tools. This is in order to match llvm's defaults (See
  https://reviews.llvm.org/D125728).
- Add a pass to lower sign-extension operations to MVP.

v110
----

- Add support for non-nullable locals in wasm GC. (#4959)
- Add support for multiple memories. (#4811)
- Add support for the wasm Strings proposal. (see PRs with [Strings] in name)
- Add a new flag to Directize, `--pass-arg=directize-initial-contents-immutable`
  which indicates the initial table contents are immutable. That is the case for
  LLVM, for example, and it allows us to optimize more indirect calls to direct
  ones. (#4942)
- Change constant values of some reference types in the C and JS APIs. This is
  only observable if you hardcode specific values instead of calling the
  relevant methods (like `BinaryenTypeDataref()`). (#4755)
- `BinaryenModulePrintStackIR`, `BinaryenModuleWriteStackIR` and
  `BinaryenModuleAllocateAndWriteStackIR` now have an extra boolean
  argument `optimize`. (#4832)
- Remove support for the `let` instruction that has been removed from the typed
  function references spec.
- HeapType::ext has been restored but is no longer a subtype of HeapType::any to
  match the latest updates in the GC spec. (#4898)
- `i31ref` and `dataref` are now nullable to match the latest GC spec. (#4843)
- Add support for `extern.externalize` and `extern.internalize`. (#4975)

v109
----

- Add Global Struct Inference pass (#4659) (#4714)
- Restore and fix SpillPointers pass (#4570)
- Update relaxed SIMD instructions to latest spec

v108
----

- Add CMake flag BUILD_TOOLS to control building tools (#4655)
- Add CMake flag JS_OF_OCAML for js_of_ocaml (#4637)
- Remove externref (#4633)

v107
----

- Update the wasm GC type section binary format (#4625, #4631)
- Lift the restriction in liveness-traversal.h on max 65535 locals (#4567)
- Switch to nominal fuzzing by default (#4610)
- Refactor Feature::All to match FeatureSet.setAll() (#4557)
- New Signature Pruning pass (#4545)
- Add support for extended-const proposal (#4529)
- Add BUILD_TESTS CMake option to make gtest dependency optional.
- Updated tests to use filecheck 0.0.22 (#4537). Updating is required to
  successfully run the lit tests. This can be done with
  `pip3 install -r requirements-dev.txt`.

v106
----

- [wasm2js] Support exports of Globals (#4523)
- MergeSimilarFunctions optimization pass (#4414)
- Various wasm-ctor-eval improvements, including support for GC.

v105
----

- This release contains binaries for ARM64 MacOS devices (#4397)
- Otherwise, mostly bug fixes and incremental optimization improvements.

v104
----

- Bugfixes only, release created due to incorrect github release artifacts in
  v103 release (#4398).

v103
----

- The EffectAnalyzer now takes advantage of immutability of globals. To achieve
  that it must have access to the module. That is already the case in the C++
  API, but the JS API allowed one to optionally not add a module when calling
  `getSideEffects()`. It is now mandatory to pass in the module.
- JS and Wasm builds now emit ECMAScript modules. New usage is:
  ```js
  import Binaryen from "path/to/binaryen.js";
  const binaryen = await Binaryen();
  ...
  ```
- CallIndirect changed from storing a Signature to storing a HeapType

v102
----

- Add `BinaryenUpdateMaps` to the C API.

- Adds a TrapsNeverHappen mode (#4059). This has many of the benefits of
  IgnoreImplicitTraps, but can be used safely in more cases. IgnoreImplicitTraps
  is now deprecated.

- Adds type argument for BinaryenAddTable method (#4107). For the binaryen.js api
  this parameter is optional and by default is set to funcref type.

- Replace `BinaryenExpressionGetSideEffects`'s features parameter with a module
  parameter.

- OptimizeInstructions now lifts identical code in `select`/`if` arms (#3828). This may cause direct `BinaryenTupleExtract(BinaryenTupleMake(...))` to [use multivalue types](https://github.com/grain-lang/grain/pull/1158).

v101
----

- `BinaryenSetFunctionTable` and `module.setFunctionTable` have been removed
  in favor of `BinaryenAddTable` and `module.addTable` respectively.
- `BinaryenIsFunctionTableImported` is removed.
- A new type `BinaryenElementSegmentRef` has been added to the C API with
  new apis in both C & JS:
  - `BinaryenAddActiveElementSegment`
  - `BinaryenAddPassiveElementSegment`
  - `BinaryenRemoveElementSegment`
  - `BinaryenGetElementSegment`
  - `BinaryenGetElementSegmentByIndex`
  - `BinaryenElementSegmentGetName`
  - `BinaryenElementSegmentSetName`
  - `BinaryenElementSegmentGetTable`
  - `BinaryenElementSegmentSetTable`
  - `BinayenElementSegmentIsPassive`
  - `module.addActiveElementSegment`
  - `module.addPassiveElementSegment`
  - `module.removeElementSegment`
  - `module.getElementSegment`
  - `module.getElementSegmentByIndex`
  - `module.getTableSegments`
  - `module.getNumElementSegments`
  - `binaryen.getElementSegmentInfo`
- `BinaryenAddTable` and `module.addTable` no longer take offset and function
    names.
- `BinaryenGetNumFunctionTableSegments` is replaced with
  `BinaryenGetNumElementSegments`.
- `BinaryenGetFunctionTableSegmentOffset` is replaced with
  `BinaryenElementSegmentGetOffset`.
- `BinaryenGetFunctionTableSegmentLength` is replaced with
  `BinaryenElementSegmentGetLength`.
- `BinaryenGetFunctionTableSegmentData` is replaced with
  `BinaryenElementSegmentGetData`.
- Boolean values in the C API now should use `bool` instead of `int`.
- Experimental SIMD instructions have been removed and the names and opcodes of
  the standard instructions have been updated to match the final spec.

v100
----

- `wasm-dis` now supports options to enable or disable Wasm features.
- Reference types support has been improved by allowing multiple tables in a
  module.
- `call_indirect` and `return_call_indirect` now take an additional table name
  parameter. This is necessary for reference types support.
- New getter/setter methods have been introduced for `call_indirect` table name:
  - `BinaryenCallIndirectGetTable`
  - `BinaryenCallIndirectSetTable`
  - JS API `CallIndirect.table`
- New APIs have been added to add and manipulate multiple tables in a module:
  - `BinaryenAddTable`
  - `BinaryenRemoveTable`
  - `BinaryenGetNumTables`
  - `BinaryenGetTable`
  - `BinaryenGetTableByIndex`
  - `BinaryenTableGetName`
  - `BinaryenTableGetInitial`
  - `BinaryenTableHasMax`
  - `BinaryenTableGetMax`
  - `BinaryenTableImportGetModule`
  - `BinaryenTableImportGetBase`
  - `module.addTable`
  - `module.removeTable`
  - `module.getTable`
  - `module.getTableByIndex`
  - `module.getNumTables`
  - `binaryen.getTableInfo`

v99
---

- Fix optimization behavior on assuming memory is zero-filled. We made that
  assumption before, but it is incorrect in general, which caused problems.
  The fixed behavior is to not assume it, but require the user to pass it in as
  a flag, `--zero-filled-memory`. Large binaries with lots of empty bytes in the
  data section may regress without that flag. Toolchains like Emscripten can
  pass the flag automatically for users if they know it is right to assume,
  which can avoid any regressions. (#3306)
- `RefFunc` C and JS API constructors (`BinaryenRefFunc` and `ref.func`
  respectively) now take an extra `type` parameter, similar to `RefNull`. This
  is necessary for typed function references support.
- JS API functions for atomic notify/wait instructions are renamed.
  - `module.atomic.notify` -> `module.memory.atomic.notify`
  - `module.i32.atomic.wait` -> `module.memory.atomic.wait32`
  - `module.i64.atomic.wait` -> `module.memory.atomic.wait64`
- Remove old/broken SpollPointers pass.  This pass: Spills values that might be
  pointers to the C stack. This allows Boehm-style GC to see them properly.
  This can be revived if needed from git history (#3261).
- Make `NUM_PARAMS` in `FuncCastEmulation` a runtime configuration option named
  `max-func-params`. This defaults to the original value of 16.
- `BinaryenGetFunction`, `BinaryenGetGlobal` and `BinaryenGetEvent` now return
  `NULL` instead of aborting when the respective element does not yet exist.

v98
---

- Add `--fast-math` mode. (#3155)
- Initial implementation of "Memory64" proposal (#3130)
- Lots of changes in support of GC proposal

v97
---

- Remove asm2wasm, which supported Emscripten's fastcomp backend, after fastcomp
  was removed.
- The new feature flag `--enable-anyref` enables just the `anyref` type incl.
  basic subtyping of `externref`, `funcref` and `exnref` (if enabled).
- Enabling the exception handling or anyref features without also enabling
  reference types is a validation error now.
- The `Host` expression and its respective APIs have been refactored into
  separate `MemorySize` and `MemoryGrow` expressions to align with other memory
  instructions.

v96
---

- Fuzzing: Compare wasm2js to the interpreter (#3026)
- Fix CountLeadingZeroes on MSVC, which lead to bad optimizations (#3028)
- Asyncify verbose option (#3022)
- wasm2js: Add an "Export" scope for name resolution, avoids annoying
  warnings (#2998)
- Extend the C- and JS-APIs (#2586)

v95
---

- Add Asyncify "add list" that adds to the list of functions to be instrumented.
  Rename old lists to be clearer and more consistent with that, so now there is
  "remove list" to remove, "add list" to add, and "only list" which if set means
  that only those functions should be instrumented and nothing else.
- Renamed various ambiguous C-API functions for consistency:
  - `BinaryenBlockGetChild` to `BinaryenBlockGetChildAt`
  - `BinaryenSwitchGetName` to `BinaryenSwitchGetNameAt`
  - `BinaryenCallGetOperand` to `BinaryenCallGetOperandAt`
  - `BinaryenCallIndirectGetOperand` to `BinaryenCallIndirectGetOperandAt`
  - `BinaryenHostGetOperand` to `BinaryenHostGetOperandAt`
  - `BinaryenThrowGetOperand` to `BinaryenThrowGetOperandAt`
  - `BinaryenTupleMakeGetOperand` to `BinaryenTupleMakeGetOperandAt`

v94
---

- The C-API's `BinaryenSetAPITracing` and the JS-API's `setAPITracing` have been
  removed because this feature was not very useful anymore and had a significant
  maintainance cost.
- wasm-emscripten-finalize will no longer generate `stackSave`, `stackAlloc`,
  `stackRestore` function.  It not expects them to be included in the input
  file.

v93
---

- First release with binaries built with github actions.


v92
---

- The `multivalue` feature has been added. It allows functions and control flow
  structures to return tuples and for locals and globals to have tuple types.
  Tuples are created with the new `tuple.make` pseudoinstruction and their
  elements are retrieved with the new `tuple.extract` pseudoinstruction.
- The internal type interner has been rewritten to avoid taking locks in far
  more situations. Depending on the workload, this may result in large speedups
  and increased parallelism.
- Represent the `dylink` section in Binaryen IR, so we can read, write, and
  update it.

v91
---

- `BinaryenExpressionGetSideEffects` (C API) and `getSideEffects` (JS API) now
  takes an additional `features` parameter.
- Reference type support is added. Supported instructions are `ref.null`,
  `ref.is_null`, `ref.func`, and typed `select`. Table instructions are not
  supported yet. For typed `select`, C/JS API can take an additional 'type'
  parameter.

v90
---

- `local.tee`'s C/Binaryen.js API now takes an additional type parameter for its
  local type, like `local.get`. This is required to handle subtypes.
- Added load_splat SIMD instructions
- Binaryen.js instruction API changes:
  - `notify` -> `atomic.notify`
  - `i32.wait` / `i64.wait` -> `i32.atomic.wait` / `i64.atomic.wait`
- Binaryen.js: `flags` argument in `setMemory` function is removed.
- `atomic.fence` instruction support is added.
- wasm-emscripten-finalize: Don't rely on name section being present in the
  input. Use the exported names for things instead.
- Added `mutable` parameter to BinaryenAddGlobalImport.
- Replace BinaryenSIMDBitselect* with BinaryenSIMDTernary* in the C API and add
  qfma/qfms instructions.
- Added `offset` parameter to BinaryenSetFunctionTable.
- Add the ability to create multivalue Types in the C and JS APIs.
- Remove named function types. They are replaced by `params` and `results` types
  local to each function.
- Binaryen.js can now be compiled to Wasm using the `binaryen_wasm` target.
  Unlike the JS variant, the Wasm variant requires asynchronously awaiting the
  Wasm blob's instantiation and initialization before being usable, using the
  `binaryen.ready` promise, e.g. `binaryen.ready.then(() => ...)`.
- Binaryen.js now uses `binaryen` (was `Binaryen`) as its global name to align
  with the npm package.
- Binaryen.js: The result of `getMemorySegmentInfoByIndex` now has the same
  structure as the respective inputs on creation (`byteOffset` -> `offset`).

v88
---

- wasm-emscripten-finalize: For -pie binaries that import a mutable stack
  pointer we internalize this an import it as immutable.
- The `tail-call` feature including the `return_call` and `return_call_indirect`
  instructions is ready to use.

v87
---

- Rename Bysyncify => Asyncify

v86
---

- The --initial-stack-pointer argument to wasm-emscripten-finalize no longer
  has any effect.  It will be removed completely in future release.

v85
---

- Wast file parsing rules now don't allow a few invalid formats for typeuses
  that were previously allowed. Typeuse entries should follow this format,
  meaning they should have (type) -> (param) -> (result) order if more than one
  of them exist.
  ```
  typeuse ::= (type index|name)+ |
              (type index|name)+ (param ..)* (result ..)* |
              (param ..)* (result ..)*
  ```
  Also, all (local) nodes in function definition should be after all typeuse
  elements.
- Removed APIs related to deprecated instruction names in Binaryen.js:
  - `get_local` / `getLocal`
  - `set_local` / `setLocal`
  - `tee_local` / `teeLocal`
  - `get_global` / `getGlobal`
  - `set_global` / `setGlobal`
  - `current_memory` / `currentMemory`
  - `grow_memory` / `growMemory`
  They are now available as their new instruction names:
  `local.get`, `local.set`, `local.tee`, `global.get`, `global.set`,
  `memory.size`, and `memory.grow`.
- Add feature handling to the C/JS API with no feature enabled by default.

v84
---

- Generate dynCall thunks for any signatures used in "invoke" calls.

v81
---

- Fix AsmConstWalker handling of string address in arg0 with -fPIC code

v80
---

- Change default feature set in the absence of a target features section from
  all features to MVP.

v79
---

- Improve support for side modules

v78
---

- Add `namedGlobals` to metadata output of wasm-emscripten-finalize
- Add support for llvm PIC code.
- Add --side-module option to wasm-emscripten-finalize.
- Add `segmentPassive` argument to `BinaryenSetMemory` for marking segments
  passive.
- Make `-o -` print to stdout instead of a file named "-".

v73
---

- Remove wasm-merge tool.

v73
---

- Remove jsCall generation from wasm-emscripten-finalize.  This is not needed
  as of https://github.com/emscripten-core/emscripten/pull/8255.

v55
---

- `RelooperCreate` in the C API now has a Module parameter, and
  `RelooperRenderAndDispose` does not.
  - The JS API now has the `Relooper` constructor receive the `Module`.
- Relooper: Condition properties on Branches must not have side effects.

older
-----

- `BinaryenSetFunctionTable` in the C API no longer accepts an array of
  functions, instead it accepts an array of function names, `const char**
  funcNames`. Previously, you could not include imported functions because they
  are of type `BinaryenImportRef` instead of `BinaryenFunctionRef`. #1650

- `BinaryenSetFunctionTable` in the C API now expects the initial and maximum
  table size as additional parameters, like `BinaryenSetMemory` does for pages,
  so tables can be grown dynamically. #1687

- Add `shared` parameters to `BinaryenAddMemoryImport` and `BinaryenSetMemory`,
  to support a shared memory. #1686
