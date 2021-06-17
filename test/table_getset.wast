(module
  (type $vec (array (mut i32)))
  (table $t2 2 externref)
  (table $t3 3 funcref)
  (table $t4 2 (ref null data))
  (elem (table $t3) (i32.const 1) func $dummy)
  (func $dummy)

  (func $init
    (table.set $t3 (i32.const 2) (table.get $t3 (i32.const 1)))
  )

  (func $getExtern (param $i i32) (result externref)
    (table.get $t2 (local.get $i))
  )
  (func $getFunc (param $i i32) (result funcref)
    (table.get $t3 (local.get $i))
  )

  (func $setFunc (param $ref funcref)
    (table.set $t3 (i32.const 0) (local.get $ref))
  )

  (func $isNullFunc (param $i i32) (result i32)
    (ref.is_null (call $getFunc (local.get $i)))
  )

  (func $test
    (call $init)
    (drop
      (call $getExtern (i32.const 0))
    )
    (drop
      (call $getFunc (i32.const 0))
    )
    (drop
      (call $isNullFunc (i32.const 0))
    )

    (table.set $t4
      (i32.const 0)
      (array.new_default_with_rtt $vec (i32.const 3) (rtt.canon $vec))
    )

    (drop
      (array.get $vec
        (ref.cast (table.get $t4 (i32.const 0)) (rtt.canon $vec))
        (i32.const 0)
      )
    )
  )
)