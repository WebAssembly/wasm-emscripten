;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s --enable-memory64 --enable-reference-types --enable-bulk-memory --table64-lowering -S -o - | filecheck %s

(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (type $1 (func (result i64)))

  ;; CHECK:      (table $t64 10 100 funcref)
  (table $t64 i64 10 100 funcref)

  ;; CHECK:      (table $t32 10 100 funcref)

  ;; CHECK:      (elem $elem64 (table $t64) (i32.const 0) funcref (item (ref.null nofunc)))
  (elem $elem64 (table $t64) (i64.const 0) funcref (ref.null func))

  (table $t32 10 100 funcref)
  ;; CHECK:      (elem $elem32 (table $t32) (i32.const 0) funcref (item (ref.null nofunc)))
  (elem $elem32 (table $t32) (i32.const 0) funcref (ref.null func))

  ;; CHECK:      (func $test_call_indirect
  ;; CHECK-NEXT:  (call_indirect $t64 (type $0)
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
  ;; CHECK-NEXT:   (table.size $t64)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test_table_size (result i64)
    (table.size $t64)
  )

  ;; CHECK:      (func $test_table_grow (result i64)
  ;; CHECK-NEXT:  (i64.extend_i32_u
  ;; CHECK-NEXT:   (table.grow $t64
  ;; CHECK-NEXT:    (ref.null nofunc)
  ;; CHECK-NEXT:    (i32.wrap_i64
  ;; CHECK-NEXT:     (i64.const 10)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test_table_grow (result i64)
    (table.grow $t64 (ref.null func) (i64.const 10))
  )

  ;; CHECK:      (func $test_table_fill
  ;; CHECK-NEXT:  (table.fill $t64
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
    (table.fill $t64 (i64.const 0) (ref.null func) (i64.const 10))
  )
)
