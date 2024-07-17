;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --closed-world            --gufa -Os                -all -S -o - | filecheck %s --check-prefix NOP
;; RUN: foreach %s %t wasm-opt --closed-world --type-ssa --gufa -Os --type-merging -all -S -o - | filecheck %s --check-prefix YES

;; Show that the combination of type-ssa and type-merging can find things that
;; otherwise cannot be optimized. NOP will fail to optimize something that YES
;; can.

(module
  ;; NOP:      (rec
  ;; NOP-NEXT:  (type $0 (func (param (ref $A)) (result i32)))

  ;; NOP:       (type $A (sub (struct (field i32))))
  ;; YES:      (type $0 (func (result i32)))

  ;; YES:      (rec
  ;; YES-NEXT:  (type $1 (func (param (ref $A))))

  ;; YES:       (type $A (sub (struct)))
  (type $A (sub (struct (field (mut i32)))))

  ;; NOP:      (type $2 (func (result i32)))

  ;; NOP:      (import "a" "b" (func $import (type $2) (result i32)))
  ;; YES:       (type $A_2 (sub $A (struct)))

  ;; YES:       (type $A_1 (sub $A (struct)))

  ;; YES:      (import "a" "b" (func $import (type $0) (result i32)))
  (import "a" "b" (func $import (result i32)))

  ;; NOP:      (export "main1" (func $main1))

  ;; NOP:      (export "main2" (func $main2))

  ;; NOP:      (func $main1 (type $2) (result i32)
  ;; NOP-NEXT:  (call $get-a-1
  ;; NOP-NEXT:   (struct.new $A
  ;; NOP-NEXT:    (i32.const 42)
  ;; NOP-NEXT:   )
  ;; NOP-NEXT:  )
  ;; NOP-NEXT: )
  ;; YES:      (export "main1" (func $main1))

  ;; YES:      (export "main2" (func $main2))

  ;; YES:      (func $main1 (type $0) (result i32)
  ;; YES-NEXT:  (call $get-a-1
  ;; YES-NEXT:   (struct.new_default $A)
  ;; YES-NEXT:  )
  ;; YES-NEXT:  (i32.const 42)
  ;; YES-NEXT: )
  (func $main1 (export "main1") (result i32)
    ;; YES can infer a result here, 42.
    (call $get-a-1
      (struct.new $A (i32.const 42))
    )
  )

  ;; NOP:      (func $main2 (type $2) (result i32)
  ;; NOP-NEXT:  (call $get-a-2
  ;; NOP-NEXT:   (struct.new $A
  ;; NOP-NEXT:    (i32.const 1337)
  ;; NOP-NEXT:   )
  ;; NOP-NEXT:  )
  ;; NOP-NEXT: )
  ;; YES:      (func $main2 (type $0) (result i32)
  ;; YES-NEXT:  (call $get-a-2
  ;; YES-NEXT:   (struct.new_default $A)
  ;; YES-NEXT:  )
  ;; YES-NEXT:  (i32.const 1337)
  ;; YES-NEXT: )
  (func $main2 (export "main2") (result i32)
    ;; YES can infer a result here, 1337.
    (call $get-a-2
      (struct.new $A (i32.const 1337))
    )
  )

  ;; NOP:      (func $get-a-1 (type $0) (param $0 (ref $A)) (result i32)
  ;; NOP-NEXT:  (if
  ;; NOP-NEXT:   (call $import)
  ;; NOP-NEXT:   (then
  ;; NOP-NEXT:    (return
  ;; NOP-NEXT:     (call $get-a-1
  ;; NOP-NEXT:      (local.get $0)
  ;; NOP-NEXT:     )
  ;; NOP-NEXT:    )
  ;; NOP-NEXT:   )
  ;; NOP-NEXT:  )
  ;; NOP-NEXT:  (struct.get $A 0
  ;; NOP-NEXT:   (local.get $0)
  ;; NOP-NEXT:  )
  ;; NOP-NEXT: )
  ;; YES:      (func $get-a-1 (type $1) (param $0 (ref $A))
  ;; YES-NEXT:  (if
  ;; YES-NEXT:   (call $import)
  ;; YES-NEXT:   (then
  ;; YES-NEXT:    (call $get-a-1
  ;; YES-NEXT:     (local.get $0)
  ;; YES-NEXT:    )
  ;; YES-NEXT:   )
  ;; YES-NEXT:  )
  ;; YES-NEXT: )
  (func $get-a-1 (param $ref (ref $A)) (result i32)
    ;; YES infers the result and applies it in the caller, so nothing is
    ;; returned any more (but we do keep the possibly infinite recursion, which
    ;; is necessary to avoid inlining making this testcase trivial even in NOP).
    (if
      (call $import)
      (then
        (return
          (call $get-a-1
            (local.get $ref)
          )
        )
      )
    )
    (struct.get $A 0 (local.get 0))
  )

  ;; NOP:      (func $get-a-2 (type $0) (param $0 (ref $A)) (result i32)
  ;; NOP-NEXT:  (if
  ;; NOP-NEXT:   (call $import)
  ;; NOP-NEXT:   (then
  ;; NOP-NEXT:    (return
  ;; NOP-NEXT:     (call $get-a-2
  ;; NOP-NEXT:      (local.get $0)
  ;; NOP-NEXT:     )
  ;; NOP-NEXT:    )
  ;; NOP-NEXT:   )
  ;; NOP-NEXT:  )
  ;; NOP-NEXT:  (struct.get $A 0
  ;; NOP-NEXT:   (local.get $0)
  ;; NOP-NEXT:  )
  ;; NOP-NEXT: )
  ;; YES:      (func $get-a-2 (type $1) (param $0 (ref $A))
  ;; YES-NEXT:  (if
  ;; YES-NEXT:   (call $import)
  ;; YES-NEXT:   (then
  ;; YES-NEXT:    (call $get-a-2
  ;; YES-NEXT:     (local.get $0)
  ;; YES-NEXT:    )
  ;; YES-NEXT:   )
  ;; YES-NEXT:  )
  ;; YES-NEXT: )
  (func $get-a-2 (param $ref (ref $A)) (result i32)
    ;; Parallel to the above.
    (if
      (call $import)
      (then
        (return
          (call $get-a-2
            (local.get $ref)
          )
        )
      )
    )
    (struct.get $A 0 (local.get 0))
  )
)
