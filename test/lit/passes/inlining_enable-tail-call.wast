;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --inlining --enable-tail-call -S -o - | filecheck %s

(module
  (table 1 1 funcref)
  (elem (i32.const 0) $tabled)
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (table $0 1 1 funcref)

  ;; CHECK:      (elem (i32.const 0) $tabled)

  ;; CHECK:      (export "user" (func $user))

  ;; CHECK:      (export "exported" (func $exported))

  ;; CHECK:      (func $user
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y f64)
  ;; CHECK-NEXT:  (local $2 f32)
  ;; CHECK-NEXT:  (local $3 i64)
  ;; CHECK-NEXT:  (local $4 f32)
  ;; CHECK-NEXT:  (local $5 i64)
  ;; CHECK-NEXT:  (local $6 f32)
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$exported
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$tabled
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$multi
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$multi0
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$ok
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$int (result i32)
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result f64)
  ;; CHECK-NEXT:    (block $__inlined_func$double (result f64)
  ;; CHECK-NEXT:     (f64.const 3.14159)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$int2 (result i32)
  ;; CHECK-NEXT:     (i32.const 112)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (block (result f64)
  ;; CHECK-NEXT:    (block $__inlined_func$double2 (result f64)
  ;; CHECK-NEXT:     (f64.const 113.14159)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$with-local
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f32.const 2.1418280601501465)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$with-local2
  ;; CHECK-NEXT:    (local.set $3
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $3
  ;; CHECK-NEXT:     (i64.const 4)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block $__inlined_func$return (result i32)
  ;; CHECK-NEXT:     (br $__inlined_func$return
  ;; CHECK-NEXT:      (i32.const 5)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$multipass
  ;; CHECK-NEXT:    (block
  ;; CHECK-NEXT:     (block
  ;; CHECK-NEXT:      (block $__inlined_func$multipass2
  ;; CHECK-NEXT:       (drop
  ;; CHECK-NEXT:        (i32.const 6)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$param
  ;; CHECK-NEXT:    (local.set $4
  ;; CHECK-NEXT:     (f32.const 12.34000015258789)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $5
  ;; CHECK-NEXT:     (i64.const 890005350012)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $6
  ;; CHECK-NEXT:     (f32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (block
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (local.get $4)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (local.get $5)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (local.get $6)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $user (export "user")
    (local $x i32)
    (local $y f64)
    (call $exported)
    (call $tabled)
    (call $multi)
    (call $multi)
    (call $ok)
    (drop (call $int))
    (drop (call $double))
    (local.set $x (call $int2))
    (local.set $y (call $double2))
    (call $with-local)
    (call $with-local2)
    (drop (call $return))
    (call $multipass)
    (call $param (f32.const 12.34) (i64.const 890005350012))
  )
  ;; CHECK:      (func $exported
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $exported (export "exported")
    (nop)
  )
  ;; CHECK:      (func $recursive
  ;; CHECK-NEXT:  (call $recursive)
  ;; CHECK-NEXT: )
  (func $recursive
    (call $recursive)
  )
  ;; CHECK:      (func $tabled
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $tabled
    (nop)
  )
  ;; CHECK:      (func $cycle1
  ;; CHECK-NEXT:  (block $__inlined_func$cycle2
  ;; CHECK-NEXT:   (block
  ;; CHECK-NEXT:    (call $cycle1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $cycle1
    (call $cycle2)
  )
  (func $cycle2
    (call $cycle1)
  )
  (func $multi
    (nop)
  )
  (func $ok
    (drop (i32.const 1))
  )
  (func $int (result i32)
    (i32.const 2)
  )
  (func $double (result f64)
    (f64.const 3.14159)
  )
  (func $int2 (result i32)
    (i32.const 112)
  )
  (func $double2 (result f64)
    (f64.const 113.14159)
  )
  (func $with-local
    (local $x f32)
    (local.set $x (f32.const 2.141828))
  )
  (func $with-local2
    (local $y i64)
    (local.set $y (i64.const 4))
  )
  (func $return (result i32)
    (return (i32.const 5))
  )
  (func $multipass
    (call $multipass2)
  )
  (func $multipass2
    (drop (i32.const 6))
  )
  (func $param (param $x f32) (param $y i64)
    (local $z f32)
    (drop (local.get $x))
    (drop (local.get $y))
    (drop (local.get $z))
  )
)
(module
  ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (func $child (param $0 i32) (result i32)
  ;; CHECK-NEXT:  (i32.const 1234)
  ;; CHECK-NEXT: )
  (func $child (param i32) (result i32)
    (i32.const 1234)
  )
  ;; CHECK:      (func $parent (result i32)
  ;; CHECK-NEXT:  (call $child
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $parent (result i32)
    (call $child
      (unreachable) ;; call is not performed, no sense to inline
    )
  )
)
(module
 ;; CHECK:      (type $f32_i32_=>_i32 (func (param f32 i32) (result i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (global $hangLimit (mut i32) (i32.const 25))
 (global $hangLimit (mut i32) (i32.const 25))
 ;; CHECK:      (memory $0 1 1)
 (memory $0 1 1)
 ;; CHECK:      (export "hangLimitInitializer" (func $hangLimitInitializer))
 (export "hangLimitInitializer" (func $hangLimitInitializer))
 (func $func_3 (result i32)
  (local $0 i32)
  (select
   (local.get $0) ;; we depend on the zero-init value here, so it must be set when inlining!
   (local.tee $0
    (i32.const -1)
   )
   (i32.const 1)
  )
 )
 ;; CHECK:      (func $func_4 (param $0 f32) (param $1 i32) (result i32)
 ;; CHECK-NEXT:  (local $2 i64)
 ;; CHECK-NEXT:  (local $3 f64)
 ;; CHECK-NEXT:  (local $4 f32)
 ;; CHECK-NEXT:  (local $5 i32)
 ;; CHECK-NEXT:  (local $6 i32)
 ;; CHECK-NEXT:  (local $7 f64)
 ;; CHECK-NEXT:  (local $8 i32)
 ;; CHECK-NEXT:  (loop $label$0 (result i32)
 ;; CHECK-NEXT:   (block $block
 ;; CHECK-NEXT:    (if
 ;; CHECK-NEXT:     (i32.eqz
 ;; CHECK-NEXT:      (global.get $hangLimit)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (return
 ;; CHECK-NEXT:      (i32.const 54)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (global.set $hangLimit
 ;; CHECK-NEXT:     (i32.sub
 ;; CHECK-NEXT:      (global.get $hangLimit)
 ;; CHECK-NEXT:      (i32.const 1)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.eqz
 ;; CHECK-NEXT:    (if (result i32)
 ;; CHECK-NEXT:     (i32.const 1)
 ;; CHECK-NEXT:     (if (result i32)
 ;; CHECK-NEXT:      (i32.eqz
 ;; CHECK-NEXT:       (block (result i32)
 ;; CHECK-NEXT:        (block $__inlined_func$func_3 (result i32)
 ;; CHECK-NEXT:         (local.set $8
 ;; CHECK-NEXT:          (i32.const 0)
 ;; CHECK-NEXT:         )
 ;; CHECK-NEXT:         (select
 ;; CHECK-NEXT:          (local.get $8)
 ;; CHECK-NEXT:          (local.tee $8
 ;; CHECK-NEXT:           (i32.const -1)
 ;; CHECK-NEXT:          )
 ;; CHECK-NEXT:          (i32.const 1)
 ;; CHECK-NEXT:         )
 ;; CHECK-NEXT:        )
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:      (br $label$0)
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (unreachable)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $func_4 (param $0 f32) (param $1 i32) (result i32)
  (local $2 i64)
  (local $3 f64)
  (local $4 f32)
  (local $5 i32)
  (local $6 i32)
  (local $7 f64)
  (loop $label$0 (result i32)
   (block
    (if
     (i32.eqz
      (global.get $hangLimit)
     )
     (return
      (i32.const 54)
     )
    )
    (global.set $hangLimit
     (i32.sub
      (global.get $hangLimit)
      (i32.const 1)
     )
    )
   )
   (i32.eqz
    (if (result i32)
     (i32.const 1)
     (if (result i32)
      (i32.eqz
       (call $func_3)
      )
      (br $label$0)
      (i32.const 0)
     )
     (unreachable)
    )
   )
  )
 )
 ;; CHECK:      (func $hangLimitInitializer
 ;; CHECK-NEXT:  (global.set $hangLimit
 ;; CHECK-NEXT:   (i32.const 25)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $hangLimitInitializer
  (global.set $hangLimit
   (i32.const 25)
  )
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $T (func (param i32)))
 (type $T (func (param i32)))
 (table 10 funcref)
 ;; CHECK:      (table $0 10 funcref)

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (block $__inlined_func$1
 ;; CHECK-NEXT:   (call_indirect (type $T)
 ;; CHECK-NEXT:    (if (result i32)
 ;; CHECK-NEXT:     (i32.const 0)
 ;; CHECK-NEXT:     (unreachable)
 ;; CHECK-NEXT:     (unreachable)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (i32.const 1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0
  (call $1)
 )
 (func $1
  (call_indirect (type $T)
   (if (result i32) ;; if copy must preserve the forced type
    (i32.const 0)
    (unreachable)
    (unreachable)
   )
   (i32.const 1)
  )
 )
)
(module
 (func $0
  (block $label$1 ;; copy this name
   (br_table $label$1 $label$1
    (i32.const 0)
   )
  )
 )
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (block $__inlined_func$0
 ;; CHECK-NEXT:   (block $label$1
 ;; CHECK-NEXT:    (br_table $label$1 $label$1
 ;; CHECK-NEXT:     (i32.const 0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $1
  (call $0)
 )
)
(module
 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (func $0 (result i32)
 ;; CHECK-NEXT:  (return
 ;; CHECK-NEXT:   (block $__inlined_func$1 (result i32)
 ;; CHECK-NEXT:    (i32.const 42)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0 (result i32)
  (return_call $1)
 )
 (func $1 (result i32)
  (i32.const 42)
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (block $__inlined_func$1
 ;; CHECK-NEXT:    (local.set $0
 ;; CHECK-NEXT:     (i32.const 42)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (drop
 ;; CHECK-NEXT:     (local.get $0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (return)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0
  (return_call $1
   (i32.const 42)
  )
 )
 (func $1 (param i32)
  (drop
   (local.get 0)
  )
 )
)
(module
 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (func $0 (result i32)
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (return
 ;; CHECK-NEXT:   (block $__inlined_func$1 (result i32)
 ;; CHECK-NEXT:    (local.set $0
 ;; CHECK-NEXT:     (i32.const 42)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0 (result i32)
  (return_call $1
   (i32.const 42)
  )
 )
 (func $1 (param i32) (result i32)
  (local.get 0)
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (block $__inlined_func$1 (result i32)
 ;; CHECK-NEXT:     (block
 ;; CHECK-NEXT:      (br $__inlined_func$1
 ;; CHECK-NEXT:       (block (result i32)
 ;; CHECK-NEXT:        (block $__inlined_func$2 (result i32)
 ;; CHECK-NEXT:         (i32.const 42)
 ;; CHECK-NEXT:        )
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0
  (drop
   (call $1)
  )
 )
 (func $1 (result i32)
  (return_call $2)
 )
 (func $2 (result i32)
  (i32.const 42)
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (block $__inlined_func$1
 ;; CHECK-NEXT:   (block
 ;; CHECK-NEXT:    (block
 ;; CHECK-NEXT:     (block
 ;; CHECK-NEXT:      (block $__inlined_func$2
 ;; CHECK-NEXT:       (local.set $0
 ;; CHECK-NEXT:        (i32.const 42)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:       (drop
 ;; CHECK-NEXT:        (local.get $0)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (br $__inlined_func$1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (br $__inlined_func$1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0
  (call $1)
 )
 (func $1
  (return_call $2
   (i32.const 42)
  )
 )
 (func $2 (param i32)
  (drop
   (local.get 0)
  )
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $T (func (param i32) (result i32)))
 (type $T (func (param i32) (result i32)))
 (table 10 funcref)
 ;; CHECK:      (table $0 10 funcref)

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (block $__inlined_func$1 (result i32)
 ;; CHECK-NEXT:     (br $__inlined_func$1
 ;; CHECK-NEXT:      (call_indirect (type $T)
 ;; CHECK-NEXT:       (i32.const 42)
 ;; CHECK-NEXT:       (i32.const 0)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0
  (drop
   (call $1)
  )
 )
 (func $1 (result i32)
  (return_call_indirect (type $T)
   (i32.const 42)
   (i32.const 0)
  )
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $T (func (param i32)))
 (type $T (func (param i32)))
 (table 10 funcref)
 ;; CHECK:      (table $0 10 funcref)

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (block $__inlined_func$1
 ;; CHECK-NEXT:   (block
 ;; CHECK-NEXT:    (call_indirect (type $T)
 ;; CHECK-NEXT:     (i32.const 42)
 ;; CHECK-NEXT:     (i32.const 0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (br $__inlined_func$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (br $__inlined_func$1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0
  (call $1)
 )
 (func $1
  (return_call_indirect (type $T)
   (i32.const 42)
   (i32.const 0)
  )
 )
)
(module
 ;; CHECK:      (type $6 (func))
 (type $6 (func))
 ;; CHECK:      (global $global$0 (mut i32) (i32.const 10))

 ;; CHECK:      (memory $0 1 1)
 (memory $0 1 1)
 (global $global$0 (mut i32) (i32.const 10))
 ;; CHECK:      (export "func_102_invoker" (func $19))
 (export "func_102_invoker" (func $19))
 (func $2 (; 2 ;) (type $6)
  (if
   (global.get $global$0)
   (return)
  )
  (global.set $global$0
   (i32.const 1)
  )
 )
 (func $13 (; 13 ;) (type $6)
  (if
   (global.get $global$0)
   (unreachable)
  )
  (return_call $2)
 )
 ;; CHECK:      (func $19
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (block
 ;; CHECK-NEXT:    (block $__inlined_func$13
 ;; CHECK-NEXT:     (block
 ;; CHECK-NEXT:      (if
 ;; CHECK-NEXT:       (global.get $global$0)
 ;; CHECK-NEXT:       (unreachable)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:      (block
 ;; CHECK-NEXT:       (block
 ;; CHECK-NEXT:        (block $__inlined_func$2
 ;; CHECK-NEXT:         (block
 ;; CHECK-NEXT:          (if
 ;; CHECK-NEXT:           (global.get $global$0)
 ;; CHECK-NEXT:           (br $__inlined_func$2)
 ;; CHECK-NEXT:          )
 ;; CHECK-NEXT:          (global.set $global$0
 ;; CHECK-NEXT:           (i32.const 1)
 ;; CHECK-NEXT:          )
 ;; CHECK-NEXT:         )
 ;; CHECK-NEXT:        )
 ;; CHECK-NEXT:        (br $__inlined_func$13)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (br $__inlined_func$13)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $19 (; 19 ;) (type $6)
  (call $13)
  (unreachable)
 )
)

(module
 ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

 ;; CHECK:      (export "is_even" (func $is_even))
 (export "is_even" (func $is_even))
 ;; CHECK:      (func $is_even (param $i i32) (result i32)
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (if (result i32)
 ;; CHECK-NEXT:   (i32.eqz
 ;; CHECK-NEXT:    (local.get $i)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (block $__inlined_func$is_odd (result i32)
 ;; CHECK-NEXT:     (local.set $1
 ;; CHECK-NEXT:      (i32.sub
 ;; CHECK-NEXT:       (local.get $i)
 ;; CHECK-NEXT:       (i32.const 1)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (if (result i32)
 ;; CHECK-NEXT:      (i32.eqz
 ;; CHECK-NEXT:       (local.get $1)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:      (return_call $is_even
 ;; CHECK-NEXT:       (i32.sub
 ;; CHECK-NEXT:        (local.get $1)
 ;; CHECK-NEXT:        (i32.const 1)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $is_even (param $i i32) (result i32)
  (if (result i32)
   (i32.eqz (local.get $i))
   (i32.const 1)
   (return_call $is_odd
    (i32.sub
     (local.get $i)
     (i32.const 1)
    )
   )
  )
 )
 (func $is_odd (param $i i32) (result i32)
  (if (result i32)
   (i32.eqz (local.get $i))
   (i32.const 0)
   (return_call $is_even
    (i32.sub
     (local.get $i)
     (i32.const 1)
    )
   )
  )
 )
)
