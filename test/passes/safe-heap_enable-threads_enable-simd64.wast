(module
 (memory (shared i64 100 100))
 (func $loads
  (drop (i32.load (i64.const 1)))
  (drop (i32.atomic.load (i64.const 1)))
  (drop (i32.load offset=31 (i64.const 2)))
  (drop (i32.load align=2 (i64.const 3)))
  (drop (i32.load align=1 (i64.const 4)))
  (drop (i32.load8_s (i64.const 5)))
  (drop (i32.load16_u (i64.const 6)))
  (drop (i64.load8_s (i64.const 7)))
  (drop (i64.load16_u (i64.const 8)))
  (drop (i64.load32_s (i64.const 9)))
  (drop (i64.load align=4 (i64.const 10)))
  (drop (i64.load (i64.const 11)))
  (drop (f32.load (i64.const 12)))
  (drop (f64.load (i64.const 13)))
  (drop (v128.load (i64.const 14)))
 )
 (func $stores
  (i32.store (i64.const 1) (i32.const 100))
  (i32.atomic.store (i64.const 1) (i32.const 100))
  (i32.store offset=31 (i64.const 2) (i32.const 200))
  (i32.store align=2 (i64.const 3) (i32.const 300))
  (i32.store align=1 (i64.const 4) (i32.const 400))
  (i32.store8 (i64.const 5) (i32.const 500))
  (i32.store16 (i64.const 6) (i32.const 600))
  (i64.store8 (i64.const 7) (i64.const 700))
  (i64.store16 (i64.const 8) (i64.const 800))
  (i64.store32 (i64.const 9) (i64.const 900))
  (i64.store align=4 (i64.const 10) (i64.const 1000))
  (i64.store (i64.const 11) (i64.const 1100))
  (f32.store (i64.const 12) (f32.const 1200))
  (f64.store (i64.const 13) (f64.const 1300))
  (v128.store (i64.const 14) (v128.const i32x4 1 2 3 4))
 )
)
;; not shared
(module
 (memory i64 100 100)
 (func $loads
  (drop (i32.load (i64.const 1)))
 )
)
;; pre-existing
(module
 (type $FUNCSIG$v (func))
 (import "env" "DYNAMICTOP_PTR" (global $DYNAMICTOP_PTR i32))
 (import "env" "segfault" (func $segfault))
 (import "env" "alignfault" (func $alignfault))
 (memory $0 (shared i64 100 100))
 (func $actions
  (drop (i32.load (i64.const 1)))
  (i32.store (i64.const 1) (i32.const 100))
 )
)
