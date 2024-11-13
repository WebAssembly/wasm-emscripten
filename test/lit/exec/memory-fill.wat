;; NOTE: Assertions have been generated by update_lit_checks.py --output=fuzz-exec and should not be edited.

;; RUN: wasm-opt %s --enable-bulk-memory --memory-copy-fill-lowering --fuzz-exec -q -o /dev/null 2>&1 | filecheck %s

;; Tests derived from bulk-memory.wast spec tests

;; memory.fill
(module
  (import "fuzzing-support" "log-i32" (func $log-i32 (param i32)))
  (memory 1)


  (func $assert_load (param i32 i32)
    (if (i32.ne (local.get 1) (i32.load8_u (local.get 0)))
    (then (unreachable)))
  )

  (func $print_memory (param i32 i32)
    (loop $loop
      (call $log-i32 (local.get 0))
      (call $log-i32 (i32.load8_u (local.get 0)))
      (local.set 0 (i32.add (local.get 0) (i32.const 1)))
      (br_if $loop (i32.ne (local.get 0) (local.get 1)))
    )
  )
  ;; basic fill test
  ;; CHECK:      [fuzz-exec] calling test1
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 2]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 3]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 4]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 255]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 2]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 255]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 3]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 255]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 4]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  (func $test1 (export "test1")
    (call $print_memory (i32.const 0) (i32.const 5))
    (memory.fill (i32.const 1) (i32.const 0xff) (i32.const 3))
    (call $print_memory (i32.const 0) (i32.const 5))
    (call $assert_load (i32.const 0) (i32.const 0))
    (call $assert_load (i32.const 1) (i32.const 0xff))
    (call $assert_load (i32.const 2) (i32.const 0xff))
    (call $assert_load (i32.const 3) (i32.const 0xff))
    (call $assert_load (i32.const 4) (i32.const 0x0))
  )
  ;; Fill value is stored as a byte
  ;; CHECK:      [fuzz-exec] calling test2
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 170]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 170]
  (func $test2 (export "test2")
    (memory.fill (i32.const 0) (i32.const 0xbbaa) (i32.const 2))
    (call $print_memory (i32.const 0) (i32.const 2))
    (call $assert_load (i32.const 0) (i32.const 0xaa))
    (call $assert_load (i32.const 1) (i32.const 0xaa))
  )
  ;; Fill all of memory
  ;; CHECK:      [fuzz-exec] calling test3
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 2]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 3]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 4]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 5]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  (func $test3 (export "test3")
    (memory.fill (i32.const 0) (i32.const 0) (i32.const 0x10000))
    ;; let's not print all of memory; just beyond what we filled before
    (call $print_memory (i32.const 0) (i32.const 6))
  )
  ;; Succeed when writing 0 bytes at the end of the region
  ;; CHECK:      [fuzz-exec] calling test4
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 2]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 3]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 4]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 5]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 6]
  ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
  (func $test4 (export "test4")
    (memory.fill (i32.const 0x10000) (i32.const 0) (i32.const 0))
    ;; let's not print all of memory; just beyond what we filled before
    (call $print_memory (i32.const 0) (i32.const 7))
  )
  ;; Writing 0 bytes outside of memory is not allowed
  ;; CHECK:      [fuzz-exec] calling test5
  ;; CHECK-NEXT: [trap out of bounds memory access in memory.fill]
  (func $test5 (export "test5")
    (memory.fill (i32.const 0x10001) (i32.const 0) (i32.const 0))
    ;; should not be reached
    (call $print_memory (i32.const 0) (i32.const 6))
  )
  ;; again we do not test negative/overflowing addresses as the spec test does.
)
;; CHECK:      [fuzz-exec] calling test1
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 2]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 3]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 4]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]
;; CHECK-NEXT: [LoggingExternalInterface logging 255]
;; CHECK-NEXT: [LoggingExternalInterface logging 2]
;; CHECK-NEXT: [LoggingExternalInterface logging 255]
;; CHECK-NEXT: [LoggingExternalInterface logging 3]
;; CHECK-NEXT: [LoggingExternalInterface logging 255]
;; CHECK-NEXT: [LoggingExternalInterface logging 4]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]

;; CHECK:      [fuzz-exec] calling test2
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 170]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]
;; CHECK-NEXT: [LoggingExternalInterface logging 170]

;; CHECK:      [fuzz-exec] calling test3
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 2]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 3]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 4]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 5]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]

;; CHECK:      [fuzz-exec] calling test4
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 2]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 3]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 4]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 5]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 6]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]

;; CHECK:      [fuzz-exec] calling test5
;; CHECK-NEXT: [trap unreachable]
;; CHECK-NEXT: [fuzz-exec] comparing test1
;; CHECK-NEXT: [fuzz-exec] comparing test2
;; CHECK-NEXT: [fuzz-exec] comparing test3
;; CHECK-NEXT: [fuzz-exec] comparing test4
;; CHECK-NEXT: [fuzz-exec] comparing test5
