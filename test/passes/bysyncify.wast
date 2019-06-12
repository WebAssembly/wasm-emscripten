(module
  (memory 1 2)
  (import "env" "import" (func $import))
  (import "env" "import2" (func $import2 (result i32)))
  (import "env" "import3" (func $import3 (param i32)))
  (func $calls-import
    (call $import)
  )
  (func $calls-import2 (result i32)
    (local $temp i32)
    (local.set $temp (call $import2))
    (return (local.get $temp))
  )
  (func $calls-import2-drop
    (drop (call $import2))
  )
  (func $calls-nothing
    (drop (i32.eqz (i32.const 17)))
  )
  (func $many-locals (param $x i32) (result i32)
    (local $y i32)
    (loop $l
      (local.set $x
        (i32.add (local.get $y) (i32.const 1))
      )
      (local.set $y
        (i32.div_s (local.get $x) (i32.const 3))
      )
      (br_if $l (local.get $y))
    )
    (call $import)
    (return (local.get $y))
  )
  (func $calls-import2-if (param $x i32)
    (if (local.get $x)
      (call $import)
    )
  )
  (func $calls-import2-if-else (param $x i32)
    (if (local.get $x)
      (call $import3 (i32.const 1))
      (call $import3 (i32.const 2))
    )
  )
  (func $calls-import2-if-else-oneside (param $x i32) (result i32)
    (if (local.get $x)
      (return (i32.const 1))
      (call $import3 (i32.const 2))
    )
    (return (i32.const 3))
  )
  (func $calls-import2-if-else-oneside2 (param $x i32) (result i32)
    (if (local.get $x)
      (call $import3 (i32.const 1))
      (return (i32.const 2))
    )
    (return (i32.const 3))
  )
)

