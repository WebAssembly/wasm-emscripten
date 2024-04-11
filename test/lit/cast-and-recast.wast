;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; Test that our hack for br_if output types does not cause the binary to grow
;; linearly with each roundtrip. When we emit a br_if whose output type is not
;; refined enough (Binaryen IR uses the value's type; wasm uses the target's)
;; then we add a cast. We then remove trivial casts like it during load, when we
;; see they are unneeded.
;;
;; This is also used as the input in test/lit/binary/cast-and-recast.test, which
;; verifies the binary format itself.

;; RUN: wasm-opt %s -all --roundtrip --roundtrip --roundtrip -S -o - | filecheck %s

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (sub (struct )))
    (type $A (sub (struct)))
    ;; CHECK:       (type $B (sub $A (struct )))
    (type $B (sub $A (struct)))
  )

  ;; CHECK:      (func $test (type $2) (param $B (ref $B)) (param $x i32) (result anyref)
  ;; CHECK-NEXT:  (block $label$1 (result (ref $A))
  ;; CHECK-NEXT:   (br_if $label$1
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $B (ref $B)) (param $x i32) (result anyref)
    (block $out (result (ref $A))
      ;; The br_if's value is of type $B which is more precise than the block's
      ;; type, $A.
      (br_if $out
        (local.get $B)
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $test-drop (type $2) (param $B (ref $B)) (param $x i32) (result anyref)
  ;; CHECK-NEXT:  (block $label$1 (result (ref $A))
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (br_if $label$1
  ;; CHECK-NEXT:     (local.get $B)
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-drop (param $B (ref $B)) (param $x i32) (result anyref)
    ;; As above, but with a drop of the br_if value.
    (block $out (result (ref $A))
      (drop
        (br_if $out
          (local.get $B)
          (local.get $x)
        )
      )
      (unreachable)
    )
  )

  ;; CHECK:      (func $test-same (type $3) (param $A (ref $A)) (param $x i32) (result anyref)
  ;; CHECK-NEXT:  (block $label$1 (result (ref $A))
  ;; CHECK-NEXT:   (br_if $label$1
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-same (param $A (ref $A)) (param $x i32) (result anyref)
    ;; As above, but now we use $A everywhere, which means there is no
    ;; difference between the type in Binaryen IR and wasm, so we do not need
    ;; to emit any extra cast here. That cannot be observed in this test (as if
    ;; a cast were added, the binary reader would remove it), but keep it here
    ;; for completeness, and because this file serves as the input to
    ;; test/lit/binary/cast-and-recast.test.
    (block $out (result (ref $A))
      (br_if $out
        (local.get $A)
        (local.get $x)
      )
    )
  )
)
