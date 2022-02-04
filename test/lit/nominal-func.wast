;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --nominal -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --nominal --roundtrip -S -o - | filecheck %s

(module
  ;; This will be the "canonical" function type rather than $foo_t
  (type $bad_t (func))

  ;; CHECK:      (type $foo_t (func_subtype func))
  (type $foo_t (func))

  ;; CHECK:      (func $foo (type $foo_t)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $foo (type $foo_t)
    (unreachable)
  )

  ;; $foo needs to be assigned type foo_t rather than bad_t for this to validate.
  ;; CHECK:      (func $make-ref (type $func.0) (result (ref $foo_t))
  ;; CHECK-NEXT:  (ref.func $foo)
  ;; CHECK-NEXT: )
  (func $make-ref (result (ref $foo_t))
    (ref.func $foo)
  )
)
