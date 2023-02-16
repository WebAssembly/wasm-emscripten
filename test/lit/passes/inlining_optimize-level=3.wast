;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --inlining --optimize-level=3 -S -o - | filecheck %s

(module
  ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (table $0 1 1 funcref)

  ;; CHECK:      (elem (i32.const 0) $no-loops-but-one-use-but-tabled)

  ;; CHECK:      (export "yes" (func $yes))
  (export "yes" (func $yes))
  ;; CHECK:      (export "no-loops-but-one-use-but-exported" (func $no-loops-but-one-use-but-exported))
  (export "no-loops-but-one-use-but-exported" (func $no-loops-but-one-use-but-exported))
  (table 1 1 funcref)
  (elem (i32.const 0) $no-loops-but-one-use-but-tabled)

  ;; CHECK:      (export "A" (func $recursive-inlining-1))

  ;; CHECK:      (export "B" (func $recursive-inlining-2))

  ;; CHECK:      (export "BA" (func $b-recursive-inlining-1))

  ;; CHECK:      (export "BB" (func $b-recursive-inlining-2))

  ;; CHECK:      (func $yes (result i32)
  ;; CHECK-NEXT:  (i32.const 1)
  ;; CHECK-NEXT: )
  (func $yes (result i32) ;; inlinable: small, lightweight, even with multi uses and a global use, ok when opt-level=3
    (i32.const 1)
  )
  (func $yes-big-but-single-use (result i32)
    (nop) (nop) (nop) (nop) (nop) (nop)
    (nop) (nop) (nop) (nop) (nop) (nop)
    (nop) (nop) (nop) (nop) (nop) (nop)
    (nop) (nop) (nop) (nop) (nop) (nop)
    (nop) (nop) (nop) (nop) (nop) (nop)
    (nop) (nop) (nop) (nop) (nop) (nop)
    (i32.const 1)
  )
  (func $no-calls (result i32)
    (call $yes)
  )
  (func $yes-calls-but-one-use (result i32)
    (call $yes)
  )
  (func $no-loops (result i32)
    (loop (result i32)
      (i32.const 1)
    )
  )
  (func $yes-loops-but-one-use (result i32)
    (loop (result i32)
      (i32.const 1)
    )
  )
  ;; CHECK:      (func $no-loops-but-one-use-but-exported (result i32)
  ;; CHECK-NEXT:  (loop $loop-in (result i32)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-loops-but-one-use-but-exported (result i32)
    (loop (result i32)
      (i32.const 1)
    )
  )
  ;; CHECK:      (func $no-loops-but-one-use-but-tabled (result i32)
  ;; CHECK-NEXT:  (loop $loop-in (result i32)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-loops-but-one-use-but-tabled (result i32)
    (loop (result i32)
      (i32.const 1)
    )
  )
  ;; CHECK:      (func $intoHere
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$yes (result i32)
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$yes-big-but-single-use (result i32)
  ;; CHECK-NEXT:     (block (result i32)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (nop)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block (result i32)
  ;; CHECK-NEXT:     (block $__inlined_func$no-calls (result i32)
  ;; CHECK-NEXT:      (block (result i32)
  ;; CHECK-NEXT:       (block $__inlined_func$yes0 (result i32)
  ;; CHECK-NEXT:        (i32.const 1)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block (result i32)
  ;; CHECK-NEXT:     (block $__inlined_func$no-calls0 (result i32)
  ;; CHECK-NEXT:      (block (result i32)
  ;; CHECK-NEXT:       (block $__inlined_func$yes1 (result i32)
  ;; CHECK-NEXT:        (i32.const 1)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block (result i32)
  ;; CHECK-NEXT:     (block $__inlined_func$yes-calls-but-one-use (result i32)
  ;; CHECK-NEXT:      (block (result i32)
  ;; CHECK-NEXT:       (block $__inlined_func$yes2 (result i32)
  ;; CHECK-NEXT:        (i32.const 1)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$no-loops (result i32)
  ;; CHECK-NEXT:     (loop $loop-in (result i32)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$no-loops0 (result i32)
  ;; CHECK-NEXT:     (loop $loop-in1 (result i32)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$yes-loops-but-one-use (result i32)
  ;; CHECK-NEXT:     (loop $loop-in0 (result i32)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$no-loops-but-one-use-but-exported (result i32)
  ;; CHECK-NEXT:     (loop $loop-in2 (result i32)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$no-loops-but-one-use-but-tabled (result i32)
  ;; CHECK-NEXT:     (loop $loop-in3 (result i32)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $intoHere
    (drop (call $yes))
    (drop (call $yes-big-but-single-use))
    (drop (call $no-calls))
    (drop (call $no-calls))
    (drop (call $yes-calls-but-one-use))
    (drop (call $no-loops))
    (drop (call $no-loops))
    (drop (call $yes-loops-but-one-use))
    (drop (call $no-loops-but-one-use-but-exported))
    (drop (call $no-loops-but-one-use-but-tabled))
  )

  ;; Two functions that call each other. (Exported, so that they are not removed
  ;; after inlining.) We should only perform a limited amount of inlining here -
  ;; a little might help, but like loop unrolling it loses its benefit quickly.
  ;; Specifically here we will see the infinite recursion after one inlining,
  ;; and stop (since we do not inline a method into itself).

  ;; CHECK:      (func $recursive-inlining-1 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (block $__inlined_func$recursive-inlining-2 (result i32)
  ;; CHECK-NEXT:   (local.set $1
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (call $recursive-inlining-1
  ;; CHECK-NEXT:     (local.get $1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $recursive-inlining-1 (export "A") (param $x i32) (result i32)
    (call $recursive-inlining-2
      (local.get $x)
    )
  )

  ;; CHECK:      (func $recursive-inlining-2 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (call $recursive-inlining-1
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $recursive-inlining-2 (export "B") (param $x i32) (result i32)
    (call $recursive-inlining-1
      (local.get $x)
    )
  )

  ;; As above, but both call the second. The first will inline the second
  ;; several times before hitting the limit (of around 5).

  ;; CHECK:      (func $b-recursive-inlining-1 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (block $__inlined_func$b-recursive-inlining-2 (result i32)
  ;; CHECK-NEXT:   (local.set $1
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$b-recursive-inlining-20 (result i32)
  ;; CHECK-NEXT:     (local.set $2
  ;; CHECK-NEXT:      (local.get $1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (block (result i32)
  ;; CHECK-NEXT:      (block $__inlined_func$b-recursive-inlining-21 (result i32)
  ;; CHECK-NEXT:       (local.set $3
  ;; CHECK-NEXT:        (local.get $2)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (block (result i32)
  ;; CHECK-NEXT:        (block $__inlined_func$b-recursive-inlining-22 (result i32)
  ;; CHECK-NEXT:         (local.set $4
  ;; CHECK-NEXT:          (local.get $3)
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (block (result i32)
  ;; CHECK-NEXT:          (block $__inlined_func$b-recursive-inlining-23 (result i32)
  ;; CHECK-NEXT:           (local.set $5
  ;; CHECK-NEXT:            (local.get $4)
  ;; CHECK-NEXT:           )
  ;; CHECK-NEXT:           (call $b-recursive-inlining-2
  ;; CHECK-NEXT:            (local.get $5)
  ;; CHECK-NEXT:           )
  ;; CHECK-NEXT:          )
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b-recursive-inlining-1 (export "BA") (param $x i32) (result i32)
    (call $b-recursive-inlining-2
      (local.get $x)
    )
  )

  ;; CHECK:      (func $b-recursive-inlining-2 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (call $b-recursive-inlining-2
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b-recursive-inlining-2 (export "BB") (param $x i32) (result i32)
    (call $b-recursive-inlining-2
      (local.get $x)
    )
  )

  ;; Verify that we can do a large number (larger than the limit of ~5 we just
  ;; saw for recursion) of calls into a single function, if we can do it all in
  ;; a single iteration (which is the usual case, and the case here).

  ;; CHECK:      (func $call-many-getters
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter0
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter1
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter2
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter3
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter4
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter5
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter6
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter7
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$getter8
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-many-getters
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
    (call $getter)
  )

  (func $getter
    (drop (i32.const 1))
  )
)

(module
 (func $bar
  (drop
   (block $__inlined_func$bar (result i32)
    (return) ;; After inlining, this return will be replaced with a br to a
             ;; new block. That block's name must not collide with the name
             ;; of the outer block here, which has been chosen so as to
             ;; potentially collide. If it collides, we will fail to validate
             ;; as the new outer block will have type none.
   )
  )
 )
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $foo
 ;; CHECK-NEXT:  (block $__inlined_func$bar_0
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block $__inlined_func$bar
 ;; CHECK-NEXT:     (br $__inlined_func$bar_0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo
  (call $bar)
 )
)

;; Similar to the above, but now the name collision happens due to a break in
;; one of the call's params. We must emit a different, non-colliding name.
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (block $__inlined_func$0_0
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block
 ;; CHECK-NEXT:     (block $__inlined_func$0_0_0
 ;; CHECK-NEXT:      (local.set $0
 ;; CHECK-NEXT:       (block (result i32)
 ;; CHECK-NEXT:        (br_if $__inlined_func$0_0
 ;; CHECK-NEXT:         (i32.const 10)
 ;; CHECK-NEXT:        )
 ;; CHECK-NEXT:        (i32.const 0)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:      (unreachable)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $1
  (block $__inlined_func$0_0
   (drop
    (call $0_0
     (block (result i32)
      (br_if $__inlined_func$0_0
       (i32.const 10)
      )
      (i32.const 0)
     )
    )
   )
  )
 )
 (func $0_0 (param $0 i32) (result i32)
  (unreachable)
 )
)

;; We inline multiple times here, and in the sequence of those inlinings we
;; turn the code in $B unreachable (when we inline $D), and no later inlining
;; (of $C or $A, or even $C's inlining in $A) should turn it into anything else
;; than an unreachable - once it is unreachable, we should keep it that way.
;; (That avoids possible validation problems, and maximizes DCE.) To keep it
;; unreachable we'll add an unreachable instruction after the inlined code.
(module
 ;; CHECK:      (type $f32_=>_none (func (param f32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $A (param $0 f32)
 ;; CHECK-NEXT:  (local $1 f32)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result f32)
 ;; CHECK-NEXT:    (block $__inlined_func$C (result f32)
 ;; CHECK-NEXT:     (local.set $1
 ;; CHECK-NEXT:      (local.get $0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $A (param $0 f32)
  (drop
   (call $C
    (local.get $0)
   )
  )
 )
 ;; CHECK:      (func $B
 ;; CHECK-NEXT:  (local $0 f32)
 ;; CHECK-NEXT:  (call $A
 ;; CHECK-NEXT:   (block
 ;; CHECK-NEXT:    (block $__inlined_func$C
 ;; CHECK-NEXT:     (local.tee $0
 ;; CHECK-NEXT:      (block
 ;; CHECK-NEXT:       (block $__inlined_func$D
 ;; CHECK-NEXT:        (unreachable)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (drop
 ;; CHECK-NEXT:      (local.get $0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (unreachable)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $B
  (call $A
   (call $C
    (call $D)
   )
  )
 )
 (func $C (param $0 f32) (result f32)
  (local.get $0)
 )
 (func $D (result f32)
  (unreachable)
 )
)
