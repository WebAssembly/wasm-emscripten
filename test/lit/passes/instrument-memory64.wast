;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --instrument-memory --enable-memory64 -S -o - | filecheck %s

(module
  (memory i64 256 256)
  ;; CHECK:      (type $1 (func))
  (type $1 (func))
  ;; CHECK:      (type $i32_i32_i64_i64_=>_i64 (func (param i32 i32 i64 i64) (result i64)))

  ;; CHECK:      (type $i32_i32_=>_i32 (func (param i32 i32) (result i32)))

  ;; CHECK:      (type $i32_i64_=>_i64 (func (param i32 i64) (result i64)))

  ;; CHECK:      (type $i32_f32_=>_f32 (func (param i32 f32) (result f32)))

  ;; CHECK:      (type $i32_f64_=>_f64 (func (param i32 f64) (result f64)))

  ;; CHECK:      (import "env" "load_ptr" (func $load_ptr (param i32 i32 i64 i64) (result i64)))

  ;; CHECK:      (import "env" "load_val_i32" (func $load_val_i32 (param i32 i32) (result i32)))

  ;; CHECK:      (import "env" "load_val_i64" (func $load_val_i64 (param i32 i64) (result i64)))

  ;; CHECK:      (import "env" "load_val_f32" (func $load_val_f32 (param i32 f32) (result f32)))

  ;; CHECK:      (import "env" "load_val_f64" (func $load_val_f64 (param i32 f64) (result f64)))

  ;; CHECK:      (import "env" "store_ptr" (func $store_ptr (param i32 i32 i64 i64) (result i64)))

  ;; CHECK:      (import "env" "store_val_i32" (func $store_val_i32 (param i32 i32) (result i32)))

  ;; CHECK:      (import "env" "store_val_i64" (func $store_val_i64 (param i32 i64) (result i64)))

  ;; CHECK:      (import "env" "store_val_f32" (func $store_val_f32 (param i32 f32) (result f32)))

  ;; CHECK:      (import "env" "store_val_f64" (func $store_val_f64 (param i32 f64) (result f64)))

  ;; CHECK:      (memory $0 i64 256 256)

  ;; CHECK:      (func $A
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.load8_s
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (i32.load8_u
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (i32.load16_s
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 3)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i32.load16_u
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 5)
  ;; CHECK-NEXT:    (i32.load
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 5)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 6)
  ;; CHECK-NEXT:    (i64.load8_s
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 6)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 7)
  ;; CHECK-NEXT:    (i64.load8_u
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 7)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:    (i64.load16_s
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 8)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 9)
  ;; CHECK-NEXT:    (i64.load16_u
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 9)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (i64.load32_s
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 10)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 11)
  ;; CHECK-NEXT:    (i64.load32_u
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 11)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 12)
  ;; CHECK-NEXT:    (i64.load
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 12)
  ;; CHECK-NEXT:      (i32.const 8)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_f32
  ;; CHECK-NEXT:    (i32.const 13)
  ;; CHECK-NEXT:    (f32.load
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 13)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_f64
  ;; CHECK-NEXT:    (i32.const 14)
  ;; CHECK-NEXT:    (f64.load
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 14)
  ;; CHECK-NEXT:      (i32.const 8)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 15)
  ;; CHECK-NEXT:    (i32.load8_s offset=1
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 15)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 1)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 16)
  ;; CHECK-NEXT:    (i32.load8_u offset=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 16)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 2)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 17)
  ;; CHECK-NEXT:    (i32.load16_s offset=3 align=1
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 17)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 3)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 18)
  ;; CHECK-NEXT:    (i32.load16_u offset=4 align=1
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 18)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 4)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i32
  ;; CHECK-NEXT:    (i32.const 19)
  ;; CHECK-NEXT:    (i32.load offset=5 align=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 19)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 5)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (i64.load8_s offset=6
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 20)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 6)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 21)
  ;; CHECK-NEXT:    (i64.load8_u offset=7
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 21)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (i64.const 7)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 22)
  ;; CHECK-NEXT:    (i64.load16_s offset=8 align=1
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 22)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 8)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 23)
  ;; CHECK-NEXT:    (i64.load16_u offset=9 align=1
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 23)
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i64.const 9)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 24)
  ;; CHECK-NEXT:    (i64.load32_s offset=10 align=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 24)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 10)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 25)
  ;; CHECK-NEXT:    (i64.load32_u offset=11 align=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 25)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 11)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_i64
  ;; CHECK-NEXT:    (i32.const 26)
  ;; CHECK-NEXT:    (i64.load offset=12 align=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 26)
  ;; CHECK-NEXT:      (i32.const 8)
  ;; CHECK-NEXT:      (i64.const 12)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_f32
  ;; CHECK-NEXT:    (i32.const 27)
  ;; CHECK-NEXT:    (f32.load offset=13 align=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 27)
  ;; CHECK-NEXT:      (i32.const 4)
  ;; CHECK-NEXT:      (i64.const 13)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $load_val_f64
  ;; CHECK-NEXT:    (i32.const 28)
  ;; CHECK-NEXT:    (f64.load offset=14 align=2
  ;; CHECK-NEXT:     (call $load_ptr
  ;; CHECK-NEXT:      (i32.const 28)
  ;; CHECK-NEXT:      (i32.const 8)
  ;; CHECK-NEXT:      (i64.const 14)
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $A (type $1)
    (drop (i32.load8_s (i64.const 0)))
    (drop (i32.load8_u (i64.const 0)))
    (drop (i32.load16_s (i64.const 0)))
    (drop (i32.load16_u (i64.const 0)))
    (drop (i32.load (i64.const 0)))
    (drop (i64.load8_s (i64.const 0)))
    (drop (i64.load8_u (i64.const 0)))
    (drop (i64.load16_s (i64.const 0)))
    (drop (i64.load16_u (i64.const 0)))
    (drop (i64.load32_s (i64.const 0)))
    (drop (i64.load32_u (i64.const 0)))
    (drop (i64.load (i64.const 0)))
    (drop (f32.load (i64.const 0)))
    (drop (f64.load (i64.const 0)))

    (drop (i32.load8_s align=1 offset=1 (i64.const 0)))
    (drop (i32.load8_u align=1 offset=2 (i64.const 0)))
    (drop (i32.load16_s align=1 offset=3 (i64.const 0)))
    (drop (i32.load16_u align=1 offset=4 (i64.const 0)))
    (drop (i32.load align=2 offset=5 (i64.const 0)))
    (drop (i64.load8_s align=1 offset=6 (i64.const 0)))
    (drop (i64.load8_u align=1 offset=7 (i64.const 0)))
    (drop (i64.load16_s align=1 offset=8 (i64.const 0)))
    (drop (i64.load16_u align=1 offset=9 (i64.const 0)))
    (drop (i64.load32_s align=2 offset=10 (i64.const 0)))
    (drop (i64.load32_u align=2 offset=11 (i64.const 0)))
    (drop (i64.load align=2 offset=12 (i64.const 0)))
    (drop (f32.load align=2 offset=13 (i64.const 0)))
    (drop (f64.load align=2 offset=14 (i64.const 0)))
  )

  ;; CHECK:      (func $B
  ;; CHECK-NEXT:  (i32.store8
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 29)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i32
  ;; CHECK-NEXT:    (i32.const 29)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store16
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i32
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 31)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i32
  ;; CHECK-NEXT:    (i32.const 31)
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store8
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 32)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 32)
  ;; CHECK-NEXT:    (i64.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store16
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 33)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 33)
  ;; CHECK-NEXT:    (i64.const 5)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store32
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 34)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 34)
  ;; CHECK-NEXT:    (i64.const 6)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 35)
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 35)
  ;; CHECK-NEXT:    (i64.const 7)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f32.store
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 36)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_f32
  ;; CHECK-NEXT:    (i32.const 36)
  ;; CHECK-NEXT:    (f32.const 8)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f64.store
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 37)
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_f64
  ;; CHECK-NEXT:    (i32.const 37)
  ;; CHECK-NEXT:    (f64.const 9)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store8 offset=1
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 38)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i64.const 1)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i32
  ;; CHECK-NEXT:    (i32.const 38)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store16 offset=2 align=1
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 39)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (i64.const 2)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i32
  ;; CHECK-NEXT:    (i32.const 39)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store offset=3 align=2
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 40)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i64.const 3)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i32
  ;; CHECK-NEXT:    (i32.const 40)
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store8 offset=4
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 41)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i64.const 4)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 41)
  ;; CHECK-NEXT:    (i64.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store16 offset=5
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:    (i64.const 5)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i64.const 5)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store32 offset=6 align=2
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 43)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i64.const 6)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 43)
  ;; CHECK-NEXT:    (i64.const 6)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store offset=7 align=2
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 44)
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:    (i64.const 7)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_i64
  ;; CHECK-NEXT:    (i32.const 44)
  ;; CHECK-NEXT:    (i64.const 7)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f32.store offset=8 align=2
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 45)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:    (i64.const 8)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_f32
  ;; CHECK-NEXT:    (i32.const 45)
  ;; CHECK-NEXT:    (f32.const 8)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f64.store offset=9 align=2
  ;; CHECK-NEXT:   (call $store_ptr
  ;; CHECK-NEXT:    (i32.const 46)
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:    (i64.const 9)
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $store_val_f64
  ;; CHECK-NEXT:    (i32.const 46)
  ;; CHECK-NEXT:    (f64.const 9)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $B (type $1)
    (i32.store8 (i64.const 0) (i32.const 1))
    (i32.store16 (i64.const 0) (i32.const 2))
    (i32.store (i64.const 0) (i32.const 3))
    (i64.store8 (i64.const 0) (i64.const 4))
    (i64.store16 (i64.const 0) (i64.const 5))
    (i64.store32 (i64.const 0) (i64.const 6))
    (i64.store (i64.const 0) (i64.const 7))
    (f32.store (i64.const 0) (f32.const 8))
    (f64.store (i64.const 0) (f64.const 9))

    (i32.store8 align=1 offset=1 (i64.const 0) (i32.const 1))
    (i32.store16 align=1 offset=2 (i64.const 0) (i32.const 2))
    (i32.store align=2 offset=3 (i64.const 0) (i32.const 3))
    (i64.store8 align=1 offset=4 (i64.const 0) (i64.const 4))
    (i64.store16 align=2 offset=5 (i64.const 0) (i64.const 5))
    (i64.store32 align=2 offset=6 (i64.const 0) (i64.const 6))
    (i64.store align=2 offset=7 (i64.const 0) (i64.const 7))
    (f32.store align=2 offset=8 (i64.const 0) (f32.const 8))
    (f64.store align=2 offset=9 (i64.const 0) (f64.const 9))
  )
)
