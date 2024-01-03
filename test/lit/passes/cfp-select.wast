;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --cfp -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $substruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Rather than load from the struct, we can test between the two types
    ;; possible here.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)
