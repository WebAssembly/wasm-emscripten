(module
  (memory 1)
  (data (i32.const 4) "\10\04\00\00")
  (export "memory" (memory $0))
  (export "trunc_i8_i32" (func $trunc_i8_i32))
  (export "trunc_i16_i32" (func $trunc_i16_i32))
  (export "trunc_i8_i64" (func $trunc_i8_i64))
  (export "trunc_i16_i64" (func $trunc_i16_i64))
  (export "trunc_i32_i64" (func $trunc_i32_i64))
  (func $trunc_i8_i32 (param $0 i32) (param $1 i32)
    (drop
      (i32.store8
        (get_local $0)
        (get_local $1)
      )
    )
  )
  (func $trunc_i16_i32 (param $0 i32) (param $1 i32)
    (drop
      (i32.store16
        (get_local $0)
        (get_local $1)
      )
    )
  )
  (func $trunc_i8_i64 (param $0 i32) (param $1 i64)
    (drop
      (i64.store8
        (get_local $0)
        (get_local $1)
      )
    )
  )
  (func $trunc_i16_i64 (param $0 i32) (param $1 i64)
    (drop
      (i64.store16
        (get_local $0)
        (get_local $1)
      )
    )
  )
  (func $trunc_i32_i64 (param $0 i32) (param $1 i64)
    (drop
      (i64.store
        (get_local $0)
        (get_local $1)
      )
    )
  )
)
;; METADATA: { "asmConsts": {},"staticBump": 1040, "initializers": [] }
