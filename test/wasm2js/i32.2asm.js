
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
 function $0($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 + $1_1 | 0;
 }
 
 function $1($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 - $1_1 | 0;
 }
 
 function $2($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return Math_imul($0_1, $1_1) | 0;
 }
 
 function $3($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) / ($1_1 | 0) | 0;
 }
 
 function $4($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 >>> 0) / ($1_1 >>> 0) | 0;
 }
 
 function $5($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) % ($1_1 | 0) | 0;
 }
 
 function $6($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 >>> 0) % ($1_1 >>> 0) | 0;
 }
 
 function $7($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 & $1_1;
 }
 
 function $8($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 | $1_1;
 }
 
 function $9($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 ^ $1_1;
 }
 
 function $10($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 << $1_1;
 }
 
 function $11($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 >> $1_1;
 }
 
 function $12($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 >>> $1_1;
 }
 
 function $13($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return __wasm_rotl_i32($0_1, $1_1) | 0;
 }
 
 function $14($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return __wasm_rotr_i32($0_1, $1_1) | 0;
 }
 
 function $15($0_1) {
  $0_1 = $0_1 | 0;
  return Math_clz32($0_1) | 0;
 }
 
 function $16($0_1) {
  $0_1 = $0_1 | 0;
  var $1_1 = 0;
  if ($0_1) {
   $1_1 = 31 - Math_clz32($0_1 ^ $0_1 + -1) | 0
  } else {
   $1_1 = 32
  }
  return $1_1 | 0;
 }
 
 function $17($0_1) {
  $0_1 = $0_1 | 0;
  return __wasm_popcnt_i32($0_1) | 0;
 }
 
 function $18($0_1) {
  $0_1 = $0_1 | 0;
  return !$0_1 | 0;
 }
 
 function $19($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) == ($1_1 | 0) | 0;
 }
 
 function $20($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) != ($1_1 | 0) | 0;
 }
 
 function $21($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) < ($1_1 | 0) | 0;
 }
 
 function $22($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 >>> 0 < $1_1 >>> 0 | 0;
 }
 
 function $23($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) <= ($1_1 | 0) | 0;
 }
 
 function $24($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 >>> 0 <= $1_1 >>> 0 | 0;
 }
 
 function $25($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) > ($1_1 | 0) | 0;
 }
 
 function $26($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 >>> 0 > $1_1 >>> 0 | 0;
 }
 
 function $27($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return ($0_1 | 0) >= ($1_1 | 0) | 0;
 }
 
 function $28($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $0_1 >>> 0 >= $1_1 >>> 0 | 0;
 }
 
 function __wasm_popcnt_i32($0_1) {
  var $1_1 = 0, $2_1 = 0;
  label$2 : while (1) {
   $2_1 = $1_1;
   if ($0_1) {
    {
     $0_1 = $0_1 - 1 & $0_1;
     $1_1 = $1_1 + 1 | 0;
     continue;
    }
   }
   break;
  };
  return $2_1;
 }
 
 function __wasm_rotl_i32($0_1, $1_1) {
  var $2_1 = 0;
  $2_1 = $1_1 & 31;
  $1_1 = 0 - $1_1 & 31;
  return ($0_1 & -1 >>> $2_1) << $2_1 | ($0_1 & -1 << $1_1) >>> $1_1;
 }
 
 function __wasm_rotr_i32($0_1, $1_1) {
  var $2_1 = 0;
  $2_1 = $1_1 & 31;
  $1_1 = 0 - $1_1 & 31;
  return ($0_1 & -1 << $2_1) >>> $2_1 | ($0_1 & -1 >>> $1_1) << $1_1;
 }
 
 var FUNCTION_TABLE = [];
 return {
  "add": $0, 
  "sub": $1, 
  "mul": $2, 
  "div_s": $3, 
  "div_u": $4, 
  "rem_s": $5, 
  "rem_u": $6, 
  "and": $7, 
  "or": $8, 
  "xor": $9, 
  "shl": $10, 
  "shr_s": $11, 
  "shr_u": $12, 
  "rotl": $13, 
  "rotr": $14, 
  "clz": $15, 
  "ctz": $16, 
  "popcnt": $17, 
  "eqz": $18, 
  "eq": $19, 
  "ne": $20, 
  "lt_s": $21, 
  "lt_u": $22, 
  "le_s": $23, 
  "le_u": $24, 
  "gt_s": $25, 
  "gt_u": $26, 
  "ge_s": $27, 
  "ge_u": $28
 };
}

var memasmFunc = new ArrayBuffer(65536);
var retasmFunc = asmFunc({Math,Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array,NaN,Infinity}, {abort:function() { throw new Error('abort'); }},memasmFunc);
export var add = retasmFunc.add;
export var sub = retasmFunc.sub;
export var mul = retasmFunc.mul;
export var div_s = retasmFunc.div_s;
export var div_u = retasmFunc.div_u;
export var rem_s = retasmFunc.rem_s;
export var rem_u = retasmFunc.rem_u;
export var and = retasmFunc.and;
export var or = retasmFunc.or;
export var xor = retasmFunc.xor;
export var shl = retasmFunc.shl;
export var shr_s = retasmFunc.shr_s;
export var shr_u = retasmFunc.shr_u;
export var rotl = retasmFunc.rotl;
export var rotr = retasmFunc.rotr;
export var clz = retasmFunc.clz;
export var ctz = retasmFunc.ctz;
export var popcnt = retasmFunc.popcnt;
export var eqz = retasmFunc.eqz;
export var eq = retasmFunc.eq;
export var ne = retasmFunc.ne;
export var lt_s = retasmFunc.lt_s;
export var lt_u = retasmFunc.lt_u;
export var le_s = retasmFunc.le_s;
export var le_u = retasmFunc.le_u;
export var gt_s = retasmFunc.gt_s;
export var gt_u = retasmFunc.gt_u;
export var ge_s = retasmFunc.ge_s;
export var ge_u = retasmFunc.ge_u;
