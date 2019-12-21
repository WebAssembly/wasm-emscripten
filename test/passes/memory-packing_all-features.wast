(module
  (import "env" "memory" (memory $0 2048 2048))
  (import "env" "memoryBase" (global $memoryBase i32))
  ;; nothing
)

(module
  (import "env" "memory" (memory $0 2048 2048))
  (import "env" "memoryBase" (global $memoryBase i32))
  (data (i32.const 4066) "") ;; empty
)

(module
  (import "env" "memory" (memory $0 2048 2048))
  (import "env" "memoryBase" (global $memoryBase i32))

  (data (global.get $memoryBase) "waka this cannot be optimized\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00we don't know where it will go")

  (data (i32.const 1024) "waka this CAN be optimized\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00we DO know where it will go")

  (data (i32.const 2000) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeros before")

  (data (i32.const 3000) "zeros after\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")

  (data (i32.const 4000) "zeros\00in\00the\00middle\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00nice skip here\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00another\00but no")

  (func $nonzero-size-init-of-active-will-trap
    (memory.init 0
      (i32.const 42)
      (i32.const 0)
      (i32.const 13)
    )
    (data.drop 0)
  )

  (func $nonzero-offset-init-of-active-will-trap
    (memory.init 0
      (i32.const 42)
      (i32.const 13)
      (i32.const 0)
    )
    (data.drop 0)
  )

  (func $zero-offset-size-init-of-active-may-trap
    (memory.init 0
      (i32.const 42)
      (i32.const 0)
      (i32.const 0)
    )
    (data.drop 0)
  )
)

(module
  (import "env" "memory" (memory $0 2048 2048))

  (data passive "not referenced, delete me") ;; 0

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes at start") ;; 1

  (func $zeroes-at-start
    (memory.init 1
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (data.drop 1)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes at start") ;; 2

  (func $zeroes-at-start-not-split
    (memory.init 2
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (memory.init 2
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (memory.init 2
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (data.drop 2)
  )

  (data passive "\00\00\00few zeroes at start") ;; 3

  (func $few-zeroes-at-start
    (memory.init 3
      (i32.const 0)
      (i32.const 0)
      (i32.const 22)
    )
    (data.drop 3)
  )

  (data passive "zeroes at end\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 4

  (func $zeroes-at-end
    (memory.init 4
      (i32.const 0)
      (i32.const 0)
      (i32.const 33)
    )
    (data.drop 4)
  )

  (data passive "zeroes at end\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 5

  (func $zeroes-at-end-not-split
    (memory.init 5
      (i32.const 0)
      (i32.const 0)
      (i32.const 33)
    )
    (memory.init 5
      (i32.const 0)
      (i32.const 0)
      (i32.const 33)
    )
    (memory.init 5
      (i32.const 0)
      (i32.const 0)
      (i32.const 33)
    )
    (data.drop 5)
  )

  (data passive "few zeroes at end\00\00\00") ;; 6

  (func $few-zeroes-at-end
    (memory.init 6
      (i32.const 0)
      (i32.const 0)
      (i32.const 20)
    )
    (data.drop 6)
  )

  (data passive "zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00in middle") ;; 7

  (func $zeroes-in-middle
    (memory.init 7
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (data.drop 7)
  )

  (data passive "zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00in middle") ;; 8

  (func $zeroes-in-middle-not-split
    (memory.init 8
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (memory.init 8
      (i32.const 0)
      (i32.const 0)
      (i32.const 35)
    )
    (data.drop 8)
  )

  (data passive "few zeroes\00\00\00in middle") ;; 9

  (func $few-zeroes-in-middle
    (memory.init 9
      (i32.const 0)
      (i32.const 0)
      (i32.const 22)
    )
    (data.drop 9)
  )

  (data passive "multiple\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00spans\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00of zeroes") ;; 10

  (func $multiple-spans-of-zeroes
    (memory.init 10
      (i32.const 0)
      (i32.const 0)
      (i32.const 62)
    )
    (data.drop 10)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 11

  (func $even-more-zeroes
    (memory.init 11
      (i32.const 0)
      (i32.const 0)
      (i32.const 94)
    )
    (data.drop 11)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 12

  (func $only-zeroes
    (memory.init 12
      (i32.const 0)
      (i32.const 0)
      (i32.const 20)
    )
    (data.drop 12)
  )

  (data passive "no zeroes") ;; 13

  (func $no-zeroes
    (memory.init 13
      (i32.const 0)
      (i32.const 0)
      (i32.const 9)
    )
    (data.drop 13)
  )

  (data passive "") ;; 14

  (func $empty
    (memory.init 14
      (i32.const 0)
      (i32.const 0)
      (i32.const 0)
    )
    (data.drop 14)
  )

  (data passive "only dropped") ;; 15

  (func $only-dropped
    (data.drop 15)
    (data.drop 15)
  )

  (data passive "\00\00\00\00\00") ;; 16

  (func $only-dropped-zeroes
    (data.drop 16)
    (data.drop 16)
  )

  (data passive "") ;; not referenced

  (data passive "\00\00\00\00\00") ;; not referenced
)

(module
  (import "env" "memory" (memory $0 2048 2048))
  (import "env" "param" (global $param i32))

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 0

  (func $nonconst-dest
    (memory.init 0
      (global.get $param)
      (i32.const 0)
      (i32.const 94)
    )
    (data.drop 0)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 1

  (func $nonconst-offset
    (memory.init 1
      (i32.const 0)
      (global.get $param)
      (i32.const 94)
    )
    (data.drop 1)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 2

  (func $nonconst-size
    (memory.init 2
      (i32.const 0)
      (i32.const 0)
      (global.get $param)
    )
    (data.drop 2)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 3

  (func $partial-skip-start
    (memory.init 3
      (i32.const 0)
      (i32.const 10)
      (i32.const 84)
    )
    (data.drop 3)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 4

  (func $full-skip-start
    (memory.init 4
      (i32.const 0)
      (i32.const 22)
      (i32.const 72)
    )
    (data.drop 4)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 5

  (func $partial-skip-end
    (memory.init 5
      (i32.const 0)
      (i32.const 0)
      (i32.const 84)
    )
    (data.drop 5)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 6

  (func $full-skip-end
    (memory.init 6
      (i32.const 0)
      (i32.const 0)
      (i32.const 72)
    )
    (data.drop 6)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 7

  (func $slice-zeroes
    (memory.init 7
      (i32.const 0)
      (i32.const 25)
      (i32.const 10)
    )
    (data.drop 7)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 8

  (func $slice-nonzeroes
    (memory.init 8
      (i32.const 0)
      (i32.const 21)
      (i32.const 2)
    )
    (data.drop 8)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 9

  (func $zero-size
    (memory.init 9
      (i32.const 0)
      (i32.const 40)
      (i32.const 0)
    )
    (data.drop 9)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 10

  (func $zero-size-undropped
    (memory.init 10
      (i32.const 0)
      (i32.const 40)
      (i32.const 0)
    )
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 11

  (func $out-of-bounds-offset
    (memory.init 11
      (i32.const 0)
      (i32.const 95)
      (i32.const 1)
    )
    (data.drop 11)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00more\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00zeroes\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00") ;; 12

  (func $out-of-bounds-size
    (memory.init 12
      (i32.const 0)
      (i32.const 0)
      (i32.const 95)
    )
    (data.drop 12)
  )
)

(module
  (import "env" "memory" (memory $0 2048 2048))
  (data passive "hi\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00hi") ;; 0

  (func $zero-length-init-zeroes
    (memory.init 0
      (i32.const 0)
      (i32.const 10)
      (i32.const 0)
    )
    (data.drop 0)
  )

  (data passive "hi\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00hi") ;; 1

  (func $zero-length-init-nonzeroes
    (memory.init 1
      (i32.const 0)
      (i32.const 23)
      (i32.const 0)
    )
    (data.drop 1)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00hi") ;; 2

  (func $zero-length-init-zeroes-todo
    (memory.init 2
      (i32.const 0)
      (i32.const 10)
      (i32.const 0)
    )
    (data.drop 2)
  )

  (data passive "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00even\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00hi") ;; 3

  (func $zero-length-init-nonzeroes-todo
    (memory.init 3
      (i32.const 0)
      (i32.const 21)
      (i32.const 0)
    )
    (data.drop 3)
  )
)
