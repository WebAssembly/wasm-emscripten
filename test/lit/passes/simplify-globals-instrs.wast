;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --simplify-globals -all -S -o - | filecheck %s

;; The globals will become immutable, and after doing so we can propagate the
;; value of the first into the second. The second's value then becomes constant
;; as well.
(module
  ;; CHECK:      (global $A i32 (i32.const 40))
  (global $A (mut i32) (i32.const 40))

  ;; CHECK:      (global $B i32 (i32.const 42))
  (global $B (mut i32) (i32.add
    (global.get $A)
    (i32.const 2)
  ))

  (memory 1)

  (data (i32.sub
    (i32.const 100)
    (global.get $B)
  ) "foo")

  (table 10 funcref)

  ;; CHECK:      (memory $0 1)

  ;; CHECK:      (data (i32.const 58) "foo")

  ;; CHECK:      (table $0 10 funcref)

  ;; CHECK:      (elem $0 (table $0) (i32.const 1) funcref (ref.null nofunc) (ref.null nofunc))
  (elem $0 (table 0) (i32.sub
    (global.get $B)
    (i32.const 41)
  ) funcref (ref.null func) (ref.null func))
)

;; Test we ignore stuff we can't optimize. The first global is now imported, so
;; there is no constant value to apply for it.
(module
  ;; CHECK:      (import "a" "a" (global $A (mut i32)))
  (import "a" "a" (global $A (mut i32)))

  ;; CHECK:      (global $B i32 (i32.add
  ;; CHECK-NEXT:  (global.get $A)
  ;; CHECK-NEXT:  (i32.const 2)
  ;; CHECK-NEXT: ))
  (global $B (mut i32) (i32.add
    (global.get $A)
    (i32.const 2)
  ))
)
