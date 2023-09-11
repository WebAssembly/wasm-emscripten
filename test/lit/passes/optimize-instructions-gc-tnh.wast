;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-instructions --traps-never-happen -all -S -o - | filecheck %s --check-prefix TNH
;; RUN: wasm-opt %s --optimize-instructions                      -all -S -o - | filecheck %s --check-prefix NO_TNH

(module
  ;; TNH:      (type $struct (sub (struct (field (mut i32)))))
  ;; NO_TNH:      (type $struct (sub (struct (field (mut i32)))))
  (type $struct (struct_subtype (field (mut i32)) data))

  ;; TNH:      (type $void (func))
  ;; NO_TNH:      (type $void (func))
  (type $void (func))

  ;; TNH:      (import "a" "b" (func $import (type $2) (result i32)))
  ;; NO_TNH:      (import "a" "b" (func $import (type $2) (result i32)))
  (import "a" "b" (func $import (result i32)))

  ;; TNH:      (func $ref.eq (type $7) (param $a eqref) (param $b eqref) (result i32)
  ;; TNH-NEXT:  (ref.eq
  ;; TNH-NEXT:   (local.get $a)
  ;; TNH-NEXT:   (local.get $b)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.eq (type $7) (param $a eqref) (param $b eqref) (result i32)
  ;; NO_TNH-NEXT:  (ref.eq
  ;; NO_TNH-NEXT:   (ref.cast (ref $struct)
  ;; NO_TNH-NEXT:    (local.get $a)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (ref.cast (ref struct)
  ;; NO_TNH-NEXT:    (local.get $b)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.eq (param $a (ref null eq)) (param $b (ref null eq)) (result i32)
    ;; When traps never happen we can remove all the casts here, since they do
    ;; not affect the comparison of the references.
    (ref.eq
      ;; When traps can happen we can still improve this by removing and
      ;; combining redundant casts.
      (ref.cast (ref struct)
        (ref.as_non_null
          (ref.cast (ref null $struct)
            (local.get $a)
          )
        )
      )
      ;; Note that we can remove the non-null casts here in both modes, as the
      ;; ref.cast struct also checks for null.
      (ref.cast (ref struct)
        (ref.as_non_null
          (ref.as_non_null
            (local.get $b)
          )
        )
      )
    )
  )

  ;; TNH:      (func $ref.eq-no (type $8) (param $a eqref) (param $b eqref) (param $any anyref)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.eq-no (type $8) (param $a eqref) (param $b eqref) (param $any anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.eq
  ;; NO_TNH-NEXT:    (ref.cast (ref null $struct)
  ;; NO_TNH-NEXT:     (local.get $any)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (ref.cast (ref struct)
  ;; NO_TNH-NEXT:     (local.get $any)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.eq-no (param $a (ref null eq)) (param $b (ref null eq)) (param $any anyref)
    ;; We must leave the inputs to ref.eq of type eqref or a subtype.
    (drop
      (ref.eq
        (ref.cast (ref null $struct)
          (local.get $any) ;; *Not* an eqref!
        )
        (ref.as_non_null
          (ref.cast (ref struct)
            (ref.as_non_null
              (local.get $any) ;; *Not* an eqref!
            )
          )
        )
      )
    )
  )

  ;; TNH:      (func $ref.is (type $3) (param $a eqref) (result i32)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.cast (ref $struct)
  ;; TNH-NEXT:    (local.get $a)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (i32.const 0)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.is (type $3) (param $a eqref) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast (ref $struct)
  ;; NO_TNH-NEXT:    (local.get $a)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (i32.const 0)
  ;; NO_TNH-NEXT: )
  (func $ref.is (param $a (ref null eq)) (result i32)
    ;; In this case non-nullability is enough to tell that the ref.is will
    ;; return 0. TNH does not help here.
    (ref.is_null
      (ref.cast (ref $struct)
        (ref.as_non_null
          (ref.cast (ref struct)
            (local.get $a)
          )
        )
      )
    )
  )

  ;; TNH:      (func $ref.is_b (type $9) (param $a eqref) (param $f funcref) (result i32)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.is_null
  ;; TNH-NEXT:    (local.get $a)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (ref.is_null
  ;; TNH-NEXT:   (local.get $f)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.is_b (type $9) (param $a eqref) (param $f funcref) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.is_null
  ;; NO_TNH-NEXT:    (ref.cast (ref null $struct)
  ;; NO_TNH-NEXT:     (local.get $a)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (ref.is_null
  ;; NO_TNH-NEXT:   (ref.cast (ref null $void)
  ;; NO_TNH-NEXT:    (local.get $f)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.is_b (param $a eqref) (param $f funcref) (result i32)
    ;; Here we only have a cast, and no cast operations that force the value
    ;; to be non-nullable. That means we cannot remove the ref.is, but we can
    ;; remove the cast in TNH.
    (drop
      (ref.is_null
        (ref.cast (ref null $struct)
          (local.get $a)
        )
      )
    )
    ;; It works on func references, too.
    (ref.is_null
      (ref.cast (ref null $void)
        (local.get $f)
      )
    )
  )

  ;; TNH:      (func $ref.test (type $3) (param $a eqref) (result i32)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block (result i32)
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (ref.cast i31ref
  ;; TNH-NEXT:      (local.get $a)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (i32.const 1)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (block (result i32)
  ;; TNH-NEXT:   (drop
  ;; TNH-NEXT:    (ref.as_non_null
  ;; TNH-NEXT:     (local.get $a)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.test (type $3) (param $a eqref) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block (result i32)
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (ref.cast i31ref
  ;; NO_TNH-NEXT:      (local.get $a)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.const 1)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (block (result i32)
  ;; NO_TNH-NEXT:   (drop
  ;; NO_TNH-NEXT:    (ref.as_non_null
  ;; NO_TNH-NEXT:     (local.get $a)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.test (param $a eqref) (result i32)
    (drop
      (ref.test i31ref
        (ref.cast i31ref
          (local.get $a)
        )
      )
    )
    (ref.test (ref eq)
      (ref.cast (ref eq)
        (local.get $a)
      )
    )
  )

  ;; TNH:      (func $if.arm.null (type $4) (param $x i32) (param $ref (ref $struct))
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (block (result (ref $struct))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (local.get $x)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (local.get $ref)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (block (result (ref $struct))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (local.get $x)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (local.get $ref)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 2)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $if.arm.null (type $4) (param $x i32) (param $ref (ref $struct))
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (if (result (ref null $struct))
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:    (local.get $ref)
  ;; NO_TNH-NEXT:    (ref.null none)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (if (result (ref null $struct))
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:    (ref.null none)
  ;; NO_TNH-NEXT:    (local.get $ref)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 2)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $if.arm.null (param $x i32) (param $ref (ref $struct))
    ;; A set will trap on a null, so in tnh mode we know the null arm is not
    ;; executed, and the other one is.
    (struct.set $struct 0
      (if (result (ref null $struct))
        (local.get $x)
        (local.get $ref)
        (ref.null none)
      )
      (i32.const 1)
    )
    (struct.set $struct 0
      (if (result (ref null $struct))
        (local.get $x)
        (ref.null none)
        (local.get $ref)
      )
      (i32.const 2)
    )
  )

  ;; TNH:      (func $select.arm.null (type $4) (param $x i32) (param $ref (ref $struct))
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (block (result (ref $struct))
  ;; TNH-NEXT:    (block
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (ref.null none)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (local.get $x)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (local.get $ref)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (block (result (ref $struct))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (ref.null none)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (block (result (ref $struct))
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (local.get $x)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (local.get $ref)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 2)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $select.arm.null (type $4) (param $x i32) (param $ref (ref $struct))
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (select (result (ref null $struct))
  ;; NO_TNH-NEXT:    (local.get $ref)
  ;; NO_TNH-NEXT:    (ref.null none)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (select (result (ref null $struct))
  ;; NO_TNH-NEXT:    (ref.null none)
  ;; NO_TNH-NEXT:    (local.get $ref)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 2)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $select.arm.null (param $x i32) (param $ref (ref $struct))
    ;; As above but with a select.
    (struct.set $struct 0
      (select (result (ref null $struct))
        (local.get $ref)
        (ref.null none)
        (local.get $x)
      )
      (i32.const 1)
    )
    (struct.set $struct 0
      (select (result (ref null $struct))
        (ref.null none)
        (local.get $ref)
        (local.get $x)
      )
      (i32.const 2)
    )
  )

  ;; TNH:      (func $select.arm.null.effects (type $void)
  ;; TNH-NEXT:  (local $temp i32)
  ;; TNH-NEXT:  (local $1 (ref $struct))
  ;; TNH-NEXT:  (local $2 (ref $struct))
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (block (result (ref $struct))
  ;; TNH-NEXT:    (local.set $1
  ;; TNH-NEXT:     (struct.new $struct
  ;; TNH-NEXT:      (local.tee $temp
  ;; TNH-NEXT:       (i32.const 1)
  ;; TNH-NEXT:      )
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (block
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (block (result nullref)
  ;; TNH-NEXT:       (local.set $temp
  ;; TNH-NEXT:        (i32.const 2)
  ;; TNH-NEXT:       )
  ;; TNH-NEXT:       (ref.null none)
  ;; TNH-NEXT:      )
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (local.get $temp)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (local.get $1)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (block (result (ref $struct))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (block (result nullref)
  ;; TNH-NEXT:      (local.set $temp
  ;; TNH-NEXT:       (i32.const 2)
  ;; TNH-NEXT:      )
  ;; TNH-NEXT:      (ref.null none)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (block (result (ref $struct))
  ;; TNH-NEXT:     (local.set $2
  ;; TNH-NEXT:      (struct.new $struct
  ;; TNH-NEXT:       (local.tee $temp
  ;; TNH-NEXT:        (i32.const 1)
  ;; TNH-NEXT:       )
  ;; TNH-NEXT:      )
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (local.get $temp)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (local.get $2)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 2)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $select.arm.null.effects (type $void)
  ;; NO_TNH-NEXT:  (local $temp i32)
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (select (result (ref null $struct))
  ;; NO_TNH-NEXT:    (struct.new $struct
  ;; NO_TNH-NEXT:     (local.tee $temp
  ;; NO_TNH-NEXT:      (i32.const 1)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (block (result nullref)
  ;; NO_TNH-NEXT:     (local.set $temp
  ;; NO_TNH-NEXT:      (i32.const 2)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:     (ref.null none)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (local.get $temp)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (select (result (ref null $struct))
  ;; NO_TNH-NEXT:    (block (result nullref)
  ;; NO_TNH-NEXT:     (local.set $temp
  ;; NO_TNH-NEXT:      (i32.const 2)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:     (ref.null none)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (struct.new $struct
  ;; NO_TNH-NEXT:     (local.tee $temp
  ;; NO_TNH-NEXT:      (i32.const 1)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (local.get $temp)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 2)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $select.arm.null.effects
    (local $temp i32)
    ;; As above but there are conflicting effects and we must add a local when
    ;; we optimize.
    (struct.set $struct 0
      (select (result (ref null $struct))
        (struct.new $struct
          (local.tee $temp
            (i32.const 1)
          )
        )
        (block (result (ref null none))
          (local.set $temp
            (i32.const 2)
          )
          (ref.null none)
        )
        (local.get $temp)
      )
      (i32.const 1)
    )
    (struct.set $struct 0
      (select (result (ref null $struct))
        (block (result (ref null none))
          (local.set $temp
            (i32.const 2)
          )
          (ref.null none)
        )
        (struct.new $struct
          (local.tee $temp
            (i32.const 1)
          )
        )
        (local.get $temp)
      )
      (i32.const 2)
    )
  )

  ;; TNH:      (func $null.arm.null.effects (type $void)
  ;; TNH-NEXT:  (block ;; (replaces something unreachable we can't emit)
  ;; TNH-NEXT:   (drop
  ;; TNH-NEXT:    (select
  ;; TNH-NEXT:     (unreachable)
  ;; TNH-NEXT:     (ref.null none)
  ;; TNH-NEXT:     (call $get-i32)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (drop
  ;; TNH-NEXT:    (i32.const 1)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (unreachable)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (block
  ;; TNH-NEXT:   (drop
  ;; TNH-NEXT:    (block (result nullref)
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (call $get-i32)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (ref.null none)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (unreachable)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $null.arm.null.effects (type $void)
  ;; NO_TNH-NEXT:  (block ;; (replaces something unreachable we can't emit)
  ;; NO_TNH-NEXT:   (drop
  ;; NO_TNH-NEXT:    (select
  ;; NO_TNH-NEXT:     (unreachable)
  ;; NO_TNH-NEXT:     (ref.null none)
  ;; NO_TNH-NEXT:     (call $get-i32)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (drop
  ;; NO_TNH-NEXT:    (i32.const 1)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (block
  ;; NO_TNH-NEXT:   (drop
  ;; NO_TNH-NEXT:    (block (result nullref)
  ;; NO_TNH-NEXT:     (drop
  ;; NO_TNH-NEXT:      (call $get-i32)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:     (ref.null none)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $null.arm.null.effects
    ;; Verify we do not error on a null reference in a select, even if cast to
    ;; non-null.
    (struct.set $struct 0
      (select (result (ref null $struct))
        (ref.as_non_null
          (ref.null none)
        )
        (ref.null none)
        (call $get-i32)
      )
      (i32.const 1)
    )
    ;; The same, but without ref.as_non_null.
    (struct.set $struct 0
      (select (result (ref null $struct))
        (ref.null none)
        (ref.null none)
        (call $get-i32)
      )
      (i32.const 1)
    )
  )

  ;; TNH:      (func $set-get-cast (type $10) (param $ref structref)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (struct.get $struct 0
  ;; TNH-NEXT:    (ref.cast (ref $struct)
  ;; TNH-NEXT:     (local.get $ref)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (ref.cast (ref $struct)
  ;; TNH-NEXT:    (local.get $ref)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (ref.cast (ref null $struct)
  ;; TNH-NEXT:    (local.get $ref)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (block (result i32)
  ;; TNH-NEXT:    (return)
  ;; TNH-NEXT:    (i32.const 1)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $set-get-cast (type $10) (param $ref structref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.get $struct 0
  ;; NO_TNH-NEXT:    (ref.cast (ref $struct)
  ;; NO_TNH-NEXT:     (local.get $ref)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (ref.cast (ref null $struct)
  ;; NO_TNH-NEXT:    (local.get $ref)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (ref.cast (ref null $struct)
  ;; NO_TNH-NEXT:    (local.get $ref)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (block (result i32)
  ;; NO_TNH-NEXT:    (return)
  ;; NO_TNH-NEXT:    (i32.const 1)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $set-get-cast (param $ref (ref null struct))
    ;; A nullable cast flowing into a place that traps on null can become a
    ;; non-nullable cast.
    (drop
      (struct.get $struct 0
        (ref.cast (ref null $struct)
          (local.get $ref)
        )
      )
    )
    ;; Ditto for a set, at least in traps-happen mode.
    ;; TODO handle non-TNH as well, but we need to be careful of effects in
    ;;      other children.
    (struct.set $struct 0
      (ref.cast (ref null $struct)
        (local.get $ref)
      )
      (i32.const 1)
    )
    ;; Even in TNH mode, a child with an effect of control flow transfer
    ;; prevents us from optimizing - if the parent is not necessarily reached,
    ;; we cannot infer the child won't trap.
    (struct.set $struct 0
      (ref.cast (ref null $struct)
        (local.get $ref)
      )
      (block (result i32)
        ;; This block has type i32, to check that we don't just look for
        ;; unreachable. We must scan for any transfer of control flow in the
        ;; child of the struct.set.
        (return)
        (i32.const 1)
      )
    )
  )

  ;; TNH:      (func $cast-if-null (type $5) (param $x (ref none)) (result (ref $struct))
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (i32.const 1)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (unreachable)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $cast-if-null (type $5) (param $x (ref none)) (result (ref $struct))
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (if (result (ref none))
  ;; NO_TNH-NEXT:    (i32.const 1)
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $cast-if-null (param $x (ref none)) (result (ref $struct))
    ;; We can remove the unreachable arm of the if here in TNH mode. While doing
    ;; so we must refinalize properly or else we'll hit an error in pass-debug
    ;; mode.
    (ref.cast (ref $struct)
      (if (result (ref none))
        (i32.const 1)
        (unreachable)
        (local.get $x)
      )
    )
  )

  ;; TNH:      (func $cast-if-null-flip (type $5) (param $x (ref none)) (result (ref $struct))
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (i32.const 1)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (unreachable)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $cast-if-null-flip (type $5) (param $x (ref none)) (result (ref $struct))
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (if (result (ref none))
  ;; NO_TNH-NEXT:    (i32.const 1)
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $cast-if-null-flip (param $x (ref none)) (result (ref $struct))
    ;; As above but with arms flipped.
    (ref.cast (ref $struct)
      (if (result (ref none))
        (i32.const 1)
        (local.get $x)
        (unreachable)
      )
    )
  )

  ;; TNH:      (func $cast-to-bottom (type $11) (param $ref (ref any)) (param $nullable-ref anyref)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block (result (ref none))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (local.get $ref)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block (result (ref none))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (local.get $nullable-ref)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block (result (ref none))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (local.get $ref)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block (result nullref)
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (local.get $nullable-ref)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (ref.null none)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $cast-to-bottom (type $11) (param $ref (ref any)) (param $nullable-ref anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block (result (ref none))
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (local.get $ref)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block (result (ref none))
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (local.get $nullable-ref)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block (result (ref none))
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (local.get $ref)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast nullref
  ;; NO_TNH-NEXT:    (local.get $nullable-ref)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $cast-to-bottom (param $ref (ref any)) (param $nullable-ref anyref)
    ;; Non-nullable casts to none must trap (regardless of whether the input is
    ;; nullable or not, the output is an impossible type).
    (drop
      (ref.cast (ref none)
        (local.get $ref)
      )
    )
    (drop
      (ref.cast (ref none)
        (local.get $nullable-ref)
      )
    )
    ;; Nullable casts to null have more possibilities. First, if the input is
    ;; non-nullable then we trap.
    (drop
      (ref.cast nullref
        (local.get $ref)
      )
    )
    ;; Second, if the value may be a null, then we either return a null or we
    ;; trap. In TNH mode we dismiss the possibility of a trap and so we can just
    ;; return a null here. (In non-TNH mode we could do a check for null etc.,
    ;; but we'd be increasing code size.)
    (drop
      (ref.cast nullref
        (local.get $nullable-ref)
      )
    )
  )

  ;; TNH:      (func $null.cast-other.effects (type $12) (param $x (ref null $struct))
  ;; TNH-NEXT:  (local $i i32)
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (local.get $x)
  ;; TNH-NEXT:   (call $import)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (local.get $x)
  ;; TNH-NEXT:   (i32.const 10)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (struct.set $struct 0
  ;; TNH-NEXT:   (local.get $x)
  ;; TNH-NEXT:   (local.tee $i
  ;; TNH-NEXT:    (i32.const 10)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $null.cast-other.effects (type $12) (param $x (ref null $struct))
  ;; NO_TNH-NEXT:  (local $i i32)
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (ref.as_non_null
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (call $import)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (local.get $x)
  ;; NO_TNH-NEXT:   (i32.const 10)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (struct.set $struct 0
  ;; NO_TNH-NEXT:   (local.get $x)
  ;; NO_TNH-NEXT:   (local.tee $i
  ;; NO_TNH-NEXT:    (i32.const 10)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $null.cast-other.effects (param $x (ref null $struct))
    (local $i i32)
    (struct.set $struct 0
      ;; We cannot remove this ref.as_non_null, even though the struct.set will
      ;; trap if the ref is null, because that would move the trap from before
      ;; the call to the import to be after it. But in TNH we can assume it does
      ;; not trap, and remove it.
      (ref.as_non_null
        (local.get $x)
      )
      (call $import)
    )
    (struct.set $struct 0
      ;; This one can be removed even without TNH, as there are no effects after
      ;; it.
      (ref.as_non_null
        (local.get $x)
      )
      (i32.const 10)
    )
    (struct.set $struct 0
      ;; This one can be removed even without TNH, even though there are effects
      ;; in it. A tee only has local effects, which do not interfere with a
      ;; trap.
      (ref.as_non_null
        (local.get $x)
      )
      (local.tee $i
        (i32.const 10)
      )
    )
  )

  ;; TNH:      (func $select.unreachable.child (type $6) (param $x (ref $struct)) (result (ref $struct))
  ;; TNH-NEXT:  (block ;; (replaces something unreachable we can't emit)
  ;; TNH-NEXT:   (drop
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:   (unreachable)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $select.unreachable.child (type $6) (param $x (ref $struct)) (result (ref $struct))
  ;; NO_TNH-NEXT:  (block ;; (replaces something unreachable we can't emit)
  ;; NO_TNH-NEXT:   (drop
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $select.unreachable.child (param $x (ref $struct)) (result (ref $struct))
    ;; We will turn the false arm of the select into an unreachable first, and
    ;; then process the select. While doing so we must not error, as the select
    ;; itself will still have a reachable type (a full refinalize only
    ;; happens at the very end of the function).
    (ref.cast (ref $struct)
      (select (result (ref $struct))
        (ref.as_non_null
          (ref.null none)
        )
        (local.get $x)
        (i32.const 1)
      )
    )
  )

  ;; TNH:      (func $select.unreachable.child.flip (type $6) (param $x (ref $struct)) (result (ref $struct))
  ;; TNH-NEXT:  (select
  ;; TNH-NEXT:   (local.get $x)
  ;; TNH-NEXT:   (unreachable)
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $select.unreachable.child.flip (type $6) (param $x (ref $struct)) (result (ref $struct))
  ;; NO_TNH-NEXT:  (select
  ;; NO_TNH-NEXT:   (local.get $x)
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $select.unreachable.child.flip (param $x (ref $struct)) (result (ref $struct))
    ;; Flip case of the above.
    (ref.cast (ref $struct)
      (select (result (ref $struct))
        (local.get $x)
        (ref.as_non_null
          (ref.null none)
        )
        (i32.const 1)
      )
    )
  )

  ;; TNH:      (func $if.null.child.but.no.flow (type $void)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (block (result (ref nofunc))
  ;; TNH-NEXT:    (drop
  ;; TNH-NEXT:     (if (result (ref nofunc))
  ;; TNH-NEXT:      (i32.const 1)
  ;; TNH-NEXT:      (return)
  ;; TNH-NEXT:      (unreachable)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (unreachable)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $if.null.child.but.no.flow (type $void)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block (result (ref nofunc))
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (if (result (ref nofunc))
  ;; NO_TNH-NEXT:      (i32.const 1)
  ;; NO_TNH-NEXT:      (return)
  ;; NO_TNH-NEXT:      (unreachable)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $if.null.child.but.no.flow
    ;; The if's true arm has a bottom type, which the cast would trap on. But we
    ;; cannot optimize using that fact, as the null does not actually flow out -
    ;; we return from the function before. So we should not replace the if with
    ;; the false arm (that would trap, and change the behavior; tnh can remove
    ;; traps, not add them).
    (drop
      (ref.cast (ref func)
        (if (result (ref nofunc))
          (i32.const 1)
          (block (result (ref nofunc))
            (return)
          )
          (unreachable)
        )
      )
    )
  )

  ;; TNH:      (func $set-of-as-non-null (type $void)
  ;; TNH-NEXT:  (local $x anyref)
  ;; TNH-NEXT:  (local.set $x
  ;; TNH-NEXT:   (local.get $x)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.as_non_null
  ;; TNH-NEXT:    (local.tee $x
  ;; TNH-NEXT:     (local.get $x)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $set-of-as-non-null (type $void)
  ;; NO_TNH-NEXT:  (local $x anyref)
  ;; NO_TNH-NEXT:  (local.set $x
  ;; NO_TNH-NEXT:   (ref.as_non_null
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.as_non_null
  ;; NO_TNH-NEXT:    (local.tee $x
  ;; NO_TNH-NEXT:     (local.get $x)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $set-of-as-non-null
    (local $x anyref)
    ;; We can remove the ref.as_non_null here because the local is nullable and
    ;; we are ignoring traps.
    ;; TODO: Should we keep the cast to let us refine the local later?
    (local.set $x
      (ref.as_non_null
        (local.get $x)
      )
    )
    ;; The same for a tee.
    (drop
      (local.tee $x
        (ref.as_non_null
          (local.get $x)
        )
      )
    )
  )


  ;; Helper functions.

  ;; TNH:      (func $get-i32 (type $2) (result i32)
  ;; TNH-NEXT:  (unreachable)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $get-i32 (type $2) (result i32)
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $get-i32 (result i32)
    (unreachable)
  )
  ;; TNH:      (func $get-ref (type $13) (result (ref $struct))
  ;; TNH-NEXT:  (unreachable)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $get-ref (type $13) (result (ref $struct))
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $get-ref (result (ref $struct))
    (unreachable)
  )
  ;; TNH:      (func $get-null (type $14) (result nullref)
  ;; TNH-NEXT:  (unreachable)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $get-null (type $14) (result nullref)
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $get-null (result (ref null none))
    (unreachable)
  )
)
