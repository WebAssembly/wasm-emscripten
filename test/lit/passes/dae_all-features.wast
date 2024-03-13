;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --dae --all-features -S -o - | filecheck %s

(module

  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (type $1 (func (param i32)))

  ;; CHECK:      (type $2 (func (result i32)))

  ;; CHECK:      (type $3 (func (result f64)))

  ;; CHECK:      (type $4 (func (param f64)))

  ;; CHECK:      (import "a" "b" (func $get-i32 (type $2) (result i32)))
  (import "a" "b" (func $get-i32 (result i32)))
  ;; CHECK:      (import "a" "c" (func $get-f64 (type $3) (result f64)))
  (import "a" "c" (func $get-f64 (result f64)))

  ;; CHECK:      (table $0 2 2 funcref)

  ;; CHECK:      (elem $0 (i32.const 0) $a9 $c8)

  ;; CHECK:      (export "a8" (func $a8))
  (export "a8" (func $a8))
  (table 2 2 funcref)
  (elem (i32.const 0) $a9 $c8)
  ;; CHECK:      (func $a (type $0)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a (param $x i32))
  ;; CHECK:      (func $b (type $0)
  ;; CHECK-NEXT:  (call $a)
  ;; CHECK-NEXT: )
  (func $b
    (call $a (i32.const 1)) ;; best case scenario
  )
  ;; CHECK:      (func $a1 (type $0)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $a1 (param $x i32)
    (unreachable)
  )
  ;; CHECK:      (func $b1 (type $0)
  ;; CHECK-NEXT:  (call $a1)
  ;; CHECK-NEXT: )
  (func $b1
    (call $a1 (i32.const 2)) ;; same value in both, so works
  )
  ;; CHECK:      (func $b11 (type $0)
  ;; CHECK-NEXT:  (call $a1)
  ;; CHECK-NEXT: )
  (func $b11
    (call $a1 (i32.const 2))
  )
  ;; CHECK:      (func $a2 (type $1) (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a2 (param $x i32)
    (drop (local.get $x))
  )
  ;; CHECK:      (func $b2 (type $0)
  ;; CHECK-NEXT:  (call $a2
  ;; CHECK-NEXT:   (i32.const 3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b2
    (call $a2 (i32.const 3)) ;; different value!
  )
  ;; CHECK:      (func $b22 (type $0)
  ;; CHECK-NEXT:  (call $a2
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b22
    (call $a2 (i32.const 4))
  )
  ;; CHECK:      (func $a3 (type $0)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const -1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a3 (param $x i32)
    (drop (i32.const -1)) ;; diff value, but at least unused, so no need to send
  )
  ;; CHECK:      (func $b3 (type $0)
  ;; CHECK-NEXT:  (call $a3)
  ;; CHECK-NEXT: )
  (func $b3
    (call $a3 (i32.const 3))
  )
  ;; CHECK:      (func $b33 (type $0)
  ;; CHECK-NEXT:  (call $a3)
  ;; CHECK-NEXT: )
  (func $b33
    (call $a3 (i32.const 4))
  )
  ;; CHECK:      (func $a4 (type $0)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a4 (param $x i32)
    ;; This function is called with one constant and one unreachable. We can
    ;; remove the param despite the unreachable's effects.
  )
  ;; CHECK:      (func $b4 (type $0)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $b4
    ;; This call will vanish entirely, because the unreachable child executes
    ;; first (so we cannot see here that we removed the parameter from $a4, but
    ;; that can be confirmed in $a4 itself).
    (call $a4 (unreachable))
  )
  ;; CHECK:      (func $b43 (type $0)
  ;; CHECK-NEXT:  (call $a4)
  ;; CHECK-NEXT: )
  (func $b43
    ;; We will remove the parameter here.
    (call $a4 (i32.const 4))
  )
  ;; CHECK:      (func $a5 (type $0)
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
  ;; CHECK:      (func $b5 (type $0)
  ;; CHECK-NEXT:  (call $a5)
  ;; CHECK-NEXT: )
  (func $b5
    (call $a5 (i32.const 1) (f64.const 3.14159))
  )
  ;; CHECK:      (func $a6 (type $1) (param $0 i32)
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
  ;; CHECK:      (func $b6 (type $0)
  ;; CHECK-NEXT:  (call $a6
  ;; CHECK-NEXT:   (call $get-i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b6
    (call $a6 (call $get-i32) (f64.const 3.14159))
  )
  ;; CHECK:      (func $a7 (type $4) (param $0 f64)
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
  ;; CHECK:      (func $b7 (type $0)
  ;; CHECK-NEXT:  (call $a7
  ;; CHECK-NEXT:   (call $get-f64)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b7
    (call $a7 (i32.const 1) (call $get-f64))
  )
  ;; CHECK:      (func $a8 (type $1) (param $x i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a8 (param $x i32)) ;; exported, do not optimize
  ;; CHECK:      (func $b8 (type $0)
  ;; CHECK-NEXT:  (call $a8
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b8
    (call $a8 (i32.const 1))
  )
  ;; CHECK:      (func $a9 (type $1) (param $x i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $a9 (param $x i32)) ;; tabled, do not optimize
  ;; CHECK:      (func $b9 (type $0)
  ;; CHECK-NEXT:  (call $a9
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b9
    (call $a9 (i32.const 1))
  )
  ;; CHECK:      (func $a10 (type $0)
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
  ;; CHECK:      (func $a11 (type $0)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (call $a11)
  ;; CHECK-NEXT:  (call $a11)
  ;; CHECK-NEXT: )
  (func $a11 (param $x i32) ;; partially successful recursion
    (call $a11 (i32.const 1))
    (call $a11 (i32.const 2))
  )
  ;; CHECK:      (func $a12 (type $1) (param $x i32)
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
  ;; CHECK:      (func $c1 (type $0)
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
  ;; CHECK:      (func $c2 (type $0)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c2 (result i32)
    (i32.const 1)
  )
  ;; CHECK:      (func $c3 (type $0)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c3 (result i32)
    (i32.const 2)
  )
  ;; CHECK:      (func $c4 (type $2) (result i32)
  ;; CHECK-NEXT:  (i32.const 3)
  ;; CHECK-NEXT: )
  (func $c4 (result i32)
    (i32.const 3)
  )
  ;; CHECK:      (func $c5 (type $1) (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c5 (param $x i32) (result i32)
    (local.get $x)
  )
  ;; CHECK:      (func $c6 (type $0)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $c6 (result i32)
    (unreachable)
  )
  ;; CHECK:      (func $c7 (type $0)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return)
  ;; CHECK-NEXT: )
  (func $c7 (result i32)
    (return (i32.const 4))
  )
  ;; CHECK:      (func $c8 (type $2) (result i32)
  ;; CHECK-NEXT:  (i32.const 5)
  ;; CHECK-NEXT: )
  (func $c8 (result i32)
    (i32.const 5)
  )
)
(module ;; both operations at once: remove params and return value
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (export "a" (func $a))

  ;; CHECK:      (func $a (type $0)
  ;; CHECK-NEXT:  (call $b)
  ;; CHECK-NEXT: )
  (func $a (export "a")
    (drop
      (call $b
        (i32.const 1)
      )
    )
  )
  ;; CHECK:      (func $b (type $0)
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
  ;; CHECK:      (type $0 (func (param i32) (result i32)))

  ;; CHECK:      (type $1 (func (result i32)))

  ;; CHECK:      (func $foo (type $0) (param $x i32) (result i32)
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
  ;; CHECK:      (func $bar (type $1) (result i32)
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
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (table $0 1 1 funcref)

  ;; CHECK:      (func $foo (type $T) (result i32)
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
  ;; CHECK:      (func $bar (type $1)
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
 ;; CHECK:      (type $0 (func (param funcref i32 f64) (result i64)))

 ;; CHECK:      (type $1 (func (param f32) (result funcref)))

 ;; CHECK:      (elem declare func $0)

 ;; CHECK:      (export "export" (func $export))

 ;; CHECK:      (func $0 (type $0) (param $0 funcref) (param $1 i32) (param $2 f64) (result i64)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $0 (param $0 funcref) (param $1 i32) (param $2 f64) (result i64)
  (nop)
  (unreachable)
 )
 ;; CHECK:      (func $export (type $1) (param $0 f32) (result funcref)
 ;; CHECK-NEXT:  (ref.func $0)
 ;; CHECK-NEXT: )
 (func $export (export "export") (param $0 f32) (result funcref)
  ;; a ref.func should prevent us from changing the type of a function, as it
  ;; may escape
  (ref.func $0)
 )
)
(module
 ;; CHECK:      (type $i64 (func (param i64)))
 (type $i64 (func (param i64)))
 ;; CHECK:      (type $1 (func))

 ;; CHECK:      (global $global$0 (ref $i64) (ref.func $0))
 (global $global$0 (ref $i64) (ref.func $0))
 ;; CHECK:      (export "even" (func $1))
 (export "even" (func $1))
 ;; the argument to this function cannot be removed due to the ref.func of it
 ;; in a global
 ;; CHECK:      (func $0 (type $i64) (param $0 i64)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $0 (param $0 i64)
  (unreachable)
 )
 ;; CHECK:      (func $1 (type $1)
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
 ;; CHECK:      (func $2 (type $1)
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
 ;; CHECK:      (type $0 (func))

 ;; CHECK:      (func $0 (type $0)
 ;; CHECK-NEXT:  (local $0 i31ref)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $0 (param $x i31ref)
  (nop)
 )
 ;; CHECK:      (func $1 (type $0)
 ;; CHECK-NEXT:  (call $0)
 ;; CHECK-NEXT: )
 (func $1
  (call $0
   (ref.i31 (i32.const 0))
  )
 )
)

;; Arguments that read an immutable global can be optimized, as that is a
;; constant value.
(module
 ;; CHECK:      (type $0 (func))

 ;; CHECK:      (type $1 (func (param i32)))

 ;; CHECK:      (type $2 (func (param i32 i32)))

 ;; CHECK:      (global $immut i32 (i32.const 42))
 (global $immut i32 (i32.const 42))

 ;; CHECK:      (global $immut2 i32 (i32.const 43))
 (global $immut2 i32 (i32.const 43))

 ;; CHECK:      (global $mut (mut i32) (i32.const 1337))
 (global $mut (mut i32) (i32.const 1337))

 ;; CHECK:      (func $foo (type $1) (param $0 i32)
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

 ;; CHECK:      (func $foo-caller (type $0)
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

 ;; CHECK:      (func $bar (type $2) (param $x i32) (param $y i32)
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

 ;; CHECK:      (func $bar-caller (type $0)
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

(module
 ;; CHECK:      (type $0 (func))

 ;; CHECK:      (func $0 (type $0)
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (local.set $0
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block
 ;; CHECK-NEXT:     (block
 ;; CHECK-NEXT:      (drop
 ;; CHECK-NEXT:       (i32.const 1)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:      (return)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (return)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0 (param $0 i32) (result i32)
  ;; The returns here are nested in each other, and one is a recursive call to
  ;; this function itself, which makes this a corner case we might emit invalid
  ;; code for. We end up removing the parameter, and then the call vanishes as
  ;; it was unreachable; we also remove the return as well as it is dropped in
  ;; the other caller, below.
  (return
   (drop
    (call $0
     (return
      (i32.const 1)
     )
    )
   )
  )
 )

 ;; CHECK:      (func $other-call (type $0)
 ;; CHECK-NEXT:  (call $0)
 ;; CHECK-NEXT: )
 (func $other-call
  (drop
   (call $0
    (i32.const 1)
   )
  )
 )
)

(module
 ;; CHECK:      (type $A (func (result (ref $A))))
 (type $A (func (result (ref $A))))

 ;; CHECK:      (type $1 (func))

 ;; CHECK:      (func $no-caller (type $A) (result (ref $A))
 ;; CHECK-NEXT:  (block ;; (replaces unreachable CallRef we can't emit)
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (ref.null nofunc)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $no-caller (type $A) (result (ref $A))
  ;; This return_call is to a bottom type, which we should ignore and not error
  ;; on. There is nothing to optimize here (other passes will turn this call
  ;; into an unreachable). In particular we should not be confused by the fact
  ;; that this expression itself is unreachable (as a return call).
  (return_call_ref $A
   (ref.null nofunc)
  )
 )

 ;; CHECK:      (func $caller (type $1)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $no-caller)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $caller
  (drop
   (call $no-caller)
  )
 )
)

(module
 ;; CHECK:      (type $0 (func (param f64) (result i32)))

 ;; CHECK:      (func $target (type $0) (param $0 f64) (result i32)
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (local $2 i32)
 ;; CHECK-NEXT:  (local $3 i32)
 ;; CHECK-NEXT:  (local $4 i32)
 ;; CHECK-NEXT:  (local.set $1
 ;; CHECK-NEXT:   (call $target
 ;; CHECK-NEXT:    (f64.const 1.1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $2
 ;; CHECK-NEXT:   (call $target
 ;; CHECK-NEXT:    (f64.const 4.4)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $target
 ;; CHECK-NEXT:   (local.get $0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $target (param $a i32) (param $b f64) (param $c i32) (result i32)
  ;; Test removing a parameter despite calls having interesting non-unreachable
  ;; effects. This also tests recursion of such calls. We can remove all the i32
  ;; parameters here.
  (call $target
   (call $target
    (i32.const 0)
    (f64.const 1.1)
    (i32.const 2)
   )
   (local.get $b)
   (call $target
    (i32.const 3)
    (f64.const 4.4)
    (i32.const 5)
   )
  )
 )
)

(module
 ;; CHECK:      (type $0 (func))

 ;; CHECK:      (type $v128 (func (result v128)))
 (type $v128 (func (result v128)))

 ;; CHECK:      (type $2 (func (result f32)))

 ;; CHECK:      (table $0 10 funcref)
 (table $0 10 funcref)

 ;; CHECK:      (func $caller-effects (type $0)
 ;; CHECK-NEXT:  (local $0 v128)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result f32)
 ;; CHECK-NEXT:    (local.set $0
 ;; CHECK-NEXT:     (call_indirect $0 (type $v128)
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (call $target)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $caller-effects
  (drop
   (call $target
    (i64.const 0)
    ;; We'd like to remove this unused parameter, but it has effects, so we'll
    ;; move it to a local first.
    (call_indirect $0 (type $v128)
     (i32.const 0)
    )
    (i64.const 0)
   )
  )
 )

 ;; CHECK:      (func $target (type $2) (result f32)
 ;; CHECK-NEXT:  (local $0 i64)
 ;; CHECK-NEXT:  (local $1 i64)
 ;; CHECK-NEXT:  (local $2 v128)
 ;; CHECK-NEXT:  (local.set $0
 ;; CHECK-NEXT:   (i64.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $1
 ;; CHECK-NEXT:    (i64.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $target (param $0 i64) (param $1 v128) (param $2 i64) (result f32)
  ;; All parameters here should vanish.
  (unreachable)
 )
)

(module
 ;; CHECK:      (type $0 (func (param i32)))

 ;; CHECK:      (type $1 (func (param i32 i64) (result f32)))

 ;; CHECK:      (func $caller-later-br (type $0) (param $x i32)
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (block $block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block
 ;; CHECK-NEXT:     (local.set $1
 ;; CHECK-NEXT:      (block (result i32)
 ;; CHECK-NEXT:       (if
 ;; CHECK-NEXT:        (local.get $x)
 ;; CHECK-NEXT:        (then
 ;; CHECK-NEXT:         (return)
 ;; CHECK-NEXT:        )
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:       (i32.const 42)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (br $block)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $caller-later-br (param $x i32)
  (block $block
   (drop
    (call $target
     (i64.const 0)
     ;; We'd like to remove this unused parameter, and we can do so by moving
     ;; it to a local, but we need to be careful: the br right after us must be
     ;; kept around, as it is the only thing that makes the outer block have
     ;; type none and not unreachable.
     (block (result i32)
       (if
         (local.get $x)
         (then
           (return)
         )
       )
       (i32.const 42)
     )
     ;; We won't remove this unreachable param (we leave it for DCE, first).
     (br $block)
    )
   )
  )
 )

 ;; CHECK:      (func $target (type $1) (param $0 i32) (param $1 i64) (result f32)
 ;; CHECK-NEXT:  (local $2 i64)
 ;; CHECK-NEXT:  (local.set $2
 ;; CHECK-NEXT:   (i64.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $target (param $0 i64) (param $1 i32) (param $2 i64) (result f32)
  ;; The i32 parameter should vanish.
  (unreachable)
 )
)
