(module
 (memory 1 1)

 (func $i8x16.relaxed_swizzle (param $0 v128) (param $1 v128) (result v128)
  (i8x16.relaxed_swizzle
   (local.get $0)
   (local.get $1)
  )
 )

 (func $i32x4.relaxed_trunc_f32x4_s (param $0 v128) (result v128)
  (i32x4.relaxed_trunc_f32x4_s
   (local.get $0)
  )
 )
 (func $i32x4.relaxed_trunc_f32x4_u (param $0 v128) (result v128)
  (i32x4.relaxed_trunc_f32x4_u
   (local.get $0)
  )
 )
 (func $i32x4.relaxed_trunc_f64x2_s_zero (param $0 v128) (result v128)
  (i32x4.relaxed_trunc_f64x2_s_zero
   (local.get $0)
  )
 )
 (func $i32x4.relaxed_trunc_f64x2_u_zero (param $0 v128) (result v128)
  (i32x4.relaxed_trunc_f64x2_u_zero
   (local.get $0)
  )
 )

 (func $f32x4.relaxed_fma (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (f32x4.relaxed_fma
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )
 (func $f32x4.relaxed_fms (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (f32x4.relaxed_fms
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )
 (func $f64x2.relaxed_fma (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (f64x2.relaxed_fma
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )
 (func $f64x2.relaxed_fms (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (f64x2.relaxed_fms
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )

 (func $i8x16.laneselect (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (i8x16.laneselect
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )
 (func $i16x8.laneselect (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (i16x8.laneselect
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )
 (func $i32x4.laneselect (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (i32x4.laneselect
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )
 (func $i64x2.laneselect (param $0 v128) (param $1 v128) (param $2 v128) (result v128)
  (i64x2.laneselect
   (local.get $0)
   (local.get $1)
   (local.get $2)
  )
 )

 (func $f32x4.relaxed_min (param $0 v128) (param $1 v128) (result v128)
  (f32x4.relaxed_min
   (local.get $0)
   (local.get $1)
  )
 )
 (func $f32x4.relaxed_max (param $0 v128) (param $1 v128) (result v128)
  (f32x4.relaxed_max
   (local.get $0)
   (local.get $1)
  )
 )
 (func $f64x2.relaxed_min (param $0 v128) (param $1 v128) (result v128)
  (f64x2.relaxed_min
   (local.get $0)
   (local.get $1)
  )
 )
 (func $f64x2.relaxed_max (param $0 v128) (param $1 v128) (result v128)
  (f64x2.relaxed_max
   (local.get $0)
   (local.get $1)
  )
 )

)
