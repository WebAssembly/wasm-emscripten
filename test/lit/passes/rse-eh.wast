;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: wasm-opt %s --rse -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (type $1 (func (result i32 exnref)))

  ;; CHECK:      (type $2 (func (param i32)))

  ;; CHECK:      (tag $e-i32 (param i32))
  (tag $e-i32 (param i32))
  ;; CHECK:      (tag $e-empty)
  (tag $e-empty)

  ;; CHECK:      (func $foo (type $0)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo)

  ;; CHECK:      (func $try_table1 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (block $outer
  ;; CHECK-NEXT:   (block $catch_all
  ;; CHECK-NEXT:    (try_table (catch_all $catch_all)
  ;; CHECK-NEXT:     (nop)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br $outer)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $try_table1
    (local $x i32)
    (block $outer
      (block $catch_all
        (try_table (catch_all $catch_all)
        )
        (br $outer)
      )
      (local.set $x (i32.const 1))
    )
    ;; try_table will not throw. So this should NOT be dropped
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $try_table2 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (block $catch_all
  ;; CHECK-NEXT:   (try_table (catch_all $catch_all)
  ;; CHECK-NEXT:    (throw $e-i32
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $x
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $try_table2
    (local $x i32)
    (block $catch_all
      (try_table (catch_all $catch_all)
        (throw $e-i32 (i32.const 0))
        (local.set $x (i32.const 1))
      )
    )
    ;; local.set is after 'throw' so it will not run. This should NOT be
    ;; dropped.
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $try_table3 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (block $outer
  ;; CHECK-NEXT:   (block $catch_all
  ;; CHECK-NEXT:    (try_table (catch_all $catch_all)
  ;; CHECK-NEXT:     (call $foo)
  ;; CHECK-NEXT:     (local.set $x
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br $outer)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $try_table3
    (local $x i32)
    (block $outer
      (block $catch_all
        (try_table (catch_all $catch_all)
          (call $foo)
          (local.set $x (i32.const 1))
        )
        (br $outer)
      )
    )
    ;; (call $foo) may throw and the local.set may not run, so this should NOT
    ;; be dropped
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $try_table4 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (block $outer
  ;; CHECK-NEXT:   (block $catch_all
  ;; CHECK-NEXT:    (try_table (catch_all $catch_all)
  ;; CHECK-NEXT:     (local.set $x
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (call $foo)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br $outer)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $try_table4
    (local $x i32)
    (block $outer
      (block $catch_all
        (try_table (catch_all $catch_all)
          (local.set $x (i32.const 1))
          (call $foo)
        )
        (br $outer)
      )
    )
    ;; Even if (call $foo) throws, local.set runs before it, so this should be
    ;; dropped
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $nested-try_table1 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $exn exnref)
  ;; CHECK-NEXT:  (block $catch_all0
  ;; CHECK-NEXT:   (try_table (catch_all $catch_all0)
  ;; CHECK-NEXT:    (local.set $exn
  ;; CHECK-NEXT:     (block $catch_all_ref1 (result exnref)
  ;; CHECK-NEXT:      (try_table (catch_all_ref $catch_all_ref1)
  ;; CHECK-NEXT:       (throw $e-i32
  ;; CHECK-NEXT:        (i32.const 0)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $x
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (throw_ref
  ;; CHECK-NEXT:     (local.get $exn)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-try_table1
    (local $x i32)
    (local $exn exnref)
    (block $catch_all0
      (try_table (catch_all $catch_all0)
        (local.set $exn
          (block $catch_all_ref1 (result exnref)
            (try_table (catch_all_ref $catch_all_ref1)
              (throw $e-i32 (i32.const 0))
            )
          )
        )
        (local.set $x (i32.const 1))
        (throw_ref (local.get $exn))
      )
    )
    ;; The exception is caught by the inner catch_all_ref, which runs the
    ;; local.set, so this should be dropped
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $nested-try_table2 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $exn exnref)
  ;; CHECK-NEXT:  (local $pair (i32 exnref))
  ;; CHECK-NEXT:  (block $catch_all0
  ;; CHECK-NEXT:   (try_table (catch_all $catch_all0)
  ;; CHECK-NEXT:    (local.set $pair
  ;; CHECK-NEXT:     (block $catch1 (type $1) (result i32 exnref)
  ;; CHECK-NEXT:      (try_table (catch_ref $e-i32 $catch1)
  ;; CHECK-NEXT:       (throw $e-i32
  ;; CHECK-NEXT:        (i32.const 0)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $exn
  ;; CHECK-NEXT:     (tuple.extract 2 1
  ;; CHECK-NEXT:      (local.get $pair)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $x
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (throw_ref
  ;; CHECK-NEXT:     (local.get $exn)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-try_table2
    (local $x i32)
    (local $exn exnref)
    (local $pair (i32 exnref))
    (block $catch_all0
      (try_table (catch_all $catch_all0)
        (local.set $pair
          (block $catch1 (result i32 exnref)
            (try_table (catch_ref $e-i32 $catch1)
              (throw $e-i32 (i32.const 0))
            )
          )
        )
        (local.set $exn
          (tuple.extract 2 1 (local.get $pair))
        )
        (local.set $x (i32.const 1))
        (throw_ref (local.get $exn))
      )
    )
    ;; Unlike nested-try_table1, the exception may not be caught by the inner
    ;; catch, so the local.set may not run. So this should NOT be dropped.
    ;; TODO This actually can be removed if we analyze tags in CFGWalker,
    ;; because we throw an i32 and catch an i32 too in the inner try_table. Add
    ;; this to the analysis.
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $nested-try_table3 (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $exn exnref)
  ;; CHECK-NEXT:  (local $pair (i32 exnref))
  ;; CHECK-NEXT:  (block $catch_all0
  ;; CHECK-NEXT:   (try_table (catch_all $catch_all0)
  ;; CHECK-NEXT:    (block $outer1
  ;; CHECK-NEXT:     (local.set $pair
  ;; CHECK-NEXT:      (block $catch1 (type $1) (result i32 exnref)
  ;; CHECK-NEXT:       (try_table (catch_ref $e-i32 $catch1)
  ;; CHECK-NEXT:        (call $foo)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (br $outer1)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (local.set $exn
  ;; CHECK-NEXT:      (tuple.extract 2 1
  ;; CHECK-NEXT:       (local.get $pair)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (local.set $x
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (throw_ref
  ;; CHECK-NEXT:      (local.get $exn)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-try_table3
    (local $x i32)
    (local $exn exnref)
    (local $pair (i32 exnref))
    (block $catch_all0
      (try_table (catch_all $catch_all0)
        (block $outer1
          (local.set $pair
            (block $catch1 (result i32 exnref)
              (try_table (catch_ref $e-i32 $catch1)
                (call $foo)
              )
              (br $outer1)
            )
          )
          (local.set $exn
            (tuple.extract 2 1 (local.get $pair))
          )
          (local.set $x (i32.const 1))
          (throw_ref (local.get $exn))
        )
      )
    )
    ;; Unlike nested-try_table1, the exception may not be caught by the inner
    ;; catch, so the local.set may not run. So this should NOT be dropped.
    ;; Unlike nested-try_table2, In this case we don't know what (call $foo)
    ;; will throw, so we can't drop this even if we analyze tags.
    (local.set $x (i32.const 1))
  )

  ;; CHECK:      (func $catchless-try_table (type $0)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (try_table
  ;; CHECK-NEXT:   (call $foo)
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $catchless-try_table
    (local $x i32)
    (try_table
      (call $foo)
      (local.set $x (i32.const 1))
    )
    ;; The only way we end up here is when (call $foo) does not throw, because
    ;; if (call $foo) throws, it will throw to the caller because it is within
    ;; a catchless try_table. In that case the local.set after (call $foo) would
    ;; have run before this, so this can be dropped.
    (local.set $x (i32.const 1))
  )
)
