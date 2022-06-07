;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --nominal --gsi -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; A non-reference global does not confuse us.
  ;; CHECK:      (global $global-other i32 (i32.const 123456))
  (global $global-other i32 (i32.const 123456))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    ;; We can infer that this get can reference either $global1 or $global2,
    ;; and nothing else (aside from a null), and can emit a select between
    ;; those values.
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; As above, but now the field is mutable, so we cannot optimize.
(module
  ;; CHECK:      (type $struct (struct_subtype (field (mut i32)) data))
  (type $struct (struct (mut i32)))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; Just one global. We do not optimize here - we let other passes do that.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; Three globals. For now, we do not optimize here.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global3 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 99999)
  ;; CHECK-NEXT: ))
  (global $global3 (ref $struct) (struct.new $struct
    (i32.const 99999)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; Three globals, as above, but now two agree on their values. We can optimize
;; by comparing to the one that has a single value.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global3 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global3 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; As above, but move the different value of the three to the middle.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global3 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global3 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; As above, but move the different value of the three to the end.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global3 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global3 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global2) ;; XXX bad!
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; Four values, two pairs of equal ones. We do not optimize this.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global3 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global3 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global4 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global4 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; Four values, three equal and one unique. We can optimize this with a single
;; comparison on the unique one.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global3 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global3 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global4 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global4 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global2) ;; XXX bad
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; A struct.new inside a function stops us from optimizing.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.new $struct
        (i32.const 1)
      )
    )
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; We ignore imports, as we assume a closed world, but that might change in the
;; future. For now, we will optimize here.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (import "a" "b" (global $global-import (ref $struct)))
  (import "a" "b" (global $global-import (ref $struct)))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; A struct.new in a non-toplevel position in a global stops us from
;; optimizing.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $tuple (struct_subtype (field anyref) (field anyref) data))
  (type $tuple (struct anyref anyref))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global-tuple (ref $tuple) (struct.new $tuple
  ;; CHECK-NEXT:  (struct.new $struct
  ;; CHECK-NEXT:   (i32.const 999999)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (ref.null any)
  ;; CHECK-NEXT: ))
  (global $global-tuple (ref $tuple) (struct.new $tuple
    (struct.new $struct
      (i32.const 999999)
    )
    (ref.null any)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; As above, but remove the struct.new in a nested position, while keeping all
;; the other stuff in the above test. Now we should optimize.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $tuple (struct_subtype (field anyref) (field anyref) data))
  (type $tuple (struct anyref anyref))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (global $global-tuple (ref $tuple) (struct.new $tuple
  ;; CHECK-NEXT:  (ref.null any)
  ;; CHECK-NEXT:  (ref.null any)
  ;; CHECK-NEXT: ))
  (global $global-tuple (ref $tuple) (struct.new $tuple
    (ref.null any)
    (ref.null any)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; When one of the globals is mutable, we cannot optimize.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (mut (ref $struct)) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (mut (ref $struct)) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; A subtype is not optimizable, which prevents $struct from being optimized.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct_subtype i32 data))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (type $sub-struct (struct_subtype (field i32) $struct))
  (type $sub-struct (struct_subtype i32 $struct))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $sub-struct
  ;; CHECK-NEXT:    (i32.const 999999)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.new $sub-struct
        (i32.const 999999)
      )
    )
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; A *super*-type is not optimizable, but that does not block us, and we can
;; optimize.
(module
  ;; CHECK:      (type $super-struct (struct_subtype (field i32) data))
  (type $super-struct (struct_subtype i32 data))

  ;; CHECK:      (type $struct (struct_subtype (field i32) $super-struct))
  (type $struct (struct_subtype i32 $super-struct))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $super-struct
  ;; CHECK-NEXT:    (i32.const 999999)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.new $super-struct
        (i32.const 999999)
      )
    )
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; One global for each of the type and the subtype. The optimization will pick
;; between their 2 values.
(module
  ;; CHECK:      (type $super-struct (struct_subtype (field i32) data))
  (type $super-struct (struct_subtype i32 data))

  ;; CHECK:      (type $struct (struct_subtype (field i32) $super-struct))
  (type $struct (struct_subtype i32 $super-struct))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $super-struct) (struct.new $super-struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $super-struct) (struct.new $super-struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $super-struct)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    ;; We cannot optimize the first - it has just one global - but the second
    ;; will consider the struct and sub-struct, find 2 possible values, and
    ;; optimize.
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
    (drop
      (struct.get $super-struct 0
        (ref.null $super-struct)
      )
    )
  )
)

;; One global has a non-constant field, so we cannot optimize.
(module
  ;; CHECK:      (type $struct (struct_subtype (field i32) data))
  (type $struct (struct i32))

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global1 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.add
  ;; CHECK-NEXT:   (i32.const 41)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct) (struct.new $struct
    (i32.add
      (i32.const 41)
      (i32.const 1)
    )
  ))

  ;; CHECK:      (global $global2 (ref $struct) (struct.new $struct
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct) (struct.new $struct
    (i32.const 1337)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (ref.null $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (struct.get $struct 0
        (ref.null $struct)
      )
    )
  )
)

