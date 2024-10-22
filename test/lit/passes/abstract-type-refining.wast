;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --abstract-type-refining --remove-unused-types --traps-never-happen \
;; RUN:     -all --closed-world --preserve-type-order -S -o - | filecheck %s --check-prefix=YESTNH
;; RUN: foreach %s %t wasm-opt --abstract-type-refining --remove-unused-types \
;; RUN:     -all --closed-world --preserve-type-order -S -o - | filecheck %s --check-prefix=NO_TNH

;; Run in both TNH and non-TNH mode.

;; $A :> $B :> $C :> $D :> $E
;;
;; $A and $D have no struct.news, so any operations on them must, in TNH mode,
;; actually refer to a subtype of them (that has a struct.new). As a result, in
;; TNH mode $A and $D will also not be emitted in the output anymore.
(module
  ;; NO_TNH:      (rec
  ;; NO_TNH-NEXT:  (type $A (sub (struct)))
  (type $A (sub (struct)))

  ;; YESTNH:      (rec
  ;; YESTNH-NEXT:  (type $B (sub (struct)))
  ;; NO_TNH:       (type $B (sub $A (struct)))
  (type $B (sub $A (struct)))

  ;; YESTNH:       (type $C (sub $B (struct)))
  ;; NO_TNH:       (type $C (sub $B (struct)))
  (type $C (sub $B (struct)))

  ;; NO_TNH:       (type $D (sub $C (struct)))
  (type $D (sub $C (struct)))

  ;; YESTNH:       (type $E (sub $C (struct)))
  ;; NO_TNH:       (type $E (sub $D (struct)))
  (type $E (sub $D (struct)))

  ;; YESTNH:       (type $3 (func (param anyref)))

  ;; YESTNH:       (type $4 (func))

  ;; YESTNH:      (global $global anyref (struct.new_default $B))
  ;; NO_TNH:       (type $5 (func (param anyref)))

  ;; NO_TNH:       (type $6 (func))

  ;; NO_TNH:      (global $global anyref (struct.new_default $B))
  (global $global anyref (struct.new $B))

  ;; YESTNH:      (func $new (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $C)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $E)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $5) (param $x anyref)
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

  ;; YESTNH:      (func $ref.cast (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $E)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $E)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $5) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $C)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $D)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $E)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    ;; List out all possible casts for comprehensiveness. For other instructions
    ;; we are more focused, below.
    (drop
      (ref.cast (ref $A)     ;; This will be $B in TNH.
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $D)     ;; This will be $E in TNH.
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $E)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.test (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.test (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.test (type $5) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.test (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.test (param $x anyref)
    (drop
      (ref.test (ref $A)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $br_on (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (block $block (result (ref $B))
  ;; YESTNH-NEXT:    (drop
  ;; YESTNH-NEXT:     (br_on_cast $block anyref (ref $B)
  ;; YESTNH-NEXT:      (local.get $x)
  ;; YESTNH-NEXT:     )
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $br_on (type $5) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block $block (result anyref)
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (br_on_cast $block anyref (ref $A)
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
          (br_on_cast $block anyref (ref $A)
            (local.get $x)
          )
        )
        (unreachable)
      )
    )
  )

  ;; YESTNH:      (func $basic (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref struct)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $basic (type $5) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref struct)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $basic (param $x anyref)
    ;; Casts to basic types should not be modified.
    (drop
      (ref.cast (ref struct)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $locals (type $4)
  ;; YESTNH-NEXT:  (local $A (ref $B))
  ;; YESTNH-NEXT:  (local $B (ref $B))
  ;; YESTNH-NEXT:  (local $C (ref $C))
  ;; YESTNH-NEXT:  (local $D (ref $E))
  ;; YESTNH-NEXT:  (local $E (ref $E))
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $locals (type $6)
  ;; NO_TNH-NEXT:  (local $A (ref $A))
  ;; NO_TNH-NEXT:  (local $B (ref $B))
  ;; NO_TNH-NEXT:  (local $C (ref $C))
  ;; NO_TNH-NEXT:  (local $D (ref $D))
  ;; NO_TNH-NEXT:  (local $E (ref $E))
  ;; NO_TNH-NEXT:  (nop)
  ;; NO_TNH-NEXT: )
  (func $locals
    ;; Local variable types are also updated.
    (local $A (ref $A))
    (local $B (ref $B))
    (local $C (ref $C))
    (local $D (ref $D))
    (local $E (ref $E))
  )
)

;; $A has two subtypes. As a result, we cannot optimize it.
(module
  (rec
    ;; YESTNH:      (rec
    ;; YESTNH-NEXT:  (type $A (sub (struct)))
    ;; NO_TNH:      (rec
    ;; NO_TNH-NEXT:  (type $A (sub (struct)))
    (type $A (sub (struct)))

    ;; YESTNH:       (type $B (sub $A (struct)))
    ;; NO_TNH:       (type $B (sub $A (struct)))
    (type $B (sub $A (struct)))

    ;; YESTNH:       (type $B1 (sub $A (struct)))
    ;; NO_TNH:       (type $B1 (sub $A (struct)))
    (type $B1 (sub $A (struct))) ;; this is a new type
  )

  ;; YESTNH:       (type $3 (func (param anyref)))

  ;; YESTNH:      (global $global anyref (struct.new_default $B))
  ;; NO_TNH:       (type $3 (func (param anyref)))

  ;; NO_TNH:      (global $global anyref (struct.new_default $B))
  (global $global anyref (struct.new $B))

  ;; YESTNH:      (func $new (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $B1)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $B1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $B1)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $A)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B1)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    (drop
      (ref.cast (ref $A)     ;; This will not be optimized like before.
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B1)
        (local.get $x)
      )
    )
  )
)

;; As above, but now $B is never created, so we can optimize casts of $A to
;; $B1.
(module
  (rec
    ;; NO_TNH:      (rec
    ;; NO_TNH-NEXT:  (type $A (sub (struct)))
    (type $A (sub (struct)))

    (type $B (sub $A (struct)))

    ;; YESTNH:      (rec
    ;; YESTNH-NEXT:  (type $B1 (sub (struct)))
    ;; NO_TNH:       (type $B1 (sub $A (struct)))
    (type $B1 (sub $A (struct))) ;; this is a new type
  )

  ;; YESTNH:       (type $1 (func (param anyref)))

  ;; YESTNH:      (func $new (type $1) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $B1)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:       (type $2 (func (param anyref)))

  ;; NO_TNH:      (func $new (type $2) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $B1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $B1)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $1) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $2) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B1)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    (drop
      (ref.cast (ref $A)     ;; This will be optimized to $B1.
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)     ;; $B is never created, so this will trap, in both TNH
        (local.get $x) ;; and non-TNH modes.
      )
    )
    (drop
      (ref.cast (ref $B1)
        (local.get $x)
      )
    )
  )
)

