;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-split %s --split-funcs=second -g -o1 %t.1.wasm -o2 %t.2.wasm -all | filecheck %s
;; RUN: wasm-dis %t.1.wasm | filecheck %s --check-prefix PRIMARY
;; RUN: wasm-dis %t.2.wasm | filecheck %s --check-prefix SECONDARY

;; Test that we handle ref.func operations properly as we split out $second.
;; ref.funcs that refer to the other module must be fixed up to refer to
;; something in the same module, that then trampolines to the other.
(module
 ;; PRIMARY:      (type $0 (func))

 ;; PRIMARY:      (import "placeholder" "1" (func $placeholder_1))

 ;; PRIMARY:      (global $glob1 (ref func) (ref.func $prime))

 ;; PRIMARY:      (global $glob2 (ref func) (ref.func $2))

 ;; PRIMARY:      (table $table 2 2 funcref)
 (table $table 1 1 funcref)

 (global $glob1 (ref func) (ref.func $prime))

 (global $glob2 (ref func) (ref.func $second))

 (elem (i32.const 0) $in-table)

 ;; PRIMARY:      (elem $0 (i32.const 0) $in-table $placeholder_1)

 ;; PRIMARY:      (export "prime" (func $prime))

 ;; PRIMARY:      (export "table" (table $table))

 ;; PRIMARY:      (export "global" (global $glob1))

 ;; PRIMARY:      (export "global_3" (global $glob2))

 ;; PRIMARY:      (func $prime
 ;; PRIMARY-NEXT:  (drop
 ;; PRIMARY-NEXT:   (ref.func $prime)
 ;; PRIMARY-NEXT:  )
 ;; PRIMARY-NEXT:  (drop
 ;; PRIMARY-NEXT:   (ref.func $2)
 ;; PRIMARY-NEXT:  )
 ;; PRIMARY-NEXT: )
 (func $prime
  (drop
   (ref.func $prime)
  )
  (drop
   (ref.func $second)
  )
 )

 ;; SECONDARY:      (type $0 (func))

 ;; SECONDARY:      (import "primary" "table" (table $table 2 2 funcref))

 ;; SECONDARY:      (import "primary" "global" (global $glob1 (ref func)))

 ;; SECONDARY:      (import "primary" "global_3" (global $glob2 (ref func)))

 ;; SECONDARY:      (import "primary" "prime" (func $prime))

 ;; SECONDARY:      (elem $0 (i32.const 1) $second)

 ;; SECONDARY:      (func $second
 ;; SECONDARY-NEXT:  (drop
 ;; SECONDARY-NEXT:   (ref.func $prime)
 ;; SECONDARY-NEXT:  )
 ;; SECONDARY-NEXT:  (drop
 ;; SECONDARY-NEXT:   (ref.func $second)
 ;; SECONDARY-NEXT:  )
 ;; SECONDARY-NEXT: )
 (func $second
  (drop
   (ref.func $prime)
  )
  (drop
   (ref.func $second)
  )
 )

 ;; PRIMARY:      (func $in-table
 ;; PRIMARY-NEXT:  (nop)
 ;; PRIMARY-NEXT: )
 (func $in-table
  ;; This empty function is in the table. Just being present in the table is not
  ;; enough of a reason for us to make a trampoline, even though in our IR the
  ;; table is a list of ref.funcs.
 )
)
;; PRIMARY:      (func $2
;; PRIMARY-NEXT:  (call_indirect (type $0)
;; PRIMARY-NEXT:   (i32.const 1)
;; PRIMARY-NEXT:  )
;; PRIMARY-NEXT: )
