(module
  (func $ifs
    (if (i32.const 0) (nop))
    (if (i32.const 0) (nop) (nop))
    (if (i32.const 0) (nop) (unreachable))
    (drop
      (if (result i32) (i32.const 0)
        (i32.add (i32.const 1) (i32.const 2))
        (i32.add (i32.const 1) (i32.const 2))
      )
    )
    (drop
      (if (result i32) (i32.const 0)
        (i32.add (i32.const 1) (i32.const 2))
        (i32.add (i32.const 1) (i32.const 333333333))
      )
    )
  )
  (func $ifs-blocks
    (if (i32.const 0)
      (block
        (nop)
      )
      (block
        (nop)
      )
    )
    (if (i32.const 0)
      (block
        (unreachable)
        (nop)
      )
      (block
        (nop)
      )
    )
    (if (i32.const 0)
      (block
        (nop)
      )
      (block
        (unreachable)
        (nop)
      )
    )
    (if (i32.const 0)
      (block
        (nop)
        (unreachable)
      )
      (block
        (nop)
      )
    )
    (if (i32.const 0)
      (block
        (nop)
      )
      (block
        (nop)
        (unreachable)
      )
    )
  )
)