;; A chain, $A :> $B :> $C, where we can optimize $A all the way to $C.
(module
  ;; NO_TNH:      (rec
  ;; NO_TNH-NEXT:  (type $A (sub (struct)))
  (type $A (sub (struct)))

  ;; NO_TNH:       (type $B (sub $A (struct)))
  (type $B (sub $A (struct)))

  ;; YESTNH:      (rec
  ;; YESTNH-NEXT:  (type $C (sub (struct)))
  ;; NO_TNH:       (type $C (sub $B (struct)))
  (type $C (sub $B (struct)))

  ;; YESTNH:       (type $1 (func (param anyref)))

  ;; YESTNH:      (func $new (type $1) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (struct.new_default $C)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:       (type $3 (func (param anyref)))

  ;; NO_TNH:      (func $new (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.new_default $C)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (struct.new $C)
    )
  )

  ;; YESTNH:      (func $ref.cast (type $1) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $C)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    (drop
      (ref.cast (ref $A)     ;; This can be $C.
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)     ;; This can also be $C.
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C)
        (local.get $x)
      )
    )
  )
)

;; More testing for cases where no types or subtypes are created. No type is
;; created here. No type needs to be emitted in the output.
(module
  (rec
    (type $A (sub (struct)))

    (type $B (sub $A (struct)))

    (type $C1 (sub $B (struct)))

    (type $C2 (sub $B (struct)))
  )

  ;; YESTNH:      (rec
  ;; YESTNH-NEXT:  (type $0 (func (param anyref)))

  ;; YESTNH:       (type $1 (func))

  ;; YESTNH:      (func $ref.cast (type $0) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (rec
  ;; NO_TNH-NEXT:  (type $0 (func (param anyref)))

  ;; NO_TNH:       (type $1 (func))

  ;; NO_TNH:      (func $ref.cast (type $0) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    ;; All these will trap.
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C1)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C2)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.cast.null (type $0) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast nullref
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast nullref
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast nullref
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast nullref
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast.null (type $0) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast nullref
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast nullref
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast nullref
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast nullref
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast.null (param $x anyref)
    ;; These can only pass through a null.
    (drop
      (ref.cast (ref null $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref null $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref null $C1)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref null $C2)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.test (type $0) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.test (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.test nullref
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.test (type $0) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.test (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.test nullref
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.test (param $x anyref)
    ;; This will return 0.
    (drop
      (ref.test (ref $A)
        (local.get $x)
      )
    )
    ;; This can test for a null.
    (drop
      (ref.test (ref null $A)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $br_on (type $0) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (block $block (result (ref none))
  ;; YESTNH-NEXT:    (drop
  ;; YESTNH-NEXT:     (br_on_cast $block anyref (ref none)
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
  ;; NO_TNH:      (func $br_on (type $0) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block $block (result (ref none))
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (br_on_cast $block anyref (ref none)
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
          (br_on_cast $block anyref (ref $B)
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

  ;; YESTNH:      (func $locals (type $1)
  ;; YESTNH-NEXT:  (local $A (ref none))
  ;; YESTNH-NEXT:  (local $B (ref none))
  ;; YESTNH-NEXT:  (local $C1 (ref none))
  ;; YESTNH-NEXT:  (local $C2 nullref)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $locals (type $1)
  ;; NO_TNH-NEXT:  (local $A (ref none))
  ;; NO_TNH-NEXT:  (local $B (ref none))
  ;; NO_TNH-NEXT:  (local $C1 (ref none))
  ;; NO_TNH-NEXT:  (local $C2 nullref)
  ;; NO_TNH-NEXT:  (nop)
  ;; NO_TNH-NEXT: )
  (func $locals
    ;; All these locals can become nullable or even non-nullable null types.
    ;; This checks no problem happens due to that.
    (local $A (ref $A))
    (local $B (ref $B))
    (local $C1 (ref $C1))
    (local $C2 (ref null $C2))
  )
)

;; As above, but now $C1 is created.
(module
  (rec
    ;; NO_TNH:      (rec
    ;; NO_TNH-NEXT:  (type $A (sub (struct)))
    (type $A (sub (struct)))

    ;; NO_TNH:       (type $B (sub $A (struct)))
    (type $B (sub $A (struct)))

    ;; YESTNH:      (rec
    ;; YESTNH-NEXT:  (type $C1 (sub (struct)))
    ;; NO_TNH:       (type $C1 (sub $B (struct)))
    (type $C1 (sub $B (struct)))

    (type $C2 (sub $B (struct)))
  )

  ;; YESTNH:       (type $1 (func (param anyref)))

  ;; YESTNH:      (global $global anyref (struct.new_default $C1))
  ;; NO_TNH:       (type $3 (func (param anyref)))

  ;; NO_TNH:      (global $global anyref (struct.new_default $C1))
  (global $global anyref (struct.new $C1))

  ;; YESTNH:      (func $ref.cast (type $1) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref none)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $C1)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast (param $x anyref)
    ;; These three can be cast to $C1 in TNH.
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C1)
        (local.get $x)
      )
    )
    ;; This will trap.
    (drop
      (ref.cast (ref $C2)
        (local.get $x)
      )
    )
  )

  ;; YESTNH:      (func $ref.cast.null (type $1) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref null $C1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref null $C1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref null $C1)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast nullref
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $ref.cast.null (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref null $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref null $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref null $C1)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast nullref
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.cast.null (param $x anyref)
    ;; These three can be cast to $C1 in TNH.
    (drop
      (ref.cast (ref null $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref null $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref null $C1)
        (local.get $x)
      )
    )
    ;; This returns null.
    (drop
      (ref.cast (ref null $C2)
        (local.get $x)
      )
    )
  )
)

;; Function subtyping, which is a TODO - for now we do nothing.
(module
  ;; YESTNH:      (rec
  ;; YESTNH-NEXT:  (type $A (sub (func)))
  ;; NO_TNH:      (rec
  ;; NO_TNH-NEXT:  (type $A (sub (func)))
  (type $A (sub (func)))

  ;; YESTNH:       (type $B (sub $A (func)))
  ;; NO_TNH:       (type $B (sub $A (func)))
  (type $B (sub $A (func)))

  ;; YESTNH:       (type $C (sub $B (func)))
  ;; NO_TNH:       (type $C (sub $B (func)))
  (type $C (sub $B (func)))

  ;; YESTNH:       (type $3 (func (param funcref)))

  ;; YESTNH:      (func $A (type $A)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:       (type $3 (func (param funcref)))

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

  ;; YESTNH:      (func $casts (type $3) (param $x funcref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $A)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $casts (type $3) (param $x funcref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $C)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $casts (param $x funcref)
    ;; $A and $C have functions of their types, so in theory we could optimize
    ;; $B here.
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C)
        (local.get $x)
      )
    )
  )
)

;; As above, but now the functions are also public types (exported). We should
;; be careful here in the future even when we do optimize function types.
(module
  ;; YESTNH:      (type $A (sub (func)))
  ;; NO_TNH:      (type $A (sub (func)))
  (type $A (sub (func)))

  ;; YESTNH:      (rec
  ;; YESTNH-NEXT:  (type $B (sub $A (func)))
  ;; NO_TNH:      (rec
  ;; NO_TNH-NEXT:  (type $B (sub $A (func)))
  (type $B (sub $A (func)))

  ;; YESTNH:       (type $C (sub $B (func)))
  ;; NO_TNH:       (type $C (sub $B (func)))
  (type $C (sub $B (func)))

  ;; YESTNH:       (type $3 (func (param funcref)))

  ;; YESTNH:      (elem declare func $A $C)

  ;; YESTNH:      (export "A" (func $A))

  ;; YESTNH:      (func $A (type $A)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.func $A)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:       (type $3 (func (param funcref)))

  ;; NO_TNH:      (elem declare func $A $C)

  ;; NO_TNH:      (export "A" (func $A))

  ;; NO_TNH:      (func $A (type $A)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.func $A)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $A (export "A") (type $A)
    ;; Also create a function reference to use the type in that way as well.
    (drop
      (ref.func $A)
    )
  )

  ;; YESTNH:      (func $C (type $C)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.func $C)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $C (type $C)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.func $C)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $C (type $C)
    (drop
      (ref.func $C)
    )
  )

  ;; YESTNH:      (func $casts (type $3) (param $x funcref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $A)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $casts (type $3) (param $x funcref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $C)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $casts (param $x funcref)
    ;; $A and $C have functions of their types, and references to them, so in
    ;; theory we could optimize $B here.
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C)
        (local.get $x)
      )
    )
  )
)

;; Array subtyping, which is a TODO - for now we do nothing.
(module
  ;; YESTNH:      (rec
  ;; YESTNH-NEXT:  (type $A (sub (array (mut i32))))
  ;; NO_TNH:      (rec
  ;; NO_TNH-NEXT:  (type $A (sub (array (mut i32))))
  (type $A (sub (array (mut i32))))

  ;; YESTNH:       (type $B (sub $A (array (mut i32))))
  ;; NO_TNH:       (type $B (sub $A (array (mut i32))))
  (type $B (sub $A (array (mut i32))))

  ;; YESTNH:       (type $C (sub $B (array (mut i32))))
  ;; NO_TNH:       (type $C (sub $B (array (mut i32))))
  (type $C (sub $B (array (mut i32))))

  ;; YESTNH:       (type $3 (func (param anyref)))

  ;; YESTNH:      (global $A (ref $A) (array.new $A
  ;; YESTNH-NEXT:  (i32.const 10)
  ;; YESTNH-NEXT:  (i32.const 20)
  ;; YESTNH-NEXT: ))
  ;; NO_TNH:       (type $3 (func (param anyref)))

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

  ;; YESTNH:      (func $casts (type $3) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $A)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $B)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $C)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $casts (type $3) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $B)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $C)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $casts (param $x anyref)
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $B)
        (local.get $x)
      )
    )
    (drop
      (ref.cast (ref $C)
        (local.get $x)
      )
    )
  )
)

