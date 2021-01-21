;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; NOTE: Assertions have been generated by update_lit_checks.py
;; RUN: wasm-opt %s --optimize-instructions --enable-bulk-memory -S -o - | filecheck %s

(module
  (memory 0)
  ;; CHECK:      (func $optimize-bulk-memory-copy (param $dst i32) (param $src i32) (param $sz i32)
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $sz)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store8
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (i32.load8_u
  ;; CHECK-NEXT:    (local.get $src)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store16 align=1
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (i32.load16_u align=1
  ;; CHECK-NEXT:    (local.get $src)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (i32.const 3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i32.store align=1
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (i32.load align=1
  ;; CHECK-NEXT:    (local.get $src)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (i32.const 5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (i32.const 6)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (i64.store align=1
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (i64.load align=1
  ;; CHECK-NEXT:    (local.get $src)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (i32.const 16)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (local.get $dst)
  ;; CHECK-NEXT:   (local.get $src)
  ;; CHECK-NEXT:   (local.get $sz)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (memory.copy
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.load
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $optimize-bulk-memory-copy (param $dst i32) (param $src i32) (param $sz i32)
    (memory.copy  ;; skip
      (local.get $dst)
      (local.get $dst)
      (local.get $sz)
    )

    (memory.copy  ;; skip
      (local.get $dst)
      (local.get $src)
      (i32.const 0)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 1)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 2)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 3)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 4)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 5)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 6)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 7)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 8)
    )

    (memory.copy
      (local.get $dst)
      (local.get $src)
      (i32.const 16)
    )

    (memory.copy  ;; skip
      (local.get $dst)
      (local.get $src)
      (local.get $sz)
    )

    (memory.copy  ;; skip
      (i32.const 0)
      (i32.const 0)
      (i32.load
        (i32.const 3) ;; side effect
      )
    )
  )
)
