;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-ctor-eval --ctors=test --kept-exports=test --quiet -all -S -o - | filecheck %s

(module
 ;; CHECK:      (type $A (struct (field (mut (ref null $A))) (field i32)))
 (type $A (struct (field (mut (ref null $A))) (field i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_3 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_3))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  ;; This generates a self-cycle where the global $a's ref field points to
  ;; itself. To handle this, wasm-ctor-eval will emit a new global with a null
  ;; in the ref field, and add a start function that adds the self-reference.
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $a)
  )
 )

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (struct.get $A 0
 ;; CHECK-NEXT:    (global.get $a)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  ;; Getting $A.0.1 (reading from the reference in the global's first field)
  ;; checks that we have a proper reference there. If we could do --fuzz-exec
  ;; here we could validate that (but atm we can't use --output=fuzz-exec at the
  ;; same time as --all-items in the update_lit_checks.py note).
  (struct.get $A 1
   (struct.get $A 0
    (global.get $a)
   )
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_3)
;; CHECK-NEXT:   (global.get $ctor-eval$global_3)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; As above, but with $A's fields reversed. This verifies we use the right
 ;; field index in the start function.

 ;; CHECK:      (type $A (struct (field i32) (field (mut (ref null $A)))))
 (type $A (struct (field i32) (field (mut (ref null $A)))))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_3 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_3))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (i32.const 42)
     (ref.null $A)
    )
   )
  )
  (struct.set $A 1
   (local.get $a)
   (local.get $a)
  )
 )

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 0
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $a)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (struct.get $A 0
   (struct.get $A 1
    (global.get $a)
   )
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 1
;; CHECK-NEXT:   (global.get $ctor-eval$global_3)
;; CHECK-NEXT:   (global.get $ctor-eval$global_3)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; A cycle between two globals.

 ;; CHECK:      (type $A (struct (field (mut (ref null $A))) (field i32)))
 (type $A (struct (field (mut (ref null $A))) (field i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_8 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_8))
 (global $a (mut (ref null $A)) (ref.null $A))

 ;; CHECK:      (global $ctor-eval$global_7 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_8)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $b (mut (ref null $A)) (global.get $ctor-eval$global_7))
 (global $b (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $A))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (struct.new $A
     (global.get $a)  ;; $b can refer to $a since we've created $a already.
     (i32.const 1337)
    )
   )
  )
  ;; $a needs a set to allow us to create the cycle.
  (struct.set $A 0
   (local.get $a)
   (local.get $b)
  )
 )

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (i32.add
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $a)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $b)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (i32.add
   (struct.get $A 1
    (global.get $a)
   )
   (struct.get $A 1
    (global.get $b)
   )
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_8)
;; CHECK-NEXT:   (global.get $ctor-eval$global_7)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; A cycle between two globals of different types. One of them has an
 ;; immutable field in the cycle.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $B))) (field i32)))
  (type $A (struct (field (mut (ref null $B))) (field i32)))

  ;; CHECK:       (type $B (struct (field (ref null $A)) (field i32)))
  (type $B (struct (field (ref null $A)) (field i32)))
 )

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_8 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_8))
 (global $a (mut (ref null $A)) (ref.null $A))

 ;; CHECK:      (global $ctor-eval$global_7 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_8)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $b (mut (ref null $B)) (global.get $ctor-eval$global_7))
 (global $b (mut (ref null $B)) (ref.null $B))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $B))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (struct.new $B
     (global.get $a)
     (i32.const 1337)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $b)
  )
 )

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (i32.add
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $a)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (struct.get $B 1
 ;; CHECK-NEXT:    (global.get $b)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (i32.add
   (struct.get $A 1
    (global.get $a)
   )
   (struct.get $B 1
    (global.get $b)
   )
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_8)
;; CHECK-NEXT:   (global.get $ctor-eval$global_7)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; As above, but with the order of globals reversed.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $B))) (field i32)))
  (type $A (struct (field (mut (ref null $B))) (field i32)))

  ;; CHECK:       (type $B (struct (field (ref null $A)) (field i32)))
  (type $B (struct (field (ref null $A)) (field i32)))
 )


 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_8 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_8))

 ;; CHECK:      (global $ctor-eval$global_7 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_8)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $b (mut (ref null $B)) (global.get $ctor-eval$global_7))
 (global $b (mut (ref null $B)) (ref.null $B))

 (global $a (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $B))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (struct.new $B
     (global.get $a)
     (i32.const 1337)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $b)
  )
 )

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (i32.add
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $a)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (struct.get $B 1
 ;; CHECK-NEXT:    (global.get $b)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (i32.add
   (struct.get $A 1
    (global.get $a)
   )
   (struct.get $B 1
    (global.get $b)
   )
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_8)
;; CHECK-NEXT:   (global.get $ctor-eval$global_7)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
  ;; A cycle as above, but with non-nullability rather than immutability.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $B))) (field i32)))
  (type $A (struct (field (mut (ref null $B))) (field i32)))

  ;; CHECK:       (type $B (struct (field (mut (ref $A))) (field i32)))
  (type $B (struct (field (mut (ref $A))) (field i32)))
 )

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_8 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_8))
 (global $a (mut (ref null $A)) (ref.null $A))

 (global $b (mut (ref null $B)) (ref.null $B))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $B))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (struct.new $B
     (local.get $a)
     (i32.const 1337)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $b)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_7 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_8)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_8)