;; $A is never created, but $B is, so all appearances of $A, like in the cast
;; and the struct field, could be replaced by $B, except that $A is a public type,
;; so might be created outside the module.
(module
  (rec
    ;; YESTNH:      (rec
    ;; YESTNH-NEXT:  (type $A (sub (struct (field (ref null $A)))))
    ;; NO_TNH:      (rec
    ;; NO_TNH-NEXT:  (type $A (sub (struct (field (ref null $A)))))
    (type $A (sub (struct (field (ref null $A)))))

    ;; YESTNH:       (type $B (sub $A (struct (field (ref null $A)))))
    ;; NO_TNH:       (type $B (sub $A (struct (field (ref null $A)))))
    (type $B (sub $A (struct (field (ref null $A)))))
  )

  ;; YESTNH:      (type $2 (func (param anyref)))

  ;; YESTNH:      (global $global (ref $B) (struct.new_default $B))
  ;; NO_TNH:      (type $2 (func (param anyref)))

  ;; NO_TNH:      (global $global (ref $B) (struct.new_default $B))
  (global $global (ref $B) (struct.new_default $B))

  ;; YESTNH:      (export "global" (global $global))
  ;; NO_TNH:      (export "global" (global $global))
  (export "global" (global $global))

  ;; YESTNH:      (func $new (type $2) (param $x anyref)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (ref.cast (ref $A)
  ;; YESTNH-NEXT:    (local.get $x)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $new (type $2) (param $x anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $A)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $new (param $x anyref)
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
  )
)

