;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; As in monomorphize-types.wast, test in both "always" mode, which always
;; monomorphizes, and in "careful" mode which does it only when it appears to
;; actually help.

;; RUN: foreach %s %t wasm-opt --monomorphize-always -all -S -o - | filecheck %s --check-prefix ALWAYS
;; RUN: foreach %s %t wasm-opt --monomorphize        -all -S -o - | filecheck %s --check-prefix CAREFUL












;; TODO: nesting inside, children that are some in and some out
























(module
  ;; ALWAYS:      (type $0 (func (param i32) (result i32)))

  ;; ALWAYS:      (type $1 (func (param i32 i32 i32 i32 i32 i32 i32 i32 i32 anyref funcref i32 f64 i32 anyref anyref i32)))

  ;; ALWAYS:      (type $2 (func (param i32 i32 i32 i32 i32 i32)))

  ;; ALWAYS:      (type $struct (struct ))
  (type $struct (struct))

  (memory 10 20)

  ;; ALWAYS:      (global $imm i32 (i32.const 10))
  ;; CAREFUL:      (type $0 (func (param i32) (result i32)))

  ;; CAREFUL:      (type $1 (func (param i32 i32 i32 i32 i32 i32 i32 i32 i32 anyref funcref i32 f64 i32 anyref anyref i32)))

  ;; CAREFUL:      (type $2 (func (param i32 i32 i32 i32 i32 i32)))

  ;; CAREFUL:      (global $imm i32 (i32.const 10))
  (global $imm i32 (i32.const 10))

  ;; ALWAYS:      (global $mut (mut i32) (i32.const 20))
  ;; CAREFUL:      (global $mut (mut i32) (i32.const 20))
  (global $mut (mut i32) (i32.const 20))

  ;; ALWAYS:      (memory $0 10 20)

  ;; ALWAYS:      (elem declare func $target)

  ;; ALWAYS:      (func $caller (type $0) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (block $out (result i32)
  ;; ALWAYS-NEXT:   (call $target_2
  ;; ALWAYS-NEXT:    (block (result i32)
  ;; ALWAYS-NEXT:     (i32.const 0)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (if (result i32)
  ;; ALWAYS-NEXT:     (i32.const 1)
  ;; ALWAYS-NEXT:     (then
  ;; ALWAYS-NEXT:      (i32.const 2)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:     (else
  ;; ALWAYS-NEXT:      (i32.const 3)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (call $caller
  ;; ALWAYS-NEXT:     (i32.const 4)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (local.tee $x
  ;; ALWAYS-NEXT:     (i32.const 5)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (br_if $out
  ;; ALWAYS-NEXT:     (i32.const 12)
  ;; ALWAYS-NEXT:     (i32.const 13)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:   (i32.const 14)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (memory $0 10 20)

  ;; CAREFUL:      (func $caller (type $0) (param $x i32) (result i32)
  ;; CAREFUL-NEXT:  (block $out (result i32)
  ;; CAREFUL-NEXT:   (call $target_2
  ;; CAREFUL-NEXT:    (block (result i32)
  ;; CAREFUL-NEXT:     (i32.const 0)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (if (result i32)
  ;; CAREFUL-NEXT:     (i32.const 1)
  ;; CAREFUL-NEXT:     (then
  ;; CAREFUL-NEXT:      (i32.const 2)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:     (else
  ;; CAREFUL-NEXT:      (i32.const 3)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (call $caller
  ;; CAREFUL-NEXT:     (i32.const 4)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (local.get $x)
  ;; CAREFUL-NEXT:    (local.tee $x
  ;; CAREFUL-NEXT:     (i32.const 5)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (br_if $out
  ;; CAREFUL-NEXT:     (i32.const 12)
  ;; CAREFUL-NEXT:     (i32.const 13)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:   (i32.const 14)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $caller (param $x i32) (result i32)
    ;; Show the variety of things we can and cannot move into the call context.
    ;;
    ;; Note that in CAREFUL mode we only optimize here if we properly take into
    ;; account the call context in the cost. The function we are calling has
    ;; an empty body, so the monomorphized function will contain basically just
    ;; the moved code from the call context. If we didn't measure that in the
    ;; cost before monomorphization then it would seem like we went from cost 0
    ;; (empty body) to the cost of the operations that remain after we
    ;; optimize (which is the i32.load, which might trap so it remains). But if
    ;; we take into account the context, then monomorphization definitely helps
    ;; as it removes a bunch of constants.
    (block $out (result i32)
      (call $target
        ;; We can't move control flow.
        (block (result i32)
          (i32.const 0)
        )
        (if (result i32)
          (i32.const 1)
          (then
            (i32.const 2)
          )
          (else
            (i32.const 3)
          )
        )
        ;; We don't move calls.
        (call $caller
          (i32.const 4)
        )
        ;; We can't move local operations.
        (local.get $x)
        (local.tee $x
          (i32.const 5)
        )
        ;; We can move globals, even mutable.
        (global.get $imm)
        (global.get $mut)
        ;; We can move loads and other options that might trap.
        (i32.load
          (i32.const 6)
        )
        ;; We can move constants.
        (i32.const 7)
        (ref.null any)
        (ref.func $target)
        ;; We can move math operations.
        (i32.eqz
          (i32.const 8)
        )
        (f64.add
          (f64.const 2.71828)
          (f64.const 3.14159)
        )
        ;; We can move selects.
        (select
          (i32.const 9)
          (i32.const 10)
          (i32.const 11)
        )
        ;; We can move GC operations.
        (ref.cast (ref null none)
          (ref.null none)
        )
        (struct.new $struct)
        ;; We can't move control flow.
        (br_if $out
          (i32.const 12)
          (i32.const 13)
        )
      )
      (i32.const 14)
    )
  )

  ;; ALWAYS:      (func $target (type $1) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32) (param $6 i32) (param $7 i32) (param $8 i32) (param $9 anyref) (param $10 funcref) (param $11 i32) (param $12 f64) (param $13 i32) (param $14 anyref) (param $15 anyref) (param $16 i32)
  ;; ALWAYS-NEXT:  (nop)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $target (type $1) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32) (param $6 i32) (param $7 i32) (param $8 i32) (param $9 anyref) (param $10 funcref) (param $11 i32) (param $12 f64) (param $13 i32) (param $14 anyref) (param $15 anyref) (param $16 i32)
  ;; CAREFUL-NEXT:  (nop)
  ;; CAREFUL-NEXT: )
  (func $target
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param anyref)
    (param funcref)
    (param i32)
    (param f64)
    (param i32)
    (param anyref)
    (param anyref)
    (param i32)
  )
)


;; TODO: nesting inside, children that are some in and some out



;; ALWAYS:      (func $target_2 (type $2) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32)
;; ALWAYS-NEXT:  (local $6 i32)
;; ALWAYS-NEXT:  (local $7 i32)
;; ALWAYS-NEXT:  (local $8 i32)
;; ALWAYS-NEXT:  (local $9 i32)
;; ALWAYS-NEXT:  (local $10 i32)
;; ALWAYS-NEXT:  (local $11 i32)
;; ALWAYS-NEXT:  (local $12 i32)
;; ALWAYS-NEXT:  (local $13 i32)
;; ALWAYS-NEXT:  (local $14 i32)
;; ALWAYS-NEXT:  (local $15 anyref)
;; ALWAYS-NEXT:  (local $16 funcref)
;; ALWAYS-NEXT:  (local $17 i32)
;; ALWAYS-NEXT:  (local $18 f64)
;; ALWAYS-NEXT:  (local $19 i32)
;; ALWAYS-NEXT:  (local $20 anyref)
;; ALWAYS-NEXT:  (local $21 anyref)
;; ALWAYS-NEXT:  (local $22 i32)
;; ALWAYS-NEXT:  (local.set $6
;; ALWAYS-NEXT:   (local.get $0)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $7
;; ALWAYS-NEXT:   (local.get $1)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $8
;; ALWAYS-NEXT:   (local.get $2)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $9
;; ALWAYS-NEXT:   (local.get $3)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $10
;; ALWAYS-NEXT:   (local.get $4)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $11
;; ALWAYS-NEXT:   (global.get $imm)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $12
;; ALWAYS-NEXT:   (global.get $mut)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $13
;; ALWAYS-NEXT:   (i32.load
;; ALWAYS-NEXT:    (i32.const 6)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $14
;; ALWAYS-NEXT:   (i32.const 7)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $15
;; ALWAYS-NEXT:   (ref.null none)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $16
;; ALWAYS-NEXT:   (ref.func $target)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $17
;; ALWAYS-NEXT:   (i32.eqz
;; ALWAYS-NEXT:    (i32.const 8)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $18
;; ALWAYS-NEXT:   (f64.add
;; ALWAYS-NEXT:    (f64.const 2.71828)
;; ALWAYS-NEXT:    (f64.const 3.14159)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $19
;; ALWAYS-NEXT:   (select
;; ALWAYS-NEXT:    (i32.const 9)
;; ALWAYS-NEXT:    (i32.const 10)
;; ALWAYS-NEXT:    (i32.const 11)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $20
;; ALWAYS-NEXT:   (ref.cast nullref
;; ALWAYS-NEXT:    (ref.null none)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $21
;; ALWAYS-NEXT:   (struct.new_default $struct)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (local.set $22
;; ALWAYS-NEXT:   (local.get $5)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (nop)
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $target_2 (type $2) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32)
;; CAREFUL-NEXT:  (drop
;; CAREFUL-NEXT:   (i32.load
;; CAREFUL-NEXT:    (i32.const 6)
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )
(module
  ;; ALWAYS:      (type $0 (func (param i32) (result i32)))

  ;; ALWAYS:      (type $1 (func (param i32 i32 i32 i32 i32 i32 i32 i32 i32 anyref funcref i32 f64 i32 anyref anyref i32) (result i32)))

  ;; ALWAYS:      (type $2 (func (param i32 i32 i32 i32 i32 i32)))

  ;; ALWAYS:      (type $struct (struct ))
  (type $struct (struct))

  (memory 10 20)

  ;; ALWAYS:      (global $imm i32 (i32.const 10))
  ;; CAREFUL:      (type $0 (func (param i32) (result i32)))

  ;; CAREFUL:      (type $1 (func (param i32 i32 i32 i32 i32 i32 i32 i32 i32 anyref funcref i32 f64 i32 anyref anyref i32) (result i32)))

  ;; CAREFUL:      (type $2 (func (param i32 i32 i32 i32 i32 i32)))

  ;; CAREFUL:      (global $imm i32 (i32.const 10))
  (global $imm i32 (i32.const 10))

  ;; ALWAYS:      (global $mut (mut i32) (i32.const 20))
  ;; CAREFUL:      (global $mut (mut i32) (i32.const 20))
  (global $mut (mut i32) (i32.const 20))

  ;; ALWAYS:      (memory $0 10 20)

  ;; ALWAYS:      (elem declare func $target)

  ;; ALWAYS:      (func $caller (type $0) (param $x i32) (result i32)
  ;; ALWAYS-NEXT:  (block $out (result i32)
  ;; ALWAYS-NEXT:   (call $target_2
  ;; ALWAYS-NEXT:    (block (result i32)
  ;; ALWAYS-NEXT:     (i32.const 0)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (if (result i32)
  ;; ALWAYS-NEXT:     (i32.const 1)
  ;; ALWAYS-NEXT:     (then
  ;; ALWAYS-NEXT:      (i32.const 2)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:     (else
  ;; ALWAYS-NEXT:      (i32.const 3)
  ;; ALWAYS-NEXT:     )
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (call $caller
  ;; ALWAYS-NEXT:     (i32.const 4)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (local.get $x)
  ;; ALWAYS-NEXT:    (local.tee $x
  ;; ALWAYS-NEXT:     (i32.const 5)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:    (br_if $out
  ;; ALWAYS-NEXT:     (i32.const 12)
  ;; ALWAYS-NEXT:     (i32.const 13)
  ;; ALWAYS-NEXT:    )
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:   (i32.const 14)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (memory $0 10 20)

  ;; CAREFUL:      (func $caller (type $0) (param $x i32) (result i32)
  ;; CAREFUL-NEXT:  (block $out (result i32)
  ;; CAREFUL-NEXT:   (call $target_2
  ;; CAREFUL-NEXT:    (block (result i32)
  ;; CAREFUL-NEXT:     (i32.const 0)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (if (result i32)
  ;; CAREFUL-NEXT:     (i32.const 1)
  ;; CAREFUL-NEXT:     (then
  ;; CAREFUL-NEXT:      (i32.const 2)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:     (else
  ;; CAREFUL-NEXT:      (i32.const 3)
  ;; CAREFUL-NEXT:     )
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (call $caller
  ;; CAREFUL-NEXT:     (i32.const 4)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (local.get $x)
  ;; CAREFUL-NEXT:    (local.tee $x
  ;; CAREFUL-NEXT:     (i32.const 5)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:    (br_if $out
  ;; CAREFUL-NEXT:     (i32.const 12)
  ;; CAREFUL-NEXT:     (i32.const 13)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:   (i32.const 14)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $caller (param $x i32) (result i32)
    ;; As above, but now the call is dropped, so the context can include the
    ;; drop.
    (block $out (result i32)
      (drop
        (call $target
          (block (result i32)
            (i32.const 0)
          )
          (if (result i32)
            (i32.const 1)
            (then
              (i32.const 2)
            )
            (else
              (i32.const 3)
            )
          )
          (call $caller
            (i32.const 4)
          )
          (local.get $x)
          (local.tee $x
            (i32.const 5)
          )
          (global.get $imm)
          (global.get $mut)
          (i32.load
            (i32.const 6)
          )
          (i32.const 7)
          (ref.null any)
          (ref.func $target)
          (i32.eqz
            (i32.const 8)
          )
          (f64.add
            (f64.const 2.71828)
            (f64.const 3.14159)
          )
          (select
            (i32.const 9)
            (i32.const 10)
            (i32.const 11)
          )
          (ref.cast (ref null none)
            (ref.null none)
          )
          (struct.new $struct)
          (br_if $out
            (i32.const 12)
            (i32.const 13)
          )
        )
      )
      (i32.const 14)
    )
  )

  ;; ALWAYS:      (func $target (type $1) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32) (param $6 i32) (param $7 i32) (param $8 i32) (param $9 anyref) (param $10 funcref) (param $11 i32) (param $12 f64) (param $13 i32) (param $14 anyref) (param $15 anyref) (param $16 i32) (result i32)
  ;; ALWAYS-NEXT:  (local.get $7)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $target (type $1) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32) (param $6 i32) (param $7 i32) (param $8 i32) (param $9 anyref) (param $10 funcref) (param $11 i32) (param $12 f64) (param $13 i32) (param $14 anyref) (param $15 anyref) (param $16 i32) (result i32)
  ;; CAREFUL-NEXT:  (local.get $7)
  ;; CAREFUL-NEXT: )
  (func $target
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param i32)
    (param anyref)
    (param funcref)
    (param i32)
    (param f64)
    (param i32)
    (param anyref)
    (param anyref)
    (param i32)
    (result i32)
    (local.get 7)
  )
)




;; ALWAYS:      (func $target_2 (type $2) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32)
;; ALWAYS-NEXT:  (local $6 i32)
;; ALWAYS-NEXT:  (local $7 i32)
;; ALWAYS-NEXT:  (local $8 i32)
;; ALWAYS-NEXT:  (local $9 i32)
;; ALWAYS-NEXT:  (local $10 i32)
;; ALWAYS-NEXT:  (local $11 i32)
;; ALWAYS-NEXT:  (local $12 i32)
;; ALWAYS-NEXT:  (local $13 i32)
;; ALWAYS-NEXT:  (local $14 i32)
;; ALWAYS-NEXT:  (local $15 anyref)
;; ALWAYS-NEXT:  (local $16 funcref)
;; ALWAYS-NEXT:  (local $17 i32)
;; ALWAYS-NEXT:  (local $18 f64)
;; ALWAYS-NEXT:  (local $19 i32)
;; ALWAYS-NEXT:  (local $20 anyref)
;; ALWAYS-NEXT:  (local $21 anyref)
;; ALWAYS-NEXT:  (local $22 i32)
;; ALWAYS-NEXT:  (drop
;; ALWAYS-NEXT:   (block (result i32)
;; ALWAYS-NEXT:    (local.set $6
;; ALWAYS-NEXT:     (local.get $0)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $7
;; ALWAYS-NEXT:     (local.get $1)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $8
;; ALWAYS-NEXT:     (local.get $2)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $9
;; ALWAYS-NEXT:     (local.get $3)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $10
;; ALWAYS-NEXT:     (local.get $4)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $11
;; ALWAYS-NEXT:     (global.get $imm)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $12
;; ALWAYS-NEXT:     (global.get $mut)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $13
;; ALWAYS-NEXT:     (i32.load
;; ALWAYS-NEXT:      (i32.const 6)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $14
;; ALWAYS-NEXT:     (i32.const 7)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $15
;; ALWAYS-NEXT:     (ref.null none)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $16
;; ALWAYS-NEXT:     (ref.func $target)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $17
;; ALWAYS-NEXT:     (i32.eqz
;; ALWAYS-NEXT:      (i32.const 8)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $18
;; ALWAYS-NEXT:     (f64.add
;; ALWAYS-NEXT:      (f64.const 2.71828)
;; ALWAYS-NEXT:      (f64.const 3.14159)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $19
;; ALWAYS-NEXT:     (select
;; ALWAYS-NEXT:      (i32.const 9)
;; ALWAYS-NEXT:      (i32.const 10)
;; ALWAYS-NEXT:      (i32.const 11)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $20
;; ALWAYS-NEXT:     (ref.cast nullref
;; ALWAYS-NEXT:      (ref.null none)
;; ALWAYS-NEXT:     )
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $21
;; ALWAYS-NEXT:     (struct.new_default $struct)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.set $22
;; ALWAYS-NEXT:     (local.get $5)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:    (local.get $13)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $target_2 (type $2) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32)
;; CAREFUL-NEXT:  (drop
;; CAREFUL-NEXT:   (i32.load
;; CAREFUL-NEXT:    (i32.const 6)
;; CAREFUL-NEXT:   )
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )
(module
  (memory 10 20)

  ;; ALWAYS:      (type $0 (func))

  ;; ALWAYS:      (type $1 (func (param f32)))

  ;; ALWAYS:      (type $2 (func (param f64)))

  ;; ALWAYS:      (memory $0 10 20)

  ;; ALWAYS:      (func $caller (type $0)
  ;; ALWAYS-NEXT:  (call $target_2
  ;; ALWAYS-NEXT:   (block $label$1 (result f64)
  ;; ALWAYS-NEXT:    (f64.const 0)
  ;; ALWAYS-NEXT:   )
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT:  (call $target_3)
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (type $0 (func))

  ;; CAREFUL:      (type $1 (func (param f32)))

  ;; CAREFUL:      (memory $0 10 20)

  ;; CAREFUL:      (func $caller (type $0)
  ;; CAREFUL-NEXT:  (call $target
  ;; CAREFUL-NEXT:   (f32.demote_f64
  ;; CAREFUL-NEXT:    (block $label$1 (result f64)
  ;; CAREFUL-NEXT:     (f64.const 0)
  ;; CAREFUL-NEXT:    )
  ;; CAREFUL-NEXT:   )
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT:  (call $target_2)
  ;; CAREFUL-NEXT: )
  (func $caller
    ;; Nesting: the f32.demote_f64 operation can be moved into the context, but
    ;; its child cannot, so we stop there. (In CAREFUL mode, we end up doing
    ;; nothing here, as the benefit of monomorphization is not worth it.)
    (call $target
      (f32.demote_f64
        (block $label$1 (result f64)
          (f64.const 0)
        )
      )
    )

    ;; Now the child is an f64.abs, which can be moved into the context, so it
    ;; all is moved. This ends up worthwhile in CAREFUL mode (since we can
    ;; optimize all the math here).
    (call $target
      (f32.demote_f64
        (f64.abs         ;; this changed
          (f64.const 0)
        )
      )
    )
  )

  ;; ALWAYS:      (func $target (type $1) (param $f32 f32)
  ;; ALWAYS-NEXT:  (f32.store
  ;; ALWAYS-NEXT:   (i32.const 42)
  ;; ALWAYS-NEXT:   (local.get $f32)
  ;; ALWAYS-NEXT:  )
  ;; ALWAYS-NEXT: )
  ;; CAREFUL:      (func $target (type $1) (param $0 f32)
  ;; CAREFUL-NEXT:  (f32.store
  ;; CAREFUL-NEXT:   (i32.const 42)
  ;; CAREFUL-NEXT:   (local.get $0)
  ;; CAREFUL-NEXT:  )
  ;; CAREFUL-NEXT: )
  (func $target (param $f32 f32)
    ;; When monomorphized the first time, the param here will be f64 and not
    ;; i32, showing we handle a type change.
    ;;
    ;; When monomorphized the second time, the param will go away entirely.
    (f32.store
      (i32.const 42)
      (local.get $f32)
    )
  )
)

;; ALWAYS:      (func $target_2 (type $2) (param $0 f64)
;; ALWAYS-NEXT:  (local $f32 f32)
;; ALWAYS-NEXT:  (local.set $f32
;; ALWAYS-NEXT:   (f32.demote_f64
;; ALWAYS-NEXT:    (local.get $0)
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (f32.store
;; ALWAYS-NEXT:   (i32.const 42)
;; ALWAYS-NEXT:   (local.get $f32)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; ALWAYS:      (func $target_3 (type $0)
;; ALWAYS-NEXT:  (local $f32 f32)
;; ALWAYS-NEXT:  (local.set $f32
;; ALWAYS-NEXT:   (f32.demote_f64
;; ALWAYS-NEXT:    (f64.abs
;; ALWAYS-NEXT:     (f64.const 0)
;; ALWAYS-NEXT:    )
;; ALWAYS-NEXT:   )
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT:  (f32.store
;; ALWAYS-NEXT:   (i32.const 42)
;; ALWAYS-NEXT:   (local.get $f32)
;; ALWAYS-NEXT:  )
;; ALWAYS-NEXT: )

;; CAREFUL:      (func $target_2 (type $0)
;; CAREFUL-NEXT:  (f32.store
;; CAREFUL-NEXT:   (i32.const 42)
;; CAREFUL-NEXT:   (f32.const 0)
;; CAREFUL-NEXT:  )
;; CAREFUL-NEXT: )
