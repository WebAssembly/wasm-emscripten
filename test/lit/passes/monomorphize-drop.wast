;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; As in monomorphize-types.wast, test in both "always" mode, which always
;; monomorphizes, and in "careful" mode which does it only when it appears to
;; actually help, and use a minimum benefit of 0 to make it easy to write
;; small testcases.

;; RUN: foreach %s %t wasm-opt --monomorphize-always                                -all -S -o - | filecheck %s --check-prefix ALWAYS
;; RUN: foreach %s %t wasm-opt --monomorphize --pass-arg=monomorphize-min-benefit@0 -all -S -o - | filecheck %s --check-prefix CAREFUL

(module
  ;; Test that dropped functions are monomorphized, and the drop is reverse-
  ;; inlined into the called function, enabling more optimizations.

  ;; ALWAYS:      (type $0 (func (result i32)))

  ;; ALWAYS:      (type $1 (func (param i32)))

  ;; ALWAYS:      (type $2 (func (param i32 i32) (result i32)))

  ;; ALWAYS:      (type $3 (func (param i32) (result i32)))

  ;; ALWAYS:      (type $4 (func))

  ;; ALWAYS:      (type $5 (func (param i32 i32)))

  ;; ALWAYS:      (func $work (type $2) (param $x i32) (param $y i32) (result i32)
  ;; ALWAYS-NEXT:  (i32.mul
  ;; ALWAYS-NEXT:   (i32.xor
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (local.get $y)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:   (i32.div_s
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (local.get $y)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (type $0 (func (result i32)))

  ;; CAREFUL:      (type $1 (func (param i32)))

  ;; CAREFUL:      (type $2 (func (param i32 i32) (result i32)))

  ;; CAREFUL:      (type $3 (func (param i32) (result i32)))

  ;; CAREFUL:      (type $4 (func))

  ;; CAREFUL:      (type $5 (func (param i32 i32)))

  ;; CAREFUL:      (func $work (type $2) (param $0 i32) (param $1 i32) (result i32)
  ;; CAREFUL-NEXT:  (i32.mul
  ;; CAREFUL-NEXT:   (i32.div_s
  ;; CAREFUL-NEXT:    (local.get $0)
  ;; CAREFUL-NEXT:    (local.get $1)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:   (i32.xor
  ;; CAREFUL-NEXT:    (local.get $0)
  ;; CAREFUL-NEXT:    (local.get $1)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $work (param $x i32) (param $y i32) (result i32)
    ;; Do some nontrivial work that we return. If this is dropped then we don't
    ;; need that work.
    (i32.mul
      (i32.xor
        (local.get $x)
        (local.get $y)
      )
      (i32.div_s
        (local.get $x)
        (local.get $y)
      )
    )
  )

  ;; ALWAYS:      (func $calls (type $1) (param $x i32)
  ;; ALWAYS-NEXT:  (call $work_5)
  ;; ALWAYS-NEXT:  (call $work_5)
  ;; ALWAYS-NEXT:  (call $work_6
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $work_7
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $calls (type $1) (param $x i32)
  ;; CAREFUL-NEXT:  (call $work_5)
  ;; CAREFUL-NEXT:  (call $work_5)
  ;; CAREFUL-NEXT:  (call $work_6
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $work_7
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $calls (param $x i32)
    ;; Both of these can call the same monomorphized function. In CAREFUL mode
    ;; that function's body can also be optimized into a nop.
    (drop
      (call $work
        (i32.const 3)
        (i32.const 4)
      )
    )
    (drop
      (call $work
        (i32.const 3)
        (i32.const 4)
      )
    )
    ;; Another call, now with an unknown parameter. This calls a different
    ;; monomorphized function, but once again the body can be optimized into a
    ;; nop in CAREFUL.
    (drop
      (call $work
        (i32.const 3)
        (local.get $x)
      )
    )
    ;; Two unknown parameters. Yet another monomorphized function, but the same
    ;; outcome.
    (drop
      (call $work
        (local.get $x)
        (local.get $x)
      )
    )
  )

  ;; ALWAYS:      (func $call-undropped-trivial (type $3) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (call $work
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-undropped-trivial (type $3) (param $x i32) (result i32)
  ;; CAREFUL-NEXT:  (call $work
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-undropped-trivial (param $x i32) (result i32)
    ;; A call of the same target that is dropped in the previous function, but
    ;; now without a drop. We know nothing nontrivial here, so we do nothing.
    (call $work
      (local.get $x)
      (local.get $x)
    )
  )

  ;; ALWAYS:      (func $call-undropped (type $0) (result i32)
  ;; ALWAYS-NEXT:  (call $work_8)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-undropped (type $0) (result i32)
  ;; CAREFUL-NEXT:  (call $work_8)
  ;; CAREFUL-NEXT: )
  (func $call-undropped (result i32)
    ;; As above but now with constant params. We can monomorphize here - there
    ;; is no issue in optimizing here without a drop and with a drop elsewhere -
    ;; but we do call a different function of course, that returns an i32.
    (call $work
      (i32.const 3)
      (i32.const 4)
    )
  )

  ;; ALWAYS:      (func $call-no-params-return (type $0) (result i32)
  ;; ALWAYS-NEXT:  (return_call $work
  ;; ALWAYS-NEXT:   (i32.const 10)
  ;; ALWAYS-NEXT:   (i32.const 20)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-no-params-return (type $0) (result i32)
  ;; CAREFUL-NEXT:  (return_call $work
  ;; CAREFUL-NEXT:   (i32.const 10)
  ;; CAREFUL-NEXT:   (i32.const 20)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-no-params-return (result i32)
    ;; Return calls can be monomorphized too, but we have that as a TODO atm.
    (return_call $work
      (i32.const 10)
      (i32.const 20)
    )
  )
)

;; ALWAYS:      (func $work_5 (type $4)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (i32.const 3)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $y
;; ALWAYS-NEXT:     (i32.const 4)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (i32.mul
;; ALWAYS-NEXT:     (i32.xor
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.div_s
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $work_6 (type $1) (param $0 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (i32.const 3)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $y
;; ALWAYS-NEXT:     (local.get $0)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (i32.mul
;; ALWAYS-NEXT:     (i32.xor
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.div_s
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $work_7 (type $5) (param $0 i32) (param $1 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (local.get $0)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $y
;; ALWAYS-NEXT:     (local.get $1)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (i32.mul
;; ALWAYS-NEXT:     (i32.xor
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.div_s
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $work_8 (type $0) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 3)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $y
;; ALWAYS-NEXT:   (i32.const 4)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (i32.mul
;; ALWAYS-NEXT:   (i32.xor
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (local.get $y)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.div_s
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (local.get $y)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $work_5 (type $4)
;; CAREFUL-NEXT:  (nop)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $work_6 (type $1) (param $0 i32)
;; CAREFUL-NEXT:  (drop
;; CAREFUL-NEXT:   (i32.div_s
;; CAREFUL-NEXT:    (i32.const 3)
;; CAREFUL-NEXT:    (local.get $0)
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $work_7 (type $5) (param $0 i32) (param $1 i32)
;; CAREFUL-NEXT:  (drop
;; CAREFUL-NEXT:   (i32.div_s
;; CAREFUL-NEXT:    (local.get $0)
;; CAREFUL-NEXT:    (local.get $1)
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $work_8 (type $0) (result i32)
;; CAREFUL-NEXT:  (i32.const 0)
;; CAREFUL-NEXT: )
(module
  ;; ALWAYS:      (type $0 (func))

  ;; ALWAYS:      (type $1 (func (param i32 i32) (result i32)))

  ;; ALWAYS:      (type $2 (func (result i32)))

  ;; ALWAYS:      (import "a" "b" (func $import (type $1) (param i32 i32) (result i32)))
  ;; CAREFUL:      (type $0 (func))

  ;; CAREFUL:      (type $1 (func (param i32 i32) (result i32)))

  ;; CAREFUL:      (type $2 (func (result i32)))

  ;; CAREFUL:      (import "a" "b" (func $import (type $1) (param i32 i32) (result i32)))
  (import "a" "b" (func $import (param i32 i32) (result i32)))

  ;; ALWAYS:      (func $no-params (type $2) (result i32)
  ;; ALWAYS-NEXT:  (i32.const 42)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $no-params (type $2) (result i32)
  ;; CAREFUL-NEXT:  (i32.const 42)
  ;; CAREFUL-NEXT: )
  (func $no-params (result i32)
    ;; A function that will be dropped, and has no params.
    (i32.const 42)
  )

  ;; ALWAYS:      (func $call-no-params (type $2) (result i32)
  ;; ALWAYS-NEXT:  (call $no-params_6)
  ;; ALWAYS-NEXT:  (call $no-params)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-no-params (type $2) (result i32)
  ;; CAREFUL-NEXT:  (call $no-params_6)
  ;; CAREFUL-NEXT:  (call $no-params)
  ;; CAREFUL-NEXT: )
  (func $call-no-params (result i32)
    ;; We can optimize the drop into the target.
    (drop
      (call $no-params)
    )
    ;; Without a drop, the call context is trivial and we do nothing.
    (call $no-params)
  )

  ;; ALWAYS:      (func $call-import (type $0)
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $import
  ;; ALWAYS-NEXT:    (i32.const 3)
  ;; ALWAYS-NEXT:    (i32.const 4)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-import (type $0)
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $import
  ;; CAREFUL-NEXT:    (i32.const 3)
  ;; CAREFUL-NEXT:    (i32.const 4)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-import
    ;; Calling an import allows no optimizations.
    (drop
      (call $import
        (i32.const 3)
        (i32.const 4)
      )
    )
  )

  ;; ALWAYS:      (func $import-work (type $1) (param $x i32) (param $y i32) (result i32)
  ;; ALWAYS-NEXT:  (call $import
  ;; ALWAYS-NEXT:   (i32.xor
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (local.get $y)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:   (i32.div_s
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (local.get $y)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $import-work (type $1) (param $0 i32) (param $1 i32) (result i32)
  ;; CAREFUL-NEXT:  (call $import
  ;; CAREFUL-NEXT:   (i32.xor
  ;; CAREFUL-NEXT:    (local.get $0)
  ;; CAREFUL-NEXT:    (local.get $1)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:   (i32.div_s
  ;; CAREFUL-NEXT:    (local.get $0)
  ;; CAREFUL-NEXT:    (local.get $1)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $import-work (param $x i32) (param $y i32) (result i32)
    ;; Do some work and also call an import.
    (call $import
      (i32.xor
        (local.get $x)
        (local.get $y)
      )
      (i32.div_s
        (local.get $x)
        (local.get $y)
      )
    )
  )

  ;; ALWAYS:      (func $call-import-work (type $0)
  ;; ALWAYS-NEXT:  (call $import-work_7)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-import-work (type $0)
  ;; CAREFUL-NEXT:  (call $import-work_7)
  ;; CAREFUL-NEXT: )
  (func $call-import-work
    ;; This is monomorphized with the drop.
    (drop
      (call $import-work
        (i32.const 3)
        (i32.const 4)
      )
    )
  )
)

;; ALWAYS:      (func $no-params_6 (type $0)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (i32.const 42)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $import-work_7 (type $0)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local $y i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (i32.const 3)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $y
;; ALWAYS-NEXT:     (i32.const 4)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (call $import
;; ALWAYS-NEXT:     (i32.xor
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.div_s
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (local.get $y)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $no-params_6 (type $0)
;; CAREFUL-NEXT:  (nop)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $import-work_7 (type $0)
;; CAREFUL-NEXT:  (drop
;; CAREFUL-NEXT:   (call $import
;; CAREFUL-NEXT:    (i32.const 7)
;; CAREFUL-NEXT:    (i32.const 0)
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )
(module
  ;; ALWAYS:      (type $0 (func (param i32)))

  ;; ALWAYS:      (type $1 (func))

  ;; ALWAYS:      (type $2 (func (result i32)))

  ;; ALWAYS:      (type $3 (func (param i32) (result i32)))

  ;; ALWAYS:      (import "a" "c" (func $import (type $2) (result i32)))
  ;; CAREFUL:      (type $0 (func (param i32)))

  ;; CAREFUL:      (type $1 (func))

  ;; CAREFUL:      (type $2 (func (result i32)))

  ;; CAREFUL:      (type $3 (func (param i32) (result i32)))

  ;; CAREFUL:      (import "a" "c" (func $import (type $2) (result i32)))
  (import "a" "c" (func $import (result i32)))

  ;; ALWAYS:      (func $return-normal (type $3) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (drop
  ;; ALWAYS-NEXT:     (call $import)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (return
  ;; ALWAYS-NEXT:     (i32.const 0)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (i32.const 1)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $return-normal (type $3) (param $0 i32) (result i32)
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $0)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (drop
  ;; CAREFUL-NEXT:     (call $import)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (return
  ;; CAREFUL-NEXT:     (i32.const 0)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (i32.const 1)
  ;; CAREFUL-NEXT: )
  (func $return-normal (param $x i32) (result i32)
    ;; This function has a return, which needs to be handled in the
    ;; monomorphized function, as we'll no longer return a value.
    (if
      (local.get $x)
      (then
        (drop
          (call $import)
        )
        (return
          (i32.const 0)
        )
      )
    )
    ;; Also return a value by flowing it out.
    (i32.const 1)
  )

  ;; ALWAYS:      (func $call-return-normal (type $0) (param $x i32)
  ;; ALWAYS-NEXT:  (call $return-normal_3)
  ;; ALWAYS-NEXT:  (call $return-normal_4)
  ;; ALWAYS-NEXT:  (call $return-normal_5
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-return-normal (type $0) (param $x i32)
  ;; CAREFUL-NEXT:  (call $return-normal_3)
  ;; CAREFUL-NEXT:  (call $return-normal_4)
  ;; CAREFUL-NEXT:  (call $return-normal_5
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-return-normal (param $x i32)
    ;; Call the above function with 0, 1, and an unknown value, to test the two
    ;; code paths there + the case of the input being unknown. We monomorphize
    ;; them all (differently).
    (drop
      (call $return-normal
        (i32.const 0)
      )
    )
    (drop
      (call $return-normal
        (i32.const 1)
      )
    )
    (drop
      (call $return-normal
        (local.get $x)
      )
    )
  )
)

;; ALWAYS:      (func $return-normal_3 (type $1)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (i32.const 0)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (block (result i32)
;; ALWAYS-NEXT:     (if
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (then
;; ALWAYS-NEXT:       (drop
;; ALWAYS-NEXT:        (call $import)
;; ALWAYS-NEXT:       )
;; ALWAYS-NEXT:       (block
;; ALWAYS-NEXT:        (drop
;; ALWAYS-NEXT:         (i32.const 0)
;; ALWAYS-NEXT:        )
;; ALWAYS-NEXT:        (return)
;; ALWAYS-NEXT:       )
;; ALWAYS-NEXT:      )
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.const 1)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $return-normal_4 (type $1)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (i32.const 1)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (block (result i32)
;; ALWAYS-NEXT:     (if
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (then
;; ALWAYS-NEXT:       (drop
;; ALWAYS-NEXT:        (call $import)
;; ALWAYS-NEXT:       )
;; ALWAYS-NEXT:       (block
;; ALWAYS-NEXT:        (drop
;; ALWAYS-NEXT:         (i32.const 0)
;; ALWAYS-NEXT:        )
;; ALWAYS-NEXT:        (return)
;; ALWAYS-NEXT:       )
;; ALWAYS-NEXT:      )
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.const 1)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $return-normal_5 (type $0) (param $0 i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $x
;; ALWAYS-NEXT:     (local.get $0)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (block (result i32)
;; ALWAYS-NEXT:     (if
;; ALWAYS-NEXT:      (local.get $x)
;; ALWAYS-NEXT:      (then
;; ALWAYS-NEXT:       (drop
;; ALWAYS-NEXT:        (call $import)
;; ALWAYS-NEXT:       )
;; ALWAYS-NEXT:       (block
;; ALWAYS-NEXT:        (drop
;; ALWAYS-NEXT:         (i32.const 0)
;; ALWAYS-NEXT:        )
;; ALWAYS-NEXT:        (return)
;; ALWAYS-NEXT:       )
;; ALWAYS-NEXT:      )
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:     (i32.const 1)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $return-normal_3 (type $1)
;; CAREFUL-NEXT:  (nop)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $return-normal_4 (type $1)
;; CAREFUL-NEXT:  (drop
;; CAREFUL-NEXT:   (call $import)
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $return-normal_5 (type $0) (param $0 i32)
;; CAREFUL-NEXT:  (if
;; CAREFUL-NEXT:   (local.get $0)
;; CAREFUL-NEXT:   (then
;; CAREFUL-NEXT:    (drop
;; CAREFUL-NEXT:     (call $import)
;; CAREFUL-NEXT:    )
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )
(module
  ;; ALWAYS:      (type $0 (func (result i32)))

  ;; ALWAYS:      (type $1 (func (param i32) (result i32)))

  ;; ALWAYS:      (type $2 (func (param i32)))

  ;; ALWAYS:      (import "a" "c" (func $import (type $0) (result i32)))
  ;; CAREFUL:      (type $0 (func (result i32)))

  ;; CAREFUL:      (type $1 (func (param i32) (result i32)))

  ;; CAREFUL:      (type $2 (func (param i32)))

  ;; CAREFUL:      (import "a" "c" (func $import (type $0) (result i32)))
  (import "a" "c" (func $import (result i32)))

  ;; ALWAYS:      (func $return-call (type $1) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (return_call $import)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (i32.const 1)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $return-call (type $1) (param $0 i32) (result i32)
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $0)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (return_call $import)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (i32.const 1)
  ;; CAREFUL-NEXT: )
  (func $return-call (param $x i32) (result i32)
    ;; As above, but now with a return_call. We do not monomorphize the drop
    ;; part, as if we included the drop we'd turn the call into a non-return
    ;; call, which can break things.
    (if
      (local.get $x)
      (then
        (return_call $import)
      )
    )
    (i32.const 1)
  )

  ;; ALWAYS:      (func $call-return-call (type $2) (param $x i32)
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call_3)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call_4)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-return-call (type $2) (param $x i32)
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call_3)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call_4)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call
  ;; CAREFUL-NEXT:    (local.get $x)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-return-call (param $x i32)
    ;; As above, but due to the return call we won't monomorphize the drop. As
    ;; a result we monomorphize the first two, leaving drops here, and do
    ;; nothing for the last (as the call context is trivial).
    (drop
      (call $return-call
        (i32.const 0)
      )
    )
    (drop
      (call $return-call
        (i32.const 1)
      )
    )
    (drop
      (call $return-call
        (local.get $x)
      )
    )
  )
)

;; ALWAYS:      (func $return-call_3 (type $0) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block (result i32)
;; ALWAYS-NEXT:   (if
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (then
;; ALWAYS-NEXT:     (return_call $import)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $return-call_4 (type $0) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block (result i32)
;; ALWAYS-NEXT:   (if
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (then
;; ALWAYS-NEXT:     (return_call $import)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $return-call_3 (type $0) (result i32)
;; CAREFUL-NEXT:  (i32.const 1)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $return-call_4 (type $0) (result i32)
;; CAREFUL-NEXT:  (return_call $import)
;; CAREFUL-NEXT: )
(module
  ;; ALWAYS:      (type $i (func (result i32)))
  ;; CAREFUL:      (type $i (func (result i32)))
  (type $i (func (result i32)))

  ;; ALWAYS:      (type $1 (func (param i32) (result i32)))

  ;; ALWAYS:      (type $2 (func (param i32)))

  ;; ALWAYS:      (import "a" "c" (func $import (type $i) (result i32)))
  ;; CAREFUL:      (type $1 (func (param i32) (result i32)))

  ;; CAREFUL:      (type $2 (func (param i32)))

  ;; CAREFUL:      (import "a" "c" (func $import (type $i) (result i32)))
  (import "a" "c" (func $import (result i32)))

  ;; ALWAYS:      (table $table 10 10 funcref)
  ;; CAREFUL:      (table $table 10 10 funcref)
  (table $table 10 10 funcref)

  ;; ALWAYS:      (func $return-call-indirect (type $1) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (return_call_indirect $table (type $i)
  ;; ALWAYS-NEXT:     (call $import)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (i32.const 1)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $return-call-indirect (type $1) (param $0 i32) (result i32)
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $0)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (return_call_indirect $table (type $i)
  ;; CAREFUL-NEXT:     (call $import)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (i32.const 1)
  ;; CAREFUL-NEXT: )
  (func $return-call-indirect (param $x i32) (result i32)
    ;; As above, but now with a return_call_indirect. The outcome below is
    ;; similar.
    (if
      (local.get $x)
      (then
        (return_call_indirect (type $i)
          (call $import)
        )
      )
    )
    (i32.const 1)
  )

  ;; ALWAYS:      (func $call-return-call-indirect (type $2) (param $x i32)
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call-indirect_3)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call-indirect_4)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call-indirect
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-return-call-indirect (type $2) (param $x i32)
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call-indirect_3)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call-indirect_4)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call-indirect
  ;; CAREFUL-NEXT:    (local.get $x)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $call-return-call-indirect (param $x i32)
    (drop
      (call $return-call-indirect
        (i32.const 0)
      )
    )
    (drop
      (call $return-call-indirect
        (i32.const 1)
      )
    )
    (drop
      (call $return-call-indirect
        (local.get $x)
      )
    )
  )
)

;; ALWAYS:      (func $return-call-indirect_3 (type $i) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block (result i32)
;; ALWAYS-NEXT:   (if
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (then
;; ALWAYS-NEXT:     (return_call_indirect $table (type $i)
;; ALWAYS-NEXT:      (call $import)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $return-call-indirect_4 (type $i) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block (result i32)
;; ALWAYS-NEXT:   (if
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (then
;; ALWAYS-NEXT:     (return_call_indirect $table (type $i)
;; ALWAYS-NEXT:      (call $import)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $return-call-indirect_3 (type $i) (result i32)
;; CAREFUL-NEXT:  (i32.const 1)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $return-call-indirect_4 (type $i) (result i32)
;; CAREFUL-NEXT:  (return_call_indirect $table (type $i)
;; CAREFUL-NEXT:   (call $import)
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )
(module
  ;; ALWAYS:      (type $i (func (result i32)))
  ;; CAREFUL:      (type $i (func (result i32)))
  (type $i (func (result i32)))

  ;; ALWAYS:      (type $1 (func (param i32) (result i32)))

  ;; ALWAYS:      (import "a" "c" (func $import (type $i) (result i32)))
  ;; CAREFUL:      (type $1 (func (param i32) (result i32)))

  ;; CAREFUL:      (import "a" "c" (func $import (type $i) (result i32)))
  (import "a" "c" (func $import (result i32)))

  ;; ALWAYS:      (table $table 10 10 funcref)
  ;; CAREFUL:      (table $table 10 10 funcref)
  (table $table 10 10 funcref)

  ;; ALWAYS:      (elem declare func $import)

  ;; ALWAYS:      (func $return-call-ref (type $1) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (return_call_ref $i
  ;; ALWAYS-NEXT:     (ref.func $import)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (i32.const 1)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (elem declare func $import)

  ;; CAREFUL:      (func $return-call-ref (type $1) (param $0 i32) (result i32)
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $0)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (return_call $import)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (i32.const 1)
  ;; CAREFUL-NEXT: )
  (func $return-call-ref (param $x i32) (result i32)
    ;; As above, but now with a return_call_ref. The outcome below is similar.
    (if
      (local.get $x)
      (then
        (return_call_ref $i
          (ref.func $import)
        )
      )
    )
    (i32.const 1)
  )

  ;; ALWAYS:      (func $call-return-call-ref (type $1) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call-ref_3)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call-ref_4)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (drop
  ;; ALWAYS-NEXT:   (call $return-call-ref
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (return_call $import)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (return_call_indirect $table (type $i)
  ;; ALWAYS-NEXT:     (i32.const 7)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (if
  ;; ALWAYS-NEXT:   (local.get $x)
  ;; ALWAYS-NEXT:   (then
  ;; ALWAYS-NEXT:    (return_call_ref $i
  ;; ALWAYS-NEXT:     (ref.func $import)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (unreachable)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $call-return-call-ref (type $1) (param $x i32) (result i32)
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call-ref_3)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call-ref_4)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (drop
  ;; CAREFUL-NEXT:   (call $return-call-ref
  ;; CAREFUL-NEXT:    (local.get $x)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (return_call $import)
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (return_call_indirect $table (type $i)
  ;; CAREFUL-NEXT:     (i32.const 7)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (if
  ;; CAREFUL-NEXT:   (local.get $x)
  ;; CAREFUL-NEXT:   (then
  ;; CAREFUL-NEXT:    (return_call_ref $i
  ;; CAREFUL-NEXT:     (ref.func $import)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (unreachable)
  ;; CAREFUL-NEXT: )
  (func $call-return-call-ref (param $x i32) (result i32)
    ;; As before, a set of three calls (with similar outcomes as before: the
    ;; first two are monomorphized without the drop; the last is unchanged).
    (drop
      (call $return-call-ref
        (i32.const 0)
      )
    )
    (drop
      (call $return-call-ref
        (i32.const 1)
      )
    )
    (drop
      (call $return-call-ref
        (local.get $x)
      )
    )

    ;; Also add some return calls here, to show that it is fine for the caller
    ;; to have them: we can still monomorphize some of the previous calls
    ;; (without their drops).
    (if
      (local.get $x)
      (then
        (return_call $import)
      )
    )
    (if
      (local.get $x)
      (then
        (return_call_indirect (type $i)
          (i32.const 7)
        )
      )
    )
    (if
      (local.get $x)
      (then
        (return_call_ref $i
          (ref.func $import)
        )
      )
    )
    (unreachable)
  )
)

;; ALWAYS:      (func $return-call-ref_3 (type $i) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block (result i32)
;; ALWAYS-NEXT:   (if
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (then
;; ALWAYS-NEXT:     (return_call_ref $i
;; ALWAYS-NEXT:      (ref.func $import)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $return-call-ref_4 (type $i) (result i32)
;; ALWAYS-NEXT:  (local $x i32)
;; ALWAYS-NEXT:  (local.set $x
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (block (result i32)
;; ALWAYS-NEXT:   (if
;; ALWAYS-NEXT:    (local.get $x)
;; ALWAYS-NEXT:    (then
;; ALWAYS-NEXT:     (return_call_ref $i
;; ALWAYS-NEXT:      (ref.func $import)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:   (i32.const 1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $return-call-ref_3 (type $i) (result i32)
;; CAREFUL-NEXT:  (i32.const 1)
;; CAREFUL-NEXT: )

;; CAREFUL:      (func $return-call-ref_4 (type $i) (result i32)
;; CAREFUL-NEXT:  (return_call $import)
;; CAREFUL-NEXT: )
