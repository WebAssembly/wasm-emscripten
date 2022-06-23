;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-instructions --traps-never-happen -all --nominal -S -o - | filecheck %s --check-prefix TNH
;; RUN: wasm-opt %s --optimize-instructions                      -all --nominal -S -o - | filecheck %s --check-prefix NO_TNH

(module
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

  ;; TNH:      (func $ref.is (type $eqref_=>_i32) (param $a eqref) (result i32)
  ;; TNH-NEXT:  (ref.is_null
  ;; TNH-NEXT:   (local.get $a)
  ;; TNH-NEXT:  )
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
)
