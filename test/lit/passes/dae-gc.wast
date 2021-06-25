;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --dae -S -o - | filecheck %s

(module
 ;; CHECK:      (type $none_=>_none (func))
 ;; CHECK:      (type ${} (struct ))
 (type ${} (struct))

 ;; CHECK:      (type $rtt_${}_=>_none (func (param (rtt ${}))))
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
 ;; CHECK-NEXT:   (ref.as_non_null
 ;; CHECK-NEXT:    (local.tee $0
 ;; CHECK-NEXT:     (i31.new
 ;; CHECK-NEXT:      (i32.const 2)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.tee $0
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $bar (param $0 i31ref)
  (drop
   ;; after the parameter is removed, we create a nullable local to replace it,
   ;; and must update the tee's type accordingly to avoid a validation error,
   ;; and also add a ref.as_non_null so that the outside still receives the
   ;; same type as before
   (local.tee $0
    (i31.new
     (i32.const 2)
    )
   )
  )
  ;; test for an unreachable tee, whose type must be unreachable even after
  ;; the change (the tee would need to be dropped if it were not unreachable,
  ;; so the correctness in this case is visible in the output)
  (local.tee $0
   (unreachable)
  )
 )
 ;; a function that gets an rtt that is never used. we cannot create a local for
 ;; that parameter, as it is not defaultable, so do not remove the parameter.
 ;; CHECK:      (func $get-rtt (param $0 (rtt ${}))
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $get-rtt (param $0 (rtt ${}))
  (nop)
 )
 ;; CHECK:      (func $send-rtt
 ;; CHECK-NEXT:  (call $get-rtt
 ;; CHECK-NEXT:   (rtt.canon ${})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $send-rtt
  (call $get-rtt
   (rtt.canon ${})
  )
 )
)
