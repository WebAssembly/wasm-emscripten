// kitchen sink, tests the full API

function cleanInfo(info) {
  var ret = {};
  for (var x in info) {
    if (x !== 'value') {
      ret[x] = info[x];
    }
  }
  return ret;
}

var module;

// helpers

var v128_bytes = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16];

function makeInt32(x) {
  return module.i32.const(x);
}

function makeFloat32(x) {
  return module.f32.const(x);
}

function makeInt64(x) {
  return module.i64.const(x);
}

function makeFloat64(x) {
  return module.f64.const(x);
}

function makeVec128(i8s) {
  return module.v128.const(i8s)
}

function makeSomething() {
  return makeInt32(1337);
}

function makeDroppedInt32(x) {
  return module.drop(module.i32.const(x));
}

// tests

function test_types() {
  console.log("  // BinaryenTypeNone: " + binaryen.none);
  console.log("  //", binaryen.expandType(binaryen.none).join(","));

  console.log("  // BinaryenTypeUnreachable: " + binaryen.unreachable);
  console.log("  //", binaryen.expandType(binaryen.unreachable).join(","));

  console.log("  // BinaryenTypeInt32: " + binaryen.i32);
  console.log("  //", binaryen.expandType(binaryen.i32).join(","));

  console.log("  // BinaryenTypeInt64: " + binaryen.i64);
  console.log("  //", binaryen.expandType(binaryen.i64).join(","));

  console.log("  // BinaryenTypeFloat32: " + binaryen.f32);
  console.log("  //", binaryen.expandType(binaryen.f32).join(","));

  console.log("  // BinaryenTypeFloat64: " + binaryen.f64);
  console.log("  //", binaryen.expandType(binaryen.f64).join(","));

  console.log("  // BinaryenTypeVec128: " + binaryen.v128);
  console.log("  //", binaryen.expandType(binaryen.v128).join(","));

  console.log("  // BinaryenTypeAuto: " + binaryen.auto);

  var i32_pair = binaryen.createType([binaryen.i32, binaryen.i32]);
  console.log("  //", binaryen.expandType(i32_pair).join(","));

  var duplicate_pair = binaryen.createType([binaryen.i32, binaryen.i32]);
  console.log("  //", binaryen.expandType(duplicate_pair).join(","));

  assert(i32_pair == duplicate_pair);

  var f32_pair = binaryen.createType([binaryen.f32, binaryen.f32]);
  console.log("  //", binaryen.expandType(f32_pair).join(","));
}

function test_features() {
  console.log("Features.MVP: " + binaryen.Features.MVP);
  console.log("Features.Atomics: " + binaryen.Features.Atomics);
  console.log("Features.BulkMemory: " + binaryen.Features.BulkMemory);
  console.log("Features.MutableGlobals: " + binaryen.Features.MutableGlobals);
  console.log("Features.NontrappingFPToInt: " + binaryen.Features.NontrappingFPToInt);
  console.log("Features.SignExt: " + binaryen.Features.SignExt);
  console.log("Features.SIMD128: " + binaryen.Features.SIMD128);
  console.log("Features.ExceptionHandling: " + binaryen.Features.ExceptionHandling);
  console.log("Features.TailCall: " + binaryen.Features.TailCall);
  console.log("Features.ReferenceTypes: " + binaryen.Features.ReferenceTypes);
  console.log("Features.Multivalue: " + binaryen.Features.Multivalue);
  console.log("Features.GC: " + binaryen.Features.GC);
  console.log("Features.Memory64: " + binaryen.Features.Memory64);
  console.log("Features.RelaxedSIMD: " + binaryen.Features.RelaxedSIMD);
  console.log("Features.ExtendedConst: " + binaryen.Features.ExtendedConst);
  console.log("Features.Strings: " + binaryen.Features.Strings);
  console.log("Features.MultiMemory: " + binaryen.Features.MultiMemory);
  console.log("Features.All: " + binaryen.Features.All);
}

