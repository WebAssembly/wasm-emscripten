;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; Test in both "always" mode, which always monomorphizes, and in "careful"
;; mode which does it only when it appears to actually help.

;; RUN: foreach %s %t wasm-opt --nominal --monomorphize-always -all -S -o - | filecheck %s --check-prefix ALWAYS
;; RUN: foreach %s %t wasm-opt --nominal --monomorphize        -all -S -o - | filecheck %s --check-prefix CAREFUL

(module
  ;; ALWAYS:      (type $A (struct_subtype  data))
  ;; CAREFUL:      (type $A (struct_subtype  data))
  (type $A (struct_subtype data))
  ;; ALWAYS:      (type $B (struct_subtype  $A))
  ;; CAREFUL:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; ALWAYS:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; ALWAYS:      (type $none_=>_none (func_subtype func))

  ;; ALWAYS:      (type $ref|$B|_=>_none (func_subtype (param (ref $B)) func))

  ;; ALWAYS:      (import "a" "b" (func $import (param (ref $A))))
  ;; CAREFUL:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; CAREFUL:      (type $none_=>_none (func_subtype func))

  ;; CAREFUL:      (import "a" "b" (func $import (param (ref $A))))
  (import "a" "b" (func $import (param (ref $A))))

  ;; ALWAYS:      (func $calls (type $none_=>_none)
  ;; ALWAYS-NEXT:  (call $refinable
  ;; ALWAYS-NEXT:   (struct.new_default $A)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $refinable
  ;; ALWAYS-NEXT:   (struct.new_default $A)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $refinable_0
  ;; ALWAYS-NEXT:   (struct.new_default $B)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $refinable_0
  ;; ALWAYS-NEXT:   (struct.new_default $B)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $calls (type $none_=>_none)
  ;; CAREFUL-NEXT:  (call $refinable
  ;; CAREFUL-NEXT:   (struct.new_default $A)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $refinable
  ;; CAREFUL-NEXT:   (struct.new_default $A)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $refinable
  ;; CAREFUL-NEXT:   (struct.new_default $B)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $refinable
  ;; CAREFUL-NEXT:   (struct.new_default $B)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
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

  ;; ALWAYS:      (func $call-import (type $none_=>_none)
  ;; ALWAYS-NEXT:  (call $import
  ;; ALWAYS-NEXT:   (struct.new_default $B)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-import (type $none_=>_none)
  ;; CAREFUL-NEXT:  (call $import
  ;; CAREFUL-NEXT:   (struct.new_default $B)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-import
    ;; Calls to imports are left as they are.
    (call $import
      (struct.new $B)
    )
  )

  ;; ALWAYS:      (func $refinable (type $ref|$A|_=>_none) (param $ref (ref $A))
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $ref)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $refinable (type $ref|$A|_=>_none) (param $0 (ref $A))
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $refinable (param $ref (ref $A))
    ;; Helper function for the above. Use the parameter to see we update types
    ;; etc when we make a refined version of the function (if we didn't,
    ;; validation would error).
    (drop
      (local.get $ref)
    )
  )
)

;; As above, but now the refinable function uses the local in a way that
;; requires a fixup.
;; ALWAYS:      (func $refinable_0 (type $ref|$B|_=>_none) (param $ref (ref $B))
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (local.get $ref)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )
(module
  ;; ALWAYS:      (type $A (struct_subtype  data))
  ;; CAREFUL:      (type $none_=>_none (func_subtype func))

  ;; CAREFUL:      (type $A (struct_subtype  data))
  (type $A (struct_subtype data))
  ;; ALWAYS:      (type $B (struct_subtype  $A))
  ;; CAREFUL:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))



  ;; ALWAYS:      (type $none_=>_none (func_subtype func))

  ;; ALWAYS:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; ALWAYS:      (type $ref|$B|_=>_none (func_subtype (param (ref $B)) func))

  ;; ALWAYS:      (func $calls (type $none_=>_none)
  ;; ALWAYS-NEXT:  (call $refinable_0
  ;; ALWAYS-NEXT:   (struct.new_default $B)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; CAREFUL:      (func $calls (type $none_=>_none)
  ;; CAREFUL-NEXT:  (call $refinable
  ;; CAREFUL-NEXT:   (struct.new_default $B)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $calls
    (call $refinable
      (struct.new $B)
    )
  )

  ;; ALWAYS:      (func $refinable (type $ref|$A|_=>_none) (param $ref (ref $A))
  ;; ALWAYS-NEXT:  (local $unref (ref $A))
  ;; ALWAYS-NEXT:  (local.set $unref
  ;; ALWAYS-NEXT:   (local.get $ref)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (local.set $ref
  ;; ALWAYS-NEXT:   (local.get $unref)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $refinable (type $ref|$A|_=>_none) (param $0 (ref $A))
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $refinable (param $ref (ref $A))
    (local $unref (ref $A))
    (local.set $unref
      (local.get $ref)
    )
    ;; If we refine $ref then this set will be invalid - we'd be setting a less-
    ;; refined type into a local/param that is more refined. We should fix this
    ;; up by using a temp local.
    (local.set $ref
      (local.get $unref)
    )
  )
)

;; Multiple refinings of the same function, and of different functions.
;; ALWAYS:      (func $refinable_0 (type $ref|$B|_=>_none) (param $ref (ref $B))
;; ALWAYS-NEXT:  (local $unref (ref $A))
;; ALWAYS-NEXT:  (local $2 (ref $A))
;; ALWAYS-NEXT:  (local.set $2
;; ALWAYS-NEXT:   (local.get $ref)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block
;; ALWAYS-NEXT:   (local.set $unref
;; ALWAYS-NEXT:    (local.get $2)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (local.set $2
;; ALWAYS-NEXT:    (local.get $unref)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )
(module
  ;; ALWAYS:      (type $A (struct_subtype  data))
  ;; CAREFUL:      (type $none_=>_none (func_subtype func))

  ;; CAREFUL:      (type $A (struct_subtype  data))
  (type $A (struct_subtype data))
  ;; ALWAYS:      (type $B (struct_subtype  $A))
  ;; CAREFUL:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; ALWAYS:      (type $none_=>_none (func_subtype func))

  ;; ALWAYS:      (type $C (struct_subtype  $B))
  ;; CAREFUL:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; CAREFUL:      (type $C (struct_subtype  $B))
  (type $C (struct_subtype $B))

  ;; ALWAYS:      (type $ref|$A|_=>_none (func_subtype (param (ref $A)) func))

  ;; ALWAYS:      (type $ref|$B|_=>_none (func_subtype (param (ref $B)) func))

  ;; ALWAYS:      (type $ref|$C|_=>_none (func_subtype (param (ref $C)) func))

  ;; ALWAYS:      (func $calls1 (type $none_=>_none)
  ;; ALWAYS-NEXT:  (call $refinable1
  ;; ALWAYS-NEXT:   (struct.new_default $A)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $refinable1_0
  ;; ALWAYS-NEXT:   (struct.new_default $B)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $calls1 (type $none_=>_none)
  ;; CAREFUL-NEXT:  (call $refinable1
  ;; CAREFUL-NEXT:   (struct.new_default $A)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $refinable1
  ;; CAREFUL-NEXT:   (struct.new_default $B)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $calls1
    (call $refinable1
      (struct.new $A)
    )
    (call $refinable1
      (struct.new $B)
    )
  )

  ;; ALWAYS:      (func $calls2 (type $none_=>_none)
  ;; ALWAYS-NEXT:  (call $refinable1_1
  ;; ALWAYS-NEXT:   (struct.new_default $C)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $refinable2_0
  ;; ALWAYS-NEXT:   (struct.new_default $B)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $calls2 (type $none_=>_none)
  ;; CAREFUL-NEXT:  (call $refinable1
  ;; CAREFUL-NEXT:   (struct.new_default $C)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $refinable2
  ;; CAREFUL-NEXT:   (struct.new_default $B)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $calls2
    (call $refinable1
      (struct.new $C)
    )
    (call $refinable2
      (struct.new $B)
    )
  )

  ;; ALWAYS:      (func $refinable1 (type $ref|$A|_=>_none) (param $ref (ref $A))
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $ref)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $refinable1 (type $ref|$A|_=>_none) (param $0 (ref $A))
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $refinable1 (param $ref (ref $A))
    (drop
      (local.get $ref)
    )
  )

  ;; ALWAYS:      (func $refinable2 (type $ref|$A|_=>_none) (param $ref (ref $A))
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $ref)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $refinable2 (type $ref|$A|_=>_none) (param $0 (ref $A))
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $refinable2 (param $ref (ref $A))
    (drop
      (local.get $ref)
    )
  )
)
;; ALWAYS:      (func $refinable1_0 (type $ref|$B|_=>_none) (param $ref (ref $B))
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (local.get $ref)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $refinable1_1 (type $ref|$C|_=>_none) (param $ref (ref $C))
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (local.get $ref)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $refinable2_0 (type $ref|$B|_=>_none) (param $ref (ref $B))
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (local.get $ref)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )
