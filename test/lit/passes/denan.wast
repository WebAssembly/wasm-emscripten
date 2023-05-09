;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --denan -S -o - | filecheck %s

(module
  ;; CHECK:      (type $f32_=>_f32 (func (param f32) (result f32)))

  ;; CHECK:      (type $f64_=>_f64 (func (param f64) (result f64)))

  ;; CHECK:      (type $i32_f32_i64_f64_=>_none (func (param i32 f32 i64 f64)))

  ;; CHECK:      (type $f32_f64_=>_none (func (param f32 f64)))

  ;; CHECK:      (global $global$1 (mut f32) (f32.const 0))
  (global $global$1 (mut f32) (f32.const nan))
  ;; CHECK:      (global $global$2 (mut f32) (f32.const 12.34000015258789))
  (global $global$2 (mut f32) (f32.const 12.34))
  ;; CHECK:      (func $foo32 (param $x f32) (result f32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $deNan32
  ;; CHECK-NEXT:   (call $foo32
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo32 (param $x f32) (result f32)
    (call $foo32 (local.get $x))
  )
  ;; CHECK:      (func $foo64 (param $x f64) (result f64)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $deNan64
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $deNan64
  ;; CHECK-NEXT:   (call $foo64
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo64 (param $x f64) (result f64)
    (call $foo64 (local.get $x))
  )
  ;; CHECK:      (func $various (param $x i32) (param $y f32) (param $z i64) (param $w f64)
  ;; CHECK-NEXT:  (local.set $y
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (local.get $y)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $w
  ;; CHECK-NEXT:   (call $deNan64
  ;; CHECK-NEXT:    (local.get $w)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $various (param $x i32) (param $y f32) (param $z i64) (param $w f64)
  )
  ;; CHECK:      (func $ignore-local.get (param $f f32) (param $d f64)
  ;; CHECK-NEXT:  (local.set $f
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (local.get $f)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $d
  ;; CHECK-NEXT:   (call $deNan64
  ;; CHECK-NEXT:    (local.get $d)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $f)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $d)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $f
  ;; CHECK-NEXT:   (local.get $f)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $d
  ;; CHECK-NEXT:   (local.get $d)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $f)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $d)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (f32.abs
  ;; CHECK-NEXT:     (local.get $f)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $deNan64
  ;; CHECK-NEXT:    (f64.abs
  ;; CHECK-NEXT:     (local.get $d)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $f
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (f32.abs
  ;; CHECK-NEXT:     (local.get $f)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $d
  ;; CHECK-NEXT:   (call $deNan64
  ;; CHECK-NEXT:    (f64.abs
  ;; CHECK-NEXT:     (local.get $d)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $f)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $d)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $ignore-local.get (param $f f32) (param $d f64)
    (drop (local.get $f))
    (drop (local.get $d))
    (local.set $f (local.get $f))
    (local.set $d (local.get $d))
    (drop (local.get $f))
    (drop (local.get $d))
    (drop (f32.abs (local.get $f)))
    (drop (f64.abs (local.get $d)))
    (local.set $f (f32.abs (local.get $f)))
    (local.set $d (f64.abs (local.get $d)))
    (drop (local.get $f))
    (drop (local.get $d))
  )
  ;; CHECK:      (func $tees (param $x f32) (result f32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.tee $x
  ;; CHECK-NEXT:   (local.tee $x
  ;; CHECK-NEXT:    (local.tee $x
  ;; CHECK-NEXT:     (local.tee $x
  ;; CHECK-NEXT:      (local.get $x)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $tees (param $x f32) (result f32)
    (local.tee $x
      (local.tee $x
        (local.tee $x
          (local.tee $x
            (local.get $x))))))
  ;; CHECK:      (func $select (param $x f32) (result f32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $deNan32
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (select
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (local.get $x)
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $select (param $x f32) (result f32)
    (select
      (local.get $x)
      (local.get $x)
      (i32.const 1)))
)
;; existing names should not be a problem
;; CHECK:      (func $deNan32 (param $0 f32) (result f32)
;; CHECK-NEXT:  (if (result f32)
;; CHECK-NEXT:   (f32.eq
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:   (f32.const 0)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $deNan64 (param $0 f64) (result f64)
;; CHECK-NEXT:  (if (result f64)
;; CHECK-NEXT:   (f64.eq
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:   (f64.const 0)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $f32_=>_f32 (func (param f32) (result f32)))

  ;; CHECK:      (type $f64_=>_f64 (func (param f64) (result f64)))

  ;; CHECK:      (func $deNan32
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $deNan32)
  ;; CHECK:      (func $deNan64
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $deNan64)
  ;; CHECK:      (func $foo32 (param $x f32) (result f32)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $deNan32_4
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $deNan32_4
  ;; CHECK-NEXT:   (call $foo32
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo32 (param $x f32) (result f32)
    (call $foo32 (local.get $x))
  )
  ;; CHECK:      (func $foo64 (param $x f64) (result f64)
  ;; CHECK-NEXT:  (local.set $x
  ;; CHECK-NEXT:   (call $deNan64_4
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $deNan64_4
  ;; CHECK-NEXT:   (call $foo64
  ;; CHECK-NEXT:    (local.get $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo64 (param $x f64) (result f64)
    (call $foo64 (local.get $x))
  )

)

;; CHECK:      (func $deNan32_4 (param $0 f32) (result f32)
;; CHECK-NEXT:  (if (result f32)
;; CHECK-NEXT:   (f32.eq
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:   (f32.const 0)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $deNan64_4 (param $0 f64) (result f64)
;; CHECK-NEXT:  (if (result f64)
;; CHECK-NEXT:   (f64.eq
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $0)
;; CHECK-NEXT:   (f64.const 0)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
