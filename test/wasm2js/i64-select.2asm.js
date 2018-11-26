import { p } from 'env';
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
 var nan = global.NaN;
 var infinity = global.Infinity;
 var p = env.p;
 var i64toi32_i32$HIGH_BITS = 0;
 function $0($0_1) {
  $0_1 = $0_1 | 0;
  var i64toi32_i32$1 = 0, i64toi32_i32$0 = 0, $9 = 0, $1_1 = 0, $1$hi = 0, $2 = 0, $2$hi = 0, i64toi32_i32$2 = 0, i64toi32_i32$3 = 0;
  i64toi32_i32$0 = p(4294967295 | 0) | 0;
  i64toi32_i32$1 = i64toi32_i32$HIGH_BITS;
  $1_1 = i64toi32_i32$0;
  $1$hi = i64toi32_i32$1;
  i64toi32_i32$1 = p(0 | 0) | 0;
  i64toi32_i32$0 = i64toi32_i32$HIGH_BITS;
  $2 = i64toi32_i32$1;
  $2$hi = i64toi32_i32$0;
  i64toi32_i32$0 = $1$hi;
  i64toi32_i32$2 = $1_1;
  i64toi32_i32$1 = $2$hi;
  i64toi32_i32$3 = $2;
  i64toi32_i32$1 = i64toi32_i32$0;
  $9 = i64toi32_i32$2;
  i64toi32_i32$1 = i64toi32_i32$1;
  i64toi32_i32$0 = $9;
  i64toi32_i32$HIGH_BITS = i64toi32_i32$1;
  return i64toi32_i32$0 | 0;
 }
 
 function $1() {
  var i64toi32_i32$1 = 0, i64toi32_i32$0 = 0, $4 = 0, i64toi32_i32$2 = 0, i64toi32_i32$3 = 0;
  i64toi32_i32$0 = 4294967295;
  i64toi32_i32$2 = 4294967295;
  i64toi32_i32$1 = 0;
  i64toi32_i32$3 = 0;
  i64toi32_i32$1 = i64toi32_i32$0;
  $4 = i64toi32_i32$2;
  i64toi32_i32$1 = i64toi32_i32$1;
  i64toi32_i32$0 = $4;
  i64toi32_i32$HIGH_BITS = i64toi32_i32$1;
  return i64toi32_i32$0 | 0;
 }
 
 return {
  
 };
}

const memasmFunc = new ArrayBuffer(65536);
const retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); },p},memasmFunc);
