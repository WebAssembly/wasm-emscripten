;; Test that wasm-emscripten-finalize can locate data within passive segments
;; even when compiled with PIC, which means that segment addresses are non-constant.

;; RUN: wasm-emscripten-finalize --enable-bulk-memory %s -o out.wasm | filecheck %s

;; CHECK:  "asmConsts": {
;; CHECK:    "3": "hello"
;; CHECK:  },

(module
 (import "env" "memory" (memory $memory 1 1))
 (import "env" "__memory_base" (global $__memory_base i32))
 (import "env" "emscripten_asm_const_int" (func $emscripten_asm_const_int (param i32 i32 i32) (result i32)))
 (data passive "xxxhello\00yyy")
 ;; memory init function similar to those generated by wasm-ld
 (start $__wasm_init_memory)
 (func $__wasm_init_memory
  (memory.init 0
   (i32.add
    (i32.const 0)
    (global.get $__memory_base)
   )
   (i32.const 0)
   (i32.const 12)
  )
 )
 ;; EM_ASM call passing string at address 3 in the passive segment
 (func $foo (result i32)
  (call $emscripten_asm_const_int
   (i32.add
    (global.get $__memory_base)
    (i32.const 3)
   )
   (i32.const 0)
   (i32.const 0)
  )
 )
)
