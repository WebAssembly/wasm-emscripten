;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_test.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --inlining-optimizing --enable-threads -S -o - | filecheck %s

(module
  (table 1 1 funcref)
  (elem (i32.const 0) $tabled)
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (table $0 1 1 funcref)

  ;; CHECK:      (elem (i32.const 0) $tabled)

  ;; CHECK:      (export "user" (func $user))

  ;; CHECK:      (export "exported" (func $exported))

  ;; CHECK:      (export "exported_small" (func $exported_small))

  ;; CHECK:      (func $user
  ;; CHECK-NEXT:  (call $exported)
  ;; CHECK-NEXT:  (call $tabled)
  ;; CHECK-NEXT:  (call $multi)
  ;; CHECK-NEXT:  (call $multi)
  ;; CHECK-NEXT: )
  (func $user (export "user")
    (local $x i32)
    (local $y f64)
    (call $exported)
    (call $exported_small)
    (call $tabled)
    (call $tabled_small)
    (call $multi)
    (call $multi)
    (call $multi_small)
    (call $multi_small)
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
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $exported (export "exported")
    (nop)
    (nop)
  )
  ;; CHECK:      (func $exported_small
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $exported_small (export "exported_small")
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
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $tabled
    (nop)
    (nop)
  )
  (func $tabled_small
    (nop)
  )
  ;; CHECK:      (func $cycle1
  ;; CHECK-NEXT:  (call $cycle1)
  ;; CHECK-NEXT: )
  (func $cycle1
    (call $cycle2)
  )
  (func $cycle2
    (call $cycle1)
  )
  ;; CHECK:      (func $multi
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $multi
    (nop)
    (nop)
  )
  (func $multi_small
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
 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (func $main (result i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $main (result i32)
  (call $func_51)
  (i32.const 0)
 )
 (func $func_51
  (unreachable) ;; void function but having unreachable body, when inlined, type must be fixed
 )
)
(module
 ;; CHECK:      (type $none_=>_i64 (func (result i64)))

 ;; CHECK:      (memory $0 (shared 1 1))
 (memory $0 (shared 1 1))
 (func $0 (result i32)
  (i32.atomic.store16
   (i32.const 0)
   (i32.const 0)
  )
  (i32.const 1)
 )
 ;; CHECK:      (func $1 (result i64)
 ;; CHECK-NEXT:  (i32.atomic.store16 $0
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (i64.const 0)
 ;; CHECK-NEXT: )
 (func $1 (result i64)
  (drop
   (call $0)
  )
  (i64.const 0)
 )
)
;; potential infinite recursion
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $main
 ;; CHECK-NEXT:  (call $one)
 ;; CHECK-NEXT:  (call $one)
 ;; CHECK-NEXT: )
 (func $main
  (call $one)
  (call $one)
 )
 ;; CHECK:      (func $one
 ;; CHECK-NEXT:  (call $one)
 ;; CHECK-NEXT: )
 (func $one
  (call $one)
 )
)
;; potential infinite cycling recursion
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $main
 ;; CHECK-NEXT:  (call $two)
 ;; CHECK-NEXT:  (call $two)
 ;; CHECK-NEXT: )
 (func $main
  (call $one)
  (call $one)
 )
 (func $one
  (call $two)
 )
 ;; CHECK:      (func $two
 ;; CHECK-NEXT:  (call $two)
 ;; CHECK-NEXT: )
 (func $two
  (call $one)
 )
)
;; make sure to dce, as we may be combining unreachable code with others
(module
 ;; CHECK:      (type $0 (func))
 (type $0 (func))
 (type $1 (func (param i32 i32) (result i32)))
 (table 89 89 funcref)
 ;; CHECK:      (memory $0 17)
 (memory $0 17)
 ;; CHECK:      (table $0 89 89 funcref)

 ;; CHECK:      (start $1)
 (start $1)
 (func $0 (; 0 ;) (type $1) (param $0 i32) (param $1 i32) (result i32)
  (i32.store
   (i32.const 4)
   (local.tee $0
    (i32.const 0)
   )
  )
  (i32.store
   (i32.add
    (local.get $0)
    (i32.const 56)
   )
   (i32.const 0)
  )
  (i64.store
   (i32.const 49)
   (i64.load
    (i32.const 24)
   )
  )
  (unreachable)
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (i32.store $0
 ;; CHECK-NEXT:   (i32.const 4)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (i32.store $0
 ;; CHECK-NEXT:   (i32.const 56)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (i64.store $0
 ;; CHECK-NEXT:   (i32.const 49)
 ;; CHECK-NEXT:   (i64.load $0
 ;; CHECK-NEXT:    (i32.const 24)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $1 (; 1 ;) (type $0)
  (drop
   (call $0
    (i32.const 0)
    (i32.const 0)
   )
  )
 )
)
