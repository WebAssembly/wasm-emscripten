(module
 (type $i32_=>_i32 (func (param i32) (result i32)))
 (type $none_=>_none (func))
 (type $i32_=>_none (func (param i32)))
 (type $i32_i32_i32_=>_none (func (param i32 i32 i32)))
 (type $none_=>_i32 (func (result i32)))
 (type $i32_i32_=>_i32 (func (param i32 i32) (result i32)))
 (import "env" "__cxa_allocate_exception" (func $__cxa_allocate_exception (param i32) (result i32)))
 (import "env" "__cxa_throw" (func $__cxa_throw (param i32 i32 i32)))
 (import "env" "_Unwind_CallPersonality" (func $_Unwind_CallPersonality (param i32) (result i32)))
 (import "env" "__cxa_begin_catch" (func $__cxa_begin_catch (param i32) (result i32)))
 (import "env" "_Z7putchari" (func $putchar\28int\29 (param i32) (result i32)))
 (import "env" "__cxa_end_catch" (func $__cxa_end_catch))
 (global $__stack_pointer (mut i32) (i32.const 66128))
 (memory $0 2)
 (data $.rodata (i32.const 568) "\ff\00\0d\01\02\00\01\01\00\00\00\00\00\00\00\00")
 (table $0 1 1 funcref)
 (tag $tag$0 (param i32))
 (export "memory" (memory $0))
 (export "__wasm_call_ctors" (func $__wasm_call_ctors))
 (export "__cpp_exception" (tag $tag$0))
 (export "main" (func $main))
 (func $__wasm_call_ctors
 )
 (func $__original_main (result i32)
  (local $0 i32)
  (local $1 i32)
  (local.set $0
   (global.get $__stack_pointer)
  )
  (i32.store
   (local.tee $1
    (call $__cxa_allocate_exception
     (i32.const 4)
    )
   )
   (i32.const 3)
  )
  (try $label$8
   (do
    (call $__cxa_throw
     (local.get $1)
     (i32.const 0)
     (i32.const 0)
    )
   )
   (catch $tag$0
    (local.set $1
     (pop i32)
    )
    (global.set $__stack_pointer
     (local.get $0)
    )
    (i32.store offset=4
     (i32.const 0)
     (i32.const 568)
    )
    (i32.store
     (i32.const 0)
     (i32.const 0)
    )
    (drop
     (call $_Unwind_CallPersonality
      (local.get $1)
     )
    )
    (block $label$3
     (block $label$4
      (br_if $label$4
       (i32.ne
        (i32.load offset=8
         (i32.const 0)
        )
        (i32.const 1)
       )
      )
      (try $label$7
       (do
        (drop
         (call $putchar\28int\29
          (i32.load
           (call $__cxa_begin_catch
            (local.get $1)
           )
          )
         )
        )
        (br $label$3)
       )
       (catch_all
        (global.set $__stack_pointer
         (local.get $0)
        )
        (call $__cxa_end_catch)
        (rethrow $label$7)
       )
      )
     )
     (rethrow $label$8)
    )
    (call $__cxa_end_catch)
    (return
     (i32.const 0)
    )
   )
  )
  (unreachable)
 )
 (func $main (param $0 i32) (param $1 i32) (result i32)
  (call $__original_main)
 )
 ;; custom section "producers", size 111
 ;; features section: exception-handling
)

