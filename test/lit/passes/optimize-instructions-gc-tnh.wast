;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-instructions --traps-never-happen -all --nominal -S -o - | filecheck %s --check-prefix TNH
;; RUN: wasm-opt %s --optimize-instructions                      -all --nominal -S -o - | filecheck %s --check-prefix NO_TNH

(module
  ;; TNH:      (type $struct (struct_subtype  data))
  ;; NO_TNH:      (type $struct (struct_subtype  data))
  (type $struct (struct_subtype data))

  ;; TNH:      (func $ref.eq (type $eqref_eqref_=>_i32) (param $a eqref) (param $b eqref) (result i32)
  ;; TNH-NEXT:  (ref.eq
  ;; TNH-NEXT:   (local.get $a)
  ;; TNH-NEXT:   (local.get $b)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.eq (type $eqref_eqref_=>_i32) (param $a eqref) (param $b eqref) (result i32)
  ;; NO_TNH-NEXT:  (ref.eq
  ;; NO_TNH-NEXT:   (ref.as_non_null
  ;; NO_TNH-NEXT:    (ref.cast_static $struct
  ;; NO_TNH-NEXT:     (local.get $a)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (ref.as_data
  ;; NO_TNH-NEXT:    (local.get $b)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.eq (param $a (ref null eq)) (param $b (ref null eq)) (result i32)
    ;; When traps never happen we can remove all the casts here, since they do
    ;; not affect the comparison of the references.
    (ref.eq
      (ref.as_data
        (ref.as_non_null
          (ref.cast_static $struct
            (local.get $a)
          )
        )
      )
      ;; Note that we can remove the non-null casts here in both modes, as the
      ;; ref.as_data also checks for null.
      (ref.as_data
        (ref.as_non_null
          (ref.as_non_null
            (local.get $b)
          )
        )
      )
    )
  )

  ;; TNH:      (func $ref.eq-no (type $eqref_eqref_=>_none) (param $a eqref) (param $b eqref)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (i32.const 1)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.eq
  ;; TNH-NEXT:    (block (result (ref null $struct))
  ;; TNH-NEXT:     (drop
  ;; TNH-NEXT:      (ref.null any)
  ;; TNH-NEXT:     )
  ;; TNH-NEXT:     (ref.null $struct)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:    (ref.as_data
  ;; TNH-NEXT:     (ref.null any)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.eq-no (type $eqref_eqref_=>_none) (param $a eqref) (param $b eqref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.eq
  ;; NO_TNH-NEXT:    (block (result (ref $struct))
  ;; NO_TNH-NEXT:     (drop
  ;; NO_TNH-NEXT:      (ref.func $ref.eq-no)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:     (unreachable)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (block (result (ref data))
  ;; NO_TNH-NEXT:     (drop
  ;; NO_TNH-NEXT:      (ref.func $ref.eq-no)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:     (unreachable)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.eq
  ;; NO_TNH-NEXT:    (block (result (ref null $struct))
  ;; NO_TNH-NEXT:     (drop
  ;; NO_TNH-NEXT:      (ref.null any)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:     (ref.null $struct)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (ref.as_data
  ;; NO_TNH-NEXT:     (ref.null any)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.eq-no (param $a (ref null eq)) (param $b (ref null eq))
    ;; We must leave the inputs to ref.eq of type eqref or a subtype. Note that
    ;; these casts will trap, so other opts might get in the way before we can
    ;; do anything. The crucial thing we test here is that we do not emit
    ;; something that does not validate (as ref.eq inputs must be eqrefs).
    (drop
      (ref.eq
        (ref.cast_static $struct
          (ref.func $ref.eq-no) ;; *Not* an eqref!
        )
        (ref.as_non_null
          (ref.as_data
            (ref.as_non_null
              (ref.func $ref.eq-no) ;; *Not* an eqref!
            )
          )
        )
      )
    )
    ;; As above, but now with nulls of a non-eq type.
    ;; Note that we could in theory change a null's type to get validation in
    ;; such cases.
    (drop
      (ref.eq
        (ref.cast_static $struct
          (ref.null any) ;; *Not* an eqref!
        )
        (ref.as_non_null
          (ref.as_data
            (ref.as_non_null
              (ref.null any) ;; *Not* an eqref!
            )
          )
        )
      )
    )
  )

  ;; TNH:      (func $ref.is (type $eqref_=>_i32) (param $a eqref) (result i32)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.cast_static $struct
  ;; TNH-NEXT:    (ref.as_data
  ;; TNH-NEXT:     (local.get $a)
  ;; TNH-NEXT:    )
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (i32.const 0)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.is (type $eqref_=>_i32) (param $a eqref) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.cast_static $struct
  ;; NO_TNH-NEXT:    (ref.as_data
  ;; NO_TNH-NEXT:     (local.get $a)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (i32.const 0)
  ;; NO_TNH-NEXT: )
  (func $ref.is (param $a (ref null eq)) (result i32)
    ;; In this case non-nullability is enough to tell that the ref.is will
    ;; return 0. TNH does not help here.
    (ref.is_null
      (ref.cast_static $struct
        (ref.as_non_null
          (ref.as_data
            (local.get $a)
          )
        )
      )
    )
  )

  ;; TNH:      (func $ref.is_b (type $eqref_=>_i32) (param $a eqref) (result i32)
  ;; TNH-NEXT:  (ref.is_null
  ;; TNH-NEXT:   (local.get $a)
  ;; TNH-NEXT:  )
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.is_b (type $eqref_=>_i32) (param $a eqref) (result i32)
  ;; NO_TNH-NEXT:  (ref.is_null
  ;; NO_TNH-NEXT:   (ref.cast_static $struct
  ;; NO_TNH-NEXT:    (local.get $a)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $ref.is_b(param $a (ref null eq)) (result i32)
    ;; Here we only have a cast, and no ref.as operations that force the value
    ;; to be non-nullable. That means we cannot remove the ref.is, but we can
    ;; remove the cast in TNH.
    (ref.is_null
      (ref.cast_static $struct
        (local.get $a)
      )
    )
  )

  ;; TNH:      (func $ref.is_func_a (type $anyref_=>_i32) (param $a anyref) (result i32)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.as_func
  ;; TNH-NEXT:    (local.get $a)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (i32.const 1)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.is_func_a (type $anyref_=>_i32) (param $a anyref) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.as_func
  ;; NO_TNH-NEXT:    (local.get $a)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (i32.const 1)
  ;; NO_TNH-NEXT: )
  (func $ref.is_func_a (param $a (ref null any)) (result i32)
    ;; The check must succeed. We can return 1 here, and drop the rest, with or
    ;; without TNH (in particular, TNH should not just remove the cast but not
    ;; return a 1).
    (ref.is_func
      (ref.as_func
        (local.get $a)
      )
    )
  )

  ;; TNH:      (func $ref.is_func_b (type $anyref_=>_i32) (param $a anyref) (result i32)
  ;; TNH-NEXT:  (drop
  ;; TNH-NEXT:   (ref.as_data
  ;; TNH-NEXT:    (local.get $a)
  ;; TNH-NEXT:   )
  ;; TNH-NEXT:  )
  ;; TNH-NEXT:  (i32.const 0)
  ;; TNH-NEXT: )
  ;; NO_TNH:      (func $ref.is_func_b (type $anyref_=>_i32) (param $a anyref) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.as_data
  ;; NO_TNH-NEXT:    (local.get $a)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (i32.const 0)
  ;; NO_TNH-NEXT: )
  (func $ref.is_func_b (param $a (ref null any)) (result i32)
    ;; A case where the type cannot match, and we return 0.
    (ref.is_func
      (ref.as_data
        (local.get $a)
      )
    )
  )
)
