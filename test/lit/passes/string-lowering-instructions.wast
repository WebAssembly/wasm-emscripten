;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --string-lowering  -all -S -o - | filecheck %s

(module
  (func $string.as
    (param $a stringref)
    (param $b stringview_wtf8)
    (param $c stringview_wtf16)
    (param $d stringview_iter)
    (local.set $b ;; validate the output type
      (string.as_wtf8
        (local.get $a)
      )
    )
    (local.set $c
      (string.as_wtf16
        (local.get $a)
      )
    )
    (local.set $d
      (string.as_iter
        (local.get $a)
      )
    )
  )
)
