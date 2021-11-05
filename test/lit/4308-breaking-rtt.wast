;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; Regression test for issue #4308 in which the flow returned when executing a
;; breaking rtt field was incorrect.

;; RUN: wasm-opt %s -all --precompute -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --precompute --fuzz-exec

(module
 ;; CHECK:      (type ${} (struct ))
 (type ${} (struct ))
 ;; CHECK:      (type $none_=>_rtt_{} (func (result (rtt ${}))))
 (type $none_=>_rtt_{} (func (result (rtt ${}))))
 ;; CHECK:      (type $f32_f32_ref?|data|_=>_externref (func (param f32 f32 (ref null data)) (result externref)))
 (type $f32_f32_ref?|data|_=>_externref (func (param f32 f32 (ref null data)) (result externref)))

 ;; CHECK:      (export "func_117" (func $1))
 (export "func_117" (func $1))

 ;; CHECK:      (func $0 (result (rtt ${}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $0 (result (rtt ${}))
  (unreachable)
 )

 ;; CHECK:      (func $1 (param $0 f32) (param $1 f32) (param $2 (ref null data)) (result externref)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (ref.cast
 ;; CHECK-NEXT:    (ref.cast
 ;; CHECK-NEXT:     (struct.new_default ${})
 ;; CHECK-NEXT:     (call $0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (call $0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null extern)
 ;; CHECK-NEXT: )
 (func $1 (param $0 f32) (param $1 f32) (param $2 (ref null data)) (result externref)
  (drop
   (ref.cast
    (ref.cast
     (struct.new_default ${})
     (call $0)
    )
    (call $0)
   )
  )
  (ref.null extern)
 )
)
