function asmFunc(global, env, buffer) {
 "use asm";
 var HEAP8 = new global.Int8Array(buffer);
 var HEAP16 = new global.Int16Array(buffer);
 var HEAP32 = new global.Int32Array(buffer);
 var HEAPU8 = new global.Uint8Array(buffer);
 var HEAPU16 = new global.Uint16Array(buffer);
 var HEAPU32 = new global.Uint32Array(buffer);
 var HEAPF32 = new global.Float32Array(buffer);
 var HEAPF64 = new global.Float64Array(buffer);
 var Math_imul = global.Math.imul;
 var Math_fround = global.Math.fround;
 var Math_abs = global.Math.abs;
 var Math_clz32 = global.Math.clz32;
 var Math_min = global.Math.min;
 var Math_max = global.Math.max;
 var Math_floor = global.Math.floor;
 var Math_ceil = global.Math.ceil;
 var Math_sqrt = global.Math.sqrt;
 var abort = env.abort;
 var i64toi32_i32$HIGH_BITS = 0;
 function dummy() {
  
 }
 
 function $1($0, $0$hi, $1, $1$hi, r, r$hi) {
  $0 = $0 | 0;
  $0$hi = $0$hi | 0;
  $1 = $1 | 0;
  $1$hi = $1$hi | 0;
  r = r | 0;
  r$hi = r$hi | 0;
  var i64toi32_i32$0 = 0, i64toi32_i32$5 = 0, i64toi32_i32$3 = 0, i64toi32_i32$4 = 0, $5$hi = 0;
  i64toi32_i32$0 = $0$hi;
  i64toi32_i32$0 = $1$hi;
  i64toi32_i32$0 = $0$hi;
  i64toi32_i32$3 = $1;
  i64toi32_i32$4 = $0 + i64toi32_i32$3 | 0;
  i64toi32_i32$5 = i64toi32_i32$0 + $1$hi | 0;
  if (i64toi32_i32$4 >>> 0 < i64toi32_i32$3 >>> 0) i64toi32_i32$5 = i64toi32_i32$5 + 1 | 0;
  $5$hi = i64toi32_i32$5;
  i64toi32_i32$5 = r$hi;
  i64toi32_i32$5 = $5$hi;
  i64toi32_i32$0 = i64toi32_i32$4;
  i64toi32_i32$3 = r;
  return (i64toi32_i32$0 | 0) == (i64toi32_i32$3 | 0) & (i64toi32_i32$5 | 0) == (r$hi | 0) | 0 | 0;
 }
 
 function $2($0, $0$hi, $1, $1$hi, r, r$hi) {
  $0 = $0 | 0;
  $0$hi = $0$hi | 0;
  $1 = $1 | 0;
  $1$hi = $1$hi | 0;
  r = r | 0;
  r$hi = r$hi | 0;
  var i64toi32_i32$0 = 0, i64toi32_i32$5 = 0, i64toi32_i32$2 = 0, i64toi32_i32$3 = 0, $5$hi = 0;
  i64toi32_i32$0 = $0$hi;
  i64toi32_i32$0 = $1$hi;
  i64toi32_i32$0 = $0$hi;
  i64toi32_i32$2 = $0;
  i64toi32_i32$3 = $1;
  i64toi32_i32$5 = (i64toi32_i32$2 >>> 0 < i64toi32_i32$3 >>> 0) + $1$hi | 0;
  i64toi32_i32$5 = i64toi32_i32$0 - i64toi32_i32$5 | 0;
  $5$hi = i64toi32_i32$5;
  i64toi32_i32$5 = r$hi;
  i64toi32_i32$5 = $5$hi;
  i64toi32_i32$0 = i64toi32_i32$2 - i64toi32_i32$3 | 0;
  i64toi32_i32$2 = r$hi;
  i64toi32_i32$3 = r;
  return (i64toi32_i32$0 | 0) == (i64toi32_i32$3 | 0) & (i64toi32_i32$5 | 0) == (i64toi32_i32$2 | 0) | 0 | 0;
 }
 
 return {
  check_add_i64: $1, 
  check_sub_i64: $2
 };
}

