;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-split %s -all --split-funcs=second-in-table -g -o1 %t.1.wasm -o2 %t.2.wasm
;; RUN: wasm-dis -all %t.1.wasm | filecheck %s --check-prefix PRIMARY
;; RUN: wasm-dis -all %t.2.wasm | filecheck %s --check-prefix SECONDARY

(module
 (type $func-array (array (mut funcref)))

 ;; PRIMARY:      (type $0 (func))

 ;; PRIMARY:      (import "placeholder" "0" (func $placeholder_0 (type $0)))

 ;; PRIMARY:      (table $table 3 funcref)
 (table $table 3 funcref)

 ;; PRIMARY:      (table $t1 1 funcref)

 ;; PRIMARY:      (elem $passive func $in-table $f1)
 (elem $passive func $in-table $second-in-table)

 ;; PRIMARY:      (elem $1 (table $t1) (i32.const 0) func $placeholder_0)

 ;; PRIMARY:      (export "table" (table $table))

 ;; PRIMARY:      (export "table_1" (table $t1))

 ;; PRIMARY:      (func $in-table (type $0)
 ;; PRIMARY-NEXT:  (nop)
 ;; PRIMARY-NEXT: )
 (func $in-table
  ;; This is in a passive segment, but it is in the main module so we need no
  ;; special handling.
 )

 ;; SECONDARY:      (type $0 (func))

 ;; SECONDARY:      (import "primary" "table_1" (table $timport$0 1 funcref))

 ;; SECONDARY:      (import "primary" "table" (table $table 3 funcref))

 ;; SECONDARY:      (elem $0 (table $timport$0) (i32.const 0) func $second-in-table)

 ;; SECONDARY:      (func $second-in-table (type $0)
 ;; SECONDARY-NEXT:  (nop)
 ;; SECONDARY-NEXT: )
 (func $second-in-table
  ;; This is in a passive segment, and it is in the secondary module, so we will
  ;; handle it by adding a trampoline from the segment as a new function "$1".
 )
)
;; PRIMARY:      (func $f1 (type $0)
;; PRIMARY-NEXT:  (call_indirect $t1 (type $0)
;; PRIMARY-NEXT:   (i32.const 0)
;; PRIMARY-NEXT:  )
;; PRIMARY-NEXT: )
