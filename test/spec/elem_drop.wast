
;; Implicitly dropped elements

(module
  (table 10 funcref)
  (elem $e (i32.const 0) func $f)
  (func $f)
  (func (export "init")
    (table.init $e (i32.const 0) (i32.const 0) (i32.const 1))
  )
)
(assert_trap (invoke "init") "out of bounds table access")

(assert_invalid
  (module
    (table 10 funcref)
    (elem $e declare func $f)
    (func $f)
    (func (export "init")
      (table.init $e (i32.const 0) (i32.const 0) (i32.const 1))
    )
  )
)
;;; Binaryen does not add declarative element segments to the IR. Therefore, a
;;;  table.init on a declarative segment will result in a validation error, not
;;;  a trap.
;; (assert_trap (invoke "init") "out of bounds table access")
