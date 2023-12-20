;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt -O1 -S -o - | filecheck %s

(module
 ;; CHECK:      (type $0 (func (result i32)))

 ;; CHECK:      (memory $0 1 1)
 (memory $0 1 1)
 (global $global$0 (mut i32) (i32.const 10))
 ;; CHECK:      (export "foo" (func $foo))

 ;; CHECK:      (func $foo (result i32)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (i32.load align=1
 ;; CHECK-NEXT:   (i32.const 4)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo (export "foo") (result i32)
  (i32.load offset=4 align=1
   (i32.and
    (block $label$1 (result i32)
     (global.set $global$0
      (i32.const 0)
     )
     (i32.const -64)
    )
    (i32.const 15)
   )
  )
 )
 (func $signed-overflow (param $0 f32) (result i32)
  (i32.sub
   (i32.const 268435456)
   (i32.const -2147483648)
  )
 )
)