;; One global each for two subtypes of a common supertype, and one for the
;; supertype.
(module
  ;; CHECK:      (type $super-struct (struct_subtype (field i32) data))
  (type $super-struct (struct_subtype i32 data))

  ;; CHECK:      (type $struct1 (struct_subtype (field i32) (field f32) $super-struct))
  (type $struct1 (struct_subtype i32 f32 $super-struct))

  ;; CHECK:      (type $struct2 (struct_subtype (field i32) (field f64) $super-struct))
  (type $struct2 (struct_subtype i32 f64 $super-struct))


  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global0 (ref $super-struct) (struct.new $super-struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global0 (ref $super-struct) (struct.new $super-struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global1 (ref $struct1) (struct.new $struct1
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT:  (f32.const 3.141590118408203)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct1) (struct.new $struct1
    (i32.const 1337)
    (f32.const 3.14159)
  ))

  ;; CHECK:      (global $global2 (ref $struct2) (struct.new $struct2
  ;; CHECK-NEXT:  (i32.const 99999)
  ;; CHECK-NEXT:  (f64.const 2.71828)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct2) (struct.new $struct2
    (i32.const 99999)
    (f64.const 2.71828)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $super-struct 0
  ;; CHECK-NEXT:    (ref.null $super-struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct1 0
  ;; CHECK-NEXT:    (ref.null $struct1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct2 0
  ;; CHECK-NEXT:    (ref.null $struct2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    ;; This has three possible values due to the two children, so we do not
    ;; optimize.
    (drop
      (struct.get $super-struct 0
        (ref.null $super-struct)
      )
    )
    ;; These each have one possible value, so we also do not optimize.
    (drop
      (struct.get $struct1 0
        (ref.null $struct1)
      )
    )
    (drop
      (struct.get $struct2 0
        (ref.null $struct2)
      )
    )
  )
)

;; As above, but now the subtypes each have 2 values, and we can optimize.
(module
  ;; CHECK:      (type $super-struct (struct_subtype (field i32) data))
  (type $super-struct (struct_subtype i32 data))

  ;; CHECK:      (type $struct1 (struct_subtype (field i32) (field f32) $super-struct))
  (type $struct1 (struct_subtype i32 f32 $super-struct))

  ;; CHECK:      (type $struct2 (struct_subtype (field i32) (field f64) $super-struct))
  (type $struct2 (struct_subtype i32 f64 $super-struct))


  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (global $global0 (ref $super-struct) (struct.new $super-struct
  ;; CHECK-NEXT:  (i32.const 42)
  ;; CHECK-NEXT: ))
  (global $global0 (ref $super-struct) (struct.new $super-struct
    (i32.const 42)
  ))

  ;; CHECK:      (global $global1 (ref $struct1) (struct.new $struct1
  ;; CHECK-NEXT:  (i32.const 1337)
  ;; CHECK-NEXT:  (f32.const 3.141590118408203)
  ;; CHECK-NEXT: ))
  (global $global1 (ref $struct1) (struct.new $struct1
    (i32.const 1337)
    (f32.const 3.14159)
  ))

  ;; CHECK:      (global $global1b (ref $struct1) (struct.new $struct1
  ;; CHECK-NEXT:  (i32.const 1338)
  ;; CHECK-NEXT:  (f32.const 3.141590118408203)
  ;; CHECK-NEXT: ))
  (global $global1b (ref $struct1) (struct.new $struct1
    (i32.const 1338)
    (f32.const 3.14159)
  ))

  ;; CHECK:      (global $global2 (ref $struct2) (struct.new $struct2
  ;; CHECK-NEXT:  (i32.const 99999)
  ;; CHECK-NEXT:  (f64.const 2.71828)
  ;; CHECK-NEXT: ))
  (global $global2 (ref $struct2) (struct.new $struct2
    (i32.const 99999)
    (f64.const 2.71828)
  ))

  ;; CHECK:      (global $global2b (ref $struct2) (struct.new $struct2
  ;; CHECK-NEXT:  (i32.const 99998)
  ;; CHECK-NEXT:  (f64.const 2.71828)
  ;; CHECK-NEXT: ))
  (global $global2b (ref $struct2) (struct.new $struct2
    (i32.const 99998)
    (f64.const 2.71828)
  ))

  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $super-struct 0
  ;; CHECK-NEXT:    (ref.null $super-struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 1337)
  ;; CHECK-NEXT:    (i32.const 1338)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 99999)
  ;; CHECK-NEXT:    (i32.const 99998)
  ;; CHECK-NEXT:    (ref.eq
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null $struct2)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (global.get $global2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    ;; This still cannot be optimized.
    (drop
      (struct.get $super-struct 0
        (ref.null $super-struct)
      )
    )
    ;; These can be optimized, and will be different from one another.
    (drop
      (struct.get $struct1 0
        (ref.null $struct1)
      )
    )
    (drop
      (struct.get $struct2 0
        (ref.null $struct2)
      )
    )
  )
)
