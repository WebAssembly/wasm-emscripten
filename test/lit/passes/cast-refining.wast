;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --cast-refining --traps-never-happen -all --closed-world --nominal -S -o - | filecheck %s --check-prefix=YESTNH
;; RUN: foreach %s %t wasm-opt --cast-refining                      -all --closed-world --nominal -S -o - | filecheck %s --check-prefix=NO_TNH

;; Run in both TNH and non-TNH mode.

;; $A :> $B :> $C :> $D :> $E
;;
;; $A and $D have no struct.news, so we can optimize casts of them to their
;; subtypes, in TNH mode. In that mode $A and $D will also not be emitted in
;; the output anymore.
(module
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (type $A (struct ))
  (type $A (struct))

  ;; YESTNH:      (type $B (struct ))
  ;; NO_TNH:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (type $C (struct_subtype  $B))
  ;; NO_TNH:      (type $C (struct_subtype  $B))
  (type $C (struct_subtype $B))

  ;; NO_TNH:      (type $D (struct_subtype  $C))
  (type $D (struct_subtype $C))

  ;; YESTNH:      (type $E (struct_subtype  $C))
  ;; NO_TNH:      (type $E (struct_subtype  $D))
  (type $E (struct_subtype $D))

  ;; YESTNH:      (global $global anyref (struct.new_default $B))
  ;; NO_TNH:      (global $global anyref (struct.new_default $B))
  (global $global anyref (struct.new $B))

  ;; YESTNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $C)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $E)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $C)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $E)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $C)
    )
    (drop
      (struct.new $E)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $E
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $E
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $C
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $D
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $E
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    ;; List out all possible casts for comprehensiveness. For other instructions
    ;; we are more focused, below.
    (drop
      (ref.cast $A     ;; This will be $B in TNH.
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C
        (local.get $x)
      )
    )
    (drop
      (ref.cast $D     ;; This will be $E in TNH.
        (local.get $x)
      )
    )
    (drop
      (ref.cast $E
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.test (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.test $B
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.test (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.test $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.test (param $x anyref)
    (drop
      (ref.test $A
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $br_on (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (block $block (result (ref $B))
  ;; YESTNH-NEXT:    (drop
  ;; YESTNH-NEXT:     (br_on_cast $block $B
  ;; YESTNH-NEXT:      (local.get $x)
  ;; YESTNH-NEXT:     )
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $br_on (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block $block (result anyref)
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (br_on_cast $block $A
  ;; NO_TNH-NEXT:      (local.get $x)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $br_on (param $x anyref)
    (drop
      (block $block (result anyref)
        (drop
          (br_on_cast $block $A
            (local.get $x)
          )
        )
        (unreachable)
      )
    )
  )

  ;; YESTNH:      (func $basic (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast struct
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.as_i31
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $basic (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast struct
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.as_i31
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $basic (param $x anyref)
    ;; Casts to basic types should not be modified.
    (drop
      (ref.cast struct
        (local.get $x)
      )
    )
    (drop
      (ref.as_i31
        (local.get $x)
      )
    )
  )
)

;; $A has two subtypes. As a result, we cannot optimize it.
(module
  ;; YESTNH:      (type $A (struct ))
  ;; NO_TNH:      (type $A (struct ))
  (type $A (struct))

  ;; YESTNH:      (type $B (struct_subtype  $A))
  ;; NO_TNH:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (type $B1 (struct_subtype  $A))
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (type $B1 (struct_subtype  $A))
  (type $B1 (struct_subtype $A)) ;; this is a new type

  ;; YESTNH:      (global $global anyref (struct.new_default $B))
  ;; NO_TNH:      (global $global anyref (struct.new_default $B))
  (global $global anyref (struct.new $B))

  ;; YESTNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $B1)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $B1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $B1)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $A
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B1
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    (drop
      (ref.cast $A     ;; This will not be optimized like before.
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B1
        (local.get $x)
      )
    )
  )
)

;; As above, but now $B is never created, so we can optimize casts of $A to
;; $B1.
(module
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (type $A (struct ))
  (type $A (struct))

  (type $B (struct_subtype $A))

  ;; YESTNH:      (type $B1 (struct ))
  ;; NO_TNH:      (type $B1 (struct_subtype  $A))
  (type $B1 (struct_subtype $A)) ;; this is a new type

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $B1)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $B1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $B1)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B1
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    (drop
      (ref.cast $A     ;; This will be optimized to $B1.
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B     ;; $B is never created, so this will trap, in both TNH
        (local.get $x) ;; and non-TNH modes.
      )
    )
    (drop
      (ref.cast $B1
        (local.get $x)
      )
    )
  )
)

;; A chain, $A :> $B :> $C, where we can optimize $A all the way to $C.
(module
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (type $A (struct ))
  (type $A (struct))

  ;; NO_TNH:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; YESTNH:      (type $C (struct ))
  ;; NO_TNH:      (type $C (struct_subtype  $B))
  (type $C (struct_subtype $B))

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $C)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $C)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $C)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $C
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    (drop
      (ref.cast $A     ;; This can be $C.
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B     ;; This can also be $C.
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C
        (local.get $x)
      )
    )
  )
)

;; More testing for cases where no types or subtypes are created. No type is
;; created here. No type needs to be emitted in the output.
(module
  (type $A (struct))

  (type $B (struct_subtype $A))

  (type $C1 (struct_subtype $B))

  (type $C2 (struct_subtype $B))

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    ;; All these will trap.
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C1
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C2
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.cast.null (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast.null (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast.null (param $x anyref)
    ;; These can only pass through a null.
    (drop
      (ref.cast null $A
        (local.get $x)
      )
    )
    (drop
      (ref.cast null $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast null $C1
        (local.get $x)
      )
    )
    (drop
      (ref.cast null $C2
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.test (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.test none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.test null none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.test (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.test none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.test null none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.test (param $x anyref)
    ;; This will return 0.
    (drop
      (ref.test $A
        (local.get $x)
      )
    )
    ;; This can test for a null.
    (drop
      (ref.test null $A
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $br_on (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (block $block (result (ref none))
  ;; YESTNH-NEXT:    (drop
  ;; YESTNH-NEXT:     (br_on_cast $block none
  ;; YESTNH-NEXT:      (local.get $x)
  ;; YESTNH-NEXT:     )
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (block $block0 (result (ref any))
  ;; YESTNH-NEXT:    (br_on_non_null $block0
  ;; YESTNH-NEXT:     (local.get $x)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $br_on (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block $block (result (ref none))
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (br_on_cast $block none
  ;; NO_TNH-NEXT:      (local.get $x)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block $block0 (result (ref any))
  ;; NO_TNH-NEXT:    (br_on_non_null $block0
  ;; NO_TNH-NEXT:     (local.get $x)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $br_on (param $x anyref)
    ;; As above, this can be a cast to the bottom type.
    (drop
      (block $block (result anyref)
        (drop
          (br_on_cast $block $B
            (local.get $x)
          )
        )
        (unreachable)
      )
    )
    ;; Non-cast br_on* can be ignored.
    (drop
      (block $block (result anyref)
        (br_on_non_null $block
          (local.get $x)
        )
        (unreachable)
      )
    )
  )
)

;; As above, but now $C1 is created.
(module
  ;; NO_TNH:      (type $A (struct ))
  (type $A (struct))

  ;; NO_TNH:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; YESTNH:      (type $C1 (struct ))
  ;; NO_TNH:      (type $C1 (struct_subtype  $B))
  (type $C1 (struct_subtype $B))

  (type $C2 (struct_subtype $B))

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (global $global anyref (struct.new_default $C1))
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (global $global anyref (struct.new_default $C1))
  (global $global anyref (struct.new $C1))

  ;; YESTNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $C1
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    ;; These three can be cast to $C1 in TNH.
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C1
        (local.get $x)
      )
    )
    ;; This will trap.
    (drop
      (ref.cast $C2
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.cast.null (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null $C1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null $C1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null $C1
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast null none
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast.null (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null $C1
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast null none
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast.null (param $x anyref)
    ;; These three can be cast to $C1 in TNH.
    (drop
      (ref.cast null $A
        (local.get $x)
      )
    )
    (drop
      (ref.cast null $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast null $C1
        (local.get $x)
      )
    )
    ;; This returns null.
    (drop
      (ref.cast null $C2
        (local.get $x)
      )
    )
  )
)

;; Function subtyping, which is a TODO - for now we do nothing.
(module
  ;; YESTNH:      (type $A (func))
  ;; NO_TNH:      (type $A (func))
  (type $A (func))

  ;; YESTNH:      (type $funcref_=>_none (func (param funcref)))

  ;; YESTNH:      (type $B (func_subtype $A))
  ;; NO_TNH:      (type $funcref_=>_none (func (param funcref)))

  ;; NO_TNH:      (type $B (func_subtype $A))
  (type $B (func_subtype $A))

  ;; YESTNH:      (type $C (func_subtype $B))
  ;; NO_TNH:      (type $C (func_subtype $B))
  (type $C (func_subtype $B))

  ;; YESTNH:      (func $A (type $A)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $A (type $A)
  ;; NO_TNH-NEXT:  (nop)
  ;; NO_TNH-NEXT: )
  (func $A (type $A)
  )

  ;; YESTNH:      (func $C (type $A)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $C (type $A)
  ;; NO_TNH-NEXT:  (nop)
  ;; NO_TNH-NEXT: )
  (func $C (type $A)
  )

  ;; YESTNH:      (func $casts (type $funcref_=>_none) (param $x funcref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $A
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $casts (type $funcref_=>_none) (param $x funcref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $C
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $casts (param $x funcref)
    ;; $A and $C have functions of their types, so in theory we could optimize
    ;; $B here.
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C
        (local.get $x)
      )
    )
  )
)

;; Array subtyping, which is a TODO - for now we do nothing.
(module
  ;; YESTNH:      (type $A (array (mut i32)))
  ;; NO_TNH:      (type $A (array (mut i32)))
  (type $A (array (mut i32)))

  ;; YESTNH:      (type $B (array_subtype (mut i32) $A))
  ;; NO_TNH:      (type $B (array_subtype (mut i32) $A))
  (type $B (array_subtype (mut i32) $A))

  ;; YESTNH:      (type $C (array_subtype (mut i32) $B))
  ;; NO_TNH:      (type $C (array_subtype (mut i32) $B))
  (type $C (array_subtype (mut i32) $B))

  ;; YESTNH:      (type $anyref_=>_none (func (param anyref)))

  ;; YESTNH:      (global $A (ref $A) (array.new $A
  ;; YESTNH-NEXT:  (i32.const 10)
  ;; YESTNH-NEXT:  (i32.const 20)
  ;; YESTNH-NEXT: ))
  ;; NO_TNH:      (type $anyref_=>_none (func (param anyref)))

  ;; NO_TNH:      (global $A (ref $A) (array.new $A
  ;; NO_TNH-NEXT:  (i32.const 10)
  ;; NO_TNH-NEXT:  (i32.const 20)
  ;; NO_TNH-NEXT: ))
  (global $A (ref $A) (array.new $A
    (i32.const 10)
    (i32.const 20)
  ))

  ;; YESTNH:      (global $B (ref $B) (array.new $B
  ;; YESTNH-NEXT:  (i32.const 10)
  ;; YESTNH-NEXT:  (i32.const 20)
  ;; YESTNH-NEXT: ))
  ;; NO_TNH:      (global $B (ref $B) (array.new $B
  ;; NO_TNH-NEXT:  (i32.const 10)
  ;; NO_TNH-NEXT:  (i32.const 20)
  ;; NO_TNH-NEXT: ))
  (global $B (ref $B) (array.new $B
    (i32.const 10)
    (i32.const 20)
  ))

  ;; YESTNH:      (global $C (ref $C) (array.new $C
  ;; YESTNH-NEXT:  (i32.const 10)
  ;; YESTNH-NEXT:  (i32.const 20)
  ;; YESTNH-NEXT: ))
  ;; NO_TNH:      (global $C (ref $C) (array.new $C
  ;; NO_TNH-NEXT:  (i32.const 10)
  ;; NO_TNH-NEXT:  (i32.const 20)
  ;; NO_TNH-NEXT: ))
  (global $C (ref $C) (array.new $C
    (i32.const 10)
    (i32.const 20)
  ))

  ;; YESTNH:      (func $casts (type $anyref_=>_none) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $A
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $B
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast $C
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $casts (type $anyref_=>_none) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $A
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $B
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast $C
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $casts (param $x anyref)
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (ref.cast $C
        (local.get $x)
      )
    )
  )
)

;; TODO: test a local type gets modded
