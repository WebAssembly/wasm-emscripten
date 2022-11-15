;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-casts -all --nominal -S -o - \
;; RUN:   | filecheck %s

(module
  ;; CHECK:      (type $A (struct_subtype  data))
  (type $A (struct_subtype data))

  ;; CHECK:      (type $B (struct_subtype  $A))
  (type $B (struct_subtype $A))

  ;; CHECK:      (func $ref.as (type $ref?|$A|_=>_none) (param $x (ref null $A))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
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
    ;; After the first ref.as, we can use the cast value in later gets, which is
    ;; more refined.
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
  ;; CHECK-NEXT:   (local.get $x)
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
    ;; As above, but the param is now non-nullable anyhow, so we should do
    ;; nothing.
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

  ;; CHECK:      (func $ref.cast (type $ref|data|_=>_none) (param $x (ref data))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast_static $A
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
  (func $ref.cast (param $x (ref data))
    ;; As $ref.as but with ref.casts: we should use the cast value after it has
    ;; been computed, in both gets.
    (drop
      (ref.cast_static $A
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

  ;; CHECK:      (func $not-past-set (type $ref|data|_=>_none) (param $x (ref data))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast_static $A
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
  (func $not-past-set (param $x (ref data))
    (drop
      (ref.cast_static $A
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

  ;; CHECK:      (func $best (type $ref|data|_=>_none) (param $x (ref data))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (local $2 (ref $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast_static $A
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $2
  ;; CHECK-NEXT:    (ref.cast_static $B
  ;; CHECK-NEXT:     (local.get $1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $best (param $x (ref data))
    (drop
      (ref.cast_static $A
        (local.get $x)
      )
    )
    ;; Here we should use $A.
    (drop
      (local.get $x)
    )
    (drop
      (ref.cast_static $B
        (local.get $x)
      )
    )
    ;; Here we should use $B, which is even better.
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $best-2 (type $ref|data|_=>_none) (param $x (ref data))
  ;; CHECK-NEXT:  (local $1 (ref $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast_static $B
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast_static $A
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $best-2 (param $x (ref data))
    ;; As above, but with the casts reversed. Now we should use $B in both
    ;; gets.
    (drop
      (ref.cast_static $B
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
    (drop
      (ref.cast_static $A
        (local.get $x)
      )
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $fallthrough (type $ref|data|_=>_none) (param $x (ref data))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast_static $A
  ;; CHECK-NEXT:     (block (result (ref data))
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $fallthrough (param $x (ref data))
    (drop
      (ref.cast_static $A
        ;; We look through the block, and optimize.
        (block (result (ref data))
          (local.get $x)
        )
      )
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $past-basic-block (type $ref|data|_=>_none) (param $x (ref data))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast_static $A
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
  (func $past-basic-block (param $x (ref data))
    (drop
      (ref.cast_static $A
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

  ;; CHECK:      (func $get (type $none_=>_ref|data|) (result (ref data))
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $get (result (ref data))
    ;; Helper for the above.
    (unreachable)
  )
)
