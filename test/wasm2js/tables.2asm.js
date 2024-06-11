import * as env from 'env';

function asmFunc(imports) {
 var env = imports.env;
 var FUNCTION_TABLE = env.table;
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
 function table_get() {
  return FUNCTION_TABLE[1];
 }
 
 function table_set() {
  FUNCTION_TABLE[1] = table_set;
 }
 
 function table_size() {
  return FUNCTION_TABLE.length | 0;
 }
 
 function table_grow() {
  return wasm2js_table_grow(table_grow, 42) | 0;
 }
 
 function table_fill(dest, value, size) {
  dest = dest | 0;
  size = size | 0;
  wasm2js_table_fill(dest, value, size);
 }
 
 function table_copy(dest, source, size) {
  dest = dest | 0;
  source = source | 0;
  size = size | 0;
  wasm2js_table_copy(dest, source, size);
 }
 
 FUNCTION_TABLE[1] = table_get;
 return {
  "table_get": table_get, 
  "table_set": table_set, 
  "table_size": table_size, 
  "table_grow": table_grow, 
  "table_fill": table_fill, 
  "table_copy": table_copy
 };
}

var retasmFunc = asmFunc({
  "env": env,
});
export var table_get = retasmFunc.table_get;
export var table_set = retasmFunc.table_set;
export var table_size = retasmFunc.table_size;
export var table_grow = retasmFunc.table_grow;
export var table_fill = retasmFunc.table_fill;
export var table_copy = retasmFunc.table_copy;
