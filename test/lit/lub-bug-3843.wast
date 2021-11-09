;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --precompute -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --precompute --nominal -S -o - | filecheck %s --check-prefix NOMNL

;; Regression test for a bug (#3843) in which the LUB calculation done during
;; the refinalization of the select incorrectly produced a new type rather than
;; returning (ref null $A).

(module
 ;; CHECK:      (type $A (struct (field (ref null $C))))
 ;; NOMNL:      (type $A (struct_subtype (field (ref null $C)) data))
 (type $A (struct (field (ref null $C))))

 ;; CHECK:      (type $B (struct (field (ref null $D))))
 ;; NOMNL:      (type $B (struct_subtype (field (ref null $D)) $A))
 (type $B (struct_subtype (field (ref null $D)) $A))

 ;; CHECK:      (type $D (struct (field (mut (ref $A))) (field (mut (ref $A)))))
 ;; NOMNL:      (type $D (struct_subtype (field (mut (ref $A))) (field (mut (ref $A))) $C))
 (type $D (struct_subtype (field (mut (ref $A))) (field (mut (ref $A))) $C))

 ;; CHECK:      (type $C (struct (field (mut (ref $A)))))
 ;; NOMNL:      (type $C (struct_subtype (field (mut (ref $A))) data))
 (type $C (struct (field (mut (ref $A)))))


 ;; CHECK:      (func $foo (param $a (ref null $A)) (result (ref null $A))
 ;; CHECK-NEXT:  (select (result (ref null $A))
 ;; CHECK-NEXT:   (local.get $a)
 ;; CHECK-NEXT:   (ref.null $B)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $foo (type $ref?|$A|_=>_ref?|$A|) (param $a (ref null $A)) (result (ref null $A))
 ;; NOMNL-NEXT:  (select (result (ref null $A))
 ;; NOMNL-NEXT:   (local.get $a)
 ;; NOMNL-NEXT:   (ref.null $B)
 ;; NOMNL-NEXT:   (i32.const 0)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $foo (param $a (ref null $A)) (result (ref null $A))
  ;; the select should have type $A
  (select (result (ref null $A))
   ;; one arm has type $A
   (local.get $a)
   ;; one arm has type $B (a subtype of $A)
   (ref.null $B)
   (i32.const 0)
  )
 )
)
