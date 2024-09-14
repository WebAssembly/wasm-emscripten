;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-split -all -g --multi-split %s --manifest %s.manifest --out-prefix=%t -o %t.wasm
;; RUN: wasm-dis %t.wasm | filecheck --check-prefix=PRIMARY
;; RUN: wasm-dis %t1.wasm | filecheck --check-prefix=CHECK-A
;; RUN: wasm-dis %t2.wasm | filecheck --check-prefix=CHECK-B
;; RUN: wasm-dis %t3.wasm | filecheck --check-prefix=CHECK-C

(module
 (type $ret-i32 (func (result i32)))
 ;; PRIMARY:      (type $ret-i64 (func (result i64)))
 (type $ret-i64 (func (result i64)))
 ;; PRIMARY:      (type $ret-f32 (func (result f32)))
 (type $ret-f32 (func (result f32)))
 ;; CHECK-A:      (type $0 (func (result i64)))

 ;; CHECK-A:      (type $1 (func (result f32)))

 ;; CHECK-A:      (type $2 (func (result i32)))

 ;; CHECK-A:      (import "" "table" (table $timport$0 1 funcref))

 ;; CHECK-A:      (import "" "a" (func $B (result i64)))

 ;; CHECK-A:      (import "" "b" (func $C (result f32)))

 ;; CHECK-A:      (elem $0 (i32.const 0) $A)

 ;; CHECK-A:      (func $A (result i32)
 ;; CHECK-A-NEXT:  (drop
 ;; CHECK-A-NEXT:   (call_ref $2
 ;; CHECK-A-NEXT:    (ref.func $A)
 ;; CHECK-A-NEXT:   )
 ;; CHECK-A-NEXT:  )
 ;; CHECK-A-NEXT:  (drop
 ;; CHECK-A-NEXT:   (call_ref $0
 ;; CHECK-A-NEXT:    (ref.func $B)
 ;; CHECK-A-NEXT:   )
 ;; CHECK-A-NEXT:  )
 ;; CHECK-A-NEXT:  (drop
 ;; CHECK-A-NEXT:   (call_ref $1
 ;; CHECK-A-NEXT:    (ref.func $C)
 ;; CHECK-A-NEXT:   )
 ;; CHECK-A-NEXT:  )
 ;; CHECK-A-NEXT:  (i32.const 0)
 ;; CHECK-A-NEXT: )
 (func $A (type $ret-i32) (result i32)
  (drop
   (call_ref $ret-i32
    (ref.func $A)
   )
  )
  (drop
   (call_ref $ret-i64
    (ref.func $B)
   )
  )
  (drop
   (call_ref $ret-f32
    (ref.func $C)
   )
  )
  (i32.const 0)
 )
 ;; CHECK-B:      (type $0 (func (result i32)))

 ;; CHECK-B:      (type $1 (func (result f32)))

 ;; CHECK-B:      (type $2 (func (result i64)))

 ;; CHECK-B:      (import "" "table_3" (table $timport$0 2 funcref))

 ;; CHECK-B:      (import "" "table" (table $timport$1 1 funcref))

 ;; CHECK-B:      (import "" "b" (func $C (result f32)))

 ;; CHECK-B:      (elem $0 (table $timport$0) (i32.const 0) func $B $1)

 ;; CHECK-B:      (func $B (result i64)
 ;; CHECK-B-NEXT:  (drop
 ;; CHECK-B-NEXT:   (call_ref $0
 ;; CHECK-B-NEXT:    (ref.func $1)
 ;; CHECK-B-NEXT:   )
 ;; CHECK-B-NEXT:  )
 ;; CHECK-B-NEXT:  (drop
 ;; CHECK-B-NEXT:   (call_ref $2
 ;; CHECK-B-NEXT:    (ref.func $B)
 ;; CHECK-B-NEXT:   )
 ;; CHECK-B-NEXT:  )
 ;; CHECK-B-NEXT:  (drop
 ;; CHECK-B-NEXT:   (call_ref $1
 ;; CHECK-B-NEXT:    (ref.func $C)
 ;; CHECK-B-NEXT:   )
 ;; CHECK-B-NEXT:  )
 ;; CHECK-B-NEXT:  (i64.const 0)
 ;; CHECK-B-NEXT: )
 (func $B (type $ret-i64) (result i64)
  (drop
   (call_ref $ret-i32
    (ref.func $A)
   )
  )
  (drop
   (call_ref $ret-i64
    (ref.func $B)
   )
  )
  (drop
   (call_ref $ret-f32
    (ref.func $C)
   )
  )
  (i64.const 0)
 )
 ;; CHECK-C:      (type $0 (func (result i64)))

 ;; CHECK-C:      (type $1 (func (result i32)))

 ;; CHECK-C:      (type $2 (func (result f32)))

 ;; CHECK-C:      (import "" "table_4" (table $timport$0 2 funcref))

 ;; CHECK-C:      (import "" "table_3" (table $timport$1 2 funcref))

 ;; CHECK-C:      (elem $0 (table $timport$0) (i32.const 0) func $0 $C)

 ;; CHECK-C:      (func $0 (result i64)
 ;; CHECK-C-NEXT:  (call_indirect (type $0)
 ;; CHECK-C-NEXT:   (i32.const 0)
 ;; CHECK-C-NEXT:  )
 ;; CHECK-C-NEXT: )

 ;; CHECK-C:      (func $C (result f32)
 ;; CHECK-C-NEXT:  (drop
 ;; CHECK-C-NEXT:   (call_ref $1
 ;; CHECK-C-NEXT:    (ref.func $3)
 ;; CHECK-C-NEXT:   )
 ;; CHECK-C-NEXT:  )
 ;; CHECK-C-NEXT:  (drop
 ;; CHECK-C-NEXT:   (call_ref $0
 ;; CHECK-C-NEXT:    (ref.func $2)
 ;; CHECK-C-NEXT:   )
 ;; CHECK-C-NEXT:  )
 ;; CHECK-C-NEXT:  (drop
 ;; CHECK-C-NEXT:   (call_ref $2
 ;; CHECK-C-NEXT:    (ref.func $C)
 ;; CHECK-C-NEXT:   )
 ;; CHECK-C-NEXT:  )
 ;; CHECK-C-NEXT:  (f32.const 0)
 ;; CHECK-C-NEXT: )
 (func $C (type $ret-f32) (result f32)
  (drop
   (call_ref $ret-i32
    (ref.func $A)
   )
  )
  (drop
   (call_ref $ret-i64
    (ref.func $B)
   )
  )
  (drop
   (call_ref $ret-f32
    (ref.func $C)
   )
  )
  (f32.const 0)
 )
)
;; PRIMARY:      (table $0 1 funcref)

