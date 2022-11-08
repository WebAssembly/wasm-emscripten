;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --nominal --monomorphize -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $A (struct_subtype  data))
  (type $A (struct_subtype data))
  ;; CHECK:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; CHECK:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (type $ref|$B|_=>_none (func_subtype (param (ref $B)) func))

  ;; CHECK:      (import "a" "b" (func $import (param (ref $A))))
  (import "a" "b" (func $import (param (ref $A))))

  ;; CHECK:      (func $calls (type $none_=>_none)
  ;; CHECK-NEXT:  (call $refinable
  ;; CHECK-NEXT:   (struct.new_default $A)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $refinable
  ;; CHECK-NEXT:   (struct.new_default $A)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $refinable_0
  ;; CHECK-NEXT:   (struct.new_default $B)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $refinable_0
  ;; CHECK-NEXT:   (struct.new_default $B)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $calls
    ;; Two calls with $A, two with $B. The calls to $B should both go to the
    ;; same new function which has a refined parameter of $B.
    (call $refinable
      (struct.new $A)
    )
    (call $refinable
      (struct.new $A)
    )
    (call $refinable
      (struct.new $B)
    )
    (call $refinable
      (struct.new $B)
    )
  )

  ;; CHECK:      (func $call-import (type $none_=>_none)
  ;; CHECK-NEXT:  (call $import
  ;; CHECK-NEXT:   (struct.new_default $B)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-import
    ;; Calls to imports are left as they are.
    (call $import
      (struct.new $B)
    )
  )

  ;; CHECK:      (func $refinable (type $ref|$A|_=>_none) (param $ref (ref $A))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $refinable (param $ref (ref $A))
    ;; Helper function for the above
  )
)
;; CHECK:      (func $refinable_0 (type $ref|$B|_=>_none) (param $ref (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
