
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
 function $1($0, $1_1, $2, $3, $4, $5) {
  $0 = $0 | 0;
  $1_1 = $1_1 | 0;
  $2 = $2 | 0;
  $3 = $3 | 0;
  $4 = $4 | 0;
  $5 = $5 | 0;
  $1_1 = $1_1 + $3 | 0;
  $0 = $0 + $2 | 0;
  if ($0 >>> 0 < $2 >>> 0) {
   $1_1 = $1_1 + 1 | 0
  }
  return ($0 | 0) == ($4 | 0) & ($1_1 | 0) == ($5 | 0);
 }
 
 function legalstub$1($0, $1_1, $2, $3, $4, $5) {
  var $6 = 0, $7 = 0, $8 = 0, $9 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0, $14 = 0, $15 = 0, $16 = 0, $17 = 0, $18 = 0, $19 = 0;
  $11 = $0;
  $7 = 32;
  $0 = $7 & 31;
  if (32 >>> 0 <= $7 >>> 0) {
   {
    $6 = $1_1 << $0;
    $8 = 0;
   }
  } else {
   {
    $6 = (1 << $0) - 1 & $1_1 >>> 32 - $0 | $6 << $0;
    $8 = $1_1 << $0;
   }
  }
  $12 = $11 | $8;
  $13 = $6 | $17;
  $14 = $2;
  $2 = 0;
  $1_1 = $3;
  $3 = 32;
  $0 = $3 & 31;
  if (32 >>> 0 <= $3 >>> 0) {
   {
    $2 = $1_1 << $0;
    $9 = 0;
   }
  } else {
   {
    $2 = (1 << $0) - 1 & $1_1 >>> 32 - $0 | $2 << $0;
    $9 = $1_1 << $0;
   }
  }
  $15 = $14 | $9;
  $16 = $2 | $18;
  $2 = 0;
  $1_1 = $5;
  $0 = 32 & 31;
  if (32 >>> 0 <= $3 >>> 0) {
   {
    $2 = $1_1 << $0;
    $10 = 0;
   }
  } else {
   {
    $2 = (1 << $0) - 1 & $1_1 >>> 32 - $0 | $2 << $0;
    $10 = $1_1 << $0;
   }
  }
  return $1($12, $13, $15, $16, $10 | $4, $2 | $19);
 }
 
 function legalstub$2($0, $1_1, $2, $3, $4, $5) {
  var $6 = 0;
  $6 = $2;
  $2 = $3;
  $3 = $6 | 0;
  return ($4 | 0) == ($0 - $3 | 0) & ($5 | 0) == ($1_1 - (($0 >>> 0 < $3 >>> 0) + $2 | 0) | 0);
 }
 
 var FUNCTION_TABLE = [];
 return {
  "check_add_i64": legalstub$1, 
  "check_sub_i64": legalstub$2
 };
}

var memasmFunc = new ArrayBuffer(65536);
var retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); }},memasmFunc);
export var check_add_i64 = retasmFunc.check_add_i64;
export var check_sub_i64 = retasmFunc.check_sub_i64;
