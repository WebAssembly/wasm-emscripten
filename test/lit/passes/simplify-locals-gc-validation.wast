;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --simplify-locals -all -S -o - | filecheck %s

;; Tests for validation of non-nullable locals. In this form a local.set allows
;; a local.get until the end of the current block.

(module
  ;; CHECK:      (func $test-nn (param $x (ref any))
  ;; CHECK-NEXT:  (local $nn anyref)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (block $inner
  ;; CHECK-NEXT:   (call $test-nn
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.tee $nn
  ;; CHECK-NEXT:      (ref.as_non_null
  ;; CHECK-NEXT:       (ref.null none)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $test-nn
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $nn)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-nn (param $x (ref any))
    (local $nn (ref any))
    ;; We can sink this set into the block, but we should then update things so
    ;; that we still validate, as then the final local.get is not structurally
    ;; dominated. (Note that we end up with several ref.as_non_nulls here, but
    ;; later passes could remove them.)
    (local.set $nn
      (ref.as_non_null
        (ref.null any)
      )
    )
    (block $inner
      (call $test-nn
        (local.get $nn)
      )
    )
    (call $test-nn
      (local.get $nn)
    )
  )
)
