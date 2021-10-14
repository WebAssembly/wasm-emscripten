;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: foreach %s %t wasm-opt --inlining --all-features -S -o - | filecheck %s
;; RUN: foreach %s %t wasm-opt --inlining --all-features --nominal -S -o - | filecheck %s --check-prefix=NOMNL

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $none_=>_funcref (func (result funcref)))

  ;; CHECK:      (elem declare func $foo)

  ;; CHECK:      (export "ref_func_test" (func $ref_func_test))
  ;; NOMNL:      (type $none_=>_none (func_subtype func))

  ;; NOMNL:      (type $none_=>_funcref (func_subtype (result funcref) func))

  ;; NOMNL:      (elem declare func $foo)

  ;; NOMNL:      (export "ref_func_test" (func $ref_func_test))
  (export "ref_func_test" (func $ref_func_test))

  ;; $foo should not be removed after being inlined, because there is 'ref.func'
  ;; instruction that refers to it
  ;; CHECK:      (func $foo
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  ;; NOMNL:      (func $foo
  ;; NOMNL-NEXT:  (nop)
  ;; NOMNL-NEXT: )
  (func $foo)

  ;; CHECK:      (func $ref_func_test (result funcref)
  ;; CHECK-NEXT:  (block
  ;; CHECK-NEXT:   (block $__inlined_func$foo
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (ref.func $foo)
  ;; CHECK-NEXT: )
  ;; NOMNL:      (func $ref_func_test (result funcref)
  ;; NOMNL-NEXT:  (block
  ;; NOMNL-NEXT:   (block $__inlined_func$foo
  ;; NOMNL-NEXT:    (nop)
  ;; NOMNL-NEXT:   )
  ;; NOMNL-NEXT:  )
  ;; NOMNL-NEXT:  (ref.func $foo)
  ;; NOMNL-NEXT: )
  (func $ref_func_test (result funcref)
    (call $foo)
    (ref.func $foo)
  )
)

(module
 ;; a function reference in a global's init should be noticed, and prevent us
 ;; from removing an inlined function
 ;; CHECK:      (type $none_=>_i32 (func (result i32)))

 ;; CHECK:      (global $global$0 (mut funcref) (ref.func $0))
 ;; NOMNL:      (type $none_=>_i32 (func_subtype (result i32) func))

 ;; NOMNL:      (global $global$0 (mut funcref) (ref.func $0))
 (global $global$0 (mut funcref) (ref.func $0))

 ;; CHECK:      (func $0 (result i32)
 ;; CHECK-NEXT:  (i32.const 1337)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $0 (result i32)
 ;; NOMNL-NEXT:  (i32.const 1337)
 ;; NOMNL-NEXT: )
 (func $0 (result i32)
  (i32.const 1337)
 )

 ;; CHECK:      (func $1 (result i32)
 ;; CHECK-NEXT:  (block $__inlined_func$0 (result i32)
 ;; CHECK-NEXT:   (i32.const 1337)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $1 (result i32)
 ;; NOMNL-NEXT:  (block $__inlined_func$0 (result i32)
 ;; NOMNL-NEXT:   (i32.const 1337)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $1 (result i32)
  (call $0)
 )
)

(module
 ;; a function reference in the start should be noticed, and prevent us
 ;; from removing an inlined function
 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (start $0)
 ;; NOMNL:      (type $none_=>_none (func_subtype func))

 ;; NOMNL:      (start $0)
 (start $0)

 ;; CHECK:      (func $0
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $0
 ;; NOMNL-NEXT:  (nop)
 ;; NOMNL-NEXT: )
 (func $0
  (nop)
 )

 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (block $__inlined_func$0
 ;; CHECK-NEXT:   (nop)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $1
 ;; NOMNL-NEXT:  (block $__inlined_func$0
 ;; NOMNL-NEXT:   (nop)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $1
  (call $0)
 )
)

;; inline a return_call_ref
(module
 ;; CHECK:      (type $none_=>_none (func))
 ;; NOMNL:      (type $none_=>_none (func_subtype func))
 (type $none_=>_none (func))

 ;; CHECK:      (export "func_36_invoker" (func $1))
 ;; NOMNL:      (export "func_36_invoker" (func $1))
 (export "func_36_invoker" (func $1))

 (func $0
  (return_call_ref
   (ref.null $none_=>_none)
  )
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (block $__inlined_func$0
 ;; CHECK-NEXT:   (block
 ;; CHECK-NEXT:    (call_ref
 ;; CHECK-NEXT:     (ref.null $none_=>_none)
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:    (br $__inlined_func$0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (br $__inlined_func$0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $1
 ;; NOMNL-NEXT:  (block $__inlined_func$0
 ;; NOMNL-NEXT:   (block
 ;; NOMNL-NEXT:    (call_ref
 ;; NOMNL-NEXT:     (ref.null $none_=>_none)
 ;; NOMNL-NEXT:    )
 ;; NOMNL-NEXT:    (br $__inlined_func$0)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:   (br $__inlined_func$0)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $1
  (call $0)
 )
)

;; handle non-nullable parameter types (which turn into local types after
;; inlining)
(module
 (func $0 (param $non-null (ref func)) (result (ref func))
  (local.get $non-null)
 )
 ;; CHECK:      (type $none_=>_ref|func| (func (result (ref func))))

 ;; CHECK:      (elem declare func $1)

 ;; CHECK:      (func $1 (result (ref func))
 ;; CHECK-NEXT:  (local $0 funcref)
 ;; CHECK-NEXT:  (block $__inlined_func$0 (result (ref func))
 ;; CHECK-NEXT:   (local.set $0
 ;; CHECK-NEXT:    (ref.func $1)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (ref.as_non_null
 ;; CHECK-NEXT:    (local.get $0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (type $none_=>_ref|func| (func_subtype (result (ref func)) func))

 ;; NOMNL:      (elem declare func $1)

 ;; NOMNL:      (func $1 (result (ref func))
 ;; NOMNL-NEXT:  (local $0 funcref)
 ;; NOMNL-NEXT:  (block $__inlined_func$0 (result (ref func))
 ;; NOMNL-NEXT:   (local.set $0
 ;; NOMNL-NEXT:    (ref.func $1)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:   (ref.as_non_null
 ;; NOMNL-NEXT:    (local.get $0)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $1 (result (ref func))
  (call $0
   (ref.func $1)
  )
 )
)

;; never inline an rtt parameter, as those cannot be handled as locals
(module
 ;; CHECK:      (type $struct (struct ))
 ;; NOMNL:      (type $struct (struct_subtype  data))
 (type $struct (struct))
 ;; CHECK:      (type $rtt_$struct_=>_none (func (param (rtt $struct))))

 ;; CHECK:      (type $none_=>_none (func))

 ;; CHECK:      (func $0 (param $rtt (rtt $struct))
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (type $rtt_$struct_=>_none (func_subtype (param (rtt $struct)) func))

 ;; NOMNL:      (type $none_=>_none (func_subtype func))

 ;; NOMNL:      (func $0 (param $rtt (rtt $struct))
 ;; NOMNL-NEXT:  (nop)
 ;; NOMNL-NEXT: )
 (func $0 (param $rtt (rtt $struct))
 )
 ;; CHECK:      (func $1
 ;; CHECK-NEXT:  (call $0
 ;; CHECK-NEXT:   (rtt.canon $struct)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $1
 ;; NOMNL-NEXT:  (call $0
 ;; NOMNL-NEXT:   (rtt.canon $struct)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $1
  (call $0
   (rtt.canon $struct)
  )
 )
)
