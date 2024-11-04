;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --gto --closed-world -all -S -o - | filecheck %s
;; (remove-unused-names is added to test fallthrough values without a block
;; name getting in the way)

(module
  ;; The struct here has three fields, and the second of them has no struct.set
  ;; which means we can make it immutable.

  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $0 (func (param (ref null $struct))))

  ;; CHECK:       (type $two-params (func (param (ref $struct) (ref $struct))))

  ;; CHECK:       (type $2 (func (result (ref null $struct))))

  ;; CHECK:       (type $struct (struct (field (mut funcref)) (field funcref) (field (mut funcref))))
  (type $struct (struct (field (mut funcref)) (field (mut funcref)) (field (mut funcref))))

  (type $two-params (func (param (ref $struct)) (param (ref $struct))))

  ;; Test that we update tag types properly.
  (table 0 funcref)

  ;; CHECK:       (type $4 (func (param (ref $struct))))

  ;; CHECK:      (type $5 (func (param (ref $struct))))

  ;; CHECK:      (table $0 0 funcref)

  ;; CHECK:      (elem declare func $func-two-params)

  ;; CHECK:      (tag $tag (param (ref $struct)))
  (tag $tag (param (ref $struct)))

  ;; CHECK:      (func $func (type $4) (param $x (ref $struct))
  ;; CHECK-NEXT:  (local $temp (ref null $struct))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $struct
  ;; CHECK-NEXT:    (ref.null nofunc)
  ;; CHECK-NEXT:    (ref.null nofunc)
  ;; CHECK-NEXT:    (ref.null nofunc)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $struct 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (ref.null nofunc)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $struct 2
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (ref.null nofunc)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $temp
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 1
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $struct))
    (local $temp (ref null $struct))
    ;; The presence of a struct.new does not prevent this optimization: we just
    ;; care about writes using struct.set.
    (drop
      (struct.new $struct
        (ref.null func)
        (ref.null func)
        (ref.null func)
      )
    )
    (struct.set $struct 0
      (local.get $x)
      (ref.null func)
    )
    (struct.set $struct 2
      (local.get $x)
      (ref.null func)
    )
    ;; Test that local types remain valid after our work (otherwise, we'd get a
    ;; validation error).
    (local.set $temp
      (local.get $x)
    )
    ;; Test that struct.get types remain valid after our work.
    (drop
      (struct.get $struct 0
        (local.get $x)
      )
    )
    (drop
      (struct.get $struct 1
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $foo (type $2) (result (ref null $struct))
  ;; CHECK-NEXT:  (try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch $tag
  ;; CHECK-NEXT:    (return
  ;; CHECK-NEXT:     (pop (ref $struct))
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (ref.null none)
  ;; CHECK-NEXT: )
  (func $foo (result (ref null $struct))
    ;; Use a tag so that we test proper updating of its type after making
    ;; changes.
    (try
      (do
        (nop)
      )
      (catch $tag
        (return
          (pop (ref $struct))
        )
      )
    )
    (ref.null $struct)
  )

  ;; CHECK:      (func $func-two-params (type $two-params) (param $x (ref $struct)) (param $y (ref $struct))
  ;; CHECK-NEXT:  (local $z (ref null $two-params))
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (ref.func $func-two-params)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call_indirect $0 (type $two-params)
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func-two-params (param $x (ref $struct)) (param $y (ref $struct))
    ;; This function has two params, which means a tuple type is used for its
    ;; signature, which we must also update. To verify the update is correct,
    ;; assign it to a local.
    (local $z (ref null $two-params))
    (local.set $z
      (ref.func $func-two-params)
    )
    ;; Also check that a call_indirect still validates after the rewriting.
    (call_indirect (type $two-params)
     (local.get $x)
     (local.get $y)
     (i32.const 0)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $0) (param $struct (ref null $struct))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 2
  ;; CHECK-NEXT:    (local.get $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $struct (ref null $struct))
    ;; --gto will remove fields that are not read from, so add reads to any
    ;; that don't already have them.
    (drop (struct.get $struct 2 (local.get $struct)))
  )
)

(module
  ;; Test recursion between structs where we only modify one. Specifically $B
  ;; has no writes to either of its fields.

  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $B (struct (field (ref null $A)) (field f64)))

    ;; CHECK:       (type $1 (func (param (ref null $A) (ref null $B))))

    ;; CHECK:       (type $A (struct (field (mut (ref null $B))) (field (mut i32))))
    (type $A (struct (field (mut (ref null $B))) (field (mut i32)) ))
    (type $B (struct (field (mut (ref null $A))) (field (mut f64)) ))
  )

  ;; CHECK:       (type $3 (func (param (ref $A))))

  ;; CHECK:      (func $func (type $3) (param $x (ref $A))
  ;; CHECK-NEXT:  (struct.set $A 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (ref.null none)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $A 1
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $A))
    (struct.set $A 0
      (local.get $x)
      (ref.null $B)
    )
    (struct.set $A 1
      (local.get $x)
      (i32.const 20)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $1) (param $A (ref null $A)) (param $B (ref null $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 0
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 1
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 0
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 1
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $A (ref null $A)) (param $B (ref null $B))
    (drop (struct.get $A 0 (local.get $A)))
    (drop (struct.get $A 1 (local.get $A)))
    (drop (struct.get $B 0 (local.get $B)))
    (drop (struct.get $B 1 (local.get $B)))
  )
)

