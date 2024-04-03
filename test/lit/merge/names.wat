;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: wasm-merge -g %s first %s.second second -all -o %t.wasm
;; RUN: wasm-opt -all %t.wasm -S -o - | filecheck %s
(module
   ;; CHECK:      (type $0 (func))

   ;; CHECK:      (type $t (struct (field $a i32) (field $b i32)))

   ;; CHECK:      (type $2 (func (param (ref $t))))

   ;; CHECK:      (type $3 (struct (field i64) (field i32)))

   ;; CHECK:      (type $4 (func (param (ref $3))))

   ;; CHECK:      (global $global$0 i32 (i32.const 0))

   ;; CHECK:      (global $global$1 i32 (i32.const 0))

   ;; CHECK:      (global $global$2 i32 (i32.const 0))

   ;; CHECK:      (global $glob0 i32 (i32.const 0))

   ;; CHECK:      (memory $mem0 0)

   ;; CHECK:      (memory $1 0)

   ;; CHECK:      (memory $mem2 0)

   ;; CHECK:      (memory $3 0)

   ;; CHECK:      (table $table0 1 funcref)

   ;; CHECK:      (table $1 1 funcref)

   ;; CHECK:      (table $table2 1 funcref)

   ;; CHECK:      (table $3 1 funcref)

   ;; CHECK:      (tag $tag0)

   ;; CHECK:      (tag $tag$1)

   ;; CHECK:      (tag $tag$2)

   ;; CHECK:      (tag $tag$3)

   ;; CHECK:      (export "m0" (memory $mem0))

   ;; CHECK:      (export "m1" (memory $1))

   ;; CHECK:      (export "f0" (func $func0))

   ;; CHECK:      (export "f1" (func $1))

   ;; CHECK:      (export "t0" (table $table0))

   ;; CHECK:      (export "t1" (table $1))

   ;; CHECK:      (export "g0" (global $glob0))

   ;; CHECK:      (export "g1" (global $global$2))

   ;; CHECK:      (export "tag0" (tag $tag0))

   ;; CHECK:      (export "tag1" (tag $tag$1))

   ;; CHECK:      (export "func" (func $2))

   ;; CHECK:      (export "m2" (memory $mem2))

   ;; CHECK:      (export "m3" (memory $3))

   ;; CHECK:      (export "f2" (func $func2))

   ;; CHECK:      (export "f3" (func $1_3))

   ;; CHECK:      (export "t2" (table $table2))

   ;; CHECK:      (export "t3" (table $3))

   ;; CHECK:      (export "g2" (global $global$1))

   ;; CHECK:      (export "g3" (global $global$0))

   ;; CHECK:      (export "tag2" (tag $tag$2))

   ;; CHECK:      (export "tag3" (tag $tag$3))

   ;; CHECK:      (export "func2" (func $2_3))

   ;; CHECK:      (func $func0 (type $0)
   ;; CHECK-NEXT:  (nop)
   ;; CHECK-NEXT: )
   (func $func0 (export "f0"))
   (func (export "f1"))

   (table $table0 (export "t0") 1 funcref)
   (table (export "t1") 1 funcref)

   (global $glob0 (export g0) i32 (i32.const 0))
   (global (export g1) i32 (i32.const 0))

   (memory $mem0 (export "m0") 0)
   (memory (export "m1") 0)

   (elem $elem0 func)
   (elem func)

   (data $data0 "")
   (data "")

   (tag $tag0 (export tag0))
   (tag (export tag1))

   (type $t (struct (field $a i32) (field $b i32)))

   (func (export "func") (param $x (ref $t)))
)
;; CHECK:      (func $1 (type $0)
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )

;; CHECK:      (func $2 (type $2) (param $x (ref $t))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )

;; CHECK:      (func $func2 (type $0)
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )

;; CHECK:      (func $1_3 (type $0)
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )

;; CHECK:      (func $2_3 (type $4) (param $0 (ref $3))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
