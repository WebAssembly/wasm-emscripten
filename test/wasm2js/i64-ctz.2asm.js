import { setTempRet0 } from 'env';

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
 function legalstub$popcnt64($0, $1) {
  $0 = __wasm_popcnt_i64($0, $1);
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0;
 }
 
 function legalstub$ctz64($0, $1) {
  $0 = __wasm_ctz_i64($0, $1);
  setTempRet0(i64toi32_i32$HIGH_BITS | 0);
  return $0;
 }
 
 function __wasm_ctz_i64($0, $1) {
  var $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6 = 0, $7 = 0;
  if ($0 | $1) {
   {
    $3 = 63;
    $6 = $3;
    $2 = $1 + -1 | 0;
    $4 = -1;
    $5 = $4 + $0 | 0;
    if ($5 >>> 0 < $4 >>> 0) {
     $2 = $2 + 1 | 0
    }
    $7 = Math_clz32($0 ^ $5) + 32 | 0;
    $0 = Math_clz32($1 ^ $2);
    $0 = ($0 | 0) == (32 | 0) ? $7 : $0;
    $1 = $6 - $0 | 0;
    i64toi32_i32$HIGH_BITS = 0 - ($3 >>> 0 < $0 >>> 0) | 0;
    return $1;
   }
  }
  i64toi32_i32$HIGH_BITS = 0;
  return 64;
 }
 
 function __wasm_popcnt_i64($0, $1) {
  var $2 = 0, $3 = 0, $4 = 0, $5 = 0;
  label$2 : while (1) {
   $3 = $4;
   $2 = $5;
   if ($0 | $1) {
    {
     $2 = $0;
     $3 = 1;
     $0 = $2 - $3 & $2;
     $1 = $1 - ($2 >>> 0 < $3 >>> 0) & $1;
     $2 = 1 + $4 | 0;
     if ($2 >>> 0 < $3 >>> 0) {
      $5 = $5 + 1 | 0
     }
     $4 = $2;
     continue;
    }
   }
   break;
  };
  i64toi32_i32$HIGH_BITS = $2;
  return $3;
 }
 
 var FUNCTION_TABLE = [];
 return {
  "a": legalstub$popcnt64, 
  "b": legalstub$ctz64
 };
}

var memasmFunc = new ArrayBuffer(65536);
var retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); },setTempRet0},memasmFunc);
export var a = retasmFunc.a;
export var b = retasmFunc.b;
