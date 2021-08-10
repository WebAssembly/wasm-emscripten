;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_test.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --local-cse --all-features -S -o - | filecheck %s

(module
  (memory 100 100)
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $i32_i32_i32_=>_i32 (func (param i32 i32 i32) (result i32)))

  ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

  ;; CHECK:      (type $i32_i32_=>_i32 (func (param i32 i32) (result i32)))

  ;; CHECK:      (type $f64_f64_i32_=>_f32 (func (param f64 f64 i32) (result f32)))

  ;; CHECK:      (memory $0 100 100)

  ;; CHECK:      (func $basics
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $2
  ;; CHECK-NEXT:    (i32.add
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (i32.const 2)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (nop)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:    (i32.const 2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $3
  ;; CHECK-NEXT:    (i32.add
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:     (local.get $y)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $basics)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (i32.const 100)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:    (local.get $y)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $basics
    (local $x i32)
    (local $y i32)
    ;; These two adds can be optimized.
    (drop
      (i32.add (i32.const 1) (i32.const 2))
    )
    (drop
      (i32.add (i32.const 1) (i32.const 2))
    )
    (if (i32.const 0) (nop))
    ;; This add is after an if, which means we are no longer in the same basic
    ;; block - which means we cannot optimize it with the previous identical
    ;; adds.
    (drop
      (i32.add (i32.const 1) (i32.const 2))
    )
    ;; More adds with different contents than the previous, but all three are
    ;; identical.
    (drop
      (i32.add (local.get $x) (local.get $y))
    )
    (drop
      (i32.add (local.get $x) (local.get $y))
    )
    (drop
      (i32.add (local.get $x) (local.get $y))
    )
    ;; Calls have side effects, but that is not a problem for these adds which
    ;; only use locals, so we can optimize the add after the call.
    (call $basics)
    (drop
      (i32.add (local.get $x) (local.get $y))
    )
    ;; Modify $x, which means we cannot optimize the add after the set.
    (local.set $x (i32.const 100))
    (drop
      (i32.add (local.get $x) (local.get $y))
    )
  )
  ;; CHECK:      (func $recursive1
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $3
  ;; CHECK-NEXT:    (i32.add
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (local.tee $2
  ;; CHECK-NEXT:      (i32.add
  ;; CHECK-NEXT:       (i32.const 2)
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $recursive1
    (local $x i32)
    (local $y i32)
    (drop
      (i32.add
        (i32.const 1)
        (i32.add
          (i32.const 2)
          (i32.const 3)
        )
      )
    )
    (drop
      (i32.add
        (i32.const 1)
        (i32.add
          (i32.const 2)
          (i32.const 3)
        )
      )
    )
    (drop
      (i32.add
        (i32.const 2)
        (i32.const 3)
      )
    )
  )
  ;; CHECK:      (func $recursive2
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $3
  ;; CHECK-NEXT:    (i32.add
  ;; CHECK-NEXT:     (i32.const 1)
  ;; CHECK-NEXT:     (local.tee $2
  ;; CHECK-NEXT:      (i32.add
  ;; CHECK-NEXT:       (i32.const 2)
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $recursive2
    (local $x i32)
    (local $y i32)
    (drop
      (i32.add
        (i32.const 1)
        (i32.add
          (i32.const 2)
          (i32.const 3)
        )
      )
    )
    (drop
      (i32.add
        (i32.const 2)
        (i32.const 3)
      )
    )
    (drop
      (i32.add
        (i32.const 1)
        (i32.add
          (i32.const 2)
          (i32.const 3)
        )
      )
    )
  )
  ;; CHECK:      (func $self
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y i32)
  ;; CHECK-NEXT:  (local $2 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (local.tee $2
  ;; CHECK-NEXT:     (i32.add
  ;; CHECK-NEXT:      (i32.const 2)
  ;; CHECK-NEXT:      (i32.const 3)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.get $2)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $self
    (local $x i32)
    (local $y i32)
    (drop
      (i32.add
        (i32.add
          (i32.const 2)
          (i32.const 3)
        )
        (i32.add
          (i32.const 2)
          (i32.const 3)
        )
      )
    )
    (drop
      (i32.add
        (i32.const 2)
        (i32.const 3)
      )
    )
  )
  ;; CHECK:      (func $loads
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.load
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.load
  ;; CHECK-NEXT:    (i32.const 10)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $loads
    (drop
      (i32.load (i32.const 10))
    )
    (drop
      (i32.load (i32.const 10)) ;; implicit traps, sad
    )
  )
  ;; CHECK:      (func $8 (param $var$0 i32) (result i32)
  ;; CHECK-NEXT:  (local $var$1 i32)
  ;; CHECK-NEXT:  (local $var$2 i32)
  ;; CHECK-NEXT:  (local $var$3 i32)
  ;; CHECK-NEXT:  (local $4 i32)
  ;; CHECK-NEXT:  (block $label$0 (result i32)
  ;; CHECK-NEXT:   (i32.store
  ;; CHECK-NEXT:    (local.tee $var$2
  ;; CHECK-NEXT:     (local.tee $4
  ;; CHECK-NEXT:      (i32.add
  ;; CHECK-NEXT:       (local.get $var$1)
  ;; CHECK-NEXT:       (i32.const 4)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.and
  ;; CHECK-NEXT:     (i32.load
  ;; CHECK-NEXT:      (local.get $var$2)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.xor
  ;; CHECK-NEXT:      (local.tee $var$2
  ;; CHECK-NEXT:       (i32.const 74)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (i32.const -1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.store
  ;; CHECK-NEXT:    (local.tee $var$1
  ;; CHECK-NEXT:     (local.get $4)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.or
  ;; CHECK-NEXT:     (i32.load
  ;; CHECK-NEXT:      (local.get $var$1)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (i32.and
  ;; CHECK-NEXT:      (local.get $var$2)
  ;; CHECK-NEXT:      (i32.const 8)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $8 (param $var$0 i32) (result i32)
    (local $var$1 i32)
    (local $var$2 i32)
    (local $var$3 i32)
    (block $label$0 (result i32)
      (i32.store
        (local.tee $var$2
          (i32.add
            (local.get $var$1)
            (i32.const 4)
          )
        )
        (i32.and
          (i32.load
            (local.get $var$2)
          )
          (i32.xor
            (local.tee $var$2
              (i32.const 74)
            )
            (i32.const -1)
          )
        )
      )
      (i32.store
        (local.tee $var$1
          (i32.add
            (local.get $var$1)
            (i32.const 4)
          )
        )
        (i32.or
          (i32.load
            (local.get $var$1)
          )
          (i32.and
            (local.get $var$2)
            (i32.const 8)
          )
        )
      )
      (i32.const 0)
    )
  )
  ;; CHECK:      (func $loop1 (param $x i32) (param $y i32) (result i32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $loop1 (param $x i32) (param $y i32) (result i32)
    (local.set $x (local.get $y))
    (local.set $y (local.get $x))
    (local.set $x (local.get $y))
    (local.set $y (local.get $x))
    (return (local.get $y))
  )
  ;; CHECK:      (func $loop2 (param $x i32) (param $y i32) (param $z i32) (result i32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $z)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $z)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $loop2 (param $x i32) (param $y i32) (param $z i32) (result i32)
    (local.set $x (local.get $y))
    (local.set $y (local.get $z))
    (local.set $z (local.get $x))
    (local.set $x (local.get $y))
    (local.set $y (local.get $z))
    (local.set $z (local.get $x))
    (return (local.get $x))
  )
  ;; CHECK:      (func $loop3 (param $x i32) (param $y i32) (param $z i32) (result i32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $z)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (local.get $z)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $z
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (local.get $y)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $loop3 (param $x i32) (param $y i32) (param $z i32) (result i32)
    (local.set $x (local.get $y))
    (local.set $y (local.get $z))
    (local.set $z (local.get $y))
    (local.set $y (local.get $z))
    (local.set $z (local.get $y))
    (return (local.get $y))
  )
  ;; CHECK:      (func $handle-removing (param $var$0 f64) (param $var$1 f64) (param $var$2 i32) (result f32)
  ;; CHECK-NEXT:  (local.set $var$2
  ;; CHECK-NEXT:   (select
  ;; CHECK-NEXT:    (local.tee $var$2
  ;; CHECK-NEXT:     (i32.const 32767)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (local.tee $var$2
  ;; CHECK-NEXT:     (i32.const 1024)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.const -2147483648)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f32.const 1)
  ;; CHECK-NEXT: )
  (func $handle-removing (param $var$0 f64) (param $var$1 f64) (param $var$2 i32) (result f32)
   (local.set $var$2
    (select
     (local.tee $var$2
      (i32.const 32767)
     )
     (local.tee $var$2
      (i32.const 1024)
     )
     (i32.const -2147483648)
    )
   )
   (f32.const 1)
  )
)
;; a testcase that fails if we don't handle equivalent local canonicalization properly
(module
 ;; CHECK:      (type $2 (func (param i64 f32 i32)))

 ;; CHECK:      (type $0 (func))
 (type $0 (func))
 ;; CHECK:      (type $1 (func (param i32 f64) (result i32)))
 (type $1 (func (param i32 f64) (result i32)))
 (type $2 (func (param i64 f32 i32)))
 ;; CHECK:      (global $global$0 (mut i32) (i32.const 10))
 (global $global$0 (mut i32) (i32.const 10))
 (table 23 23 funcref)
 ;; CHECK:      (table $0 23 23 funcref)

 ;; CHECK:      (export "func_1_invoker" (func $1))
 (export "func_1_invoker" (func $1))
 ;; CHECK:      (export "func_6" (func $2))
 (export "func_6" (func $2))
 ;; CHECK:      (func $0 (param $var$0 i64) (param $var$1 f32) (param $var$2 i32)
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (block $label$1 (result i32)
 ;; CHECK-NEXT:    (drop
 ;; CHECK-NEXT:     (br_if $label$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:      (br_if $label$1
 ;; CHECK-NEXT:       (i32.const 128)
 ;; CHECK-NEXT:       (i32.const 0)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (i32.const -14051)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (global.set $global$0
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $0 (; 0 ;) (type $2) (param $var$0 i64) (param $var$1 f32) (param $var$2 i32)
  (if
   (block $label$1 (result i32)
    (drop
     (br_if $label$1
      (i32.const 0)
      (br_if $label$1
       (i32.const 128)
       (i32.const 0)
      )
     )
    )
    (i32.const -14051)
   )
   (global.set $global$0
    (i32.const 0)
   )
  )
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (call $0
 ;; CHECK-NEXT:   (i64.const 1125899906842624)
 ;; CHECK-NEXT:   (f32.const -nan:0x7fc91a)
 ;; CHECK-NEXT:   (i32.const -46)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $1 (; 1 ;) (type $0)
  (call $0
   (i64.const 1125899906842624)
   (f32.const -nan:0x7fc91a)
   (i32.const -46)
  )
 )
 ;; CHECK:      (func $2 (param $var$0 i32) (param $var$1 f64) (result i32)
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (global.get $global$0)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (i32.const 0)
 ;; CHECK-NEXT: )
 (func $2 (; 2 ;) (type $1) (param $var$0 i32) (param $var$1 f64) (result i32)
  (if
   (global.get $global$0)
   (unreachable)
  )
  (i32.const 0)
 )
)
(module
 ;; CHECK:      (type $i32_=>_none (func (param i32)))

 ;; CHECK:      (import "env" "out" (func $out (param i32)))
 (import "env" "out" (func $out (param i32)))
 ;; CHECK:      (func $each-pass-must-clear (param $var$0 i32)
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (call $out
 ;; CHECK-NEXT:   (local.tee $1
 ;; CHECK-NEXT:    (i32.eqz
 ;; CHECK-NEXT:     (local.get $var$0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $out
 ;; CHECK-NEXT:   (local.get $1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $each-pass-must-clear (param $var$0 i32)
  (call $out
   (i32.eqz
    (local.get $var$0)
   )
  )
  (call $out
   (i32.eqz
    (local.get $var$0)
   )
  )
 )
)
(module
 ;; CHECK:      (type $none_=>_i64 (func (result i64)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (global $glob (mut i32) (i32.const 1))
 (global $glob (mut i32) (i32.const 1))
 ;; CHECK:      (func $i64-shifts (result i64)
 ;; CHECK-NEXT:  (local $temp i64)
 ;; CHECK-NEXT:  (local $1 i64)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (local.tee $1
 ;; CHECK-NEXT:    (i64.add
 ;; CHECK-NEXT:     (i64.const 1)
 ;; CHECK-NEXT:     (i64.const 2)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (i64.const 9999)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (local.get $1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.get $temp)
 ;; CHECK-NEXT: )
 (func $i64-shifts (result i64)
  (local $temp i64)
  (local.set $temp
   (i64.add
    (i64.const 1)
    (i64.const 2)
   )
  )
  (local.set $temp
   (i64.const 9999)
  )
  (local.set $temp
   (i64.add
    (i64.const 1)
    (i64.const 2)
   )
  )
  (local.get $temp)
 )
 ;; CHECK:      (func $global
 ;; CHECK-NEXT:  (local $x i32)
 ;; CHECK-NEXT:  (local $y i32)
 ;; CHECK-NEXT:  (local $2 i32)
 ;; CHECK-NEXT:  (local.set $x
 ;; CHECK-NEXT:   (local.tee $2
 ;; CHECK-NEXT:    (global.get $glob)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $y
 ;; CHECK-NEXT:   (local.get $2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $y
 ;; CHECK-NEXT:   (local.get $2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $global
  (local $x i32)
  (local $y i32)
  (local.set $x (global.get $glob))
  (local.set $y (global.get $glob))
  (local.set $y (global.get $glob))
 )
)

(module
 ;; After --flatten, there will be a series of chain copies between multiple
 ;; locals, but some of the locals will be funcref type and others anyref
 ;; type. We cannot make locals of different types a common subexpression.
 ;; CHECK:      (type $none_=>_anyref (func (result anyref)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $subtype-test (result anyref)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT:  (loop $label$1 (result funcref)
 ;; CHECK-NEXT:   (ref.null func)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $subtype-test (result anyref)
  (nop)
  (loop $label$1 (result funcref)
   (ref.null func)
  )
 )

 ;; CHECK:      (func $test
 ;; CHECK-NEXT:  (local $0 anyref)
 ;; CHECK-NEXT:  (local $1 funcref)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block $label$1 (result funcref)
 ;; CHECK-NEXT:    (local.set $0
 ;; CHECK-NEXT:     (local.tee $1
 ;; CHECK-NEXT:      (ref.null func)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $test
  (local $0 anyref)
  (drop
   (block $label$1 (result funcref)
    (local.set $0
     (ref.null func)
    )
    ;; After --flatten, this will be assigned to a local of funcref type. After
    ;; --local-cse, even if we set (ref.null func) to local $0 above, this
    ;; should not be replaced with $0, because it is of type anyref.
    (ref.null func)
   )
  )
 )
)

(module
 ;; CHECK:      (type $A (struct (field i32)))
 (type $A (struct (field i32)))

 ;; CHECK:      (type $ref|$A|_=>_none (func (param (ref $A))))

 ;; CHECK:      (func $GC (param $ref (ref $A))
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.tee $1
 ;; CHECK-NEXT:    (struct.get $A 0
 ;; CHECK-NEXT:     (local.get $ref)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $GC (param $ref (ref $A))
  ;; Repeated loads from a struct can be optimized.
  ;;
  ;; Note that these struct.gets cannot trap as the reference is non-nullable,
  ;; so there are no side effects here, and we can optimize.
  (drop
   (struct.get $A 0
    (local.get $ref)
   )
  )
  (drop
   (struct.get $A 0
    (local.get $ref)
   )
  )
  (drop
   (struct.get $A 0
    (local.get $ref)
   )
  )
 )
)
