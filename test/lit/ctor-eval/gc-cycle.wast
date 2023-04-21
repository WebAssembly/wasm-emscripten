;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-ctor-eval --ctors=test --kept-exports=test --quiet -all -S -o - | filecheck %s

(module
 ;; CHECK:      (type $A (struct (field (mut (ref null $A))) (field i32)))
 (type $A (struct (field (mut (ref null $A))) (field i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_2 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_2))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func "test"
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

 (func "keepalive" (result i32)
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

;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 1
;; CHECK-NEXT:   (struct.get $A 0
;; CHECK-NEXT:    (global.get $a)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_2)
;; CHECK-NEXT:   (global.get $ctor-eval$global_2)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
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

 ;; CHECK:      (global $ctor-eval$global_2 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_2))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func "test"
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

 (func "keepalive" (result i32)
  (struct.get $A 0
   (struct.get $A 1
    (global.get $a)
   )
  )
 )
)

;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 0
;; CHECK-NEXT:   (struct.get $A 1
;; CHECK-NEXT:    (global.get $a)
;; CHECK-NEXT:   )
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 1
;; CHECK-NEXT:   (global.get $ctor-eval$global_2)
;; CHECK-NEXT:   (global.get $ctor-eval$global_2)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
 ;; A cycle between two globals.

 ;; CHECK:      (type $A (struct (field (mut (ref null $A))) (field i32)))
 (type $A (struct (field (mut (ref null $A))) (field i32)))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_5 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_5))
 (global $a (mut (ref null $A)) (ref.null $A))

 (global $b (mut (ref null $A)) (ref.null $A))

 (func "test"
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

 (func "keepalive" (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (global $ctor-eval$global_6 (ref $A) (struct.new $A
;; CHECK-NEXT:  (global.get $ctor-eval$global_5)
;; CHECK-NEXT:  (i32.const 1337)
;; CHECK-NEXT: ))

;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 1
;; CHECK-NEXT:   (global.get $a)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_5)
;; CHECK-NEXT:   (global.get $ctor-eval$global_6)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
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

 ;; CHECK:      (global $ctor-eval$global_5 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_6 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_5)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_5))
 (global $a (mut (ref null $A)) (ref.null $A))

 (global $b (mut (ref null $B)) (ref.null $B))

 (func "test"
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

 (func "keepalive" (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 1
;; CHECK-NEXT:   (global.get $a)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_5)
;; CHECK-NEXT:   (global.get $ctor-eval$global_6)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
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


 (global $b (mut (ref null $B)) (ref.null $B))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $ctor-eval$global_5 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_6 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_5)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_5))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func "test"
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

 (func "keepalive" (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 1
;; CHECK-NEXT:   (global.get $a)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_5)
;; CHECK-NEXT:   (global.get $ctor-eval$global_6)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
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

 ;; CHECK:      (global $ctor-eval$global_5 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_6 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_5)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_5))
 (global $a (mut (ref null $A)) (ref.null $A))

 (global $b (mut (ref null $B)) (ref.null $B))

 (func "test"
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

 (func "keepalive" (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 1
;; CHECK-NEXT:   (global.get $a)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_5)
;; CHECK-NEXT:   (global.get $ctor-eval$global_6)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
(module
  ;; A cycle as above, but with globals in reverse order and with both non-
  ;; and immutability.

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

 ;; CHECK:      (global $ctor-eval$global_5 (ref $A) (struct.new $A
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT:  (i32.const 42)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $ctor-eval$global_6 (ref $B) (struct.new $B
 ;; CHECK-NEXT:  (global.get $ctor-eval$global_5)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: ))

 ;; CHECK:      (global $a (mut (ref null $A)) (global.get $ctor-eval$global_5))
 (global $a (mut (ref null $A)) (ref.null $A))

 (func "test"
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

 (func "keepalive" (result i32)
  (struct.get $A 1
   (global.get $a)
  )
 )
)

;; TODO Cycle of 3

;; TODO: Cycle with a struct and an array
;; CHECK:      (export "test" (func $0_3))

;; CHECK:      (export "keepalive" (func $1))

;; CHECK:      (start $start)

;; CHECK:      (func $1 (type $none_=>_i32) (result i32)
;; CHECK-NEXT:  (struct.get $A 1
;; CHECK-NEXT:   (global.get $a)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $start (type $none_=>_none)
;; CHECK-NEXT:  (struct.set $A 0
;; CHECK-NEXT:   (global.get $ctor-eval$global_5)
;; CHECK-NEXT:   (global.get $ctor-eval$global_6)
;; CHECK-NEXT:  )
;; CHECK-NEXT: )

;; CHECK:      (func $0_3 (type $none_=>_none)
;; CHECK-NEXT:  (local $a (ref $A))
;; CHECK-NEXT:  (local $b (ref $B))
;; CHECK-NEXT:  (nop)
;; CHECK-NEXT: )
