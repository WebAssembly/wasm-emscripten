;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-casts -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $A (struct ))
  (type $A (struct_subtype data))

  ;; CHECK:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; CHECK:      (global $a (mut i32) (i32.const 10))
  (global $a (mut i32) (i32.const 10))

  ;; CHECK:      (func $ref.as (type $ref?|$A|_=>_none) (param $x (ref null $A))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $ref.as (param $x (ref null $A))
    ;; Formerly, after the first ref.as, we can use the cast value in later gets,
    ;;  which is more refined. However, the ref.as is moved up to the first
    ;; local.get.
    (drop
      (local.get $x)
    )
    (drop
      (ref.as_non_null
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    ;; In this case we don't really need the last ref.as here, but we leave that
    ;; for later opts.
    (drop
      (ref.as_non_null
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $ref.as-no (type $ref|$A|_=>_none) (param $x (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $ref.as-no (param $x (ref $A))
    ;; As above, but the param is now non-nullable anyhow, so we should not
    ;; tee a new local variable.
    (drop
      (local.get $x)
    )
    (drop
      (ref.as_non_null
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    (drop
      (ref.as_non_null
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $ref.cast (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $ref.cast (param $x (ref struct))
    ;; As $ref.as but with ref.casts: we should use the cast value after it has
    ;; been computed, in both gets.
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $not-past-set (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $get)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $not-past-set (param $x (ref struct))
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    ;; The local.set in the middle stops us from helping the last get.
    (local.set $x
      (call $get)
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $best (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (local $2 (ref $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $a
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $a
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $2
  ;; CHECK-NEXT:    (ref.cast $B
  ;; CHECK-NEXT:     (local.get $1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $a
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $best (param $x (ref struct))
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (global.set $a
      (i32.const 30)
    )
    ;; Here we should use $A.
    (drop
      (local.get $x)
    )
    (global.set $a
      (i32.const 30)
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (global.set $a
      (i32.const 30)
    )
    ;; Here we should use $B, which is even better.
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $best-2 (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast $B
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $B
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $best-2 (param $x (ref struct))
    ;; As above, but with the casts reversed. Now we should use $B in both
    ;; gets.
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $fallthrough (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (block (result (ref struct))
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $fallthrough (param $x (ref struct))
    (drop
      (ref.cast $A
        ;; We look through the block, and optimize.
        (block (result (ref struct))
          (local.get $x)
        )
      )
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $past-basic-block (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $A
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (return)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $past-basic-block (param $x (ref struct))
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    ;; The if means the later get is in another basic block. We do not handle
    ;; this atm.
    (if
      (i32.const 0)
      (return)
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $multiple (type $ref|struct|_ref|struct|_=>_none) (param $x (ref struct)) (param $y (ref struct))
  ;; CHECK-NEXT:  (local $a (ref struct))
  ;; CHECK-NEXT:  (local $b (ref struct))
  ;; CHECK-NEXT:  (local $4 (ref $A))
  ;; CHECK-NEXT:  (local $5 (ref $A))
  ;; CHECK-NEXT:  (local.set $a
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $b
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $4
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (local.get $a)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $5
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (local.get $b)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $b
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $b)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $multiple (param $x (ref struct)) (param $y (ref struct))
    (local $a (ref struct))
    (local $b (ref struct))
    ;; Two different locals, with overlapping lives.
    (local.set $a
      (local.get $x)
    )
    (local.set $b
      (local.get $y)
    )
    (drop
      (ref.cast $A
        (local.get $a)
      )
    )
    (drop
      (ref.cast $A
        (local.get $b)
      )
    )
    ;; These two can be optimized.
    (drop
      (local.get $a)
    )
    (drop
      (local.get $b)
    )
    (local.set $b
      (local.get $x)
    )
    ;; Now only the first can be, since $b changed.
    (drop
      (local.get $a)
    )
    (drop
      (local.get $b)
    )
  )

  ;; CHECK:      (func $testMoveCast (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $B))
  ;; CHECK-NEXT:  (local $2 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $B
  ;; CHECK-NEXT:    (local.tee $1
  ;; CHECK-NEXT:     (ref.cast $B
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $B
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $get)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $get)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.tee $2
  ;; CHECK-NEXT:     (ref.cast $A
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $A
  ;; CHECK-NEXT:    (local.get $2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $testMoveCast (param $x (ref struct))
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    (drop
      (ref.cast $B
        (local.get $x)
      )
    )
    ;; Casts cannot be moved past local sets.
    (local.set $x
      (call $get)
    )
    (drop
      (local.get $x)
    )
    (drop
      ;; This will be moved above as the first RefAs.
      (ref.as_non_null
        (local.get $x)
      )
    )
    (local.set $x
      (call $get)
    )
    (drop
      (local.get $x)
    )
    (drop
      (ref.as_non_null
        (local.get $x)
      )
    )
    (drop
      (ref.cast null $A
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $testMoveCastSideEffects (type $ref|struct|_ref|struct|_=>_none) (param $x (ref struct)) (param $y (ref struct))
  ;; CHECK-NEXT:  (local $2 (ref $A))
  ;; CHECK-NEXT:  (local $3 (ref $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $a
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $2
  ;; CHECK-NEXT:    (ref.cast $A
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $3
  ;; CHECK-NEXT:    (ref.cast $B
  ;; CHECK-NEXT:     (local.get $y)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $A
  ;; CHECK-NEXT:    (local.get $2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $B
  ;; CHECK-NEXT:    (local.get $3)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $a
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast $B
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $testMoveCastSideEffects (param $x (ref struct)) (param $y (ref struct))
    (drop
      (local.get $x)
    )
    ;; Cannot move past global set due to trap possibility.
    (global.set $a
      (i32.const 30)
    )
    (drop
      (local.get $x)
    )
    (drop
      (local.get $y)
    )
    (drop
      (ref.cast $A
        (local.get $x)
      )
    )
    (local.set $x
      (local.get $y)
    )
    (drop
      (ref.cast $B
        (local.get $y)
      )
    )
    (global.set $a
      (i32.const 30)
    )
    (drop
      (ref.cast $B
        (ref.as_non_null
          (local.get $x)
        )
      )
    )
  )

  ;; CHECK:      (func $get (type $none_=>_ref|struct|) (result (ref struct))
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $get (result (ref struct))
    ;; Helper for the above.
    (unreachable)
  )
)
