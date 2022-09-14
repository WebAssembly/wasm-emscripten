;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; Run without global effects, and run with, and also run with but discard them
;; first (to check that discard works; that should be the same as without).

;; RUN: foreach %s %t wasm-opt                                                    --vacuum -S -o - | filecheck --check-prefix WITHOUT %s
;; RUN: foreach %s %t wasm-opt --generate-global-effects                          --vacuum -S -o - | filecheck --check-prefix INCLUDE %s
;; RUN: foreach %s %t wasm-opt --generate-global-effects --discard-global-effects --vacuum -S -o - | filecheck --check-prefix DISCARD %s

(module
  ;; WITHOUT:      (type $none_=>_none (func))

  ;; WITHOUT:      (func $foo
  ;; WITHOUT-NEXT:  (call $nop)
  ;; WITHOUT-NEXT:  (call $unreachable)
  ;; WITHOUT-NEXT:  (call $call-nop)
  ;; WITHOUT-NEXT:  (call $call-unreachable)
  ;; WITHOUT-NEXT: )
  ;; INCLUDE:      (type $none_=>_none (func))

  ;; INCLUDE:      (func $foo
  ;; INCLUDE-NEXT:  (call $unreachable)
  ;; INCLUDE-NEXT:  (call $call-nop)
  ;; INCLUDE-NEXT:  (call $call-unreachable)
  ;; INCLUDE-NEXT: )
  ;; DISCARD:      (type $none_=>_none (func))

  ;; DISCARD:      (func $foo
  ;; DISCARD-NEXT:  (call $nop)
  ;; DISCARD-NEXT:  (call $unreachable)
  ;; DISCARD-NEXT:  (call $call-nop)
  ;; DISCARD-NEXT:  (call $call-unreachable)
  ;; DISCARD-NEXT: )
  (func $foo
    ;; Calling a function with no effects can be optimized away in INCLUDE (but
    ;; not WITHOUT or DISCARD, where the global effect info is not available).
    (call $nop)
    ;; Calling a function with effects cannot.
    (call $unreachable)
    ;; Calling something that calls something with no effects can be optimized
    ;; away in principle, but atm we don't look that far, so this is not
    ;; optimized.
    (call $call-nop)
    ;; Calling something that calls something with effects cannot.
    (call $call-unreachable)
  )

  ;; WITHOUT:      (func $cycle
  ;; WITHOUT-NEXT:  (call $cycle)
  ;; WITHOUT-NEXT: )
  ;; INCLUDE:      (func $cycle
  ;; INCLUDE-NEXT:  (call $cycle)
  ;; INCLUDE-NEXT: )
  ;; DISCARD:      (func $cycle
  ;; DISCARD-NEXT:  (call $cycle)
  ;; DISCARD-NEXT: )
  (func $cycle
    ;; Calling a function with no effects in a cycle cannot be optimized out -
    ;; this must keep hanging forever.
    (call $cycle)
  )

  ;; WITHOUT:      (func $nop
  ;; WITHOUT-NEXT:  (nop)
  ;; WITHOUT-NEXT: )
  ;; INCLUDE:      (func $nop
  ;; INCLUDE-NEXT:  (nop)
  ;; INCLUDE-NEXT: )
  ;; DISCARD:      (func $nop
  ;; DISCARD-NEXT:  (nop)
  ;; DISCARD-NEXT: )
  (func $nop
    (nop)
  )

  ;; WITHOUT:      (func $unreachable
  ;; WITHOUT-NEXT:  (unreachable)
  ;; WITHOUT-NEXT: )
  ;; INCLUDE:      (func $unreachable
  ;; INCLUDE-NEXT:  (unreachable)
  ;; INCLUDE-NEXT: )
  ;; DISCARD:      (func $unreachable
  ;; DISCARD-NEXT:  (unreachable)
  ;; DISCARD-NEXT: )
  (func $unreachable
    (unreachable)
  )

  ;; WITHOUT:      (func $call-nop
  ;; WITHOUT-NEXT:  (call $nop)
  ;; WITHOUT-NEXT: )
  ;; INCLUDE:      (func $call-nop
  ;; INCLUDE-NEXT:  (nop)
  ;; INCLUDE-NEXT: )
  ;; DISCARD:      (func $call-nop
  ;; DISCARD-NEXT:  (call $nop)
  ;; DISCARD-NEXT: )
  (func $call-nop
    ;; This call to a nop can be optimized out, as above, in INCLUDE.
    (call $nop)
  )

  ;; WITHOUT:      (func $call-unreachable
  ;; WITHOUT-NEXT:  (call $unreachable)
  ;; WITHOUT-NEXT: )
  ;; INCLUDE:      (func $call-unreachable
  ;; INCLUDE-NEXT:  (call $unreachable)
  ;; INCLUDE-NEXT: )
  ;; DISCARD:      (func $call-unreachable
  ;; DISCARD-NEXT:  (call $unreachable)
  ;; DISCARD-NEXT: )
  (func $call-unreachable
    (call $unreachable)
  )
)
