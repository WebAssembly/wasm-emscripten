;; Test running a wasm file in fuzz_shell.js.

(module
  (func $test (export "test") (result i32)
    (i32.const 42)
  )
)

;; Build to a binary wasm.
;;
;; RUN: wasm-opt %s -o %t.wasm -q

;; Run in d8.
;;
;; RUN: v8 %S/../../../scripts/fuzz_shell.js -- %t.wasm | filecheck %s
;;
;; CHECK: [fuzz-exec] calling test
;; CHECK: [fuzz-exec] note result: test => 42

