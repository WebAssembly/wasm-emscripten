;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --dae --all-features -S -o - | filecheck %s

(module

  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $i32_=>_none (func (param i32)))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (type $none_=>_f64 (func (result f64)))

  ;; CHECK:      (type $f64_=>_none (func (param f64)))

  ;; CHECK:      (import "a" "b" (func $get-i32 (result i32)))
  (import "a" "b" (func $get-i32 (result i32)))
  ;; CHECK:      (import "a" "c" (func $get-f64 (result f64)))
  (import "a" "c" (func $get-f64 (result f64)))

  ;; CHECK:      (table $0 2 2 funcref)

  ;; CHECK:      (elem (i32.const 0) $a9 $c8)

  ;; CHECK:      (export "a8" (func $a8))
  (export "a8" (func $a8))
  (table 2 2 funcref)
  (elem (i32.const 0) $a9 $c8)
  ;; CHECK:      (func $a
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a (param $x i32))
  ;; CHECK:      (func $b
  ;; CHECK-NEXT:  (call $a)
  ;; CHECK-NEXT: )
  (func $b
    (call $a (i32.const 1)) ;; best case scenario
  )
  ;; CHECK:      (func $a1
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $a1 (param $x i32)
    (unreachable)
  )
  ;; CHECK:      (func $b1
  ;; CHECK-NEXT:  (call $a1)
  ;; CHECK-NEXT: )
  (func $b1
    (call $a1 (i32.const 2)) ;; same value in both, so works
  )
  ;; CHECK:      (func $b11
  ;; CHECK-NEXT:  (call $a1)
  ;; CHECK-NEXT: )
  (func $b11
    (call $a1 (i32.const 2))
  )
  ;; CHECK:      (func $a2 (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a2 (param $x i32)
    (drop (local.get $x))
  )
  ;; CHECK:      (func $b2
  ;; CHECK-NEXT:  (call $a2
  ;; CHECK-NEXT:   (i32.const 3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b2
    (call $a2 (i32.const 3)) ;; different value!
  )
  ;; CHECK:      (func $b22
  ;; CHECK-NEXT:  (call $a2
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b22
    (call $a2 (i32.const 4))
  )
  ;; CHECK:      (func $a3
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const -1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a3 (param $x i32)
    (drop (i32.const -1)) ;; diff value, but at least unused, so no need to send
  )
  ;; CHECK:      (func $b3
  ;; CHECK-NEXT:  (call $a3)
  ;; CHECK-NEXT: )
  (func $b3
    (call $a3 (i32.const 3))
  )
  ;; CHECK:      (func $b33
  ;; CHECK-NEXT:  (call $a3)
  ;; CHECK-NEXT: )
  (func $b33
    (call $a3 (i32.const 4))
  )
  ;; CHECK:      (func $a4 (param $x i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a4 (param $x i32) ;; diff value, but with effects
  )
  ;; CHECK:      (func $b4
  ;; CHECK-NEXT:  (call $a4
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b4
    (call $a4 (unreachable))
  )
  ;; CHECK:      (func $b43
  ;; CHECK-NEXT:  (call $a4
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b43
    (call $a4 (i32.const 4))
  )
  ;; CHECK:      (func $a5
  ;; CHECK-NEXT:  (local $0 f64)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (f64.const 3.14159)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $1
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (block
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a5 (param $x i32) (param $y f64) ;; optimize two
    (drop (local.get $x))
    (drop (local.get $y))
  )
  ;; CHECK:      (func $b5
  ;; CHECK-NEXT:  (call $a5)
  ;; CHECK-NEXT: )
  (func $b5
    (call $a5 (i32.const 1) (f64.const 3.14159))
  )
  ;; CHECK:      (func $a6 (param $0 i32)
  ;; CHECK-NEXT:  (local $1 f64)
  ;; CHECK-NEXT:  (local.set $1
  ;; CHECK-NEXT:   (f64.const 3.14159)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a6 (param $x i32) (param $y f64) ;; optimize just one
    (drop (local.get $x))
    (drop (local.get $y))
  )
  ;; CHECK:      (func $b6
  ;; CHECK-NEXT:  (call $a6
  ;; CHECK-NEXT:   (call $get-i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b6
    (call $a6 (call $get-i32) (f64.const 3.14159))
  )
  ;; CHECK:      (func $a7 (param $0 f64)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local.set $1
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a7 (param $x i32) (param $y f64) ;; optimize just the other one
    (drop (local.get $x))
    (drop (local.get $y))
  )
  ;; CHECK:      (func $b7
  ;; CHECK-NEXT:  (call $a7
  ;; CHECK-NEXT:   (call $get-f64)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b7
    (call $a7 (i32.const 1) (call $get-f64))
  )
  ;; CHECK:      (func $a8 (param $x i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a8 (param $x i32)) ;; exported, do not optimize
  ;; CHECK:      (func $b8
  ;; CHECK-NEXT:  (call $a8
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b8
    (call $a8 (i32.const 1))
  )
  ;; CHECK:      (func $a9 (param $x i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a9 (param $x i32)) ;; tabled, do not optimize
  ;; CHECK:      (func $b9
  ;; CHECK-NEXT:  (call $a9
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b9
    (call $a9 (i32.const 1))
  )
  ;; CHECK:      (func $a10
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (call $a10)
  ;; CHECK-NEXT:   (call $a10)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a10 (param $x i32) ;; recursion
    (call $a10 (i32.const 1))
    (call $a10 (i32.const 1))
  )
  ;; CHECK:      (func $a11
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (call $a11)
  ;; CHECK-NEXT:  (call $a11)
  ;; CHECK-NEXT: )
  (func $a11 (param $x i32) ;; partially successful recursion
    (call $a11 (i32.const 1))
    (call $a11 (i32.const 2))
  )
  ;; CHECK:      (func $a12 (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $a12
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $a12
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a12 (param $x i32) ;; unsuccessful recursion
    (drop (local.get $x))
    (call $a12 (i32.const 1))
    (call $a12 (i32.const 2))
  )
  ;; return values
  ;; CHECK:      (func $c1
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (call $c2)
  ;; CHECK-NEXT:  (call $c3)
  ;; CHECK-NEXT:  (call $c3)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $c4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $c4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $c5
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $c6)
  ;; CHECK-NEXT:  (call $c7)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $c8)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c1
    (local $x i32)
    (drop (call $c2))
    (drop (call $c3))
    (drop (call $c3))
    (drop (call $c4))
    (local.set $x (call $c4))
    (drop (call $c5 (unreachable)))
    (drop (call $c6))
    (drop (call $c7))
    (drop (call $c8))
  )
  ;; CHECK:      (func $c2
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c2 (result i32)
    (i32.const 1)
  )
  ;; CHECK:      (func $c3
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c3 (result i32)
    (i32.const 2)
  )
  ;; CHECK:      (func $c4 (result i32)
  ;; CHECK-NEXT:  (i32.const 3)
  ;; CHECK-NEXT: )
  (func $c4 (result i32)
    (i32.const 3)
  )
  ;; CHECK:      (func $c5 (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c5 (param $x i32) (result i32)
    (local.get $x)
  )
  ;; CHECK:      (func $c6
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $c6 (result i32)
    (unreachable)
  )
  ;; CHECK:      (func $c7
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return)
  ;; CHECK-NEXT: )
  (func $c7 (result i32)
    (return (i32.const 4))
  )
  ;; CHECK:      (func $c8 (result i32)
  ;; CHECK-NEXT:  (i32.const 5)
  ;; CHECK-NEXT: )
  (func $c8 (result i32)
    (i32.const 5)
  )
)
(module ;; both operations at once: remove params and return value
  (func "a"
    (drop
      (call $b
        (i32.const 1)
      )
    )
  )
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (export "a" (func $0))

  ;; CHECK:      (func $0
  ;; CHECK-NEXT:  (call $b)
  ;; CHECK-NEXT: )

  ;; CHECK:      (func $b
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b (param $x i32) (result i32)
    (local.get $x)
  )
)
(module ;; tail calls inhibit dropped result removal
  ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

  ;; CHECK:      (type $none_=>_i32 (func (result i32)))

  ;; CHECK:      (func $foo (param $x i32) (result i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (return_call $bar)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: )
  (func $foo (param $x i32) (result i32)
    (drop
      (return_call $bar
        (i32.const 0)
      )
    )
    (i32.const 42)
  )
  ;; CHECK:      (func $bar (result i32)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.const 7)
  ;; CHECK-NEXT: )
  (func $bar (param $x i32) (result i32)
    (i32.const 7)
  )
)
(module ;; indirect tail calls inhibit dropped result removal
  ;; CHECK:      (type $T (func (result i32)))
  (type $T (func (result i32)))
  (table 1 1 funcref)
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (table $0 1 1 funcref)

  ;; CHECK:      (func $foo (result i32)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (return_call_indirect $0 (type $T)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo (param $x i32) (result i32)
    (drop
      (return_call_indirect (type $T)
        (i32.const 0)
      )
    )
  )
  ;; CHECK:      (func $bar
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $foo)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $bar
    (drop
      (call $foo
        (i32.const 42)
      )
    )
  )
)
(module
 ;; CHECK:      (type $funcref_i32_f64_=>_i64 (func (param funcref i32 f64) (result i64)))

 ;; CHECK:      (type $f32_=>_funcref (func (param f32) (result funcref)))

 ;; CHECK:      (elem declare func $0)

 ;; CHECK:      (export "export" (func $1))

 ;; CHECK:      (func $0 (param $0 funcref) (param $1 i32) (param $2 f64) (result i64)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $0 (param $0 funcref) (param $1 i32) (param $2 f64) (result i64)
  (nop)
  (unreachable)
 )
 (func "export" (param $0 f32) (result funcref)
  ;; a ref.func should prevent us from changing the type of a function, as it
  ;; may escape
  (ref.func $0)
 )
)
;; CHECK:      (func $1 (param $0 f32) (result funcref)
;; CHECK-NEXT:  (ref.func $0)
;; CHECK-NEXT: )
(module
 ;; CHECK:      (type $i64 (func (param i64)))
 (type $i64 (func (param i64)))
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (global $global$0 (ref $i64) (ref.func $0))
 (global $global$0 (ref $i64) (ref.func $0))
 ;; CHECK:      (export "even" (func $1))
 (export "even" (func $1))
 ;; the argument to this function cannot be removed due to the ref.func of it
 ;; in a global
 ;; CHECK:      (func $0 (param $0 i64)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $0 (param $0 i64)
  (unreachable)
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (call_ref $i64
 ;; CHECK-NEXT:   (i64.const 0)
 ;; CHECK-NEXT:   (global.get $global$0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $1
  (call_ref $i64
   (i64.const 0)
   (global.get $global$0)
  )
 )
 ;; CHECK:      (func $2
 ;; CHECK-NEXT:  (call $0
 ;; CHECK-NEXT:   (i64.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $2
  (call $0
   (i64.const 0)
  )
 )
)
(module
 ;; a removable non-nullable parameter
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (local $0 i31ref)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $0 (param $x i31ref)
  (nop)
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (call $0)
 ;; CHECK-NEXT: )
 (func $1
  (call $0
   (i31.new (i32.const 0))
  )
 )
)

;; Arguments that read an immutable global can be optimized, as that is a
;; constant value.
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $i32_=>_none (func (param i32)))

 ;; CHECK:      (type $i32_i32_=>_none (func (param i32 i32)))

 ;; CHECK:      (global $immut i32 (i32.const 42))
 (global $immut i32 (i32.const 42))

 ;; CHECK:      (global $immut2 i32 (i32.const 43))
 (global $immut2 i32 (i32.const 43))

 ;; CHECK:      (global $mut (mut i32) (i32.const 1337))
 (global $mut (mut i32) (i32.const 1337))

 ;; CHECK:      (func $foo (param $0 i32)
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (local.set $1
 ;; CHECK-NEXT:   (global.get $immut)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo (param $x i32) (param $y i32)
  ;; "Use" the params to avoid other optimizations kicking in.
  (drop (local.get $x))
  (drop (local.get $y))
 )

 ;; CHECK:      (func $foo-caller
 ;; CHECK-NEXT:  (global.set $mut
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $foo
 ;; CHECK-NEXT:   (global.get $mut)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (global.set $mut
 ;; CHECK-NEXT:   (i32.const 2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $foo
 ;; CHECK-NEXT:   (global.get $mut)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo-caller
  ;; Note how the mutable param has a different value in each call, which shows
  ;; the reason that we cannot optimize in this case. But we can optimize the
  ;; immutable param.
  (global.set $mut (i32.const 1))
  (call $foo
   (global.get $immut)
   (global.get $mut)
  )
  (global.set $mut (i32.const 2))
  (call $foo
   (global.get $immut)
   (global.get $mut)
  )
 )

 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (drop (local.get $x))
  (drop (local.get $y))
 )

 ;; CHECK:      (func $bar-caller
 ;; CHECK-NEXT:  (global.set $mut
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $bar
 ;; CHECK-NEXT:   (global.get $immut)
 ;; CHECK-NEXT:   (global.get $immut)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (global.set $mut
 ;; CHECK-NEXT:   (i32.const 2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $bar
 ;; CHECK-NEXT:   (global.get $mut)
 ;; CHECK-NEXT:   (global.get $immut2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $bar-caller
  ;; Corner cases of mixing mutable with immutable and mixing two immutables.
  (global.set $mut (i32.const 1))
  (call $bar
   (global.get $immut)
   (global.get $immut)
  )
  (global.set $mut (i32.const 2))
  (call $bar
   (global.get $mut)
   (global.get $immut2)
  )
 )
)
