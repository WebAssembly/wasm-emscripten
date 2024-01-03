;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --cfp -all -S -o - | filecheck %s

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

;; Three types (in a chain) with three values.
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

;; Three types with two values.
(module
  ;; CHECK:      (type $struct (sub (struct (field i32))))
  (type $struct (sub (struct i32)))
  ;; CHECK:      (type $substruct (sub $struct (struct (field i32) (field f64))))
  (type $substruct (sub $struct (struct i32 f64)))

  ;; CHECK:      (type $2 (func))

  ;; CHECK:      (type $subsubstruct (sub $substruct (struct (field i32) (field f64) (field anyref))))
  (type $subsubstruct (sub $substruct (struct i32 f64 anyref)))

  ;; CHECK:      (type $4 (func (param (ref null $struct)) (result i32)))

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
    ;; Three types are possible here, but two have the same value, and we can
    ;; differentiate between them with a test.
    (struct.get $struct 0
      (local.get $struct)
    )
  )
)

;; Three types with two values, but non-consecutive (see below).
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

