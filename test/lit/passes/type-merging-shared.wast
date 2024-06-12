;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --closed-world --type-merging --remove-unused-types -all -S -o - | filecheck %s

(module
 ;; Shared and non-shared types are not merged.
 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $C' (shared (func)))

 ;; CHECK:       (type $B' (shared (array i8)))

 ;; CHECK:       (type $B (array i8))

 ;; CHECK:       (type $A' (shared (struct )))

 ;; CHECK:       (type $A (struct ))
 (type $A (struct))
 (type $A' (shared (struct)))
 (type $B (array i8))
 (type $B' (shared (array i8)))
 ;; CHECK:       (type $C (func))
 (type $C (func))
 (type $C' (shared (func)))

 ;; CHECK:      (func $foo (type $C)
 ;; CHECK-NEXT:  (local $a (ref null $A))
 ;; CHECK-NEXT:  (local $a' (ref null $A'))
 ;; CHECK-NEXT:  (local $b (ref null $B))
 ;; CHECK-NEXT:  (local $b' (ref null $B'))
 ;; CHECK-NEXT:  (local $c (ref null $C))
 ;; CHECK-NEXT:  (local $c' (ref null $C'))
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $foo
  (local $a (ref null $A))
  (local $a' (ref null $A'))
  (local $b (ref null $B))
  (local $b' (ref null $B'))
  (local $c (ref null $C))
  (local $c' (ref null $C'))
 )
)

(module
 ;; But two shared types can be merged.
 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $B (shared (array i8)))

 ;; CHECK:       (type $A (shared (struct )))
 (type $A (shared (struct)))
 (type $A' (shared (struct)))
 (type $B (shared (array i8)))
 (type $B' (shared (array i8)))
 ;; CHECK:       (type $C (shared (func)))
 (type $C (shared (func)))
 (type $C' (shared (func)))

 ;; CHECK:      (func $foo (type $C)
 ;; CHECK-NEXT:  (local $a (ref null $A))
 ;; CHECK-NEXT:  (local $a' (ref null $A))
 ;; CHECK-NEXT:  (local $b (ref null $B))
 ;; CHECK-NEXT:  (local $b' (ref null $B))
 ;; CHECK-NEXT:  (local $c (ref null $C))
 ;; CHECK-NEXT:  (local $c' (ref null $C))
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $foo
  (local $a (ref null $A))
  (local $a' (ref null $A'))
  (local $b (ref null $B))
  (local $b' (ref null $B'))
  (local $c (ref null $C))
  (local $c' (ref null $C'))
 )
)
