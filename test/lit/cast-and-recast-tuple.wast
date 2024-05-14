;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; Part of cast-and-recast.wast, but containing tuples. This is split out
;; because we do not roundtrip tuple-containing code properly. We also use only
;; one roundtrip because of the accumulation of tuple logic, which would
;; otherwise make the output here very hard to read.

;; RUN: wasm-opt %s -all --roundtrip -S -o - | filecheck %s

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (sub (struct )))
    (type $A (sub (struct)))
    ;; CHECK:       (type $B (sub $A (struct )))
    (type $B (sub $A (struct)))
  )

  ;; CHECK:      (func $test-local-tuple-1 (type $5) (param $B (ref $B)) (param $x i32) (result anyref i32)
  ;; CHECK-NEXT:  (local $2 (tuple (ref $B) i32))
  ;; CHECK-NEXT:  (local $3 (ref $B))
  ;; CHECK-NEXT:  (local $4 (tuple (ref $A) i32))
  ;; CHECK-NEXT:  (local.set $4
  ;; CHECK-NEXT:   (block $label$1 (type $3) (result (ref $A) i32)
  ;; CHECK-NEXT:    (local.set $2
  ;; CHECK-NEXT:     (br_if $label$1
  ;; CHECK-NEXT:      (tuple.make 2
  ;; CHECK-NEXT:       (local.get $B)
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (block (result (ref $B))
  ;; CHECK-NEXT:      (local.set $3
  ;; CHECK-NEXT:       (tuple.extract 2 0
  ;; CHECK-NEXT:        (local.get $2)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (drop
  ;; CHECK-NEXT:       (tuple.extract 2 1
  ;; CHECK-NEXT:        (local.get $2)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $3)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (tuple.make 2
  ;; CHECK-NEXT:   (tuple.extract 2 0
  ;; CHECK-NEXT:    (local.get $4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (tuple.extract 2 1
  ;; CHECK-NEXT:    (local.get $4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-local-tuple-1 (param $B (ref $B)) (param $x i32) (result anyref i32)
    ;; A dropped tuple that contains a ref. As it is dropped, we do not need to
    ;; do anything for this br_if. However, due to our general handling of
    ;; tuples the code here will grow quite a bit due the roundtrip, but we can
    ;; at least verify that there is no ref.cast added anywhere here.
    (block $out (result (ref $A) i32)
      (tuple.drop 2
        (br_if $out
          (tuple.make 2
            (local.get $B)
            (i32.const 3)
          )
          (local.get $x)
        )
      )
      (unreachable)
    )
  )

  ;; CHECK:      (func $test-local-tuple-2 (type $7) (param $B (ref $B)) (param $x i32) (result i32 i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 (tuple i32 i32))
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (local $6 (tuple i32 i32))
  ;; CHECK-NEXT:  (local.set $6
  ;; CHECK-NEXT:   (block $label$1 (type $4) (result i32 i32)
  ;; CHECK-NEXT:    (local.set $4
  ;; CHECK-NEXT:     (br_if $label$1
  ;; CHECK-NEXT:      (tuple.make 2
  ;; CHECK-NEXT:       (i32.const -1)
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $temp
  ;; CHECK-NEXT:     (block (result i32)
  ;; CHECK-NEXT:      (local.set $5
  ;; CHECK-NEXT:       (tuple.extract 2 0
  ;; CHECK-NEXT:        (local.get $4)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $3
  ;; CHECK-NEXT:       (tuple.extract 2 1
  ;; CHECK-NEXT:        (local.get $4)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $5)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (tuple.make 2
  ;; CHECK-NEXT:   (tuple.extract 2 0
  ;; CHECK-NEXT:    (local.get $6)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (tuple.extract 2 1
  ;; CHECK-NEXT:    (local.get $6)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-local-tuple-2 (param $B (ref $B)) (param $x i32) (result i32 i32)
    (local $temp (tuple i32 i32))
    ;; As above, but the tuple contains no references, so we do not need to do
    ;; anything for the br_if.
    (block $out (result i32 i32)
      (local.set $temp
        (br_if $out
          (tuple.make 2
            (i32.const -1)
            (i32.const 3)
          )
          (local.get $x)
        )
      )
      (unreachable)
    )
  )

  ;; CHECK:      (func $test-local-tuple-3 (type $5) (param $B (ref $B)) (param $x i32) (result anyref i32)
  ;; CHECK-NEXT:  (local $temp (ref $B))
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 (tuple (ref $B) i32))
  ;; CHECK-NEXT:  (local $5 (ref $B))
  ;; CHECK-NEXT:  (local $6 (tuple (ref $B) i32))
  ;; CHECK-NEXT:  (local.set $6
  ;; CHECK-NEXT:   (block $label$1 (type $6) (result (ref $B) i32)
  ;; CHECK-NEXT:    (local.set $4
  ;; CHECK-NEXT:     (br_if $label$1
  ;; CHECK-NEXT:      (tuple.make 2
  ;; CHECK-NEXT:       (local.get $B)
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $temp
  ;; CHECK-NEXT:     (block (result (ref $B))
  ;; CHECK-NEXT:      (local.set $5
  ;; CHECK-NEXT:       (tuple.extract 2 0
  ;; CHECK-NEXT:        (local.get $4)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $3
  ;; CHECK-NEXT:       (tuple.extract 2 1
  ;; CHECK-NEXT:        (local.get $4)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $5)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (tuple.make 2
  ;; CHECK-NEXT:   (tuple.extract 2 0
  ;; CHECK-NEXT:    (local.get $6)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (tuple.extract 2 1
  ;; CHECK-NEXT:    (local.get $6)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-local-tuple-3 (param $B (ref $B)) (param $x i32) (result anyref i32)
    (local $temp (tuple (ref $B) i32))
    ;; As above, but it is not dropped. However, it has the right type, there is
    ;; no refining, so we do not need to do anything for the br_if.
    (block $out (result (ref $B) i32)
      (local.set $temp
        (br_if $out
          (tuple.make 2
            (local.get $B)
            (i32.const 3)
          )
          (local.get $x)
        )
      )
      (unreachable)
    )
  )

  ;; CHECK:      (func $test-local-tuple-4-bad (type $5) (param $B (ref $B)) (param $x i32) (result anyref i32)
  ;; CHECK-NEXT:  (local $temp (ref $B))
  ;; CHECK-NEXT:  (local $3 (ref $B))
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (local $6 (tuple (ref $B) i32))
  ;; CHECK-NEXT:  (local $7 (ref $B))
  ;; CHECK-NEXT:  (local $8 (ref $B))
  ;; CHECK-NEXT:  (local $9 (tuple (ref $A) i32))
  ;; CHECK-NEXT:  (local.set $9
  ;; CHECK-NEXT:   (block $label$1 (type $3) (result (ref $A) i32)
  ;; CHECK-NEXT:    (local.set $6
  ;; CHECK-NEXT:     (br_if $label$1
  ;; CHECK-NEXT:      (tuple.make 2
  ;; CHECK-NEXT:       (local.get $B)
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $3
  ;; CHECK-NEXT:     (block (result (ref $B))
  ;; CHECK-NEXT:      (local.set $7
  ;; CHECK-NEXT:       (tuple.extract 2 0
  ;; CHECK-NEXT:        (local.get $6)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $5
  ;; CHECK-NEXT:       (tuple.extract 2 1
  ;; CHECK-NEXT:        (local.get $6)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $7)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $temp
  ;; CHECK-NEXT:     (block (result (ref $B))
  ;; CHECK-NEXT:      (local.set $8
  ;; CHECK-NEXT:       (ref.cast (ref $B)
  ;; CHECK-NEXT:        (local.get $3)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.set $4
  ;; CHECK-NEXT:       (local.get $5)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $8)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (tuple.make 2
  ;; CHECK-NEXT:   (tuple.extract 2 0
  ;; CHECK-NEXT:    (local.get $9)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (tuple.extract 2 1
  ;; CHECK-NEXT:    (local.get $9)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-local-tuple-4-bad (param $B (ref $B)) (param $x i32) (result anyref i32)
    (local $temp (tuple (ref $B) i32))
    ;; As above, but none of the mitigating circumstances happens: we have a
    ;; tuple with a reference that is refined compared to the break target. As a
    ;; result we must fix this up, which we do by adding locals, saving the
    ;; br_if's output to them, and then loading from those locals and casting.
    ;;
    ;; Comparing to $test-local-tuple-4, we end up with 3 more locals, and also
    ;; there is now a ref.cast.
    (block $out (result (ref $A) i32)
      (local.set $temp
        (br_if $out
          (tuple.make 2
            (local.get $B)
            (i32.const 3)
          )
          (local.get $x)
        )
      )
      (unreachable)
    )
  )
)
