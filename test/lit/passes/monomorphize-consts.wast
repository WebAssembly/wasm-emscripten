;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; As in monomorphize-types.wast, test in both "always" mode, which always
;; monomorphizes, and in "careful" mode which does it only when it appears to
;; actually help.

;; RUN: foreach %s %t wasm-opt --monomorphize-always -all -S -o - | filecheck %s --check-prefix ALWAYS
;; RUN: foreach %s %t wasm-opt --monomorphize        -all -S -o - | filecheck %s --check-prefix CAREFUL

(module
  ;; Test that constants are monomorphized.

  ;; ALWAYS:      (type $0 (func (param i32)))

  ;; ALWAYS:      (type $1 (func (param i32 i32 funcref stringref)))

  ;; ALWAYS:      (type $2 (func (param i32) (result i32)))

  ;; ALWAYS:      (type $3 (func (result i32)))

  ;; ALWAYS:      (type $4 (func (param i32 i32)))

  ;; ALWAYS:      (import "a" "b" (func $import (type $0) (param i32)))
  ;; CAREFUL:      (type $0 (func (param i32)))

  ;; CAREFUL:      (type $1 (func (param i32 i32 funcref stringref)))

  ;; CAREFUL:      (type $2 (func (param i32) (result i32)))

  ;; CAREFUL:      (type $3 (func (result i32)))

  ;; CAREFUL:      (type $4 (func (param i32 i32)))

  ;; CAREFUL:      (import "a" "b" (func $import (type $0) (param i32)))
  (import "a" "b" (func $import (param i32)))

  ;; ALWAYS:      (elem declare func $calls)

  ;; ALWAYS:      (func $calls (type $4) (param $x i32) (param $y i32)
  ;; ALWAYS-NEXT:  (call $target_9
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $target_9
  ;; ALWAYS-NEXT:   (local.get $y)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $target_10
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (elem declare func $calls)

  ;; CAREFUL:      (func $calls (type $4) (param $x i32) (param $y i32)
  ;; CAREFUL-NEXT:  (call $target_9
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $target_9
  ;; CAREFUL-NEXT:   (local.get $y)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $target_10
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $calls (param $x i32) (param $y i32)
    ;; All but the parameter are constants that can be handled.
    (call $target
      (i32.const 1)
      (local.get $x)
      (ref.func $calls)
      (string.const "foo")
    )

    ;; This has the same effective call context, as the constants are identical,
    ;; and the non-constant is different, which we keep as a variable anyhow.
    ;; This will call the same refined function as the previous call.
    (call $target
      (i32.const 1)
      (local.get $y) ;; this changed
      (ref.func $calls)
      (string.const "foo")
    )

    ;; This has a different call context: one constant is different, so we'll
    ;; call a different refined function.
    (call $target
      (i32.const 3)  ;; this changed
      (local.get $x)
      (ref.func $calls)
      (string.const "foo")
    )
  )

  ;; ALWAYS:      (func $more-calls (type $0) (param $x i32)
  ;; ALWAYS-NEXT:  (call $target_9
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $other-target_11
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $work_12
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $more-calls (type $0) (param $x i32)
  ;; CAREFUL-NEXT:  (call $target_9
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $other-target_11
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $work_12
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $more-calls (param $x i32)
    ;; Identical to the first call in the previous function (except for the non-
    ;; constant second param, which is ok to be different). We should call the
    ;; same refined function before, even though we are in a different
    ;; function here.
    (call $target
      (i32.const 1)
      (local.get $x)
      (ref.func $calls)
      (string.const "foo")
    )

    ;; Call a different function but with the exact same params. This tests that
    ;; we handle identical contexts but with different functions. This will call
    ;; a different refined function than before
    (call $other-target
      (i32.const 1)
      (local.get $x)
      (ref.func $calls)
      (string.const "foo")
    )

    ;; Call yet another different function with the same context, this time the
    ;; function is worth optimizing even in CAREFUL mode, as the constants
    ;; unlock actual work.
    (call $work
      (i32.const 3)
      (local.get $x)
      (ref.func $calls)
      (string.const "foo")
    )
  )

  ;; ALWAYS:      (func $fail (type $0) (param $x i32)
  ;; ALWAYS-NEXT:  (call $target
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (block (result funcref)
  ;; ALWAYS-NEXT:    (ref.func $calls)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:   (block (result stringref)
  ;; ALWAYS-NEXT:    (string.const "foo")
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $fail (type $0) (param $x i32)
  ;; CAREFUL-NEXT:  (call $target
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (block (result funcref)
  ;; CAREFUL-NEXT:    (ref.func $calls)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:   (block (result stringref)
  ;; CAREFUL-NEXT:    (string.const "foo")
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $fail (param $x i32)
    ;; No operand is a constant here, so we do nothing.
    (call $target
      (local.get $x)
      (local.get $x)
      (block (result funcref)
        (ref.func $calls)
      )
      (block (result stringref)
        (string.const "foo")
      )
    )
  )

  ;; ALWAYS:      (func $mutual-recursion-a (type $2) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (if (result i32)
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (i32.add
  ;; ALWAYS-NEXT:     (call $mutual-recursion-b
  ;; ALWAYS-NEXT:      (local.get $x)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:     (call $mutual-recursion-b_13)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:   (else
  ;; ALWAYS-NEXT:    (i32.const 42)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $mutual-recursion-a (type $2) (param $0 i32) (result i32)
  ;; CAREFUL-NEXT:  (if (result i32)
  ;; CAREFUL-NEXT:   (local.get $0)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (i32.add
  ;; CAREFUL-NEXT:     (call $mutual-recursion-b
  ;; CAREFUL-NEXT:      (local.get $0)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:     (call $mutual-recursion-b_13)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:   (else
  ;; CAREFUL-NEXT:    (i32.const 42)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $mutual-recursion-a (param $x i32) (result i32)
    ;; We ignore direct recursion (see test in other monomorphize-types) but we
    ;; do handle mutual recursion normally. This also tests another function
    ;; that can be optimized, with a different signature than before.
    (if (result i32)
      (local.get $x)
      (then
        (i32.add
          ;; This call cannot be monomorphized.
          (call $mutual-recursion-b
            (local.get $x)
          )
          ;; The constant here allows us to monomorphize (in ALWAYS; to see the
          ;; benefit in CAREFUL, we need additional cycles, which we do not do
          ;; yet).
          (call $mutual-recursion-b
            (i32.const 0)
          )
        )
      )
      (else
        (i32.const 42)
      )
    )
  )

  ;; ALWAYS:      (func $mutual-recursion-b (type $2) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (i32.add
  ;; ALWAYS-NEXT:   (call $mutual-recursion-a_14)
  ;; ALWAYS-NEXT:   (i32.const 1337)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $mutual-recursion-b (type $2) (param $0 i32) (result i32)
  ;; CAREFUL-NEXT:  (i32.add
  ;; CAREFUL-NEXT:   (call $mutual-recursion-a_14)
  ;; CAREFUL-NEXT:   (i32.const 1337)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $mutual-recursion-b (param $x i32) (result i32)
    (i32.add
      ;; This can be optimized (as the constant 0 allows work to happen).
      (call $mutual-recursion-a
        (i32.const 0)
      )
      (i32.const 1337)
    )
  )

  ;; ALWAYS:      (func $target (type $1) (param $x i32) (param $y i32) (param $func funcref) (param $str stringref)
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $y)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $func)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $str)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $target (type $1) (param $0 i32) (param $1 i32) (param $2 funcref) (param $3 stringref)
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $target (param $x i32) (param $y i32) (param $func funcref) (param $str stringref)
    (drop
      (local.get $x)
    )
    (drop
      (local.get $y)
    )
    (drop
      (local.get $func)
    )
    (drop
      (local.get $str)
    )
  )

  ;; ALWAYS:      (func $other-target (type $1) (param $x i32) (param $y i32) (param $func funcref) (param $str stringref)
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $func)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $str)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (local.get $y)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $other-target (type $1) (param $0 i32) (param $1 i32) (param $2 funcref) (param $3 stringref)
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $other-target (param $x i32) (param $y i32) (param $func funcref) (param $str stringref)
    ;; Similar to $target, but the inside is a little reordered.
    (drop
      (local.get $func)
    )
    (drop
      (local.get $str)
    )
    (drop
      (local.get $x)
    )
    (drop
      (local.get $y)
    )
  )

  ;; ALWAYS:      (func $work (type $1) (param $x i32) (param $y i32) (param $func funcref) (param $str stringref)
  ;; ALWAYS-NEXT:  (call $import
  ;; ALWAYS-NEXT:   (i32.add
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (i32.add
  ;; ALWAYS-NEXT:     (ref.is_null
  ;; ALWAYS-NEXT:      (local.get $func)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:     (ref.is_null
  ;; ALWAYS-NEXT:      (local.get $str)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $import
  ;; ALWAYS-NEXT:   (local.get $y)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $work (type $1) (param $0 i32) (param $1 i32) (param $2 funcref) (param $3 stringref)
  ;; CAREFUL-NEXT:  (call $import
  ;; CAREFUL-NEXT:   (i32.add
  ;; CAREFUL-NEXT:    (local.get $0)
  ;; CAREFUL-NEXT:    (i32.add
  ;; CAREFUL-NEXT:     (ref.is_null
  ;; CAREFUL-NEXT:      (local.get $2)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:     (ref.is_null
  ;; CAREFUL-NEXT:      (local.get $3)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $import
  ;; CAREFUL-NEXT:   (local.get $1)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $work (param $x i32) (param $y i32) (param $func funcref) (param $str stringref)
    ;; Similar to $target, but the inside has actual work that can be optimized
    ;; away if we have constants here. Specifically the refs are not null and
    ;; $x is 3, so we sent 3 to the import here.
    (call $import
      (i32.add
        (local.get $x)
        (i32.add
          (ref.is_null
            (local.get $func)
          )
          (ref.is_null
            (local.get $str)
          )
        )
      )
    )
    ;; This parameter is unknown, so we can't do any optimization in this part.
    (call $import
      (local.get $y)
    )
  )
)
;; ALWAYS:      (func $target_9 (type $0) (param $0 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (local $func funcref)
;; ALWAYS-NEXT:  (local $str stringref)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $y
;; ALWAYS-NEXT:   (local.get $0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $func
;; ALWAYS-NEXT:   (ref.func $calls)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $str
;; ALWAYS-NEXT:   (string.const "foo")
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $y)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $func)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $str)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $target_10 (type $0) (param $0 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (local $func funcref)
;; ALWAYS-NEXT:  (local $str stringref)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 3)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $y
;; ALWAYS-NEXT:   (local.get $0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $func
;; ALWAYS-NEXT:   (ref.func $calls)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $str
;; ALWAYS-NEXT:   (string.const "foo")
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $y)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $func)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $str)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $other-target_11 (type $0) (param $0 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (local $func funcref)
;; ALWAYS-NEXT:  (local $str stringref)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $y
;; ALWAYS-NEXT:   (local.get $0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $func
;; ALWAYS-NEXT:   (ref.func $calls)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $str
;; ALWAYS-NEXT:   (string.const "foo")
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $func)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $str)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (drop
;; ALWAYS-NEXT:    (local.get $y)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $work_12 (type $0) (param $0 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (local $func funcref)
;; ALWAYS-NEXT:  (local $str stringref)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 3)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $y
;; ALWAYS-NEXT:   (local.get $0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $func
;; ALWAYS-NEXT:   (ref.func $calls)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $str
;; ALWAYS-NEXT:   (string.const "foo")
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block
;; ALWAYS-NEXT:   (call $import
;; ALWAYS-NEXT:    (i32.add
;; ALWAYS-NEXT:     (local.get $x)
;; ALWAYS-NEXT:     (i32.add
;; ALWAYS-NEXT:      (ref.is_null
;; ALWAYS-NEXT:       (local.get $func)
;; ALWAYS-NEXT:      )
;; ALWAYS-NEXT:      (ref.is_null
;; ALWAYS-NEXT:       (local.get $str)
;; ALWAYS-NEXT:      )
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (call $import
;; ALWAYS-NEXT:    (local.get $y)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $mutual-recursion-b_13 (type $3) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (i32.add
;; ALWAYS-NEXT:   (call $mutual-recursion-a
;; ALWAYS-NEXT:    (i32.const 0)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1337)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $mutual-recursion-a_14 (type $3) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (if (result i32)
;; ALWAYS-NEXT:   (local.get $x)
;; ALWAYS-NEXT:   (then
;; ALWAYS-NEXT:    (i32.add
;; ALWAYS-NEXT:     (call $mutual-recursion-b
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (call $mutual-recursion-b_13)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (else
;; ALWAYS-NEXT:    (i32.const 42)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $target_9 (type $0) (param $0 i32)
;; CAREFUL-NEXT:  (nop)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $target_10 (type $0) (param $0 i32)
;; CAREFUL-NEXT:  (nop)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $other-target_11 (type $0) (param $0 i32)
;; CAREFUL-NEXT:  (nop)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $work_12 (type $0) (param $0 i32)
;; CAREFUL-NEXT:  (call $import
;; CAREFUL-NEXT:   (i32.const 3)
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT:  (call $import
;; CAREFUL-NEXT:   (local.get $0)
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $mutual-recursion-b_13 (type $3) (result i32)
;; CAREFUL-NEXT:  (i32.add
;; CAREFUL-NEXT:   (call $mutual-recursion-a
;; CAREFUL-NEXT:    (i32.const 0)
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:   (i32.const 1337)
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $mutual-recursion-a_14 (type $3) (result i32)
;; CAREFUL-NEXT:  (i32.const 42)
;; CAREFUL-NEXT: )
