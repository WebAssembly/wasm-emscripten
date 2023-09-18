;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: wasm-opt %s --simplify-locals -all -S -o - \
;; RUN:   | filecheck %s

(module
  ;; CHECK:      (type $A (sub (struct (field structref))))

  ;; CHECK:      (type $B (sub $A (struct (field (ref struct)))))

  ;; CHECK:      (type $struct (struct (field (mut i32))))
  (type $struct (struct (field (mut i32))))

  ;; CHECK:      (type $struct-immutable (struct (field i32)))
  (type $struct-immutable (struct (field i32)))

  (type $A (sub (struct (field (ref null struct)))))

  ;; $B is a subtype of $A, and its field has a more refined type (it is non-
  ;; nullable).
  (type $B (sub $A (struct (field (ref struct)))))

  ;; Writes to heap objects cannot be reordered with reads.
  ;; CHECK:      (func $no-reorder-past-write (type $4) (param $x (ref $struct)) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  (local.set $temp
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $struct 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $temp)
  ;; CHECK-NEXT: )
  (func $no-reorder-past-write (param $x (ref $struct)) (result i32)
    (local $temp i32)
    (local.set $temp
      (struct.get $struct 0
        (local.get $x)
      )
    )
    (struct.set $struct 0
      (local.get $x)
      (i32.const 42)
    )
    (local.get $temp)
  )

  ;; CHECK:      (func $reorder-past-write-if-immutable (type $6) (param $x (ref $struct)) (param $y (ref $struct-immutable)) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (struct.set $struct 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.get $struct-immutable 0
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $reorder-past-write-if-immutable (param $x (ref $struct)) (param $y (ref $struct-immutable)) (result i32)
    (local $temp i32)
    (local.set $temp
      (struct.get $struct-immutable 0
        (local.get $y)
      )
    )
    (struct.set $struct 0
      (local.get $x)
      (i32.const 42)
    )
    (local.get $temp)
  )

  ;; CHECK:      (func $unreachable-struct.get (type $6) (param $x (ref $struct)) (param $y (ref $struct-immutable)) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  (local.tee $temp
  ;; CHECK-NEXT:   (block ;; (replaces something unreachable we can't emit)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (unreachable)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.set $struct 0
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $temp)
  ;; CHECK-NEXT: )
  (func $unreachable-struct.get (param $x (ref $struct)) (param $y (ref $struct-immutable)) (result i32)
    (local $temp i32)
    ;; As above, but the get's ref is unreachable. This tests we do not hit an
    ;; assertion on the get's type not having a heap type (as we depend on
    ;; finding the heap type there in the reachable case).
    ;; We simply do not handle this case, leaving it for DCE.
    (local.set $temp
      (struct.get $struct-immutable 0
        (unreachable)
      )
    )
    (struct.set $struct 0
      (local.get $x)
      (i32.const 42)
    )
    (local.get $temp)
  )

  ;; CHECK:      (func $no-block-values-if-br_on (type $3)
  ;; CHECK-NEXT:  (local $temp anyref)
  ;; CHECK-NEXT:  (block $block
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (br_on_null $block
  ;; CHECK-NEXT:     (ref.null none)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $temp
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $block)
  ;; CHECK-NEXT:   (local.set $temp
  ;; CHECK-NEXT:    (ref.null none)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.as_non_null
  ;; CHECK-NEXT:    (local.get $temp)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-block-values-if-br_on
   (local $temp (ref null any))
   (block $block
    (drop
     ;; This br_on should inhibit trying to create a block return value for
     ;; this block. Aside from the br_on, it looks correct, i.e., we have a
     ;; break with a set before it, and a set before the end of the block. Due
     ;; to the br_on's presence, the pass should not do anything to this
     ;; function.
     ;;
     ;; TODO: support br_on in this optimization eventually, but the variable
     ;;       possible return values and sent values make that nontrivial.
     (br_on_null $block
      (ref.null any)
     )
    )
    (local.set $temp
     (ref.null any)
    )
    (br $block)
    (local.set $temp
     (ref.null any)
    )
   )
   ;; Attempt to use the local that the pass will try to move to a block return
   ;; value, to cause the optimization to try to run.
   (drop
    (ref.as_non_null
     (local.get $temp)
    )
   )
  )

  ;; CHECK:      (func $if-nnl (type $3)
  ;; CHECK-NEXT:  (local $x (ref func))
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (ref.func $if-nnl)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $helper
  ;; CHECK-NEXT:   (local.tee $x
  ;; CHECK-NEXT:    (ref.func $if-nnl)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $helper
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $if-nnl
   (local $x (ref func))
   ;; We want to turn this if into an if-else with a set on the outside:
   ;;
   ;;  (local.set $x
   ;;   (if
   ;;    (i32.const 1)
   ;;    (ref.func $if-nnl)
   ;;    (local.get $x)))
   ;;
   ;; That will not validate, however (no set dominates the get), so we'll get
   ;; fixed up by adding a ref.as_non_null. But that may be dangerous - if no
   ;; set exists before us, then that new instruction will trap, in fact. So we
   ;; do not optimize here.
   (if
    (i32.const 1)
    (local.set $x
     (ref.func $if-nnl)
    )
   )
   ;; An exta set + gets, just to avoid other optimizations kicking in
   ;; (without them, the function only has a set and nothing else, and will
   ;; remove the set entirely). Nothing should change here.
   (call $helper
    (local.tee $x
     (ref.func $if-nnl)
    )
   )
   (call $helper
    (local.get $x)
   )
  )

  ;; CHECK:      (func $if-nnl-previous-set (type $3)
  ;; CHECK-NEXT:  (local $x (ref func))
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (ref.func $if-nnl)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:   (local.set $x
  ;; CHECK-NEXT:    (ref.func $if-nnl)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $helper
  ;; CHECK-NEXT:   (local.tee $x
  ;; CHECK-NEXT:    (ref.func $if-nnl)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $helper
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $if-nnl-previous-set
   (local $x (ref func))
   ;; As the above testcase, but now there is a set before the if. We could
   ;; optimize in this case, but don't atm. TODO
   (local.set $x
    (ref.func $if-nnl)
   )
   (if
    (i32.const 1)
    (local.set $x
     (ref.func $if-nnl)
    )
   )
   (call $helper
    (local.tee $x
     (ref.func $if-nnl)
    )
   )
   (call $helper
    (local.get $x)
   )
  )

  ;; CHECK:      (func $helper (type $8) (param $ref (ref func))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $helper (param $ref (ref func))
  )

  ;; CHECK:      (func $needs-refinalize (type $9) (param $b (ref $B)) (result anyref)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (struct.get $B 0
  ;; CHECK-NEXT:   (local.get $b)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $needs-refinalize (param $b (ref $B)) (result anyref)
    (local $a (ref null $A))
    (local.set $a
      (local.get $b)
    )
    ;; This begins as a struct.get of $A, but after we move the set's value onto
    ;; the get, we'll be reading from $B. $B's field has a more refined type, so
    ;; we must update the type of the struct.get using refinalize.
    (struct.get $A 0
      (local.get $a)
    )
  )

  ;; CHECK:      (func $call-vs-mutable-read (type $4) (param $0 (ref $struct)) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  (local.set $temp
  ;; CHECK-NEXT:   (call $side-effect)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct 0
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $temp)
  ;; CHECK-NEXT: )
  (func $call-vs-mutable-read (param $0 (ref $struct)) (result i32)
    (local $temp i32)
    (local.set $temp
      ;; This call may have arbitrary side effects, for all we know, as we
      ;; optimize this function using --simplify-locals.
      (call $side-effect)
    )
    (drop
      ;; This reads a mutable field, which means the call might modify it.
      (struct.get $struct 0
        (local.get $0)
      )
    )
    ;; We should not move the call to here!
    (local.get $temp)
  )

  ;; CHECK:      (func $side-effect (type $10) (result i32)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $side-effect (result i32)
    ;; Helper function for the above.
    (unreachable)
  )

  ;; CHECK:      (func $pick-refined (type $11) (param $nn-any (ref any)) (result anyref)
  ;; CHECK-NEXT:  (local $any anyref)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (call $use-any
  ;; CHECK-NEXT:   (local.get $nn-any)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $use-nn-any
  ;; CHECK-NEXT:   (local.get $nn-any)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (local.get $nn-any)
  ;; CHECK-NEXT: )
  (func $pick-refined (param $nn-any (ref any)) (result anyref)
    (local $any anyref)
    (local.set $any
      (local.get $nn-any)
    )
    ;; Use the locals so neither is trivially removed.
    (call $use-any
      (local.get $any)
    )
    (call $use-nn-any
      (local.get $nn-any)
    )
    ;; This copy is not needed, as they hold the same value.
    (local.set $any
      (local.get $nn-any)
    )
    ;; This local.get might as well use the non-nullable local, which is more
    ;; refined. In fact, all uses of locals can be switched to that one in the
    ;; entire function (and the other local would be removed by other passes).
    (local.get $any)
  )

  ;; CHECK:      (func $pick-casted (type $12) (param $any anyref) (result anyref)
  ;; CHECK-NEXT:  (local $nn-any (ref any))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (call $use-any
  ;; CHECK-NEXT:   (local.tee $nn-any
  ;; CHECK-NEXT:    (ref.as_non_null
  ;; CHECK-NEXT:     (local.get $any)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $use-nn-any
  ;; CHECK-NEXT:   (local.get $nn-any)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (local.get $nn-any)
  ;; CHECK-NEXT: )
  (func $pick-casted (param $any anyref) (result anyref)
    (local $nn-any (ref any))
    (local.set $nn-any
      (ref.as_non_null
        (local.get $any)
      )
    )
    ;; Use the locals so neither is trivially removed.
    (call $use-any
      (local.get $any)
    )
    (call $use-nn-any
      (local.get $nn-any)
    )
    ;; This copy is not needed, as they hold the same value.
    (local.set $any
      (local.get $nn-any)
    )
    ;; This local.get might as well use the non-nullable local.
    (local.get $any)
  )

  ;; CHECK:      (func $pick-fallthrough (type $13) (param $x i32)
  ;; CHECK-NEXT:  (local $t i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $pick-fallthrough (param $x i32)
    (local $t i32)
    ;; Similar to the above test wth looking through a cast, but using a non-gc
    ;; type of fallthrough value.
    (local.set $t
      (block (result i32)
        (local.get $x)
      )
    )
    ;; The locals are identical, as we set $t = $x (we can look through to the
    ;; block value). Both these gets can go to $x, and we do not need to set $t
    ;; as it will have 0 uses.
    (drop
      (local.get $x)
    )
    (drop
      (local.get $t)
    )
  )

  ;; CHECK:      (func $ignore-unrefined (type $14) (param $A (ref $A))
  ;; CHECK-NEXT:  (local $B (ref null $B))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 0
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 0
  ;; CHECK-NEXT:    (local.tee $B
  ;; CHECK-NEXT:     (ref.cast (ref $B)
  ;; CHECK-NEXT:      (local.get $A)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $A 0
  ;; CHECK-NEXT:    (local.get $A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $B 0
  ;; CHECK-NEXT:    (local.get $B)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $ignore-unrefined (param $A (ref $A))
    ;; $A is a supertype, but non-nullable; $B is a subtype, but nullable. We
    ;; should not switch any of the gets from $B to $A: that would improve
    ;; nullability but not the heap type.
    (local $B (ref null $B))
    (local.set $B
      (ref.cast (ref $B)
        (local.get $A)
      )
    )
    ;; Read from both locals a few times. We should keep reading from the same
    ;; locals as before.
    (drop
      (struct.get $A 0
        (local.get $A)
      )
    )
    (drop
      (struct.get $B 0
        (local.get $B)
      )
    )
    (drop
      (struct.get $A 0
        (local.get $A)
      )
    )
    (drop
      (struct.get $B 0
        (local.get $B)
      )
    )
  )

  ;; CHECK:      (func $use-nn-any (type $15) (param $nn-any (ref any))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $use-nn-any (param $nn-any (ref any))
    ;; Helper function for the above.
  )

  ;; CHECK:      (func $use-any (type $7) (param $any anyref)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $use-any (param $any anyref)
    ;; Helper function for the above.
  )

  ;; CHECK:      (func $remove-tee-refinalize (type $16) (param $a (ref null $A)) (param $b (ref null $B)) (result structref)
  ;; CHECK-NEXT:  (struct.get $B 0
  ;; CHECK-NEXT:   (local.get $b)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $remove-tee-refinalize
    (param $a (ref null $A))
    (param $b (ref null $B))
    (result (ref null struct))

    ;; The local.tee receives a $B and flows out an $A. After we remove it (it is
    ;; obviously unnecessary), the struct.get will be reading from the more
    ;; refined type $B.
    (struct.get $A 0
      (local.tee $a
        (local.get $b)
      )
    )
  )

  ;; CHECK:      (func $redundant-tee-finalize (type $7) (param $x anyref)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast (ref any)
  ;; CHECK-NEXT:    (ref.cast (ref any)
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $redundant-tee-finalize (param $x anyref)
    ;; The tee in the middle will be removed, as it copies a local to itself.
    ;; After doing so, the outer cast should become non-nullable as we
    ;; refinalize.
    (drop
      (ref.cast anyref
        (local.tee $x
          (ref.cast (ref any)
            (local.get $x)
          )
        )
      )
    )
  )

  ;; CHECK:      (func $equivalent-set-removal-branching (type $17) (param $0 i32) (param $any anyref)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (block $block
  ;; CHECK-NEXT:   (local.set $1
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br_if $block
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (br_on_null $block
  ;; CHECK-NEXT:     (local.get $any)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $block)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $equivalent-set-removal-branching (param $0 i32) (param $any anyref)
    (local $1 i32)
    (block $block
      (local.set $1 (local.get $0))
      (br_if $block
        (local.get $0)
      )
      (drop
        (br_on_null $block
          (local.get $any)
        )
      )
      ;; We can optimize these to both use the same local index, as they must
      ;; contain the same value, even past the br_if and br_on_null.
      (drop (local.get $0))
      (drop (local.get $1))
      (br $block)
      ;; But we do not optimize these as they are after an unconditional br
      ;; (so they are unreachable code).
      (drop (local.get $0))
      (drop (local.get $1))
    )
    ;; Past the end of the block we do not optimize. The local.set actually does
    ;; dominate these, but currently we do not realize that in this pass. TODO
    (drop (local.get $0))
    (drop (local.get $1))
  )
)
