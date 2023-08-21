;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --ignore-implicit-traps --optimize-instructions -all -S -o - \
;; RUN:   | filecheck %s

(module
  ;; CHECK:      (type $struct.A (struct (field i32)))
  (type $struct.A (struct i32))
  ;; CHECK:      (tag $e (param (ref null $struct.A)))
  (tag $e (param (ref null $struct.A)))

  ;; CHECK:      (func $ref-cast-statically-removed (type $none_=>_none)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch $e
  ;; CHECK-NEXT:    (throw $e
  ;; CHECK-NEXT:     (pop (ref null $struct.A))
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $ref-cast-statically-removed
    (try
      (do)
      (catch $e
        (throw $e
          ;; Because --ignore-implicit-traps is given, this ref.cast null is
          ;; assumed not to throw so this ref.cast null can be statically removed.
          ;; But that creates a block around this, making 'pop' nested into it,
          ;; which is invalid. We fix this up at the end up OptimizeInstruction,
          ;; assigning the 'pop' to a local at the start of this 'catch' body
          ;; and later using 'local.get' to get it.
          (ref.cast (ref null $struct.A)
            (pop (ref null $struct.A))
          )
        )
      )
    )
  )
)
