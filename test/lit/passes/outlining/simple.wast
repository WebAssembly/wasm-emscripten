(module
  (tag $catch_a (param i32))
  (func $d
    (block $block_a
      	(drop (i32.const 20))
      	(drop (i32.const 10))
	)
	(block $block_b
		(drop (if (i32.const 0)
		  (then
			(i32.const 40))
		  (else
			(i32.const 5))
		))
	)
	(block $block_c
		(try $try_a
			(do
			  (nop)
			)
			(catch $catch_a
				(drop
				  (i32.const 8)
				)
			)
		)
  	)
  )
)
