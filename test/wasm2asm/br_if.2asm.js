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
 var i64toi32_i32$HIGH_BITS = 0;
 function dummy() {
  
 }
 
 function $1($0) {
  $0 = $0 | 0;
  block : {
   if ($0) break block;
   return 2 | 0;
  };
  return 3 | 0;
 }
 
 function $2($0) {
  $0 = $0 | 0;
  block : {
   dummy();
   if ($0) break block;
   return 2 | 0;
  };
  return 3 | 0;
 }
 
 function $3($0) {
  $0 = $0 | 0;
  block : {
   dummy();
   dummy();
   if ($0) break block;
  };
 }
 
 function $4($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   $2 = 10;
   if ($0) break block;
   return 11 | 0;
  };
  return $2 | 0;
 }
 
 function $5($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   dummy();
   $2 = 20;
   if ($0) break block;
   return 21 | 0;
  };
  return $2 | 0;
 }
 
 function $6($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   dummy();
   dummy();
   $2 = 11;
   if ($0) break block;
   $2 = $2;
  };
  return $2 | 0;
 }
 
 function $7($0) {
  $0 = $0 | 0;
  block : {
   loop_in : do {
    if ($0) break block;
    return 2 | 0;
    break loop_in;
   } while (1);
  };
  return 3 | 0;
 }
 
 function $8($0) {
  $0 = $0 | 0;
  block : {
   loop_in : do {
    dummy();
    if ($0) break block;
    return 2 | 0;
    break loop_in;
   } while (1);
  };
  return 4 | 0;
 }
 
 function $9($0) {
  $0 = $0 | 0;
  fake_return_waka123 : {
   loop_in : do {
    dummy();
    if ($0) break fake_return_waka123;
    break loop_in;
   } while (1);
  };
 }
 
 function $10($0, $1) {
  $0 = $0 | 0;
  $1 = $1 | 0;
  block : {
   if ($0) {
    if ($1) break block;
   } else dummy();
  };
 }
 
 function $11($0, $1) {
  $0 = $0 | 0;
  $1 = $1 | 0;
  block : {
   if ($0) dummy(); else if ($1) break block;;
  };
 }
 
 function $12($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   block0 : {
    $2 = 8;
    if ($0) break block;
   };
   $2 = 4 + 16 | 0;
  };
  return 1 + $2 | 0 | 0;
 }
 
 function $13($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   block1 : {
    $2 = 8;
    if ($0) break block;
   };
   $2 = 4;
   break block;
  };
  return 1 + $2 | 0 | 0;
 }
 
 function $14($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   block2 : {
    $2 = 8;
    if ($0) break block;
   };
   $2 = 4;
   if (1) break block;
   $2 = 16;
  };
  return 1 + $2 | 0 | 0;
 }
 
 function $15($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   block3 : {
    $2 = 8;
    if ($0) break block;
   };
   $2 = 4;
   if (1) break block;
   $2 = 16;
  };
  return 1 + $2 | 0 | 0;
 }
 
 function $16($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   block4 : {
    $2 = 8;
    if ($0) break block;
   };
   $2 = 4;
   switch (1 | 0) {
   default:
    break block;
   };
  };
  return 1 + $2 | 0 | 0;
 }
 
 function $17($0) {
  $0 = $0 | 0;
  var $2 = 0;
  block : {
   block5 : {
    $2 = 8;
    if ($0) break block;
   };
   $2 = 4;
   switch (1 | 0) {
   default:
    break block;
   };
  };
  return 1 + $2 | 0 | 0;
 }
 
 return {
  as_block_first: $1, 
  as_block_mid: $2, 
  as_block_last: $3, 
  as_block_first_value: $4, 
  as_block_mid_value: $5, 
  as_block_last_value: $6, 
  as_loop_first: $7, 
  as_loop_mid: $8, 
  as_loop_last: $9, 
  as_if_then: $10, 
  as_if_else: $11, 
  nested_block_value: $12, 
  nested_br_value: $13, 
  nested_br_if_value: $14, 
  nested_br_if_value_cond: $15, 
  nested_br_table_value: $16, 
  nested_br_table_value_index: $17
 };
}

