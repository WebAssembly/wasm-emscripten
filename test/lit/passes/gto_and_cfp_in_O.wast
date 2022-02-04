;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt -O -all --nominal -S -o - | filecheck %s

;; Test that -O, with nominal typing + GC enabled, will run global type
;; optimization in conjunction with constant field propagation etc.

(module
  (type $struct (struct_subtype (field (mut funcref)) (field (mut i32)) data))

  (global $glob (ref $struct) (struct.new $struct
    (ref.func $by-ref)
    (i32.const 100)
  ))

  (func $by-ref
    ;; This function is kept alive by the reference in $glob. After we remove
    ;; the field that the funcref is written to, we remove the funcref, which
    ;; means this function can be removed.
    ;;
    ;; Once it is removed, this write no longer exists, and does not hamper
    ;; constant field propagation from inferring the value of the i32 field.
    (struct.set $struct 1
      (global.get $glob)
      (i32.const 200)
    )
  )

  ;; CHECK:      (type $func.0 (func_subtype (result i32) func))

  ;; CHECK:      (export "main" (func $main))

  ;; CHECK:      (func $main (type $func.0) (; has Stack IR ;) (result i32)
  ;; CHECK-NEXT:  (i32.const 100)
  ;; CHECK-NEXT: )
  (func $main (export "main") (result i32)
    ;; After all the above optimizations, we can infer that $main should simply
    ;; return 100.
    (struct.get $struct 1
      (global.get $glob)
    )
  )
)

