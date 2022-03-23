;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --nominal --signature-pruning -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $sig (func_subtype (param i32 f64) func))
  (type $sig (func_subtype (param i32) (param i64) (param f32) (param f64) func))

  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (elem declare func $func)

  ;; CHECK:      (func $func (type $sig) (param $0 i32) (param $1 f64)
  ;; CHECK-NEXT:  (local $2 f32)
  ;; CHECK-NEXT:  (local $3 i64)
  ;; CHECK-NEXT:  (i32.store
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f64.store
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (type $sig) (param $i32 i32) (param $i64 i64) (param $f32 f32) (param $f64 f64)
    ;; Use the first and last parameter. The middle parameters will be removed
    ;; both from the function and from $sig, and also in the calls below.
    (i32.store
      (i32.const 0)
      (local.get $i32)
    )
    (f64.store
      (i32.const 0)
      (local.get $f64)
    )
  )

  ;; CHECK:      (func $caller (type $none_=>_none)
  ;; CHECK-NEXT:  (call $func
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (f64.const 3)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call_ref
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:   (f64.const 7)
  ;; CHECK-NEXT:   (ref.func $func)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $caller
    (call $func
      (i32.const 0)
      (i64.const 1)
      (f32.const 2)
      (f64.const 3)
    )
    (call_ref
      (i32.const 4)
      (i64.const 5)
      (f32.const 6)
      (f64.const 7)
      (ref.func $func)
    )
  )
)

(module
  ;; CHECK:      (type $sig (func_subtype (param i64 f32) func))
  (type $sig (func_subtype (param i32) (param i64) (param f32) (param f64) func))

  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (elem declare func $func)

  ;; CHECK:      (func $func (type $sig) (param $0 i64) (param $1 f32)
  ;; CHECK-NEXT:  (local $2 f64)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (i64.store
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f32.store
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (type $sig) (param $i32 i32) (param $i64 i64) (param $f32 f32) (param $f64 f64)
    ;; Use the middle two parameters.
    (i64.store
      (i32.const 0)
      (local.get $i64)
    )
    (f32.store
      (i32.const 0)
      (local.get $f32)
    )
  )

  ;; CHECK:      (func $caller (type $none_=>_none)
  ;; CHECK-NEXT:  (call $func
  ;; CHECK-NEXT:   (i64.const 1)
  ;; CHECK-NEXT:   (f32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call_ref
  ;; CHECK-NEXT:   (i64.const 5)
  ;; CHECK-NEXT:   (f32.const 6)
  ;; CHECK-NEXT:   (ref.func $func)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $caller
    (call $func
      (i32.const 0)
      (i64.const 1)
      (f32.const 2)
      (f64.const 3)
    )
    (call_ref
      (i32.const 4)
      (i64.const 5)
      (f32.const 6)
      (f64.const 7)
      (ref.func $func)
    )
  )
)

(module
  ;; CHECK:      (type $sig (func_subtype (param i32 i64 f32) func))
  (type $sig (func_subtype (param i32) (param i64) (param f32) (param f64) func))

  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (elem declare func $func)

  ;; CHECK:      (func $func (type $sig) (param $0 i32) (param $1 i64) (param $2 f32)
  ;; CHECK-NEXT:  (local $3 f64)
  ;; CHECK-NEXT:  (i64.store
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (f32.store
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (local.get $2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $func (type $sig) (param $i32 i32) (param $i64 i64) (param $f32 f32) (param $f64 f64)
    ;; Use the middle two parameters.
    (i64.store
      (i32.const 0)
      (local.get $i64)
    )
    (f32.store
      (i32.const 0)
      (local.get $f32)
    )
  )

  ;; CHECK:      (func $caller (type $none_=>_none)
  ;; CHECK-NEXT:  (call $func
  ;; CHECK-NEXT:   (block $block (result i32)
  ;; CHECK-NEXT:    (call $caller)
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i64.const 1)
  ;; CHECK-NEXT:   (f32.const 2)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call_ref
  ;; CHECK-NEXT:   (i32.const 4)
  ;; CHECK-NEXT:   (i64.const 5)
  ;; CHECK-NEXT:   (f32.const 6)
  ;; CHECK-NEXT:   (ref.func $func)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $caller
    ;; As above, but now one of the unused parameters has a side effect which
    ;; prevents us from removing it (flattening the IR first would avoid this
    ;; limitation). We only end up removing a single unused param, the last.
    (call $func
      (block (result i32)
        (call $caller)
        (i32.const 0)
      )
      (i64.const 1)
      (f32.const 2)
      (f64.const 3)
    )
    (call_ref
      (i32.const 4)
      (i64.const 5)
      (f32.const 6)
      (f64.const 7)
      (ref.func $func)
    )
  )
)

(module
  ;; CHECK:      (type $sig (func_subtype func))
  (type $sig (func_subtype (param i32) (param i64) (param f32) (param f64) func))

  (memory 1 1)

  ;; CHECK:      (type $none_=>_none (func_subtype func))

  ;; CHECK:      (memory $0 1 1)

  ;; CHECK:      (elem declare func $func)

  ;; CHECK:      (func $func (type $sig)
  ;; CHECK-NEXT:  (local $0 f64)
  ;; CHECK-NEXT:  (local $1 f32)
  ;; CHECK-NEXT:  (local $2 i64)
  ;; CHECK-NEXT:  (local $3 i32)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $func (type $sig) (param $i32 i32) (param $i64 i64) (param $f32 f32) (param $f64 f64)
    ;; Use nothing at all: all params can be removed.
  )

  ;; CHECK:      (func $caller (type $none_=>_none)
  ;; CHECK-NEXT:  (call $func)
  ;; CHECK-NEXT:  (call_ref
  ;; CHECK-NEXT:   (ref.func $func)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $caller
    (call $func
      (i32.const 0)
      (i64.const 1)
      (f32.const 2)
      (f64.const 3)
    )
    (call_ref
      (i32.const 4)
      (i64.const 5)
      (f32.const 6)
      (f64.const 7)
      (ref.func $func)
    )
  )
)

