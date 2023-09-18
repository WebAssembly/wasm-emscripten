;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --remove-unused-names --local-subtyping -all -S -o - \
;; RUN:   | filecheck %s

;; --remove-unused-names is run to avoid adding names to blocks. Block names
;; can prevent non-nullable local validation (we emit named blocks in the binary
;; format, if we need them, but never emit unnamed ones), which affects some
;; testcases.

(module
  (type ${} (sub (struct)))

  (type ${i32} (sub (struct (field i32))))

  (type $array (sub (array i8)))

  ;; CHECK:      (type $ret-any (sub (func (result anyref))))
  (type $ret-any (sub (func (result anyref))))
  ;; CHECK:      (type $ret-i31 (sub $ret-any (func (result i31ref))))
  (type $ret-i31 (sub $ret-any (func (result i31ref))))

  ;; CHECK:      (import "out" "i32" (func $i32 (type $1) (result i32)))
  (import "out" "i32" (func $i32 (result i32)))
  ;; CHECK:      (import "out" "i64" (func $i64 (type $6) (result i64)))
  (import "out" "i64" (func $i64 (result i64)))

  ;; Refinalization can find a more specific type, where the declared type was
  ;; not the optimal LUB.
  ;; CHECK:      (func $refinalize (type $2) (param $x i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (if (result (ref i31))
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:    (ref.i31
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (ref.i31
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block $block (result (ref i31))
  ;; CHECK-NEXT:    (br $block
  ;; CHECK-NEXT:     (ref.i31
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (ref.i31
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $refinalize (param $x i32)
    (drop
      (if (result anyref)
        (local.get $x)
        (ref.i31 (i32.const 0))
        (ref.i31 (i32.const 1))
      )
    )
    (drop
      (block $block (result anyref)
        (br $block
          (ref.i31 (i32.const 0))
        )
        (ref.i31 (i32.const 1))
      )
    )
  )

  ;; A simple case where a local has a single assignment that we can use as a
  ;; more specific type. A similar thing with a parameter, however, is not a
  ;; thing we can optimize. Also, ignore a local with zero assignments.
  ;; CHECK:      (func $simple-local-but-not-param (type $7) (param $x funcref)
  ;; CHECK-NEXT:  (local $y (ref $1))
  ;; CHECK-NEXT:  (local $unused funcref)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.func $i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (ref.func $i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-local-but-not-param (param $x funcref)
    (local $y funcref)
    (local $unused funcref)
    (local.set $x
      (ref.func $i32)
    )
    (local.set $y
      (ref.func $i32)
    )
  )

  ;; CHECK:      (func $locals-with-multiple-assignments (type $8) (param $struct structref)
  ;; CHECK-NEXT:  (local $x eqref)
  ;; CHECK-NEXT:  (local $y (ref i31))
  ;; CHECK-NEXT:  (local $z structref)
  ;; CHECK-NEXT:  (local $w (ref func))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.i31
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (ref.i31
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (ref.i31
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $struct)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $w
  ;; CHECK-NEXT:   (ref.func $i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $w
  ;; CHECK-NEXT:   (ref.func $i64)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $locals-with-multiple-assignments (param $struct (ref null struct))
    (local $x anyref)
    (local $y anyref)
    (local $z anyref)
    (local $w funcref)
    ;; x is assigned two different types with a new LUB possible
    (local.set $x
      (ref.i31 (i32.const 0))
    )
    (local.set $x
      (local.get $struct)
    )
    ;; y and z are assigned the same more specific type twice
    (local.set $y
      (ref.i31 (i32.const 0))
    )
    (local.set $y
      (ref.i31 (i32.const 1))
    )
    (local.set $z
      (local.get $struct)
    )
    (local.set $z
      (local.get $struct)
    )
    ;; w is assigned two different types *without* a new LUB heap type possible,
    ;; as it already had the optimal LUB heap type (but it can become non-
    ;; nullable).
    (local.set $w
      (ref.func $i32)
    )
    (local.set $w
      (ref.func $i64)
    )
  )

  ;; In some cases multiple iterations are necessary, as one inferred new type
  ;; applies to a get which then allows another inference.
  ;; CHECK:      (func $multiple-iterations (type $0)
  ;; CHECK-NEXT:  (local $x (ref $1))
  ;; CHECK-NEXT:  (local $y (ref $1))
  ;; CHECK-NEXT:  (local $z (ref $1))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.func $i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $multiple-iterations
    (local $x funcref)
    (local $y funcref)
    (local $z funcref)
    (local.set $x
      (ref.func $i32)
    )
    (local.set $y
      (local.get $x)
    )
    (local.set $z
      (local.get $y)
    )
  )

  ;; Sometimes a refinalize is necessary in between the iterations.
  ;; CHECK:      (func $multiple-iterations-refinalize (type $2) (param $i i32)
  ;; CHECK-NEXT:  (local $x (ref $1))
  ;; CHECK-NEXT:  (local $y (ref $6))
  ;; CHECK-NEXT:  (local $z (ref func))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.func $i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (ref.func $i64)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (select (result (ref func))
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:    (local.get $y)
  ;; CHECK-NEXT:    (local.get $i)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $multiple-iterations-refinalize (param $i i32)
    (local $x funcref)
    (local $y funcref)
    (local $z funcref)
    (local.set $x
      (ref.func $i32)
    )
    (local.set $y
      (ref.func $i64)
    )
    (local.set $z
      (select
        (local.get $x)
        (local.get $y)
        (local.get $i)
      )
    )
  )

  ;; CHECK:      (func $multiple-iterations-refinalize-call-ref (type $0)
  ;; CHECK-NEXT:  (local $f (ref $ret-i31))
  ;; CHECK-NEXT:  (local $x i31ref)
  ;; CHECK-NEXT:  (local.set $f
  ;; CHECK-NEXT:   (ref.func $ret-i31)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call_ref $ret-i31
  ;; CHECK-NEXT:    (local.get $f)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $multiple-iterations-refinalize-call-ref
    (local $f (ref null $ret-any))
    (local $x (anyref))
    (local.set $f
      (ref.func $ret-i31)
    )
    (local.set $x
      ;; After $f is refined to hold $ret-i31 and the call_ref is refinalized,
      ;; we will be able to refine $x to i31.
      (call_ref $ret-any
        (local.get $f)
      )
    )
  )

  ;; CHECK:      (func $multiple-iterations-refinalize-call-ref-bottom (type $0)
  ;; CHECK-NEXT:  (local $f nullfuncref)
  ;; CHECK-NEXT:  (local $x anyref)
  ;; CHECK-NEXT:  (local.set $f
  ;; CHECK-NEXT:   (ref.null nofunc)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (block ;; (replaces something unreachable we can't emit)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $f)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $multiple-iterations-refinalize-call-ref-bottom
    (local $f (ref null $ret-any))
    (local $x (anyref))
    ;; Same as above, but now we refine $f to nullfuncref. Check that we don't crash.
    (local.set $f
      (ref.null nofunc)
    )
    (local.set $x
      ;; We can no longer refine $x because there is no result type we can use
      ;; after refining $f.
      (call_ref $ret-any
        (local.get $f)
      )
    )
  )

  ;; CHECK:      (func $ret-i31 (type $ret-i31) (result i31ref)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $ret-i31 (type $ret-i31) (result i31ref)
    (unreachable)
  )

  ;; CHECK:      (func $nondefaultable (type $0)
  ;; CHECK-NEXT:  (local $x (funcref funcref))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (tuple.make
  ;; CHECK-NEXT:    (ref.func $i32)
  ;; CHECK-NEXT:    (ref.func $i32)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nondefaultable
    (local $x (funcref funcref))
    ;; This tuple is assigned non-nullable values, which means the subtype is
    ;; nondefaultable, and we must not apply it.
    (local.set $x
      (tuple.make
        (ref.func $i32)
        (ref.func $i32)
      )
    )
  )

  ;; CHECK:      (func $uses-default (type $2) (param $i i32)
  ;; CHECK-NEXT:  (local $x (ref null $2))
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (local.get $i)
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (ref.func $uses-default)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $uses-default (param $i i32)
    (local $x funcref)
    (if
      (local.get $i)
      ;; The only set to this local uses a more specific type than funcref.
      (local.set $x (ref.func $uses-default))
    )
    (drop
      ;; This get may use the default value, but it is ok to have a null of a
      ;; more refined type in the local.
      (local.get $x)
    )
  )

  ;; CHECK:      (func $unreachables (type $3) (result funcref)
  ;; CHECK-NEXT:  (local $temp (ref $3))
  ;; CHECK-NEXT:  (local.set $temp
  ;; CHECK-NEXT:   (ref.func $unreachables)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $3))
  ;; CHECK-NEXT:    (local.tee $temp
  ;; CHECK-NEXT:     (ref.func $unreachables)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $temp)
  ;; CHECK-NEXT: )
  (func $unreachables (result funcref)
    (local $temp funcref)
    ;; Set a value that allows us to refine the local's type.
    (local.set $temp
      (ref.func $unreachables)
    )
    (unreachable)
    ;; A tee that is not reachable. We must still update its type, and the
    ;; parents.
    (drop
      (block (result funcref)
        (local.tee $temp
          (ref.func $unreachables)
        )
      )
    )
    ;; A get that is not reachable. We must still update its type.
    (local.get $temp)
  )

  ;; CHECK:      (func $incompatible-sets (type $1) (result i32)
  ;; CHECK-NEXT:  (local $temp (ref $1))
  ;; CHECK-NEXT:  (local.set $temp
  ;; CHECK-NEXT:   (ref.func $incompatible-sets)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $temp
  ;; CHECK-NEXT:    (block
  ;; CHECK-NEXT:     (drop
  ;; CHECK-NEXT:      (ref.null nofunc)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (unreachable)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.tee $temp
  ;; CHECK-NEXT:   (block
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (ref.null nofunc)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $incompatible-sets (result i32)
    (local $temp funcref)
    ;; Set a value that allows us to specialize the local type.
    (local.set $temp
      (ref.func $incompatible-sets)
    )
    ;; Make all code unreachable from here.
    (unreachable)
    ;; In unreachable code, assign values that are not compatible with the more
    ;; specific type we will optimize to. Those cannot be left as they are, and
    ;; will be fixed up so that they validate. (All we need is validation, as
    ;; their contents do not matter, given they are not reached.)
    (drop
      (local.tee $temp
        (ref.null func)
      )
    )
    (local.set $temp
      (ref.null func)
    )
    (unreachable)
  )

  ;; CHECK:      (func $become-non-nullable (type $0)
  ;; CHECK-NEXT:  (local $x (ref $0))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.func $become-non-nullable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $become-non-nullable
    (local $x (ref null func))
    (local.set $x
      (ref.func $become-non-nullable)
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $already-non-nullable (type $0)
  ;; CHECK-NEXT:  (local $x (ref $0))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.func $already-non-nullable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $already-non-nullable
    (local $x (ref func))
    (local.set $x
      (ref.func $already-non-nullable)
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $cannot-become-non-nullable (type $0)
  ;; CHECK-NEXT:  (local $x (ref null $0))
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (ref.func $become-non-nullable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $cannot-become-non-nullable
    (local $x (ref null func))
    ;; The set is in a nested scope, so we should not make the local non-
    ;; nullable, as it would not validate. (We can refine the heap type,
    ;; though.)
    (if
      (i32.const 1)
      (local.set $x
        (ref.func $become-non-nullable)
      )
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $cannot-become-non-nullable-block (type $0)
  ;; CHECK-NEXT:  (local $x (ref null $0))
  ;; CHECK-NEXT:  (block $name
  ;; CHECK-NEXT:   (br_if $name
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (ref.func $become-non-nullable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $cannot-become-non-nullable-block
    (local $x (ref null func))
    ;; A named block prevents us from optimizing here, the same as above.
    (block $name
      ;; Add a br_if to avoid the name being removed.
      (br_if $name
        (i32.const 1)
      )
      (local.set $x
        (ref.func $become-non-nullable)
      )
    )
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $become-non-nullable-block-unnamed (type $0)
  ;; CHECK-NEXT:  (local $x (ref $0))
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (ref.func $become-non-nullable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $become-non-nullable-block-unnamed
    (local $x (ref null func))
    ;; An named block does *not* prevent us from optimizing here. Unlike above,
    ;; an unnamed block is never emitted in the binary format, so it does not
    ;; prevent validation.
    (block
      (local.set $x
        (ref.func $become-non-nullable)
      )
    )
    (drop
      (local.get $x)
    )
  )
)
