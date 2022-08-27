;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --inlining -S -o - | filecheck %s

;; Test that we inline functions with unreachable bodies. This is important to
;; propagate the trap to the caller (where it might lead to DCE).

(module
  (func $trap
    (unreachable)
  )
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (func $call-trap
  ;; CHECK-NEXT:  (block $__inlined_func$trap
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (br $__inlined_func$trap)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-trap
    ;; In this case the call had type none, but the inlined code is unreachable,
    ;; so we'll add a br to the new block to keep the type as none (the br is
    ;; not actually reached, and other opts will remove it).
    (call $trap)
  )

  (func $trap-result (result i32)
    ;; As above, but now there is a declared result.
    (unreachable)
  )

  ;; CHECK:      (func $call-trap-result (result i32)
  ;; CHECK-NEXT:  (block $__inlined_func$trap-result (result i32)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-trap-result (result i32)
    (call $trap-result)
  )

  (func $contents-then-trap
    ;; Add some contents in addition to the trap.
    (nop)
    (drop
      (i32.const 1337)
    )
    (nop)
    (unreachable)
  )
  ;; CHECK:      (func $call-contents-then-trap
  ;; CHECK-NEXT:  (block $__inlined_func$contents-then-trap
  ;; CHECK-NEXT:   (block
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1337)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $__inlined_func$contents-then-trap)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-contents-then-trap
    (call $contents-then-trap)
  )
)