(module
  ;; As before, but flipped so that $A's fields can become immutable.

  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (struct (field (ref null $B)) (field i32)))

    ;; CHECK:       (type $1 (func (param (ref null $A) (ref null $B))))

    ;; CHECK:       (type $B (struct (field (mut (ref null $A))) (field (mut f64))))
    (type $B (struct (field (mut (ref null $A))) (field (mut f64)) ))

    (type $A (struct (field (mut (ref null $B))) (field (mut i32)) ))
  )

  ;; CHECK:       (type $3 (func (param (ref $B))))

  ;; CHECK:      (func $func (type $3) (param $x (ref $B))
  ;; CHECK-NEXT:  (struct.set $B 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (ref.null none)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $B 1
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (f64.const 3.14159)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $B))
    (struct.set $B 0
      (local.get $x)
      (ref.null $A)
    )
    (struct.set $B 1
      (local.get $x)
      (f64.const 3.14159)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $1) (param $A (ref null $A)) (param $B (ref null $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 0
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 1
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 0
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 1
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $A (ref null $A)) (param $B (ref null $B))
    (drop (struct.get $A 0 (local.get $A)))
    (drop (struct.get $A 1 (local.get $A)))
    (drop (struct.get $B 0 (local.get $B)))
    (drop (struct.get $B 1 (local.get $B)))
  )
)

(module
  ;; As before, but now one field in each can become immutable.

  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $0 (func (param (ref null $A) (ref null $B))))

    ;; CHECK:       (type $B (struct (field (ref null $A)) (field (mut f64))))
    (type $B (struct (field (mut (ref null $A))) (field (mut f64)) ))

    ;; CHECK:       (type $A (struct (field (mut (ref null $B))) (field i32)))
    (type $A (struct (field (mut (ref null $B))) (field (mut i32)) ))
  )

  ;; CHECK:       (type $3 (func (param (ref $A) (ref $B))))

  ;; CHECK:      (func $func (type $3) (param $x (ref $A)) (param $y (ref $B))
  ;; CHECK-NEXT:  (struct.set $A 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (ref.null none)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $B 1
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:   (f64.const 3.14159)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $A)) (param $y (ref $B))
    (struct.set $A 0
      (local.get $x)
      (ref.null $B)
    )
    (struct.set $B 1
      (local.get $y)
      (f64.const 3.14159)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $0) (param $A (ref null $A)) (param $B (ref null $B))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 0
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 1
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 0
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 1
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $A (ref null $A)) (param $B (ref null $B))
    (drop (struct.get $A 0 (local.get $A)))
    (drop (struct.get $A 1 (local.get $A)))
    (drop (struct.get $B 0 (local.get $B)))
    (drop (struct.get $B 1 (local.get $B)))
  )
)

