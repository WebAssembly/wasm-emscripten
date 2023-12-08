;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s -all -o %t.text.wast -g -S
;; RUN: wasm-as %s -all -g -o %t.wasm
;; RUN: wasm-dis %t.wasm -all -o %t.bin.wast
;; RUN: wasm-as %s -all -o %t.nodebug.wasm
;; RUN: wasm-dis %t.nodebug.wasm -all -o %t.bin.nodebug.wast
;; RUN: cat %t.text.wast | filecheck %s --check-prefix=CHECK-TEXT
;; RUN: cat %t.bin.wast | filecheck %s --check-prefix=CHECK-BIN
;; RUN: cat %t.bin.nodebug.wast | filecheck %s --check-prefix=CHECK-BIN-NODEBUG

(module ;; tests duplicate types are named properly
 (type (func))
 (type (func))
 (type (func))

 (type (func (param i32)))

 ;; CHECK-TEXT:      (type $0 (func (param i32)))
 ;; CHECK-BIN:      (type $0 (func (param i32)))
 ;; CHECK-BIN-NODEBUG:      (type $0 (func (param i32)))
 (type $0 (func (param i32)))

 (type (func (param i32)))

 (type $b (func (param i32) (result f32)))

 (type (func (param i32) (result f32)))

 ;; CHECK-TEXT:      (type $1 (func (param i32) (result i32)))

 ;; CHECK-TEXT:      (func $f0 (type $0) (param $0 i32)
 ;; CHECK-TEXT-NEXT:  (nop)
 ;; CHECK-TEXT-NEXT: )
 ;; CHECK-BIN:      (type $1 (func (param i32) (result i32)))

 ;; CHECK-BIN:      (func $f0 (type $0) (param $0 i32)
 ;; CHECK-BIN-NEXT:  (nop)
 ;; CHECK-BIN-NEXT: )
 (func $f0 (param i32))

 ;; CHECK-TEXT:      (func $f1 (type $1) (param $0 i32) (result i32)
 ;; CHECK-TEXT-NEXT:  (i32.const 0)
 ;; CHECK-TEXT-NEXT: )
 ;; CHECK-BIN:      (func $f1 (type $1) (param $0 i32) (result i32)
 ;; CHECK-BIN-NEXT:  (i32.const 0)
 ;; CHECK-BIN-NEXT: )
 (func $f1 (param i32) (result i32)
  (i32.const 0)
 )
)
;; CHECK-BIN-NODEBUG:      (type $1 (func (param i32) (result i32)))

;; CHECK-BIN-NODEBUG:      (func $0 (type $0) (param $0 i32)
;; CHECK-BIN-NODEBUG-NEXT:  (nop)
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $1 (type $1) (param $0 i32) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (i32.const 0)
;; CHECK-BIN-NODEBUG-NEXT: )
