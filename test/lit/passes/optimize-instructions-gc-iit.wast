;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-instructions --ignore-implicit-traps --enable-reference-types --enable-gc -S -o - \
;; RUN:   | filecheck %s

(module
  (type $A (struct (field i32)))
  (type $B (struct (field i32) (field f64)))
  (type $C (struct (field i64) (field f32)))

  ;; CHECK:      (func $store-trunc2 (param $a (ref $A)) (param $b (ref $B)) (param $c (ref $C)) (param $a-rtt (rtt $A)) (param $b-rtt (rtt $B)) (param $c-rtt (rtt $C))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $A))
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $a-rtt)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $B))
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $a-rtt)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $b)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:    (local.get $b-rtt)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast
  ;; CHECK-NEXT:    (local.get $b)
  ;; CHECK-NEXT:    (local.get $c-rtt)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $store-trunc2
    (param $a (ref $A))
    (param $b (ref $B))
    (param $c (ref $C))

    (param $a-rtt (rtt $A))
    (param $b-rtt (rtt $B))
    (param $c-rtt (rtt $C))

    ;; a cast of A to an rtt of A: static subtyping matches.
    (drop
      (ref.cast
        (local.get $a)
        (local.get $a-rtt)
      )
    )
    ;; a cast of B to a supertype: static subtyping matches.
    (drop
      (ref.cast
        (local.get $b)
        (local.get $a-rtt)
      )
    )
    ;; a cast of A to a subtype: static subtyping does not match.
    (drop
      (ref.cast
        (local.get $a)
        (local.get $b-rtt)
      )
    )
    ;; a cast of B to an unrelated type: static subtyping does not match.
    (drop
      (ref.cast
        (local.get $b)
        (local.get $c-rtt)
      )
    )
  )
)
