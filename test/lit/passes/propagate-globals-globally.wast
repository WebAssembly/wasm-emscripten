;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --propagate-globals-globally -all -S -o - | filecheck %s

;; The global constants should be applied to other globals.

(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (global $A i32 (i32.const 42))
  (global $A i32 (i32.const 42))

  ;; CHECK:      (global $B i32 (i32.const 42))
  (global $B i32 (global.get $A))

  ;; CHECK:      (global $C i32 (i32.add
  ;; CHECK-NEXT:  (global.get $B)
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $C i32 (i32.add
    (global.get $B)
    (global.get $A)
  ))

  ;; CHECK:      (global $D (ref string) (string.const "foo"))
  (global $D (ref string) (string.const "foo"))

  ;; CHECK:      (global $E (ref string) (string.const "foo"))
  (global $E (ref string) (global.get $D))

  ;; CHECK:      (func $test (type $0)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $A)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $B)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $C)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $D)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    ;; We should not change anything here: this pass propagates globals
    ;; *globally*, and not to functions.
    (drop
      (global.get $A)
    )
    (drop
      (global.get $B)
    )
    (drop
      (global.get $C)
    )
    (drop
      (global.get $D)
    )
  )
)
