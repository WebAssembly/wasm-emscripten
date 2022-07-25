;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_test.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --simplify-globals -S -o - | filecheck %s

;; A global that is only read in order to be written is not needed.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $global i32 (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (func $simple
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple
    (if
      (global.get $global)
      (global.set $global (i32.const 1))
    )
  )
  ;; CHECK:      (func $more-with-no-side-effects
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.eqz
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (block $block
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $more-with-no-side-effects
    (if
      ;; Also test for other operations in the condition, with no effects.
      (i32.eqz
        (global.get $global)
      )
      ;; Also test for other operations in the body, with no effects.
      (block
        (nop)
        (global.set $global (i32.const 1))
      )
    )
  )
  ;; CHECK:      (func $additional-set
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $additional-set
    ;; An additional set does not prevent this optimization: the value written
    ;; will never be read in a way that matters.
    (global.set $global (i32.const 2))
  )
)
;; An additional read prevents the read-only-to-write optimization.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $global (mut i32) (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (func $additional-read
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (global.get $global)
  ;; CHECK-NEXT:   (global.set $global
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $global)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $additional-read
    (if
      (global.get $global)
      (global.set $global (i32.const 1))
    )
    (drop
      (global.get $global)
    )
  )
)
;; We do not optimize if-elses in the read-only-to-write optimization.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $global (mut i32) (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (func $if-else
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (global.get $global)
  ;; CHECK-NEXT:   (global.set $global
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (nop)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $if-else
    (if
      (global.get $global)
      (global.set $global (i32.const 1))
      (nop)
    )
  )
)
;; Side effects in the body prevent the read-only-to-write optimization.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $global (mut i32) (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (global $other (mut i32) (i32.const 0))
  (global $other (mut i32) (i32.const 0))
  ;; CHECK:      (func $side-effects-in-body
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (global.get $global)
  ;; CHECK-NEXT:   (block $block
  ;; CHECK-NEXT:    (global.set $global
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (global.set $other
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $side-effects-in-body
    (if
      (global.get $global)
      (block
        (global.set $global (i32.const 1))
        (global.set $other (i32.const 2))
        (drop (global.get $other))
      )
    )
  )
)
;; Nested patterns work as well, in a single run of the pass.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $a i32 (i32.const 0))
  (global $a (mut i32) (i32.const 0))
  ;; CHECK:      (global $b i32 (i32.const 0))
  (global $b (mut i32) (i32.const 0))
  ;; CHECK:      (global $c i32 (i32.const 0))
  (global $c (mut i32) (i32.const 0))

  ;; CHECK:      (func $nested
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (block $block
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (if
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:     (block $block1
  ;; CHECK-NEXT:      (if
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:       (block $block3
  ;; CHECK-NEXT:        (drop
  ;; CHECK-NEXT:         (i32.const 2)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (drop
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested
    (if
      (global.get $a)
      (block
        (global.set $a (i32.const 1))
        (if
          (global.get $b)
          (block
            (if
              (global.get $c)
              (block
                (global.set $c (i32.const 2))
              )
            )
            (global.set $b (i32.const 3))
          )
        )
      )
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once i32 (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (func $clinit
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (return)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $clinit
    ;; A read-only-to-write that takes an entire function body, and is in the
    ;; form if "if already set, return; set it". In particular, the set is not
    ;; in the if body in this case.
    (if
      (global.get $once)
      (return)
    )
    (global.set $once
      (i32.const 1)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once (mut i32) (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (func $clinit
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (global.get $once)
  ;; CHECK-NEXT:   (return)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $once
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $clinit
    ;; As above, but the optimization fails because the function body has too
    ;; many elements - a nop is added at the end.
    (if
      (global.get $once)
      (return)
    )
    (global.set $once
      (i32.const 1)
    )
    (nop)
  )
)

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once (mut i32) (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (func $clinit
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (global.get $once)
  ;; CHECK-NEXT:   (return)
  ;; CHECK-NEXT:   (nop)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $once
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $clinit
    ;; As above, but the optimization fails because the if has an else.
    (if
      (global.get $once)
      (return)
      (nop)
    )
    (global.set $once
      (i32.const 1)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once (mut i32) (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (func $clinit
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (global.get $once)
  ;; CHECK-NEXT:   (nop)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $once
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $clinit
    ;; As above, but the optimization fails because the if body is not a
    ;; return.
    (if
      (global.get $once)
      (nop)
    )
    (global.set $once
      (i32.const 1)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

  ;; CHECK:      (global $once (mut i32) (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (func $clinit
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (call $foo
  ;; CHECK-NEXT:    (global.get $once)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (return)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (global.set $once
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $clinit
    ;; As above, but the optimization fails because the if condition has effects.
    (if
      (call $foo ;; This call may have side effects and it receives the global's
                 ;; value, which is dangerous.
        (global.get $once)
      )
      (return)
    )
    (global.set $once
      (i32.const 1)
    )
  )

  ;; CHECK:      (func $foo (param $x i32) (result i32)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $foo (param $x i32) (result i32)
    (unreachable)
  )
)

;; Using the global's value in a way that can cause side effects prevents the
;; read-only-to-write optimization.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (global $global (mut i32) (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (global $other i32 (i32.const 0))
  (global $other (mut i32) (i32.const 0))
  ;; CHECK:      (func $side-effects-in-condition
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (if (result i32)
  ;; CHECK-NEXT:    (global.get $global)
  ;; CHECK-NEXT:    (call $foo)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (global.set $global
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $side-effects-in-condition
    (if
      (if (result i32)
        (global.get $global) ;; the global's value may cause foo() to be called
        (call $foo)
        (i32.const 1)
      )
      (global.set $global (i32.const 1))
    )
  )

  ;; CHECK:      (func $foo (result i32)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $foo (result i32)
    (unreachable)
  )
)

;; As above, but now the global's value is not the condition of the if, so there
;; is no problem.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (global $global i32 (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (global $other i32 (i32.const 0))
  (global $other (mut i32) (i32.const 0))
  ;; CHECK:      (func $side-effects-in-condition-2
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (if (result i32)
  ;; CHECK-NEXT:    (call $foo)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $side-effects-in-condition-2
    (if
      (if (result i32)
        (call $foo) ;; these side effects are not a problem, as the global's
                    ;; value cannot reach them.
        (i32.const 1)
        (global.get $global) ;; the global's value flows out through the if,
                             ;; safely
      )
      (global.set $global (i32.const 1))
    )
  )

  ;; CHECK:      (func $foo (result i32)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $foo (result i32)
    (unreachable)
  )
)

;; As above, but now the global's value flows into a side effect.
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $global (mut i32) (i32.const 0))
  (global $global (mut i32) (i32.const 0))
  ;; CHECK:      (global $other i32 (i32.const 0))
  (global $other (mut i32) (i32.const 0))
  ;; CHECK:      (func $side-effects-in-condition-3
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (local.tee $temp
  ;; CHECK-NEXT:    (global.get $global)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (global.set $global
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $side-effects-in-condition-3
    (local $temp i32)
    (if
      (local.tee $temp
        (global.get $global) ;; the global's value flows into a place that has
      )                      ;; side effects, so it may be noticed.
      (global.set $global (i32.const 1))
    )
  )
)

;; As above, but now the global's value flows through multiple layers of
;; things that have no side effects and are safe.
(module
  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once i32 (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (func $side-effects-in-condition-4
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.eqz
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (local.tee $x
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.load $0
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.add
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:      (i32.const 1337)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $side-effects-in-condition-4
    (local $x i32)
    (local $y i32)
    (if
      (i32.eqz
        (select
          (local.tee $x
            (i32.const 1)
          )
          (i32.load
            (i32.const 2)
          )
          (i32.add
            (global.get $once)
            (i32.const 1337)
          )
        )
      )
      (global.set $once
        (i32.const 1)
      )
    )
  )
)

(module
  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once i32 (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (func $nested-pattern
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (block $block (result i32)
  ;; CHECK-NEXT:    (if
  ;; CHECK-NEXT:     (i32.eqz
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.eq
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-pattern
    (local $x i32)
    (local $y i32)
    (if
      (block (result i32)
        ;; Another appearance of the pattern nested inside this one. This should
        ;; not prevent us from optimizing.
        (if
          (i32.eqz
            (global.get $once)
          )
          (global.set $once
            (i32.const 1)
          )
        )
        (i32.eq
          (global.get $once)
          (i32.const 0)
        )
      )
      (global.set $once
        (i32.const 1)
      )
    )
  )
)

(module
  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once (mut i32) (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (func $almost-nested-pattern
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (block $block (result i32)
  ;; CHECK-NEXT:    (if
  ;; CHECK-NEXT:     (i32.eqz
  ;; CHECK-NEXT:      (global.get $once)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.set $once
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (nop)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.eq
  ;; CHECK-NEXT:     (global.get $once)
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (global.set $once
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $almost-nested-pattern
    (local $x i32)
    (local $y i32)
    (if
      (block (result i32)
        ;; This is almost the nested pattern, but not quite, as it has an
        ;; "else" arm.
        (if
          (i32.eqz
            (global.get $once)
          )
          (global.set $once
            (i32.const 1)
          )
          (nop) ;; This breaks the pattern we are looking for.
        )
        (i32.eq
          (global.get $once)
          (i32.const 0)
        )
      )
      (global.set $once
        (i32.const 1)
      )
    )
  )
)

(module
  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once i32 (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (func $nested-pattern-thrice
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (block $block (result i32)
  ;; CHECK-NEXT:    (if
  ;; CHECK-NEXT:     (i32.eqz
  ;; CHECK-NEXT:      (block $block1 (result i32)
  ;; CHECK-NEXT:       (if
  ;; CHECK-NEXT:        (i32.const 0)
  ;; CHECK-NEXT:        (drop
  ;; CHECK-NEXT:         (i32.const 1)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.eq
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-pattern-thrice
    (local $x i32)
    (local $y i32)
    (if
      (block (result i32)
        (if
          (i32.eqz
            (block (result i32)
              ;; A third nested appearance.
              (if
                (global.get $once)
                (global.set $once
                  (i32.const 1)
                )
              )
              (global.get $once)
            )
          )
          (global.set $once
            (i32.const 1)
          )
        )
        (i32.eq
          (global.get $once)
          (i32.const 0)
        )
      )
      (global.set $once
        (i32.const 1)
      )
    )
  )
)

(module
  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (global $once (mut i32) (i32.const 0))
  (global $once (mut i32) (i32.const 0))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (func $nested-pattern
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (block $block (result i32)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (global.get $once)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (if
  ;; CHECK-NEXT:     (i32.eqz
  ;; CHECK-NEXT:      (global.get $once)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.set $once
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.eq
  ;; CHECK-NEXT:     (global.get $once)
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (global.set $once
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-pattern
    (local $x i32)
    (local $y i32)
    (if
      (block (result i32)

        ;; As above, but adding a drop of another get. This is *not* the
        ;; pattern we are looking for, and it will prevent us from
        ;; optimizing as we will no longer see that the number of gets
        ;; matches the number of read-only-to-write patterns. In the
        ;; future we could do a more complex counting operation to handle
        ;; this too.
        (drop
          (global.get $once)
        )

        (if
          (i32.eqz
            (global.get $once)
          )
          (global.set $once
            (i32.const 1)
          )
        )
        (i32.eq
          (global.get $once)
          (i32.const 0)
        )
      )
      (global.set $once
        (i32.const 1)
      )
    )
  )
)
