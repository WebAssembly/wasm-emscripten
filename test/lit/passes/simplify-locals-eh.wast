;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --simplify-locals -all -S -o - | filecheck %s

(module
  ;; CHECK:      (tag $e-i32 (param i32))
  (tag $e-i32 (param i32))

  ;; CHECK:      (func $bar (type $1) (result i32)
  ;; CHECK-NEXT:  (i32.const 3)
  ;; CHECK-NEXT: )
  (func $bar (result i32) (i32.const 3))

  ;; CHECK:      (func $call-cannot-be-sinked-into-try (type $2)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (local.set $0
  ;; CHECK-NEXT:   (call $bar)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block $tryend
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (block $catch (result i32)
  ;; CHECK-NEXT:     (try_table (catch $e-i32 $catch)
  ;; CHECK-NEXT:      (drop
  ;; CHECK-NEXT:       (local.get $0)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (br $tryend)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $call-cannot-be-sinked-into-try_table (local $0 i32)
    (drop
      ;; This local.tee should NOT be sinked into 'try_table' below, because it
      ;; may throw
      (local.tee $0 (call $bar))
    )
    (block $tryend
      (drop
        (block $catch (result i32)
          (try_table (catch $e-i32 $catch)
            (drop (local.get $0))
          )
          (br $tryend)
        )
      )
    )
  )

  ;; CHECK:      (func $non-call-can-be-sinked-into-try (type $2)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (block $tryend
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (block $catch (result i32)
  ;; CHECK-NEXT:     (try_table (catch $e-i32 $catch)
  ;; CHECK-NEXT:      (drop
  ;; CHECK-NEXT:       (i32.const 3)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (br $tryend)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $non-call-can-be-sinked-into-try_table (local $0 i32)
    (drop
      ;; This local.tee can be sinked into 'try_table' below, because it cannot
      ;; throw
      (local.tee $0 (i32.const 3))
    )
    (block $tryend
      (drop
        (block $catch (result i32)
          (try_table (catch $e-i32 $catch)
            (drop (local.get $0))
          )
          (br $tryend)
        )
      )
    )
  )

  ;; CHECK:      (func $return-call-can-be-sinked-into-try (type $1) (result i32)
  ;; CHECK-NEXT:  (local $0 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (block $tryend (result i32)
  ;; CHECK-NEXT:   (try_table (result i32) (catch $e-i32 $tryend)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (if (result i32)
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:      (then
  ;; CHECK-NEXT:       (return_call $return-call-can-be-sinked-into-try)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (else
  ;; CHECK-NEXT:       (i32.const 1)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $return-call-can-be-sinked-into-try_table (result i32)
    (local $0 i32)
    (drop
      ;; This cannot throw either, so it can be sunk. Wrap the return_call in an
      ;; if so the whole expression does not return unconditionally.
      (local.tee $0
        (if (result i32)
          (i32.const 0)
          (then
            (return_call $return-call-can-be-sinked-into-try_table)
          )
          (else
            (i32.const 1)
          )
        )
      )
    )
    (block $tryend (result i32)
      (try_table (result i32) (catch $e-i32 $tryend)
        (drop (local.get $0))
        (i32.const 0)
      )
    )
  )

  ;; CHECK:      (func $equivalent-set-removal-call (type $0) (param $0 i32)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (call $equivalent-set-removal-call
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $equivalent-set-removal-call (param $0 i32)
    (local $1 i32)
    (local.set $1 (local.get $0))
    (drop (local.get $0))
    (drop (local.get $1))
    ;; Even with EH enabled we can look past the call and optimize the final 1
    ;; to a 0, since they contain the same (and while the call might branch,
    ;; such a branch does not cause a problem here, as if we branch we just
    ;; don't reach the change later down).
    (call $equivalent-set-removal-call
      (i32.const 2)
    )
    (drop (local.get $0))
    (drop (local.get $1))
  )

  ;; CHECK:      (func $equivalent-set-removal-if (type $3) (param $p i32) (param $0 i32)
  ;; CHECK-NEXT:  (local $1 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.set $1
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (local.get $p)
  ;; CHECK-NEXT:   (then
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (else
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $0)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $1)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $equivalent-set-removal-if (param $p i32) (param $0 i32)
    (local $1 i32)
    (local.set $1 (local.get $0))
    (drop (local.get $0))
    ;; This local.get of 1 can be of 0.
    (drop (local.get $1))
    (if
      (local.get $p)
      (then
        (block
          ;; We also optimize in this block, which is adjacent to the code before
          ;; us. It is valid to optimize the 1 to a 0 here, as it is dominated by
          ;; the code earlier.
          (drop (local.get $0))
          (drop (local.get $1))
        )
      )
      (else
        (block
          ;; We could also optimize here, but atm just look at code adjacent to
          ;; its dominator. TODO
          (drop (local.get $0))
          (drop (local.get $1))
        )
      )
    )
    ;; As in the else, this could be optimized. TODO
    (drop (local.get $0))
    (drop (local.get $1))
  )
)
