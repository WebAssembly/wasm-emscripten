;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --dae -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --dae --nominal -S -o - | filecheck %s --check-prefix NOMNL

(module
 ;; CHECK:      (type $return_{} (func (result (ref ${}))))
 ;; NOMNL:      (type $return_{} (func_subtype (result (ref ${})) func))
 (type $return_{} (func (result (ref ${}))))

 ;; CHECK:      (type ${i32_f32} (struct (field i32) (field f32)))
 ;; NOMNL:      (type ${i32_f32} (struct_subtype (field i32) (field f32) ${i32}))
 (type ${i32_f32} (struct_subtype (field i32) (field f32) ${i32}))

 ;; CHECK:      (type ${i32_i64} (struct (field i32) (field i64)))
 ;; NOMNL:      (type ${i32_i64} (struct_subtype (field i32) (field i64) ${i32}))
 (type ${i32_i64} (struct_subtype (field i32) (field i64) ${i32}))

 ;; CHECK:      (type ${i32} (struct (field i32)))
 ;; NOMNL:      (type ${i32} (struct_subtype (field i32) ${}))
 (type ${i32} (struct_subtype (field i32) ${}))

 ;; CHECK:      (type ${} (struct ))
 ;; NOMNL:      (type ${} (struct_subtype  data))
 (type ${} (struct))

 (table 1 1 funcref)

 ;; We cannot refine the return type if nothing is actually returned.
 ;; CHECK:      (func $refine-return-no-return (result anyref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-no-return)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-no-return (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-no-return)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $refine-return-no-return (result anyref)
  ;; Call this function, so that we attempt to optimize it. Note that we do not
  ;; just drop the result, as that would cause the drop optimizations to kick
  ;; in.
  (local $temp anyref)
  (local.set $temp (call $refine-return-no-return))

  (unreachable)
 )

 ;; We cannot refine the return type if it is already the best it can be.
 ;; CHECK:      (func $refine-return-no-refining (result anyref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-no-refining)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null any)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-no-refining (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-no-refining)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (ref.null any)
 ;; NOMNL-NEXT: )
 (func $refine-return-no-refining (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-no-refining))

  (ref.null any)
 )

 ;; Refine the return type based on the value flowing out.
 ;; CHECK:      (func $refine-return-flow (result funcref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-flow)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null func)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-flow (type $none_=>_funcref) (result funcref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-flow)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (ref.null func)
 ;; NOMNL-NEXT: )
 (func $refine-return-flow (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-flow))

  (ref.null func)
 )
 ;; CHECK:      (func $call-refine-return-flow (result funcref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $call-refine-return-flow)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if (result funcref)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (call $refine-return-flow)
 ;; CHECK-NEXT:   (call $refine-return-flow)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-refine-return-flow (type $none_=>_funcref) (result funcref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $call-refine-return-flow)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if (result funcref)
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (call $refine-return-flow)
 ;; NOMNL-NEXT:   (call $refine-return-flow)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-refine-return-flow (result anyref)
  (local $temp anyref)
  (local.set $temp (call $call-refine-return-flow))

  ;; After refining the return value of the above function, refinalize will
  ;; update types here, which will lead to updating the if, and then the entire
  ;; function's return value.
  (if (result anyref)
   (i32.const 1)
   (call $refine-return-flow)
   (call $refine-return-flow)
  )
 )

 ;; Refine the return type based on a return.
 ;; CHECK:      (func $refine-return-return (result funcref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-return)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (return
 ;; CHECK-NEXT:   (ref.null func)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-return (type $none_=>_funcref) (result funcref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-return)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (return
 ;; NOMNL-NEXT:   (ref.null func)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $refine-return-return (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-return))

  (return (ref.null func))
 )

 ;; Refine the return type based on multiple values.
 ;; CHECK:      (func $refine-return-many (result funcref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-many)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null func)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 2)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null func)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null func)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-many (type $none_=>_funcref) (result funcref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-many)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null func)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 2)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null func)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (ref.null func)
 ;; NOMNL-NEXT: )
 (func $refine-return-many (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-many))

  (if
   (i32.const 1)
   (return (ref.null func))
  )
  (if
   (i32.const 2)
   (return (ref.null func))
  )
  (ref.null func)
 )

 ;; CHECK:      (func $refine-return-many-blocked (result anyref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-many-blocked)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null func)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 2)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null data)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null func)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-many-blocked (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-many-blocked)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null func)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 2)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null data)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (ref.null func)
 ;; NOMNL-NEXT: )
 (func $refine-return-many-blocked (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-many-blocked))

  (if
   (i32.const 1)
   (return (ref.null func))
  )
  (if
   (i32.const 2)
   ;; The refined return value is blocked by this return.
   (return (ref.null data))
  )
  (ref.null func)
 )

 ;; CHECK:      (func $refine-return-many-blocked-2 (result anyref)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-many-blocked-2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null func)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 2)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null func)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null data)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-many-blocked-2 (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-many-blocked-2)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null func)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 2)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null func)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (ref.null data)
 ;; NOMNL-NEXT: )
 (func $refine-return-many-blocked-2 (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-many-blocked-2))

  (if
   (i32.const 1)
   (return (ref.null func))
  )
  (if
   (i32.const 2)
   (return (ref.null func))
  )
  ;; The refined return value is blocked by this value.
  (ref.null data)
 )

 ;; CHECK:      (func $refine-return-many-middle (result (ref null ${i32}))
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (call $refine-return-many-middle)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null ${i32_i64})
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (ref.null ${i32_f32})
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-many-middle (type $none_=>_ref?|${i32}|) (result (ref null ${i32}))
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (call $refine-return-many-middle)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null ${i32_i64})
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (ref.null ${i32_f32})
 ;; NOMNL-NEXT: )
 (func $refine-return-many-middle (result anyref)
  (local $temp anyref)
  (local.set $temp (call $refine-return-many-middle))

  ;; Return two different struct types, with an LUB that is not equal to either
  ;; of them.
  (if
   (i32.const 1)
   (return (ref.null ${i32_i64}))
  )
  (ref.null ${i32_f32})
 )

 ;; We can refine the return types of tuples.
 ;; CHECK:      (func $refine-return-tuple (result funcref i32)
 ;; CHECK-NEXT:  (local $temp anyref)
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (tuple.extract 0
 ;; CHECK-NEXT:    (call $refine-return-tuple)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (tuple.make
 ;; CHECK-NEXT:   (ref.null func)
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $refine-return-tuple (type $none_=>_funcref_i32) (result funcref i32)
 ;; NOMNL-NEXT:  (local $temp anyref)
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (tuple.extract 0
 ;; NOMNL-NEXT:    (call $refine-return-tuple)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (tuple.make
 ;; NOMNL-NEXT:   (ref.null func)
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $refine-return-tuple (result anyref i32)
  (local $temp anyref)
  (local.set $temp
   (tuple.extract 0
    (call $refine-return-tuple)
   )
  )

  (tuple.make
   (ref.null func)
   (i32.const 1)
  )
 )

 ;; This function does a return call of the one after it. The one after it
 ;; returns a ref.func of this one. They both begin by returning a funcref;
 ;; after refining the return type of the second function, it will have a more
 ;; specific type (which is ok as subtyping is allowed with tail calls).
 ;; CHECK:      (func $do-return-call (result funcref)
 ;; CHECK-NEXT:  (return_call $return-ref-func)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $do-return-call (type $none_=>_funcref) (result funcref)
 ;; NOMNL-NEXT:  (return_call $return-ref-func)
 ;; NOMNL-NEXT: )
 (func $do-return-call (result funcref)
  (return_call $return-ref-func)
 )
 ;; CHECK:      (func $return-ref-func (result (ref $none_=>_funcref))
 ;; CHECK-NEXT:  (ref.func $do-return-call)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $return-ref-func (type $none_=>_ref|none_->_funcref|) (result (ref $none_=>_funcref))
 ;; NOMNL-NEXT:  (ref.func $do-return-call)
 ;; NOMNL-NEXT: )
 (func $return-ref-func (result funcref)
  (ref.func $do-return-call)
 )

 ;; Show that we can optimize the return type of a function that does a tail
 ;; call.
 ;; CHECK:      (func $tail-callee (result (ref ${}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-callee (type $return_{}) (result (ref ${}))
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $tail-callee (result (ref ${}))
  (unreachable)
 )
 ;; CHECK:      (func $tail-caller-yes (result (ref ${}))
 ;; CHECK-NEXT:  (return_call $tail-callee)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-yes (type $return_{}) (result (ref ${}))
 ;; NOMNL-NEXT:  (return_call $tail-callee)
 ;; NOMNL-NEXT: )
 (func $tail-caller-yes (result anyref)
  ;; This function's return type can be refined because of this call, whose
  ;; target's return type is more specific than anyref.
  (return_call $tail-callee)
 )
 ;; CHECK:      (func $tail-caller-no (result anyref)
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null any)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (return_call $tail-callee)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-no (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null any)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (return_call $tail-callee)
 ;; NOMNL-NEXT: )
 (func $tail-caller-no (result anyref)
  ;; This function's return type cannot be refined because of another return
  ;; whose type prevents it.
  (if (i32.const 1)
   (return (ref.null any))
  )
  (return_call $tail-callee)
 )
 ;; CHECK:      (func $tail-call-caller
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $tail-caller-yes)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $tail-caller-no)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-call-caller (type $none_=>_none)
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (call $tail-caller-yes)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (call $tail-caller-no)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $tail-call-caller
  ;; Call the functions to cause optimization to happen.
  (drop
   (call $tail-caller-yes)
  )
  (drop
   (call $tail-caller-no)
  )
 )

 ;; As above, but with an indirect tail call.
 ;; CHECK:      (func $tail-callee-indirect (result (ref ${}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-callee-indirect (type $return_{}) (result (ref ${}))
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $tail-callee-indirect (result (ref ${}))
  (unreachable)
 )
 ;; CHECK:      (func $tail-caller-indirect-yes (result (ref ${}))
 ;; CHECK-NEXT:  (return_call_indirect $0 (type $return_{})
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-indirect-yes (type $return_{}) (result (ref ${}))
 ;; NOMNL-NEXT:  (return_call_indirect $0 (type $return_{})
 ;; NOMNL-NEXT:   (i32.const 0)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $tail-caller-indirect-yes (result anyref)
  (return_call_indirect (type $return_{}) (i32.const 0))
 )
 ;; CHECK:      (func $tail-caller-indirect-no (result anyref)
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null any)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (return_call_indirect $0 (type $return_{})
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-indirect-no (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null any)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (return_call_indirect $0 (type $return_{})
 ;; NOMNL-NEXT:   (i32.const 0)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $tail-caller-indirect-no (result anyref)
  (if (i32.const 1)
   (return (ref.null any))
  )
  (return_call_indirect (type $return_{}) (i32.const 0))
 )
 ;; CHECK:      (func $tail-call-caller-indirect
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $tail-caller-indirect-yes)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $tail-caller-indirect-no)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-call-caller-indirect (type $none_=>_none)
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (call $tail-caller-indirect-yes)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (call $tail-caller-indirect-no)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $tail-call-caller-indirect
  (drop
   (call $tail-caller-indirect-yes)
  )
  (drop
   (call $tail-caller-indirect-no)
  )
 )

 ;; As above, but with a tail call by function reference.
 ;; CHECK:      (func $tail-callee-call_ref (result (ref ${}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-callee-call_ref (type $return_{}) (result (ref ${}))
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $tail-callee-call_ref (result (ref ${}))
  (unreachable)
 )
 ;; CHECK:      (func $tail-caller-call_ref-yes (result (ref ${}))
 ;; CHECK-NEXT:  (return_call_ref
 ;; CHECK-NEXT:   (ref.null $return_{})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-call_ref-yes (type $return_{}) (result (ref ${}))
 ;; NOMNL-NEXT:  (return_call_ref
 ;; NOMNL-NEXT:   (ref.null $return_{})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $tail-caller-call_ref-yes (result anyref)
  (return_call_ref (ref.null $return_{}))
 )
 ;; CHECK:      (func $tail-caller-call_ref-no (result anyref)
 ;; CHECK-NEXT:  (if
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (return
 ;; CHECK-NEXT:    (ref.null any)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (return_call_ref
 ;; CHECK-NEXT:   (ref.null $return_{})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-call_ref-no (type $none_=>_anyref) (result anyref)
 ;; NOMNL-NEXT:  (if
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (return
 ;; NOMNL-NEXT:    (ref.null any)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (return_call_ref
 ;; NOMNL-NEXT:   (ref.null $return_{})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $tail-caller-call_ref-no (result anyref)
  (if (i32.const 1)
   (return (ref.null any))
  )
  (return_call_ref (ref.null $return_{}))
 )
 ;; CHECK:      (func $tail-caller-call_ref-unreachable
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-caller-call_ref-unreachable (type $none_=>_none)
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $tail-caller-call_ref-unreachable (result anyref)
  ;; An unreachable means there is no function signature to even look at. We
  ;; should not hit an assertion on such things.
  (return_call_ref (unreachable))
 )
 ;; CHECK:      (func $tail-call-caller-call_ref
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $tail-caller-call_ref-yes)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (call $tail-caller-call_ref-no)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $tail-caller-call_ref-unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $tail-call-caller-call_ref (type $none_=>_none)
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (call $tail-caller-call_ref-yes)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (call $tail-caller-call_ref-no)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $tail-caller-call_ref-unreachable)
 ;; NOMNL-NEXT: )
 (func $tail-call-caller-call_ref
  (drop
   (call $tail-caller-call_ref-yes)
  )
  (drop
   (call $tail-caller-call_ref-no)
  )
  (drop
   (call $tail-caller-call_ref-unreachable)
  )
 )
)