;; CHECK-NEXT:   (global.get $ctor-eval$global_7)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
  ;; A cycle as above, but with globals in reverse order and with both non-
  ;; nullability and immutability.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $B))) (field i32)))
  (type $A (struct (field (mut (ref null $B))) (field i32)))

  ;; CHECK:       (type $B (struct (field (ref $A)) (field i32)))
  (type $B (struct (field (ref $A)) (field i32)))
 )


 (global $b (mut (ref null $B)) (ref.null $B))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_8 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_8))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $B))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (struct.new $B
     (local.get $a)
     (i32.const 1337)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $b)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_7 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_8)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_8)
;; CHECK-NEXT:   (global.get $ctor-eval$global_7)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; A cycle between three globals.

 ;; CHECK:      (type $A (struct (field (mut (ref null $A))) (field i32)))
 (type $A (struct (field (mut (ref null $A))) (field i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_13 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_14 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_14))
 (global $a (mut (ref null $A)) (ref.null $A))

 (global $b (mut (ref null $A)) (ref.null $A))

 (global $c (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $A))
  (local $c (ref $A))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (struct.new $A
     (global.get $a)
     (i32.const 1337)
    )
   )
  )
  (global.set $c
   (local.tee $c
    (struct.new $A
     (global.get $b)
     (i32.const 99999)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $c)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_12 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_13)
 ;; CHECK-NEXT:  (i32.const 99999)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_13)
