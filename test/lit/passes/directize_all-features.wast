;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt --directize                                                 --all-features -S -o - | filecheck %s --check-prefix=CHECK
;; RUN: foreach %s %t wasm-opt --directize --pass-arg=directize-initial-contents-immutable --all-features -S -o - | filecheck %s --check-prefix=IMMUT

(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))

 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo)

 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  ;; helper function
  (unreachable)
 )

 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call $foo
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (type $i32_=>_i32 (func (param i32) (result i32)))

 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (type $i32_=>_i32 (func (param i32) (result i32)))

 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 ;; CHECK:      (table $1 5 5 funcref)
 ;; IMMUT:      (table $1 5 5 funcref)
 (table $1 5 5 funcref)
 (elem (table $0) (i32.const 1) func $dummy)
 (elem (table $1) (i32.const 1) func $f)
 ;; CHECK:      (elem $0 (table $0) (i32.const 1) func $dummy)

 ;; CHECK:      (elem $1 (table $1) (i32.const 1) func $f)

 ;; CHECK:      (func $dummy (param $0 i32) (result i32)
 ;; CHECK-NEXT:  (local.get $0)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem $0 (table $0) (i32.const 1) func $dummy)

 ;; IMMUT:      (elem $1 (table $1) (i32.const 1) func $f)

 ;; IMMUT:      (func $dummy (param $0 i32) (result i32)
 ;; IMMUT-NEXT:  (local.get $0)
 ;; IMMUT-NEXT: )
 (func $dummy (param i32) (result i32)
  (local.get 0)
 )
 ;; CHECK:      (func $f (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $f (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $f (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $g (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call $f
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $g (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $f
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $g (param $x i32) (param $y i32)
  (call_indirect $1 (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

;; at table edges
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 ;; CHECK:      (table $1 5 5 funcref)
 ;; IMMUT:      (table $1 5 5 funcref)
 (table $1 5 5 funcref)
 (elem (table $1) (i32.const 4) func $foo)
 ;; CHECK:      (elem (table $1) (i32.const 4) func $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (table $1) (i32.const 4) func $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call $foo
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect $1 (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 4)
  )
 )
)

(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 0) $foo)
 ;; CHECK:      (elem (i32.const 0) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 0) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call $foo
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 0)
  )
 )
)

(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 ;; CHECK:      (table $1 5 5 funcref)
 ;; IMMUT:      (table $1 5 5 funcref)
 (table $1 5 5 funcref)
 (elem (i32.const 0) $foo $foo $foo $foo $foo)
 (elem (table $1) (i32.const 0) func $foo $foo $foo $foo $foo)
 ;; CHECK:      (elem $0 (table $0) (i32.const 0) func $foo $foo $foo $foo $foo)

 ;; CHECK:      (elem $1 (table $1) (i32.const 0) func $foo $foo $foo $foo $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem $0 (table $0) (i32.const 0) func $foo $foo $foo $foo $foo)

 ;; IMMUT:      (elem $1 (table $1) (i32.const 0) func $foo $foo $foo $foo $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call $foo
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect $1 (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 2)
  )
 )
)

