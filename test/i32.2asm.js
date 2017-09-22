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
 function $$0(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 + $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$1(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 - $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$2(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = Math_imul($$2, $$3);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$3(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) / ($$3 | 0) | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$4(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 >>> 0) / ($$3 >>> 0) | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$5(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) % ($$3 | 0) | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$6(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 >>> 0) % ($$3 >>> 0) | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$7(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 & $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$8(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 | $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$9(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 ^ $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$10(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 << $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$11(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 >> $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$12(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 >>> $$3 | 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$13(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = __wasm_rotl_i32($$2, $$3);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$14(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = __wasm_rotr_i32($$2, $$3);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$15(x) {
  x = x | 0;
  var $$1 = 0, $$2 = 0, wasm2asm_i32$0 = 0;
  $$1 = x;
  $$2 = Math_clz32($$1);
  wasm2asm_i32$0 = $$2;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$16(x) {
  x = x | 0;
  var $$1 = 0, $$2 = 0, wasm2asm_i32$0 = 0;
  $$1 = x;
  $$2 = __wasm_ctz_i32($$1);
  wasm2asm_i32$0 = $$2;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$17(x) {
  x = x | 0;
  var $$1 = 0, $$2 = 0, wasm2asm_i32$0 = 0;
  $$1 = x;
  $$2 = __wasm_popcnt_i32($$1);
  wasm2asm_i32$0 = $$2;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$18(x) {
  x = x | 0;
  var $$1 = 0, $$2 = 0, wasm2asm_i32$0 = 0;
  $$1 = x;
  $$2 = ($$1 | 0) == (0 | 0);
  wasm2asm_i32$0 = $$2;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$19(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) == ($$3 | 0);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$20(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) != ($$3 | 0);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$21(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) < ($$3 | 0);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$22(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 >>> 0 < $$3 >>> 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$23(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) <= ($$3 | 0);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$24(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 >>> 0 <= $$3 >>> 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$25(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) > ($$3 | 0);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$26(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 >>> 0 > $$3 >>> 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$27(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = ($$2 | 0) >= ($$3 | 0);
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function $$28(x, y) {
  x = x | 0;
  y = y | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, wasm2asm_i32$0 = 0;
  $$2 = x;
  $$3 = y;
  $$4 = $$2 >>> 0 >= $$3 >>> 0;
  wasm2asm_i32$0 = $$4;
  return wasm2asm_i32$0 | 0;
 }
 
 function __wasm_ctz_i32(x) {
  x = x | 0;
  var $$1 = 0, $$2 = 0, $$3 = 0, $$4 = 0, $$5 = 0, $$6 = 0, $$7 = 0, $$8 = 0, $$9 = 0, $$10 = 0, wasm2asm_i32$0 = 0;
  $$1 = x;
  $$2 = ($$1 | 0) == (0 | 0);
  if ($$2) $$9 = 32; else {
   $$3 = x;
   $$4 = x;
   $$5 = $$4 - 1 | 0;
   $$6 = $$3 ^ $$5 | 0;
   $$7 = Math_clz32($$6);
   $$8 = 31 - $$7 | 0;
   $$9 = $$8;
  }
  $$10 = $$9;
  wasm2asm_i32$0 = $$10;
  return wasm2asm_i32$0 | 0;
 }
 
 function __wasm_popcnt_i32(x) {
  x = x | 0;
  var count = 0, $$2 = 0, $$3 = 0, $$4 = 0, $$5 = 0, $$6 = 0, $$7 = 0, $$8 = 0, $$9 = 0, $$10 = 0, $$11 = 0, $$12 = 0, $$13 = 0, $$14 = 0, $$15 = 0, wasm2asm_i32$0 = 0;
  count = 0;
  b : {
   l : do {
    $$2 = count;
    $$3 = x;
    $$4 = ($$3 | 0) == (0 | 0);
    $$5 = $$2;
    if ($$4) break b;
    $$6 = $$5;
    $$7 = x;
    $$8 = x;
    $$9 = $$8 - 1 | 0;
    $$10 = $$7 & $$9 | 0;
    x = $$10;
    $$11 = count;
    $$12 = $$11 + 1 | 0;
    count = $$12;
    continue l;
    break l;
   } while (1);
  };
  $$13 = $$5;
  $$14 = $$13;
  $$15 = $$14;
  wasm2asm_i32$0 = $$15;
  return wasm2asm_i32$0 | 0;
 }
 
 function __wasm_rotl_i32(x, k) {
  x = x | 0;
  k = k | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, $$5 = 0, $$6 = 0, $$7 = 0, $$8 = 0, $$9 = 0, $$10 = 0, $$11 = 0, $$12 = 0, $$13 = 0, $$14 = 0, $$15 = 0, $$16 = 0, $$17 = 0, $$18 = 0, $$19 = 0, $$20 = 0, wasm2asm_i32$0 = 0;
  $$2 = k;
  $$3 = $$2 & 31 | 0;
  $$4 = 4294967295 >>> $$3 | 0;
  $$5 = x;
  $$6 = $$4 & $$5 | 0;
  $$7 = k;
  $$8 = $$7 & 31 | 0;
  $$9 = $$6 << $$8 | 0;
  $$10 = k;
  $$11 = $$10 & 31 | 0;
  $$12 = 32 - $$11 | 0;
  $$13 = 4294967295 << $$12 | 0;
  $$14 = x;
  $$15 = $$13 & $$14 | 0;
  $$16 = k;
  $$17 = $$16 & 31 | 0;
  $$18 = 32 - $$17 | 0;
  $$19 = $$15 >>> $$18 | 0;
  $$20 = $$9 | $$19 | 0;
  wasm2asm_i32$0 = $$20;
  return wasm2asm_i32$0 | 0;
 }
 
 function __wasm_rotr_i32(x, k) {
  x = x | 0;
  k = k | 0;
  var $$2 = 0, $$3 = 0, $$4 = 0, $$5 = 0, $$6 = 0, $$7 = 0, $$8 = 0, $$9 = 0, $$10 = 0, $$11 = 0, $$12 = 0, $$13 = 0, $$14 = 0, $$15 = 0, $$16 = 0, $$17 = 0, $$18 = 0, $$19 = 0, $$20 = 0, wasm2asm_i32$0 = 0;
  $$2 = k;
  $$3 = $$2 & 31 | 0;
  $$4 = 4294967295 << $$3 | 0;
  $$5 = x;
  $$6 = $$4 & $$5 | 0;
  $$7 = k;
  $$8 = $$7 & 31 | 0;
  $$9 = $$6 >>> $$8 | 0;
  $$10 = k;
  $$11 = $$10 & 31 | 0;
  $$12 = 32 - $$11 | 0;
  $$13 = 4294967295 >>> $$12 | 0;
  $$14 = x;
  $$15 = $$13 & $$14 | 0;
  $$16 = k;
  $$17 = $$16 & 31 | 0;
  $$18 = 32 - $$17 | 0;
  $$19 = $$15 << $$18 | 0;
  $$20 = $$9 | $$19 | 0;
  wasm2asm_i32$0 = $$20;
  return wasm2asm_i32$0 | 0;
 }
 
 return {
  add: $$0, 
  sub: $$1, 
  mul: $$2, 
  div_s: $$3, 
  div_u: $$4, 
  rem_s: $$5, 
  rem_u: $$6, 
  and: $$7, 
  or: $$8, 
  xor: $$9, 
  shl: $$10, 
  shr_s: $$11, 
  shr_u: $$12, 
  rotl: $$13, 
  rotr: $$14, 
  clz: $$15, 
  ctz: $$16, 
  popcnt: $$17, 
  eqz: $$18, 
  eq: $$19, 
  ne: $$20, 
  lt_s: $$21, 
  lt_u: $$22, 
  le_s: $$23, 
  le_u: $$24, 
  gt_s: $$25, 
  gt_u: $$26, 
  ge_s: $$27, 
  ge_u: $$28
 };
}

