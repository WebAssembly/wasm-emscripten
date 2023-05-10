;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; Run in both TNH and non-TNH mode.

;; RUN: wasm-opt %s --vacuum --traps-never-happen -all -S -o - | filecheck %s --check-prefix=YESTNH
;; RUN: wasm-opt %s --vacuum                      -all -S -o - | filecheck %s --check-prefix=NO_TNH

(module
  ;; YESTNH:      (type $struct (struct (field (mut i32))))

  ;; YESTNH:      (tag $tag (param i32))
  ;; NO_TNH:      (type $struct (struct (field (mut i32))))

  ;; NO_TNH:      (tag $tag (param i32))
  (tag $tag (param i32))

  (memory 1 1)

  (type $struct (struct (field (mut i32))))

  ;; YESTNH:      (func $drop (type $i32_anyref_=>_none) (param $x i32) (param $y anyref)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $drop (type $i32_anyref_=>_none) (param $x i32) (param $y anyref)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (i32.load
  ;; NO_TNH-NEXT:    (local.get $x)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.as_non_null
  ;; NO_TNH-NEXT:    (local.get $y)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (ref.as_i31
  ;; NO_TNH-NEXT:    (local.get $y)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $drop (param $x i32) (param $y anyref)
    ;; A load might trap, normally, but if traps never happen then we can
    ;; remove it.
    (drop
      (i32.load (local.get $x))
    )

    ;; A trap on a null value can also be ignored.
    (drop
      (ref.as_non_null
        (local.get $y)
      )
    )

    ;; Other ref.as* as well.
    (drop
      (ref.as_i31
        (local.get $y)
      )
    )

    ;; Ignore unreachable code.
    (drop
      (unreachable)
    )
  )

  ;; Other side effects prevent us making any changes.
  ;; YESTNH:      (func $other-side-effects (type $i32_=>_i32) (param $x i32) (result i32)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (call $other-side-effects
  ;; YESTNH-NEXT:    (i32.const 1)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (local.set $x
  ;; YESTNH-NEXT:   (i32.const 2)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (i32.const 1)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $other-side-effects (type $i32_=>_i32) (param $x i32) (result i32)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (call $other-side-effects
  ;; NO_TNH-NEXT:    (i32.const 1)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (block (result i32)
  ;; NO_TNH-NEXT:    (local.set $x
  ;; NO_TNH-NEXT:     (i32.const 2)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.load
  ;; NO_TNH-NEXT:     (local.get $x)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (i32.const 1)
  ;; NO_TNH-NEXT: )
  (func $other-side-effects (param $x i32) (result i32)
    ;; A call has all manner of other side effects.
    (drop
      (call $other-side-effects (i32.const 1))
    )

    ;; Add to the load an additional specific side effect, of writing to a
    ;; local. We can remove the load, but not the write to a local.
    (drop
      (block (result i32)
        (local.set $x (i32.const 2))
        (i32.load (local.get $x))
      )
    )

    (i32.const 1)
  )

  ;; A helper function for the above, that returns nothing.
  ;; YESTNH:      (func $return-nothing (type $none_=>_none)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $return-nothing (type $none_=>_none)
  ;; NO_TNH-NEXT:  (nop)
  ;; NO_TNH-NEXT: )
  (func $return-nothing)

  ;; YESTNH:      (func $partial (type $ref|$struct|_=>_ref?|$struct|) (param $x (ref $struct)) (result (ref null $struct))
  ;; YESTNH-NEXT:  (local $y (ref null $struct))
  ;; YESTNH-NEXT:  (local.set $y
  ;; YESTNH-NEXT:   (local.get $x)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (local.set $y
  ;; YESTNH-NEXT:   (local.get $x)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (local.get $y)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $partial (type $ref|$struct|_=>_ref?|$struct|) (param $x (ref $struct)) (result (ref null $struct))
  ;; NO_TNH-NEXT:  (local $y (ref null $struct))
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.get $struct 0
  ;; NO_TNH-NEXT:    (local.tee $y
  ;; NO_TNH-NEXT:     (local.get $x)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (struct.get $struct 0
  ;; NO_TNH-NEXT:    (local.tee $y
  ;; NO_TNH-NEXT:     (local.get $x)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (local.get $y)
  ;; NO_TNH-NEXT: )
  (func $partial (param $x (ref $struct)) (result (ref null $struct))
    (local $y (ref null $struct))
    ;; The struct.get's side effect can be ignored due to tnh, and the value is
    ;; dropped anyhow, so we can remove it. We cannot remove the local.tee
    ;; inside it, however, so we must only vacuum out the struct.get and
    ;; nothing more. (In addition, a drop of a tee will become a set.)
    (drop
      (struct.get $struct 0
        (local.tee $y
          (local.get $x)
        )
      )
    )
    ;; Similar, but with an eqz on the outside, which can also be removed.
    (drop
      (i32.eqz
        (struct.get $struct 0
          (local.tee $y
            (local.get $x)
          )
        )
      )
    )
    (local.get $y)
  )

  ;; YESTNH:      (func $toplevel (type $none_=>_none)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $toplevel (type $none_=>_none)
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $toplevel
    ;; A removable side effect at the top level of a function. We can turn this
    ;; into a nop.
    (unreachable)
  )

  ;; YESTNH:      (func $drop-loop (type $none_=>_none)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (loop $loop (result i32)
  ;; YESTNH-NEXT:    (br_if $loop
  ;; YESTNH-NEXT:     (i32.const 1)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (i32.const 10)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $drop-loop (type $none_=>_none)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (loop $loop (result i32)
  ;; NO_TNH-NEXT:    (br_if $loop
  ;; NO_TNH-NEXT:     (i32.const 1)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.const 10)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $drop-loop
    ;; A loop has the effect of potentially being infinite. Even in TNH mode we
    ;; do not optimize out such loops.
    (drop
      (loop $loop (result i32)
        (br_if $loop
          (i32.const 1)
        )
        (i32.const 10)
      )
    )
  )

  ;; YESTNH:      (func $loop-effects (type $none_=>_none)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (loop $loop (result i32)
  ;; YESTNH-NEXT:    (drop
  ;; YESTNH-NEXT:     (i32.atomic.load
  ;; YESTNH-NEXT:      (i32.const 0)
  ;; YESTNH-NEXT:     )
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (br_if $loop
  ;; YESTNH-NEXT:     (i32.const 1)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (i32.const 10)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $loop-effects (type $none_=>_none)
  ;; NO_TNH-NEXT:  (drop
  ;; NO_TNH-NEXT:   (loop $loop (result i32)
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (i32.atomic.load
  ;; NO_TNH-NEXT:      (i32.const 0)
  ;; NO_TNH-NEXT:     )
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (br_if $loop
  ;; NO_TNH-NEXT:     (i32.const 1)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.const 10)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $loop-effects
    ;; As above, but the loop also has an atomic load effect. That prevents
    ;; optimization.
    (drop
      (loop $loop (result i32)
        (drop
          (i32.atomic.load
            (i32.const 0)
          )
        )
        (br_if $loop
          (i32.const 1)
        )
        (i32.const 10)
      )
    )
  )

  ;; YESTNH:      (func $if-unreachable (type $i32_=>_none) (param $p i32)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (local.get $p)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (block
  ;; YESTNH-NEXT:   (drop
  ;; YESTNH-NEXT:    (local.get $p)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:   (call $if-unreachable
  ;; YESTNH-NEXT:    (i32.const 0)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (if
  ;; YESTNH-NEXT:   (local.get $p)
  ;; YESTNH-NEXT:   (unreachable)
  ;; YESTNH-NEXT:   (unreachable)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $if-unreachable (type $i32_=>_none) (param $p i32)
  ;; NO_TNH-NEXT:  (if
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (if
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (call $if-unreachable
  ;; NO_TNH-NEXT:    (i32.const 0)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (if
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $if-unreachable (param $p i32)
    ;; The if arm can be nopped, as in tnh we assume we never reach it.
    (if
      (local.get $p)
      (unreachable)
    )
    ;; This else arm can be removed.
    (if
      (local.get $p)
      (call $if-unreachable
        (i32.const 0)
      )
      (unreachable)
    )
    ;; Both of these can be removed, but we leave this for DCE to handle.
    (if
      (local.get $p)
      (unreachable)
      (unreachable)
    )
  )

  ;; YESTNH:      (func $if-unreachable-value (type $i32_=>_i32) (param $p i32) (result i32)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (local.get $p)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (i32.const 1)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $if-unreachable-value (type $i32_=>_i32) (param $p i32) (result i32)
  ;; NO_TNH-NEXT:  (if (result i32)
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $if-unreachable-value (param $p i32) (result i32)
    ;; When removing the unreachable arm we must update the IR properly, as it
    ;; cannot have a nop there.
    (if (result i32)
      (local.get $p)
      (unreachable)
      (i32.const 1)
    )
  )

  ;; YESTNH:      (func $if-unreachable-value-2 (type $i32_=>_i32) (param $p i32) (result i32)
  ;; YESTNH-NEXT:  (drop
  ;; YESTNH-NEXT:   (local.get $p)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (i32.const 1)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $if-unreachable-value-2 (type $i32_=>_i32) (param $p i32) (result i32)
  ;; NO_TNH-NEXT:  (if (result i32)
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:   (unreachable)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $if-unreachable-value-2 (param $p i32) (result i32)
    ;; As above but in the other arm.
    (if (result i32)
      (local.get $p)
      (i32.const 1)
      (unreachable)
    )
  )

  ;; YESTNH:      (func $block-unreachable (type $i32_=>_none) (param $p i32)
  ;; YESTNH-NEXT:  (if
  ;; YESTNH-NEXT:   (local.get $p)
  ;; YESTNH-NEXT:   (block
  ;; YESTNH-NEXT:    (i32.store
  ;; YESTNH-NEXT:     (i32.const 0)
  ;; YESTNH-NEXT:     (i32.const 1)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (if
  ;; YESTNH-NEXT:     (local.get $p)
  ;; YESTNH-NEXT:     (return)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $block-unreachable (type $i32_=>_none) (param $p i32)
  ;; NO_TNH-NEXT:  (if
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (block
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 0)
  ;; NO_TNH-NEXT:     (i32.const 1)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (if
  ;; NO_TNH-NEXT:     (local.get $p)
  ;; NO_TNH-NEXT:     (return)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 2)
  ;; NO_TNH-NEXT:     (i32.const 3)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $block-unreachable (param $p i32)
    (if
      (local.get $p)
      (block
        (i32.store
          (i32.const 0)
          (i32.const 1)
        )
        (if
          (local.get $p)
          (return)
        )
        ;; This store can be removed as it leads up to an unreachable which we
        ;; assume is never reached.
        (i32.store
          (i32.const 2)
          (i32.const 3)
        )
        (unreachable)
      )
    )
  )

  ;; YESTNH:      (func $block-unreachable-named (type $i32_=>_none) (param $p i32)
  ;; YESTNH-NEXT:  (if
  ;; YESTNH-NEXT:   (local.get $p)
  ;; YESTNH-NEXT:   (block $named
  ;; YESTNH-NEXT:    (i32.store
  ;; YESTNH-NEXT:     (i32.const 0)
  ;; YESTNH-NEXT:     (i32.const 1)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (br_if $named
  ;; YESTNH-NEXT:     (local.get $p)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $block-unreachable-named (type $i32_=>_none) (param $p i32)
  ;; NO_TNH-NEXT:  (if
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (block $named
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 0)
  ;; NO_TNH-NEXT:     (i32.const 1)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (br_if $named
  ;; NO_TNH-NEXT:     (local.get $p)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 2)
  ;; NO_TNH-NEXT:     (i32.const 3)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $block-unreachable-named (param $p i32)
    (if
      (local.get $p)
      (block $named
        (i32.store
          (i32.const 0)
          (i32.const 1)
        )
        ;; As above, but now the block is named and we use a br_if. We should
        ;; again only remove the last store.
        (br_if $named
          (local.get $p)
        )
        (i32.store
          (i32.const 2)
          (i32.const 3)
        )
        (unreachable)
      )
    )
  )

  ;; YESTNH:      (func $block-unreachable-all (type $i32_=>_none) (param $p i32)
  ;; YESTNH-NEXT:  (nop)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $block-unreachable-all (type $i32_=>_none) (param $p i32)
  ;; NO_TNH-NEXT:  (if
  ;; NO_TNH-NEXT:   (local.get $p)
  ;; NO_TNH-NEXT:   (block
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 0)
  ;; NO_TNH-NEXT:     (i32.const 1)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 2)
  ;; NO_TNH-NEXT:     (i32.const 3)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $block-unreachable-all (param $p i32)
    (if
      (local.get $p)
      (block
        ;; Both stores can be removed, and even the entire if arm and then the
        ;; entire if.
        (i32.store
          (i32.const 0)
          (i32.const 1)
        )
        (i32.store
          (i32.const 2)
          (i32.const 3)
        )
        (unreachable)
      )
    )
  )

  ;; YESTNH:      (func $block-unreachable-but-call (type $none_=>_none)
  ;; YESTNH-NEXT:  (i32.store
  ;; YESTNH-NEXT:   (i32.const 0)
  ;; YESTNH-NEXT:   (i32.const 1)
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT:  (call $block-unreachable-but-call)
  ;; YESTNH-NEXT:  (unreachable)
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $block-unreachable-but-call (type $none_=>_none)
  ;; NO_TNH-NEXT:  (i32.store
  ;; NO_TNH-NEXT:   (i32.const 0)
  ;; NO_TNH-NEXT:   (i32.const 1)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (call $block-unreachable-but-call)
  ;; NO_TNH-NEXT:  (i32.store
  ;; NO_TNH-NEXT:   (i32.const 2)
  ;; NO_TNH-NEXT:   (i32.const 3)
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT:  (unreachable)
  ;; NO_TNH-NEXT: )
  (func $block-unreachable-but-call
    ;; A call cannot be removed, even if it leads to a trap, since it might have
    ;; non-trap effects (like mayNotReturn). We can remove the store after it,
    ;; though.
    (i32.store
      (i32.const 0)
      (i32.const 1)
    )
    (call $block-unreachable-but-call)
    (i32.store
      (i32.const 2)
      (i32.const 3)
    )
    (unreachable)
  )

  ;; YESTNH:      (func $catch-pop (type $none_=>_none)
  ;; YESTNH-NEXT:  (try $try
  ;; YESTNH-NEXT:   (do
  ;; YESTNH-NEXT:    (call $catch-pop)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:   (catch $tag
  ;; YESTNH-NEXT:    (drop
  ;; YESTNH-NEXT:     (pop i32)
  ;; YESTNH-NEXT:    )
  ;; YESTNH-NEXT:    (unreachable)
  ;; YESTNH-NEXT:   )
  ;; YESTNH-NEXT:  )
  ;; YESTNH-NEXT: )
  ;; NO_TNH:      (func $catch-pop (type $none_=>_none)
  ;; NO_TNH-NEXT:  (try $try
  ;; NO_TNH-NEXT:   (do
  ;; NO_TNH-NEXT:    (call $catch-pop)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:   (catch $tag
  ;; NO_TNH-NEXT:    (drop
  ;; NO_TNH-NEXT:     (pop i32)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (i32.store
  ;; NO_TNH-NEXT:     (i32.const 0)
  ;; NO_TNH-NEXT:     (i32.const 1)
  ;; NO_TNH-NEXT:    )
  ;; NO_TNH-NEXT:    (unreachable)
  ;; NO_TNH-NEXT:   )
  ;; NO_TNH-NEXT:  )
  ;; NO_TNH-NEXT: )
  (func $catch-pop
    (try $try
      (do
        ;; Put a call here so the entire try-catch is not removed as trivial.
        (call $catch-pop)
      )
      (catch $tag
        ;; A pop on the way to a trap cannot be removed. But the store can.
        ;; TODO: The pop can actually be removed since it is valid per the spec
        ;;       because of the unreachable afterwards. We need to fix our
        ;;       validation rules to handle that though.
        (drop
          (pop i32)
        )
        (i32.store
          (i32.const 0)
          (i32.const 1)
        )
        (unreachable)
      )
    )
  )
)
