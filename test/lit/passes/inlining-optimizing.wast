;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --inlining-optimizing -S -o - | filecheck %s

(module
 ;; CHECK:      (type $none_=>_none (func))
 (type $none_=>_none (func))
 (type $none_=>_i32 (func (result i32)))
 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $0
  (nop)
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call_ref
 ;; CHECK-NEXT:    (unreachable)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $1
  ;; $0 will be inlined into here. We will then optimize this function - but
  ;; we do so *without* optimizing $0 (as inlining-optimizing only optimizes
  ;; where it inlines, for efficiency). As part of the optimiziations, we will
  ;; try to precompute the cast here, which will try to look up $0. We should
  ;; not hit an assertion, rather we should skip precomputing it, the same as if
  ;; we were optimizing $1 before $0 were added to the module.
  (call $0)
  (drop
   (call_ref
    (ref.cast
     (ref.func $0)
     (rtt.canon $none_=>_i32)
    )
   )
  )
 )
)

