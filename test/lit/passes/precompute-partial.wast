;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: foreach %s %t wasm-opt --precompute-propagate -all -S -o - | filecheck %s

(module
  ;; CHECK:      (func $simple-1 (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-1 (param $param i32) (result i32)
    ;; Test simple i32 operations.
    (i32.eqz
      (select
        (i32.const 42)
        (i32.const 1337)
        (local.get $param)
      )
    )
  )

  ;; CHECK:      (func $simple-2 (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-2 (param $param i32) (result i32)
    (i32.eqz
      (select
        (i32.const 0)
        (i32.const 10)
        (local.get $param)
      )
    )
  )

  ;; CHECK:      (func $simple-3 (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-3 (param $param i32) (result i32)
    (i32.eqz
      (select
        (i32.const 20)
        (i32.const 0)
        (local.get $param)
      )
    )
  )

  ;; CHECK:      (func $simple-4 (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-4 (param $param i32) (result i32)
    (i32.eqz
      (select
        (i32.const 0)
        (i32.const 0)
        (local.get $param)
      )
    )
  )

  ;; CHECK:      (func $binary (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 11)
  ;; CHECK-NEXT:   (i32.const 21)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $binary (param $param i32) (result i32)
    (i32.add
      (select
        (i32.const 10)
        (i32.const 20)
        (local.get $param)
      )
      (i32.const 1)
    )
  )

  ;; CHECK:      (func $binary-2 (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 11)
  ;; CHECK-NEXT:   (i32.const 21)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $binary-2 (param $param i32) (result i32)
    ;; As above but with the select in the other arm.
    (i32.add
      (i32.const 1)
      (select
        (i32.const 10)
        (i32.const 20)
        (local.get $param)
      )
    )
  )

  ;; CHECK:      (func $binary-3 (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (i32.add
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:    (i32.const 20)
  ;; CHECK-NEXT:    (local.get $param)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 30)
  ;; CHECK-NEXT:    (i32.const 40)
  ;; CHECK-NEXT:    (local.get $param)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $binary-3 (param $param i32) (result i32)
    ;; Two selects. We do not optimize here (but in theory could, as the
    ;; condition is identical - other passes should merge that).
    (i32.add
      (select
        (i32.const 10)
        (i32.const 20)
        (local.get $param)
      )
      (select
        (i32.const 30)
        (i32.const 40)
        (local.get $param)
      )
    )
  )

  ;; CHECK:      (func $unreachable (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (i32.eqz
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $unreachable (param $param i32) (result i32)
    ;; We should ignore unreachable code like this. We would need to make sure
    ;; to emit the proper type on the outside, and it's simpler to just defer
    ;; this to DCE.
    (i32.eqz
      (select
        (i32.const 0)
        (i32.const 0)
        (unreachable)
      )
    )
  )

  ;; CHECK:      (func $tuple (type $1) (param $param i32) (result i32 i32)
  ;; CHECK-NEXT:  (tuple.make 2
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (local.get $param)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $tuple (param $param i32) (result i32 i32)
    ;; We should ignore tuples, as select outputs cannot be tuples.
    (tuple.make 2
      (select
        (i32.const 0)
        (i32.const 1)
        (local.get $param)
      )
      (i32.const 2)
    )
  )

  ;; CHECK:      (func $control-flow (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (block $target (result i32)
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (br_if $target
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $control-flow (param $param i32) (result i32)
    ;; We ignore control flow structures to avoid issues with removing them (as
    ;; the condition might refer to them, as in this testcase).
    (block $target (result i32)
      (select
        (i32.const 0)
        (i32.const 1)
        ;; If we precomputed the block into the select arms, this br_if
        ;; would have nowhere to go.
        (br_if $target
          (i32.const 1)
          (local.get $param)
        )
      )
    )
  )

  ;; CHECK:      (func $break (type $0) (param $x i32) (result i32)
  ;; CHECK-NEXT:  (block $label (result i32)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (br_if $label
  ;; CHECK-NEXT:     (select
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.const 3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $break (param $x i32) (result i32)
    ;; We should change nothing here: we cannot precompute breaks yet TODO
    (block $label (result i32)
      (drop
        (br_if $label
          (select
            (i32.const 0)
            (i32.const 1)
            (local.get $x)
          )
          (i32.const 2)
        )
      )
      (i32.const 3)
    )
  )

  ;; CHECK:      (func $toplevel (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (i32.const 20)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $toplevel (param $param i32) (result i32)
    ;; There is nothing to do for a select with no parent, but do not error at
    ;; least.
    (select
      (i32.const 10)
      (i32.const 20)
      (local.get $param)
    )
  )

  ;; CHECK:      (func $toplevel-nested (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $toplevel-nested (param $param i32) (result i32)
    ;; The inner select can be optimized. XXX why
    (select
      (i32.const 0)
      (i32.const 10)
      (i32.eqz
        (select
          (i32.const 0)
          (i32.const 20)
          (local.get $param)
        )
      )
    )
  )

  ;; CHECK:      (func $nested (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested (param $param i32) (result i32)
    ;; Both the outer and inner selects can be optimized.
    (i32.eqz
      (select
        (i32.const 0)
        (i32.const 10)
        (i32.eqz
          (select
            (i32.const 0)
            (i32.const 20)
            (local.get $param)
          )
        )
      )
    )
  )

  ;; CHECK:      (func $nested-arms (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (i32.eqz
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (i32.const 10)
  ;; CHECK-NEXT:     (i32.const 20)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (i32.const 30)
  ;; CHECK-NEXT:     (i32.const 40)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (i32.const 50)
  ;; CHECK-NEXT:     (i32.const 60)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-arms (param $param i32) (result i32)
    ;; We can do nothing for selects nested directly in other selects, but do
    ;; not error at least.
    (i32.eqz
      (select
        (select
          (i32.const 10)
          (i32.const 20)
          (local.get $param)
        )
        (select
          (i32.const 30)
          (i32.const 40)
          (local.get $param)
        )
        (select
          (i32.const 50)
          (i32.const 60)
          (local.get $param)
        )
      )
    )
  )

  ;; CHECK:      (func $nested-indirect-arms (type $0) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (i32.eqz
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (select
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:     (local.get $param)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nested-indirect-arms (param $param i32) (result i32)
    ;; As above, but now there is something in the middle, that can be optimized
    ;; into those selects.
    (i32.eqz
      (select
        (i32.eqz
          (select
            (i32.const 0)
            (i32.const 10)
            (local.get $param)
          )
        )
        (i32.eqz
          (select
            (i32.const 20)
            (i32.const 0)
            (local.get $param)
          )
        )
        (i32.eqz
          (select
            (i32.const 0)
            (i32.const 30)
            (local.get $param)
          )
        )
      )
    )
  )
)

;; References.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $vtable (sub (struct (field funcref))))
    (type $vtable (sub (struct funcref)))

    ;; CHECK:       (type $specific-func (sub (func (result i32))))
    (type $specific-func (sub (func (result i32))))

    ;; CHECK:       (type $specific-func.sub (sub $specific-func (func (result i32))))
    (type $specific-func.sub (sub $specific-func (func (result i32))))

    ;; CHECK:       (type $vtable.sub (sub $vtable (struct (field (ref $specific-func)))))
    (type $vtable.sub (sub $vtable (struct (field (ref $specific-func)))))
  )

  ;; CHECK:      (global $A$vtable (ref $vtable) (struct.new $vtable
  ;; CHECK-NEXT:  (ref.func $A$func)
  ;; CHECK-NEXT: ))
  (global $A$vtable (ref $vtable) (struct.new $vtable
    (ref.func $A$func)
  ))

  ;; CHECK:      (global $B$vtable (ref $vtable) (struct.new $vtable
  ;; CHECK-NEXT:  (ref.func $B$func)
  ;; CHECK-NEXT: ))
  (global $B$vtable (ref $vtable) (struct.new $vtable
    (ref.func $B$func)
  ))

  ;; CHECK:      (func $test-expanded (type $0) (param $x i32) (result funcref)
  ;; CHECK-NEXT:  (select (result (ref $specific-func))
  ;; CHECK-NEXT:   (ref.func $A$func)
  ;; CHECK-NEXT:   (ref.func $B$func)
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-expanded (export "test-expanded") (param $x i32) (result funcref)
    ;; We can apply the struct.get to the select arms: As the globals are all
    ;; immutable, we can read the function references from them, and emit a
    ;; select on those values, saving the struct.get operation entirely.
    ;;
    ;; Note that this test also checks updating the type of the select, which
    ;; initially returned a struct, and afterwards returns a func.
    (struct.get $vtable 0
      (select
        (global.get $A$vtable)
        (global.get $B$vtable)
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $test-subtyping (type $0) (param $x i32) (result funcref)
  ;; CHECK-NEXT:  (select (result (ref $specific-func))
  ;; CHECK-NEXT:   (ref.func $A$func)
  ;; CHECK-NEXT:   (ref.func $B$func)
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-subtyping (export "test-subtyping") (param $x i32) (result funcref)
    ;; As above, but now we have struct.news directly in the arms, and one is
    ;; of a subtype of the final result (which should not prevent optimization).
    (struct.get $vtable.sub 0
      (select
        (struct.new $vtable.sub
          (ref.func $A$func)
        )
        (struct.new $vtable.sub
          (ref.func $B$func) ;; this function is of a subtype of the field type
        )
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $test-expanded-twice (type $5) (param $x i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-expanded-twice (export "test-expanded-twice") (param $x i32) (result i32)
    ;; As $test-expanded, but we have two operations that can be applied. Both
    ;; references are non-null, so the select arms will become 0.
    (ref.is_null
      (struct.get $vtable 0
        (select
          (global.get $A$vtable)
          (global.get $B$vtable)
          (local.get $x)
        )
      )
    )
  )

  ;; CHECK:      (func $test-expanded-twice-stop (type $6) (param $x i32)
  ;; CHECK-NEXT:  (call $send-i32
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-expanded-twice-stop (export "test-expanded-twice-stop") (param $x i32)
    ;; As $test-expanded-twice, but we stop after two expansions when we fail on
    ;; the call.
    (call $send-i32
      (ref.is_null
        (struct.get $vtable 0
          (select
            (global.get $A$vtable)
            (global.get $B$vtable)
            (local.get $x)
          )
        )
      )
    )
  )

  ;; CHECK:      (func $send-i32 (type $6) (param $x i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $send-i32 (param $x i32)
    ;; Helper for above.
  )

  ;; CHECK:      (func $binary (type $5) (param $param i32) (result i32)
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $param)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $binary (param $param i32) (result i32)
    ;; (Note that this works because immutable globals can be compared.)
    (ref.eq
      (select
        (global.get $A$vtable)
        (global.get $B$vtable)
        (local.get $param)
      )
      (global.get $A$vtable)
    )
  )

  ;; CHECK:      (func $test-trap (type $0) (param $x i32) (result funcref)
  ;; CHECK-NEXT:  (struct.get $vtable 0
  ;; CHECK-NEXT:   (select (result (ref null $vtable))
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:    (global.get $B$vtable)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-trap (export "test-trap") (param $x i32) (result funcref)
    ;; One arm has a null, which makes the struct.get trap, so we ignore this
    ;; for now. TODO: handle traps
    (struct.get $vtable 0
      (select
        (ref.null $vtable)
        (global.get $B$vtable)
        (local.get $x)
      )
    )
  )

  ;; CHECK:      (func $A$func (type $specific-func) (result i32)
  ;; CHECK-NEXT:  (i32.const 1)
  ;; CHECK-NEXT: )
  (func $A$func (type $specific-func) (result i32)
    ;; Helper for above.
    (i32.const 1)
  )

  ;; CHECK:      (func $B$func (type $specific-func.sub) (result i32)
  ;; CHECK-NEXT:  (i32.const 2)
  ;; CHECK-NEXT: )
  (func $B$func (type $specific-func.sub) (result i32)
    ;; Helper for above.
    (i32.const 2)
  )
)

;; References with nesting.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $vtable (sub (struct (field funcref))))
    (type $vtable (sub (struct funcref)))

    ;; CHECK:       (type $itable (sub (struct (field (ref $vtable)))))
    (type $itable (sub (struct (ref $vtable))))
  )

  ;; CHECK:      (global $A$itable (ref $itable) (struct.new $itable
  ;; CHECK-NEXT:  (struct.new $vtable
  ;; CHECK-NEXT:   (ref.func $A$func)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $A$itable (ref $itable) (struct.new $itable
    (struct.new $vtable
      (ref.func $A$func)
    )
  ))

  ;; CHECK:      (global $B$itable (ref $itable) (struct.new $itable
  ;; CHECK-NEXT:  (struct.new $vtable
  ;; CHECK-NEXT:   (ref.func $B$func)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $B$itable (ref $itable) (struct.new $itable
    (struct.new $vtable
      (ref.func $B$func)
    )
  ))

  ;; CHECK:      (func $test-expanded (type $3) (param $x i32) (result funcref)
  ;; CHECK-NEXT:  (select (result (ref $2))
  ;; CHECK-NEXT:   (ref.func $A$func)
  ;; CHECK-NEXT:   (ref.func $B$func)
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-expanded (export "test-expanded") (param $x i32) (result funcref)
    ;; Nesting in the global declarations means we cannot precompute the inner
    ;; struct.get by itself (as that would return the inner struct.new), but
    ;; when we do the outer struct.get, it all comes together.
    (struct.get $vtable 0
      (struct.get $itable 0
        (select
          (global.get $A$itable)
          (global.get $B$itable)
          (local.get $x)
        )
      )
    )
  )

  ;; CHECK:      (func $A$func (type $2)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $A$func
    ;; Helper for above.
  )

  ;; CHECK:      (func $B$func (type $2)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $B$func
    ;; Helper for above.
  )
)
