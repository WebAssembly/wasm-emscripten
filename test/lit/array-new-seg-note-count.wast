;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s -all --roundtrip -S -o - | filecheck %s

;; Test that the array type is emitted into the type section properly.
(module
 ;; CHECK:      (type $vec (array i32))
 (type $vec (array i32))
 ;; CHECK:      (type $none_=>_ref|$vec| (func (result (ref $vec))))

 ;; CHECK:      (data "")
 (data "")
 ;; CHECK:      (func $test (result (ref $vec))
 ;; CHECK-NEXT:  (array.new_data $vec 0
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $test (result (ref $vec))
  (array.new_data $vec 0
   (i32.const 0)
   (i32.const 0)
  )
 )
)
