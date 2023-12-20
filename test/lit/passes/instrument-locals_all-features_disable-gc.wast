;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --instrument-locals --all-features --disable-gc -S -o - | filecheck %s

(module
  ;; CHECK:      (type $0 (func (param i32 i32 i32) (result i32)))

  ;; CHECK:      (type $1 (func (param i32 i32 i64) (result i64)))

  ;; CHECK:      (type $2 (func (param i32 i32 f32) (result f32)))

  ;; CHECK:      (type $3 (func (param i32 i32 f64) (result f64)))

  ;; CHECK:      (type $4 (func (param i32 i32 funcref) (result funcref)))

  ;; CHECK:      (type $5 (func (param i32 i32 externref) (result externref)))

  ;; CHECK:      (type $6 (func (param i32 i32 v128) (result v128)))

  ;; CHECK:      (type $7 (func))

  ;; CHECK:      (import "env" "get_i32" (func $get_i32 (param i32 i32 i32) (result i32)))

  ;; CHECK:      (import "env" "get_i64" (func $get_i64 (param i32 i32 i64) (result i64)))

  ;; CHECK:      (import "env" "get_f32" (func $get_f32 (param i32 i32 f32) (result f32)))

  ;; CHECK:      (import "env" "get_f64" (func $get_f64 (param i32 i32 f64) (result f64)))

  ;; CHECK:      (import "env" "set_i32" (func $set_i32 (param i32 i32 i32) (result i32)))

  ;; CHECK:      (import "env" "set_i64" (func $set_i64 (param i32 i32 i64) (result i64)))

  ;; CHECK:      (import "env" "set_f32" (func $set_f32 (param i32 i32 f32) (result f32)))

  ;; CHECK:      (import "env" "set_f64" (func $set_f64 (param i32 i32 f64) (result f64)))

  ;; CHECK:      (import "env" "get_funcref" (func $get_funcref (param i32 i32 funcref) (result funcref)))

  ;; CHECK:      (import "env" "set_funcref" (func $set_funcref (param i32 i32 funcref) (result funcref)))

  ;; CHECK:      (import "env" "get_externref" (func $get_externref (param i32 i32 externref) (result externref)))

  ;; CHECK:      (import "env" "set_externref" (func $set_externref (param i32 i32 externref) (result externref)))

  ;; CHECK:      (import "env" "get_v128" (func $get_v128 (param i32 i32 v128) (result v128)))

  ;; CHECK:      (import "env" "set_v128" (func $set_v128 (param i32 i32 v128) (result v128)))

  ;; CHECK:      (elem declare func $test)

  ;; CHECK:      (func $test
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i64)
  ;; CHECK-NEXT:  (local $z f32)
  ;; CHECK-NEXT:  (local $w f64)
  ;; CHECK-NEXT:  (local $F funcref)
  ;; CHECK-NEXT:  (local $X externref)
  ;; CHECK-NEXT:  (local $S v128)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_i32
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_f32
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (local.get $z)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_f64
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (local.get $w)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_funcref
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (local.get $F)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_externref
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i32.const 5)
  ;; CHECK-NEXT:    (local.get $X)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_i32
  ;; CHECK-NEXT:    (i32.const 5)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_f32
  ;; CHECK-NEXT:    (i32.const 6)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (local.get $z)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_f64
  ;; CHECK-NEXT:    (i32.const 7)
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (local.get $w)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_funcref
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (local.get $F)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_externref
  ;; CHECK-NEXT:    (i32.const 9)
  ;; CHECK-NEXT:    (i32.const 5)
  ;; CHECK-NEXT:    (local.get $X)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $set_i32
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (i64.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (call $set_f32
  ;; CHECK-NEXT:    (i32.const 11)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (f32.const 3.2100000381469727)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $w
  ;; CHECK-NEXT:   (call $set_f64
  ;; CHECK-NEXT:    (i32.const 12)
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (f64.const 4.321)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $F
  ;; CHECK-NEXT:   (ref.func $test)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $X
  ;; CHECK-NEXT:   (call $set_externref
  ;; CHECK-NEXT:    (i32.const 14)
  ;; CHECK-NEXT:    (i32.const 5)
  ;; CHECK-NEXT:    (call $get_externref
  ;; CHECK-NEXT:     (i32.const 13)
  ;; CHECK-NEXT:     (i32.const 5)
  ;; CHECK-NEXT:     (local.get $X)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $set_i32
  ;; CHECK-NEXT:    (i32.const 15)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 11)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (i64.const 22)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (call $set_f32
  ;; CHECK-NEXT:    (i32.const 16)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (f32.const 33.209999084472656)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $w
  ;; CHECK-NEXT:   (call $set_f64
  ;; CHECK-NEXT:    (i32.const 17)
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (f64.const 44.321)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $F
  ;; CHECK-NEXT:   (call $set_funcref
  ;; CHECK-NEXT:    (i32.const 19)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (call $get_funcref
  ;; CHECK-NEXT:     (i32.const 18)
  ;; CHECK-NEXT:     (i32.const 4)
  ;; CHECK-NEXT:     (local.get $F)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $X
  ;; CHECK-NEXT:   (call $set_externref
  ;; CHECK-NEXT:    (i32.const 21)
  ;; CHECK-NEXT:    (i32.const 5)
  ;; CHECK-NEXT:    (call $get_externref
  ;; CHECK-NEXT:     (i32.const 20)
  ;; CHECK-NEXT:     (i32.const 5)
  ;; CHECK-NEXT:     (local.get $X)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $get_v128
  ;; CHECK-NEXT:    (i32.const 22)
  ;; CHECK-NEXT:    (i32.const 6)
  ;; CHECK-NEXT:    (local.get $S)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $S
  ;; CHECK-NEXT:   (call $set_v128
  ;; CHECK-NEXT:    (i32.const 23)
  ;; CHECK-NEXT:    (i32.const 6)
  ;; CHECK-NEXT:    (v128.const i32x4 0x00000000 0x00000001 0x00000002 0x00000003)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (local $x i32)
    (local $y i64)
    (local $z f32)
    (local $w f64)
    (local $F funcref)
    (local $X externref)
    (local $S v128)

    (drop (local.get $x))
    (drop (local.get $y))
    (drop (local.get $z))
    (drop (local.get $w))
    (drop (local.get $F))
    (drop (local.get $X))

    (drop (local.get $x))
    (drop (local.get $y))
    (drop (local.get $z))
    (drop (local.get $w))
    (drop (local.get $F))
    (drop (local.get $X))

    (local.set $x (i32.const 1))
    (local.set $y (i64.const 2))
    (local.set $z (f32.const 3.21))
    (local.set $w (f64.const 4.321))
    (local.set $F (ref.func $test))
    (local.set $X (local.get $X))

    (local.set $x (i32.const 11))
    (local.set $y (i64.const 22))
    (local.set $z (f32.const 33.21))
    (local.set $w (f64.const 44.321))
    (local.set $F (local.get $F))
    (local.set $X (local.get $X))

    ;; Add new instructions here so expected output doesn't change too much, it
    ;; depends on order of instructions in this file.
    (drop (local.get $S))
    (local.set $S (v128.const i32x4 0x00000000 0x00000001 0x00000002 0x00000003))
  )
)
