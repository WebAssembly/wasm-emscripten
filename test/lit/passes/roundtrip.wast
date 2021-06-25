;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --roundtrip -S -o - | filecheck %s

(module
 ;; CHECK:      (type $none (func))
 (type $none (func))
 ;; CHECK:      (type $none_=>_funcref_ref|$none| (func (result funcref (ref $none))))
 ;; CHECK:      (elem declare func $foo)
 ;; CHECK:      (func $foo
 ;; CHECK-NEXT:  (local $0 (funcref (ref null $none)))
 ;; CHECK-NEXT:  (local $1 funcref)
 ;; CHECK-NEXT:  (local.set $0
 ;; CHECK-NEXT:   (block $label$1 (result funcref (ref $none))
 ;; CHECK-NEXT:    (tuple.make
 ;; CHECK-NEXT:     (ref.null func)
 ;; CHECK-NEXT:     (ref.func $foo)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result funcref)
 ;; CHECK-NEXT:    (local.set $1
 ;; CHECK-NEXT:     (tuple.extract 0
 ;; CHECK-NEXT:      (local.get $0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (drop
 ;; CHECK-NEXT:     (ref.as_non_null
 ;; CHECK-NEXT:      (tuple.extract 1
 ;; CHECK-NEXT:       (local.get $0)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo
  (drop
   ;; a tuple type with a non-nullable element, that must be carefully handled
   (block (result funcref (ref $none))
    (tuple.make
     (ref.null func)
     (ref.func $foo)
    )
   )
  )
 )
)
