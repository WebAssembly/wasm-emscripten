;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --outlining -S -o - | filecheck %s

;; TODO: Add a test that creates an outlined function with one return value
;; TODO: Add a test that creates an outlined function that returns multiple values
;; TODO: Add a test that fails to outline a single control flow that repeats


(module
  ;; CHECK:      (type $0 (func (result i32)))

  ;; CHECK:      (type $1 (func (param i32)))

  ;; CHECK:      (func $outline$ (param $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )

  ;; CHECK:      (func $a (result i32)
  ;; CHECK-NEXT:  (call $outline$
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a (result i32)
    (drop
      (i32.const 7)
    )
    (drop
      (i32.const 1)
    )
    (drop
      (i32.const 2)
    )
    (return
      (i32.const 4)
    )
  )
  ;; CHECK:      (func $b (result i32)
  ;; CHECK-NEXT:  (call $outline$
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (i32.const 5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b (result i32)
    (drop
      (i32.const 0)
    )
    (drop
      (i32.const 1)
    )
    (drop
      (i32.const 2)
    )
    (return
      (i32.const 5)
    )
  )
)

;; Tests that outlining occurs properly when the sequence is at the end of a function.
(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (func $outline$
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )

  ;; CHECK:      (func $a
  ;; CHECK-NEXT:  (call $outline$)
  ;; CHECK-NEXT: )
  (func $a
    (drop
      (i32.const 1)
    )
    (drop
      (i32.const 2)
    )
  )
  ;; CHECK:      (func $b
  ;; CHECK-NEXT:  (call $outline$)
  ;; CHECK-NEXT: )
  (func $b
    (drop
      (i32.const 1)
    )
    (drop
      (i32.const 2)
    )
  )
)

;; Tests that outlining occurs properly when the sequence is at the beginning of a function.
;; Also tests that the outlined function has no arguments.
(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (func $outline$
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )

  ;; CHECK:      (func $a
  ;; CHECK-NEXT:  (call $outline$)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 6)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a
    (drop
      (i32.const 0)
    )
    (drop
      (i32.add
        (i32.const 0)
        (i32.const 1)
      )
    )
    (drop
      (i32.const 1)
    )
    (drop
      (i32.const 6)
    )
  )
  ;; CHECK:      (func $b
  ;; CHECK-NEXT:  (call $outline$)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b
    (drop
      (i32.const 0)
    )
    (drop
      (i32.add
        (i32.const 0)
        (i32.const 1)
      )
    )
    (drop
      (i32.const 1)
    )
    (drop
      (i32.const 7)
    )
  )
)

;; Tests multiple sequences being outlined from the same source function into different
;; outlined functions.
(module
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.sub
  ;; CHECK-NEXT:    (i32.const 3)
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )

  ;; CHECK:      (func $a
  ;; CHECK-NEXT:  (call $outline$)
  ;; CHECK-NEXT:  (call $outline$_4)
  ;; CHECK-NEXT: )
  (func $a
    (drop
      (i32.add
        (i32.const 0)
        (i32.const 1)
      )
    )
    (drop
      (i32.sub
        (i32.const 3)
        (i32.const 4)
      )
    )
  )
  ;; CHECK:      (func $b
  ;; CHECK-NEXT:  (call $outline$_4)
  ;; CHECK-NEXT: )
  (func $b
    (drop
      (i32.sub
        (i32.const 3)
        (i32.const 4)
      )
    )
  )
  ;; CHECK:      (func $c
  ;; CHECK-NEXT:  (call $outline$)
  ;; CHECK-NEXT: )
  (func $c
    (drop
      (i32.add
        (i32.const 0)
        (i32.const 1)
      )
    )
  )
)

;; Tests that outlining works correctly with If control flow
(module
  ;; CHECK:      (type $0 (func (param i32)))

  ;; CHECK:      (type $1 (func))

  ;; CHECK:      (func $outline$
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 10)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )

  ;; CHECK:      (func $a (param $0 i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.eqz
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $outline$)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a (param i32)
    (if
      (i32.eqz
        (local.get 0)
      )
      (drop
        (i32.const 10)
      )
    )
  )
  ;; CHECK:      (func $b (param $0 i32)
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.eqz
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $outline$)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b (param i32)
    (if
      (i32.eqz
        (local.get 0)
      )
      (drop
        (i32.const 10)
      )
    )
  )
)

;; Tests that local.get instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func (param i32)))

  ;; CHECK:      (func $j (param $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $j (param i32)
    (drop (i32.add
      (local.get 0)
      (i32.const 1)))
  )
  ;; CHECK:      (func $k (param $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $k (param i32)
    (drop (i32.add
      (local.get 0)
      (i32.const 1)))
  )
)

;; Tests local.set instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (func $l
  ;; CHECK-NEXT:  (local $i i32)
  ;; CHECK-NEXT:  (local.set $i
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $l
    (local $i i32)
    (local.set $i
      (i32.const 7))
  )
  ;; CHECK:      (func $m
  ;; CHECK-NEXT:  (local $i i32)
  ;; CHECK-NEXT:  (local.set $i
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $m
    (local $i i32)
    (local.set $i
      (i32.const 7))
  )
)

;; Tests branch instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (func $n
  ;; CHECK-NEXT:  (block $label1
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $label1)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $label1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $n
    (block $label1
      (drop (i32.const 4))
      (br $label1)
      (drop (i32.const 4))
      (br $label1)
    )
  )
)

;; Tests that local.get instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func (param i32)))

  ;; CHECK:      (func $a (param $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a (param i32)
    (drop
      (i32.add
        (local.get 0)
        (i32.const 1)
      )
    )
  )
  ;; CHECK:      (func $b (param $0 i32)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.add
  ;; CHECK-NEXT:    (local.get $0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b (param i32)
    (drop
      (i32.add
        (local.get 0)
        (i32.const 1)
      )
    )
  )
)

;; Tests local.set instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (func $a
  ;; CHECK-NEXT:  (local $i i32)
  ;; CHECK-NEXT:  (local.set $i
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a
    (local $i i32)
    (local.set $i
      (i32.const 7)
    )
  )
  ;; CHECK:      (func $b
  ;; CHECK-NEXT:  (local $i i32)
  ;; CHECK-NEXT:  (local.set $i
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b
    (local $i i32)
    (local.set $i
      (i32.const 7)
    )
  )
)

;; Tests branch instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (func $a
  ;; CHECK-NEXT:  (block $label1
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $label1)
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (i32.const 4)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br $label1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a
    (block $label1
      (drop
        (i32.const 4)
      )
      (br $label1)
      (drop
        (i32.const 4)
      )
      (br $label1)
    )
  )
)

;; Tests return instructions are correctly filtered from being outlined.
(module
  ;; CHECK:      (type $0 (func (result i32)))

  ;; CHECK:      (func $a (result i32)
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $a (result i32)
    (return
      (i32.const 2)
    )
  )
  ;; CHECK:      (func $b (result i32)
  ;; CHECK-NEXT:  (return
  ;; CHECK-NEXT:   (i32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $b (result i32)
    (return
      (i32.const 2)
    )
  )
)
