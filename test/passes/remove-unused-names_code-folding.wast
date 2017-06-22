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
  (func $ifs-blocks-big
    (if (i32.const 0)
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
    )
    (if (i32.const 0)
      (block
        (unreachable)
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
    )
    (if (i32.const 0)
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
      (block
        (unreachable)
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
    )
    (if (i32.const 0)
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
        (unreachable)
      )
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
    )
    (if (i32.const 0)
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
      )
      (block
        (drop (i32.add (i32.const 1) (i32.const 2)))
        (unreachable)
      )
    )
  )
  (func $ifs-blocks-long
    (if (i32.const 1)
      (block
        (drop (i32.const -1234))
        (drop (i32.const -1000))
        (drop (i32.const 1))
        (nop)
        (unreachable)
      )
      (block
        (drop (i32.const 999))
        (drop (i32.const 1))
        (nop)
        (unreachable)
      )
    )
    (drop
      (if (result i32) (i32.const 2)
        (block (result i32)
          (drop (i32.const -1234))
          (drop (i32.const -1000))
          (drop (i32.const 1))
          (nop)
          (unreachable)
          (i32.const 2)
        )
        (block (result i32)
          (drop (i32.const 999))
          (drop (i32.const 1))
          (nop)
          (unreachable)
          (i32.const 2)
        )
      )
    )
    (drop
      (if (result i32) (i32.const 3)
        (block (result i32)
          (drop (i32.const -1234))
          (drop (i32.const -1000))
          (drop (i32.const 1))
          (nop)
          (i32.const 2)
        )
        (block (result i32)
          (drop (i32.const 999))
          (drop (i32.const 1))
          (nop)
          (i32.const 2)
        )
      )
    )
  )
  (func $if-worth-it-i-dunno
    ;; just 2, so not worth it
    (if (i32.const 0)
      (block
        (drop (i32.const -1234))
        (drop (i32.const -1000))
        (unreachable)
        (unreachable)
      )
      (block
        (drop (i32.const 999))
        (drop (i32.const 1))
        (unreachable)
        (unreachable)
      )
    )
    ;; 3, so why not
    (if (i32.const 0)
      (block
        (drop (i32.const -1234))
        (drop (i32.const -1000))
        (unreachable)
        (unreachable)
        (unreachable)
      )
      (block
        (drop (i32.const 999))
        (drop (i32.const 1))
        (unreachable)
        (unreachable)
        (unreachable)
      )
    )
    ;; just 2, but we'll empty out a block
    (if (i32.const 0)
      (block
        (unreachable)
        (unreachable)
      )
      (block
        (drop (i32.const 999))
        (drop (i32.const 1))
        (unreachable)
        (unreachable)
      )
    )
    ;; just 2, but we'll empty out a block
    (if (i32.const 0)
      (block
        (drop (i32.const -1234))
        (drop (i32.const -1000))
        (unreachable)
        (unreachable)
      )
      (block
        (unreachable)
        (unreachable)
      )
    )
  )
  (func $ifs-named-block (param $x i32) (param $y i32) (result i32)
    (block $out
      (block $out2
        (if (get_local $x)
          (block
            (br_if $out (get_local $y i32))
            (nop)
          )
          (block
            (br_if $out (get_local $y i32))
            (nop)
          )
        )
        (if (get_local $x)
          (block
            (br_if $out (get_local $y i32))
            (nop)
          )
          (block
            (br_if $out2 (get_local $y i32))
            (nop)
          )
        )
        (if (get_local $x)
          (block
            (nop)
            (br_if $out (get_local $y i32))
            (nop)
          )
          (block
            (nop)
            (br_if $out2 (get_local $y i32))
            (nop)
          )
        )
        (if (get_local $x)
          (block $left
            (br_if $left (get_local $y i32))
            (nop)
          )
          (block
            (br_if $out (get_local $y i32))
            (nop)
          )
        )
        (if (get_local $x)
          (block
            (br_if $out (get_local $y i32))
            (nop)
          )
          (block $right
            (br_if $right (get_local $y i32))
            (nop)
          )
        )
      )
      (return (i32.const 10))
    )
    (return (i32.const 20))
  )
  (func $block
    (block $x
      (if (i32.const 0)
        (block
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br $x)
        )
      )
      (if (i32.const 0)
        (block
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br $x)
        )
      )
      ;; no fallthrough, another thing to merge
      (drop (i32.const 1))
      (drop (i32.const 2))
      (br $x)
    )
  )
  (func $block2
    (block $x
      (if (i32.const 0)
        (block
          (drop (i32.const 1))
          (drop (i32.const 333333))
          (br $x)
        )
      )
      (if (i32.const 0)
        (block
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br $x)
        )
      )
      ;; no fallthrough, another thing to merge
      (drop (i32.const 1))
      (drop (i32.const 2))
      (br $x)
    )
  )
  (func $block3
    (block $x
      (if (i32.const 0)
        (block
          (drop (i32.const 1000))
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br $x)
        )
      )
      (if (i32.const 0)
        (block
          (drop (i32.const 2000))
          (drop (i32.const 3000))
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br $x)
        )
      )
      (drop (i32.const 4000))
      (drop (i32.const 5000))
      (drop (i32.const 6000))
      ;; no fallthrough, another thing to merge
      (drop (i32.const 1))
      (drop (i32.const 2))
      (br $x)
    )
  )
  (func $mixture
    (block $out ;; then we reach the block, and the tail infos are stale, should ignore
      (if (i32.const 1) ;; then we optimize the if, pushing those brs outside!
        (block
          (drop (i32.const 2)) ;; first we note the block tails for $out
          (nop) (nop) (nop) (nop) (nop) (nop) ;; totally worth it
          (br $out)
        )
        (block
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out)
        )
      )
    )
    (block $out2
      (if (i32.const 1)
        (block
          (drop (i32.const 3)) ;; leave something
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out2)
        )
        (block
          (drop (i32.const 4)) ;; leave something
          (drop (i32.const 5)) ;; leave something
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out2)
        )
      )
    )
    ;; now a case where do **do** want to fold for the block (which we can only do in a later pass)
    (block $out3
      (if (i32.const 1)
        (block
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out3)
        )
        (block
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out3)
        )
      )
      (if (i32.const 1)
        (block
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out3)
        )
        (block
          (drop (i32.const 2))
          (nop) (nop) (nop) (nop) (nop) (nop)
          (br $out3)
        )
      )
      (drop (i32.const 2))
      (nop) (nop) (nop) (nop) (nop) (nop)
      (br $out3)
    )
  )
  (func $block-corners
    ;; these should be merged
    (block $x
      (if (i32.const 0)
        (block
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br $x)
        )
      )
      (drop (i32.const 1))
      (drop (i32.const 2))
    )
    ;; these should not
    ;; values
    (drop
      (block $y (result i32)
        (if (i32.const 0)
          (block
            (drop (i32.const 1))
            (drop (i32.const 2))
            (br $y (i32.const 3))
          )
        )
        (drop (i32.const 1))
        (drop (i32.const 2))
        (br $y (i32.const 3))
      )
    )
    (drop
      (block $z (result i32)
        (if (i32.const 0)
          (block
            (drop (i32.const 1))
            (drop (i32.const 2))
            (br $z (i32.const 2))
          )
        )
        (drop (i32.const 1))
        (drop (i32.const 2))
        (i32.const 3)
      )
    )
    ;; condition
    (block $w
      (if (i32.const 0)
        (block
          (drop (i32.const 1))
          (drop (i32.const 2))
          (br_if $w (i32.const 3))
        )
      )
      (drop (i32.const 1))
      (drop (i32.const 2))
    )
  )
)
