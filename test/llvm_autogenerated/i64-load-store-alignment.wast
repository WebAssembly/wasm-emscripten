(module
  (memory 1)
  (data (i32.const 4) "\10\04\00\00")
  (export "memory" (memory $0))
  (export "ldi64_a1" (func $ldi64_a1))
  (export "ldi64_a2" (func $ldi64_a2))
  (export "ldi64_a4" (func $ldi64_a4))
  (export "ldi64_a8" (func $ldi64_a8))
  (export "ldi64" (func $ldi64))
  (export "ldi64_a16" (func $ldi64_a16))
  (export "ldi8_a1" (func $ldi8_a1))
  (export "ldi8_a2" (func $ldi8_a2))
  (export "ldi16_a1" (func $ldi16_a1))
  (export "ldi16_a2" (func $ldi16_a2))
  (export "ldi16_a4" (func $ldi16_a4))
  (export "ldi32_a1" (func $ldi32_a1))
  (export "ldi32_a2" (func $ldi32_a2))
  (export "ldi32_a4" (func $ldi32_a4))
  (export "ldi32_a8" (func $ldi32_a8))
  (export "sti64_a1" (func $sti64_a1))
  (export "sti64_a2" (func $sti64_a2))
  (export "sti64_a4" (func $sti64_a4))
  (export "sti64_a8" (func $sti64_a8))
  (export "sti64" (func $sti64))
  (export "sti64_a16" (func $sti64_a16))
  (export "sti8_a1" (func $sti8_a1))
  (export "sti8_a2" (func $sti8_a2))
  (export "sti16_a1" (func $sti16_a1))
  (export "sti16_a2" (func $sti16_a2))
  (export "sti16_a4" (func $sti16_a4))
  (export "sti32_a1" (func $sti32_a1))
  (export "sti32_a2" (func $sti32_a2))
  (export "sti32_a4" (func $sti32_a4))
  (export "sti32_a8" (func $sti32_a8))
  (func $ldi64_a1 (param $0 i32) (result i64)
    (return
      (i64.load align=1
        (get_local $0)
      )
    )
  )
  (func $ldi64_a2 (param $0 i32) (result i64)
    (return
      (i64.load align=2
        (get_local $0)
      )
    )
  )
  (func $ldi64_a4 (param $0 i32) (result i64)
    (return
      (i64.load align=4
        (get_local $0)
      )
    )
  )
  (func $ldi64_a8 (param $0 i32) (result i64)
    (return
      (i64.load
        (get_local $0)
      )
    )
  )
  (func $ldi64 (param $0 i32) (result i64)
    (return
      (i64.load
        (get_local $0)
      )
    )
  )
  (func $ldi64_a16 (param $0 i32) (result i64)
    (return
      (i64.load
        (get_local $0)
      )
    )
  )
  (func $ldi8_a1 (param $0 i32) (result i64)
    (return
      (i64.load8_u
        (get_local $0)
      )
    )
  )
  (func $ldi8_a2 (param $0 i32) (result i64)
    (return
      (i64.load8_u
        (get_local $0)
      )
    )
  )
  (func $ldi16_a1 (param $0 i32) (result i64)
    (return
      (i64.load16_u align=1
        (get_local $0)
      )
    )
  )
  (func $ldi16_a2 (param $0 i32) (result i64)
    (return
      (i64.load16_u
        (get_local $0)
      )
    )
  )
  (func $ldi16_a4 (param $0 i32) (result i64)
    (return
      (i64.load16_u
        (get_local $0)
      )
    )
  )
  (func $ldi32_a1 (param $0 i32) (result i64)
    (return
      (i64.load32_u align=1
        (get_local $0)
      )
    )
  )
  (func $ldi32_a2 (param $0 i32) (result i64)
    (return
      (i64.load32_u align=2
        (get_local $0)
      )
    )
  )
  (func $ldi32_a4 (param $0 i32) (result i64)
    (return
      (i64.load32_u
        (get_local $0)
      )
    )
  )
  (func $ldi32_a8 (param $0 i32) (result i64)
    (return
      (i64.load32_u
        (get_local $0)
      )
    )
  )
  (func $sti64_a1 (param $0 i32) (param $1 i64)
    (drop
      (i64.store align=1
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti64_a2 (param $0 i32) (param $1 i64)
    (drop
      (i64.store align=2
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti64_a4 (param $0 i32) (param $1 i64)
    (drop
      (i64.store align=4
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti64_a8 (param $0 i32) (param $1 i64)
    (drop
      (i64.store
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti64 (param $0 i32) (param $1 i64)
    (drop
      (i64.store
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti64_a16 (param $0 i32) (param $1 i64)
    (drop
      (i64.store
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti8_a1 (param $0 i32) (param $1 i64)
    (drop
      (i64.store8
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti8_a2 (param $0 i32) (param $1 i64)
    (drop
      (i64.store8
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti16_a1 (param $0 i32) (param $1 i64)
    (drop
      (i64.store16 align=1
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti16_a2 (param $0 i32) (param $1 i64)
    (drop
      (i64.store16
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti16_a4 (param $0 i32) (param $1 i64)
    (drop
      (i64.store16
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti32_a1 (param $0 i32) (param $1 i64)
    (drop
      (i64.store align=1
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti32_a2 (param $0 i32) (param $1 i64)
    (drop
      (i64.store align=2
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti32_a4 (param $0 i32) (param $1 i64)
    (drop
      (i64.store
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
  (func $sti32_a8 (param $0 i32) (param $1 i64)
    (drop
      (i64.store
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
)
;; METADATA: { "asmConsts": {},"staticBump": 1040, "initializers": [] }