;; PRIMARY:      (table $1 2 funcref)

;; PRIMARY:      (table $2 2 funcref)

;; PRIMARY:      (elem $0 (table $0) (i32.const 0) funcref (item (ref.null nofunc)))

;; PRIMARY:      (elem $1 (table $1) (i32.const 0) funcref (item (ref.null nofunc)) (item (ref.null nofunc)))

;; PRIMARY:      (elem $2 (table $2) (i32.const 0) funcref (item (ref.null nofunc)) (item (ref.null nofunc)))

;; PRIMARY:      (export "a" (func $0))

;; PRIMARY:      (export "b" (func $1))

;; PRIMARY:      (export "table" (table $0))

;; PRIMARY:      (export "table_3" (table $1))

;; PRIMARY:      (export "table_4" (table $2))

;; PRIMARY:      (func $0 (result i64)
;; PRIMARY-NEXT:  (call_indirect (type $ret-i64)
;; PRIMARY-NEXT:   (i32.const 0)
;; PRIMARY-NEXT:  )
;; PRIMARY-NEXT: )

;; PRIMARY:      (func $1 (result f32)
;; PRIMARY-NEXT:  (call_indirect (type $ret-f32)
;; PRIMARY-NEXT:   (i32.const 1)
;; PRIMARY-NEXT:  )
;; PRIMARY-NEXT: )

;; CHECK-B:      (func $1 (result i32)
;; CHECK-B-NEXT:  (call_indirect (type $0)
;; CHECK-B-NEXT:   (i32.const 0)
;; CHECK-B-NEXT:  )
;; CHECK-B-NEXT: )

;; CHECK-C:      (func $2 (result i64)
;; CHECK-C-NEXT:  (call_indirect (type $0)
;; CHECK-C-NEXT:   (i32.const 0)
;; CHECK-C-NEXT:  )
;; CHECK-C-NEXT: )

;; CHECK-C:      (func $3 (result i32)
;; CHECK-C-NEXT:  (call_indirect (type $1)
;; CHECK-C-NEXT:   (i32.const 1)
;; CHECK-C-NEXT:  )
;; CHECK-C-NEXT: )
