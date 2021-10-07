;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_test.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --generate-stack-ir --optimize-stack-ir --print-stack-ir --optimize-level=3 -S -o - | filecheck %s

(module
  ;; CHECK:      (type $FUNCSIG$v (func))

  ;; CHECK:      (type $5 (func (result i32)))

  ;; CHECK:      (type $6 (func (param i32) (result i32)))

  ;; CHECK:      (type $FUNCSIG$vf (func (param f32)))
  (type $FUNCSIG$vf (func (param f32)))
  (type $FUNCSIG$v (func))
  ;; CHECK:      (type $4 (func (result f64)))

  ;; CHECK:      (type $FUNCSIG$ddd (func (param f64 f64) (result f64)))

  ;; CHECK:      (type $FUNCSIG$id (func (param f64) (result i32)))
  (type $FUNCSIG$id (func (param f64) (result i32)))
  (type $FUNCSIG$ddd (func (param f64 f64) (result f64)))
  (type $4 (func (result f64)))
  (type $5 (func (result i32)))
  (type $6 (func (param i32) (result i32)))
  ;; CHECK:      (type $7 (func (param f64) (result f64)))
  (type $7 (func (param f64) (result f64)))
  ;; CHECK:      (type $8 (func (result i64)))
  (type $8 (func (result i64)))
  ;; CHECK:      (type $9 (func (param i32 i64)))
  (type $9 (func (param i32 i64)))
  ;; CHECK:      (import "env" "_emscripten_asm_const_vi" (func $_emscripten_asm_const_vi))
  (import "env" "_emscripten_asm_const_vi" (func $_emscripten_asm_const_vi))
  ;; CHECK:      (import "asm2wasm" "f64-to-int" (func $f64-to-int (param f64) (result i32)))
  (import "asm2wasm" "f64-to-int" (func $f64-to-int (param f64) (result i32)))
  ;; CHECK:      (import "asm2wasm" "f64-rem" (func $f64-rem (param f64 f64) (result f64)))
  (import "asm2wasm" "f64-rem" (func $f64-rem (param f64 f64) (result f64)))
  (table 10 funcref)
  (elem (i32.const 0) $z $big_negative $z $z $w $w $importedDoubles $w $z $cneg)
  ;; CHECK:      (memory $0 4096 4096)
  (memory $0 4096 4096)
  (data (i32.const 1026) "\14\00")
  ;; CHECK:      (data (i32.const 1026) "\14\00")

  ;; CHECK:      (table $0 10 funcref)

  ;; CHECK:      (elem (i32.const 0) $z $big_negative $z $z $w $w $importedDoubles $w $z $cneg)

  ;; CHECK:      (export "big_negative" (func $big_negative))
  (export "big_negative" (func $big_negative))
  ;; CHECK:      (func $big_negative
  ;; CHECK-NEXT:  (local $temp f64)
  ;; CHECK-NEXT:  f64.const -2147483648
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT:  f64.const -2147483648
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT:  f64.const -21474836480
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT:  f64.const 0.039625
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT:  f64.const -0.039625
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT: )
  (func $big_negative (type $FUNCSIG$v)
    (local $temp f64)
    (block $block0
      (local.set $temp
        (f64.const -2147483648)
      )
      (local.set $temp
        (f64.const -2147483648)
      )
      (local.set $temp
        (f64.const -21474836480)
      )
      (local.set $temp
        (f64.const 0.039625)
      )
      (local.set $temp
        (f64.const -0.039625)
      )
    )
  )
  ;; CHECK:      (func $importedDoubles (result f64)
  ;; CHECK-NEXT:  (local $temp f64)
  ;; CHECK-NEXT:  block $topmost (result f64)
  ;; CHECK-NEXT:   i32.const 8
  ;; CHECK-NEXT:   f64.load
  ;; CHECK-NEXT:   i32.const 16
  ;; CHECK-NEXT:   f64.load
  ;; CHECK-NEXT:   f64.add
  ;; CHECK-NEXT:   i32.const 16
  ;; CHECK-NEXT:   f64.load
  ;; CHECK-NEXT:   f64.neg
  ;; CHECK-NEXT:   f64.add
  ;; CHECK-NEXT:   i32.const 8
  ;; CHECK-NEXT:   f64.load
  ;; CHECK-NEXT:   f64.neg
  ;; CHECK-NEXT:   f64.add
  ;; CHECK-NEXT:   local.set $temp
  ;; CHECK-NEXT:   i32.const 24
  ;; CHECK-NEXT:   i32.load
  ;; CHECK-NEXT:   i32.const 0
  ;; CHECK-NEXT:   i32.gt_s
  ;; CHECK-NEXT:   if
  ;; CHECK-NEXT:    f64.const -3.4
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   i32.const 32
  ;; CHECK-NEXT:   f64.load
  ;; CHECK-NEXT:   f64.const 0
  ;; CHECK-NEXT:   f64.gt
  ;; CHECK-NEXT:   if
  ;; CHECK-NEXT:    f64.const 5.6
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   f64.const 1.2
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $importedDoubles (type $4) (result f64)
    (local $temp f64)
    (block $topmost (result f64)
      (local.set $temp
        (f64.add
          (f64.add
            (f64.add
              (f64.load
                (i32.const 8)
              )
              (f64.load
                (i32.const 16)
              )
            )
            (f64.neg
              (f64.load
                (i32.const 16)
              )
            )
          )
          (f64.neg
            (f64.load
              (i32.const 8)
            )
          )
        )
      )
      (if
        (i32.gt_s
          (i32.load
            (i32.const 24)
          )
          (i32.const 0)
        )
        (br $topmost
          (f64.const -3.4)
        )
      )
      (if
        (f64.gt
          (f64.load
            (i32.const 32)
          )
          (f64.const 0)
        )
        (br $topmost
          (f64.const 5.6)
        )
      )
      (f64.const 1.2)
    )
  )
  ;; CHECK:      (func $doubleCompares (param $x f64) (param $y f64) (result f64)
  ;; CHECK-NEXT:  (local $t f64)
  ;; CHECK-NEXT:  (local $Int f64)
  ;; CHECK-NEXT:  (local $Double i32)
  ;; CHECK-NEXT:  block $topmost (result f64)
  ;; CHECK-NEXT:   local.get $x
  ;; CHECK-NEXT:   f64.const 0
  ;; CHECK-NEXT:   f64.gt
  ;; CHECK-NEXT:   if
  ;; CHECK-NEXT:    f64.const 1.2
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   local.get $Int
  ;; CHECK-NEXT:   f64.const 0
  ;; CHECK-NEXT:   f64.gt
  ;; CHECK-NEXT:   if
  ;; CHECK-NEXT:    f64.const -3.4
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   local.get $Double
  ;; CHECK-NEXT:   i32.const 0
  ;; CHECK-NEXT:   i32.gt_s
  ;; CHECK-NEXT:   if
  ;; CHECK-NEXT:    f64.const 5.6
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   local.get $x
  ;; CHECK-NEXT:   local.get $y
  ;; CHECK-NEXT:   f64.lt
  ;; CHECK-NEXT:   if
  ;; CHECK-NEXT:    local.get $x
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   local.get $y
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $doubleCompares (type $FUNCSIG$ddd) (param $x f64) (param $y f64) (result f64)
    (local $t f64)
    (local $Int f64)
    (local $Double i32)
    (block $topmost (result f64)
      (if
        (f64.gt
          (local.get $x)
          (f64.const 0)
        )
        (br $topmost
          (f64.const 1.2)
        )
      )
      (if
        (f64.gt
          (local.get $Int)
          (f64.const 0)
        )
        (br $topmost
          (f64.const -3.4)
        )
      )
      (if
        (i32.gt_s
          (local.get $Double)
          (i32.const 0)
        )
        (br $topmost
          (f64.const 5.6)
        )
      )
      (if
        (f64.lt
          (local.get $x)
          (local.get $y)
        )
        (br $topmost
          (local.get $x)
        )
      )
      (local.get $y)
    )
  )
  ;; CHECK:      (func $intOps (result i32)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  local.get $x
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  i32.eq
  ;; CHECK-NEXT: )
  (func $intOps (type $5) (result i32)
    (local $x i32)
    (i32.eq
      (local.get $x)
      (i32.const 0)
    )
  )
  ;; CHECK:      (func $hexLiterals
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  i32.const 313249263
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT:  i32.const -19088752
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $hexLiterals (type $FUNCSIG$v)
    (drop
      (i32.add
        (i32.add
          (i32.const 0)
          (i32.const 313249263)
        )
        (i32.const -19088752)
      )
    )
  )
  ;; CHECK:      (func $conversions
  ;; CHECK-NEXT:  (local $i i32)
  ;; CHECK-NEXT:  (local $d f64)
  ;; CHECK-NEXT:  local.get $d
  ;; CHECK-NEXT:  call $f64-to-int
  ;; CHECK-NEXT:  local.set $i
  ;; CHECK-NEXT:  local.get $i
  ;; CHECK-NEXT:  f64.convert_i32_s
  ;; CHECK-NEXT:  local.set $d
  ;; CHECK-NEXT:  local.get $i
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  i32.shr_u
  ;; CHECK-NEXT:  f64.convert_i32_u
  ;; CHECK-NEXT:  local.set $d
  ;; CHECK-NEXT: )
  (func $conversions (type $FUNCSIG$v)
    (local $i i32)
    (local $d f64)
    (block $block0
      (local.set $i
        (call $f64-to-int
          (local.get $d)
        )
      )
      (local.set $d
        (f64.convert_i32_s
          (local.get $i)
        )
      )
      (local.set $d
        (f64.convert_i32_u
          (i32.shr_u
            (local.get $i)
            (i32.const 0)
          )
        )
      )
    )
  )
  ;; CHECK:      (func $seq
  ;; CHECK-NEXT:  (local $J f64)
  ;; CHECK-NEXT:  f64.const 0.1
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  f64.const 5.1
  ;; CHECK-NEXT:  f64.const 3.2
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  f64.const 4.2
  ;; CHECK-NEXT:  f64.sub
  ;; CHECK-NEXT:  local.set $J
  ;; CHECK-NEXT: )
  (func $seq (type $FUNCSIG$v)
    (local $J f64)
    (local.set $J
      (f64.sub
        (block $block0 (result f64)
          (drop
            (f64.const 0.1)
          )
          (f64.const 5.1)
        )
        (block $block1 (result f64)
          (drop
            (f64.const 3.2)
          )
          (f64.const 4.2)
        )
      )
    )
  )
  ;; CHECK:      (func $switcher (param $x i32) (result i32)
  ;; CHECK-NEXT:  block $topmost (result i32)
  ;; CHECK-NEXT:   block $switch-default$3
  ;; CHECK-NEXT:    block $switch-case$2
  ;; CHECK-NEXT:     block $switch-case$1
  ;; CHECK-NEXT:      local.get $x
  ;; CHECK-NEXT:      i32.const 1
  ;; CHECK-NEXT:      i32.sub
  ;; CHECK-NEXT:      br_table $switch-case$1 $switch-case$2 $switch-default$3
  ;; CHECK-NEXT:     end
  ;; CHECK-NEXT:     i32.const 1
  ;; CHECK-NEXT:     br $topmost
  ;; CHECK-NEXT:    end
  ;; CHECK-NEXT:    i32.const 2
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   nop
  ;; CHECK-NEXT:   block $switch-default$7
  ;; CHECK-NEXT:    block $switch-case$6
  ;; CHECK-NEXT:     block $switch-case$5
  ;; CHECK-NEXT:      local.get $x
  ;; CHECK-NEXT:      i32.const 5
  ;; CHECK-NEXT:      i32.sub
  ;; CHECK-NEXT:      br_table $switch-case$6 $switch-default$7 $switch-default$7 $switch-default$7 $switch-default$7 $switch-default$7 $switch-default$7 $switch-case$5 $switch-default$7
  ;; CHECK-NEXT:     end
  ;; CHECK-NEXT:     i32.const 121
  ;; CHECK-NEXT:     br $topmost
  ;; CHECK-NEXT:    end
  ;; CHECK-NEXT:    i32.const 51
  ;; CHECK-NEXT:    br $topmost
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   nop
  ;; CHECK-NEXT:   block $label$break$Lout
  ;; CHECK-NEXT:    block $switch-default$16
  ;; CHECK-NEXT:     block $switch-case$15
  ;; CHECK-NEXT:      block $switch-case$12
  ;; CHECK-NEXT:       block $switch-case$9
  ;; CHECK-NEXT:        block $switch-case$8
  ;; CHECK-NEXT:         local.get $x
  ;; CHECK-NEXT:         i32.const 2
  ;; CHECK-NEXT:         i32.sub
  ;; CHECK-NEXT:         br_table $switch-case$15 $switch-default$16 $switch-default$16 $switch-case$12 $switch-default$16 $switch-default$16 $switch-default$16 $switch-default$16 $switch-case$9 $switch-default$16 $switch-case$8 $switch-default$16
  ;; CHECK-NEXT:        end
  ;; CHECK-NEXT:        br $label$break$Lout
  ;; CHECK-NEXT:       end
  ;; CHECK-NEXT:       br $label$break$Lout
  ;; CHECK-NEXT:      end
  ;; CHECK-NEXT:      block $while-out$10
  ;; CHECK-NEXT:       loop $while-in$11
  ;; CHECK-NEXT:        br $while-out$10
  ;; CHECK-NEXT:       end
  ;; CHECK-NEXT:       unreachable
  ;; CHECK-NEXT:      end
  ;; CHECK-NEXT:     end
  ;; CHECK-NEXT:     loop $while-in$14
  ;; CHECK-NEXT:      br $label$break$Lout
  ;; CHECK-NEXT:     end
  ;; CHECK-NEXT:     unreachable
  ;; CHECK-NEXT:    end
  ;; CHECK-NEXT:    nop
  ;; CHECK-NEXT:   end
  ;; CHECK-NEXT:   i32.const 0
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $switcher (type $6) (param $x i32) (result i32)
    (block $topmost (result i32)
      (block $switch$0
        (block $switch-default$3
          (block $switch-case$2
            (block $switch-case$1
              (br_table $switch-case$1 $switch-case$2 $switch-default$3
                (i32.sub
                  (local.get $x)
                  (i32.const 1)
                )
              )
            )
            (br $topmost
              (i32.const 1)
            )
          )
          (br $topmost
            (i32.const 2)
          )
        )
        (nop)
      )
      (block $switch$4
        (block $switch-default$7
          (block $switch-case$6
            (block $switch-case$5
              (br_table $switch-case$6 $switch-default$7 $switch-default$7 $switch-default$7 $switch-default$7 $switch-default$7 $switch-default$7 $switch-case$5 $switch-default$7
                (i32.sub
                  (local.get $x)
                  (i32.const 5)
                )
              )
            )
            (br $topmost
              (i32.const 121)
            )
          )
          (br $topmost
            (i32.const 51)
          )
        )
        (nop)
      )
      (block $label$break$Lout
        (block $switch-default$16
          (block $switch-case$15
            (block $switch-case$12
              (block $switch-case$9
                (block $switch-case$8
                  (br_table $switch-case$15 $switch-default$16 $switch-default$16 $switch-case$12 $switch-default$16 $switch-default$16 $switch-default$16 $switch-default$16 $switch-case$9 $switch-default$16 $switch-case$8 $switch-default$16
                    (i32.sub
                      (local.get $x)
                      (i32.const 2)
                    )
                  )
                )
                (br $label$break$Lout)
              )
              (br $label$break$Lout)
            )
            (block $while-out$10
              (loop $while-in$11
                (block $block1
                  (br $while-out$10)
                  (br $while-in$11)
                )
              )
              (br $label$break$Lout)
            )
          )
          (block $while-out$13
            (loop $while-in$14
              (block $block3
                (br $label$break$Lout)
                (br $while-in$14)
              )
            )
            (br $label$break$Lout)
          )
        )
        (nop)
      )
      (i32.const 0)
    )
  )
  ;; CHECK:      (func $blocker
  ;; CHECK-NEXT:  block $label$break$L
  ;; CHECK-NEXT:   br $label$break$L
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $blocker (type $FUNCSIG$v)
    (block $label$break$L
      (br $label$break$L)
    )
  )
  ;; CHECK:      (func $frem (result f64)
  ;; CHECK-NEXT:  f64.const 5.5
  ;; CHECK-NEXT:  f64.const 1.2
  ;; CHECK-NEXT:  call $f64-rem
  ;; CHECK-NEXT: )
  (func $frem (type $4) (result f64)
    (call $f64-rem
      (f64.const 5.5)
      (f64.const 1.2)
    )
  )
  ;; CHECK:      (func $big_uint_div_u (result i32)
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  i32.const -1
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  i32.div_u
  ;; CHECK-NEXT:  i32.const -1
  ;; CHECK-NEXT:  i32.and
  ;; CHECK-NEXT: )
  (func $big_uint_div_u (type $5) (result i32)
    (local $x i32)
    (block $topmost (result i32)
      (local.set $x
        (i32.and
          (i32.div_u
            (i32.const -1)
            (i32.const 2)
          )
          (i32.const -1)
        )
      )
      (local.get $x)
    )
  )
  ;; CHECK:      (func $fr (param $x f32)
  ;; CHECK-NEXT:  (local $y f32)
  ;; CHECK-NEXT:  (local $z f64)
  ;; CHECK-NEXT:  local.get $z
  ;; CHECK-NEXT:  f32.demote_f64
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  local.get $y
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  f32.const 5
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  f32.const 0
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  f32.const 5
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  f32.const 0
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $fr (type $FUNCSIG$vf) (param $x f32)
    (local $y f32)
    (local $z f64)
    (block $block0
      (drop
        (f32.demote_f64
          (local.get $z)
        )
      )
      (drop
        (local.get $y)
      )
      (drop
        (f32.const 5)
      )
      (drop
        (f32.const 0)
      )
      (drop
        (f32.const 5)
      )
      (drop
        (f32.const 0)
      )
    )
  )
  ;; CHECK:      (func $negZero (result f64)
  ;; CHECK-NEXT:  f64.const -0
  ;; CHECK-NEXT: )
  (func $negZero (type $4) (result f64)
    (f64.const -0)
  )
  ;; CHECK:      (func $abs
  ;; CHECK-NEXT:  (local $x i32)
  ;; CHECK-NEXT:  (local $y f64)
  ;; CHECK-NEXT:  (local $z f32)
  ;; CHECK-NEXT:  (local $asm2wasm_i32_temp i32)
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  local.set $asm2wasm_i32_temp
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  local.get $asm2wasm_i32_temp
  ;; CHECK-NEXT:  i32.sub
  ;; CHECK-NEXT:  local.get $asm2wasm_i32_temp
  ;; CHECK-NEXT:  local.get $asm2wasm_i32_temp
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  i32.lt_s
  ;; CHECK-NEXT:  select
  ;; CHECK-NEXT:  local.set $x
  ;; CHECK-NEXT:  f64.const 0
  ;; CHECK-NEXT:  f64.abs
  ;; CHECK-NEXT:  local.set $y
  ;; CHECK-NEXT:  f32.const 0
  ;; CHECK-NEXT:  f32.abs
  ;; CHECK-NEXT:  local.set $z
  ;; CHECK-NEXT: )
  (func $abs (type $FUNCSIG$v)
    (local $x i32)
    (local $y f64)
    (local $z f32)
    (local $asm2wasm_i32_temp i32)
    (block $block0
      (local.set $x
        (block $block1 (result i32)
          (local.set $asm2wasm_i32_temp
            (i32.const 0)
          )
          (select
            (i32.sub
              (i32.const 0)
              (local.get $asm2wasm_i32_temp)
            )
            (local.get $asm2wasm_i32_temp)
            (i32.lt_s
              (local.get $asm2wasm_i32_temp)
              (i32.const 0)
            )
          )
        )
      )
      (local.set $y
        (f64.abs
          (f64.const 0)
        )
      )
      (local.set $z
        (f32.abs
          (f32.const 0)
        )
      )
    )
  )
  ;; CHECK:      (func $neg
  ;; CHECK-NEXT:  (local $x f32)
  ;; CHECK-NEXT:  local.get $x
  ;; CHECK-NEXT:  f32.neg
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  i32.const 7
  ;; CHECK-NEXT:  i32.and
  ;; CHECK-NEXT:  i32.const 8
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT:  call_indirect $0 (type $f32_=>_none)
  ;; CHECK-NEXT: )
  (func $neg (type $FUNCSIG$v)
    (local $x f32)
    (block $block0
      (local.set $x
        (f32.neg
          (local.get $x)
        )
      )
      (call_indirect (type $FUNCSIG$vf)
        (local.get $x)
        (i32.add
          (i32.and
            (i32.const 1)
            (i32.const 7)
          )
          (i32.const 8)
        )
      )
    )
  )
  ;; CHECK:      (func $cneg (param $x f32)
  ;; CHECK-NEXT:  local.get $x
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  i32.const 7
  ;; CHECK-NEXT:  i32.and
  ;; CHECK-NEXT:  i32.const 8
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT:  call_indirect $0 (type $f32_=>_none)
  ;; CHECK-NEXT: )
  (func $cneg (type $FUNCSIG$vf) (param $x f32)
    (call_indirect (type $FUNCSIG$vf)
      (local.get $x)
      (i32.add
        (i32.and
          (i32.const 1)
          (i32.const 7)
        )
        (i32.const 8)
      )
    )
  )
  ;; CHECK:      (func $___syscall_ret
  ;; CHECK-NEXT:  (local $$0 i32)
  ;; CHECK-NEXT:  local.get $$0
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  i32.shr_u
  ;; CHECK-NEXT:  i32.const -4096
  ;; CHECK-NEXT:  i32.gt_u
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $___syscall_ret (type $FUNCSIG$v)
    (local $$0 i32)
    (drop
      (i32.gt_u
        (i32.shr_u
          (local.get $$0)
          (i32.const 0)
        )
        (i32.const -4096)
      )
    )
  )
  ;; CHECK:      (func $z
  ;; CHECK-NEXT:  nop
  ;; CHECK-NEXT: )
  (func $z (type $FUNCSIG$v)
    (nop)
  )
  ;; CHECK:      (func $w
  ;; CHECK-NEXT:  nop
  ;; CHECK-NEXT: )
  (func $w (type $FUNCSIG$v)
    (nop)
  )
  ;; CHECK:      (func $block_and_after (result i32)
  ;; CHECK-NEXT:  block $waka
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   drop
  ;; CHECK-NEXT:   br $waka
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT: )
  (func $block_and_after (type $5) (result i32)
    (block $waka
      (drop
        (i32.const 1)
      )
      (br $waka)
    )
    (i32.const 0)
  )
  ;; CHECK:      (func $loop-roundtrip (param $0 f64) (result f64)
  ;; CHECK-NEXT:  loop $loop-in1 (result f64)
  ;; CHECK-NEXT:   local.get $0
  ;; CHECK-NEXT:   drop
  ;; CHECK-NEXT:   local.get $0
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $loop-roundtrip (type $7) (param $0 f64) (result f64)
    (loop $loop-in1 (result f64)
      (drop
        (local.get $0)
      )
      (local.get $0)
    )
  )
  ;; CHECK:      (func $big-i64 (result i64)
  ;; CHECK-NEXT:  i64.const -9218868437227405313
  ;; CHECK-NEXT: )
  (func $big-i64 (type $8) (result i64)
    (i64.const -9218868437227405313)
  )
  ;; CHECK:      (func $i64-store32 (param $0 i32) (param $1 i64)
  ;; CHECK-NEXT:  local.get $0
  ;; CHECK-NEXT:  local.get $1
  ;; CHECK-NEXT:  i64.store32
  ;; CHECK-NEXT: )
  (func $i64-store32 (type $9) (param $0 i32) (param $1 i64)
    (i64.store32
      (local.get $0)
      (local.get $1)
    )
  )
  ;; CHECK:      (func $return-unreachable (result i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  return
  ;; CHECK-NEXT: )
  (func $return-unreachable (result i32)
    (return (i32.const 1))
  )
  ;; CHECK:      (func $unreachable-block (result i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  return
  ;; CHECK-NEXT: )
  (func $unreachable-block (result i32)
    (f64.abs
      (block ;; note no type - valid in binaryen IR, in wasm must be i32
        (drop (i32.const 1))
        (return (i32.const 2))
      )
    )
  )
  ;; CHECK:      (func $unreachable-block-toplevel (result i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  return
  ;; CHECK-NEXT: )
  (func $unreachable-block-toplevel (result i32)
    (block ;; note no type - valid in binaryen IR, in wasm must be i32
      (drop (i32.const 1))
      (return (i32.const 2))
    )
  )
  ;; CHECK:      (func $unreachable-block0 (result i32)
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  return
  ;; CHECK-NEXT: )
  (func $unreachable-block0 (result i32)
    (f64.abs
      (block ;; note no type - valid in binaryen IR, in wasm must be i32
        (return (i32.const 2))
      )
    )
  )
  ;; CHECK:      (func $unreachable-block0-toplevel (result i32)
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  return
  ;; CHECK-NEXT: )
  (func $unreachable-block0-toplevel (result i32)
    (block ;; note no type - valid in binaryen IR, in wasm must be i32
      (return (i32.const 2))
    )
  )
  ;; CHECK:      (func $unreachable-block-with-br (result i32)
  ;; CHECK-NEXT:  block $block
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   drop
  ;; CHECK-NEXT:   br $block
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT: )
  (func $unreachable-block-with-br (result i32)
    (block $block ;; unreachable type due to last element having that type, but the block is exitable
      (drop (i32.const 1))
      (br $block)
    )
    (i32.const 1)
  )
  ;; CHECK:      (func $unreachable-if (result i32)
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  if
  ;; CHECK-NEXT:   i32.const 2
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  else
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-if (result i32)
    (f64.abs
      (if ;; note no type - valid in binaryen IR, in wasm must be i32
        (i32.const 3)
        (return (i32.const 2))
        (return (i32.const 1))
      )
    )
  )
  ;; CHECK:      (func $unreachable-if-toplevel (result i32)
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  if
  ;; CHECK-NEXT:   i32.const 2
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  else
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-if-toplevel (result i32)
    (if ;; note no type - valid in binaryen IR, in wasm must be i32
      (i32.const 3)
      (return (i32.const 2))
      (return (i32.const 1))
    )
  )
  ;; CHECK:      (func $unreachable-loop (result i32)
  ;; CHECK-NEXT:  loop $loop-in
  ;; CHECK-NEXT:   nop
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-loop (result i32)
    (f64.abs
      (loop ;; note no type - valid in binaryen IR, in wasm must be i32
        (nop)
        (return (i32.const 1))
      )
    )
  )
  ;; CHECK:      (func $unreachable-loop0 (result i32)
  ;; CHECK-NEXT:  loop $loop-in
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-loop0 (result i32)
    (f64.abs
      (loop ;; note no type - valid in binaryen IR, in wasm must be i32
        (return (i32.const 1))
      )
    )
  )
  ;; CHECK:      (func $unreachable-loop-toplevel (result i32)
  ;; CHECK-NEXT:  loop $loop-in
  ;; CHECK-NEXT:   nop
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-loop-toplevel (result i32)
    (loop ;; note no type - valid in binaryen IR, in wasm must be i32
      (nop)
      (return (i32.const 1))
    )
  )
  ;; CHECK:      (func $unreachable-loop0-toplevel (result i32)
  ;; CHECK-NEXT:  loop $loop-in
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   return
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-loop0-toplevel (result i32)
    (loop ;; note no type - valid in binaryen IR, in wasm must be i32
      (return (i32.const 1))
    )
  )
  ;; CHECK:      (func $unreachable-ifs
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  (func $unreachable-ifs
    (if (unreachable) (nop))
    (if (unreachable) (unreachable))
    (if (unreachable) (nop) (nop))
    (if (unreachable) (unreachable) (nop))
    (if (unreachable) (nop) (unreachable))
    (if (unreachable) (unreachable) (unreachable))
    ;;
    (if (i32.const 1) (unreachable) (nop))
    (if (i32.const 1) (nop) (unreachable))
    (if (i32.const 1) (unreachable) (unreachable))
  )
  ;; CHECK:      (func $unreachable-if-arm
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  if
  ;; CHECK-NEXT:   nop
  ;; CHECK-NEXT:  else
  ;; CHECK-NEXT:   unreachable
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $unreachable-if-arm
    (if
      (i32.const 1)
      (block
        (nop)
      )
      (block
        (unreachable)
        (drop
          (i32.const 1)
        )
      )
    )
  )
  ;; CHECK:      (func $local-to-stack (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $local-to-stack (param $x i32) (result i32)
    (local $temp i32)
    (local.set $temp (call $local-to-stack (i32.const 1))) ;; this set could just be on the stack
    (drop (call $local-to-stack (i32.const 2)))
    (local.get $temp)
  )
  ;; CHECK:      (func $local-to-stack-1 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.eqz
  ;; CHECK-NEXT: )
  (func $local-to-stack-1 (param $x i32) (result i32)
    (local $temp i32)
    (local.set $temp (call $local-to-stack (i32.const 1)))
    (drop (call $local-to-stack (i32.const 2)))
    (i32.eqz
      (local.get $temp)
    )
  )
  ;; CHECK:      (func $local-to-stack-1b (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $local-to-stack-1b (param $x i32) (result i32)
    (local $temp i32)
    (local.set $temp (call $local-to-stack (i32.const 1)))
    (drop (call $local-to-stack (i32.const 2)))
    (i32.add
      (local.get $temp)
      (i32.const 3)
    )
  )
  ;; CHECK:      (func $local-to-stack-1c-no (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  local.get $temp
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $local-to-stack-1c-no (param $x i32) (result i32)
    (local $temp i32)
    (local.set $temp (call $local-to-stack (i32.const 1)))
    (drop (call $local-to-stack (i32.const 2)))
    (i32.add
      (i32.const 3) ;; this is in the way
      (local.get $temp)
    )
  )
  ;; CHECK:      (func $local-to-stack-2-no (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  local.set $temp
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  local.get $temp
  ;; CHECK-NEXT:  local.get $temp
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $local-to-stack-2-no (param $x i32) (result i32)
    (local $temp i32)
    (local.set $temp (call $local-to-stack (i32.const 1)))
    (drop (call $local-to-stack (i32.const 2)))
    (i32.add
      (local.get $temp)
      (local.get $temp) ;; a second use - so cannot stack it
    )
  )
  ;; CHECK:      (func $local-to-stack-3-no (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  if
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   call $local-to-stack
  ;; CHECK-NEXT:   local.set $temp
  ;; CHECK-NEXT:  else
  ;; CHECK-NEXT:   i32.const 2
  ;; CHECK-NEXT:   call $local-to-stack
  ;; CHECK-NEXT:   local.set $temp
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  local.get $temp
  ;; CHECK-NEXT: )
  (func $local-to-stack-3-no (param $x i32) (result i32)
    (local $temp i32)
    (if (i32.const 1)
      (local.set $temp (call $local-to-stack (i32.const 1)))
      (local.set $temp (call $local-to-stack (i32.const 2))) ;; two sets for that get
    )
    (drop (call $local-to-stack (i32.const 3)))
    (local.get $temp)
  )
  ;; CHECK:      (func $local-to-stack-multi-4 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 4
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $local-to-stack-multi-4 (param $x i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (drop (call $local-to-stack-multi-4 (i32.const 2)))
    (drop (local.get $temp1))
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 3))) ;; same local, used later
    (drop (call $local-to-stack-multi-4 (i32.const 4)))
    (local.get $temp1)
  )
  ;; CHECK:      (func $local-to-stack-multi-5 (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 4
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $local-to-stack-multi-5 (param $x i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (drop (call $local-to-stack-multi-4 (i32.const 2)))
    (drop (local.get $temp1))
    (local.set $temp2 (call $local-to-stack-multi-4 (i32.const 3))) ;; different local, used later
    (drop (call $local-to-stack-multi-4 (i32.const 4)))
    (local.get $temp2)
  )
  ;; CHECK:      (func $local-to-stack-multi-6-justone (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  local.set $temp2
  ;; CHECK-NEXT:  i32.const 4
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  local.get $temp2
  ;; CHECK-NEXT:  local.get $temp2
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $local-to-stack-multi-6-justone (param $x i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (drop (call $local-to-stack-multi-4 (i32.const 2)))
    (drop (local.get $temp1))
    (local.set $temp2 (call $local-to-stack-multi-4 (i32.const 3))) ;; different local, used later
    (drop (call $local-to-stack-multi-4 (i32.const 4)))
    (i32.add
      (local.get $temp2)
      (local.get $temp2)
    )
  )
  ;; CHECK:      (func $local-to-stack-multi-7-justone (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  local.set $temp1
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  local.get $temp1
  ;; CHECK-NEXT:  local.get $temp1
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 4
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $local-to-stack-multi-7-justone (param $x i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (drop (call $local-to-stack-multi-4 (i32.const 2)))
    (drop
      (i32.add
        (local.get $temp1)
        (local.get $temp1)
      )
    )
    (local.set $temp2 (call $local-to-stack-multi-4 (i32.const 3))) ;; different local, used later
    (drop (call $local-to-stack-multi-4 (i32.const 4)))
    (local.get $temp2)
  )
  ;; CHECK:      (func $local-to-stack-overlapping-multi-8-no (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  local.set $temp1
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  local.get $temp1
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $local-to-stack-overlapping-multi-8-no (param $x i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (local.set $temp2 (call $local-to-stack-multi-4 (i32.const 1)))
    (drop (call $local-to-stack-multi-4 (i32.const 3)))
    (i32.add
      (local.get $temp2) ;; the timing
      (local.get $temp1) ;; it sucks
    )
  )
  ;; CHECK:      (func $local-to-stack-overlapping-multi-9-yes (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 3
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $local-to-stack-overlapping-multi-9-yes (param $x i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (local.set $temp2 (call $local-to-stack-multi-4 (i32.const 1)))
    (drop (call $local-to-stack-multi-4 (i32.const 3)))
    (i32.add
      (local.get $temp1) ;; the stars align
      (local.get $temp2) ;; and a time presents itself
    )
  )
  ;; CHECK:      (func $local-to-stack-through-control-flow
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  (local $temp2 i32)
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  if
  ;; CHECK-NEXT:   nop
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $local-to-stack-multi-4
  ;; CHECK-NEXT:  block $block
  ;; CHECK-NEXT:   br $block
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT: )
  (func $local-to-stack-through-control-flow
    (local $temp1 i32)
    (local $temp2 i32)
    (local.set $temp2 (call $local-to-stack-multi-4 (i32.const 0)))
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
    (if (i32.const 0) (nop))
    (drop (local.get $temp1))
    (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 2)))
    (block $block (br $block))
    (drop (local.get $temp1))
    (drop (local.get $temp2))
  )
  ;; CHECK:      (func $local-to-stack-in-control-flow
  ;; CHECK-NEXT:  (local $temp1 i32)
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  if
  ;; CHECK-NEXT:   i32.const 0
  ;; CHECK-NEXT:   call $local-to-stack-multi-4
  ;; CHECK-NEXT:   drop
  ;; CHECK-NEXT:  else
  ;; CHECK-NEXT:   i32.const 1
  ;; CHECK-NEXT:   call $local-to-stack-multi-4
  ;; CHECK-NEXT:   drop
  ;; CHECK-NEXT:  end
  ;; CHECK-NEXT: )
  (func $local-to-stack-in-control-flow
    (local $temp1 i32)
    (if (i32.const 0)
      (block
        (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 0)))
        (drop (local.get $temp1))
      )
      (block
        (local.set $temp1 (call $local-to-stack-multi-4 (i32.const 1)))
        (drop (local.get $temp1))
      )
    )
  )
  ;; CHECK:      (func $remove-block (param $x i32) (result i32)
  ;; CHECK-NEXT:  (local $temp i32)
  ;; CHECK-NEXT:  i32.const 0
  ;; CHECK-NEXT:  call $remove-block
  ;; CHECK-NEXT:  i32.const 1
  ;; CHECK-NEXT:  call $remove-block
  ;; CHECK-NEXT:  i32.const 2
  ;; CHECK-NEXT:  call $remove-block
  ;; CHECK-NEXT:  drop
  ;; CHECK-NEXT:  i32.eqz
  ;; CHECK-NEXT:  i32.add
  ;; CHECK-NEXT: )
  (func $remove-block (param $x i32) (result i32)
   (local $temp i32)
   (i32.add
    (call $remove-block (i32.const 0))
    (i32.eqz
     (block (result i32) ;; after we use the stack instead of the local, we can remove this block
      (local.set $temp (call $remove-block (i32.const 1)))
      (drop (call $remove-block (i32.const 2)))
      (local.get $temp)
     )
    )
   )
  )
)