;; CHECK-NEXT:   (global.get $ctor-eval$global_14)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_14)
;; CHECK-NEXT:   (global.get $ctor-eval$global_12)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $A))
;; CHECK-NEXT:  (local $c (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; A cycle between three globals as above, but now using different types and
 ;; also both structs and arrays. Also reverse the order of globals, make
 ;; one array immutable and one non-nullable, and make one array refer to the
 ;; other two.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $C))) (field i32)))
  (type $A (struct (field (mut (ref null $C))) (field i32)))
  ;; CHECK:       (type $B (array (ref null $A)))
  (type $B (array (ref null $A)))
  ;; CHECK:       (type $C (array (mut (ref any))))
  (type $C (array (mut (ref any))))
 )

 (global $c (mut (ref null $C)) (ref.null $C))

 (global $b (mut (ref null $B)) (ref.null $B))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_14 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_13 (ref $B) (array.new_fixed $B 10
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_14))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $B))
  (local $c (ref $C))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $C)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (array.new $B
     (global.get $a)
     (i32.const 10)
    )
   )
  )
  (global.set $c
   (local.tee $c
    (array.new_fixed $C
     (local.get $b)
     (local.get $a)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (global.get $c)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_12 (ref $C) (array.new_fixed $C 2
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_13)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_14)
;; CHECK-NEXT:   (global.get $ctor-eval$global_12)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (local $c (ref $C))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; As above but with the order of globals reversed once more.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $C))) (field i32)))
  (type $A (struct (field (mut (ref null $C))) (field i32)))
  ;; CHECK:       (type $B (array (ref null $A)))
  (type $B (array (ref null $A)))
  ;; CHECK:       (type $C (array (mut (ref any))))
  (type $C (array (mut (ref any))))
 )

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_14 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_13 (ref $B) (array.new_fixed $B 10
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_14))
 (global $a (mut (ref null $A)) (ref.null $A))

 (global $b (mut (ref null $B)) (ref.null $B))

 (global $c (mut (ref null $C)) (ref.null $C))

 (func $test (export "test")
  (local $a (ref $A))
  (local $b (ref $B))
  (local $c (ref $C))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $C)
     (i32.const 42)
    )
   )
  )
  (global.set $b
   (local.tee $b
    (array.new $B
     (global.get $a)
     (i32.const 10)
    )
   )
  )
  (global.set $c
   (local.tee $c
    (array.new_fixed $C
     (local.get $b)
     (local.get $a)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (global.get $c)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_12 (ref $C) (array.new_fixed $C 2
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_13)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_14)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_14)
;; CHECK-NEXT:   (global.get $ctor-eval$global_12)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (local $c (ref $C))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; A cycle between two globals, where some of the fields participate in the
 ;; cycle and some do not.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (mut (ref null $B))) (field (mut (ref null $B))) (field (mut (ref null $B)))))
  (type $A (struct (field (mut (ref null $B))) (field (mut (ref null $B))) (field (mut (ref null $B)))))
  ;; CHECK:       (type $B (array (ref null $A)))
  (type $B (array (ref null $A)))
 )

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_anyref (func (result anyref)))

 ;; CHECK:      (global $ctor-eval$global_16 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_16))
 (global $a (mut (ref null $A)) (ref.null $A))
 (global $b (mut (ref null $B)) (ref.null $B))

 (func $test (export "test")
  (local $a (ref $A))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (array.new_default $B
      (i32.const 0)
     )
     (ref.null $B)
     (array.new_default $B
      (i32.const 0)
     )
    )
   )
  )
  (global.set $b
   (array.new_fixed $B
    (struct.new_default $A)
    (global.get $a)
    (struct.new_default $A)
   )
  )
  (struct.set $A 1
   (local.get $a)
   (global.get $b)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_19 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_15 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_14 (ref $B) (array.new_fixed $B 3
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_15)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_16)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_19)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_17 (ref $B) (array.new_fixed $B 0))

 ;; CHECK:      (global $ctor-eval$global_18 (ref $B) (array.new_fixed $B 0))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_anyref) (result anyref)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result (ref null any))
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_16)
;; CHECK-NEXT:   (global.get $ctor-eval$global_17)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (struct.set $A 1
;; CHECK-NEXT:   (global.get $ctor-eval$global_16)
;; CHECK-NEXT:   (global.get $ctor-eval$global_14)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (struct.set $A 2
;; CHECK-NEXT:   (global.get $ctor-eval$global_16)
;; CHECK-NEXT:   (global.get $ctor-eval$global_18)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; As above, with the cycle creation logic reversed.

 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (struct (field (ref null $B)) (field (ref null $B)) (field (ref null $B))))
  (type $A (struct (field (ref null $B)) (field (ref null $B)) (field (ref null $B))))
  ;; CHECK:       (type $B (array (mut (ref null $A))))
  (type $B (array (mut (ref null $A))))
 )

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_anyref (func (result anyref)))

 ;; CHECK:      (global $ctor-eval$global_16 (ref $B) (array.new_fixed $B 3
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_19 (ref $B) (array.new_fixed $B 0))

 ;; CHECK:      (global $ctor-eval$global_15 (ref $B) (array.new_fixed $B 0))

 ;; CHECK:      (global $ctor-eval$global_14 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_15)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_16)
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_19)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_14))
 (global $a (mut (ref null $A)) (ref.null $A))
 (global $b (mut (ref null $B)) (ref.null $B))

 (func $test (export "test")
  (local $b (ref $B))
  (global.set $b
   (local.tee $b
    (array.new_fixed $B
     (struct.new_default $A)
     (ref.null $A)
     (struct.new_default $A)
    )
   )
  )
  (global.set $a
   (struct.new $A
    (array.new_default $B
     (i32.const 0)
    )
    (global.get $b)
    (array.new_default $B
     (i32.const 0)
    )
   )
  )
  (array.set $B
   (local.get $b)
   (i32.const 1)
   (global.get $a)
  )
 )

 ;; CHECK:      (global $ctor-eval$global_17 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_18 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)

 ;; CHECK:      (func $keepalive (type $none_=>_anyref) (result anyref)
 ;; CHECK-NEXT:  (struct.get $A 1
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result (ref null any))
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (array.set $B
;; CHECK-NEXT:   (global.get $ctor-eval$global_16)
;; CHECK-NEXT:   (i32.const 0)
;; CHECK-NEXT:   (global.get $ctor-eval$global_17)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (array.set $B
;; CHECK-NEXT:   (global.get $ctor-eval$global_16)
;; CHECK-NEXT:   (i32.const 1)
;; CHECK-NEXT:   (global.get $ctor-eval$global_14)
;; CHECK-NEXT:  )
;; CHECK-NEXT:  (array.set $B
;; CHECK-NEXT:   (global.get $ctor-eval$global_16)
;; CHECK-NEXT:   (i32.const 2)
;; CHECK-NEXT:   (global.get $ctor-eval$global_18)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; The start function already exists here. We must prepend to it.

 ;; CHECK:      (type $A (struct (field (mut (ref null $A))) (field i32)))
 (type $A (struct (field (mut (ref null $A))) (field i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_4 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_4))
 (global $a (mut (ref null $A)) (ref.null $A))

 ;; CHECK:      (global $b (mut (ref null $A)) (ref.null none))
 (global $b (mut (ref null $A)) (ref.null $A))

 ;; CHECK:      (export "test" (func $test_3))

 ;; CHECK:      (export "keepalive" (func $keepalive))

 ;; CHECK:      (start $start)
 (start $start)

 (func $test (export "test")
  (local $a (ref $A))
  (global.set $a
   (local.tee $a
    (struct.new $A
     (ref.null $A)
     (i32.const 42)
    )
   )
  )
  (struct.set $A 0
   (local.get $a)
   (local.get $a)
  )
 )

 ;; CHECK:      (func $keepalive (type $none_=>_i32) (result i32)
 ;; CHECK-NEXT:  (i32.add
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $a)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (struct.get $A 1
 ;; CHECK-NEXT:    (global.get $b)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $keepalive (export "keepalive") (result i32)
  (i32.add
   (struct.get $A 1
    (global.get $a)
   )
   (struct.get $A 1
    (global.get $b)
   )
  )
 )

 ;; CHECK:      (func $start (type $none_=>_none)
 ;; CHECK-NEXT:  (struct.set $A 0
 ;; CHECK-NEXT:   (global.get $ctor-eval$global_4)
 ;; CHECK-NEXT:   (global.get $ctor-eval$global_4)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (global.set $b
 ;; CHECK-NEXT:   (global.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $start
  (global.set $b
   (global.get $a)
  )
 )
)

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; CHECK:      (type $A (struct (field (mut (ref null $A)))))
 (type $A (struct (field (mut (ref null $A)))))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $anyref_=>_none (func (param anyref)))

 ;; CHECK:      (import "a" "b" (func $import (type $anyref_=>_none) (param anyref)))
 (import "a" "b" (func $import (param anyref)))

 (func $test (export "test")
  (local $a (ref $A))
  (struct.set $A 0
   (local.tee $a
    (struct.new_default $A)
   )
   (local.get $a)
  )
  ;; The previous instructions created a cycle, which we now send to an import.
  ;; The import will block us from evalling the entire function, and we will
  ;; only partially eval it, removing the statements before the call. Note that
  ;; the cycle only exists in local state - there is no global it is copied to -
  ;; and so this test verifies that we handle cycles in local state.
  (call $import
   (local.get $a)
  )
 )
)
;; CHECK:      (global $ctor-eval$global (ref $A) (struct.new $A
;; CHECK-NEXT:  (ref.null none)
;; CHECK-NEXT: ))

;; CHECK:      (export "test" (func $test_3))

;; CHECK:      (start $start)

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global)
;; CHECK-NEXT:   (global.get $ctor-eval$global)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $test_3 (type $none_=>_none)
;; CHECK-NEXT:  (call $import
;; CHECK-NEXT:   (global.get $ctor-eval$global)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )
