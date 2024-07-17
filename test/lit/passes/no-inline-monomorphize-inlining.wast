;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; Monomorphization creates a new function, which we can then inline. When we
;; mark the original as no-inline, we should not inline the copy, as the copy
;; inherits the metadata.
;;
;; Use --optimize-level=3 to ensure inlining works at the maximum (to avoid it
;; not happening because of size limits etc.).

;; RUN: foreach %s %t wasm-opt --no-inline=*noinline* --monomorphize-always --inlining --optimize-level=3 -all -S -o - | filecheck %s --check-prefix NO_INLINE
;; RUN: foreach %s %t wasm-opt                        --monomorphize-always --inlining --optimize-level=3 -all -S -o - | filecheck %s --check-prefix YESINLINE

(module
  ;; NO_INLINE:      (type $A (sub (struct)))
  ;; YESINLINE:      (type $A (sub (struct)))
  (type $A (sub (struct)))

  ;; NO_INLINE:      (type $B (sub $A (struct)))
  ;; YESINLINE:      (type $B (sub $A (struct)))
  (type $B (sub $A (struct)))

  ;; NO_INLINE:      (type $2 (func))

  ;; NO_INLINE:      (type $3 (func (param (ref $A))))

  ;; NO_INLINE:      (type $4 (func (param (ref $B))))

  ;; NO_INLINE:      (func $calls (type $2)
  ;; NO_INLINE-NEXT:  (call $refinable_noinline
  ;; NO_INLINE-NEXT:   (struct.new_default $A)
  ;; NO_INLINE-NEXT:  )
  ;; NO_INLINE-NEXT:  (call $refinable_noinline
  ;; NO_INLINE-NEXT:   (struct.new_default $A)
  ;; NO_INLINE-NEXT:  )
  ;; NO_INLINE-NEXT:  (call $refinable_noinline_2
  ;; NO_INLINE-NEXT:   (struct.new_default $B)
  ;; NO_INLINE-NEXT:  )
  ;; NO_INLINE-NEXT:  (call $refinable_noinline_2
  ;; NO_INLINE-NEXT:   (struct.new_default $B)
  ;; NO_INLINE-NEXT:  )
  ;; NO_INLINE-NEXT: )
  ;; YESINLINE:      (type $2 (func))

  ;; YESINLINE:      (func $calls (type $2)
  ;; YESINLINE-NEXT:  (local $0 (ref $A))
  ;; YESINLINE-NEXT:  (local $1 (ref $A))
  ;; YESINLINE-NEXT:  (local $2 (ref $B))
  ;; YESINLINE-NEXT:  (local $3 (ref $A))
  ;; YESINLINE-NEXT:  (local $4 (ref $B))
  ;; YESINLINE-NEXT:  (local $5 (ref $A))
  ;; YESINLINE-NEXT:  (block
  ;; YESINLINE-NEXT:   (block $__inlined_func$refinable_noinline
  ;; YESINLINE-NEXT:    (local.set $0
  ;; YESINLINE-NEXT:     (struct.new_default $A)
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:    (drop
  ;; YESINLINE-NEXT:     (local.get $0)
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:   )
  ;; YESINLINE-NEXT:  )
  ;; YESINLINE-NEXT:  (block
  ;; YESINLINE-NEXT:   (block $__inlined_func$refinable_noinline$1
  ;; YESINLINE-NEXT:    (local.set $1
  ;; YESINLINE-NEXT:     (struct.new_default $A)
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:    (drop
  ;; YESINLINE-NEXT:     (local.get $1)
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:   )
  ;; YESINLINE-NEXT:  )
  ;; YESINLINE-NEXT:  (block
  ;; YESINLINE-NEXT:   (block $__inlined_func$refinable_noinline_2$2
  ;; YESINLINE-NEXT:    (local.set $2
  ;; YESINLINE-NEXT:     (struct.new_default $B)
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:    (block
  ;; YESINLINE-NEXT:     (local.set $3
  ;; YESINLINE-NEXT:      (local.get $2)
  ;; YESINLINE-NEXT:     )
  ;; YESINLINE-NEXT:     (drop
  ;; YESINLINE-NEXT:      (local.get $3)
  ;; YESINLINE-NEXT:     )
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:   )
  ;; YESINLINE-NEXT:  )
  ;; YESINLINE-NEXT:  (block
  ;; YESINLINE-NEXT:   (block $__inlined_func$refinable_noinline_2$3
  ;; YESINLINE-NEXT:    (local.set $4
  ;; YESINLINE-NEXT:     (struct.new_default $B)
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:    (block
  ;; YESINLINE-NEXT:     (local.set $5
  ;; YESINLINE-NEXT:      (local.get $4)
  ;; YESINLINE-NEXT:     )
  ;; YESINLINE-NEXT:     (drop
  ;; YESINLINE-NEXT:      (local.get $5)
  ;; YESINLINE-NEXT:     )
  ;; YESINLINE-NEXT:    )
  ;; YESINLINE-NEXT:   )
  ;; YESINLINE-NEXT:  )
  ;; YESINLINE-NEXT: )
  (func $calls
    ;; Two calls with $A, two with $B. The calls to $B will both go to the
    ;; same new monomorphized function which has a refined parameter of $B.
    ;;
    ;; In NO_INLINE we will not inline any of these 4 calls (if we did not
    ;; propagate the no-inline flag to the copied function, we would incorrectly
    ;; inline the monomorphized ones). In YESINLINE mode we will inline all 4.
    ;;
    (call $refinable_noinline
      (struct.new $A)
    )
    (call $refinable_noinline
      (struct.new $A)
    )
    (call $refinable_noinline
      (struct.new $B)
    )
    (call $refinable_noinline
      (struct.new $B)
    )
  )

  ;; NO_INLINE:      (func $refinable_noinline (type $3) (param $ref (ref $A))
  ;; NO_INLINE-NEXT:  (drop
  ;; NO_INLINE-NEXT:   (local.get $ref)
  ;; NO_INLINE-NEXT:  )
  ;; NO_INLINE-NEXT: )
  (func $refinable_noinline (param $ref (ref $A))
    ;; Some content to make it worth inlining.
    (drop
      (local.get $ref)
    )
  )
)
;; NO_INLINE:      (func $refinable_noinline_2 (type $4) (param $0 (ref $B))
;; NO_INLINE-NEXT:  (local $ref (ref $A))
;; NO_INLINE-NEXT:  (local.set $ref
;; NO_INLINE-NEXT:   (local.get $0)
;; NO_INLINE-NEXT:  )
;; NO_INLINE-NEXT:  (drop
;; NO_INLINE-NEXT:   (local.get $ref)
;; NO_INLINE-NEXT:  )
;; NO_INLINE-NEXT: )
