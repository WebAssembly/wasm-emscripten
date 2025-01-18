;; Test that appending a run operation with a seed can lead to a different
;; order of export calls.

(module
  (import "fuzzing-support" "log-i32" (func $log (param i32)))

  (func $a (export "a") (result i32)
    (i32.const 10)
  )

  (func $b (export "b") (result i32)
    (i32.const 20)
  )

  (func $c (export "c") (result i32)
    (i32.const 30)
  )
)

;; Run normally: we should see a,b,c called in order.
;;
;; RUN: wasm-opt %s -o %t.wasm -q
;; RUN: node %S/../../../scripts/fuzz_shell.js %t.wasm | filecheck %s
;;
;; CHECK: [fuzz-exec] calling a
;; CHECK: [fuzz-exec] note result: a => 10
;; CHECK: [fuzz-exec] calling b
;; CHECK: [fuzz-exec] note result: b => 20
;; CHECK: [fuzz-exec] calling c
;; CHECK: [fuzz-exec] note result: c => 30

;; Append another run with a seed that leads to a different order
;;
;; RUN: cp %S/../../../scripts/fuzz_shell.js %t.js
;; RUN: echo "callExports(34);" >> %t.js
;; RUN: node %t.js %t.wasm | filecheck %s --check-prefix=APPENDED
;;
;; The original order: a,b,c
;; APPENDED: [fuzz-exec] calling a
;; APPENDED: [fuzz-exec] note result: a => 10
;; APPENDED: [fuzz-exec] calling b
;; APPENDED: [fuzz-exec] note result: b => 20
;; APPENDED: [fuzz-exec] calling c
;; APPENDED: [fuzz-exec] note result: c => 30

;; A new order: b,c,a
;; APPENDED: [fuzz-exec] calling b
;; APPENDED: [fuzz-exec] note result: b => 20
;; APPENDED: [fuzz-exec] calling c
;; APPENDED: [fuzz-exec] note result: c => 30
;; APPENDED: [fuzz-exec] calling a
;; APPENDED: [fuzz-exec] note result: a => 10