(module
  ;; Field #0 is already immutable.
  ;; Field #1 is mutable and can become so.
  ;; Field #2 is mutable and must remain so.

  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $0 (func (param (ref null $struct))))

  ;; CHECK:       (type $struct (struct (field i32) (field i32) (field (mut i32))))
  (type $struct (struct (field i32) (field (mut i32)) (field (mut i32))))

  ;; CHECK:       (type $2 (func (param (ref $struct))))

  ;; CHECK:      (func $func (type $2) (param $x (ref $struct))
  ;; CHECK-NEXT:  (struct.set $struct 2
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $struct))
    (struct.set $struct 2
      (local.get $x)
      (i32.const 1)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $0) (param $struct (ref null $struct))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (local.get $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 1
  ;; CHECK-NEXT:    (local.get $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 2
  ;; CHECK-NEXT:    (local.get $struct)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $struct (ref null $struct))
    (drop (struct.get $struct 0 (local.get $struct)))
    (drop (struct.get $struct 1 (local.get $struct)))
    (drop (struct.get $struct 2 (local.get $struct)))
  )
)

(module
  ;; Subtyping. Without a write in either supertype or subtype, we can
  ;; optimize the field to be immutable.

  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $super (sub (struct (field i32))))
  (type $super (sub (struct (field (mut i32)))))
  ;; CHECK:       (type $1 (func (param (ref null $super) (ref null $sub))))

  ;; CHECK:       (type $sub (sub $super (struct (field i32))))
  (type $sub (sub $super (struct (field (mut i32)))))

  ;; CHECK:       (type $3 (func))

  ;; CHECK:      (func $func (type $3)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $super
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $sub
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func
    ;; The presence of struct.new do not prevent us optimizing
    (drop
      (struct.new $super
        (i32.const 1)
      )
    )
    (drop
      (struct.new $sub
        (i32.const 1)
      )
    )
  )

  ;; CHECK:      (func $field-keepalive (type $1) (param $super (ref null $super)) (param $sub (ref null $sub))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $super 0
  ;; CHECK-NEXT:    (local.get $super)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (local.get $sub)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $super (ref null $super)) (param $sub (ref null $sub))
    (drop (struct.get $super 0 (local.get $super)))
    (drop (struct.get $sub 0 (local.get $sub)))
  )
)

(module
  ;; As above, but add a write in the super, which prevents optimization.

  ;; CHECK:      (type $super (sub (struct (field (mut i32)))))
  (type $super (sub (struct (field (mut i32)))))
  ;; CHECK:      (type $sub (sub $super (struct (field (mut i32)))))
  (type $sub (sub $super (struct (field (mut i32)))))

  ;; CHECK:      (type $2 (func (param (ref $super))))

  ;; CHECK:      (type $3 (func (param (ref null $super) (ref null $sub))))

  ;; CHECK:      (func $func (type $2) (param $x (ref $super))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $super
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new $sub
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $super 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $super))
    ;; The presence of struct.new do not prevent us optimizing
    (drop
      (struct.new $super
        (i32.const 1)
      )
    )
    (drop
      (struct.new $sub
        (i32.const 1)
      )
    )
    (struct.set $super 0
      (local.get $x)
      (i32.const 2)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $3) (param $super (ref null $super)) (param $sub (ref null $sub))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $super 0
  ;; CHECK-NEXT:    (local.get $super)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (local.get $sub)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $super (ref null $super)) (param $sub (ref null $sub))
    (drop (struct.get $super 0 (local.get $super)))
    (drop (struct.get $sub 0 (local.get $sub)))
  )
)

