(module
  (memory $0 1)
  (data (i32.const 20) "\10\00\00\00")
  (export "memory" (memory $0))
  (table 0 anyfunc)
  
)
;; METADATA: { "asmConsts": {},"staticBump": 24, "initializers": [] }
