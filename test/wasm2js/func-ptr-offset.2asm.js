
function asmFunc(importObject) {
 var env = importObject.env || importObject;
 var Math_imul = Math.imul;
 var Math_fround = Math.fround;
 var Math_abs = Math.abs;
 var Math_clz32 = Math.clz32;
 var Math_min = Math.min;
 var Math_max = Math.max;
 var Math_floor = Math.floor;
 var Math_ceil = Math.ceil;
 var Math_trunc = Math.trunc;
 var Math_sqrt = Math.sqrt;
 var abort = env.abort;
 var nan = NaN;
 var infinity = Infinity;
 function t1() {
  return 1 | 0;
 }
 
 function t2() {
  return 2 | 0;
 }
 
 function t3() {
  return 3 | 0;
 }
 
 function $3($0) {
  $0 = $0 | 0;
  return FUNCTION_TABLE[$0 | 0]() | 0 | 0;
 }
 
 var FUNCTION_TABLE = [null, t1, t2, t3];
 return {
  "call": $3
 };
}

var retasmFunc = asmFunc(  { abort: function() { throw new Error('abort'); }
  });
export var call = retasmFunc.call;
