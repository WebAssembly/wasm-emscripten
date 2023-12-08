;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s -all -o %t.text.wast -g -S
;; RUN: wasm-as %s -all -g -o %t.wasm
;; RUN: wasm-dis %t.wasm -all -o %t.bin.wast
;; RUN: wasm-as %s -all -o %t.nodebug.wasm
;; RUN: wasm-dis %t.nodebug.wasm -all -o %t.bin.nodebug.wast
;; RUN: cat %t.text.wast | filecheck %s --check-prefix=CHECK-TEXT
;; RUN: cat %t.bin.wast | filecheck %s --check-prefix=CHECK-BIN
;; RUN: cat %t.bin.nodebug.wast | filecheck %s --check-prefix=CHECK-BIN-NODEBUG

(module
  ;; CHECK-TEXT:      (type $0 (func (result i32)))

  ;; CHECK-TEXT:      (type $FUNCSIG$ii (func (param i32) (result i32)))
  ;; CHECK-BIN:      (type $0 (func (result i32)))

  ;; CHECK-BIN:      (type $FUNCSIG$ii (func (param i32) (result i32)))
  (type $FUNCSIG$ii (func (param i32) (result i32)))
  ;; CHECK-TEXT:      (type $2 (func))

  ;; CHECK-TEXT:      (type $3 (func (param i32)))

  ;; CHECK-TEXT:      (import "env" "table" (table $timport$0 9 9 funcref))
  ;; CHECK-BIN:      (type $2 (func))

  ;; CHECK-BIN:      (type $3 (func (param i32)))

  ;; CHECK-BIN:      (import "env" "table" (table $timport$0 9 9 funcref))
  ;; CHECK-BIN-NODEBUG:      (type $0 (func (result i32)))

  ;; CHECK-BIN-NODEBUG:      (type $1 (func (param i32) (result i32)))

  ;; CHECK-BIN-NODEBUG:      (type $2 (func))

  ;; CHECK-BIN-NODEBUG:      (type $3 (func (param i32)))

  ;; CHECK-BIN-NODEBUG:      (import "env" "table" (table $timport$0 9 9 funcref))
  (import "env" "table" (table 9 9 funcref))

  ;; CHECK-TEXT:      (func $break-and-binary (type $0) (result i32)
  ;; CHECK-TEXT-NEXT:  (block $x (result i32)
  ;; CHECK-TEXT-NEXT:   (f32.add
  ;; CHECK-TEXT-NEXT:    (br_if $x
  ;; CHECK-TEXT-NEXT:     (i32.trunc_f64_u
  ;; CHECK-TEXT-NEXT:      (unreachable)
  ;; CHECK-TEXT-NEXT:     )
  ;; CHECK-TEXT-NEXT:     (i32.trunc_f64_u
  ;; CHECK-TEXT-NEXT:      (unreachable)
  ;; CHECK-TEXT-NEXT:     )
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:    (f32.const 1)
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $break-and-binary (type $0) (result i32)
  ;; CHECK-BIN-NEXT:  (block $label$1 (result i32)
  ;; CHECK-BIN-NEXT:   (unreachable)
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT: )
  (func $break-and-binary (result i32)
    (block $x (result i32)
      (f32.add
        (br_if $x
          (i32.trunc_f64_u
            (unreachable)
          )
          (i32.trunc_f64_u
            (unreachable)
          )
        )
        (f32.const 1)
      )
    )
  )

  ;; CHECK-TEXT:      (func $call-and-unary (type $FUNCSIG$ii) (param $0 i32) (result i32)
  ;; CHECK-TEXT-NEXT:  (drop
  ;; CHECK-TEXT-NEXT:   (i64.eqz
  ;; CHECK-TEXT-NEXT:    (call $call-and-unary
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT:  (drop
  ;; CHECK-TEXT-NEXT:   (i64.eqz
  ;; CHECK-TEXT-NEXT:    (i32.eqz
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT:  (drop
  ;; CHECK-TEXT-NEXT:   (i64.eqz
  ;; CHECK-TEXT-NEXT:    (call_indirect $timport$0 (type $FUNCSIG$ii)
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $call-and-unary (type $FUNCSIG$ii) (param $0 i32) (result i32)
  ;; CHECK-BIN-NEXT:  (unreachable)
  ;; CHECK-BIN-NEXT: )
  (func $call-and-unary (param i32) (result i32)
    (drop
      (i64.eqz
        (call $call-and-unary
          (unreachable)
        )
      )
    )
    (drop
      (i64.eqz
        (i32.eqz
          (unreachable)
        )
      )
    )
    (drop
      (i64.eqz
        (call_indirect (type $FUNCSIG$ii)
          (unreachable)
          (unreachable)
        )
      )
    )
  )

  ;; CHECK-TEXT:      (func $tee (type $3) (param $x i32)
  ;; CHECK-TEXT-NEXT:  (local $y f32)
  ;; CHECK-TEXT-NEXT:  (drop
  ;; CHECK-TEXT-NEXT:   (i64.eqz
  ;; CHECK-TEXT-NEXT:    (local.tee $x
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT:  (drop
  ;; CHECK-TEXT-NEXT:   (local.tee $y
  ;; CHECK-TEXT-NEXT:    (i64.eqz
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $tee (type $3) (param $x i32)
  ;; CHECK-BIN-NEXT:  (local $y f32)
  ;; CHECK-BIN-NEXT:  (unreachable)
  ;; CHECK-BIN-NEXT: )
  (func $tee (param $x i32)
    (local $y f32)
    (drop
      (i64.eqz
        (local.tee $x
          (unreachable)
        )
      )
    )
    (drop
      (local.tee $y
        (i64.eqz
          (unreachable)
        )
      )
    )
  )

  ;; CHECK-TEXT:      (func $tee2 (type $2)
  ;; CHECK-TEXT-NEXT:  (local $0 f32)
  ;; CHECK-TEXT-NEXT:  (if
  ;; CHECK-TEXT-NEXT:   (i32.const 259)
  ;; CHECK-TEXT-NEXT:   (local.tee $0
  ;; CHECK-TEXT-NEXT:    (unreachable)
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $tee2 (type $2)
  ;; CHECK-BIN-NEXT:  (local $0 f32)
  ;; CHECK-BIN-NEXT:  (if
  ;; CHECK-BIN-NEXT:   (i32.const 259)
  ;; CHECK-BIN-NEXT:   (unreachable)
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT: )
  (func $tee2
    (local $0 f32)
    (if
      (i32.const 259)
      (local.set $0
        (unreachable)
      )
    )
  )

  ;; CHECK-TEXT:      (func $select (type $2)
  ;; CHECK-TEXT-NEXT:  (drop
  ;; CHECK-TEXT-NEXT:   (i64.eqz
  ;; CHECK-TEXT-NEXT:    (select
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:     (i32.const 1)
  ;; CHECK-TEXT-NEXT:     (i32.const 2)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $select (type $2)
  ;; CHECK-BIN-NEXT:  (unreachable)
  ;; CHECK-BIN-NEXT: )
  (func $select
    (drop
      (i64.eqz
        (select
          (unreachable)
          (i32.const 1)
          (i32.const 2)
        )
      )
    )
  )

  ;; CHECK-TEXT:      (func $untaken-break-should-have-value (type $0) (result i32)
  ;; CHECK-TEXT-NEXT:  (block $x (result i32)
  ;; CHECK-TEXT-NEXT:   (block
  ;; CHECK-TEXT-NEXT:    (br_if $x
  ;; CHECK-TEXT-NEXT:     (i32.const 0)
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $untaken-break-should-have-value (type $0) (result i32)
  ;; CHECK-BIN-NEXT:  (block $label$1 (result i32)
  ;; CHECK-BIN-NEXT:   (block $label$2
  ;; CHECK-BIN-NEXT:    (drop
  ;; CHECK-BIN-NEXT:     (i32.const 0)
  ;; CHECK-BIN-NEXT:    )
  ;; CHECK-BIN-NEXT:    (unreachable)
  ;; CHECK-BIN-NEXT:   )
  ;; CHECK-BIN-NEXT:   (unreachable)
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT: )
  (func $untaken-break-should-have-value (result i32)
    (block $x (result i32)
      (block
        (br_if $x
          (i32.const 0)
          (unreachable)
        )
      )
    )
  )

  ;; CHECK-TEXT:      (func $unreachable-in-block-but-code-before (type $FUNCSIG$ii) (param $0 i32) (result i32)
  ;; CHECK-TEXT-NEXT:  (if
  ;; CHECK-TEXT-NEXT:   (local.get $0)
  ;; CHECK-TEXT-NEXT:   (return
  ;; CHECK-TEXT-NEXT:    (i32.const 127)
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT:  (block $label$0 (result i32)
  ;; CHECK-TEXT-NEXT:   (br_if $label$0
  ;; CHECK-TEXT-NEXT:    (i32.const 0)
  ;; CHECK-TEXT-NEXT:    (return
  ;; CHECK-TEXT-NEXT:     (i32.const -32)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $unreachable-in-block-but-code-before (type $FUNCSIG$ii) (param $0 i32) (result i32)
  ;; CHECK-BIN-NEXT:  (if
  ;; CHECK-BIN-NEXT:   (local.get $0)
  ;; CHECK-BIN-NEXT:   (return
  ;; CHECK-BIN-NEXT:    (i32.const 127)
  ;; CHECK-BIN-NEXT:   )
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT:  (block $label$2 (result i32)
  ;; CHECK-BIN-NEXT:   (drop
  ;; CHECK-BIN-NEXT:    (i32.const 0)
  ;; CHECK-BIN-NEXT:   )
  ;; CHECK-BIN-NEXT:   (return
  ;; CHECK-BIN-NEXT:    (i32.const -32)
  ;; CHECK-BIN-NEXT:   )
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT: )
  (func $unreachable-in-block-but-code-before (param $0 i32) (result i32)
   (if
    (local.get $0)
    (return
     (i32.const 127)
    )
   )
   (block $label$0 (result i32)
    (br_if $label$0
     (i32.const 0)
     (return
      (i32.const -32)
     )
    )
   )
  )

  ;; CHECK-TEXT:      (func $br_table_unreachable_to_also_unreachable (type $0) (result i32)
  ;; CHECK-TEXT-NEXT:  (block $a (result i32)
  ;; CHECK-TEXT-NEXT:   (block $b (result i32)
  ;; CHECK-TEXT-NEXT:    (br_table $a $b
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:     (unreachable)
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $br_table_unreachable_to_also_unreachable (type $0) (result i32)
  ;; CHECK-BIN-NEXT:  (block $label$1 (result i32)
  ;; CHECK-BIN-NEXT:   (block $label$2 (result i32)
  ;; CHECK-BIN-NEXT:    (unreachable)
  ;; CHECK-BIN-NEXT:   )
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT: )
  (func $br_table_unreachable_to_also_unreachable (result i32)
    (block $a (result i32)
      (block $b (result i32)
        (br_table $a $b ;; seems to send a value, but is not taken
          (unreachable)
          (unreachable)
        )
      )
    )
  )

  ;; CHECK-TEXT:      (func $untaken-br_if (type $0) (result i32)
  ;; CHECK-TEXT-NEXT:  (block $label$8 (result i32)
  ;; CHECK-TEXT-NEXT:   (block $label$9
  ;; CHECK-TEXT-NEXT:    (drop
  ;; CHECK-TEXT-NEXT:     (if
  ;; CHECK-TEXT-NEXT:      (i32.const 0)
  ;; CHECK-TEXT-NEXT:      (br_if $label$8
  ;; CHECK-TEXT-NEXT:       (unreachable)
  ;; CHECK-TEXT-NEXT:       (i32.const 0)
  ;; CHECK-TEXT-NEXT:      )
  ;; CHECK-TEXT-NEXT:      (unreachable)
  ;; CHECK-TEXT-NEXT:     )
  ;; CHECK-TEXT-NEXT:    )
  ;; CHECK-TEXT-NEXT:   )
  ;; CHECK-TEXT-NEXT:  )
  ;; CHECK-TEXT-NEXT: )
  ;; CHECK-BIN:      (func $untaken-br_if (type $0) (result i32)
  ;; CHECK-BIN-NEXT:  (block $label$1 (result i32)
  ;; CHECK-BIN-NEXT:   (block $label$2
  ;; CHECK-BIN-NEXT:    (if
  ;; CHECK-BIN-NEXT:     (i32.const 0)
  ;; CHECK-BIN-NEXT:     (unreachable)
  ;; CHECK-BIN-NEXT:     (unreachable)
  ;; CHECK-BIN-NEXT:    )
  ;; CHECK-BIN-NEXT:   )
  ;; CHECK-BIN-NEXT:   (unreachable)
  ;; CHECK-BIN-NEXT:  )
  ;; CHECK-BIN-NEXT: )
  (func $untaken-br_if (result i32)
    (block $label$8 (result i32)
     (block $label$9
      (drop
       (if
        (i32.const 0)
        (br_if $label$8
         (unreachable)
         (i32.const 0)
        )
        (unreachable)
       )
      )
     )
    )
  )
)

;; CHECK-BIN-NODEBUG:      (func $0 (type $0) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (block $label$1 (result i32)
;; CHECK-BIN-NODEBUG-NEXT:   (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $1 (type $1) (param $0 i32) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (unreachable)
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $2 (type $3) (param $0 i32)
;; CHECK-BIN-NODEBUG-NEXT:  (local $1 f32)
;; CHECK-BIN-NODEBUG-NEXT:  (unreachable)
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $3 (type $2)
;; CHECK-BIN-NODEBUG-NEXT:  (local $0 f32)
;; CHECK-BIN-NODEBUG-NEXT:  (if
;; CHECK-BIN-NODEBUG-NEXT:   (i32.const 259)
;; CHECK-BIN-NODEBUG-NEXT:   (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $4 (type $2)
;; CHECK-BIN-NODEBUG-NEXT:  (unreachable)
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $5 (type $0) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (block $label$1 (result i32)
;; CHECK-BIN-NODEBUG-NEXT:   (block $label$2
;; CHECK-BIN-NODEBUG-NEXT:    (drop
;; CHECK-BIN-NODEBUG-NEXT:     (i32.const 0)
;; CHECK-BIN-NODEBUG-NEXT:    )
;; CHECK-BIN-NODEBUG-NEXT:    (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:   (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $6 (type $1) (param $0 i32) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (if
;; CHECK-BIN-NODEBUG-NEXT:   (local.get $0)
;; CHECK-BIN-NODEBUG-NEXT:   (return
;; CHECK-BIN-NODEBUG-NEXT:    (i32.const 127)
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT:  (block $label$2 (result i32)
;; CHECK-BIN-NODEBUG-NEXT:   (drop
;; CHECK-BIN-NODEBUG-NEXT:    (i32.const 0)
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:   (return
;; CHECK-BIN-NODEBUG-NEXT:    (i32.const -32)
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $7 (type $0) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (block $label$1 (result i32)
;; CHECK-BIN-NODEBUG-NEXT:   (block $label$2 (result i32)
;; CHECK-BIN-NODEBUG-NEXT:    (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT: )

;; CHECK-BIN-NODEBUG:      (func $8 (type $0) (result i32)
;; CHECK-BIN-NODEBUG-NEXT:  (block $label$1 (result i32)
;; CHECK-BIN-NODEBUG-NEXT:   (block $label$2
;; CHECK-BIN-NODEBUG-NEXT:    (if
;; CHECK-BIN-NODEBUG-NEXT:     (i32.const 0)
;; CHECK-BIN-NODEBUG-NEXT:     (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:     (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:    )
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:   (unreachable)
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT: )
