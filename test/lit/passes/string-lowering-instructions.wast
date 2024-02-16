;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --string-lowering  -all -S -o - | filecheck %s

(module
  (rec
    ;; CHECK:      (type $0 (array (mut i16)))

    ;; CHECK:      (type $1 (func))

    ;; CHECK:      (type $2 (func (param externref externref) (result i32)))

    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $3 (func (param externref i32 externref)))

    ;; CHECK:       (type $4 (func (result externref)))

    ;; CHECK:       (type $struct-of-string (struct (field externref) (field i32) (field anyref)))
    (type $struct-of-string (struct (field stringref) (field i32) (field anyref)))

    ;; CHECK:       (type $struct-of-array (struct (field (ref $0))))
    (type $struct-of-array (struct (field (ref $array16))))

    ;; CHECK:       (type $array16-imm (array i32))
    (type $array16-imm (array i32))

    ;; CHECK:       (type $array32 (array (mut i32)))
    (type $array32 (array (mut i32)))

    ;; CHECK:       (type $array16-open (sub (array (mut i16))))
    (type $array16-open (sub (array (mut i16))))

    ;; CHECK:       (type $array16-child (sub $array16-open (array (mut i16))))
    (type $array16-child (sub $array16-open (array (mut i16))))

    ;; CHECK:       (type $array16 (array (mut i16)))
    (type $array16 (array (mut i16)))
  )

  ;; CHECK:       (type $12 (func (param externref) (result externref)))

  ;; CHECK:       (type $13 (func (param externref) (result externref)))

  ;; CHECK:       (type $14 (func (param externref) (result i32)))

  ;; CHECK:       (type $15 (func (param externref externref) (result i32)))

  ;; CHECK:       (type $16 (func (param externref (ref $0)) (result i32)))

  ;; CHECK:       (type $17 (func (param (ref $0))))

  ;; CHECK:       (type $18 (func (param externref (ref extern) externref externref externref (ref extern))))

  ;; CHECK:      (type $19 (func (param (ref null $0) i32 i32) (result (ref extern))))

  ;; CHECK:      (type $20 (func (param i32) (result (ref extern))))

  ;; CHECK:      (type $21 (func (param externref (ref null $0) i32) (result i32)))

  ;; CHECK:      (type $22 (func (param externref) (result i32)))

  ;; CHECK:      (type $23 (func (param externref i32) (result i32)))

  ;; CHECK:      (type $24 (func (param externref i32 i32) (result (ref extern))))

  ;; CHECK:      (import "string.const" "0" (global $string.const_exported (ref extern)))

  ;; CHECK:      (import "string.const" "1" (global $string.const_value (ref extern)))

  ;; CHECK:      (import "colliding" "name" (func $fromCodePoint (type $1)))
  (import "colliding" "name" (func $fromCodePoint))

  ;; CHECK:      (import "wasm:js-string" "fromCharCodeArray" (func $fromCharCodeArray (type $19) (param (ref null $0) i32 i32) (result (ref extern))))

  ;; CHECK:      (import "wasm:js-string" "fromCodePoint" (func $fromCodePoint_17 (type $20) (param i32) (result (ref extern))))

  ;; CHECK:      (import "wasm:js-string" "intoCharCodeArray" (func $intoCharCodeArray (type $21) (param externref (ref null $0) i32) (result i32)))

  ;; CHECK:      (import "wasm:js-string" "equals" (func $equals (type $2) (param externref externref) (result i32)))

  ;; CHECK:      (import "wasm:js-string" "compare" (func $compare (type $2) (param externref externref) (result i32)))

  ;; CHECK:      (import "wasm:js-string" "length" (func $length (type $22) (param externref) (result i32)))

  ;; CHECK:      (import "wasm:js-string" "codePointAt" (func $codePointAt (type $23) (param externref i32) (result i32)))

  ;; CHECK:      (import "wasm:js-string" "substring" (func $substring (type $24) (param externref i32 i32) (result (ref extern))))

  ;; CHECK:      (func $string.as (type $18) (param $a externref) (param $a_nn (ref extern)) (param $b externref) (param $c externref) (param $d externref) (param $nn_view (ref extern))
  ;; CHECK-NEXT:  (local.set $b
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $c
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $d
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $nn_view
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $nn_view
  ;; CHECK-NEXT:   (local.get $a_nn)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.as
    (param $a stringref)
    (param $a_nn (ref string))
    (param $b stringview_wtf8)
    (param $c stringview_wtf16)
    (param $d stringview_iter)
    (param $nn_view (ref stringview_wtf16))
    ;; These operations all vanish in the lowering, as they all become extref
    ;; (JS strings).
    (local.set $b
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
    ;; The input is nullable, and string.as casts to non-null, so we will need
    ;; to keep a cast here in order to validate. (We also add a cast in all the
    ;; above as the inputs are nullable, but this is the only one that will
    ;; fail to validate. Other opts can remove the above ones.)
    (local.set $nn_view
      (string.as_wtf16
        (local.get $a)
      )
    )
    ;; The input is already non-nullable here, so no cast is needed.
    (local.set $nn_view
      (string.as_wtf16
        (local.get $a_nn)
      )
    )
  )

  ;; CHECK:      (func $string.new.gc (type $17) (param $array16 (ref $0))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $fromCharCodeArray
  ;; CHECK-NEXT:    (local.get $array16)
  ;; CHECK-NEXT:    (i32.const 7)
  ;; CHECK-NEXT:    (i32.const 8)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.new.gc (param $array16 (ref $array16))
    (drop
      (string.new_wtf16_array
        (local.get $array16)
        (i32.const 7)
        (i32.const 8)
      )
    )
  )

  ;; CHECK:      (func $string.from_code_point (type $4) (result externref)
  ;; CHECK-NEXT:  (call $fromCodePoint_17
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.from_code_point (result stringref)
    (string.from_code_point
      (i32.const 1)
    )
  )

  ;; CHECK:      (func $string.encode (type $16) (param $ref externref) (param $array16 (ref $0)) (result i32)
  ;; CHECK-NEXT:  (call $intoCharCodeArray
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:   (local.get $array16)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.encode (param $ref stringref) (param $array16 (ref $array16)) (result i32)
    (string.encode_wtf16_array
      (local.get $ref)
      (local.get $array16)
      (i32.const 10)
    )
  )

  ;; CHECK:      (func $string.eq (type $15) (param $a externref) (param $b externref) (result i32)
  ;; CHECK-NEXT:  (call $equals
  ;; CHECK-NEXT:   (local.get $a)
  ;; CHECK-NEXT:   (local.get $b)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.eq (param $a stringref) (param $b stringref) (result i32)
    (string.eq
      (local.get $a)
      (local.get $b)
    )
  )

  ;; CHECK:      (func $string.compare (type $15) (param $a externref) (param $b externref) (result i32)
  ;; CHECK-NEXT:  (call $compare
  ;; CHECK-NEXT:   (local.get $a)
  ;; CHECK-NEXT:   (local.get $b)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.compare (param $a stringref) (param $b stringref) (result i32)
    (string.compare
      (local.get $a)
      (local.get $b)
    )
  )

  ;; CHECK:      (func $string.length (type $14) (param $ref externref) (result i32)
  ;; CHECK-NEXT:  (call $length
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.length (param $ref stringview_wtf16) (result i32)
    (stringview_wtf16.length
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $string.get_codeunit (type $14) (param $ref externref) (result i32)
  ;; CHECK-NEXT:  (call $codePointAt
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.get_codeunit (param $ref stringview_wtf16) (result i32)
    (stringview_wtf16.get_codeunit
      (local.get $ref)
      (i32.const 2)
    )
  )

  ;; CHECK:      (func $string.slice (type $13) (param $ref externref) (result externref)
  ;; CHECK-NEXT:  (call $substring
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:   (i32.const 3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $string.slice (param $ref stringview_wtf16) (result stringref)
    (stringview_wtf16.slice
      (local.get $ref)
      (i32.const 2)
      (i32.const 3)
    )
  )

  ;; CHECK:      (func $if.string (type $12) (param $ref externref) (result externref)
  ;; CHECK-NEXT:  (if (result externref)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (then
  ;; CHECK-NEXT:    (ref.null noextern)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (else
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $if.string (param $ref stringref) (result stringref)
    (if (result stringref)
      (i32.const 0)
      (then
        (ref.null none) ;; this will turn into noextern
      )
      (else
        (local.get $ref)
      )
    )
  )

  ;; CHECK:      (func $if.string.flip (type $12) (param $ref externref) (result externref)
  ;; CHECK-NEXT:  (if (result externref)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (then
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (else
  ;; CHECK-NEXT:    (ref.null noextern)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $if.string.flip (param $ref stringref) (result stringref)
    ;; As above but with flipped arms.
    (if (result stringref)
      (i32.const 0)
      (then
        (local.get $ref)
      )
      (else
        (ref.null none)
      )
    )
  )

  ;; CHECK:      (func $exported-string-returner (type $4) (result externref)
  ;; CHECK-NEXT:  (global.get $string.const_exported)
  ;; CHECK-NEXT: )
  (func $exported-string-returner (export "export.1") (result stringref)
    ;; We should update the signature of this function even though it is public
    ;; (exported).
    (string.const "exported")
  )

  ;; CHECK:      (func $exported-string-receiver (type $3) (param $x externref) (param $y i32) (param $z externref)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $z)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $exported-string-receiver (export "export.2") (param $x stringref) (param $y i32) (param $z stringref)
    ;; We should update the signature of this function even though it is public
    ;; (exported).
    (drop
      (local.get $x)
    )
    (drop
      (local.get $y)
    )
    (drop
      (local.get $z)
    )
  )

  ;; CHECK:      (func $use-struct-of-array (type $1)
  ;; CHECK-NEXT:  (local $array16 (ref $0))
  ;; CHECK-NEXT:  (local $open (ref $array16-open))
  ;; CHECK-NEXT:  (local $child (ref $array16-child))
  ;; CHECK-NEXT:  (local $32 (ref $array32))
  ;; CHECK-NEXT:  (local $imm (ref $array16-imm))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $fromCharCodeArray
  ;; CHECK-NEXT:    (struct.get $struct-of-array 0
  ;; CHECK-NEXT:     (struct.new $struct-of-array
  ;; CHECK-NEXT:      (array.new_fixed $0 2
  ;; CHECK-NEXT:       (i32.const 10)
  ;; CHECK-NEXT:       (i32.const 20)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $use-struct-of-array
    ;; The array type here should switch to the new 16-bit array type that we
    ;; use for the new imports, so that it is compatible with them. Without
    ;; that, calling the import as we do below will fail.
    (local $array16 (ref $array16))

    ;; In comparison, the array16-open param should remain as it is: it is an
    ;; open type which is different then the one we care about.
    (local $open (ref $array16-open))

    ;; Likewise a child of that open type is also ignored.
    (local $child (ref $array16-child))

    ;; Another array size is also ignored.
    (local $32 (ref $array32))

    ;; An immutable array is also ignored.
    (local $imm (ref $array16-imm))

    (drop
      (string.new_wtf16_array
        (struct.get $struct-of-array 0
          (struct.new $struct-of-array
            (array.new_fixed $array16 2
              (i32.const 10)
              (i32.const 20)
            )
          )
        )
        (i32.const 0)
        (i32.const 1)
      )
    )
  )

  ;; CHECK:      (func $struct-of-string (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct-of-string
  ;; CHECK-NEXT:    (ref.null noextern)
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct-of-string
  ;; CHECK-NEXT:    (global.get $string.const_value)
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new_default $struct-of-string)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $struct-of-string
    ;; Test lowering of struct fields from stringref to externref.
    (drop
      (struct.new $struct-of-string
        (ref.null none) ;; This null must be fixed to be ext.
        (i32.const 10)
        (ref.null none) ;; Nothing to do here (field remains anyref).
      )
    )
    (drop
      (struct.new $struct-of-string
        (string.const "value") ;; Nothing to do besides change to a global.
        (i32.const 10)
        (ref.null none)
      )
    )
    (drop
      (struct.new_default $struct-of-string) ;; Nothing to do here.
    )
  )
)
