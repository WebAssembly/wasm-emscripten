(module
  (memory 1)
  (data (i32.const 12) "\01\00\00\00\00\00\00\00\00\00\00\00")
  (export "memory" (memory $0))
  (export "f" (func $f))
  (func $f (param $0 i32) (param $1 i32)
    (drop
      (i32.store offset=16
        (get_local $0)
        (get_local $1)
      )
    )
    (return)
  )
)
;; METADATA: { "asmConsts": {},"staticBump": 24, "initializers": [] }
