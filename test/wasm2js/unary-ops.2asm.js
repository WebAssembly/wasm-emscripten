
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
 var i64toi32_i32$HIGH_BITS = 0;
 function $1($0) {
  $0 = $0 | 0;
  return __wasm_popcnt_i32($0) | 0;
 }
 
 function $6($0) {
  $0 = $0 | 0;
  return Math_clz32($0) | 0;
 }
 
 function $7($0) {
  $0 = $0 | 0;
  var $1_1 = 0;
  if ($0) {
   $1_1 = 31 - Math_clz32($0 ^ $0 + -1) | 0
  } else {
   $1_1 = 32
  }
  return $1_1 | 0;
 }
 
 function $8($0, $1_1, $2, $3) {
  $0 = $0 | 0;
  $1_1 = $1_1 | 0;
  $2 = $2 | 0;
  $3 = $3 | 0;
  var $4 = 0;
  $4 = Math_clz32($0) + 32 | 0;
  $0 = Math_clz32($1_1);
  return !$3 & ($2 | 0) == ((($0 | 0) == (32 | 0) ? $4 : $0) | 0);
 }
 
 function legalstub$2($0, $1_1, $2, $3) {
  return (__wasm_popcnt_i64($0, $1_1) | 0) == ($2 | 0) & ($3 | 0) == (i64toi32_i32$HIGH_BITS | 0);
 }
 
 function legalstub$3($0, $1_1, $2) {
  return !$2 & ($0 | 0) == ($1_1 | 0);
 }
 
 function legalstub$4($0, $1_1, $2) {
  return ($0 | 0) == ($1_1 | 0) & ($2 | 0) == $0 >> 31;
 }
 
 function legalstub$5($0, $1_1) {
  return !($0 | $1_1);
 }
 
 function legalstub$8($0, $1_1, $2, $3) {
  var $4 = 0, $5 = 0, $6_1 = 0, $7_1 = 0, $8_1 = 0, $9 = 0, $10 = 0, $11 = 0, $12 = 0, $13 = 0;
  $8_1 = $0;
  $5 = 32;
  $0 = $5 & 31;
  if (32 >>> 0 <= $5 >>> 0) {
   {
    $4 = $1_1 << $0;
    $6_1 = 0;
   }
  } else {
   {
    $4 = (1 << $0) - 1 & $1_1 >>> 32 - $0 | $4 << $0;
    $6_1 = $1_1 << $0;
   }
  }
  $9 = $8_1 | $6_1;
  $10 = $4 | $12;
  $11 = $2;
  $2 = 0;
  $1_1 = $3;
  $3 = 32;
  $0 = $3 & 31;
  if (32 >>> 0 <= $3 >>> 0) {
   {
    $2 = $1_1 << $0;
    $7_1 = 0;
   }
  } else {
   {
    $2 = (1 << $0) - 1 & $1_1 >>> 32 - $0 | $2 << $0;
    $7_1 = $1_1 << $0;
   }
  }
  return $8($9, $10, $11 | $7_1, $2 | $13);
 }
 
 function legalstub$9($0, $1_1, $2, $3) {
  return (__wasm_ctz_i64($0, $1_1) | 0) == ($2 | 0) & ($3 | 0) == (i64toi32_i32$HIGH_BITS | 0);
 }
 
 function __wasm_ctz_i64($0, $1_1) {
  var $2 = 0, $3 = 0, $4 = 0, $5 = 0, $6_1 = 0, $7_1 = 0;
  if ($0 | $1_1) {
   {
    $3 = 63;
    $6_1 = $3;
    $2 = $1_1 + -1 | 0;
    $4 = -1;
    $5 = $4 + $0 | 0;
    if ($5 >>> 0 < $4 >>> 0) {
     $2 = $2 + 1 | 0
    }
    $7_1 = Math_clz32($0 ^ $5) + 32 | 0;
    $0 = Math_clz32($1_1 ^ $2);
    $0 = ($0 | 0) == (32 | 0) ? $7_1 : $0;
    $1_1 = $6_1 - $0 | 0;
    i64toi32_i32$HIGH_BITS = 0 - ($3 >>> 0 < $0 >>> 0) | 0;
    return $1_1;
   }
  }
  i64toi32_i32$HIGH_BITS = 0;
  return 64;
 }
 
 function __wasm_popcnt_i32($0) {
  var $1_1 = 0, $2 = 0;
  label$2 : while (1) {
   $2 = $1_1;
   if ($0) {
    {
     $0 = $0 - 1 & $0;
     $1_1 = $1_1 + 1 | 0;
     continue;
    }
   }
   break;
  };
  return $2;
 }
 
 function __wasm_popcnt_i64($0, $1_1) {
  var $2 = 0, $3 = 0, $4 = 0, $5 = 0;
  label$2 : while (1) {
   $3 = $4;
   $2 = $5;
   if ($0 | $1_1) {
    {
     $2 = $0;
     $3 = 1;
     $0 = $2 - $3 & $2;
     $1_1 = $1_1 - ($2 >>> 0 < $3 >>> 0) & $1_1;
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
  "i32_popcnt": $1, 
  "check_popcnt_i64": legalstub$2, 
  "check_extend_ui32": legalstub$3, 
  "check_extend_si32": legalstub$4, 
  "check_eqz_i64": legalstub$5, 
  "i32_clz": $6, 
  "i32_ctz": $7, 
  "check_clz_i64": legalstub$8, 
  "check_ctz_i64": legalstub$9
 };
}

var memasmFunc = new ArrayBuffer(65536);
var retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); }},memasmFunc);
export var i32_popcnt = retasmFunc.i32_popcnt;
export var check_popcnt_i64 = retasmFunc.check_popcnt_i64;
export var check_extend_ui32 = retasmFunc.check_extend_ui32;
export var check_extend_si32 = retasmFunc.check_extend_si32;
export var check_eqz_i64 = retasmFunc.check_eqz_i64;
export var i32_clz = retasmFunc.i32_clz;
export var i32_ctz = retasmFunc.i32_ctz;
export var check_clz_i64 = retasmFunc.check_clz_i64;
export var check_ctz_i64 = retasmFunc.check_ctz_i64;