function test_ids() {
  console.log("InvalidId: " + binaryen.InvalidId);
  console.log("BlockId: " + binaryen.BlockId);
  console.log("IfId: " + binaryen.IfId);
  console.log("LoopId: " + binaryen.LoopId);
  console.log("BreakId: " + binaryen.BreakId);
  console.log("SwitchId: " + binaryen.SwitchId);
  console.log("CallId: " + binaryen.CallId);
  console.log("CallIndirectId: " + binaryen.CallIndirectId);
  console.log("LocalGetId: " + binaryen.LocalGetId);
  console.log("LocalSetId: " + binaryen.LocalSetId);
  console.log("GlobalGetId: " + binaryen.GlobalGetId);
  console.log("GlobalSetId: " + binaryen.GlobalSetId);
  console.log("LoadId: " + binaryen.LoadId);
  console.log("StoreId: " + binaryen.StoreId);
  console.log("ConstId: " + binaryen.ConstId);
  console.log("UnaryId: " + binaryen.UnaryId);
  console.log("BinaryId: " + binaryen.BinaryId);
  console.log("SelectId: " + binaryen.SelectId);
  console.log("DropId: " + binaryen.DropId);
  console.log("ReturnId: " + binaryen.ReturnId);
  console.log("MemorySizeId: " + binaryen.MemorySizeId);
  console.log("MemoryGrowId: " + binaryen.MemoryGrowId);
  console.log("NopId: " + binaryen.NopId);
  console.log("UnreachableId: " + binaryen.UnreachableId);
  console.log("AtomicCmpxchgId: " + binaryen.AtomicCmpxchgId);
  console.log("AtomicRMWId: " + binaryen.AtomicRMWId);
  console.log("AtomicWaitId: " + binaryen.AtomicWaitId);
  console.log("AtomicNotifyId: " + binaryen.AtomicNotifyId);
  console.log("SIMDExtractId: " + binaryen.SIMDExtractId);
  console.log("SIMDReplaceId: " + binaryen.SIMDReplaceId);
  console.log("SIMDShuffleId: " + binaryen.SIMDShuffleId);
  console.log("SIMDTernaryId: " + binaryen.SIMDTernaryId);
  console.log("SIMDShiftId: " + binaryen.SIMDShiftId);
  console.log("SIMDLoadId: " + binaryen.SIMDLoadId);
  console.log("SIMDLoadStoreLaneId: " + binaryen.SIMDLoadStoreLaneId);
  console.log("MemoryInitId: " + binaryen.MemoryInitId);
  console.log("DataDropId: " + binaryen.DataDropId);
  console.log("MemoryCopyId: " + binaryen.MemoryCopyId);
  console.log("MemoryFillId: " + binaryen.MemoryFillId);
  console.log("PopId: " + binaryen.PopId);
  console.log("RefNullId: " + binaryen.RefNullId);
  console.log("RefIsNullId: " + binaryen.RefIsNullId);
  console.log("RefFuncId: " + binaryen.RefFuncId);
  console.log("RefEqId: " + binaryen.RefEqId);
  console.log("TableGetId: " + binaryen.TableGetId);
  console.log("TableSetId: " + binaryen.TableSetId);
  console.log("TableSizeId: " + binaryen.TableSizeId);
  console.log("TableGrowId: " + binaryen.TableGrowId);
  console.log("TryId: " + binaryen.TryId);
  console.log("ThrowId: " + binaryen.ThrowId);
  console.log("RethrowId: " + binaryen.RethrowId);
  console.log("TupleMakeId: " + binaryen.TupleMakeId);
  console.log("TupleExtractId: " + binaryen.TupleExtractId);
  console.log("RefI31Id: " + binaryen.RefI31Id);
  console.log("I31GetId: " + binaryen.I31GetId);
  console.log("CallRefId: " + binaryen.CallRefId);
  console.log("RefTestId: " + binaryen.RefTestId);
  console.log("RefCastId: " + binaryen.RefCastId);
  console.log("BrOnId: " + binaryen.BrOnId);
  console.log("StructNewId: " + binaryen.StructNewId);
  console.log("StructGetId: " + binaryen.StructGetId);
  console.log("StructSetId: " + binaryen.StructSetId);
  console.log("ArrayNewId: " + binaryen.ArrayNewId);
  console.log("ArrayNewFixedId: " + binaryen.ArrayNewFixedId);
  console.log("ArrayGetId: " + binaryen.ArrayGetId);
  console.log("ArraySetId: " + binaryen.ArraySetId);
  console.log("ArrayLenId: " + binaryen.ArrayLenId);
  console.log("ArrayCopy: " + binaryen.ArrayCopyId);
  console.log("RefAs: " + binaryen.RefAsId);
  console.log("StringNew: " + binaryen.StringNewId);
  console.log("StringConst: " + binaryen.StringConstId);
  console.log("StringMeasure: " + binaryen.StringMeasureId);
  console.log("StringEncode: " + binaryen.StringEncodeId);
  console.log("StringConcat: " + binaryen.StringConcatId);
  console.log("StringEq: " + binaryen.StringEqId);
  console.log("StringWTF16Get: " + binaryen.StringWTF16GetId);
  console.log("StringSliceWTF: " + binaryen.StringSliceWTFId);
}

