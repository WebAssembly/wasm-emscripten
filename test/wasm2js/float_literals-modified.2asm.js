import { setTempRet0 } from 'env';


  var scratchBuffer = new ArrayBuffer(8);
  var i32ScratchView = new Int32Array(scratchBuffer);
  var f32ScratchView = new Float32Array(scratchBuffer);
  var f64ScratchView = new Float64Array(scratchBuffer);
  
  function wasm2js_scratch_load_i32(index) {
    return i32ScratchView[index];
  }
      
  function wasm2js_scratch_store_f64(value) {
    f64ScratchView[0] = value;
  }
      
function asmFunc(global, env, buffer) {
 "almost asm";
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
 var nan = global.NaN;
 var infinity = global.Infinity;
 var setTempRet0 = env.setTempRet0;
 var i64toi32_i32$HIGH_BITS = 0;
 function $0() {
  return 2143289344 | 0;
 }
 
 function $2() {
  return -4194304 | 0;
 }
 
 function $4() {
  return 2141192192 | 0;
 }
 
 function $5() {
  return -1 | 0;
 }
 
 function $6() {
  return 2139169605 | 0;
 }
 
 function $7() {
  return 2142257232 | 0;
 }
 
 function $8() {
  return -5587746 | 0;
 }
 
 function $9() {
  return 2139095040 | 0;
 }
 
 function $11() {
  return -8388608 | 0;
 }
 
 function $12() {
  return 0 | 0;
 }
 
 function $14() {
  return -2147483648 | 0;
 }
 
 function $15() {
  return 1086918619 | 0;
 }
 
 function $16() {
  return 1 | 0;
 }
 
 function $17() {
  return 8388608 | 0;
 }
 
 function $18() {
  return 2139095039 | 0;
 }
 
 function $19() {
  return 8388607 | 0;
 }
 
 function $20() {
  return 1149239296 | 0;
 }
 
 function $29() {
  return 1343554297 | 0;
 }
 
 function legalstub$30() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$32() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(-nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$34() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$35() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(-nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$36() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$37() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$38() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(-nan);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$39() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(infinity);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$41() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(-infinity);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$42() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(0.0);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$44() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(-0.0);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$45() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(6.283185307179586);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$46() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(5.0e-324);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$47() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(2.2250738585072014e-308);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$48() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(2.225073858507201e-308);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$49() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(1797693134862315708145274.0e284);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$50() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(1267650600228229401496703.0e6);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 function legalstub$59() {
  var $0_1 = 0, $1 = 0;
  wasm2js_scratch_store_f64(1.e+100);
  $0_1 = wasm2js_scratch_load_i32(1 | 0) | 0;
  $1 = wasm2js_scratch_load_i32(0 | 0) | 0;
  i64toi32_i32$HIGH_BITS = $0_1;
  $0_1 = $1;
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0_1;
 }
 
 var FUNCTION_TABLE = [];
 return {
  "f32_nan": $0, 
  "f32_positive_nan": $0, 
  "f32_negative_nan": $2, 
  "f32_plain_nan": $0, 
  "f32_informally_known_as_plain_snan": $4, 
  "f32_all_ones_nan": $5, 
  "f32_misc_nan": $6, 
  "f32_misc_positive_nan": $7, 
  "f32_misc_negative_nan": $8, 
  "f32_infinity": $9, 
  "f32_positive_infinity": $9, 
  "f32_negative_infinity": $11, 
  "f32_zero": $12, 
  "f32_positive_zero": $12, 
  "f32_negative_zero": $14, 
  "f32_misc": $15, 
  "f32_min_positive": $16, 
  "f32_min_normal": $17, 
  "f32_max_finite": $18, 
  "f32_max_subnormal": $19, 
  "f32_trailing_dot": $20, 
  "f32_dec_zero": $12, 
  "f32_dec_positive_zero": $12, 
  "f32_dec_negative_zero": $14, 
  "f32_dec_misc": $15, 
  "f32_dec_min_positive": $16, 
  "f32_dec_min_normal": $17, 
  "f32_dec_max_subnormal": $19, 
  "f32_dec_max_finite": $18, 
  "f32_dec_trailing_dot": $29, 
  "f64_nan": legalstub$30, 
  "f64_positive_nan": legalstub$30, 
  "f64_negative_nan": legalstub$32, 
  "f64_plain_nan": legalstub$30, 
  "f64_informally_known_as_plain_snan": legalstub$34, 
  "f64_all_ones_nan": legalstub$35, 
  "f64_misc_nan": legalstub$36, 
  "f64_misc_positive_nan": legalstub$37, 
  "f64_misc_negative_nan": legalstub$38, 
  "f64_infinity": legalstub$39, 
  "f64_positive_infinity": legalstub$39, 
  "f64_negative_infinity": legalstub$41, 
  "f64_zero": legalstub$42, 
  "f64_positive_zero": legalstub$42, 
  "f64_negative_zero": legalstub$44, 
  "f64_misc": legalstub$45, 
  "f64_min_positive": legalstub$46, 
  "f64_min_normal": legalstub$47, 
  "f64_max_subnormal": legalstub$48, 
  "f64_max_finite": legalstub$49, 
  "f64_trailing_dot": legalstub$50, 
  "f64_dec_zero": legalstub$42, 
  "f64_dec_positive_zero": legalstub$42, 
  "f64_dec_negative_zero": legalstub$44, 
  "f64_dec_misc": legalstub$45, 
  "f64_dec_min_positive": legalstub$46, 
  "f64_dec_min_normal": legalstub$47, 
  "f64_dec_max_subnormal": legalstub$48, 
  "f64_dec_max_finite": legalstub$49, 
  "f64_dec_trailing_dot": legalstub$59
 };
}

var memasmFunc = new ArrayBuffer(65536);
var retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); },setTempRet0},memasmFunc);
export var f32_nan = retasmFunc.f32_nan;
export var f32_positive_nan = retasmFunc.f32_positive_nan;
export var f32_negative_nan = retasmFunc.f32_negative_nan;
export var f32_plain_nan = retasmFunc.f32_plain_nan;
export var f32_informally_known_as_plain_snan = retasmFunc.f32_informally_known_as_plain_snan;
export var f32_all_ones_nan = retasmFunc.f32_all_ones_nan;
export var f32_misc_nan = retasmFunc.f32_misc_nan;
export var f32_misc_positive_nan = retasmFunc.f32_misc_positive_nan;
export var f32_misc_negative_nan = retasmFunc.f32_misc_negative_nan;
export var f32_infinity = retasmFunc.f32_infinity;
export var f32_positive_infinity = retasmFunc.f32_positive_infinity;
export var f32_negative_infinity = retasmFunc.f32_negative_infinity;
export var f32_zero = retasmFunc.f32_zero;
export var f32_positive_zero = retasmFunc.f32_positive_zero;
export var f32_negative_zero = retasmFunc.f32_negative_zero;
export var f32_misc = retasmFunc.f32_misc;
export var f32_min_positive = retasmFunc.f32_min_positive;
export var f32_min_normal = retasmFunc.f32_min_normal;
export var f32_max_finite = retasmFunc.f32_max_finite;
export var f32_max_subnormal = retasmFunc.f32_max_subnormal;
export var f32_trailing_dot = retasmFunc.f32_trailing_dot;
export var f32_dec_zero = retasmFunc.f32_dec_zero;
export var f32_dec_positive_zero = retasmFunc.f32_dec_positive_zero;
export var f32_dec_negative_zero = retasmFunc.f32_dec_negative_zero;
export var f32_dec_misc = retasmFunc.f32_dec_misc;
export var f32_dec_min_positive = retasmFunc.f32_dec_min_positive;
export var f32_dec_min_normal = retasmFunc.f32_dec_min_normal;
export var f32_dec_max_subnormal = retasmFunc.f32_dec_max_subnormal;
export var f32_dec_max_finite = retasmFunc.f32_dec_max_finite;
export var f32_dec_trailing_dot = retasmFunc.f32_dec_trailing_dot;
export var f64_nan = retasmFunc.f64_nan;
export var f64_positive_nan = retasmFunc.f64_positive_nan;
export var f64_negative_nan = retasmFunc.f64_negative_nan;
export var f64_plain_nan = retasmFunc.f64_plain_nan;
export var f64_informally_known_as_plain_snan = retasmFunc.f64_informally_known_as_plain_snan;
export var f64_all_ones_nan = retasmFunc.f64_all_ones_nan;
export var f64_misc_nan = retasmFunc.f64_misc_nan;
export var f64_misc_positive_nan = retasmFunc.f64_misc_positive_nan;
export var f64_misc_negative_nan = retasmFunc.f64_misc_negative_nan;
export var f64_infinity = retasmFunc.f64_infinity;
export var f64_positive_infinity = retasmFunc.f64_positive_infinity;
export var f64_negative_infinity = retasmFunc.f64_negative_infinity;
export var f64_zero = retasmFunc.f64_zero;
export var f64_positive_zero = retasmFunc.f64_positive_zero;
export var f64_negative_zero = retasmFunc.f64_negative_zero;
export var f64_misc = retasmFunc.f64_misc;
export var f64_min_positive = retasmFunc.f64_min_positive;
export var f64_min_normal = retasmFunc.f64_min_normal;
export var f64_max_subnormal = retasmFunc.f64_max_subnormal;
export var f64_max_finite = retasmFunc.f64_max_finite;
export var f64_trailing_dot = retasmFunc.f64_trailing_dot;
export var f64_dec_zero = retasmFunc.f64_dec_zero;
export var f64_dec_positive_zero = retasmFunc.f64_dec_positive_zero;
export var f64_dec_negative_zero = retasmFunc.f64_dec_negative_zero;
export var f64_dec_misc = retasmFunc.f64_dec_misc;
export var f64_dec_min_positive = retasmFunc.f64_dec_min_positive;
export var f64_dec_min_normal = retasmFunc.f64_dec_min_normal;
export var f64_dec_max_subnormal = retasmFunc.f64_dec_max_subnormal;
export var f64_dec_max_finite = retasmFunc.f64_dec_max_finite;
export var f64_dec_trailing_dot = retasmFunc.f64_dec_trailing_dot;
