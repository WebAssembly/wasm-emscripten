;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --tuple-optimization -all -S -o - | filecheck %s

(module
  ;; CHECK:      (func $just-set (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local.set $1
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $2
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $just-set
    (local $tuple (i32 i32))
    (local.set $tuple
      (tuple.make
        (i32.const 1)
        (i32.const 2)
      )
    )
  )

  ;; CHECK:      (func $just-get (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $just-get
    (local $tuple (i32 i32))
    (drop
      (tuple.extract 0
        (local.get $tuple)
      )
    )
    (drop
      (tuple.extract 1
        (local.get $tuple)
      )
    )
  )

  ;; CHECK:      (func $just-get-bad (type $0) (result i32 i32)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (tuple.extract 0
  ;; CHECK-NEXT:    (local.get $tuple)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (tuple.extract 1
  ;; CHECK-NEXT:    (local.get $tuple)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $tuple)
  ;; CHECK-NEXT: )
  (func $just-get-bad (result i32 i32)
    (local $tuple (i32 i32))
    (drop
      (tuple.extract 0
        (local.get $tuple)
      )
    )
    (drop
      (tuple.extract 1
        (local.get $tuple)
      )
    )
    ;; This get is not used by something we can handle, so we should not try to
    ;; optimize this tuple.
    (local.get $tuple)
  )

  ;; CHECK:      (func $set-and-gets (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $1
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $2
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-and-gets
    (local $tuple (i32 i32))
    (local.set $tuple
      (tuple.make
        (i32.const 1)
        (i32.const 2)
      )
    )
    (drop
      (tuple.extract 0
        (local.get $tuple)
      )
    )
    (drop
      (tuple.extract 1
        (local.get $tuple)
      )
    )
    ;; Add another get for more coverage
    (drop
      (tuple.extract 0
        (local.get $tuple)
      )
    )
  )

  ;; CHECK:      (func $tee (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block
  ;; CHECK-NEXT:    (local.set $4
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $5
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $2
  ;; CHECK-NEXT:    (local.get $4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $3
  ;; CHECK-NEXT:    (local.get $5)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $tee
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    (local.set $tuple
      (local.tee $tuple2
        (tuple.make
          (i32.const 1)
          (i32.const 2)
        )
      )
    )
    ;; Read the first tuple.
    (drop
      (tuple.extract 0
        (local.get $tuple)
      )
    )
    (drop
      (tuple.extract 1
        (local.get $tuple)
      )
    )
    ;; Read the second tuple.
    (drop
      (tuple.extract 0
        (local.get $tuple2)
      )
    )
    (drop
      (tuple.extract 1
        (local.get $tuple2)
      )
    )
  )

  ;; CHECK:      (func $just-tee (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result i32)
  ;; CHECK-NEXT:    (block
  ;; CHECK-NEXT:     (local.set $1
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (local.set $2
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $just-tee
    (local $tuple (i32 i32))
    (drop
      (tuple.extract 0
        (local.tee $tuple
          (tuple.make
            (i32.const 1)
            (i32.const 2)
          )
        )
      )
    )
  )

  ;; CHECK:      (func $just-tee-bad (type $0) (result i32 i32)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local.tee $tuple
  ;; CHECK-NEXT:   (tuple.make
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $just-tee-bad (result i32 i32)
    (local $tuple (i32 i32))
    ;; This tee goes somewhere we cannot handle, so we do not optimize here.
    (local.tee $tuple
      (tuple.make
        (i32.const 1)
        (i32.const 2)
      )
    )
  )

  ;; CHECK:      (func $no-uses (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $4
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $5
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $2
  ;; CHECK-NEXT:   (local.get $4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $3
  ;; CHECK-NEXT:   (local.get $5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-uses
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    ;; The set has no uses, and the tee only has an immediate use. We can
    ;; still optimize both.
    (local.set $tuple
      (local.tee $tuple2
        (tuple.make
          (i32.const 1)
          (i32.const 2)
        )
      )
    )
  )

  ;; CHECK:      (func $corruption-tee (type $0) (result i32 i32)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local.set $tuple
  ;; CHECK-NEXT:   (local.tee $tuple2
  ;; CHECK-NEXT:    (tuple.make
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $tuple2)
  ;; CHECK-NEXT: )
  (func $corruption-tee (result i32 i32)
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    ;; As above, but the tee's tuple is bad and it prevents the other from
    ;; being optimized too, due to the copy between them.
    (local.set $tuple
      (local.tee $tuple2
        (tuple.make
          (i32.const 1)
          (i32.const 2)
        )
      )
    )
    (local.get $tuple2)
  )

  ;; CHECK:      (func $corruption-set (type $0) (result i32 i32)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local.set $tuple
  ;; CHECK-NEXT:   (local.tee $tuple2
  ;; CHECK-NEXT:    (tuple.make
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $tuple)
  ;; CHECK-NEXT: )
  (func $corruption-set (result i32 i32)
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    ;; As above, but now the set is bad.
    (local.set $tuple
      (local.tee $tuple2
        (tuple.make
          (i32.const 1)
          (i32.const 2)
        )
      )
    )
    (local.get $tuple) ;; this changed from $tuple2; same outcome.
  )

  ;; CHECK:      (func $set-after-set (type $1)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $2
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $3
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $4
  ;; CHECK-NEXT:    (local.get $2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $5
  ;; CHECK-NEXT:    (local.get $3)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (local.set $2
  ;; CHECK-NEXT:    (local.get $2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (local.set $3
  ;; CHECK-NEXT:    (local.get $3)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-after-set
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    ;; We can optimize both these tuples.
    (local.set $tuple
      (tuple.make
        (i32.const 1)
        (i32.const 2)
      )
    )
    (local.set $tuple2
      (local.get $tuple)
    )
    ;; Add a copy of a tuple to itself for extra coverage.
    (local.set $tuple
      (local.get $tuple)
    )
  )

  ;; CHECK:      (func $corruption-first-set (type $0) (result i32 i32)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local.set $tuple
  ;; CHECK-NEXT:   (tuple.make
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $tuple2
  ;; CHECK-NEXT:   (local.get $tuple)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $tuple)
  ;; CHECK-NEXT: )
  (func $corruption-first-set (result i32 i32)
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    ;; We can optimize both these tuples.
    (local.set $tuple
      (tuple.make
        (i32.const 1)
        (i32.const 2)
      )
    )
    (local.set $tuple2
      (local.get $tuple)
    )
    ;; This local.get prevents both locals from being optimized.
    (local.get $tuple)
  )

  ;; CHECK:      (func $corruption-second-set (type $0) (result i32 i32)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local $tuple2 (i32 i32))
  ;; CHECK-NEXT:  (local.set $tuple
  ;; CHECK-NEXT:   (tuple.make
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $tuple2
  ;; CHECK-NEXT:   (local.get $tuple)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $tuple2)
  ;; CHECK-NEXT: )
  (func $corruption-second-set (result i32 i32)
    (local $tuple (i32 i32))
    (local $tuple2 (i32 i32))
    (local.set $tuple
      (tuple.make
        (i32.const 1)
        (i32.const 2)
      )
    )
    (local.set $tuple2
      (local.get $tuple)
    )
    (local.get $tuple2) ;; this changed from $tuple; same outcome.
  )

  ;; CHECK:      (func $other (type $1)
  ;; CHECK-NEXT:  (local $other f64)
  ;; CHECK-NEXT:  (local $tuple (i32 i32))
  ;; CHECK-NEXT:  (local.set $other
  ;; CHECK-NEXT:   (local.tee $other
  ;; CHECK-NEXT:    (local.get $other)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $other
    ;; A non-tuple local and all operations on it should be ignored.
    (local $other f64)
    ;; A tuple local with no uses at all should be ignored.
    (local $tuple (i32 i32))
    (local.set $other
      (local.tee $other
        (local.get $other)
      )
    )
  )
)
