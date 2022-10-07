;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: wasm-opt %s -all --hybrid -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --hybrid --roundtrip -S -o - | filecheck %s

;; Check that unused types are still included in the output when they are part
;; of a recursion group with used types.

(module


 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $used (struct_subtype  data))
  (type $used (struct_subtype data))
  ;; CHECK:       (type $unused (struct_subtype  data))
  (type $unused (struct_subtype data))
 )

 ;; CHECK:      (global $g (ref null $used) (ref.null none))
 (global $g (ref null $used) (ref.null $used))
)
