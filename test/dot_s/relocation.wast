(module
  (memory 1
    (segment 0 "\04\00\00\00")
    (segment 4 "\00\00\00\00")
  )
  (export "memory" memory)
  (export "main" $main)
  (func $main (result i32)
    (local $$0 i32)
    (return
      (i32.load
        (i32.const 4)
      )
    )
  )
)
;; METADATA: { "asmConsts": {},"staticBump": 8, "initializers": [] }
