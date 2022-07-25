;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_test.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --enable-mutable-globals --asyncify --pass-arg=asyncify-relocatable -S -o - | filecheck %s

(module
)
;; CHECK:      (type $i32_=>_none (func (param i32)))

;; CHECK:      (type $none_=>_none (func))

;; CHECK:      (type $none_=>_i32 (func (result i32)))

;; CHECK:      (import "env" "__asyncify_state" (global $__asyncify_state (mut i32)))

;; CHECK:      (import "env" "__asyncify_data" (global $__asyncify_data (mut i32)))

;; CHECK:      (memory $0 1 1)

;; CHECK:      (export "asyncify_start_unwind" (func $asyncify_start_unwind))

;; CHECK:      (export "asyncify_stop_unwind" (func $asyncify_stop_unwind))

;; CHECK:      (export "asyncify_start_rewind" (func $asyncify_start_rewind))

;; CHECK:      (export "asyncify_stop_rewind" (func $asyncify_stop_rewind))

;; CHECK:      (export "asyncify_get_state" (func $asyncify_get_state))

;; CHECK:      (func $asyncify_start_unwind (param $0 i32)
;; CHECK-NEXT:  (global.set $__asyncify_state
;; CHECK-NEXT:   (i32.const 1)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (global.set $__asyncify_data
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (if
;; CHECK-NEXT:   (i32.gt_u
;; CHECK-NEXT:    (i32.load $0
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (i32.load $0 offset=4
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (unreachable)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $asyncify_stop_unwind
;; CHECK-NEXT:  (global.set $__asyncify_state
;; CHECK-NEXT:   (i32.const 0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (if
;; CHECK-NEXT:   (i32.gt_u
;; CHECK-NEXT:    (i32.load $0
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (i32.load $0 offset=4
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (unreachable)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $asyncify_start_rewind (param $0 i32)
;; CHECK-NEXT:  (global.set $__asyncify_state
;; CHECK-NEXT:   (i32.const 2)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (global.set $__asyncify_data
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (if
;; CHECK-NEXT:   (i32.gt_u
;; CHECK-NEXT:    (i32.load $0
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (i32.load $0 offset=4
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (unreachable)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $asyncify_stop_rewind
;; CHECK-NEXT:  (global.set $__asyncify_state
;; CHECK-NEXT:   (i32.const 0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (if
;; CHECK-NEXT:   (i32.gt_u
;; CHECK-NEXT:    (i32.load $0
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (i32.load $0 offset=4
;; CHECK-NEXT:     (global.get $__asyncify_data)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (unreachable)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $asyncify_get_state (result i32)
;; CHECK-NEXT:  (global.get $__asyncify_state)
;; CHECK-NEXT: )
