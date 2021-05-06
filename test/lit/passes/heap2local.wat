;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; (remove-unused-names allows the pass to see that blocks flow values)
;; RUN: wasm-opt %s -all --remove-unused-names --heap2local -S -o - | filecheck %s

(module
  (type $struct.A (struct (field (mut i32)) (field (mut f64))))

  (type $struct.packed (struct (field (mut i8))))

  (type $struct.nondefaultable (struct (field (rtt $struct.A))))

  (type $struct.recursive (struct (field (mut (ref null $struct.recursive)))))

  ;; CHECK:      (func $simple
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:    (rtt.canon $struct.A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple
    ;; Other passes can remove such a simple case of an unneeded allocation, and
    ;; we do not bother with it, as there are struct get/set operations that we
    ;; can optimize out.
    (drop
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
  )

  ;; CHECK:      (func $to-local
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local.set $ref
  ;; CHECK-NEXT:   (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:    (rtt.canon $struct.A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $to-local
    (local $ref (ref null $struct.A))
    ;; While set to a local, this allocation has no get/set operations, so we
    ;; ignore it.
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
  )

  ;; CHECK:      (func $one-get
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local $1 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (block (result (ref $struct.A))
  ;; CHECK-NEXT:      (local.set $0
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $1
  ;; CHECK-NEXT:       (f64.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:       (rtt.canon $struct.A)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $one-get
    ;; An allocation followed by an immediate get of a field. This is a non-
    ;; escaping allocation, with a use, so we can optimize it out. The
    ;; allocation is dropped (letting later opts remove it), and the allocation's
    ;; data is moved to locals: we write the initial value to the locals, and
    ;; we read from the locals instead of the struct.get.
    (drop
      (struct.get $struct.A 0
        (struct.new_default_with_rtt $struct.A
          (rtt.canon $struct.A)
        )
      )
    )
  )

  ;; CHECK:      (func $one-get-b
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local $1 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result f64)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (block (result (ref $struct.A))
  ;; CHECK-NEXT:      (local.set $0
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $1
  ;; CHECK-NEXT:       (f64.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:       (rtt.canon $struct.A)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $one-get-b
    ;; Similar to the above, but using a different field index.
    (drop
      (struct.get $struct.A 1
        (struct.new_default_with_rtt $struct.A
          (rtt.canon $struct.A)
        )
      )
    )
  )

  ;; CHECK:      (func $one-set
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local $1 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $one-set
    ;; A simple optimizable allocation only used in one set.
    (struct.set $struct.A 0
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
      (i32.const 1)
    )
  )

  ;; CHECK:      (func $packed
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get_u $struct.packed 0
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.packed
  ;; CHECK-NEXT:     (rtt.canon $struct.packed)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $packed
    ;; We do not optimize packed structs yet.
    (drop
      (struct.get $struct.packed 0
        (struct.new_default_with_rtt $struct.packed
          (rtt.canon $struct.packed)
        )
      )
    )
  )

  ;; CHECK:      (func $with-init-values
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local $1 f64)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (block (result (ref $struct.A))
  ;; CHECK-NEXT:      (local.set $2
  ;; CHECK-NEXT:       (i32.const 2)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $3
  ;; CHECK-NEXT:       (f64.const 3.14159)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $0
  ;; CHECK-NEXT:       (local.get $2)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $1
  ;; CHECK-NEXT:       (local.get $3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (struct.new_with_rtt $struct.A
  ;; CHECK-NEXT:       (local.get $0)
  ;; CHECK-NEXT:       (local.get $1)
  ;; CHECK-NEXT:       (rtt.canon $struct.A)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $with-init-values
    ;; When we get values to initialize the struct with, assign them to the
    ;; proper locals.
    (drop
      (struct.get $struct.A 0
        (struct.new_with_rtt $struct.A
          (i32.const 2)
          (f64.const 3.14159)
          (rtt.canon $struct.A)
        )
      )
    )
  )

  ;; CHECK:      (func $nondefaultable
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (struct.get $struct.nondefaultable 0
  ;; CHECK-NEXT:    (struct.new_with_rtt $struct.nondefaultable
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:     (rtt.canon $struct.nondefaultable)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $nondefaultable
    ;; We do not optimize structs with nondefaultable types that we cannot
    ;; handle, like rtts.
    (drop
      (struct.get $struct.nondefaultable 0
        (struct.new_with_rtt $struct.nondefaultable
          (rtt.canon $struct.A)
          (rtt.canon $struct.nondefaultable)
        )
      )
    )
  )

  ;; CHECK:      (func $simple-one-local-set
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $1
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-one-local-set
    (local $ref (ref null $struct.A))
    ;; A simple optimizable allocation only used in one set, and also stored
    ;; to a local. The local.set should not prevent our optimization, and the
    ;; local.set can be turned into a drop.
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    (struct.set $struct.A 0
      (local.get $ref)
      (i32.const 1)
    )
  )

  ;; CHECK:      (func $simple-one-local-get (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result f64)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $simple-one-local-get (result f64)
    (local $ref (ref null $struct.A))
    ;; A simple optimizable allocation only used in one set, and also stored
    ;; to a local. The local.set should not prevent our optimization, and the
    ;; local.set can be turned into a drop.
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; FIXME fix it and move to the endd
  ;; CHECK:      (func $with-init-values-loop
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (loop $loop
  ;; CHECK-NEXT:   (local.set $ref
  ;; CHECK-NEXT:    (struct.new_with_rtt $struct.A
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:     (block (result f64)
  ;; CHECK-NEXT:      (drop
  ;; CHECK-NEXT:       (struct.get $struct.A 0
  ;; CHECK-NEXT:        (local.get $ref)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (f64.const 2.1828)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $loop)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $with-init-values-loop
    (local $ref (ref null $struct.A))
    ;; A testcase showing why we need extra temp locals when assigning
    ;; the initial values: they must all be present at once when the
    ;; "allocation" happens, as the local might be used before.
    (loop $loop
      (local.set $ref
        (struct.new_with_rtt $struct.A
          (i32.const 2)
          (block (result f64)
            ;; imagine that we check if the reference is not null here, and if
            ;; not then we read from the struct.
            (drop
              ;; A get from the struct. This should return the old value,
              ;; before the assignment of "2" a few lines above us
              (struct.get $struct.A 0
                (local.get $ref)
              )
            )
            (f64.const 2.1828)
          )
          (rtt.canon $struct.A)
        )
        (struct.set $struct.A 0
          (local.get $ref)
          (i32.const 3)
        )
      )
      (br $loop)
    )
  )

  ;; CHECK:      (func $send-ref (param $0 (ref null $struct.A))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $send-ref (param (ref null $struct.A))
  )

  ;; CHECK:      (func $safe-to-drop (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result f64)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $safe-to-drop (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; An extra drop does not let the allocation escape.
    (drop
      (local.get $ref)
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $escape-via-call (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local.set $ref
  ;; CHECK-NEXT:   (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:    (rtt.canon $struct.A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $send-ref
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.get $struct.A 1
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $escape-via-call (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; The allocation escapes into a call.
    (call $send-ref
      (local.get $ref)
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $safe-to-drop-multiflow (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref null $struct.A))
  ;; CHECK-NEXT:    (block (result (ref null $struct.A))
  ;; CHECK-NEXT:     (local.get $ref)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result f64)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $safe-to-drop-multiflow (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; An extra drop + multiple flows through things do not stop us.
    (drop
      (block (result (ref null $struct.A))
        (block (result (ref null $struct.A))
          (loop (result (ref null $struct.A))
            (local.get $ref)
          )
        )
      )
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $escape-after-multiflow (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local.set $ref
  ;; CHECK-NEXT:   (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:    (rtt.canon $struct.A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $send-ref
  ;; CHECK-NEXT:   (block (result (ref null $struct.A))
  ;; CHECK-NEXT:    (block (result (ref null $struct.A))
  ;; CHECK-NEXT:     (local.get $ref)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.get $struct.A 1
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $escape-after-multiflow (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; An escape after multiple flows.
    (call $send-ref
      (block (result (ref null $struct.A))
        (block (result (ref null $struct.A))
          (loop (result (ref null $struct.A))
            (local.get $ref)
          )
        )
      )
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $non-exclusive-set (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local.set $ref
  ;; CHECK-NEXT:   (select (result (ref $struct.A))
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.get $struct.A 1
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $non-exclusive-set (result f64)
    (local $ref (ref null $struct.A))
    ;; A set that receives two different allocations, and so we should not try
    ;; to optimize it.
    (local.set $ref
      (select
        (struct.new_default_with_rtt $struct.A
          (rtt.canon $struct.A)
        )
        (struct.new_default_with_rtt $struct.A
          (rtt.canon $struct.A)
        )
        (i32.const 1)
      )
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $local-copies (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result f64)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $local-copies (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; Copying our allocation through locals does not bother us.
    (local.set $ref
      (local.get $ref)
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $local-copies-conditional (param $x i32) (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $3
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result f64)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (local.get $ref)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $local-copies-conditional (param $x i32) (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; Possibly copying our allocation through locals does not bother us. Note
    ;; that as a result of this the final local.get has two sets that send it
    ;; values, but we know they are both the same allocation.
    (if (local.get $x)
      (local.set $ref
        (local.get $ref)
      )
    )
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $branch-value (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result f64)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (block (result (ref null $struct.A))
  ;; CHECK-NEXT:     (call $send-ref
  ;; CHECK-NEXT:      (ref.null $struct.A)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (local.get $ref)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $branch-value (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    ;; Sending our allocation through a branch does not bother us.
    (struct.get $struct.A 1
      (block $block (result (ref null $struct.A))
        ;; Before the reference are some things that should not confuse us.
        (call $send-ref
          (ref.null $struct.A)
        )
        (local.get $ref)
      )
    )
  )

  ;; CHECK:      (func $non-exclusive-get (param $x i32) (result f64)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local.set $ref
  ;; CHECK-NEXT:   (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:    (rtt.canon $struct.A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (local.set $ref
  ;; CHECK-NEXT:    (ref.null $struct.A)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.get $struct.A 1
  ;; CHECK-NEXT:   (local.get $ref)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $non-exclusive-get (param $x i32) (result f64)
    (local $ref (ref null $struct.A))
    (local.set $ref
      (struct.new_default_with_rtt $struct.A
        (rtt.canon $struct.A)
      )
    )
    (if (local.get $x)
      (local.set $ref
        (ref.null $struct.A)
      )
    )
    ;; A get that receives two different allocations, and so we should not try
    ;; to optimize it.
    (struct.get $struct.A 1
      (local.get $ref)
    )
  )

  ;; CHECK:      (func $tee (result i32)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $struct.A))
  ;; CHECK-NEXT:    (local.set $1
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (f64.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $1)
  ;; CHECK-NEXT: )
  (func $tee (result i32)
    (local $ref (ref null $struct.A))
    (struct.get $struct.A 0
      ;; A tee flows out the value, and we can optimize this allocation.
      (local.tee $ref
        (struct.new_default_with_rtt $struct.A
          (rtt.canon $struct.A)
        )
      )
    )
  )

  ;; CHECK:      (func $escape-flow-out (result anyref)
  ;; CHECK-NEXT:  (local $ref (ref null $struct.A))
  ;; CHECK-NEXT:  (struct.set $struct.A 0
  ;; CHECK-NEXT:   (local.tee $ref
  ;; CHECK-NEXT:    (struct.new_default_with_rtt $struct.A
  ;; CHECK-NEXT:     (rtt.canon $struct.A)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $ref)
  ;; CHECK-NEXT: )
  (func $escape-flow-out (result anyref)
    (local $ref (ref null $struct.A))
    (struct.set $struct.A 0
      (local.tee $ref
        (struct.new_default_with_rtt $struct.A
          (rtt.canon $struct.A)
        )
      )
      (i32.const 1)
    )
    ;; The allocation escapes out to the caller.
    (local.get $ref)
  )
)
