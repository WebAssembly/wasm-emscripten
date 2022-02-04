;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_test.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --fpcast-emu -S -o - | filecheck %s

(module
  ;; CHECK:      (type $func.0 (func (param i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64) (result i64)))

  ;; CHECK:      (type $func.1 (func (param i32 i64 f32 f64) (result f32)))

  ;; CHECK:      (type $vijfd (func (param i32 i64 f32 f64)))
  (type $vijfd (func (param i32) (param i64) (param f32) (param f64)))
  ;; CHECK:      (type $jii (func (param i32 i32) (result i64)))
  (type $jii (func (param i32) (param i32) (result i64)))
  ;; CHECK:      (type $fjj (func (param i64 i64) (result f32)))
  (type $fjj (func (param i64) (param i64) (result f32)))
  ;; CHECK:      (type $dff (func (param f32 f32) (result f64)))
  (type $dff (func (param f32) (param f32) (result f64)))
  ;; CHECK:      (type $idd (func (param f64 f64) (result i32)))
  (type $idd (func (param f64) (param f64) (result i32)))
  ;; CHECK:      (import "env" "imported_func" (func $imported-func (param i32 i64 f32 f64) (result f32)))
  (import "env" "imported_func" (func $imported-func (param i32 i64 f32 f64) (result f32)))
  (table 10 10 funcref)
  (elem (i32.const 0) $a $b $c $d $e $e $imported-func)
  ;; CHECK:      (table $0 10 10 funcref)

  ;; CHECK:      (elem (i32.const 0) $byn$fpcast-emu$a $byn$fpcast-emu$b $byn$fpcast-emu$c $byn$fpcast-emu$d $byn$fpcast-emu$e $byn$fpcast-emu$e $byn$fpcast-emu$imported-func)

  ;; CHECK:      (func $a (param $x i32) (param $y i64) (param $z f32) (param $w f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call_indirect (type $func.0)
  ;; CHECK-NEXT:    (i64.extend_i32_u
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.const 2)
  ;; CHECK-NEXT:    (i64.extend_i32_u
  ;; CHECK-NEXT:     (i32.reinterpret_f32
  ;; CHECK-NEXT:      (f32.const 3)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.reinterpret_f64
  ;; CHECK-NEXT:     (f64.const 4)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a (param $x i32) (param $y i64) (param $z f32) (param $w f64)
    (call_indirect (type $vijfd)
      (i32.const 1)
      (i64.const 2)
      (f32.const 3)
      (f64.const 4)
      (i32.const 1337)
    )
  )
  ;; CHECK:      (func $b (param $x i32) (param $y i32) (result i64)
  ;; CHECK-NEXT:  (call_indirect (type $func.0)
  ;; CHECK-NEXT:   (i64.extend_i32_u
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i64.extend_i32_u
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (i32.const 1337)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b (param $x i32) (param $y i32) (result i64)
    (call_indirect (type $jii)
      (i32.const 1)
      (i32.const 2)
      (i32.const 1337)
    )
  )
  ;; CHECK:      (func $c (param $x i64) (param $y i64) (result f32)
  ;; CHECK-NEXT:  (f32.reinterpret_i32
  ;; CHECK-NEXT:   (i32.wrap_i64
  ;; CHECK-NEXT:    (call_indirect (type $func.0)
  ;; CHECK-NEXT:     (i64.const 1)
  ;; CHECK-NEXT:     (i64.const 2)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i64.const 0)
  ;; CHECK-NEXT:     (i32.const 1337)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $c (param $x i64) (param $y i64) (result f32)
    (call_indirect (type $fjj)
      (i64.const 1)
      (i64.const 2)
      (i32.const 1337)
    )
  )
  ;; CHECK:      (func $d (param $x f32) (param $y f32) (result f64)
  ;; CHECK-NEXT:  (f64.reinterpret_i64
  ;; CHECK-NEXT:   (call_indirect (type $func.0)
  ;; CHECK-NEXT:    (i64.extend_i32_u
  ;; CHECK-NEXT:     (i32.reinterpret_f32
  ;; CHECK-NEXT:      (f32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.extend_i32_u
  ;; CHECK-NEXT:     (i32.reinterpret_f32
  ;; CHECK-NEXT:      (f32.const 2)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $d (param $x f32) (param $y f32) (result f64)
    (call_indirect (type $dff)
      (f32.const 1)
      (f32.const 2)
      (i32.const 1337)
    )
  )
  ;; CHECK:      (func $e (param $x f64) (param $y f64) (result i32)
  ;; CHECK-NEXT:  (i32.wrap_i64
  ;; CHECK-NEXT:   (call_indirect (type $func.0)
  ;; CHECK-NEXT:    (i64.reinterpret_f64
  ;; CHECK-NEXT:     (f64.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.reinterpret_f64
  ;; CHECK-NEXT:     (f64.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $e (param $x f64) (param $y f64) (result i32)
    (call_indirect (type $idd)
      (f64.const 1)
      (f64.const 2)
      (i32.const 1337)
    )
  )
)
;; CHECK:      (func $byn$fpcast-emu$a (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (call $a
;; CHECK-NEXT:   (i32.wrap_i64
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $1)
;; CHECK-NEXT:   (f32.reinterpret_i32
;; CHECK-NEXT:    (i32.wrap_i64
;; CHECK-NEXT:     (local.get $2)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (f64.reinterpret_i64
;; CHECK-NEXT:    (local.get $3)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (i64.const 0)
;; CHECK-NEXT: )

;; CHECK:      (func $byn$fpcast-emu$b (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (call $b
;; CHECK-NEXT:   (i32.wrap_i64
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (i32.wrap_i64
;; CHECK-NEXT:    (local.get $1)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $byn$fpcast-emu$c (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (i64.extend_i32_u
;; CHECK-NEXT:   (i32.reinterpret_f32
;; CHECK-NEXT:    (call $c
;; CHECK-NEXT:     (local.get $0)
;; CHECK-NEXT:     (local.get $1)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $byn$fpcast-emu$d (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (i64.reinterpret_f64
;; CHECK-NEXT:   (call $d
;; CHECK-NEXT:    (f32.reinterpret_i32
;; CHECK-NEXT:     (i32.wrap_i64
;; CHECK-NEXT:      (local.get $0)
;; CHECK-NEXT:     )
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (f32.reinterpret_i32
;; CHECK-NEXT:     (i32.wrap_i64
;; CHECK-NEXT:      (local.get $1)
;; CHECK-NEXT:     )
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $byn$fpcast-emu$e (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (i64.extend_i32_u
;; CHECK-NEXT:   (call $e
;; CHECK-NEXT:    (f64.reinterpret_i64
;; CHECK-NEXT:     (local.get $0)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (f64.reinterpret_i64
;; CHECK-NEXT:     (local.get $1)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $byn$fpcast-emu$imported-func (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (i64.extend_i32_u
;; CHECK-NEXT:   (i32.reinterpret_f32
;; CHECK-NEXT:    (call $imported-func
;; CHECK-NEXT:     (i32.wrap_i64
;; CHECK-NEXT:      (local.get $0)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (local.get $1)
;; CHECK-NEXT:     (f32.reinterpret_i32
;; CHECK-NEXT:      (i32.wrap_i64
;; CHECK-NEXT:       (local.get $2)
;; CHECK-NEXT:      )
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (f64.reinterpret_i64
;; CHECK-NEXT:      (local.get $3)
;; CHECK-NEXT:     )
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
(module
 (type $0 (func (param i64)))
 ;; CHECK:      (type $1 (func (param f32) (result i64)))
 (type $1 (func (param f32) (result i64)))
 ;; CHECK:      (type $func.0 (func (param i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64) (result i64)))

 ;; CHECK:      (global $global$0 (mut i32) (i32.const 10))
 (global $global$0 (mut i32) (i32.const 10))
 (table 42 42 funcref)
 ;; CHECK:      (table $0 42 42 funcref)

 ;; CHECK:      (export "func_106" (func $0))
 (export "func_106" (func $0))
 ;; CHECK:      (func $0 (param $0 f32) (result i64)
 ;; CHECK-NEXT:  (block $label$1 (result i64)
 ;; CHECK-NEXT:   (loop $label$2
 ;; CHECK-NEXT:    (global.set $global$0
 ;; CHECK-NEXT:     (i32.const 0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (call_indirect (type $func.0)
 ;; CHECK-NEXT:     (br $label$1
 ;; CHECK-NEXT:      (i64.const 4294967295)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i64.const 0)
 ;; CHECK-NEXT:     (i32.const 18)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0 (; 0 ;) (type $1) (param $0 f32) (result i64)
  (block $label$1 (result i64)
   (loop $label$2
    (global.set $global$0
     (i32.const 0)
    )
    (call_indirect (type $0)
     (br $label$1
      (i64.const 4294967295)
     )
     (i32.const 18)
    )
   )
  )
 )
)
(module
 (table 42 42 funcref)
 (elem (i32.const 0) $a $b)
 ;; CHECK:      (type $func.0 (func (param f32)))

 ;; CHECK:      (type $func.1 (func (param i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64 i64) (result i64)))

 ;; CHECK:      (type $func.2 (func (param f64)))

 ;; CHECK:      (table $0 42 42 funcref)

 ;; CHECK:      (elem (i32.const 0) $byn$fpcast-emu$a $byn$fpcast-emu$b)

 ;; CHECK:      (export "dynCall_vf" (func $dynCall_vf))
 (export "dynCall_vf" (func $dynCall_vf))
 ;; CHECK:      (export "dynCall_vd" (func $min_vd))
 (export "dynCall_vd" (func $min_vd))
 ;; CHECK:      (func $a (param $0 f32)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $a (param $0 f32))
 ;; CHECK:      (func $b (param $0 f64)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $b (param $0 f64))
 ;; CHECK:      (func $dynCall_vf (param $0 f32)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $dynCall_vf (param $0 f32))
 ;; CHECK:      (func $min_vd (param $0 f32)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $min_vd (param $0 f32))
)

;; CHECK:      (func $byn$fpcast-emu$a (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (call $a
;; CHECK-NEXT:   (f32.reinterpret_i32
;; CHECK-NEXT:    (i32.wrap_i64
;; CHECK-NEXT:     (local.get $0)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (i64.const 0)
;; CHECK-NEXT: )

;; CHECK:      (func $byn$fpcast-emu$b (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64) (param $4 i64) (param $5 i64) (param $6 i64) (param $7 i64) (param $8 i64) (param $9 i64) (param $10 i64) (param $11 i64) (param $12 i64) (param $13 i64) (param $14 i64) (param $15 i64) (result i64)
;; CHECK-NEXT:  (call $b
;; CHECK-NEXT:   (f64.reinterpret_i64
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (i64.const 0)
;; CHECK-NEXT: )
