
function wasm2js_trap() { throw new Error('abort'); }

function asmFunc(imports) {
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
 var g = 0;
 function test() {
  g = 0;
  wasm2js_trap();
 }
 
 return {
  "test": test
 };
}

var retasmFunc = asmFunc({
});
export var test = retasmFunc.test;
