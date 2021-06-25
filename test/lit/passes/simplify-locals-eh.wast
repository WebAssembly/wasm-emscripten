;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --simplify-locals -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $none_=>_none (func))
  ;; CHECK:      (type $i32_=>_none (func (param i32)))
  ;; CHECK:      (type $i32_i32_=>_none (func (param i32 i32)))
  ;; CHECK:      (type $none_=>_i32 (func (result i32)))
  ;; CHECK:      (tag $e-i32 (param i32))
  (tag $e-i32 (param i32))
  ;; CHECK:      (func $foo (param $0 i32) (param $1 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo (param i32 i32))
  ;; CHECK:      (func $pop-cannot-be-sinked
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch $e-i32
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (pop i32)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (call $foo
  ;; CHECK-NEXT:     (i32.const 3)
  ;; CHECK-NEXT:     (local.get $0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $pop-cannot-be-sinked (local $0 i32)
    (try
      (do)
      (catch $e-i32
        ;; This (local.set $0) of (pop i32) cannot be sunk to (local.get $0)
        ;; below, because the pop should follow right after 'catch'.
        (local.set $0 (pop i32))
        (call $foo
          (i32.const 3)
          (local.get $0)
        )
      )
    )
  )

  ;; CHECK:      (func $pop-within-catch-can-be-sinked
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch_all
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:    (call $foo
  ;; CHECK-NEXT:     (i32.const 3)
  ;; CHECK-NEXT:     (try $try0 (result i32)
  ;; CHECK-NEXT:      (do
  ;; CHECK-NEXT:       (i32.const 0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (catch $e-i32
  ;; CHECK-NEXT:       (pop i32)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $pop-within-catch-can-be-sinked (local $0 i32)
    (try
      (do)
      (catch_all
        ;; This whole 'try' body can be sinked to eliminate local.set /
        ;; local.get. Even though it contains a pop, it is enclosed within
        ;; try-catch, so it is OK.
        (local.set $0
          (try (result i32)
            (do (i32.const 0))
            (catch $e-i32 (pop i32))
          )
        )
        (call $foo
          (i32.const 3)
          (local.get $0)
        )
      )
    )
  )

  ;; CHECK:      (func $bar (result i32)
  ;; CHECK-NEXT:  (i32.const 3)
  ;; CHECK-NEXT: )
  (func $bar (result i32) (i32.const 3))
  ;; CHECK:      (func $call-cannot-be-sinked-into-try
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (call $bar)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch $e-i32
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (pop i32)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-cannot-be-sinked-into-try (local $0 i32)
    (drop
      ;; This local.tee should NOT be sinked into 'try' below, because it may
      ;; throw
      (local.tee $0 (call $bar))
    )
    (try
      (do
        (drop (local.get $0))
      )
      (catch $e-i32
        (drop (pop i32))
      )
    )
  )

  ;; CHECK:      (func $non-call-can-be-sinked-into-try
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (i32.const 3)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch $e-i32
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (pop i32)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $non-call-can-be-sinked-into-try (local $0 i32)
    (drop
      ;; This local.tee can be sinked into 'try' below, because it cannot throw
      (local.tee $0 (i32.const 3))
    )
    (try
      (do
        (drop (local.get $0))
      )
      (catch $e-i32
        (drop (pop i32))
      )
    )
  )
)