(module
  ;; As above, but add a write in the sub, which prevents optimization.


  ;; CHECK:      (type $super (sub (struct (field (mut i32)))))
  (type $super (sub (struct (field (mut i32)))))
  ;; CHECK:      (type $sub (sub $super (struct (field (mut i32)))))
  (type $sub (sub $super (struct (field (mut i32)))))

  ;; CHECK:      (type $2 (func (param (ref $sub))))

  ;; CHECK:      (type $3 (func (param (ref null $super) (ref null $sub))))

  ;; CHECK:      (func $func (type $2) (param $x (ref $sub))
  ;; CHECK-NEXT:  (struct.set $sub 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (param $x (ref $sub))
    (struct.set $sub 0
      (local.get $x)
      (i32.const 2)
    )
  )

  ;; CHECK:      (func $field-keepalive (type $3) (param $super (ref null $super)) (param $sub (ref null $sub))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $super 0
  ;; CHECK-NEXT:    (local.get $super)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (local.get $sub)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $field-keepalive (param $super (ref null $super)) (param $sub (ref null $sub))
    (drop (struct.get $super 0 (local.get $super)))
    (drop (struct.get $sub 0 (local.get $sub)))
  )
)

;; This pass does not refine array types yet. Verify that we do not. This is
;; particularly important for array of i8 and i6, which we special-case as they
;; are used for string interop even in closed world. For now, however, we do not
;; optimize even arrays of i32.
;;
;; Nothing should change in these array types. They have no writes, but they'll
;; stay mutable. Also, this test verifies that we can even validate this module
;; in closed world, even though it contains imports of i8 and i16 arrays.
;;
;; The test also verifies that while we refine the mutability of a struct type,
;; which causes us to rewrite types, that we keep the i8 and i16 arrays in
;; their own size-1 rec groups by themselves, unmodified. The i32 array will be
;; moved into a new big rec group, together with the struct type (that is also
;; refined to be immutable).
(module
  ;; CHECK:      (type $array8 (array (mut i8)))
  (type $array8 (array (mut i8)))
  ;; CHECK:      (type $array16 (array (mut i16)))
  (type $array16 (array (mut i16)))
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $struct (struct (field funcref)))

  ;; CHECK:       (type $array32 (array (mut i32)))
  (type $array32 (array (mut i32)))

  (type $struct (struct (field (mut funcref))))

  ;; CHECK:       (type $4 (func (param funcref)))

  ;; CHECK:      (import "a" "b" (global $i8 (ref $array8)))
  (import "a" "b" (global $i8 (ref $array8)))

  ;; CHECK:      (import "a" "c" (global $i16 (ref $array16)))
  (import "a" "c" (global $i16 (ref $array16)))

  ;; CHECK:      (func $use (type $4) (param $funcref funcref)
  ;; CHECK-NEXT:  (local $array8 (ref $array8))
  ;; CHECK-NEXT:  (local $array16 (ref $array16))
  ;; CHECK-NEXT:  (local $array32 (ref $array32))
  ;; CHECK-NEXT:  (local $struct (ref $struct))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (struct.new $struct
  ;; CHECK-NEXT:     (local.get $funcref)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $use (param $funcref funcref)
    (local $array8 (ref $array8))
    (local $array16 (ref $array16))
    (local $array32 (ref $array32))
    (local $struct (ref $struct))
    (drop
      (struct.get $struct 0
        (struct.new $struct
          (local.get $funcref)
        )
      )
    )
  )
)

;; The parent is public, which prevents us from making any field immutable in
;; the child.
(module
  ;; CHECK:      (type $parent (sub (struct (field (mut i32)))))
  (type $parent (sub (struct (field (mut i32)))))
  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (type $child (sub $parent (struct (field (mut i32)))))
  (type $child (sub $parent (struct (field (mut i32)))))

  ;; CHECK:      (global $global (ref $parent) (struct.new $parent
  ;; CHECK-NEXT:  (i32.const 0)
  ;; CHECK-NEXT: ))
  (global $global (ref $parent) (struct.new $parent
    (i32.const 0)
  ))

  ;; Make the parent public by exporting the global.
  ;; CHECK:      (export "global" (global $global))
  (export "global" (global $global))

  ;; CHECK:      (func $func (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new_default $child)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func
    ;; Create the child so the type is used. No sets to the fields exist, so
    ;; in theory all fields could be immutable.
    (drop
      (struct.new_default $child)
    )
  )
)

;; $sub has a field we can make immutable. That it does not exist in the super
;; should not confuse us.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $super (sub (struct)))
    (type $super (sub (struct)))
    ;; CHECK:       (type $sub (sub $super (struct (field (ref string)))))
    (type $sub (sub $super (struct (field (mut (ref string))))))
  )

  ;; CHECK:       (type $2 (func (param stringref)))

  ;; CHECK:      (func $test (type $2) (param $string stringref)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (struct.new $sub
  ;; CHECK-NEXT:     (string.const "foo")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $string stringref)
    ;; Write and read the field.
    (drop
      (struct.get $sub 0
        (struct.new $sub
          (string.const "foo")
        )
      )
    )
  )
)

