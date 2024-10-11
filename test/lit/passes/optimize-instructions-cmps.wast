;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-instructions -S -o - | filecheck %s

;; Tests for "x < 0 || x > POSITIVE_CONST   ==>   x > POSITIVE_CONST (unsigned comparison)" optimization

(module
 ;; CHECK:      (global $g (mut i32) (i32.const 0))
 (global $g (mut i32) (i32.const 0))

 ;; CHECK:      (func $cmp (param $0 i32) (result i32)
 ;; CHECK-NEXT:  (i32.gt_u
 ;; CHECK-NEXT:   (local.get $0)
 ;; CHECK-NEXT:   (i32.const 255)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $cmp (param $0 i32) (result i32)
  (i32.or
   (i32.lt_s
    (local.get $0)
    (i32.const 0)
   )
   (i32.gt_s
    (local.get $0)
    (i32.const 255)
   )
  )
 )

 ;; CHECK:      (func $cmp2 (param $0 i32) (result i32)
 ;; CHECK-NEXT:  (i32.or
 ;; CHECK-NEXT:   (i32.lt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.gt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (i32.const -255)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $cmp2 (param $0 i32) (result i32)
  (i32.or
   (i32.lt_s
    (local.get $0)
    (i32.const 0)
   )
   (i32.gt_s
    (local.get $0)
    (i32.const -255) ;; negative number
   )
  )
 )

 ;; CHECK:      (func $cmp3 (param $0 i32) (result i32)
 ;; CHECK-NEXT:  (i32.or
 ;; CHECK-NEXT:   (i32.lt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (i32.const 10)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.gt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (i32.const 255)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $cmp3 (param $0 i32) (result i32)
  (i32.or
   (i32.lt_s
    (local.get $0)
    (i32.const 10) ;; note this
   )
   (i32.gt_s
    (local.get $0)
    (i32.const 255)
   )
  )
 )

 ;; CHECK:      (func $set_global_and_return (param $x i32) (result i32)
 ;; CHECK-NEXT:  (global.set $g
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.get $x)
 ;; CHECK-NEXT: )
 (func $set_global_and_return (param $x i32) (result i32)
   (global.set $g (local.get $x)) ;; side-effect
   (local.get $x)
 )

 ;; CHECK:      (func $cmp4 (result i32)
 ;; CHECK-NEXT:  (i32.or
 ;; CHECK-NEXT:   (i32.lt_s
 ;; CHECK-NEXT:    (call $set_global_and_return
 ;; CHECK-NEXT:     (i32.const 10)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.gt_s
 ;; CHECK-NEXT:    (call $set_global_and_return
 ;; CHECK-NEXT:     (i32.const 10)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (i32.const 255)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $cmp4 (result i32)
  (i32.or
   (i32.lt_s
    (call $set_global_and_return (i32.const 10)) ;; x with side-effect
    (i32.const 0)
   )
   (i32.gt_s
    (call $set_global_and_return (i32.const 10))
    (i32.const 255)
   )
  )
 )

 ;; CHECK:      (func $cmp5 (param $0 i32) (param $1 i32) (result i32)
 ;; CHECK-NEXT:  (i32.or
 ;; CHECK-NEXT:   (i32.lt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.gt_s
 ;; CHECK-NEXT:    (local.get $1)
 ;; CHECK-NEXT:    (i32.const 255)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $cmp5 (param $0 i32) (param $1 i32) (result i32)
  (i32.or
   (i32.lt_s
    (local.get $0)
    (i32.const 0)
   )
   (i32.gt_s
    (local.get $1) ;; note this
    (i32.const 255)
   )
  )
 )

 ;; CHECK:      (func $cmp6 (param $0 i32) (param $1 i32) (result i32)
 ;; CHECK-NEXT:  (i32.or
 ;; CHECK-NEXT:   (i32.lt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.gt_s
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:    (local.get $1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $cmp6 (param $0 i32) (param $1 i32) (result i32)
  (i32.or
   (i32.lt_s
    (local.get $0)
    (i32.const 0)
   )
   (i32.gt_s
    (local.get $0)
    (local.get $1) ;; non-constant
   )
  )
 )
)
