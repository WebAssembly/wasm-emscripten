(module
  (type $0 (func))
  (import "env" "global-mut" (global $global-mut (mut i32)))
  (func $foo (type $0)
    (set_global $global-mut
      (i32.add
        (get_global $global-mut)
        (i32.const 1)
      )
    )
  )
)
