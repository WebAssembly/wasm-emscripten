;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --stack-check --enable-memory64 -S -o - | filecheck %s

(module
 (memory i64 (data))
 ;; CHECK:      (type $none_=>_i64 (func (result i64)))

 ;; CHECK:      (type $i64_i64_=>_none (func (param i64 i64)))

 ;; CHECK:      (global $sp (mut i64) (i64.const 0))
 (global $sp (mut i64) (i64.const 0))
 (func "use_stack" (result i64)
  (global.set $sp (i64.const 42))
  (global.get $sp)
 )
)
;; CHECK:      (global $__stack_base (mut i64) (i64.const 0))

;; CHECK:      (global $__stack_limit (mut i64) (i64.const 0))

;; CHECK:      (memory $0 i64 0 65536)

;; CHECK:      (data (i64.const 0) "")

;; CHECK:      (export "use_stack" (func $0))

;; CHECK:      (export "__set_stack_limits" (func $__set_stack_limits))

;; CHECK:      (func $0 (result i64)
;; CHECK-NEXT:  (local $0 i64)
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (if
;; CHECK-NEXT:    (i32.or
;; CHECK-NEXT:     (i64.gt_u
;; CHECK-NEXT:      (local.tee $0
;; CHECK-NEXT:       (i64.const 42)
;; CHECK-NEXT:      )
;; CHECK-NEXT:      (global.get $__stack_base)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (i64.lt_u
;; CHECK-NEXT:      (local.get $0)
;; CHECK-NEXT:      (global.get $__stack_limit)
;; CHECK-NEXT:     )
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (unreachable)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (global.set $sp
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (global.get $sp)
;; CHECK-NEXT: )

;; CHECK:      (func $__set_stack_limits (param $0 i64) (param $1 i64)
;; CHECK-NEXT:  (global.set $__stack_base
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (global.set $__stack_limit
;; CHECK-NEXT:   (local.get $1)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
(module
 ;; if the global names are taken we should not crash
 (memory i64 (data))
 ;; CHECK:      (type $i64_i64_=>_none (func (param i64 i64)))

 ;; CHECK:      (global $sp (mut i64) (i64.const 0))
 (global $sp (mut i64) (i64.const 0)))
 ;; CHECK:      (global $__stack_base (mut i64) (i64.const 0))
 (global $__stack_base (mut i64) (i64.const 0))
 ;; CHECK:      (global $__stack_limit (mut i64) (i64.const 0))
 (global $__stack_limit (mut i64) (i64.const 0))
 (export "use_stack" (func $0))
 (func $0 (result i64)
  (unreachable)
 )
)
;; CHECK:      (memory $0 i64 0 65536)

;; CHECK:      (data (i64.const 0) "")

;; CHECK:      (export "__set_stack_limits" (func $__set_stack_limits))

;; CHECK:      (func $__set_stack_limits (param $0 i64) (param $1 i64)
;; CHECK-NEXT:  (global.set $__stack_base
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (global.set $__stack_limit
;; CHECK-NEXT:   (local.get $1)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
