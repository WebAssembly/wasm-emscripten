;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s -all --dae -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --dae --nominal -S -o - | filecheck %s --check-prefix NOMNL

(module
 ;; CHECK:      (type ${i32} (struct (field i32)))
 ;; NOMNL:      (type ${} (struct_subtype  data))

 ;; NOMNL:      (type ${i32} (struct_subtype (field i32) ${}))
 (type ${i32} (struct_subtype (field i32) ${}))

 ;; CHECK:      (type ${} (struct ))
 (type ${} (struct))

 ;; CHECK:      (type ${f64} (struct (field f64)))

 ;; CHECK:      (type ${i32_i64} (struct (field i32) (field i64)))
 ;; NOMNL:      (type ${f64} (struct_subtype (field f64) ${}))

 ;; NOMNL:      (type ${i32_i64} (struct_subtype (field i32) (field i64) ${i32}))
 (type ${i32_i64} (struct_subtype (field i32) (field i64) ${i32}))

 (type ${f64} (struct_subtype (field f64) ${}))

 ;; CHECK:      (type ${i32_f32} (struct (field i32) (field f32)))
 ;; NOMNL:      (type ${i32_f32} (struct_subtype (field i32) (field f32) ${i32}))
 (type ${i32_f32} (struct_subtype (field i32) (field f32) ${i32}))

 ;; CHECK:      (func $call-various-params-no
 ;; CHECK-NEXT:  (call $various-params-no
 ;; CHECK-NEXT:   (call $get_{})
 ;; CHECK-NEXT:   (call $get_{i32})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $various-params-no
 ;; CHECK-NEXT:   (call $get_{i32})
 ;; CHECK-NEXT:   (call $get_{f64})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-various-params-no (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $various-params-no
 ;; NOMNL-NEXT:   (call $get_{})
 ;; NOMNL-NEXT:   (call $get_{i32})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $various-params-no
 ;; NOMNL-NEXT:   (call $get_{i32})
 ;; NOMNL-NEXT:   (call $get_{f64})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-various-params-no
  ;; The first argument gets {} and {i32}; the second {i32} and {f64}; none of
  ;; those pairs can be optimized. Note that we do not pass in all nulls, as
  ;; all nulls are identical and we could do other optimization work due to
  ;; that.
  (call $various-params-no
   (call $get_{})
   (call $get_{i32})
  )
  (call $various-params-no
   (call $get_{i32})
   (call $get_{f64})
  )
 )
 ;; This function is called in ways that do not allow us to alter the types of
 ;; its parameters (see last function).
 ;; CHECK:      (func $various-params-no (param $x (ref null ${})) (param $y (ref null ${}))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $various-params-no (type $ref?|${}|_ref?|${}|_=>_none) (param $x (ref null ${})) (param $y (ref null ${}))
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $y)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $various-params-no (param $x (ref null ${})) (param $y (ref null ${}))
  ;; "Use" the locals to avoid other optimizations kicking in.
  (drop (local.get $x))
  (drop (local.get $y))
 )

 ;; CHECK:      (func $get_{} (result (ref null ${}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $get_{} (type $none_=>_ref?|${}|) (result (ref null ${}))
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $get_{} (result (ref null ${}))
  (unreachable)
 )
 ;; CHECK:      (func $get_{i32} (result (ref null ${i32}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $get_{i32} (type $none_=>_ref?|${i32}|) (result (ref null ${i32}))
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $get_{i32} (result (ref null ${i32}))
  (unreachable)
 )
 ;; CHECK:      (func $get_{f64} (result (ref null ${f64}))
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $get_{f64} (type $none_=>_ref?|${f64}|) (result (ref null ${f64}))
 ;; NOMNL-NEXT:  (unreachable)
 ;; NOMNL-NEXT: )
 (func $get_{f64} (result (ref null ${f64}))
  (unreachable)
 )

 ;; CHECK:      (func $call-various-params-yes
 ;; CHECK-NEXT:  (call $various-params-yes
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $various-params-yes
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:   (i32.const 1)
 ;; CHECK-NEXT:   (call $get_null_{i32_i64})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-various-params-yes (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $various-params-yes
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:   (i32.const 0)
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $various-params-yes
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:   (i32.const 1)
 ;; NOMNL-NEXT:   (call $get_null_{i32_i64})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-various-params-yes
  ;; The first argument gets {i32} and {i32}; the second {i32} and {i32_i64};
  ;; both of those pairs can be optimized to {i32}.
  ;; There is also an i32 in the middle, which should not confuse us.
  (call $various-params-yes
   (call $get_null_{i32})
   (i32.const 0)
   (call $get_null_{i32})
  )
  (call $various-params-yes
   (call $get_null_{i32})
   (i32.const 1)
   (call $get_null_{i32_i64})
  )
 )
 ;; This function is called in ways that *do* allow us to alter the types of
 ;; its parameters (see last function).
 ;; CHECK:      (func $various-params-yes (param $x (ref null ${i32})) (param $i i32) (param $y (ref null ${i32}))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $i)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $various-params-yes (type $ref?|${i32}|_i32_ref?|${i32}|_=>_none) (param $x (ref null ${i32})) (param $i i32) (param $y (ref null ${i32}))
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $i)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $y)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $various-params-yes (param $x (ref null ${})) (param $i i32) (param $y (ref null ${}))
  ;; "Use" the locals to avoid other optimizations kicking in.
  (drop (local.get $x))
  (drop (local.get $i))
  (drop (local.get $y))
 )

 ;; CHECK:      (func $call-various-params-set
 ;; CHECK-NEXT:  (call $various-params-set
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $various-params-set
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:   (call $get_null_{i32_i64})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-various-params-set (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $various-params-set
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $various-params-set
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:   (call $get_null_{i32_i64})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-various-params-set
  ;; The first argument gets {i32} and {i32}; the second {i32} and {i32_i64;
  ;; both of those pairs can be optimized to {i32}
  (call $various-params-set
   (call $get_null_{i32})
   (call $get_null_{i32})
  )
  (call $various-params-set
   (call $get_null_{i32})
   (call $get_null_{i32_i64})
  )
 )
 ;; This function is called in ways that *do* allow us to alter the types of
 ;; its parameters (see last function), however, we reuse the parameters by
 ;; writing to them, which causes problems in one case.
 ;; CHECK:      (func $various-params-set (param $x (ref null ${i32})) (param $y (ref null ${i32}))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $x
 ;; CHECK-NEXT:   (ref.null none)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $y
 ;; CHECK-NEXT:   (call $get_null_{i32_i64})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $various-params-set (type $ref?|${i32}|_ref?|${i32}|_=>_none) (param $x (ref null ${i32})) (param $y (ref null ${i32}))
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $y)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (local.set $x
 ;; NOMNL-NEXT:   (ref.null none)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (local.set $y
 ;; NOMNL-NEXT:   (call $get_null_{i32_i64})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $y)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $various-params-set (param $x (ref null ${})) (param $y (ref null ${}))
  ;; "Use" the locals to avoid other optimizations kicking in.
  (drop (local.get $x))
  (drop (local.get $y))
  ;; Write to $x a value that will not fit in the refined type, which will
  ;; force us to do a fixup: the param will get the new type, and a new local
  ;; will stay at the old type, and we will use that local throughout the
  ;; function.
  (local.set $x (ref.null ${}))
  (drop
   (local.get $x)
  )
  ;; Write to $y in a way that does not cause any issue, and we should not do
  ;; any fixup while we refine the type.
  (local.set $y (call $get_null_{i32_i64}))
  (drop
   (local.get $y)
  )
 )

 ;; CHECK:      (func $call-various-params-tee
 ;; CHECK-NEXT:  (call $various-params-tee
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-various-params-tee (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $various-params-tee
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-various-params-tee
  ;; The argument gets {i32}, which allows us to refine.
  (call $various-params-tee
   (call $get_null_{i32})
  )
 )
 ;; CHECK:      (func $various-params-tee (param $x (ref null ${i32}))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (block (result (ref null ${i32}))
 ;; CHECK-NEXT:    (local.tee $x
 ;; CHECK-NEXT:     (call $get_null_{i32_i64})
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $various-params-tee (type $ref?|${i32}|_=>_none) (param $x (ref null ${i32}))
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (block (result (ref null ${i32}))
 ;; NOMNL-NEXT:    (local.tee $x
 ;; NOMNL-NEXT:     (call $get_null_{i32_i64})
 ;; NOMNL-NEXT:    )
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $various-params-tee (param $x (ref null ${}))
  ;; "Use" the locals to avoid other optimizations kicking in.
  (drop (local.get $x))
  ;; Write to $x in a way that allows us to make the type more specific. We
  ;; must also update the type of the tee (if we do not, a validation error
  ;; would occur), and that will also cause the block's type to update as well.
  (drop
   (block (result (ref null ${}))
    (local.tee $x (call $get_null_{i32_i64}))
   )
  )
 )

 ;; CHECK:      (func $call-various-params-null
 ;; CHECK-NEXT:  (call $various-params-null
 ;; CHECK-NEXT:   (ref.as_non_null
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (call $get_null_{i32})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $various-params-null
 ;; CHECK-NEXT:   (ref.as_non_null
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (ref.as_non_null
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-various-params-null (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $various-params-null
 ;; NOMNL-NEXT:   (ref.as_non_null
 ;; NOMNL-NEXT:    (ref.null none)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:   (call $get_null_{i32})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $various-params-null
 ;; NOMNL-NEXT:   (ref.as_non_null
 ;; NOMNL-NEXT:    (ref.null none)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:   (ref.as_non_null
 ;; NOMNL-NEXT:    (ref.null none)
 ;; NOMNL-NEXT:   )
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-various-params-null
  ;; The first argument gets non-null values, allowing us to refine it. The
  ;; second gets only one.
  (call $various-params-null
   (ref.as_non_null (ref.null ${i32}))
   (call $get_null_{i32})
  )
  (call $various-params-null
   (ref.as_non_null (ref.null ${i32}))
   (ref.as_non_null (ref.null ${i32}))
  )
 )
 ;; This function is called in ways that allow us to make the first parameter
 ;; non-nullable.
 ;; CHECK:      (func $various-params-null (param $x (ref none)) (param $y (ref null ${i32}))
 ;; CHECK-NEXT:  (local $temp i32)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $temp
 ;; CHECK-NEXT:   (local.get $temp)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $various-params-null (type $ref|none|_ref?|${i32}|_=>_none) (param $x (ref none)) (param $y (ref null ${i32}))
 ;; NOMNL-NEXT:  (local $temp i32)
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $y)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (local.set $temp
 ;; NOMNL-NEXT:   (local.get $temp)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $various-params-null (param $x (ref null ${})) (param $y (ref null ${}))
  (local $temp i32)
  ;; "Use" the locals to avoid other optimizations kicking in.
  (drop (local.get $x))
  (drop (local.get $y))
  ;; Use a local in this function as well, which should be ignored by this pass
  ;; (when we scan and update all local.gets and sets, we should only do so on
  ;; parameters, and not vars - and we can crash if we scan/update things we
  ;; should not).
  (local.set $temp (local.get $temp))
 )

 ;; CHECK:      (func $call-various-params-middle
 ;; CHECK-NEXT:  (call $various-params-middle
 ;; CHECK-NEXT:   (call $get_null_{i32_i64})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $various-params-middle
 ;; CHECK-NEXT:   (call $get_null_{i32_f32})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-various-params-middle (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $various-params-middle
 ;; NOMNL-NEXT:   (call $get_null_{i32_i64})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $various-params-middle
 ;; NOMNL-NEXT:   (call $get_null_{i32_f32})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-various-params-middle
  ;; The argument gets {i32_i64} and {i32_f32}. This allows us to refine from
  ;; {} to {i32}, a type "in the middle".
  (call $various-params-middle
   (call $get_null_{i32_i64})
  )
  (call $various-params-middle
   (call $get_null_{i32_f32})
  )
 )
 ;; CHECK:      (func $various-params-middle (param $x (ref null ${i32}))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $various-params-middle (type $ref?|${i32}|_=>_none) (param $x (ref null ${i32}))
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $various-params-middle (param $x (ref null ${}))
  ;; "Use" the local to avoid other optimizations kicking in.
  (drop (local.get $x))
 )

 ;; CHECK:      (func $unused-and-refinable
 ;; CHECK-NEXT:  (local $0 dataref)
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $unused-and-refinable (type $none_=>_none)
 ;; NOMNL-NEXT:  (local $0 dataref)
 ;; NOMNL-NEXT:  (nop)
 ;; NOMNL-NEXT: )
 (func $unused-and-refinable (param $0 dataref)
  ;; This function does not use $0. It is called with ${}, so it is also
  ;; a parameter whose type we can refine. Do not do both operations: instead,
  ;; just remove it because it is ignored, without altering the type (handling
  ;; both operations would introduce some corner cases, and it just isn't worth
  ;; handling them if the param is completely unused anyhow). We should see in
  ;; the test output that the local $0 (the unused param) becomes a local
  ;; because it is unused, and that local does *not* have its type refined to
  ;; ${} (it will however be changed to be nullable, which it must be as a
  ;; local).
 )

 ;; CHECK:      (func $call-unused-and-refinable
 ;; CHECK-NEXT:  (call $unused-and-refinable)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-unused-and-refinable (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $unused-and-refinable)
 ;; NOMNL-NEXT: )
 (func $call-unused-and-refinable
  (call $unused-and-refinable
   (struct.new_default ${})
  )
 )

 ;; CHECK:      (func $non-nullable-fixup (param $0 (ref ${}))
 ;; CHECK-NEXT:  (local $1 dataref)
 ;; CHECK-NEXT:  (local.set $1
 ;; CHECK-NEXT:   (local.get $0)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $1
 ;; CHECK-NEXT:   (local.get $1)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $non-nullable-fixup (type $ref|${}|_=>_none) (param $0 (ref ${}))
 ;; NOMNL-NEXT:  (local $1 dataref)
 ;; NOMNL-NEXT:  (local.set $1
 ;; NOMNL-NEXT:   (local.get $0)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (local.set $1
 ;; NOMNL-NEXT:   (local.get $1)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $non-nullable-fixup (param $0 dataref)
  ;; Use the param to avoid other opts removing it, and to force us to do a
  ;; fixup when we refine the param's type. When doing so, we must handle the
  ;; fact that the new local's type is non-nullable.
  (local.set $0
   (local.get $0)
  )
 )

 ;; CHECK:      (func $call-non-nullable-fixup
 ;; CHECK-NEXT:  (call $non-nullable-fixup
 ;; CHECK-NEXT:   (struct.new_default ${})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-non-nullable-fixup (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $non-nullable-fixup
 ;; NOMNL-NEXT:   (struct.new_default ${})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-non-nullable-fixup
  (call $non-nullable-fixup
   (struct.new_default ${})
  )
 )

 ;; CHECK:      (func $call-update-null
 ;; CHECK-NEXT:  (call $update-null
 ;; CHECK-NEXT:   (ref.null none)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (call $update-null
 ;; CHECK-NEXT:   (struct.new_default ${})
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $call-update-null (type $none_=>_none)
 ;; NOMNL-NEXT:  (call $update-null
 ;; NOMNL-NEXT:   (ref.null none)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT:  (call $update-null
 ;; NOMNL-NEXT:   (struct.new_default ${})
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $call-update-null
  ;; Call a function with one of the parameters a null of a type that we can
  ;; update in order to get a better LUB.
  (call $update-null
   (ref.null any)
  )
  (call $update-null
   (struct.new_default ${})
  )
 )

 ;; CHECK:      (func $update-null (param $x (ref null ${}))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $update-null (type $ref?|${}|_=>_none) (param $x (ref null ${}))
 ;; NOMNL-NEXT:  (drop
 ;; NOMNL-NEXT:   (local.get $x)
 ;; NOMNL-NEXT:  )
 ;; NOMNL-NEXT: )
 (func $update-null (param $x (ref null any))
  ;; "Use" the param to avoid other optimizations kicking in. We should only
  ;; see the type of the param refined to a null ${} after updating the null
  ;; in the caller.
  (drop (local.get $x))
 )

 ;; CHECK:      (func $get_null_{i32} (result (ref null ${i32}))
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $get_null_{i32} (type $none_=>_ref?|${i32}|) (result (ref null ${i32}))
 ;; NOMNL-NEXT:  (ref.null none)
 ;; NOMNL-NEXT: )
 (func $get_null_{i32} (result (ref null ${i32}))
  ;; Helper function that returns a null value of ${i32}. We use this instead of
  ;; a direct ref.null because those can be rewritten by LUBFinder.
  (ref.null ${i32})
 )

 ;; CHECK:      (func $get_null_{i32_i64} (result (ref null ${i32_i64}))
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $get_null_{i32_i64} (type $none_=>_ref?|${i32_i64}|) (result (ref null ${i32_i64}))
 ;; NOMNL-NEXT:  (ref.null none)
 ;; NOMNL-NEXT: )
 (func $get_null_{i32_i64} (result (ref null ${i32_i64}))
  (ref.null ${i32_i64})
 )

 ;; CHECK:      (func $get_null_{i32_f32} (result (ref null ${i32_f32}))
 ;; CHECK-NEXT:  (ref.null none)
 ;; CHECK-NEXT: )
 ;; NOMNL:      (func $get_null_{i32_f32} (type $none_=>_ref?|${i32_f32}|) (result (ref null ${i32_f32}))
 ;; NOMNL-NEXT:  (ref.null none)
 ;; NOMNL-NEXT: )
 (func $get_null_{i32_f32} (result (ref null ${i32_f32}))
  (ref.null ${i32_f32})
 )
)
