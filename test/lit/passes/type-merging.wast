;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --nominal --type-merging -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $A (struct_subtype (field i32) data))
  (type $A (struct_subtype (field i32) data))

  ;; CHECK:      (type $D (struct_subtype (field i32) data))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (type $B (struct_subtype (field i32) data))
  (type $B (struct_subtype (field i32) data))

  ;; CHECK:      (type $C (struct_subtype (field i32) (field f64) data))
  (type $C (struct_subtype (field i32) (field f64) data))

  (type $D (struct_subtype (field i32) data))

  ;; CHECK:      (func $foo (type $none_=>_none)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $B))
  ;; CHECK-NEXT:  (local $c (ref null $C))
  ;; CHECK-NEXT:  (local $d (ref null $D))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast_static $A
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast_static $D
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo
    (local $a (ref null $A))
    ;; $B can be merged into $A.
    (local $b (ref null $B))
    ;; $C cannot because it adds a field.
    (local $c (ref null $C))
    ;; $D cannot because it has a cast.
    (local $d (ref null $D))

    ;; A cast of $A has no effect.
    (drop
      (ref.cast_static $A
        (local.get $a)
      )
    )
    ;; A cast of $D prevents it from being merged.
    (drop
      (ref.cast_static $D
        (local.get $a)
      )
    )
  )
)