;; imported table. only optimizable in the immutable case.
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (import "env" "table" (table $table 5 5 funcref))
 ;; IMMUT:      (import "env" "table" (table $table 5 5 funcref))
 (import "env" "table" (table $table 5 5 funcref))
 (elem (i32.const 1) $foo)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call_indirect $table (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

;; exported table. only optimizable in the immutable case.
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (export "tab" (table $0))
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (export "tab" (table $0))
 (export "tab" (table $0))
 (elem (i32.const 1) $foo)
 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

;; non-constant table offset
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (global $g (mut i32) (i32.const 1))

 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (global $g (mut i32) (i32.const 1))

 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (global $g (mut i32) (i32.const 1))
 (elem (global.get $g) $foo)
 ;; CHECK:      (elem (global.get $g) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (global.get $g) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (i32.const 1)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (global $g (mut i32) (i32.const 1))

 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (global $g (mut i32) (i32.const 1))

 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 ;; CHECK:      (table $1 5 5 funcref)
 ;; IMMUT:      (table $1 5 5 funcref)
 (table $1 5 5 funcref)
 (global $g (mut i32) (i32.const 1))
 (elem (table $1) (global.get $g) func $foo)
 ;; CHECK:      (elem (table $1) (global.get $g) func $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (table $1) (global.get $g) func $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (call_indirect $1 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (call_indirect $1 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (i32.const 1)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect $1 (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

;; non-constant call index
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (type $i32_i32_i32_=>_none (func (param i32 i32 i32)))

 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (type $i32_i32_i32_=>_none (func (param i32 i32 i32)))

 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (local.get $z)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (local.get $z)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32) (param $z i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (local.get $z)
  )
 )
)

;; bad index
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (block
 ;; IMMUT-NEXT:   (drop
 ;; IMMUT-NEXT:    (local.get $x)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:   (drop
 ;; IMMUT-NEXT:    (local.get $y)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 5)
  )
 )
)

;; missing index
(module
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (block
 ;; IMMUT-NEXT:   (drop
 ;; IMMUT-NEXT:    (local.get $x)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:   (drop
 ;; IMMUT-NEXT:    (local.get $y)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 2)
  )
 )
)

;; bad type
(module
 ;; CHECK:      (type $i32_=>_none (func (param i32)))

 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $i32_=>_none (func (param i32)))

 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (block
 ;; IMMUT-NEXT:   (drop
 ;; IMMUT-NEXT:    (local.get $x)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:   (drop
 ;; IMMUT-NEXT:    (local.get $y)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

;; no table
(module
 ;; CHECK:      (type $i32_=>_none (func (param i32)))

 ;; CHECK:      (func $foo (param $0 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (type $i32_=>_none (func (param i32)))

 ;; IMMUT:      (func $foo (param $0 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32)
  (unreachable)
 )
)

;; change types
(module
 (type (func))
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (table $0 8 8 funcref)
 ;; IMMUT:      (type $none_=>_none (func))

 ;; IMMUT:      (table $0 8 8 funcref)
 (table $0 8 8 funcref)
 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (block $block
 ;; CHECK-NEXT:   (nop)
 ;; CHECK-NEXT:   (block
 ;; CHECK-NEXT:    (block
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (unreachable)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $0
 ;; IMMUT-NEXT:  (block $block
 ;; IMMUT-NEXT:   (nop)
 ;; IMMUT-NEXT:   (block
 ;; IMMUT-NEXT:    (block
 ;; IMMUT-NEXT:    )
 ;; IMMUT-NEXT:    (unreachable)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $0
  (block ;; the type of this block will change
   (nop)
   (call_indirect (type 0)
    (i32.const 15)
   )
  )
 )
)

(module ;; indirect tail call
 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))
 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo)
 ;; CHECK:      (elem (i32.const 1) $foo)

 ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo)

 ;; IMMUT:      (func $foo (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $bar (param $x i32) (param $y i32)
 ;; CHECK-NEXT:  (return_call $foo
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar (param $x i32) (param $y i32)
 ;; IMMUT-NEXT:  (return_call $foo
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $bar (param $x i32) (param $y i32)
  (return_call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (i32.const 1)
  )
 )
)

(module
 ;; CHECK:      (type $i32_i32_i32_=>_none (func (param i32 i32 i32)))

 ;; CHECK:      (type $ii (func (param i32 i32)))
 ;; IMMUT:      (type $i32_i32_i32_=>_none (func (param i32 i32 i32)))

 ;; IMMUT:      (type $ii (func (param i32 i32)))
 (type $ii (func (param i32 i32)))

 (type $none (func))

 ;; CHECK:      (type $i32_=>_none (func (param i32)))

 ;; CHECK:      (table $0 5 5 funcref)
 ;; IMMUT:      (type $i32_=>_none (func (param i32)))

 ;; IMMUT:      (table $0 5 5 funcref)
 (table $0 5 5 funcref)
 (elem (i32.const 1) $foo1 $foo2)
 ;; CHECK:      (elem (i32.const 1) $foo1 $foo2)

 ;; CHECK:      (func $foo1 (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 1) $foo1 $foo2)

 ;; IMMUT:      (func $foo1 (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo1 (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $foo2 (param $0 i32) (param $1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $foo2 (param $0 i32) (param $1 i32)
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo2 (param i32) (param i32)
  (unreachable)
 )
 ;; CHECK:      (func $select (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (local $3 i32)
 ;; CHECK-NEXT:  (local $4 i32)
 ;; CHECK-NEXT:  (local.set $3
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $4
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (local.get $z)
 ;; CHECK-NEXT:   (call $foo1
 ;; CHECK-NEXT:    (local.get $3)
 ;; CHECK-NEXT:    (local.get $4)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (call $foo2
 ;; CHECK-NEXT:    (local.get $3)
 ;; CHECK-NEXT:    (local.get $4)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (local $3 i32)
 ;; IMMUT-NEXT:  (local $4 i32)
 ;; IMMUT-NEXT:  (local.set $3
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (local.set $4
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (if
 ;; IMMUT-NEXT:   (local.get $z)
 ;; IMMUT-NEXT:   (call $foo1
 ;; IMMUT-NEXT:    (local.get $3)
 ;; IMMUT-NEXT:    (local.get $4)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:   (call $foo2
 ;; IMMUT-NEXT:    (local.get $3)
 ;; IMMUT-NEXT:    (local.get $4)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select (param $x i32) (param $y i32) (param $z i32)
  ;; Test we can optimize a call_indirect whose index is a select between two
  ;; constants. We can emit an if and two direct calls for that.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (i32.const 1)
    (i32.const 2)
    (local.get $z)
   )
  )
 )
 ;; CHECK:      (func $select-bad-1 (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (select
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:    (i32.const 2)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-bad-1 (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (select
 ;; IMMUT-NEXT:    (local.get $z)
 ;; IMMUT-NEXT:    (i32.const 2)
 ;; IMMUT-NEXT:    (local.get $z)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-bad-1 (param $x i32) (param $y i32) (param $z i32)
  ;; As above but one select arm is not constant.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (local.get $z)
    (i32.const 2)
    (local.get $z)
   )
  )
 )
 ;; CHECK:      (func $select-bad-2 (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (select
 ;; CHECK-NEXT:    (i32.const 2)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-bad-2 (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (select
 ;; IMMUT-NEXT:    (i32.const 2)
 ;; IMMUT-NEXT:    (local.get $z)
 ;; IMMUT-NEXT:    (local.get $z)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-bad-2 (param $x i32) (param $y i32) (param $z i32)
  ;; As above but the other select arm is not constant.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (i32.const 2)
    (local.get $z)
    (local.get $z)
   )
  )
 )
 ;; CHECK:      (func $select-out-of-range (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (local $3 i32)
 ;; CHECK-NEXT:  (local $4 i32)
 ;; CHECK-NEXT:  (local.set $3
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $4
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (local.get $z)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (call $foo2
 ;; CHECK-NEXT:    (local.get $3)
 ;; CHECK-NEXT:    (local.get $4)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-out-of-range (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (local $3 i32)
 ;; IMMUT-NEXT:  (local $4 i32)
 ;; IMMUT-NEXT:  (local.set $3
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (local.set $4
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (if
 ;; IMMUT-NEXT:   (local.get $z)
 ;; IMMUT-NEXT:   (unreachable)
 ;; IMMUT-NEXT:   (call $foo2
 ;; IMMUT-NEXT:    (local.get $3)
 ;; IMMUT-NEXT:    (local.get $4)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-out-of-range (param $x i32) (param $y i32) (param $z i32)
  ;; Both are constants, but one is out of range for the table, and there is no
  ;; valid function to call there; emit an unreachable.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (i32.const 99999)
    (i32.const 2)
    (local.get $z)
   )
  )
 )
 ;; CHECK:      (func $select-both-out-of-range (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (local $3 i32)
 ;; CHECK-NEXT:  (local $4 i32)
 ;; CHECK-NEXT:  (local.set $3
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $4
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (local.get $z)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-both-out-of-range (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (local $3 i32)
 ;; IMMUT-NEXT:  (local $4 i32)
 ;; IMMUT-NEXT:  (local.set $3
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (local.set $4
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (if
 ;; IMMUT-NEXT:   (local.get $z)
 ;; IMMUT-NEXT:   (unreachable)
 ;; IMMUT-NEXT:   (unreachable)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-both-out-of-range (param $x i32) (param $y i32) (param $z i32)
  ;; Both are constants, and both are out of range for the table.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (i32.const 99999)
    (i32.const -1)
    (local.get $z)
   )
  )
 )
 ;; CHECK:      (func $select-unreachable-operand (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (select
 ;; CHECK-NEXT:    (unreachable)
 ;; CHECK-NEXT:    (i32.const 2)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-unreachable-operand (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (select
 ;; IMMUT-NEXT:    (unreachable)
 ;; IMMUT-NEXT:    (i32.const 2)
 ;; IMMUT-NEXT:    (local.get $z)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-unreachable-operand (param $x i32) (param $y i32) (param $z i32)
  ;; One operand is unreachable.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (unreachable)
    (i32.const 2)
    (local.get $z)
   )
  )
 )
 ;; CHECK:      (func $select-unreachable-condition (param $x i32) (param $y i32) (param $z i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $ii)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:   (select
 ;; CHECK-NEXT:    (i32.const 1)
 ;; CHECK-NEXT:    (i32.const 2)
 ;; CHECK-NEXT:    (unreachable)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-unreachable-condition (param $x i32) (param $y i32) (param $z i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $ii)
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (local.get $y)
 ;; IMMUT-NEXT:   (select
 ;; IMMUT-NEXT:    (i32.const 1)
 ;; IMMUT-NEXT:    (i32.const 2)
 ;; IMMUT-NEXT:    (unreachable)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-unreachable-condition (param $x i32) (param $y i32) (param $z i32)
  ;; The condition is unreachable. We should not even create any vars here, and
  ;; just not do anything.
  (call_indirect (type $ii)
   (local.get $x)
   (local.get $y)
   (select
    (i32.const 1)
    (i32.const 2)
    (unreachable)
   )
  )
 )
 ;; CHECK:      (func $select-bad-type (param $z i32)
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (local.get $z)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-bad-type (param $z i32)
 ;; IMMUT-NEXT:  (if
 ;; IMMUT-NEXT:   (local.get $z)
 ;; IMMUT-NEXT:   (unreachable)
 ;; IMMUT-NEXT:   (unreachable)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-bad-type (param $z i32)
  ;; The type here is $none, which does not match the functions at indexes 1 and
  ;; 2, so we know they will trap and can emit unreachables.
  (call_indirect (type $none)
   (select
    (i32.const 1)
    (i32.const 2)
    (local.get $z)
   )
  )
 )
)

(module
 ;; CHECK:      (type $F (func (param (ref func))))

 ;; CHECK:      (type $i32_=>_none (func (param i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (table $0 15 15 funcref)
 ;; IMMUT:      (type $F (func (param (ref func))))

 ;; IMMUT:      (type $i32_=>_none (func (param i32)))

 ;; IMMUT:      (type $none_=>_none (func))

 ;; IMMUT:      (table $0 15 15 funcref)
 (table $0 15 15 funcref)
 (type $F (func (param (ref func))))
 (elem (i32.const 10) $foo-ref $foo-ref)

 ;; CHECK:      (elem (i32.const 10) $foo-ref $foo-ref)

 ;; CHECK:      (elem declare func $select-non-nullable)

 ;; CHECK:      (func $foo-ref (param $0 (ref func))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (elem (i32.const 10) $foo-ref $foo-ref)

 ;; IMMUT:      (elem declare func $select-non-nullable)

 ;; IMMUT:      (func $foo-ref (param $0 (ref func))
 ;; IMMUT-NEXT:  (unreachable)
 ;; IMMUT-NEXT: )
 (func $foo-ref (param (ref func))
  ;; helper function
  (unreachable)
 )

 ;; CHECK:      (func $select-non-nullable (param $x i32)
 ;; CHECK-NEXT:  (local $1 (ref null $i32_=>_none))
 ;; CHECK-NEXT:  (local.set $1
 ;; CHECK-NEXT:   (ref.func $select-non-nullable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (call $foo-ref
 ;; CHECK-NEXT:    (ref.as_non_null
 ;; CHECK-NEXT:     (local.get $1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (call $foo-ref
 ;; CHECK-NEXT:    (ref.as_non_null
 ;; CHECK-NEXT:     (local.get $1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-non-nullable (param $x i32)
 ;; IMMUT-NEXT:  (local $1 (ref null $i32_=>_none))
 ;; IMMUT-NEXT:  (local.set $1
 ;; IMMUT-NEXT:   (ref.func $select-non-nullable)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT:  (if
 ;; IMMUT-NEXT:   (local.get $x)
 ;; IMMUT-NEXT:   (call $foo-ref
 ;; IMMUT-NEXT:    (ref.as_non_null
 ;; IMMUT-NEXT:     (local.get $1)
 ;; IMMUT-NEXT:    )
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:   (call $foo-ref
 ;; IMMUT-NEXT:    (ref.as_non_null
 ;; IMMUT-NEXT:     (local.get $1)
 ;; IMMUT-NEXT:    )
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-non-nullable (param $x i32)
  ;; Test we can handle a non-nullable value when optimizing a select, during
  ;; which we place values in locals.
  (call_indirect (type $F)
   (ref.func $select-non-nullable)
   (select
    (i32.const 10)
    (i32.const 11)
    (local.get $x)
   )
  )
 )

 ;; CHECK:      (func $select-non-nullable-unreachable-condition
 ;; CHECK-NEXT:  (call_indirect $0 (type $F)
 ;; CHECK-NEXT:   (ref.func $select-non-nullable)
 ;; CHECK-NEXT:   (select
 ;; CHECK-NEXT:    (i32.const 10)
 ;; CHECK-NEXT:    (i32.const 11)
 ;; CHECK-NEXT:    (unreachable)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-non-nullable-unreachable-condition
 ;; IMMUT-NEXT:  (call_indirect $0 (type $F)
 ;; IMMUT-NEXT:   (ref.func $select-non-nullable)
 ;; IMMUT-NEXT:   (select
 ;; IMMUT-NEXT:    (i32.const 10)
 ;; IMMUT-NEXT:    (i32.const 11)
 ;; IMMUT-NEXT:    (unreachable)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-non-nullable-unreachable-condition
  ;; Test we ignore an unreachable condition and don't make any changes at all
  ;; to the code (in particular, we shouldn't add any vars).
  (call_indirect (type $F)
   (ref.func $select-non-nullable)
   (select
    (i32.const 10)
    (i32.const 11)
    (unreachable)
   )
  )
 )

 ;; CHECK:      (func $select-non-nullable-unreachable-arg (param $x i32)
 ;; CHECK-NEXT:  (call_indirect $0 (type $F)
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (select
 ;; CHECK-NEXT:    (i32.const 10)
 ;; CHECK-NEXT:    (i32.const 11)
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $select-non-nullable-unreachable-arg (param $x i32)
 ;; IMMUT-NEXT:  (call_indirect $0 (type $F)
 ;; IMMUT-NEXT:   (unreachable)
 ;; IMMUT-NEXT:   (select
 ;; IMMUT-NEXT:    (i32.const 10)
 ;; IMMUT-NEXT:    (i32.const 11)
 ;; IMMUT-NEXT:    (local.get $x)
 ;; IMMUT-NEXT:   )
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $select-non-nullable-unreachable-arg (param $x i32)
  ;; Test we ignore an unreachable argument and don't make any changes at all
  ;; to the code (in particular, we shouldn't add any vars).
  (call_indirect (type $F)
   (unreachable)
   (select
    (i32.const 10)
    (i32.const 11)
    (local.get $x)
   )
  )
 )
)

;; A table.set prevents optimization.
(module
 ;; CHECK:      (type $v (func))
 ;; IMMUT:      (type $v (func))
 (type $v (func))

 ;; CHECK:      (table $has-set 5 5 funcref)
 ;; IMMUT:      (table $has-set 5 5 funcref)
 (table $has-set 5 5 funcref)

 ;; CHECK:      (table $no-set 5 5 funcref)
 ;; IMMUT:      (table $no-set 5 5 funcref)
 (table $no-set 5 5 funcref)

 ;; CHECK:      (elem $0 (table $has-set) (i32.const 1) func $foo)
 ;; IMMUT:      (elem $0 (table $has-set) (i32.const 1) func $foo)
 (elem $0 (table $has-set) (i32.const 1) $foo)

 ;; CHECK:      (elem $1 (table $no-set) (i32.const 1) func $foo)
 ;; IMMUT:      (elem $1 (table $no-set) (i32.const 1) func $foo)
 (elem $1 (table $no-set) (i32.const 1) $foo)

 ;; CHECK:      (func $foo
 ;; CHECK-NEXT:  (table.set $has-set
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (ref.func $foo)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $foo
 ;; IMMUT-NEXT:  (table.set $has-set
 ;; IMMUT-NEXT:   (i32.const 1)
 ;; IMMUT-NEXT:   (ref.func $foo)
 ;; IMMUT-NEXT:  )
 ;; IMMUT-NEXT: )
 (func $foo
  ;; Technically this set writes the same value as is already there, but the
  ;; analysis will give up on optimizing when it sees any set to a table.
  (table.set $has-set
   (i32.const 1)
   (ref.func $foo)
  )
 )

 ;; CHECK:      (func $bar
 ;; CHECK-NEXT:  (call_indirect $has-set (type $v)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $foo)
 ;; CHECK-NEXT: )
 ;; IMMUT:      (func $bar
 ;; IMMUT-NEXT:  (call $foo)
 ;; IMMUT-NEXT:  (call $foo)
 ;; IMMUT-NEXT: )
 (func $bar
  ;; We can't optimize this one, but we can the one after it. (But we can
  ;; optimize both in the immutable case.)
  (call_indirect $has-set (type $v)
   (i32.const 1)
  )
  (call_indirect $no-set (type $v)
   (i32.const 1)
  )
 )
)

;; TODO: do we need new tests?
