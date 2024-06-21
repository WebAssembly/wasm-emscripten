;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --cfp-reftest -all -S -o - | filecheck %s

;; When a struct.get can only read from two types, and those types have a
;; constant field, we can select between those two values using a ref.test.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    ;; Used below.
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $substruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Rather than load from the struct, we can test between the two types
    ;; possible here.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; As above, but now the child is a final type. This does not matter as we
;; optimize either way, if the child has no children (since without children it
;; could be marked final later, which we assume).
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub final $struct (struct (field i32) (field f64))))
  (type $substruct (sub final $struct (struct i32 f64)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $substruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; As above, but now the subtype has subtypes.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64))))
  (type $subsubstruct (sub $substruct (struct i32 f64)))

  ;; CHECK:      (func $create (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    ;; Used below.
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (local $x (ref $subsubstruct))
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Keep this type alive.
    (local $x (ref $subsubstruct))

    ;; We only test on final types for efficiency, so we do not optimize here.
    ;; The type we'd like to test on here has subtypes so it cannot be marked
    ;; final; otherwise if there are no subtypes we assume it will be marked
    ;; final later and optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; As above, but now one value is not constant.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func (param i32)))

  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1) (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create (param $x i32)
    (drop
      (struct.new $struct
        (local.get $x) ;; this changed
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We cannot optimize here.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; As above, but now the other value is not constant.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func (param i32)))

  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1) (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create (param $x i32)
    (drop
      (struct.new $struct
        (i32.const 10) ;; this changed
      )
    )
    (drop
      (struct.new $substruct
        (local.get $x) ;; this changed
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We cannot optimize here.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Almost optimizable, but the field is mutable, so we can't.
(module
  ;; CHECK:      (type $struct (sub (struct (field (mut i32)))))
  (type $struct (sub (struct (mut i32))))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $substruct (sub $struct (struct (field (mut i32)) (field f64))))
  (type $substruct (sub $struct (struct (mut i32) f64)))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We cannot optimize here.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types (in a chain) with three values, 10, 20, 30.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64) (field anyref))))
  (type $subsubstruct (sub $substruct (struct i32 f64 anyref)))

  ;; CHECK:      (type $3 (func))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (type $5 (func (param (ref null $substruct)) (result i32)))

  ;; CHECK:      (func $create (type $3)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $subsubstruct
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $subsubstruct
        (i32.const 30)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Three types are possible here, with three different values, so we do not
    ;; optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )

  ;; CHECK:      (func $get-sub (type $5) (param $substruct (ref null $substruct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (ref.test (ref $subsubstruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $substruct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-sub (param $substruct (ref null $substruct)) (result i32)
    ;; Only two types are relevant here, so we do optimize.
    (struct.get $substruct 0
      (local.get $substruct)
    )
  )
)

;; Three types with two values, 10, 20, 20.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64) (field anyref))))
  (type $subsubstruct (sub $substruct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $subsubstruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $subsubstruct
        (i32.const 20) ;; this changed
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Three types are possible here, but two have the same value, and we can
    ;; differentiate between them with a test. However, the test would be on
    ;; $substruct, which is not a final type, so we do not optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types with two values, but non-consecutive: 20, 10, 20.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64) (field anyref))))
  (type $subsubstruct (sub $substruct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $subsubstruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 20) ;; this changed
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 10) ;; this changed
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $subsubstruct
        (i32.const 20)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Three types are possible here, and two have the same value, but we still
    ;; cannot optimize: the chain of types has values A->B->A so there is no
    ;; ref.test that can differentiate the two sets.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types with two values, 10, 10, 20.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64) (field anyref))))
  (type $subsubstruct (sub $substruct (struct i32 f64 anyref)))

  ;; CHECK:      (type $3 (func))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $3)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $subsubstruct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10) ;; this changed
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 10)
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $subsubstruct
        (i32.const 20)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $subsubstruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can differentiate between the first 2 and the last 1 by testing on the
    ;; last, so we can optimize here.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types with two values and an abstract type.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64) (field anyref))))
  (type $subsubstruct (sub $substruct (struct i32 f64 anyref)))

  ;; CHECK:      (type $3 (func))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $3)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $subsubstruct
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    ;; We never create $substruct, so it doesn't matter.
    (drop
      (struct.new $subsubstruct
        (i32.const 30)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $subsubstruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can optimize since only two types are actually possible.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types with three values, now in a triangle.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $substruct.A (sub $struct (struct (field i32) (field f64))))
  (type $substruct.A (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $substruct.B (sub $struct (struct (field i32) (field f64) (field anyref))))
  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.A
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.B
  ;; CHECK-NEXT:    (i32.const -20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct.A
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $substruct.B
        (i32.const -20)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; Three types are possible here, with three different values, so we do not
    ;; optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types in a triangle, with only two values.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $substruct.A (sub $struct (struct (field i32) (field f64))))
  (type $substruct.A (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $substruct.B (sub $struct (struct (field i32) (field f64) (field anyref))))
  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.A
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.B
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct.A
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $substruct.B
        (i32.const 20) ;; this changed
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; There is no ref.test that can separate the parent from the two children,
    ;; so we cannot optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; As above, but the singular value is moved.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct.A (sub $struct (struct (field i32) (field f64))))
  (type $substruct.A (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $substruct.B (sub $struct (struct (field i32) (field f64) (field anyref))))
  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.A
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.B
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 20) ;; this changed
      )
    )
    (drop
      (struct.new $substruct.A
        (i32.const 10) ;; this changed
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $substruct.B
        (i32.const 20)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (ref.test (ref $substruct.A)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can ref.test on $substruct.A now, and optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; As above, but the singular value is moved again.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct.B (sub $struct (struct (field i32) (field f64) (field anyref))))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $substruct.A (sub $struct (struct (field i32) (field f64))))
  (type $substruct.A (sub $struct (struct i32 f64)))

  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.A
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.B
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 20)
      )
    )
    (drop
      (struct.new $substruct.A
        (i32.const 20) ;; this changed
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $substruct.B
        (i32.const 10) ;; this changed
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (ref.test (ref $substruct.B)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can ref.test on $substruct.B now, and optimize.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; A triangle with an abstract type at the top.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct.A (sub $struct (struct (field i32) (field f64))))
  (type $substruct.A (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $substruct.B (sub $struct (struct (field i32) (field f64) (field anyref))))
  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.A
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.B
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    ;; $struct is never created.
    (drop
      (struct.new $substruct.A
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
    (drop
      (struct.new $substruct.B
        (i32.const 30)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $4) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:   (ref.test (ref $substruct.A)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can optimize here as only two types are non-abstract, and picking
    ;; between the two siblings is easy.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; A triangle with an abstract type in a sibling.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  (type $substruct.A (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $substruct.B (sub $struct (struct (field i32) (field f64) (field anyref))))
  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.B
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    ;; $substruct.A is never created.
    (drop
      (struct.new $substruct.B
        (i32.const 30)
        (f64.const 3.14159)
        (ref.null any)
      )
    )
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 30)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $substruct.B)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can optimize here as only two types are non-abstract, and we can test
    ;; on the non-abstract sibling.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; A triangle with an abstract type in the other sibling.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct.A (sub $struct (struct (field i32) (field f64))))
  (type $substruct.A (sub $struct (struct i32 f64)))

  (type $substruct.B (sub $struct (struct i32 f64 anyref)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct.A
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    (drop
      (struct.new $struct
        (i32.const 10)
      )
    )
    (drop
      (struct.new $substruct.A
        (i32.const 20)
        (f64.const 3.14159)
      )
    )
    ;; $substruct.B is never created.
  )
  ;; CHECK:      (func $get (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (ref.test (ref $substruct.A)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get (param $struct (ref null $struct)) (result i32)
    ;; We can optimize here as only two types are non-abstract, and we can test
    ;; on the non-abstract sibling.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Several fields and several news.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32) (field i64) (field f64) (field f32))))
  (type $struct (sub (struct i32 i64 f64 f32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field i64) (field f64) (field f32))))
  (type $substruct (sub $struct (struct i32 i64 f64 f32)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $3 (func (param (ref null $struct)) (result i32)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i64)))

  ;; CHECK:      (type $5 (func (param (ref null $struct)) (result f64)))

  ;; CHECK:      (type $6 (func (param (ref null $struct)) (result f32)))

  ;; CHECK:      (func $create (type $2)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (i64.const 20)
  ;; CHECK-NEXT:    (f64.const 30.3)
  ;; CHECK-NEXT:    (f32.const 40.400001525878906)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (i64.const 22)
  ;; CHECK-NEXT:    (f64.const 36.36)
  ;; CHECK-NEXT:    (f32.const 40.79999923706055)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $substruct
  ;; CHECK-NEXT:    (i32.const 11)
  ;; CHECK-NEXT:    (i64.const 22)
  ;; CHECK-NEXT:    (f64.const 30.3)
  ;; CHECK-NEXT:    (f32.const 40.79999923706055)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $create
    ;; The first new is for $struct, the last two for $substruct.
    ;; The first two news agree on field 0; the last two on fields 1&3; and the
    ;; first and last on field 2. As a result, we can optimize only fields 1&3.
    ;; field.
    (drop
      (struct.new $struct
        (i32.const 10)
        (i64.const 20)
        (f64.const 30.3)
        (f32.const 40.4)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 10)
        (i64.const 22)
        (f64.const 36.36)
        (f32.const 40.8)
      )
    )
    (drop
      (struct.new $substruct
        (i32.const 11)
        (i64.const 22)
        (f64.const 30.3)
        (f32.const 40.8)
      )
    )
  )
  ;; CHECK:      (func $get-0 (type $3) (param $struct (ref null $struct)) (result i32)
  ;; CHECK-NEXT:  (struct.get $struct 0
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-0 (param $struct (ref null $struct)) (result i32)
    (struct.get $struct 0
      (local.get $struct)
    )
  )

  ;; CHECK:      (func $get-1 (type $4) (param $struct (ref null $struct)) (result i64)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i64.const 22)
  ;; CHECK-NEXT:   (i64.const 20)
  ;; CHECK-NEXT:   (ref.test (ref $substruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-1 (param $struct (ref null $struct)) (result i64)
    (struct.get $struct 1
      (local.get $struct)
    )
  )

  ;; CHECK:      (func $get-2 (type $5) (param $struct (ref null $struct)) (result f64)
  ;; CHECK-NEXT:  (struct.get $struct 2
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-2 (param $struct (ref null $struct)) (result f64)
    (struct.get $struct 2
      (local.get $struct)
    )
  )

  ;; CHECK:      (func $get-3 (type $6) (param $struct (ref null $struct)) (result f32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (f32.const 40.79999923706055)
  ;; CHECK-NEXT:   (f32.const 40.400001525878906)
  ;; CHECK-NEXT:   (ref.test (ref $substruct)
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $struct)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-3 (param $struct (ref null $struct)) (result f32)
    (struct.get $struct 3
      (local.get $struct)
    )
  )
)

