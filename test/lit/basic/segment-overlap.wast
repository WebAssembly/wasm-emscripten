;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s -all -o %t.text.wast -g -S
;; RUN: wasm-as %s -all -g -o %t.wasm
;; RUN: wasm-dis %t.wasm -all -o %t.bin.wast
;; RUN: wasm-as %s -all -o %t.nodebug.wasm
;; RUN: wasm-dis %t.nodebug.wasm -all -o %t.bin.nodebug.wast
;; RUN: cat %t.text.wast | filecheck %s --check-prefix=CHECK-TEXT
;; RUN: cat %t.bin.wast | filecheck %s --check-prefix=CHECK-BIN
;; RUN: cat %t.bin.nodebug.wast | filecheck %s --check-prefix=CHECK-BIN-NODEBUG

(module
 ;; CHECK-TEXT:      (memory $0 10)
 ;; CHECK-BIN:      (memory $0 10)
 (memory $0 10)
 (data (i32.const 100) "\ff\ff\ff\ff\ff\ff\ff\ff") ;; overlaps with the next
 (data (i32.const 104) "\00\00\00\00")
)
;; CHECK-TEXT:      (data $0 (i32.const 100) "\ff\ff\ff\ff\ff\ff\ff\ff")

;; CHECK-TEXT:      (data $1 (i32.const 104) "\00\00\00\00")

;; CHECK-BIN:      (data $0 (i32.const 100) "\ff\ff\ff\ff\ff\ff\ff\ff")

;; CHECK-BIN:      (data $1 (i32.const 104) "\00\00\00\00")

;; CHECK-BIN-NODEBUG:      (memory $m0 10)

;; CHECK-BIN-NODEBUG:      (data $0 (i32.const 100) "\ff\ff\ff\ff\ff\ff\ff\ff")

;; CHECK-BIN-NODEBUG:      (data $1 (i32.const 104) "\00\00\00\00")