function test_core() {

  // Module creation

  module = new binaryen.Module();
  // Memory
  module.setMemory(1, 256, "mem", [
    {
      name: "x0",
      passive: false,
      offset: module.i32.const(10),
      data: "hello, world".split('').map(function(x) { return x.charCodeAt(0) })
    },
    {
      name: "y1",
      passive: true,
      offset: null,
      data: "I am passive".split('').map(function(x) { return x.charCodeAt(0) })
    }
  ], true);

  // Create a tag
  var tag = module.addTag("a-tag", binaryen.i32, binaryen.none);

  // Literals and consts

  var constI32 = module.i32.const(1),
      constI64 = module.i64.const(2),
      constF32 = module.f32.const(3.14),
      constF64 = module.f64.const(2.1828),
      constF32Bits = module.f32.const_bits(0xffff1234),
      constF64Bits = module.f64.const_bits(0xffff12345678ABCDn)

  var iIfF = binaryen.createType([binaryen.i32, binaryen.i64, binaryen.f32, binaryen.f64])

  var temp1 = makeInt32(1), temp2 = makeInt32(2), temp3 = makeInt32(3),
      temp4 = makeInt32(4), temp5 = makeInt32(5),
      temp6 = makeInt32(0), temp7 = makeInt32(1),
      temp8 = makeInt32(0), temp9 = makeInt32(1),
      temp10 = makeInt32(1), temp11 = makeInt32(3), temp12 = makeInt32(5),
      temp13 = makeInt32(10), temp14 = makeInt32(11),
      temp15 = makeInt32(110), temp16 = makeInt64(111);

  var valueList = [
    // Unary
    module.i32.clz(module.i32.const(-10)),
    module.i64.ctz(module.i64.const(295147905179352825834n)),
    module.i32.popcnt(module.i32.const(-10)),
    module.f32.neg(module.f32.const(-33.612)),
    module.f64.abs(module.f64.const(-9005.841)),
    module.f32.ceil(module.f32.const(-33.612)),
    module.f64.floor(module.f64.const(-9005.841)),
    module.f32.trunc(module.f32.const(-33.612)),
    module.f32.nearest(module.f32.const(-33.612)),
    module.f64.sqrt(module.f64.const(-9005.841)),
    module.i32.eqz(module.i32.const(-10)),
    module.i64.extend_s(module.i32.const(-10)),
    module.i64.extend_u(module.i32.const(-10)),
    module.i32.wrap(module.i64.const(295147905179352825834n)),
    module.i32.trunc_s.f32(module.f32.const(-33.612)),
    module.i64.trunc_s.f32(module.f32.const(-33.612)),
    module.i32.trunc_u.f32(module.f32.const(-33.612)),
    module.i64.trunc_u.f32(module.f32.const(-33.612)),
    module.i32.trunc_s.f64(module.f64.const(-9005.841)),
    module.i64.trunc_s.f64(module.f64.const(-9005.841)),
    module.i32.trunc_u.f64(module.f64.const(-9005.841)),
    module.i64.trunc_u.f64(module.f64.const(-9005.841)),
    module.i32.trunc_s_sat.f32(module.f32.const(-33.612)),
    module.i64.trunc_s_sat.f32(module.f32.const(-33.612)),
    module.i32.trunc_u_sat.f32(module.f32.const(-33.612)),
    module.i64.trunc_u_sat.f32(module.f32.const(-33.612)),
    module.i32.trunc_s_sat.f64(module.f64.const(-9005.841)),
    module.i64.trunc_s_sat.f64(module.f64.const(-9005.841)),
    module.i32.trunc_u_sat.f64(module.f64.const(-9005.841)),
    module.i64.trunc_u_sat.f64(module.f64.const(-9005.841)),
    module.i32.reinterpret(module.f32.const(-33.612)),
    module.i64.reinterpret(module.f64.const(-9005.841)),
    module.f32.convert_s.i32(module.i32.const(-10)),
    module.f64.convert_s.i32(module.i32.const(-10)),
    module.f32.convert_u.i32(module.i32.const(-10)),
    module.f64.convert_u.i32(module.i32.const(-10)),
    module.f32.convert_s.i64(module.i64.const(295147905179352825834n)),
    module.f64.convert_s.i64(module.i64.const(295147905179352825834n)),
    module.f32.convert_u.i64(module.i64.const(295147905179352825834n)),
    module.f64.convert_u.i64(module.i64.const(295147905179352825834n)),
    module.f64.promote(module.f32.const(-33.612)),
    module.f32.demote(module.f64.const(-9005.841)),
    module.f32.reinterpret(module.i32.const(-10)),
    module.f64.reinterpret(module.i64.const(295147905179352825834n)),
    module.i8x16.splat(module.i32.const(42)),
    module.i16x8.splat(module.i32.const(42)),
    module.i32x4.splat(module.i32.const(42)),
    module.i64x2.splat(module.i64.const(1958505087099n)),
    module.f32x4.splat(module.f32.const(42.0)),
    module.f64x2.splat(module.f64.const(42.0)),
    module.v128.not(module.v128.const(v128_bytes)),
    module.v128.any_true(module.v128.const(v128_bytes)),
    module.i8x16.popcnt(module.v128.const(v128_bytes)),
    module.i8x16.abs(module.v128.const(v128_bytes)),
    module.i8x16.neg(module.v128.const(v128_bytes)),
    module.i8x16.all_true(module.v128.const(v128_bytes)),
    module.i8x16.bitmask(module.v128.const(v128_bytes)),
    module.i16x8.abs(module.v128.const(v128_bytes)),
    module.i16x8.neg(module.v128.const(v128_bytes)),
    module.i16x8.all_true(module.v128.const(v128_bytes)),
    module.i16x8.bitmask(module.v128.const(v128_bytes)),
    module.i16x8.extadd_pairwise_i8x16_s(module.v128.const(v128_bytes)),
    module.i16x8.extadd_pairwise_i8x16_u(module.v128.const(v128_bytes)),
    module.i32x4.abs(module.v128.const(v128_bytes)),
    module.i32x4.neg(module.v128.const(v128_bytes)),
    module.i32x4.all_true(module.v128.const(v128_bytes)),
    module.i32x4.bitmask(module.v128.const(v128_bytes)),
    module.i32x4.extadd_pairwise_i16x8_s(module.v128.const(v128_bytes)),
    module.i32x4.extadd_pairwise_i16x8_u(module.v128.const(v128_bytes)),
    module.i64x2.abs(module.v128.const(v128_bytes)),
    module.i64x2.neg(module.v128.const(v128_bytes)),
    module.i64x2.all_true(module.v128.const(v128_bytes)),
    module.i64x2.bitmask(module.v128.const(v128_bytes)),
    module.f32x4.abs(module.v128.const(v128_bytes)),
    module.f32x4.neg(module.v128.const(v128_bytes)),
    module.f32x4.sqrt(module.v128.const(v128_bytes)),
    module.f64x2.abs(module.v128.const(v128_bytes)),
    module.f64x2.neg(module.v128.const(v128_bytes)),
    module.f64x2.sqrt(module.v128.const(v128_bytes)),
    module.f64x2.convert_low_i32x4_s(module.v128.const(v128_bytes)),
    module.f64x2.convert_low_i32x4_u(module.v128.const(v128_bytes)),
    module.f64x2.promote_low_f32x4(module.v128.const(v128_bytes)),
    module.i32x4.trunc_sat_f32x4_s(module.v128.const(v128_bytes)),
    module.i32x4.trunc_sat_f32x4_u(module.v128.const(v128_bytes)),
    module.f32x4.convert_i32x4_s(module.v128.const(v128_bytes)),
    module.f32x4.convert_i32x4_u(module.v128.const(v128_bytes)),
    module.f32x4.demote_f64x2_zero(module.v128.const(v128_bytes)),
    module.i16x8.extend_low_i8x16_s(module.v128.const(v128_bytes)),
    module.i16x8.extend_high_i8x16_s(module.v128.const(v128_bytes)),
    module.i16x8.extend_low_i8x16_u(module.v128.const(v128_bytes)),
    module.i16x8.extend_high_i8x16_u(module.v128.const(v128_bytes)),
    module.i32x4.extend_low_i16x8_s(module.v128.const(v128_bytes)),
    module.i32x4.extend_high_i16x8_s(module.v128.const(v128_bytes)),
    module.i32x4.extend_low_i16x8_u(module.v128.const(v128_bytes)),
    module.i32x4.extend_high_i16x8_u(module.v128.const(v128_bytes)),
    module.i32x4.trunc_sat_f64x2_s_zero(module.v128.const(v128_bytes)),
    module.i32x4.trunc_sat_f64x2_u_zero(module.v128.const(v128_bytes)),
    module.i64x2.extend_low_i32x4_s(module.v128.const(v128_bytes)),
    module.i64x2.extend_high_i32x4_s(module.v128.const(v128_bytes)),
    module.i64x2.extend_low_i32x4_u(module.v128.const(v128_bytes)),
    module.i64x2.extend_high_i32x4_u(module.v128.const(v128_bytes)),
    // Binary
    module.i32.add(module.i32.const(-10), module.i32.const(-11)),
    module.f64.sub(module.f64.const(-9005.841), module.f64.const(-9007.333)),
    module.i32.div_s(module.i32.const(-10), module.i32.const(-11)),
    module.i64.div_u(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i64.rem_s(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i32.rem_u(module.i32.const(-10), module.i32.const(-11)),
    module.i32.and(module.i32.const(-10), module.i32.const(-11)),
    module.i64.or(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i32.xor(module.i32.const(-10), module.i32.const(-11)),
    module.i64.shl(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i64.shr_u(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i32.shr_s(module.i32.const(-10), module.i32.const(-11)),
    module.i32.rotl(module.i32.const(-10), module.i32.const(-11)),
    module.i64.rotr(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.f32.div(module.f32.const(-33.612), module.f32.const(-62.5)),
    module.f64.copysign(module.f64.const(-9005.841), module.f64.const(-9007.333)),
    module.f32.min(module.f32.const(-33.612), module.f32.const(-62.5)),
    module.f64.max(module.f64.const(-9005.841), module.f64.const(-9007.333)),
    module.i32.eq(module.i32.const(-10), module.i32.const(-11)),
    module.f32.ne(module.f32.const(-33.612), module.f32.const(-62.5)),
    module.i32.lt_s(module.i32.const(-10), module.i32.const(-11)),
    module.i64.lt_u(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i64.le_s(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i32.le_u(module.i32.const(-10), module.i32.const(-11)),
    module.i64.gt_s(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.i32.gt_u(module.i32.const(-10), module.i32.const(-11)),
    module.i32.ge_s(module.i32.const(-10), module.i32.const(-11)),
    module.i64.ge_u(module.i64.const(4294967274n), module.i64.const(4294967273n)),
    module.f32.lt(module.f32.const(-33.612), module.f32.const(-62.5)),
    module.f64.le(module.f64.const(-9005.841), module.f64.const(-9007.333)),
    module.f64.gt(module.f64.const(-9005.841), module.f64.const(-9007.333)),
    module.f32.ge(module.f32.const(-33.612), module.f32.const(-62.5)),
    module.i8x16.eq(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.ne(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.lt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.lt_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.gt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.gt_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.le_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.le_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.ge_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.ge_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.eq(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.ne(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.lt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.lt_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.gt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.gt_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.le_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.le_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.ge_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.ge_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.eq(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.ne(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.lt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.lt_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.gt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.gt_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.le_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.le_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.ge_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.ge_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.eq(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.ne(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.lt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.gt_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.le_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.ge_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.eq(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.ne(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.lt(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.gt(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.le(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.ge(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.eq(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.ne(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.lt(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.gt(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.le(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.ge(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.v128.and(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.v128.or(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.v128.xor(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.v128.andnot(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.add(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.add_saturate_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.add_saturate_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.sub(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.sub_saturate_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.sub_saturate_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.min_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.min_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.max_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.max_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.avgr_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.add(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.add_saturate_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.add_saturate_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.sub(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.sub_saturate_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.sub_saturate_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.mul(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.min_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.min_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.max_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.max_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.avgr_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.q15mulr_sat_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.extmul_low_i8x16_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.extmul_high_i8x16_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.extmul_low_i8x16_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.extmul_high_i8x16_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.add(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.sub(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.mul(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.min_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.min_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.max_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.max_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.dot_i16x8_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.extmul_low_i16x8_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.extmul_high_i16x8_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.extmul_low_i16x8_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i32x4.extmul_high_i16x8_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.add(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.sub(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.mul(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.extmul_low_i32x4_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.extmul_high_i32x4_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.extmul_low_i32x4_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i64x2.extmul_high_i32x4_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.add(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.sub(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.mul(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.div(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.min(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.max(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.pmin(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.pmax(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.ceil(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.floor(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.trunc(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f32x4.nearest(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.add(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.sub(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.mul(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.div(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.min(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.max(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.pmin(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.pmax(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.ceil(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.floor(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.trunc(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.f64x2.nearest(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.narrow_i16x8_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.narrow_i16x8_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.narrow_i32x4_s(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i16x8.narrow_i32x4_u(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    module.i8x16.swizzle(module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    // SIMD lane manipulation
    module.i8x16.extract_lane_s(module.v128.const(v128_bytes), 1),
    module.i8x16.extract_lane_u(module.v128.const(v128_bytes), 1),
    module.i16x8.extract_lane_s(module.v128.const(v128_bytes), 1),
    module.i16x8.extract_lane_u(module.v128.const(v128_bytes), 1),
    module.i32x4.extract_lane(module.v128.const(v128_bytes), 1),
    module.i64x2.extract_lane(module.v128.const(v128_bytes), 1),
    module.f32x4.extract_lane(module.v128.const(v128_bytes), 1),
    module.f64x2.extract_lane(module.v128.const(v128_bytes), 1),
    module.i16x8.replace_lane(module.v128.const(v128_bytes), 1, module.i32.const(42)),
    module.i8x16.replace_lane(module.v128.const(v128_bytes), 1, module.i32.const(42)),
    module.i32x4.replace_lane(module.v128.const(v128_bytes), 1, module.i32.const(42)),
    module.i64x2.replace_lane(module.v128.const(v128_bytes), 1, module.i64.const(184683593770n)),
    module.f32x4.replace_lane(module.v128.const(v128_bytes), 1, module.f32.const(42)),
    module.f64x2.replace_lane(module.v128.const(v128_bytes), 1, module.f64.const(42)),
    // SIMD shift
    module.i8x16.shl(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i8x16.shr_s(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i8x16.shr_u(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i16x8.shl(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i16x8.shr_s(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i16x8.shr_u(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i32x4.shl(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i32x4.shr_s(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i32x4.shr_u(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i64x2.shl(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i64x2.shr_s(module.v128.const(v128_bytes), module.i32.const(1)),
    module.i64x2.shr_u(module.v128.const(v128_bytes), module.i32.const(1)),
    // SIMD load
    module.v128.load8_splat(0, 1, module.i32.const(128)),
    module.v128.load16_splat(16, 1, module.i32.const(128)),
    module.v128.load32_splat(16, 4, module.i32.const(128)),
    module.v128.load64_splat(0, 4, module.i32.const(128)),
    module.v128.load8x8_s(0, 8, module.i32.const(128)),
    module.v128.load8x8_u(0, 8, module.i32.const(128)),
    module.v128.load16x4_s(0, 8, module.i32.const(128)),
    module.v128.load16x4_u(0, 8, module.i32.const(128)),
    module.v128.load32x2_s(0, 8, module.i32.const(128)),
    module.v128.load32x2_u(0, 8, module.i32.const(128)),
    module.v128.load32_zero(0, 4, module.i32.const(128)),
    module.v128.load64_zero(0, 8, module.i32.const(128)),
    // SIMD load/store lane
    module.v128.load8_lane(0, 1, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load8_lane(1, 1, 15, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load16_lane(0, 2, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load16_lane(2, 1, 7, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load32_lane(0, 4, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load32_lane(4, 2, 3, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load64_lane(0, 8, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.load64_lane(8, 4, 1, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store8_lane(0, 1, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store8_lane(1, 1, 15, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store16_lane(0, 2, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store16_lane(2, 1, 7, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store32_lane(0, 4, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store32_lane(4, 2, 3, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store64_lane(0, 8, 0, module.i32.const(128), module.v128.const(v128_bytes)),
    module.v128.store64_lane(8, 4, 1, module.i32.const(128), module.v128.const(v128_bytes)),
    // Other SIMD
    module.i8x16.shuffle(module.v128.const(v128_bytes), module.v128.const(v128_bytes), v128_bytes),
    module.v128.bitselect(module.v128.const(v128_bytes), module.v128.const(v128_bytes), module.v128.const(v128_bytes)),
    // Bulk memory
    module.memory.init("x0", makeInt32(1024), makeInt32(0), makeInt32(12)),
    module.data.drop("x0"),
    module.memory.copy(makeInt32(2048), makeInt32(1024), makeInt32(12)),
    module.memory.fill(makeInt32(0), makeInt32(42), makeInt32(1024)),
    // All the rest
    module.block('', []), // block with no name
    module.if(temp1, temp2, temp3),
    module.if(temp4, module.drop(temp5)),
    module.loop("in", makeInt32(0)),
    module.loop(null, makeInt32(0)),
    module.break("the-value", temp6, temp7),
    module.break("the-nothing", makeInt32(2)),
    module.break("the-value", null, makeInt32(3)),
    module.break("the-nothing"),
    module.switch([ "the-value" ], "the-value", temp8, temp9),
    module.switch([ "the-nothing" ], "the-nothing", makeInt32(2)),
    module.i32.eqz( // check the output type of the call node
      module.call("kitchen()sinker", [ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ], binaryen.i32)
    ),
    module.i32.eqz( // check the output type of the call node
      module.i32.trunc_s.f32(
        module.call("an-imported", [ makeInt32(13), makeFloat64(3.7) ], binaryen.f32)
      )
    ),
    module.i32.eqz( // check the output type of the call node
      module.call_indirect("t0", makeInt32(2449), [ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ], iIfF, binaryen.i32)
    ),
    module.drop(module.local.get(0, binaryen.i32)),
    module.local.set(0, makeInt32(101)),
    module.drop(module.local.tee(0, makeInt32(102), binaryen.i32)),
    module.i32.load(0, 0, makeInt32(1)),
    module.i64.load16_s(2, 1, makeInt32(8)),
    module.f32.load(0, 0, makeInt32(2)),
    module.f64.load(2, 8, makeInt32(9)),
    module.i32.store(0, 0, temp13, temp14),
    module.i64.store(2, 4, temp15, temp16),
    module.select(temp10, temp11, temp12),
    module.return(makeInt32(1337)),
    // Tail Call
    module.return_call("kitchen()sinker", [ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ], binaryen.i32),
    module.return_call_indirect("t0", makeInt32(2449), [ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ], iIfF, binaryen.i32),

    // Reference types
    module.ref.is_null(module.ref.null(binaryen.externref)),
    module.ref.is_null(module.ref.null(binaryen.funcref)),
    module.ref.is_null(module.ref.func("kitchen()sinker", binaryen.funcref)),
    module.select(temp10, module.ref.null(binaryen.funcref), module.ref.func("kitchen()sinker", binaryen.funcref)),

    // GC
    module.ref.eq(module.ref.null(binaryen.eqref), module.ref.null(binaryen.eqref)),

    // Exception handling
    module.try(
      '',
      module.throw("a-tag", [module.i32.const(0)]),
      ["a-tag"],
      [module.drop(module.i32.pop())],
      ''
    ),

    // Atomics
    module.i32.atomic.store(0,
      module.i32.const(0),
      module.i32.atomic.load(0,
        module.i32.const(0)
      )
    ),
    module.drop(
      module.memory.atomic.wait32(
        module.i32.const(0),
        module.i32.const(0),
        module.i64.const(0)
      )
    ),
    module.drop(
      module.memory.atomic.notify(
        module.i32.const(0),
        module.i32.const(0)
      )
    ),
    module.atomic.fence(),

    // Tuples
    module.tuple.make(
      [ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ]
    ),
    module.tuple.extract(
      module.tuple.make(
        [ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ]
      ), 2
    ),

    // Pop
    module.i32.pop(),
    module.i64.pop(),
    module.f32.pop(),
    module.f64.pop(),
    module.v128.pop(),
    module.funcref.pop(),
    module.externref.pop(),
    module.anyref.pop(),
    module.eqref.pop(),
    module.i31ref.pop(),
    module.structref.pop(),
    module.stringref.pop(),

    // Memory
    module.memory.size(),
    module.memory.grow(makeInt32(0)),

    // GC
    module.ref.i31(
      module.i32.const(0)
    ),
    module.i31.get_s(
      module.ref.i31(
        module.i32.const(1)
      )
    ),
    module.i31.get_u(
      module.ref.i31(
        module.i32.const(2)
      )
    ),

    // Other
    module.nop(),
    module.unreachable(),
  ];

  // Test expression utility
  console.log("getExpressionInfo=" + JSON.stringify(cleanInfo(binaryen.getExpressionInfo(valueList[3]))));
  console.log(binaryen.emitText(valueList[3])); // test printing a standalone expression

  // Add drops of concrete expressions, except the last.
  for (var i = 0; i < valueList.length - 1; i++) {
    var type = binaryen.Expression.getType(valueList[i]);
    if (type != binaryen.none && type != binaryen.unreachable) {
      valueList[i] = module.drop(valueList[i]);
    }
  }

  console.log("getExpressionInfo(i32.const)=" + JSON.stringify(binaryen.getExpressionInfo(module.i32.const(5))));
  console.log("getExpressionInfo(i64.const)=" + JSON.stringify(binaryen.getExpressionInfo(module.i64.const(30064771078n))));
  console.log("getExpressionInfo(f32.const)=" + JSON.stringify(binaryen.getExpressionInfo(module.f32.const(8.5))));
  console.log("getExpressionInfo(f64.const)=" + JSON.stringify(binaryen.getExpressionInfo(module.f64.const(9.5))));
  var elements = binaryen.getExpressionInfo(
    module.tuple.make([ makeInt32(13), makeInt64(37), makeFloat32(1.3), makeFloat64(3.7) ])
  ).operands;
  for (var i = 0; i < elements.length; i++) {
    console.log("getExpressionInfo(tuple[" + i + "])=" + JSON.stringify(binaryen.getExpressionInfo(elements[i])));
  }

  // Make the main body of the function. and one block with a return value, one without
  var value = module.block("the-value", valueList, binaryen.i32);
  var droppedValue = module.drop(value);
  var nothing = module.block("the-nothing", [ droppedValue ]);
  var body = module.block("the-body", [ nothing, makeInt32(42) ], binaryen.i32);

  // Create the function
  var sinker = module.addFunction("kitchen()sinker", iIfF, binaryen.i32, [ binaryen.i32 ], body);

  // Create a global
  var initExpr = module.i32.const(1);
  var global = module.addGlobal("a-global", binaryen.i32, false, initExpr)

  // Imports

  var iF = binaryen.createType([binaryen.i32, binaryen.f64]);
  module.addFunctionImport("an-imported", "module", "base", iF, binaryen.f32);
  module.addGlobalImport("a-global-imp", "module", "base", binaryen.i32, false);
  module.addGlobalImport("a-mut-global-imp", "module", "base", binaryen.i32, true);
  module.addTagImport("a-tag-imp", "module", "base", binaryen.i32, binaryen.none);

  // Exports

  module.addFunctionExport("kitchen()sinker", "kitchen_sinker");
  module.addGlobalExport("a-global", "a-global-exp");
  module.addTagExport("a-tag", "a-tag-exp");

  // Tables
  module.addTable("t1", 0, 2);
  var tablePtr = module.getTable("t1");
  assert(tablePtr !== 0);
  assert(tablePtr === module.getTableByIndex(0));

  var table = binaryen.getTableInfo(tablePtr);
  assert(table.name === "t1");
  assert(table.module === "");
  assert(table.base === "");
  assert(table.initial === 0);
  assert(table.max === 2);

  module.removeTable("t1");
  assert(module.getNumTables() === 0);

  module.addTable("t0", 1, 0xffffffff);
  module.addActiveElementSegment("t0", "e0", [ binaryen.getFunctionInfo(sinker).name ]);
  assert(module.getNumTables() === 1);
  assert(module.getNumElementSegments() === 1);

  // Start function. One per module
  var starter = module.addFunction("starter", binaryen.none, binaryen.none, [], module.nop());
  module.setStart(starter);

  var features = binaryen.Features.All;
  module.setFeatures(features);
  assert(module.getFeatures() == features);

  // Verify it validates
  assert(module.validate());

  // Print it out
  console.log(module.emitText());

  // Clean up the module, which owns all the objects we created above
  module.dispose();
}

function makeCallCheck(x) {
  return module.call("check", [ makeInt32(x) ], binaryen.None);
}

function test_relooper() {
  module = new binaryen.Module();
  var localTypes = [ binaryen.i32 ];

  module.addFunctionImport("check", "module", "check", binaryen.i32, binaryen.none);

  { // trivial: just one block
    var relooper = new binaryen.Relooper(module);
    var block = relooper.addBlock(makeCallCheck(1337));
    var body = relooper.renderAndDispose(block, 0, module);
    module.addFunction("just-one-block", binaryen.none, binaryen.none, localTypes, body);
  }
  { // two blocks
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    relooper.addBranch(block0, block1); // no condition, no code on branch
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("two-blocks", binaryen.none, binaryen.none, localTypes, body);
  }
  { // two blocks with code between them
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    relooper.addBranch(block0, block1, null, makeDroppedInt32(77)); // code on branch
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("two-blocks-plus-code", binaryen.none, binaryen.none, localTypes, body);
  }
  { // two blocks in a loop
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    relooper.addBranch(block0, block1, null, null);
    relooper.addBranch(block1, block0, null, null);
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("loop", binaryen.none, binaryen.none, localTypes, body);
  }
  { // two blocks in a loop with codes
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    relooper.addBranch(block0, block1, null, makeDroppedInt32(33));
    relooper.addBranch(block1, block0, null, makeDroppedInt32(-66));
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("loop-plus-code", binaryen.none, binaryen.none, localTypes, body);
  }
  { // split
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    relooper.addBranch(block0, block1, makeInt32(55), null);
    relooper.addBranch(block0, block2, null, null);
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("split", binaryen.none, binaryen.none, localTypes, body);
  }
  { // split + code
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    var temp = makeDroppedInt32(10);
    relooper.addBranch(block0, block1, makeInt32(55), temp);
    relooper.addBranch(block0, block2, null, makeDroppedInt32(20));
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("split-plus-code", binaryen.none, binaryen.none, localTypes, body);
  }
  { // if
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    relooper.addBranch(block0, block1, makeInt32(55), null);
    relooper.addBranch(block0, block2, null, null);
    relooper.addBranch(block1, block2, null, null);
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("if", binaryen.none, binaryen.none, localTypes, body);
  }
  { // if + code
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    temp = makeDroppedInt32(-1);
    relooper.addBranch(block0, block1, makeInt32(55), temp);
    relooper.addBranch(block0, block2, null, makeDroppedInt32(-2));
    relooper.addBranch(block1, block2, null, makeDroppedInt32(-3));
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("if-plus-code", binaryen.none, binaryen.none, localTypes, body);
  }
  { // if-else
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    var block3 = relooper.addBlock(makeCallCheck(3));
    relooper.addBranch(block0, block1, makeInt32(55), null);
    relooper.addBranch(block0, block2, null, null);
    relooper.addBranch(block1, block3, null, null);
    relooper.addBranch(block2, block3, null, null);
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("if-else", binaryen.none, binaryen.none, localTypes, body);
  }
  { // loop+tail
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    relooper.addBranch(block0, block1, null, null);
    relooper.addBranch(block1, block0, makeInt32(10), null);
    relooper.addBranch(block1, block2, null, null);
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("loop-tail", binaryen.none, binaryen.none, localTypes, body);
  }
  { // nontrivial loop + phi to head
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    var block3 = relooper.addBlock(makeCallCheck(3));
    var block4 = relooper.addBlock(makeCallCheck(4));
    var block5 = relooper.addBlock(makeCallCheck(5));
    var block6 = relooper.addBlock(makeCallCheck(6));
    relooper.addBranch(block0, block1, null, makeDroppedInt32(10));
    relooper.addBranch(block1, block2, makeInt32(-2), null);
    relooper.addBranch(block1, block6, null, makeDroppedInt32(20));
    relooper.addBranch(block2, block3, makeInt32(-6), null);
    relooper.addBranch(block2, block1, null, makeDroppedInt32(30));
    relooper.addBranch(block3, block4, makeInt32(-10), null);
    relooper.addBranch(block3, block5, null, null);
    relooper.addBranch(block4, block5, null, null);
    relooper.addBranch(block5, block6, null, makeDroppedInt32(40));
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("nontrivial-loop-plus-phi-to-head", binaryen.none, binaryen.none, localTypes, body);
  }
  { // switch
    var relooper = new binaryen.Relooper(module);
    temp = makeInt32(-99);
    var block0 = relooper.addBlockWithSwitch(makeCallCheck(0), temp);
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    var block3 = relooper.addBlock(makeCallCheck(3));
    relooper.addBranchForSwitch(block0, block1, [ 2, 5 ]);
    relooper.addBranchForSwitch(block0, block2, [4], makeDroppedInt32(55));
    relooper.addBranchForSwitch(block0, block3, [], null);
    var body = relooper.renderAndDispose(block0, 0, module);
    module.addFunction("switch", binaryen.none, binaryen.none, localTypes, body);
  }
  { // duff's device
    var relooper = new binaryen.Relooper(module);
    var block0 = relooper.addBlock(makeCallCheck(0));
    var block1 = relooper.addBlock(makeCallCheck(1));
    var block2 = relooper.addBlock(makeCallCheck(2));
    relooper.addBranch(block0, block1, makeInt32(10), null);
    relooper.addBranch(block0, block2, null, null);
    relooper.addBranch(block1, block2, null, null);
    relooper.addBranch(block2, block1, null, null);
    var body = relooper.renderAndDispose(block0, 3, module); // use $3 as the helper var
    module.addFunction("duffs-device", binaryen.none, binaryen.none, [ binaryen.i32, binaryen.i32, binaryen.i64, binaryen.i32, binaryen.f32, binaryen.f64, binaryen.i32 ], body);
  }

  { // return in a block
    var relooper = new binaryen.Relooper(module);
    var list = module.block("the-list", [ makeCallCheck(42), module.return(makeInt32(1337)) ]);
    var block = relooper.addBlock(list);
    var body = relooper.renderAndDispose(block, 0, module);
    module.addFunction("return", binaryen.none, binaryen.i32, localTypes, body);
  }

  console.log("raw:");
  console.log(module.emitText());

  assert(module.validate());

  module.runPasses(["precompute"]);

  assert(module.validate());

  module.optimize();

  assert(module.validate());

  console.log("optimized:");
  console.log(module.emitText());

  module.dispose();
}

function test_binaries() {
  var buffer, size;

  { // create a module and write it to binary
    module = new binaryen.Module();
    module.setFeatures(binaryen.Features.All);
    var ii = binaryen.createType([binaryen.i32, binaryen.i32]);
    var x = module.local.get(0, binaryen.i32),
        y = module.local.get(1, binaryen.i32);
    var add = module.i32.add(x, y);
    var adder = module.addFunction("adder", ii, binaryen.i32, [], add);
    var initExpr = module.i32.const(3);
    var global = module.addGlobal("a-global", binaryen.i32, false, initExpr)
    var tag = module.addTag("a-tag", binaryen.createType([binaryen.i32, binaryen.i32]), binaryen.none);
    binaryen.setDebugInfo(true); // include names section
    buffer = module.emitBinary();
    binaryen.setDebugInfo(false);
    size = buffer.length; // write out the module
    module.dispose();
  }

  assert(size > 0);
  assert(size < 512); // this is a tiny module

  // read the module from the binary
  module = binaryen.readBinary(buffer);
  module.setFeatures(binaryen.Features.All);

  // validate, print, and free
  assert(module.validate());
  console.log("module loaded from binary form:");
  console.log(module.emitText());
  module.dispose();
}

function test_interpret() {
  // create a simple module with a start method that prints a number, and interpret it, printing that number.
  module = new binaryen.Module();

  module.addFunctionImport("print-i32", "spectest", "print", binaryen.i32, binaryen.none);
  var call = module.call("print-i32", [ makeInt32(1234) ], binaryen.None);
  var starter = module.addFunction("starter", binaryen.none, binaryen.none, [], call);
  module.setStart(starter);

  console.log(module.emitText());
  assert(module.validate());
  module.interpret();
  module.dispose();
}

function test_nonvalid() {
  // create a module that fails to validate
  module = new binaryen.Module();

  var func = module.addFunction("func", binaryen.none, binaryen.none, [ binaryen.i32 ],
    module.local.set(0, makeInt64(1234)) // wrong type!
  );

  console.log(module.emitText());
  console.log("validation: " + module.validate());

  module.dispose();
}

function test_parsing() {
  var text;

  // create a module and write it to text
  module = new binaryen.Module();
  module.setFeatures(binaryen.Features.All);

  var ii = binaryen.createType([binaryen.i32, binaryen.i32]);
  var x = module.local.get(0, binaryen.i32),
      y = module.local.get(1, binaryen.i32);
  var add = module.i32.add(x, y);
  var adder = module.addFunction("adder", ii, binaryen.i32, [], add);
  var initExpr = module.i32.const(3);
  var global = module.addGlobal("a-global", binaryen.i32, false, initExpr)
  var tag = module.addTag("a-tag", binaryen.i32, binaryen.none);
  text = module.emitText();
  module.dispose();
  module = null;
  console.log('test_parsing text:\n' + text);

  text = text.replace('adder', 'ADD_ER');

  var module2 = binaryen.parseText(text);
  module2.setFeatures(binaryen.Features.All);
  assert(module2.validate());
  console.log("module loaded from text form:");
  console.log(module2.emitText());
  module2.dispose();
}

function test_internals() {
  console.log('sizeof Literal: ' + binaryen['_BinaryenSizeofLiteral']());
}

function test_for_each() {
  module = new binaryen.Module();

  var funcNames = [ "fn0", "fn1", "fn2" ];

  var fns = [
    module.addFunction(funcNames[0], binaryen.none, binaryen.none, [], module.nop()),
    module.addFunction(funcNames[1], binaryen.none, binaryen.none, [], module.nop()),
    module.addFunction(funcNames[2], binaryen.none, binaryen.none, [], module.nop())
  ];

  var i;
  for (i = 0; i < module.getNumFunctions(); i++) {
    assert(module.getFunctionByIndex(i) === fns[i]);
  }

  var exps = [
    module.addFunctionExport(funcNames[0], "export0"),
    module.addFunctionExport(funcNames[1], "export1"),
    module.addFunctionExport(funcNames[2], "export2")
  ];

  for (i = 0; i < module.getNumExports(); i++) {
    assert(module.getExportByIndex(i) === exps[i]);
  }

  var expected_offsets = [10, 125, null];
  var expected_names = ["x0", "y1", "z2"];
  var expected_data = ["hello, world", "segment data 2", "hello, passive"];
  var expected_passive = [false, false, true];

  var glos = [
    module.addGlobal("a-global", binaryen.i32, false, module.i32.const(expected_offsets[1])),
    module.addGlobal("a-global2", binaryen.i32, false, module.i32.const(2)),
    module.addGlobal("a-global3", binaryen.i32, false, module.i32.const(3))
  ];

  for (i = 0; i < module.getNumGlobals(); i++) {
    assert(module.getGlobalByIndex(i) === glos[i]);
  }

  module.setMemory(1, 256, "mem", [
    {
      name: expected_names[0],
      passive: expected_passive[0],
      offset: module.i32.const(expected_offsets[0]),
      data: expected_data[0].split('').map(function(x) { return x.charCodeAt(0) })
    },
    {
      name: expected_names[1],
      passive: expected_passive[1],
      offset: module.global.get("a-global"),
      data: expected_data[1].split('').map(function(x) { return x.charCodeAt(0) })
    },
    {
      name: expected_names[2],
      passive: expected_passive[2],
      offset: expected_offsets[2],
      data: expected_data[2].split('').map(function(x) { return x.charCodeAt(0) })
    }
  ], false);
  for (i = 0; i < module.getNumMemorySegments(); i++) {
    var segment = module.getMemorySegmentInfo(expected_names[i]);
    assert(expected_offsets[i] === segment.offset);
    var data8 = new Uint8Array(segment.data);
    var str = String.fromCharCode.apply(null, data8);
    assert(expected_data[i] === str);
    assert(expected_passive[i] === segment.passive);
  }

  module.addTable("t0", 1, 0xffffffff);
  var ftable = module.getTable("t0");
  var constExprRef = module.i32.const(0);
  module.addActiveElementSegment("t0", "e0", funcNames, constExprRef);

  var tableInfo = binaryen.getTableInfo(ftable);
  assert("" === tableInfo.module);
  assert("" === tableInfo.base);

  var segments = module.getTableSegments(ftable);
  assert(1 === segments.length);

  var elemSegment = binaryen.getElementSegmentInfo(segments[0]);
  assert(constExprRef === elemSegment.offset);
  assert(3 === elemSegment.data.length);
  for (i = 0; i < elemSegment.data.length; i++) {
    assert(funcNames[i] === elemSegment.data[i]);
  }

  console.log(module.emitText());
  module.dispose();
}

function test_expression_info() {
  module = new binaryen.Module();
  module.setMemory(1, 1, null);

  // Issue #2392
  console.log("getExpressionInfo(memory.grow)=" + JSON.stringify(binaryen.getExpressionInfo(module.memory.grow(1))));

  // Issue #2396
  console.log("getExpressionInfo(switch)=" + JSON.stringify(binaryen.getExpressionInfo(module.switch([ "label" ], "label", 0))));

  module.dispose();
}

test_types();
test_features();
test_ids();
test_core();
test_relooper();
test_binaries();
test_interpret();
test_nonvalid();
test_parsing();
test_internals();
test_for_each();
test_expression_info();
