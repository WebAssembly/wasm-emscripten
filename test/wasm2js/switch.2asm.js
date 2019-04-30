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
 function $0($0_1) {
  $0_1 = $0_1 | 0;
  var $1_1 = 0;
  $1_1 = 100;
  switch_ : {
   default_ : {
    $5 : {
     $3_1 : {
      $0_2 : {
       switch ($0_1 | 0) {
       case 0:
        break $0_2;
       case 1:
       case 2:
       case 3:
        break $3_1;
       case 5:
        break $5;
       case 4:
       case 7:
        break switch_;
       default:
        break default_;
       };
      }
      return $0_1 | 0;
     }
     $1_1 = 0 - $0_1 | 0;
     break switch_;
    }
    $1_1 = 101;
    break switch_;
   }
   $1_1 = 102;
  }
  return $1_1 | 0;
 }
 
 function $1($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  var $2_1 = 0, $3_2 = 0;
  $2_1 = 100;
  switch_ : {
   $7 : {
    $6 : {
     $3_1 : {
      $0_2 : {
       switch ($0_1 | 0) {
       case 0:
        break $0_2;
       case 1:
       case 2:
       case 3:
        break $3_1;
       case 6:
        break $6;
       case 7:
        break $7;
       default:
        break switch_;
       };
      }
      i64toi32_i32$HIGH_BITS = $1_1;
      return $0_1 | 0;
     }
     $2_1 = 0 - $0_1 | 0;
     $3_2 = 0 - ($1_1 + ($3_2 >>> 0 < $0_1 >>> 0) | 0) | 0;
     break switch_;
    }
    $2_1 = 101;
    break switch_;
   }
   $2_1 = -5;
   $3_2 = -1;
  }
  i64toi32_i32$HIGH_BITS = $3_2;
  return $2_1 | 0;
 }
 
 function $2($0_1) {
  $0_1 = $0_1 | 0;
  var $1_1 = 0, $2_1 = 0, $3_2 = 0, $4 = 0;
  $1_1 = $0_1 << 1;
  $2_1 = $1_1;
  $3_2 = $1_1;
  $4 = $1_1;
  $2_2 : {
   $1_2 : {
    $0_2 : {
     default_ : {
      switch ($0_1 & 3) {
      case 0:
       break $0_2;
      case 1:
       break $1_2;
      case 2:
       break $2_2;
      default:
       break default_;
      };
     }
     $2_1 = $1_1 + 1e3 | 0;
    }
    $3_2 = $2_1 + 100 | 0;
   }
   $4 = $3_2 + 10 | 0;
  }
  return $4 | 0;
 }
 
 function $3() {
  return 1 | 0;
 }
 
 function legalstub$1($0_1, $1_1) {
  var $2_1 = 0, $3_2 = 0, $4 = 0, $5_1 = 0, $6_1 = 0;
  $5_1 = $0_1;
  $3_2 = 32;
  $0_1 = $3_2 & 31;
  if (32 >>> 0 <= $3_2 >>> 0) {
   {
    $2_1 = $1_1 << $0_1;
    $4 = 0;
   }
  } else {
   {
    $2_1 = (1 << $0_1) - 1 & $1_1 >>> 32 - $0_1 | $2_1 << $0_1;
    $4 = $1_1 << $0_1;
   }
  }
  $1_1 = $1($5_1 | $4, $2_1 | $6_1);
  $6_1 = $1_1;
  $2_1 = i64toi32_i32$HIGH_BITS;
  $0_1 = 32 & 31;
  setTempRet0((32 >>> 0 <= $3_2 >>> 0 ? $2_1 >>> $0_1 : ((1 << $0_1) - 1 & $2_1) << 32 - $0_1 | $1_1 >>> $0_1) | 0);
  return $1_1;
 }
 
 var FUNCTION_TABLE = [];
 return {
  "stmt": $0, 
  "expr": legalstub$1, 
  "arg": $2, 
  "corner": $3
 };
}

var memasmFunc = new ArrayBuffer(65536);
var retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); },setTempRet0},memasmFunc);
export var stmt = retasmFunc.stmt;
export var expr = retasmFunc.expr;
export var arg = retasmFunc.arg;
export var corner = retasmFunc.corner;
