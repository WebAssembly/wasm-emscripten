;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --dae -S -o - | filecheck %s

(module
 ;; CHECK:      (func $foo
 ;; CHECK-NEXT:  (call $bar)
 ;; CHECK-NEXT: )
 (func $foo
  (call $bar
   (i31.new
    (i32.const 1)
   )
  )
 )
 ;; CHECK:      (func $bar
 ;; CHECK-NEXT:  (local $0 (ref null i31))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.tee $0
 ;; CHECK-NEXT:    (i31.new
 ;; CHECK-NEXT:     (i32.const 2)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $bar (param $0 i31ref)
  (drop
   ;; after the parameter is removed, we create a nullable local to replace it,
   ;; and must update the tee's type accordingly to avoid a validation error
   ;; (which is what this test checks for)
   (local.tee $0
    (i31.new
     (i32.const 2)
    )
   )
  )
 )
)
