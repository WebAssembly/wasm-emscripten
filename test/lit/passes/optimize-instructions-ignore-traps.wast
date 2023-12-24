;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --all-features --optimize-instructions --optimize-level=2 --ignore-implicit-traps -S -o - \
;; RUN:   | filecheck %s

(module
  ;; CHECK:      (type $0 (func (param i32 i32) (result i32)))
  (type $0 (func (param i32 i32) (result i32)))
  ;; CHECK:      (import "a" "b" (func $get-i32 (type $2) (result i32)))

  ;; CHECK:      (memory $0 0)
  (memory $0 0)

  (import "a" "b" (func $get-i32 (result i32)))

  ;; CHECK:      (func $conditionals (type $0) (param $0 i32) (param $1 i32) (result i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (local $6 i32)
  ;; CHECK-NEXT:  (local $7 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (loop $while-in
  ;; CHECK-NEXT:   (local.set $3
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (loop $while-in6
  ;; CHECK-NEXT:    (local.set $6
  ;; CHECK-NEXT:     (i32.add
  ;; CHECK-NEXT:      (local.get $0)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (if (result i32)
  ;; CHECK-NEXT:      (i32.or
  ;; CHECK-NEXT:       (i32.eqz
  ;; CHECK-NEXT:        (i32.rem_s
  ;; CHECK-NEXT:         (i32.add
  ;; CHECK-NEXT:          (i32.mul
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:           (local.tee $7
  ;; CHECK-NEXT:            (i32.add
  ;; CHECK-NEXT:             (local.get $0)
  ;; CHECK-NEXT:             (i32.const 2)
  ;; CHECK-NEXT:            )
  ;; CHECK-NEXT:           )
  ;; CHECK-NEXT:          )
  ;; CHECK-NEXT:          (i32.const 17)
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (i32.const 5)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.eqz
  ;; CHECK-NEXT:        (i32.rem_u
  ;; CHECK-NEXT:         (i32.add
  ;; CHECK-NEXT:          (i32.mul
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:          )
  ;; CHECK-NEXT:          (i32.const 11)
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (i32.const 3)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $7)
  ;; CHECK-NEXT:      (local.get $6)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br_if $while-in6
  ;; CHECK-NEXT:     (i32.gt_s
  ;; CHECK-NEXT:      (local.get $4)
  ;; CHECK-NEXT:      (local.tee $3
  ;; CHECK-NEXT:       (i32.add
  ;; CHECK-NEXT:        (local.get $3)
  ;; CHECK-NEXT:        (i32.const 1)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br_if $while-in
  ;; CHECK-NEXT:    (i32.ne
  ;; CHECK-NEXT:     (local.tee $1
  ;; CHECK-NEXT:      (i32.add
  ;; CHECK-NEXT:       (local.get $1)
  ;; CHECK-NEXT:       (i32.const 1)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.const 27000)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $conditionals (type $0) (param $0 i32) (param $1 i32) (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local.set $0
      (i32.const 0)
    )
    (loop $while-in
      (local.set $3
        (i32.const 0)
      )
      (loop $while-in6
        (local.set $6
          (i32.add
            (local.get $0)
            (i32.const 1)
          )
        )
        (local.set $0
          (if (result i32)
            (i32.or ;; this or is very expensive. we should compute one side, then see if we even need the other
              (i32.eqz
                (i32.rem_s
                  (i32.add
                    (i32.mul
                      (local.tee $7 ;; side effect, so we can't do this one
                        (i32.add
                          (local.get $0)
                          (i32.const 2)
                        )
                      )
                      (local.get $0)
                    )
                    (i32.const 17)
                  )
                  (i32.const 5)
                )
              )
              (i32.eqz
                (i32.rem_u
                  (i32.add
                    (i32.mul
                      (local.get $0)
                      (local.get $0)
                    )
                    (i32.const 11)
                  )
                  (i32.const 3)
                )
              )
            )
            (local.get $7)
            (local.get $6)
          )
        )
        (br_if $while-in6
          (i32.lt_s
            (local.tee $3
              (i32.add
                (local.get $3)
                (i32.const 1)
              )
            )
            (local.get $4)
          )
        )
      )
      (br_if $while-in
        (i32.ne
          (local.tee $1
            (i32.add
              (local.get $1)
              (i32.const 1)
            )
          )
          (i32.const 27000)
        )
      )
    )
    (return
      (local.get $5)
    )
  )
  ;; CHECK:      (func $side-effect (type $0) (param $0 i32) (param $1 i32) (result i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (local $6 i32)
  ;; CHECK-NEXT:  (local $7 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (loop $while-in
  ;; CHECK-NEXT:   (local.set $3
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (loop $while-in6
  ;; CHECK-NEXT:    (local.set $6
  ;; CHECK-NEXT:     (i32.add
  ;; CHECK-NEXT:      (local.get $0)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (if (result i32)
  ;; CHECK-NEXT:      (i32.or
  ;; CHECK-NEXT:       (i32.eqz
  ;; CHECK-NEXT:        (i32.rem_s
  ;; CHECK-NEXT:         (i32.add
  ;; CHECK-NEXT:          (i32.mul
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:           (local.tee $7
  ;; CHECK-NEXT:            (local.get $0)
  ;; CHECK-NEXT:           )
  ;; CHECK-NEXT:          )
  ;; CHECK-NEXT:          (i32.const 17)
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (i32.const 5)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.eqz
  ;; CHECK-NEXT:        (i32.rem_u
  ;; CHECK-NEXT:         (i32.add
  ;; CHECK-NEXT:          (i32.mul
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:          )
  ;; CHECK-NEXT:          (unreachable)
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (i32.const 3)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $7)
  ;; CHECK-NEXT:      (local.get $6)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br_if $while-in6
  ;; CHECK-NEXT:     (i32.gt_s
  ;; CHECK-NEXT:      (local.get $4)
  ;; CHECK-NEXT:      (local.tee $3
  ;; CHECK-NEXT:       (i32.add
  ;; CHECK-NEXT:        (local.get $3)
  ;; CHECK-NEXT:        (i32.const 1)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br_if $while-in
  ;; CHECK-NEXT:    (i32.ne
  ;; CHECK-NEXT:     (local.tee $1
  ;; CHECK-NEXT:      (i32.add
  ;; CHECK-NEXT:       (local.get $1)
  ;; CHECK-NEXT:       (i32.const 1)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.const 27000)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $side-effect (type $0) (param $0 i32) (param $1 i32) (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local.set $0
      (i32.const 0)
    )
    (loop $while-in
      (local.set $3
        (i32.const 0)
      )
      (loop $while-in6
        (local.set $6
          (i32.add
            (local.get $0)
            (i32.const 1)
          )
        )
        (local.set $0
          (if (result i32)
            (i32.or ;; this or is very expensive, but has a side effect on both sides
              (i32.eqz
                (i32.rem_s
                  (i32.add
                    (i32.mul
                      (local.tee $7
                        (i32.add
                          (local.get $0)
                          (i32.const 0)
                        )
                      )
                      (local.get $0)
                    )
                    (i32.const 17)
                  )
                  (i32.const 5)
                )
              )
              (i32.eqz
                (i32.rem_u
                  (i32.add
                    (i32.mul
                      (local.get $0)
                      (local.get $0)
                    )
                    (unreachable)
                  )
                  (i32.const 3)
                )
              )
            )
            (local.get $7)
            (local.get $6)
          )
        )
        (br_if $while-in6
          (i32.lt_s
            (local.tee $3
              (i32.add
                (local.get $3)
                (i32.const 1)
              )
            )
            (local.get $4)
          )
        )
      )
      (br_if $while-in
        (i32.ne
          (local.tee $1
            (i32.add
              (local.get $1)
              (i32.const 1)
            )
          )
          (i32.const 27000)
        )
      )
    )
    (return
      (local.get $5)
    )
  )
  ;; CHECK:      (func $flip (type $0) (param $0 i32) (param $1 i32) (result i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (local $5 i32)
  ;; CHECK-NEXT:  (local $6 i32)
  ;; CHECK-NEXT:  (local $7 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (loop $while-in
  ;; CHECK-NEXT:   (local.set $3
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (loop $while-in6
  ;; CHECK-NEXT:    (local.set $6
  ;; CHECK-NEXT:     (i32.add
  ;; CHECK-NEXT:      (local.get $0)
  ;; CHECK-NEXT:      (i32.const 1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (if (result i32)
  ;; CHECK-NEXT:      (if (result i32)
  ;; CHECK-NEXT:       (i32.rem_u
  ;; CHECK-NEXT:        (i32.add
  ;; CHECK-NEXT:         (i32.mul
  ;; CHECK-NEXT:          (local.get $0)
  ;; CHECK-NEXT:          (local.get $0)
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (i32.const 100)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:        (i32.const 3)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.rem_s
  ;; CHECK-NEXT:        (i32.add
  ;; CHECK-NEXT:         (i32.mul
  ;; CHECK-NEXT:          (local.get $0)
  ;; CHECK-NEXT:          (i32.eqz
  ;; CHECK-NEXT:           (local.get $0)
  ;; CHECK-NEXT:          )
  ;; CHECK-NEXT:         )
  ;; CHECK-NEXT:         (i32.const 17)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:        (i32.const 5)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (local.get $6)
  ;; CHECK-NEXT:      (local.get $7)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br_if $while-in6
  ;; CHECK-NEXT:     (i32.gt_s
  ;; CHECK-NEXT:      (local.get $4)
  ;; CHECK-NEXT:      (local.tee $3
  ;; CHECK-NEXT:       (i32.add
  ;; CHECK-NEXT:        (local.get $3)
  ;; CHECK-NEXT:        (i32.const 1)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br_if $while-in
  ;; CHECK-NEXT:    (i32.ne
  ;; CHECK-NEXT:     (local.tee $1
  ;; CHECK-NEXT:      (i32.add
  ;; CHECK-NEXT:       (local.get $1)
  ;; CHECK-NEXT:       (i32.const 1)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.const 27000)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $flip (type $0) (param $0 i32) (param $1 i32) (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local.set $0
      (i32.const 0)
    )
    (loop $while-in
      (local.set $3
        (i32.const 0)
      )
      (loop $while-in6
        (local.set $6
          (i32.add
            (local.get $0)
            (i32.const 1)
          )
        )
        (local.set $0
          (if (result i32)
            (i32.or ;; this or is very expensive, and the first side has no side effect
              (i32.eqz
                (i32.rem_s
                  (i32.add
                    (i32.mul
                      (i32.eqz
                        (i32.add
                          (local.get $0)
                          (i32.const 0)
                        )
                      )
                      (local.get $0)
                    )
                    (i32.const 17)
                  )
                  (i32.const 5)
                )
              )
              (i32.eqz
                (i32.rem_u
                  (i32.add
                    (i32.mul
                      (local.get $0)
                      (local.get $0)
                    )
                    (i32.const 100)
                  )
                  (i32.const 3)
                )
              )
            )
            (local.get $7)
            (local.get $6)
          )
        )
        (br_if $while-in6
          (i32.lt_s
            (local.tee $3
              (i32.add
                (local.get $3)
                (i32.const 1)
              )
            )
            (local.get $4)
          )
        )
      )
      (br_if $while-in
        (i32.ne
          (local.tee $1
            (i32.add
              (local.get $1)
              (i32.const 1)
            )
          )
          (i32.const 27000)
        )
      )
    )
    (return
      (local.get $5)
    )
  )
  ;; CHECK:      (func $invalidate-conditionalizeExpensiveOnBitwise (type $0) (param $0 i32) (param $1 i32) (result i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.eqz
  ;; CHECK-NEXT:    (i32.and
  ;; CHECK-NEXT:     (i32.lt_u
  ;; CHECK-NEXT:      (i32.and
  ;; CHECK-NEXT:       (i32.extend8_s
  ;; CHECK-NEXT:        (i32.sub
  ;; CHECK-NEXT:         (local.get $1)
  ;; CHECK-NEXT:         (i32.const 1)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.const 255)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (i32.const 3)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.ne
  ;; CHECK-NEXT:      (local.tee $1
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (return
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $invalidate-conditionalizeExpensiveOnBitwise (param $0 i32) (param $1 i32) (result i32)
   (if
    (i32.eqz
     (i32.and
      (i32.lt_s
       (i32.and
        (i32.shr_s
         (i32.shl
          (i32.add
           (local.get $1) ;; conflict with tee
           (i32.const -1)
          )
          (i32.const 24)
         )
         (i32.const 24)
        )
        (i32.const 255)
       )
       (i32.const 3)
      )
      (i32.ne
       (local.tee $1
        (i32.const 0)
       )
       (i32.const 0)
      )
     )
    )
    (return (local.get $0))
   )
   (return (local.get $1))
  )
  ;; CHECK:      (func $invalidate-conditionalizeExpensiveOnBitwise-ok (type $0) (param $0 i32) (param $1 i32) (result i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.eqz
  ;; CHECK-NEXT:    (if (result i32)
  ;; CHECK-NEXT:     (local.tee $1
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.lt_u
  ;; CHECK-NEXT:      (i32.and
  ;; CHECK-NEXT:       (i32.extend8_s
  ;; CHECK-NEXT:        (i32.sub
  ;; CHECK-NEXT:         (local.get $0)
  ;; CHECK-NEXT:         (i32.const 1)
  ;; CHECK-NEXT:        )
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:       (i32.const 255)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (i32.const 3)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.const 0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (return
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $invalidate-conditionalizeExpensiveOnBitwise-ok (param $0 i32) (param $1 i32) (result i32)
   (if
    (i32.eqz
     (i32.and
      (i32.lt_s
       (i32.and
        (i32.shr_s
         (i32.shl
          (i32.add
           (local.get $0) ;; no conflict
           (i32.const -1)
          )
          (i32.const 24)
         )
         (i32.const 24)
        )
        (i32.const 255)
       )
       (i32.const 3)
      )
      (i32.ne
       (local.tee $1
        (i32.const 0)
       )
       (i32.const 0)
      )
     )
    )
    (return (local.get $0))
   )
   (return (local.get $1))
  )

 ;; CHECK:      (func $conditionalize-if-type-change (type $3) (result f64)
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (loop $label$1 (result f32)
 ;; CHECK-NEXT:    (block $label$2 (result f32)
 ;; CHECK-NEXT:     (drop
 ;; CHECK-NEXT:      (block $label$3 (result f32)
 ;; CHECK-NEXT:       (br_if $label$1
 ;; CHECK-NEXT:        (i32.or
 ;; CHECK-NEXT:         (f32.gt
 ;; CHECK-NEXT:          (br_if $label$3
 ;; CHECK-NEXT:           (f32.const 1)
 ;; CHECK-NEXT:           (local.get $0)
 ;; CHECK-NEXT:          )
 ;; CHECK-NEXT:          (br $label$2
 ;; CHECK-NEXT:           (f32.const 71)
 ;; CHECK-NEXT:          )
 ;; CHECK-NEXT:         )
 ;; CHECK-NEXT:         (i64.eqz
 ;; CHECK-NEXT:          (select
 ;; CHECK-NEXT:           (i64.const 58)
 ;; CHECK-NEXT:           (i64.const -982757)
 ;; CHECK-NEXT:           (i64.eqz
 ;; CHECK-NEXT:            (i64.const 0)
 ;; CHECK-NEXT:           )
 ;; CHECK-NEXT:          )
 ;; CHECK-NEXT:         )
 ;; CHECK-NEXT:        )
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (f32.const 1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (f64.const -nan:0xfffffffffffff)
 ;; CHECK-NEXT: )
 (func $conditionalize-if-type-change (result f64)
  (local $0 i32)
  (drop
   (loop $label$1 (result f32)
    (block $label$2 (result f32)
     (drop
      (block $label$3 (result f32)
       (br_if $label$1
        (i32.or ;; this turns into an if, but then the if might not be unreachable
         (f32.gt
          (br_if $label$3
           (f32.const 1)
           (local.get $0)
          )
          (br $label$2
           (f32.const 71)
          )
         )
         (i64.eqz
          (select
           (i64.const 58)
           (i64.const -982757)
           (i64.eqz
            (i64.const 0)
           )
          )
         )
        )
       )
      )
     )
     (f32.const 1)
    )
   )
  )
  (f64.const -nan:0xfffffffffffff)
 )
 ;; CHECK:      (func $optimize-bulk-memory-copy (type $1) (param $dst i32) (param $src i32) (param $sz i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $dst)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $dst)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $sz)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $dst)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $src)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (memory.copy
 ;; CHECK-NEXT:   (call $get-i32)
 ;; CHECK-NEXT:   (call $get-i32)
 ;; CHECK-NEXT:   (local.get $sz)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (call $get-i32)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (call $get-i32)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $optimize-bulk-memory-copy (param $dst i32) (param $src i32) (param $sz i32)
  (memory.copy  ;; nop
    (local.get $dst)
    (local.get $dst)
    (local.get $sz)
  )

  (memory.copy  ;; nop
    (local.get $dst)
    (local.get $src)
    (i32.const 0)
  )
  (memory.copy  ;; not a nop as the runtime dst/src may differ
    (call $get-i32)
    (call $get-i32)
    (local.get $sz)
  )
  (memory.copy  ;; as above, but with a size of 0. the calls must remain.
    (call $get-i32)
    (call $get-i32)
    (i32.const 0)
  )
 )

 ;; CHECK:      (func $optimize-bulk-memory-fill (type $1) (param $dst i32) (param $val i32) (param $sz i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $dst)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $dst)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $val)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (memory.fill
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:   (local.get $sz)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $optimize-bulk-memory-fill (param $dst i32) (param $val i32) (param $sz i32)
  (memory.fill ;; drops
    (local.get $dst)
    (i32.const 0)
    (i32.const 0)
  )

  (memory.fill ;; drops
    (local.get $dst)
    (local.get $val)
    (i32.const 0)
  )

  (memory.fill ;; skip
    (i32.const 0)
    (i32.const 0)
    (local.get $sz)
  )
 )
)
