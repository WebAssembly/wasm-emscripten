;; i32 operations

(module
  (func (export "add") (param $x i32) (param $y i32) (result i32) (i32.add (get_local $x) (get_local $y)))
  (func (export "div_s") (param $x i32) (param $y i32) (result i32) (i32.div_s (get_local $x) (get_local $y)))
)

(assert_return (invoke "add" (i32.const 1) (i32.const 1)) (i32.const 2))
(assert_trap (invoke "div_s" (i32.const 0) (i32.const 0)) "integer divide by zero")
(assert_trap (invoke "div_s" (i32.const 0x80000000) (i32.const -1)) "integer overflow")
