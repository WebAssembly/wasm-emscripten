(module
 (memory 1)

 (func $rotl32 (param $0 i32) (param $1 i32) (result i32)
   (i32.rotl (local.get $0) (local.get $1)))
 (func $rotr32 (param $0 i32) (param $1 i32) (result i32)
   (i32.rotr (local.get $0) (local.get $1)))
 (func $rotl64 (param $0 i64) (param $1 i64) (result i64)
   (i64.rotl (local.get $0) (local.get $1)))
 (func $rotr64 (param $0 i64) (param $1 i64) (result i64)
   (i64.rotr (local.get $0) (local.get $1)))

 (func $nearest64 (param $0 f64) (result f64)
   (f64.nearest (local.get $0)))
 (func $nearest32 (param $0 f32) (result f32)
   (f32.nearest (local.get $0)))

 (func $trunc64 (param $0 f64) (result f64)
   (f64.trunc (local.get $0)))
 (func $trunc32 (param $0 f32) (result f32)
   (f32.trunc (local.get $0)))

 (func $popcnt32 (param $0 i32) (result i32)
   (i32.popcnt (local.get $0)))
 (func $ctz32 (param $0 i32) (result i32)
   (i32.ctz (local.get $0)))

 (func $i64_sdiv (param $0 i64) (param $1 i64) (result i64)
   (i64.div_s (local.get $0) (local.get $1)))
 (func $i64_udiv (param $0 i64) (param $1 i64) (result i64)
   (i64.div_u (local.get $0) (local.get $1)))
 (func $i64_srem (param $0 i64) (param $1 i64) (result i64)
   (i64.rem_s (local.get $0) (local.get $1)))
 (func $i64_urem (param $0 i64) (param $1 i64) (result i64)
   (i64.rem_u (local.get $0) (local.get $1)))
 (func $i64_mul (param $0 i64) (param $1 i64) (result i64)
   (i64.mul (local.get $0) (local.get $1)))
)

