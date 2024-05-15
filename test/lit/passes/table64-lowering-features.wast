;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s --enable-memory64 --enable-reference-types --enable-bulk-memory --table64-lowering -S -o - | filecheck %s

(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (type $1 (func (result i64)))

  ;; CHECK:      (table $t 10 100 funcref)
  (table $t i64 10 100 funcref)

  ;; CHECK:      (elem $elem (table $t) (i32.const 0) funcref (ref.null nofunc))
  (elem $elem (table $t) (i64.const 0) funcref (ref.null func))

  ;; CHECK:      (func $test_call_indirect
  ;; CHECK-NEXT:  (call_indirect $t (type $0)
  ;; CHECK-NEXT:   (i32.wrap_i64
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test_call_indirect
    (call_indirect 0 (i64.const 0))
  )

  ;; CHECK:      (func $test_table_size (result i64)
  ;; CHECK-NEXT:  (i64.extend_i32_u
  ;; CHECK-NEXT:   (table.size $t)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test_table_size (result i64)
    (table.size $t)
  )

  ;; CHECK:      (func $test_table_grow (result i64)
  ;; CHECK-NEXT:  (i64.extend_i32_u
  ;; CHECK-NEXT:   (table.grow $t
  ;; CHECK-NEXT:    (ref.null nofunc)
  ;; CHECK-NEXT:    (i32.wrap_i64
  ;; CHECK-NEXT:     (i64.const 10)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test_table_grow (result i64)
    (table.grow $t (ref.null func) (i64.const 10))
  )

  ;; CHECK:      (func $test_table_fill
  ;; CHECK-NEXT:  (table.fill $t
  ;; CHECK-NEXT:   (i32.wrap_i64
  ;; CHECK-NEXT:    (i64.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (ref.null nofunc)
  ;; CHECK-NEXT:   (i32.wrap_i64
  ;; CHECK-NEXT:    (i64.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test_table_fill
    (table.fill $t (i64.const 0) (ref.null func) (i64.const 10))
  )
)
