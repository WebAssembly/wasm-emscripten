;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; NOTE: This test was ported using port_passes_tests_to_lit.py and could be cleaned up.

;; RUN: foreach %s %t wasm-opt -all --simplify-globals -S -o - | filecheck %s

;; $single-use has a single use, and we can fold that code into its use. That
;; global then becomes an unused import.
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  (global $single-use anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  ;; CHECK:      (import "unused" "unused" (global $single-use anyref))

  ;; CHECK:      (global $other anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $other anyref (global.get $single-use))
)

;; As above, but now there is a second use, so we do nothing.
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  ;; CHECK:      (global $single-use anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $single-use anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  ;; CHECK:      (global $other anyref (global.get $single-use))
  (global $other anyref (global.get $single-use))

  ;; CHECK:      (global $other2 anyref (global.get $single-use))
  (global $other2 anyref (global.get $single-use))
)

;; As the first testcase, but now there is a second use in function code, so
;; again we do nothing.
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (global $single-use anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $single-use anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  ;; CHECK:      (global $other anyref (global.get $single-use))
  (global $other anyref (global.get $single-use))

  ;; CHECK:      (func $user (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $single-use)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $user
    (drop
      (global.get $single-use)
    )
  )
)

;; As the first testcase, but now $single-use is imported, so there is no code
;; to fold.
(module
  (type $A (struct (field anyref)))

  ;; CHECK:      (import "a" "b" (global $single-use anyref))
  (import "a" "b" (global $single-use anyref))

  ;; CHECK:      (global $other anyref (global.get $single-use))
  (global $other anyref (global.get $single-use))
)

;; As the first testcase, but now there the single use is in function code, so
;; we do nothing (as it could be executed more than once).
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (global $single-use anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $single-use anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  ;; CHECK:      (func $user (type $1)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $single-use)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $user
    (drop
      (global.get $single-use)
    )
  )
)

;; As the first testcase, but now the single use is nested in other code. We
;; can still optimize.
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  (global $single-use anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  ;; CHECK:      (import "unused" "unused" (global $single-use anyref))

  ;; CHECK:      (global $other anyref (struct.new $A
  ;; CHECK-NEXT:  (struct.new $A
  ;; CHECK-NEXT:   (ref.i31
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $other anyref (struct.new $A
    (global.get $single-use)
  ))
)

;; Test multiple separate optimizations in one module.
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  (global $single-use1 anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  (global $single-use2 anyref (struct.new $A
    (i31.new
      (i32.const 1337)
    )
  ))

  ;; CHECK:      (import "unused" "unused" (global $single-use1 anyref))

  ;; CHECK:      (import "unused" "unused" (global $single-use2 anyref))

  ;; CHECK:      (import "unused" "unused" (global $single-use3 anyref))

  ;; CHECK:      (global $other1 anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $other1 anyref (global.get $single-use1))

  (global $single-use3 anyref (struct.new $A
    (i31.new
      (i32.const 99999)
    )
  ))

  ;; CHECK:      (global $other2 anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 1337)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $other2 anyref (global.get $single-use2))

  ;; CHECK:      (global $other3 anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 99999)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $other3 anyref (global.get $single-use3))
)

;; Test multiple related optimizations in one module.
(module
  ;; CHECK:      (type $A (struct (field anyref)))
  (type $A (struct (field anyref)))

  (global $single-use1 anyref (struct.new $A
    (i31.new
      (i32.const 42)
    )
  ))

  ;; CHECK:      (import "unused" "unused" (global $single-use1 anyref))

  ;; CHECK:      (global $other1 anyref (struct.new $A
  ;; CHECK-NEXT:  (ref.i31
  ;; CHECK-NEXT:   (i32.const 42)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: ))
  (global $other1 anyref (global.get $single-use1))

  ;; CHECK:      (global $other2 anyref (global.get $single-use1))
  (global $other2 anyref (global.get $other1))

  ;; CHECK:      (global $other3 anyref (global.get $single-use1))
  (global $other3 anyref (global.get $other2))
) ;; XXX we added more uses in the pass before us!
