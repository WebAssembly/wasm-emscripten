;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --nominal --monomorphize -all -S -o - | filecheck %s

(module
  (type $A (struct_subtype data))
  (type $B (struct_subtype $A))

  (func $foo
    (call $bar
      (struct.new $A)
    )
    (call $bar
      (struct.new $A)
    )
    (call $bar
      (struct.new $B)
    )
    (call $bar
      (struct.new $B)
    )
  )

  (func $bar (param $ref (ref $A))
  )
)