;; As above, but with another type in the middle, $mid, which also contains the
;; field. We can optimize both $mid and $sub.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $super (sub (struct)))
    (type $super (sub (struct)))
    ;; CHECK:       (type $mid (sub $super (struct (field (ref string)))))
    (type $mid (sub $super (struct (field (mut (ref string))))))
    ;; CHECK:       (type $sub (sub $mid (struct (field (ref string)))))
    (type $sub (sub $mid (struct (field (mut (ref string))))))
  )

  ;; CHECK:       (type $3 (func (param stringref)))

  ;; CHECK:      (func $test (type $3) (param $string stringref)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (struct.new $sub
  ;; CHECK-NEXT:     (string.const "foo")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $mid 0
  ;; CHECK-NEXT:    (struct.new $mid
  ;; CHECK-NEXT:     (string.const "bar")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $string stringref)
    (drop
      (struct.get $sub 0
        (struct.new $sub
          (string.const "foo")
        )
      )
    )
    (drop
      (struct.get $mid 0
        (struct.new $mid
          (string.const "bar")
        )
      )
    )
  )
)

;; As above, but add another irrelevant field first. We can still optimize the
;; string, but the new mutable i32 must remain mutable, as it has a set.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $super (sub (struct)))
    (type $super (sub (struct (field (mut i32)))))
    ;; CHECK:       (type $mid (sub $super (struct (field (ref string)))))
    (type $mid (sub $super (struct (field (mut i32)) (field (mut (ref string))))))
    ;; CHECK:       (type $sub (sub $mid (struct (field (ref string)))))
    (type $sub (sub $mid (struct (field (mut i32)) (field (mut (ref string))))))
  )

  ;; CHECK:       (type $3 (func (param stringref)))

  ;; CHECK:      (func $test (type $3) (param $string stringref)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (struct.new $sub
  ;; CHECK-NEXT:     (string.const "foo")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $mid 0
  ;; CHECK-NEXT:    (struct.new $mid
  ;; CHECK-NEXT:     (string.const "bar")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (block (result (ref $mid))
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (i32.const 42)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (struct.new $mid
  ;; CHECK-NEXT:      (string.const "baz")
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $string stringref)
    (drop
      (struct.get $sub 1
        (struct.new $sub
          (i32.const 42)
          (string.const "foo")
        )
      )
    )
    (drop
      (struct.get $mid 1
        (struct.new $mid
          (i32.const 1337)
          (string.const "bar")
        )
      )
    )
    ;; A set of the first field.
    (struct.set $mid 0
      (struct.new $mid
        (i32.const 99999)
        (string.const "baz")
      )
      (i32.const 42)
    )
  )
)

;; As above, but without a set of the first field. Now we can optimize both
;; fields.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $super (sub (struct)))
    (type $super (sub (struct (field (mut i32)))))
    ;; CHECK:       (type $mid (sub $super (struct (field (ref string)))))
    (type $mid (sub $super (struct (field (mut i32)) (field (mut (ref string))))))
    ;; CHECK:       (type $sub (sub $mid (struct (field (ref string)))))
    (type $sub (sub $mid (struct (field (mut i32)) (field (mut (ref string))))))
  )

  ;; CHECK:       (type $3 (func (param stringref)))

  ;; CHECK:      (func $test (type $3) (param $string stringref)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $sub 0
  ;; CHECK-NEXT:    (struct.new $sub
  ;; CHECK-NEXT:     (string.const "foo")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $mid 0
  ;; CHECK-NEXT:    (struct.new $mid
  ;; CHECK-NEXT:     (string.const "bar")
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $string stringref)
    (drop
      (struct.get $sub 1
        (struct.new $sub
          (i32.const 42)
          (string.const "foo")
        )
      )
    )
    (drop
      (struct.get $mid 1
        (struct.new $mid
          (i32.const 1337)
          (string.const "bar")
        )
      )
    )
  )
)

