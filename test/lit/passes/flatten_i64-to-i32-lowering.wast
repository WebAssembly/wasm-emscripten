;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt -all --flatten --i64-to-i32-lowering -S -o - | filecheck %s

(module
 (memory 1 1)
 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (type $none_=>_i64 (func (result i64)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (import "env" "func" (func $import (result i64)))
 (import "env" "func" (func $import (result i64)))
 ;; CHECK:      (global $i64toi32_i32$HIGH_BITS (mut i32) (i32.const 0))

 ;; CHECK:      (memory $0 1 1)

 ;; CHECK:      (func $defined (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (local $0$hi i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$1 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$2 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$3 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$4 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$5 i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $0
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:      (block (result i32)
 ;; CHECK-NEXT:       (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:        (i32.const 0)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:       (i32.const 1)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$3
 ;; CHECK-NEXT:      (block (result i32)
 ;; CHECK-NEXT:       (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:        (i32.const 0)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:       (i32.const 2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$4
 ;; CHECK-NEXT:      (i32.add
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$3)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$5
 ;; CHECK-NEXT:      (i32.add
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (if
 ;; CHECK-NEXT:      (i32.lt_u
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$4)
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$3)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:      (local.set $i64toi32_i32$5
 ;; CHECK-NEXT:       (i32.add
 ;; CHECK-NEXT:        (local.get $i64toi32_i32$5)
 ;; CHECK-NEXT:        (i32.const 1)
 ;; CHECK-NEXT:       )
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $i64toi32_i32$4)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (local.set $0$hi
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$5)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$5
 ;; CHECK-NEXT:      (local.get $0$hi)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (global.set $i64toi32_i32$HIGH_BITS
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$5)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $defined (result i64)
  (i64.add (i64.const 1) (i64.const 2))
 )
 ;; CHECK:      (func $unreachable-select-i64 (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (i32.const 2)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $unreachable-select-i64 (result i64)
  (select
   (i64.const 1)
   (unreachable)
   (i32.const 2)
  )
 )
 ;; CHECK:      (func $unreachable-select-i64-b (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 3)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (i32.const 4)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $unreachable-select-i64-b (result i64)
  (select
   (unreachable)
   (i64.const 3)
   (i32.const 4)
  )
 )
 ;; CHECK:      (func $unreachable-select-i64-c (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$1 i32)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 5)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 6)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $unreachable-select-i64-c (result i64)
  (select
   (i64.const 5)
   (i64.const 6)
   (unreachable)
  )
 )
 ;; CHECK:      (func $mem (type $none_=>_none)
 ;; CHECK-NEXT:  (local $0 i32)
 ;; CHECK-NEXT:  (local $0$hi i32)
 ;; CHECK-NEXT:  (local $1 i32)
 ;; CHECK-NEXT:  (local $1$hi i32)
 ;; CHECK-NEXT:  (local $2 i32)
 ;; CHECK-NEXT:  (local $2$hi i32)
 ;; CHECK-NEXT:  (local $3 i32)
 ;; CHECK-NEXT:  (local $3$hi i32)
 ;; CHECK-NEXT:  (local $4 i32)
 ;; CHECK-NEXT:  (local $4$hi i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$1 i32)
 ;; CHECK-NEXT:  (local $i64toi32_i32$2 i32)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $0
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.load
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.load offset=4
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (local.set $0$hi
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:     (local.get $0$hi)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $1
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.load
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.load offset=4
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (local.set $1$hi
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:     (local.get $1$hi)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $2
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.load align=2
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.load offset=4 align=2
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (local.set $2$hi
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:     (local.get $2$hi)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $2)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $3
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.load align=1
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.load offset=4 align=1
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (local.set $3$hi
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:     (local.get $3$hi)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $3)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $4
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$2
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:      (i32.load
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.load offset=4
 ;; CHECK-NEXT:       (local.get $i64toi32_i32$2)
 ;; CHECK-NEXT:      )
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (local.set $4$hi
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result i32)
 ;; CHECK-NEXT:    (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:     (local.get $4$hi)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (local.get $4)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 1)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store offset=4
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 2)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store offset=4
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store align=2
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 3)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store offset=4 align=2
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store align=1
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 4)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store offset=4 align=1
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (local.set $i64toi32_i32$0
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (block (result i32)
 ;; CHECK-NEXT:     (local.set $i64toi32_i32$1
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:     (i32.const 5)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.store offset=4
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
 ;; CHECK-NEXT:    (local.get $i64toi32_i32$1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $mem
  (drop (i64.load align=8 (i32.const 0)))
  (drop (i64.load align=4 (i32.const 0)))
  (drop (i64.load align=2 (i32.const 0)))
  (drop (i64.load align=1 (i32.const 0)))
  (drop (i64.load          (i32.const 0)))
  (i64.store align=8 (i32.const 0) (i64.const 1))
  (i64.store align=4 (i32.const 0) (i64.const 2))
  (i64.store align=2 (i32.const 0) (i64.const 3))
  (i64.store align=1 (i32.const 0) (i64.const 4))
  (i64.store         (i32.const 0) (i64.const 5))
 )
)
(module
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $i32_i32_=>_none (func (param i32 i32)))

 ;; CHECK:      (global $f (mut i32) (i32.const -1412567121))
 (global $f (mut i64) (i64.const 0x12345678ABCDEFAF))
 ;; CHECK:      (global $g i32 (i32.const -1412567121))
 (global $g i64 (i64.const 0x12345678ABCDEFAF))
 ;; CHECK:      (global $h (mut i32) (global.get $g))
 (global $h (mut i64) (global.get $g))
 ;; CHECK:      (global $f$hi (mut i32) (i32.const 305419896))

 ;; CHECK:      (global $g$hi i32 (i32.const 305419896))

 ;; CHECK:      (global $h$hi (mut i32) (global.get $g$hi))

 ;; CHECK:      (global $i64toi32_i32$HIGH_BITS (mut i32) (i32.const 0))

 ;; CHECK:      (export "exp" (func $1))

 ;; CHECK:      (export "unreach" (func $2))

 ;; CHECK:      (func $call (type $i32_i32_=>_none) (param $0 i32) (param $0$hi i32)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $call (param i64))
 (func "exp"
  (call $call (global.get $f))
  (global.set $f (i64.const 0x1122334455667788))
 )
 (func "unreach"
  (global.set $f
   (block $label$1 (result i64)
    (unreachable)
   )
  )
 )
)
;; CHECK:      (func $1 (type $none_=>_none)
;; CHECK-NEXT:  (local $0 i32)
;; CHECK-NEXT:  (local $0$hi i32)
;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (local.set $0
;; CHECK-NEXT:    (block (result i32)
;; CHECK-NEXT:     (local.set $i64toi32_i32$0
;; CHECK-NEXT:      (global.get $f$hi)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (global.get $f)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.set $0$hi
;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (call $call
;; CHECK-NEXT:   (block (result i32)
;; CHECK-NEXT:    (local.set $i64toi32_i32$0
;; CHECK-NEXT:     (local.get $0$hi)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $i64toi32_i32$0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (global.set $f
;; CHECK-NEXT:    (block (result i32)
;; CHECK-NEXT:     (local.set $i64toi32_i32$0
;; CHECK-NEXT:      (i32.const 287454020)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (i32.const 1432778632)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (global.set $f$hi
;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $2 (type $none_=>_none)
;; CHECK-NEXT:  (local $0 i32)
;; CHECK-NEXT:  (local $0$hi i32)
;; CHECK-NEXT:  (local $1 i32)
;; CHECK-NEXT:  (local $1$hi i32)
;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
;; CHECK-NEXT:  (block $label$1
;; CHECK-NEXT:   (unreachable)
;; CHECK-NEXT:   (unreachable)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (local.set $1
;; CHECK-NEXT:    (block (result i32)
;; CHECK-NEXT:     (local.set $i64toi32_i32$0
;; CHECK-NEXT:      (local.get $0$hi)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (local.get $0)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.set $1$hi
;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (global.set $f
;; CHECK-NEXT:    (block (result i32)
;; CHECK-NEXT:     (local.set $i64toi32_i32$0
;; CHECK-NEXT:      (local.get $1$hi)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (local.get $1)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (global.set $f$hi
;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
(module
 ;; CHECK:      (type $i32_i32_=>_none (func (param i32 i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (global $f (mut i32) (i32.const -1412567121))
 (global $f (mut i64) (i64.const 0x12345678ABCDEFAF))
 ;; CHECK:      (global $f$hi (mut i32) (i32.const 305419896))

 ;; CHECK:      (global $i64toi32_i32$HIGH_BITS (mut i32) (i32.const 0))

 ;; CHECK:      (export "exp" (func $1))

 ;; CHECK:      (func $call (type $i32_i32_=>_none) (param $0 i32) (param $0$hi i32)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $call (param i64))
 (func "exp"
  (call $call (global.get $f))
  (global.set $f (i64.const 0x1122334455667788))
 )
)
;; CHECK:      (func $1 (type $none_=>_none)
;; CHECK-NEXT:  (local $0 i32)
;; CHECK-NEXT:  (local $0$hi i32)
;; CHECK-NEXT:  (local $i64toi32_i32$0 i32)
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (local.set $0
;; CHECK-NEXT:    (block (result i32)
;; CHECK-NEXT:     (local.set $i64toi32_i32$0
;; CHECK-NEXT:      (global.get $f$hi)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (global.get $f)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.set $0$hi
;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (call $call
;; CHECK-NEXT:   (block (result i32)
;; CHECK-NEXT:    (local.set $i64toi32_i32$0
;; CHECK-NEXT:     (local.get $0$hi)
;; CHECK-NEXT:    )
;; CHECK-NEXT:    (local.get $0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (local.get $i64toi32_i32$0)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (block
;; CHECK-NEXT:   (global.set $f
;; CHECK-NEXT:    (block (result i32)
;; CHECK-NEXT:     (local.set $i64toi32_i32$0
;; CHECK-NEXT:      (i32.const 287454020)
;; CHECK-NEXT:     )
;; CHECK-NEXT:     (i32.const 1432778632)
;; CHECK-NEXT:    )
;; CHECK-NEXT:   )
;; CHECK-NEXT:   (global.set $f$hi
;; CHECK-NEXT:    (local.get $i64toi32_i32$0)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
(module
 (type $i64_f64_i32_=>_none (func (param i64 f64 i32)))
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (global $i64toi32_i32$HIGH_BITS (mut i32) (i32.const 0))

 ;; CHECK:      (table $0 37 funcref)
 (table $0 37 funcref)
 ;; CHECK:      (func $0 (type $none_=>_none)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT:  (block
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (f64.const 1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (i32.const -32768)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (drop
 ;; CHECK-NEXT:    (i32.const 20)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $0
  (call_indirect (type $i64_f64_i32_=>_none)
   (unreachable)
   (f64.const 1)
   (i32.const -32768)
   (i32.const 20)
  )
 )
)
