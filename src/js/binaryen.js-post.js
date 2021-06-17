// export friendly API methods
function preserveStack(func) {
  try {
    var stack = stackSave();
    return func();
  } finally {
    stackRestore(stack);
  }
}

function strToStack(str) {
  return str ? allocate(intArrayFromString(str), ALLOC_STACK) : 0;
}

function i32sToStack(i32s) {
  const ret = stackAlloc(i32s.length << 2);
  HEAP32.set(i32s, ret >>> 2);
  return ret;
}

function i8sToStack(i8s) {
  const ret = stackAlloc(i8s.length);
  HEAP8.set(i8s, ret);
  return ret;
}

function initializeConstants() {

  // Types
  [ ['none', 'None'],
    ['i32', 'Int32'],
    ['i64', 'Int64'],
    ['f32', 'Float32'],
    ['f64', 'Float64'],
    ['v128', 'Vec128'],
    ['funcref', 'Funcref'],
    ['externref', 'Externref'],
    ['anyref', 'Anyref'],
    ['eqref', 'Eqref'],
    ['i31ref', 'I31ref'],
    ['dataref', 'Dataref'],
    ['unreachable', 'Unreachable'],
    ['auto', 'Auto']
  ].forEach(entry => {
    Module[entry[0]] = Module['_BinaryenType' + entry[1]]();
  });

  // Expression ids
  Module['ExpressionIds'] = {};
  [ 'Invalid',
    'Block',
    'If',
    'Loop',
    'Break',
    'Switch',
    'Call',
    'CallIndirect',
    'LocalGet',
    'LocalSet',
    'GlobalGet',
    'GlobalSet',
    'Load',
    'Store',
    'Const',
    'Unary',
    'Binary',
    'Select',
    'Drop',
    'Return',
    'MemorySize',
    'MemoryGrow',
    'Nop',
    'Unreachable',
    'AtomicCmpxchg',
    'AtomicRMW',
    'AtomicWait',
    'AtomicNotify',
    'AtomicFence',
    'SIMDExtract',
    'SIMDReplace',
    'SIMDShuffle',
    'SIMDTernary',
    'SIMDShift',
    'SIMDLoad',
    'SIMDLoadStoreLane',
    'MemoryInit',
    'DataDrop',
    'MemoryCopy',
    'MemoryFill',
    'RefNull',
    'RefIs',
    'RefFunc',
    'RefEq',
    'Try',
    'Throw',
    'Rethrow',
    'TupleMake',
    'TupleExtract',
    'Pop',
    'I31New',
    'I31Get',
    'CallRef',
    'RefTest',
    'RefCast',
    'BrOn',
    'RttCanon',
    'RttSub',
    'StructNew',
    'StructGet',
    'StructSet',
    'ArrayNew',
    'ArrayGet',
    'ArraySet',
    'ArrayLen',
    // FIXME: is 'RefAs' missing?
    'TableGet',
    'TableSet',
    'TableSize',
    'TableGrow',
    'TableFill',
    'TableCopy',
    'TableInit',
    'ElemDrop',
  ].forEach(name => {
    Module['ExpressionIds'][name] = Module[name + 'Id'] = Module['_Binaryen' + name + 'Id']();
  });

  // External kinds
  Module['ExternalKinds'] = {};
  [ 'Function',
    'Table',
    'Memory',
    'Global',
    'Event'
  ].forEach(name => {
    Module['ExternalKinds'][name] = Module['External' + name] = Module['_BinaryenExternal' + name]();
  });

  // Features
  Module['Features'] = {};
  [ 'MVP',
    'Atomics',
    'BulkMemory',
    'MutableGlobals',
    'NontrappingFPToInt',
    'SignExt',
    'SIMD128',
    'ExceptionHandling',
    'TailCall',
    'ReferenceTypes',
    'Multivalue',
    'GC',
    'Memory64',
    'TypedFunctionReferences',
    'All'
  ].forEach(name => {
    Module['Features'][name] = Module['_BinaryenFeature' + name]();
  });

  // Operations
  Module['Operations'] = {};
  [ 'ClzInt32',
    'CtzInt32',
    'PopcntInt32',
    'NegFloat32',
    'AbsFloat32',
    'CeilFloat32',
    'FloorFloat32',
    'TruncFloat32',
    'NearestFloat32',
    'SqrtFloat32',
    'EqZInt32',
    'ClzInt64',
    'CtzInt64',
    'PopcntInt64',
    'NegFloat64',
    'AbsFloat64',
    'CeilFloat64',
    'FloorFloat64',
    'TruncFloat64',
    'NearestFloat64',
    'SqrtFloat64',
    'EqZInt64',
    'ExtendSInt32',
    'ExtendUInt32',
    'WrapInt64',
    'TruncSFloat32ToInt32',
    'TruncSFloat32ToInt64',
    'TruncUFloat32ToInt32',
    'TruncUFloat32ToInt64',
    'TruncSFloat64ToInt32',
    'TruncSFloat64ToInt64',
    'TruncUFloat64ToInt32',
    'TruncUFloat64ToInt64',
    'TruncSatSFloat32ToInt32',
    'TruncSatSFloat32ToInt64',
    'TruncSatUFloat32ToInt32',
    'TruncSatUFloat32ToInt64',
    'TruncSatSFloat64ToInt32',
    'TruncSatSFloat64ToInt64',
    'TruncSatUFloat64ToInt32',
    'TruncSatUFloat64ToInt64',
    'ReinterpretFloat32',
    'ReinterpretFloat64',
    'ConvertSInt32ToFloat32',
    'ConvertSInt32ToFloat64',
    'ConvertUInt32ToFloat32',
    'ConvertUInt32ToFloat64',
    'ConvertSInt64ToFloat32',
    'ConvertSInt64ToFloat64',
    'ConvertUInt64ToFloat32',
    'ConvertUInt64ToFloat64',
    'PromoteFloat32',
    'DemoteFloat64',
    'ReinterpretInt32',
    'ReinterpretInt64',
    'ExtendS8Int32',
    'ExtendS16Int32',
    'ExtendS8Int64',
    'ExtendS16Int64',
    'ExtendS32Int64',
    'AddInt32',
    'SubInt32',
    'MulInt32',
    'DivSInt32',
    'DivUInt32',
    'RemSInt32',
    'RemUInt32',
    'AndInt32',
    'OrInt32',
    'XorInt32',
    'ShlInt32',
    'ShrUInt32',
    'ShrSInt32',
    'RotLInt32',
    'RotRInt32',
    'EqInt32',
    'NeInt32',
    'LtSInt32',
    'LtUInt32',
    'LeSInt32',
    'LeUInt32',
    'GtSInt32',
    'GtUInt32',
    'GeSInt32',
    'GeUInt32',
    'AddInt64',
    'SubInt64',
    'MulInt64',
    'DivSInt64',
    'DivUInt64',
    'RemSInt64',
    'RemUInt64',
    'AndInt64',
    'OrInt64',
    'XorInt64',
    'ShlInt64',
    'ShrUInt64',
    'ShrSInt64',
    'RotLInt64',
    'RotRInt64',
    'EqInt64',
    'NeInt64',
    'LtSInt64',
    'LtUInt64',
    'LeSInt64',
    'LeUInt64',
    'GtSInt64',
    'GtUInt64',
    'GeSInt64',
    'GeUInt64',
    'AddFloat32',
    'SubFloat32',
    'MulFloat32',
    'DivFloat32',
    'CopySignFloat32',
    'MinFloat32',
    'MaxFloat32',
    'EqFloat32',
    'NeFloat32',
    'LtFloat32',
    'LeFloat32',
    'GtFloat32',
    'GeFloat32',
    'AddFloat64',
    'SubFloat64',
    'MulFloat64',
    'DivFloat64',
    'CopySignFloat64',
    'MinFloat64',
    'MaxFloat64',
    'EqFloat64',
    'NeFloat64',
    'LtFloat64',
    'LeFloat64',
    'GtFloat64',
    'GeFloat64',
    'AtomicRMWAdd',
    'AtomicRMWSub',
    'AtomicRMWAnd',
    'AtomicRMWOr',
    'AtomicRMWXor',
    'AtomicRMWXchg',
    'SplatVecI8x16',
    'ExtractLaneSVecI8x16',
    'ExtractLaneUVecI8x16',
    'ReplaceLaneVecI8x16',
    'SplatVecI16x8',
    'ExtractLaneSVecI16x8',
    'ExtractLaneUVecI16x8',
    'ReplaceLaneVecI16x8',
    'SplatVecI32x4',
    'ExtractLaneVecI32x4',
    'ReplaceLaneVecI32x4',
    'SplatVecI64x2',
    'ExtractLaneVecI64x2',
    'ReplaceLaneVecI64x2',
    'SplatVecF32x4',
    'ExtractLaneVecF32x4',
    'ReplaceLaneVecF32x4',
    'SplatVecF64x2',
    'ExtractLaneVecF64x2',
    'ReplaceLaneVecF64x2',
    'EqVecI8x16',
    'NeVecI8x16',
    'LtSVecI8x16',
    'LtUVecI8x16',
    'GtSVecI8x16',
    'GtUVecI8x16',
    'LeSVecI8x16',
    'LeUVecI8x16',
    'GeSVecI8x16',
    'GeUVecI8x16',
    'EqVecI16x8',
    'NeVecI16x8',
    'LtSVecI16x8',
    'LtUVecI16x8',
    'GtSVecI16x8',
    'GtUVecI16x8',
    'LeSVecI16x8',
    'LeUVecI16x8',
    'GeSVecI16x8',
    'GeUVecI16x8',
    'EqVecI32x4',
    'NeVecI32x4',
    'LtSVecI32x4',
    'LtUVecI32x4',
    'GtSVecI32x4',
    'GtUVecI32x4',
    'LeSVecI32x4',
    'LeUVecI32x4',
    'GeSVecI32x4',
    'GeUVecI32x4',
    'EqVecI64x2',
    'NeVecI64x2',
    'LtSVecI64x2',
    'GtSVecI64x2',
    'LeSVecI64x2',
    'GeSVecI64x2',
    'EqVecF32x4',
    'NeVecF32x4',
    'LtVecF32x4',
    'GtVecF32x4',
    'LeVecF32x4',
    'GeVecF32x4',
    'EqVecF64x2',
    'NeVecF64x2',
    'LtVecF64x2',
    'GtVecF64x2',
    'LeVecF64x2',
    'GeVecF64x2',
    'NotVec128',
    'AndVec128',
    'OrVec128',
    'XorVec128',
    'AndNotVec128',
    'BitselectVec128',
    'AnyTrueVec128',
    'PopcntVecI8x16',
    'AbsVecI8x16',
    'NegVecI8x16',
    'AllTrueVecI8x16',
    'BitmaskVecI8x16',
    'ShlVecI8x16',
    'ShrSVecI8x16',
    'ShrUVecI8x16',
    'AddVecI8x16',
    'AddSatSVecI8x16',
    'AddSatUVecI8x16',
    'SubVecI8x16',
    'SubSatSVecI8x16',
    'SubSatUVecI8x16',
    'MinSVecI8x16',
    'MinUVecI8x16',
    'MaxSVecI8x16',
    'MaxUVecI8x16',
    'AvgrUVecI8x16',
    'AbsVecI16x8',
    'NegVecI16x8',
    'AllTrueVecI16x8',
    'BitmaskVecI16x8',
    'ShlVecI16x8',
    'ShrSVecI16x8',
    'ShrUVecI16x8',
    'AddVecI16x8',
    'AddSatSVecI16x8',
    'AddSatUVecI16x8',
    'SubVecI16x8',
    'SubSatSVecI16x8',
    'SubSatUVecI16x8',
    'MulVecI16x8',
    'MinSVecI16x8',
    'MinUVecI16x8',
    'MaxSVecI16x8',
    'MaxUVecI16x8',
    'AvgrUVecI16x8',
    'Q15MulrSatSVecI16x8',
    'ExtMulLowSVecI16x8',
    'ExtMulHighSVecI16x8',
    'ExtMulLowUVecI16x8',
    'ExtMulHighUVecI16x8',
    'DotSVecI16x8ToVecI32x4',
    'ExtMulLowSVecI32x4',
    'ExtMulHighSVecI32x4',
    'ExtMulLowUVecI32x4',
    'ExtMulHighUVecI32x4',
    'AbsVecI32x4',
    'NegVecI32x4',
    'AllTrueVecI32x4',
    'BitmaskVecI32x4',
    'ShlVecI32x4',
    'ShrSVecI32x4',
    'ShrUVecI32x4',
    'AddVecI32x4',
    'SubVecI32x4',
    'MulVecI32x4',
    'MinSVecI32x4',
    'MinUVecI32x4',
    'MaxSVecI32x4',
    'MaxUVecI32x4',
    'AbsVecI64x2',
    'NegVecI64x2',
    'AllTrueVecI64x2',
    'BitmaskVecI64x2',
    'ShlVecI64x2',
    'ShrSVecI64x2',
    'ShrUVecI64x2',
    'AddVecI64x2',
    'SubVecI64x2',
    'MulVecI64x2',
    'ExtMulLowSVecI64x2',
    'ExtMulHighSVecI64x2',
    'ExtMulLowUVecI64x2',
    'ExtMulHighUVecI64x2',
    'AbsVecF32x4',
    'NegVecF32x4',
    'SqrtVecF32x4',
    'AddVecF32x4',
    'SubVecF32x4',
    'MulVecF32x4',
    'DivVecF32x4',
    'MinVecF32x4',
    'MaxVecF32x4',
    'PMinVecF32x4',
    'PMaxVecF32x4',
    'CeilVecF32x4',
    'FloorVecF32x4',
    'TruncVecF32x4',
    'NearestVecF32x4',
    'AbsVecF64x2',
    'NegVecF64x2',
    'SqrtVecF64x2',
    'AddVecF64x2',
    'SubVecF64x2',
    'MulVecF64x2',
    'DivVecF64x2',
    'MinVecF64x2',
    'MaxVecF64x2',
    'PMinVecF64x2',
    'PMaxVecF64x2',
    'CeilVecF64x2',
    'FloorVecF64x2',
    'TruncVecF64x2',
    'NearestVecF64x2',
    'ExtAddPairwiseSVecI8x16ToI16x8',
    'ExtAddPairwiseUVecI8x16ToI16x8',
    'ExtAddPairwiseSVecI16x8ToI32x4',
    'ExtAddPairwiseUVecI16x8ToI32x4',
    'TruncSatSVecF32x4ToVecI32x4',
    'TruncSatUVecF32x4ToVecI32x4',
    'ConvertSVecI32x4ToVecF32x4',
    'ConvertUVecI32x4ToVecF32x4',
    'Load8SplatVec128',
    'Load16SplatVec128',
    'Load32SplatVec128',
    'Load64SplatVec128',
    'Load8x8SVec128',
    'Load8x8UVec128',
    'Load16x4SVec128',
    'Load16x4UVec128',
    'Load32x2SVec128',
    'Load32x2UVec128',
    'Load32ZeroVec128',
    'Load64ZeroVec128',
    'Load8LaneVec128',
    'Load16LaneVec128',
    'Load32LaneVec128',
    'Load64LaneVec128',
    'Store8LaneVec128',
    'Store16LaneVec128',
    'Store32LaneVec128',
    'Store64LaneVec128',
    'NarrowSVecI16x8ToVecI8x16',
    'NarrowUVecI16x8ToVecI8x16',
    'NarrowSVecI32x4ToVecI16x8',
    'NarrowUVecI32x4ToVecI16x8',
    'ExtendLowSVecI8x16ToVecI16x8',
    'ExtendHighSVecI8x16ToVecI16x8',
    'ExtendLowUVecI8x16ToVecI16x8',
    'ExtendHighUVecI8x16ToVecI16x8',
    'ExtendLowSVecI16x8ToVecI32x4',
    'ExtendHighSVecI16x8ToVecI32x4',
    'ExtendLowUVecI16x8ToVecI32x4',
    'ExtendHighUVecI16x8ToVecI32x4',
    'ExtendLowSVecI32x4ToVecI64x2',
    'ExtendHighSVecI32x4ToVecI64x2',
    'ExtendLowUVecI32x4ToVecI64x2',
    'ExtendHighUVecI32x4ToVecI64x2',
    'ConvertLowSVecI32x4ToVecF64x2',
    'ConvertLowUVecI32x4ToVecF64x2',
    'TruncSatZeroSVecF64x2ToVecI32x4',
    'TruncSatZeroUVecF64x2ToVecI32x4',
    'DemoteZeroVecF64x2ToVecF32x4',
    'PromoteLowVecF32x4ToVecF64x2',
    'SwizzleVec8x16',
    'RefIsNull',
    'RefIsFunc',
    'RefIsData',
    'RefIsI31',
    'RefAsNonNull',
    'RefAsFunc',
    'RefAsData',
    'RefAsI31',
  ].forEach(name => {
    Module['Operations'][name] = Module[name] = Module['_Binaryen' + name]();
  });

  // Expression side effects
  Module['SideEffects'] = {};
  [ 'None',
    'Branches',
    'Calls',
    'ReadsLocal',
    'WritesLocal',
    'ReadsGlobal',
    'WritesGlobal',
    'ReadsMemory',
    'WritesMemory',
    // TODO: add ReadsDataSegment & WritesDataSegment too
    'ReadsTable',
    'WritesTable',
    'ReadsElementSegment',
    'DropsElementSegment',
    'ImplicitTrap',
    'IsAtomic',
    'Throws',
    'DanglingPop',
    'Any'
  ].forEach(name => {
    Module['SideEffects'][name] = Module['_BinaryenSideEffect' + name]();
  });

  // ExpressionRunner flags
  Module['ExpressionRunner']['Flags'] = {
    'Default': Module['_ExpressionRunnerFlagsDefault'](),
    'PreserveSideeffects': Module['_ExpressionRunnerFlagsPreserveSideeffects'](),
    'TraverseCalls': Module['_ExpressionRunnerFlagsTraverseCalls']()
  };
}

// 'Module' interface
Module['Module'] = function(module) {
  assert(!module); // guard against incorrect old API usage
  wrapModule(Module['_BinaryenModuleCreate'](), this);
};

// Receives a C pointer to a C Module and a JS object, and creates
// the JS wrappings on the object to access the C data.
// This is meant for internal use only, and is necessary as we
// want to access Module from JS that were perhaps not created
// from JS.
function wrapModule(module, self = {}) {
  assert(module); // guard against incorrect old API usage

  self['ptr'] = module;

  // The size of a single literal in memory as used in Const creation,
  // which is a little different: we don't want users to need to make
  // their own Literals, as the C API handles them by value, which means
  // we would leak them. Instead, Const creation is fused together with
  // an intermediate stack allocation of this size to pass the value.
  const sizeOfLiteral = _BinaryenSizeofLiteral();

  // 'Expression' creation
  self['block'] = function(name, children, type) {
    return preserveStack(() =>
      Module['_BinaryenBlock'](module, name ? strToStack(name) : 0,
                               i32sToStack(children), children.length,
                               typeof type !== 'undefined' ? type : Module['none'])
    );
  };
  self['if'] = function(condition, ifTrue, ifFalse) {
    return Module['_BinaryenIf'](module, condition, ifTrue, ifFalse);
  };
  self['loop'] = function(label, body) {
    return preserveStack(() => Module['_BinaryenLoop'](module, strToStack(label), body));
  };
  self['break'] = self['br'] = function(label, condition, value) {
    return preserveStack(() => Module['_BinaryenBreak'](module, strToStack(label), condition, value));
  };
  self['br_if'] = function(label, condition, value) {
    return self['br'](label, condition, value);
  };
  self['switch'] = function(names, defaultName, condition, value) {
    return preserveStack(() =>
      Module['_BinaryenSwitch'](module, i32sToStack(names.map(strToStack)), names.length, strToStack(defaultName), condition, value)
    );
  };
  self['call'] = function(name, operands, type) {
    return preserveStack(() => Module['_BinaryenCall'](module, strToStack(name), i32sToStack(operands), operands.length, type));
  };
  // 'callIndirect', 'returnCall', 'returnCallIndirect' are deprecated and may
  // be removed in a future release. Please use the the snake_case names
  // instead.
  self['callIndirect'] = self['call_indirect'] = function(table, target, operands, params, results) {
    return preserveStack(() =>
      Module['_BinaryenCallIndirect'](module, strToStack(table), target, i32sToStack(operands), operands.length, params, results)
    );
  };
  self['returnCall'] = self['return_call'] = function(name, operands, type) {
    return preserveStack(() =>
      Module['_BinaryenReturnCall'](module, strToStack(name), i32sToStack(operands), operands.length, type)
    );
  };
  self['returnCallIndirect'] = self['return_call_indirect'] = function(table, target, operands, params, results) {
    return preserveStack(() =>
      Module['_BinaryenReturnCallIndirect'](module, strToStack(table), target, i32sToStack(operands), operands.length, params, results)
    );
  };

  self['local'] = {
    'get'(index, type) {
      return Module['_BinaryenLocalGet'](module, index, type);
    },
    'set'(index, value) {
      return Module['_BinaryenLocalSet'](module, index, value);
    },
    'tee'(index, value, type) {
      if (typeof type === 'undefined') {
        throw new Error("local.tee's type should be defined");
      }
      return Module['_BinaryenLocalTee'](module, index, value, type);
    }
  }

  self['global'] = {
    'get'(name, type) {
      return Module['_BinaryenGlobalGet'](module, strToStack(name), type);
    },
    'set'(name, value) {
      return Module['_BinaryenGlobalSet'](module, strToStack(name), value);
    }
  }

  self['memory'] = {
    'size'() {
      return Module['_BinaryenMemorySize'](module);
    },
    'grow'(value) {
      return Module['_BinaryenMemoryGrow'](module, value);
    },
    'init'(segment, dest, offset, size) {
      return Module['_BinaryenMemoryInit'](module, segment, dest, offset, size);
    },
    'copy'(dest, source, size) {
      return Module['_BinaryenMemoryCopy'](module, dest, source, size);
    },
    'fill'(dest, value, size) {
      return Module['_BinaryenMemoryFill'](module, dest, value, size);
    },
    'atomic': {
      'notify'(ptr, notifyCount) {
        return Module['_BinaryenAtomicNotify'](module, ptr, notifyCount);
      },
      'wait32'(ptr, expected, timeout) {
        return Module['_BinaryenAtomicWait'](module, ptr, expected, timeout, Module['i32']);
      },
      'wait64'(ptr, expected, timeout) {
        return Module['_BinaryenAtomicWait'](module, ptr, expected, timeout, Module['i64']);
      }
    }
  }

  self['data'] = {
    'drop'(segment) {
      return Module['_BinaryenDataDrop'](module, segment);
    }
  }

  self['table'] = {
    'get'(table, offset, type) {
      return Module['_BinaryenTableGet'](module, strToStack(table), offset, type);
    },
    'set'(table, value, offset, type) {
      return Module['_BinaryenTableSet'](module, strToStack(table), value, offset, type);
    },
    'size'(table) {
      Module['_BinaryenTableSize'](module, strToStack(table));
    },
    'grow'(table, delta, initialValue) {
      Module['_BinaryenTableGrow'](module, strToStack(table), delta, initialValue);
    },
    'fill'(table, size, value, dest) {
      Module['_BinaryenTableFill'](module, strToStack(table), size, value, dest);
    },
    'copy'(srcTable, destTable, size, srcOffset, destOffset) {
      Module['_BinaryenTableCopy'](module, strToStack(srcTable), strToStack(destTable), size, srcOffset, destOffset);
    },
    'init'(table, segment, size, srcOffset, destOffset) {
      Module['_BinaryenTableInit'](module, strToStack(table), strToStack(segment), size, srcOffset, destOffset);
    }
  }

  self['elem'] = {
    'drop'(segment) {
      return Module['_BinaryenElemDrop'](module, strToStack(segment));
    }
  }

  self['i32'] = {
    'load'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 4, true, offset, align, Module['i32'], ptr);
    },
    'load8_s'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 1, true, offset, align, Module['i32'], ptr);
    },
    'load8_u'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 1, false, offset, align, Module['i32'], ptr);
    },
    'load16_s'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 2, true, offset, align, Module['i32'], ptr);
    },
    'load16_u'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 2, false, offset, align, Module['i32'], ptr);
    },
    'store'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 4, offset, align, ptr, value, Module['i32']);
    },
    'store8'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 1, offset, align, ptr, value, Module['i32']);
    },
    'store16'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 2, offset, align, ptr, value, Module['i32']);
    },
    'const'(x) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralInt32'](tempLiteral, x);
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'clz'(value) {
      return Module['_BinaryenUnary'](module, Module['ClzInt32'], value);
    },
    'ctz'(value) {
      return Module['_BinaryenUnary'](module, Module['CtzInt32'], value);
    },
    'popcnt'(value) {
      return Module['_BinaryenUnary'](module, Module['PopcntInt32'], value);
    },
    'eqz'(value) {
      return Module['_BinaryenUnary'](module, Module['EqZInt32'], value);
    },
    'trunc_s': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSFloat32ToInt32'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSFloat64ToInt32'], value);
      },
    },
    'trunc_u': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncUFloat32ToInt32'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncUFloat64ToInt32'], value);
      },
    },
    'trunc_s_sat': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatSFloat32ToInt32'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatSFloat64ToInt32'], value);
      },
    },
    'trunc_u_sat': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatUFloat32ToInt32'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatUFloat64ToInt32'], value);
      },
    },
    'reinterpret'(value) {
      return Module['_BinaryenUnary'](module, Module['ReinterpretFloat32'], value);
    },
    'extend8_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendS8Int32'], value);
    },
    'extend16_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendS16Int32'], value);
    },
    'wrap'(value) {
      return Module['_BinaryenUnary'](module, Module['WrapInt64'], value);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddInt32'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubInt32'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulInt32'], left, right);
    },
    'div_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivSInt32'], left, right);
    },
    'div_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivUInt32'], left, right);
    },
    'rem_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RemSInt32'], left, right);
    },
    'rem_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RemUInt32'], left, right);
    },
    'and'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AndInt32'], left, right);
    },
    'or'(left, right) {
      return Module['_BinaryenBinary'](module, Module['OrInt32'], left, right);
    },
    'xor'(left, right) {
      return Module['_BinaryenBinary'](module, Module['XorInt32'], left, right);
    },
    'shl'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ShlInt32'], left, right);
    },
    'shr_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ShrUInt32'], left, right);
    },
    'shr_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ShrSInt32'], left, right);
    },
    'rotl'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RotLInt32'], left, right);
    },
    'rotr'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RotRInt32'], left, right);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqInt32'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeInt32'], left, right);
    },
    'lt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtSInt32'], left, right);
    },
    'lt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtUInt32'], left, right);
    },
    'le_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeSInt32'], left, right);
    },
    'le_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeUInt32'], left, right);
    },
    'gt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtSInt32'], left, right);
    },
    'gt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtUInt32'], left, right);
    },
    'ge_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeSInt32'], left, right);
    },
    'ge_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeUInt32'], left, right);
    },
    'atomic': {
      'load'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 4, offset, Module['i32'], ptr);
      },
      'load8_u'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 1, offset, Module['i32'], ptr);
      },
      'load16_u'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 2, offset, Module['i32'], ptr);
      },
      'store'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 4, offset, ptr, value, Module['i32']);
      },
      'store8'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 1, offset, ptr, value, Module['i32']);
      },
      'store16'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 2, offset, ptr, value, Module['i32']);
      },
      'rmw': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 4, offset, ptr, value, Module['i32']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 4, offset, ptr, value, Module['i32']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 4, offset, ptr, value, Module['i32']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 4, offset, ptr, value, Module['i32']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 4, offset, ptr, value, Module['i32']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 4, offset, ptr, value, Module['i32']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 4, offset, ptr, expected, replacement, Module['i32'])
        },
      },
      'rmw8_u': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 1, offset, ptr, value, Module['i32']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 1, offset, ptr, value, Module['i32']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 1, offset, ptr, value, Module['i32']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 1, offset, ptr, value, Module['i32']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 1, offset, ptr, value, Module['i32']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 1, offset, ptr, value, Module['i32']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 1, offset, ptr, expected, replacement, Module['i32'])
        },
      },
      'rmw16_u': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 2, offset, ptr, value, Module['i32']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 2, offset, ptr, value, Module['i32']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 2, offset, ptr, value, Module['i32']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 2, offset, ptr, value, Module['i32']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 2, offset, ptr, value, Module['i32']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 2, offset, ptr, value, Module['i32']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 2, offset, ptr, expected, replacement, Module['i32'])
        },
      },
    },
    'pop'() {
      return Module['_BinaryenPop'](module, Module['i32']);
    }
  };

  self['i64'] = {
    'load'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 8, true, offset, align, Module['i64'], ptr);
    },
    'load8_s'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 1, true, offset, align, Module['i64'], ptr);
    },
    'load8_u'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 1, false, offset, align, Module['i64'], ptr);
    },
    'load16_s'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 2, true, offset, align, Module['i64'], ptr);
    },
    'load16_u'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 2, false, offset, align, Module['i64'], ptr);
    },
    'load32_s'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 4, true, offset, align, Module['i64'], ptr);
    },
    'load32_u'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 4, false, offset, align, Module['i64'], ptr);
    },
    'store'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 8, offset, align, ptr, value, Module['i64']);
    },
    'store8'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 1, offset, align, ptr, value, Module['i64']);
    },
    'store16'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 2, offset, align, ptr, value, Module['i64']);
    },
    'store32'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 4, offset, align, ptr, value, Module['i64']);
    },
    'const'(x, y) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralInt64'](tempLiteral, x, y);
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'clz'(value) {
      return Module['_BinaryenUnary'](module, Module['ClzInt64'], value);
    },
    'ctz'(value) {
      return Module['_BinaryenUnary'](module, Module['CtzInt64'], value);
    },
    'popcnt'(value) {
      return Module['_BinaryenUnary'](module, Module['PopcntInt64'], value);
    },
    'eqz'(value) {
      return Module['_BinaryenUnary'](module, Module['EqZInt64'], value);
    },
    'trunc_s': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSFloat32ToInt64'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSFloat64ToInt64'], value);
      },
    },
    'trunc_u': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncUFloat32ToInt64'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncUFloat64ToInt64'], value);
      },
    },
    'trunc_s_sat': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatSFloat32ToInt64'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatSFloat64ToInt64'], value);
      },
    },
    'trunc_u_sat': {
      'f32'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatUFloat32ToInt64'], value);
      },
      'f64'(value) {
        return Module['_BinaryenUnary'](module, Module['TruncSatUFloat64ToInt64'], value);
      },
    },
    'reinterpret'(value) {
      return Module['_BinaryenUnary'](module, Module['ReinterpretFloat64'], value);
    },
    'extend8_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendS8Int64'], value);
    },
    'extend16_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendS16Int64'], value);
    },
    'extend32_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendS32Int64'], value);
    },
    'extend_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendSInt32'], value);
    },
    'extend_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendUInt32'], value);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddInt64'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubInt64'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulInt64'], left, right);
    },
    'div_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivSInt64'], left, right);
    },
    'div_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivUInt64'], left, right);
    },
    'rem_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RemSInt64'], left, right);
    },
    'rem_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RemUInt64'], left, right);
    },
    'and'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AndInt64'], left, right);
    },
    'or'(left, right) {
      return Module['_BinaryenBinary'](module, Module['OrInt64'], left, right);
    },
    'xor'(left, right) {
      return Module['_BinaryenBinary'](module, Module['XorInt64'], left, right);
    },
    'shl'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ShlInt64'], left, right);
    },
    'shr_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ShrUInt64'], left, right);
    },
    'shr_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ShrSInt64'], left, right);
    },
    'rotl'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RotLInt64'], left, right);
    },
    'rotr'(left, right) {
      return Module['_BinaryenBinary'](module, Module['RotRInt64'], left, right);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqInt64'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeInt64'], left, right);
    },
    'lt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtSInt64'], left, right);
    },
    'lt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtUInt64'], left, right);
    },
    'le_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeSInt64'], left, right);
    },
    'le_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeUInt64'], left, right);
    },
    'gt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtSInt64'], left, right);
    },
    'gt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtUInt64'], left, right);
    },
    'ge_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeSInt64'], left, right);
    },
    'ge_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeUInt64'], left, right);
    },
    'atomic': {
      'load'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 8, offset, Module['i64'], ptr);
      },
      'load8_u'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 1, offset, Module['i64'], ptr);
      },
      'load16_u'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 2, offset, Module['i64'], ptr);
      },
      'load32_u'(offset, ptr) {
        return Module['_BinaryenAtomicLoad'](module, 4, offset, Module['i64'], ptr);
      },
      'store'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 8, offset, ptr, value, Module['i64']);
      },
      'store8'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 1, offset, ptr, value, Module['i64']);
      },
      'store16'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 2, offset, ptr, value, Module['i64']);
      },
      'store32'(offset, ptr, value) {
        return Module['_BinaryenAtomicStore'](module, 4, offset, ptr, value, Module['i64']);
      },
      'rmw': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 8, offset, ptr, value, Module['i64']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 8, offset, ptr, value, Module['i64']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 8, offset, ptr, value, Module['i64']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 8, offset, ptr, value, Module['i64']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 8, offset, ptr, value, Module['i64']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 8, offset, ptr, value, Module['i64']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 8, offset, ptr, expected, replacement, Module['i64'])
        },
      },
      'rmw8_u': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 1, offset, ptr, value, Module['i64']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 1, offset, ptr, value, Module['i64']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 1, offset, ptr, value, Module['i64']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 1, offset, ptr, value, Module['i64']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 1, offset, ptr, value, Module['i64']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 1, offset, ptr, value, Module['i64']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 1, offset, ptr, expected, replacement, Module['i64'])
        },
      },
      'rmw16_u': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 2, offset, ptr, value, Module['i64']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 2, offset, ptr, value, Module['i64']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 2, offset, ptr, value, Module['i64']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 2, offset, ptr, value, Module['i64']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 2, offset, ptr, value, Module['i64']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 2, offset, ptr, value, Module['i64']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 2, offset, ptr, expected, replacement, Module['i64'])
        },
      },
      'rmw32_u': {
        'add'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAdd'], 4, offset, ptr, value, Module['i64']);
        },
        'sub'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWSub'], 4, offset, ptr, value, Module['i64']);
        },
        'and'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWAnd'], 4, offset, ptr, value, Module['i64']);
        },
        'or'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWOr'], 4, offset, ptr, value, Module['i64']);
        },
        'xor'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXor'], 4, offset, ptr, value, Module['i64']);
        },
        'xchg'(offset, ptr, value) {
          return Module['_BinaryenAtomicRMW'](module, Module['AtomicRMWXchg'], 4, offset, ptr, value, Module['i64']);
        },
        'cmpxchg'(offset, ptr, expected, replacement) {
          return Module['_BinaryenAtomicCmpxchg'](module, 4, offset, ptr, expected, replacement, Module['i64'])
        },
      },
    },
    'pop'() {
      return Module['_BinaryenPop'](module, Module['i64']);
    }
  };

  self['f32'] = {
    'load'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 4, true, offset, align, Module['f32'], ptr);
    },
    'store'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 4, offset, align, ptr, value, Module['f32']);
    },
    'const'(x) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralFloat32'](tempLiteral, x);
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'const_bits'(x) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralFloat32Bits'](tempLiteral, x);
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegFloat32'], value);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsFloat32'], value);
    },
    'ceil'(value) {
      return Module['_BinaryenUnary'](module, Module['CeilFloat32'], value);
    },
    'floor'(value) {
      return Module['_BinaryenUnary'](module, Module['FloorFloat32'], value);
    },
    'trunc'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncFloat32'], value);
    },
    'nearest'(value) {
      return Module['_BinaryenUnary'](module, Module['NearestFloat32'], value);
    },
    'sqrt'(value) {
      return Module['_BinaryenUnary'](module, Module['SqrtFloat32'], value);
    },
    'reinterpret'(value) {
      return Module['_BinaryenUnary'](module, Module['ReinterpretInt32'], value);
    },
    'convert_s': {
      'i32'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertSInt32ToFloat32'], value);
      },
      'i64'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertSInt64ToFloat32'], value);
      },
    },
    'convert_u': {
      'i32'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertUInt32ToFloat32'], value);
      },
      'i64'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertUInt64ToFloat32'], value);
      },
    },
    'demote'(value) {
      return Module['_BinaryenUnary'](module, Module['DemoteFloat64'], value);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddFloat32'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubFloat32'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulFloat32'], left, right);
    },
    'div'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivFloat32'], left, right);
    },
    'copysign'(left, right) {
      return Module['_BinaryenBinary'](module, Module['CopySignFloat32'], left, right);
    },
    'min'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinFloat32'], left, right);
    },
    'max'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxFloat32'], left, right);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqFloat32'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeFloat32'], left, right);
    },
    'lt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtFloat32'], left, right);
    },
    'le'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeFloat32'], left, right);
    },
    'gt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtFloat32'], left, right);
    },
    'ge'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeFloat32'], left, right);
    },
    'pop'() {
      return Module['_BinaryenPop'](module, Module['f32']);
    }
  };

  self['f64'] = {
    'load'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 8, true, offset, align, Module['f64'], ptr);
    },
    'store'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 8, offset, align, ptr, value, Module['f64']);
    },
    'const'(x) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralFloat64'](tempLiteral, x);
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'const_bits'(x, y) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralFloat64Bits'](tempLiteral, x, y);
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegFloat64'], value);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsFloat64'], value);
    },
    'ceil'(value) {
      return Module['_BinaryenUnary'](module, Module['CeilFloat64'], value);
    },
    'floor'(value) {
      return Module['_BinaryenUnary'](module, Module['FloorFloat64'], value);
    },
    'trunc'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncFloat64'], value);
    },
    'nearest'(value) {
      return Module['_BinaryenUnary'](module, Module['NearestFloat64'], value);
    },
    'sqrt'(value) {
      return Module['_BinaryenUnary'](module, Module['SqrtFloat64'], value);
    },
    'reinterpret'(value) {
      return Module['_BinaryenUnary'](module, Module['ReinterpretInt64'], value);
    },
    'convert_s': {
      'i32'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertSInt32ToFloat64'], value);
      },
      'i64'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertSInt64ToFloat64'], value);
      },
    },
    'convert_u': {
      'i32'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertUInt32ToFloat64'], value);
      },
      'i64'(value) {
        return Module['_BinaryenUnary'](module, Module['ConvertUInt64ToFloat64'], value);
      },
    },
    'promote'(value) {
      return Module['_BinaryenUnary'](module, Module['PromoteFloat32'], value);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddFloat64'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubFloat64'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulFloat64'], left, right);
    },
    'div'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivFloat64'], left, right);
    },
    'copysign'(left, right) {
      return Module['_BinaryenBinary'](module, Module['CopySignFloat64'], left, right);
    },
    'min'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinFloat64'], left, right);
    },
    'max'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxFloat64'], left, right);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqFloat64'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeFloat64'], left, right);
    },
    'lt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtFloat64'], left, right);
    },
    'le'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeFloat64'], left, right);
    },
    'gt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtFloat64'], left, right);
    },
    'ge'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeFloat64'], left, right);
    },
    'pop'() {
      return Module['_BinaryenPop'](module, Module['f64']);
    }
  };

  self['v128'] = {
    'load'(offset, align, ptr) {
      return Module['_BinaryenLoad'](module, 16, false, offset, align, Module['v128'], ptr);
    },
    'load8_splat'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load8SplatVec128'], offset, align, ptr);
    },
    'load16_splat'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load16SplatVec128'], offset, align, ptr);
    },
    'load32_splat'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load32SplatVec128'], offset, align, ptr);
    },
    'load64_splat'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load64SplatVec128'], offset, align, ptr);
    },
    'load8x8_s'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load8x8SVec128'], offset, align, ptr);
    },
    'load8x8_u'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load8x8UVec128'], offset, align, ptr);
    },
    'load16x4_s'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load16x4SVec128'], offset, align, ptr);
    },
    'load16x4_u'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load16x4UVec128'], offset, align, ptr);
    },
    'load32x2_s'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load32x2SVec128'], offset, align, ptr);
    },
    'load32x2_u'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load32x2UVec128'], offset, align, ptr);
    },
    'load32_zero'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load32ZeroVec128'], offset, align, ptr);
    },
    'load64_zero'(offset, align, ptr) {
      return Module['_BinaryenSIMDLoad'](module, Module['Load64ZeroVec128'], offset, align, ptr);
    },
    'load8_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Load8LaneVec128'], offset, align, index, ptr, vec);
    },
    'load16_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Load16LaneVec128'], offset, align, index, ptr, vec);
    },
    'load32_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Load32LaneVec128'], offset, align, index, ptr, vec);
    },
    'load64_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Load64LaneVec128'], offset, align, index, ptr, vec);
    },
    'store8_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Store8LaneVec128'], offset, align, index, ptr, vec);
    },
    'store16_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Store16LaneVec128'], offset, align, index, ptr, vec);
    },
    'store32_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Store32LaneVec128'], offset, align, index, ptr, vec);
    },
    'store64_lane'(offset, align, index, ptr, vec) {
      return Module['_BinaryenSIMDLoadStoreLane'](module, Module['Store64LaneVec128'], offset, align, index, ptr, vec);
    },
    'store'(offset, align, ptr, value) {
      return Module['_BinaryenStore'](module, 16, offset, align, ptr, value, Module['v128']);
    },
    'const'(i8s) {
      return preserveStack(() => {
        const tempLiteral = stackAlloc(sizeOfLiteral);
        Module['_BinaryenLiteralVec128'](tempLiteral, i8sToStack(i8s));
        return Module['_BinaryenConst'](module, tempLiteral);
      });
    },
    'not'(value) {
      return Module['_BinaryenUnary'](module, Module['NotVec128'], value);
    },
    'any_true'(value) {
      return Module['_BinaryenUnary'](module, Module['AnyTrueVec128'], value);
    },
    'and'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AndVec128'], left, right);
    },
    'or'(left, right) {
      return Module['_BinaryenBinary'](module, Module['OrVec128'], left, right);
    },
    'xor'(left, right) {
      return Module['_BinaryenBinary'](module, Module['XorVec128'], left, right);
    },
    'andnot'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AndNotVec128'], left, right);
    },
    'bitselect'(left, right, cond) {
      return Module['_BinaryenSIMDTernary'](module, Module['BitselectVec128'], left, right, cond);
    },
    'pop'() {
      return Module['_BinaryenPop'](module, Module['v128']);
    }
  };

  self['i8x16'] = {
    'shuffle'(left, right, mask) {
      return preserveStack(() => Module['_BinaryenSIMDShuffle'](module, left, right, i8sToStack(mask)));
    },
    'swizzle'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SwizzleVec8x16'], left, right);
    },
    'splat'(value) {
      return Module['_BinaryenUnary'](module, Module['SplatVecI8x16'], value);
    },
    'extract_lane_s'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneSVecI8x16'], vec, index);
    },
    'extract_lane_u'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneUVecI8x16'], vec, index);
    },
    'replace_lane'(vec, index, value) {
      return Module['_BinaryenSIMDReplace'](module, Module['ReplaceLaneVecI8x16'], vec, index, value);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqVecI8x16'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeVecI8x16'], left, right);
    },
    'lt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtSVecI8x16'], left, right);
    },
    'lt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtUVecI8x16'], left, right);
    },
    'gt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtSVecI8x16'], left, right);
    },
    'gt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtUVecI8x16'], left, right);
    },
    'le_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeSVecI8x16'], left, right);
    },
    'le_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeUVecI8x16'], left, right);
    },
    'ge_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeSVecI8x16'], left, right);
    },
    'ge_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeUVecI8x16'], left, right);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsVecI8x16'], value);
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegVecI8x16'], value);
    },
    'all_true'(value) {
      return Module['_BinaryenUnary'](module, Module['AllTrueVecI8x16'], value);
    },
    'bitmask'(value) {
      return Module['_BinaryenUnary'](module, Module['BitmaskVecI8x16'], value);
    },
    'popcnt'(value) {
      return Module['_BinaryenUnary'](module, Module['PopcntVecI8x16'], value);
    },
    'shl'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShlVecI8x16'], vec, shift);
    },
    'shr_s'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrSVecI8x16'], vec, shift);
    },
    'shr_u'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrUVecI8x16'], vec, shift);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddVecI8x16'], left, right);
    },
    'add_saturate_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddSatSVecI8x16'], left, right);
    },
    'add_saturate_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddSatUVecI8x16'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubVecI8x16'], left, right);
    },
    'sub_saturate_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubSatSVecI8x16'], left, right);
    },
    'sub_saturate_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubSatUVecI8x16'], left, right);
    },
    'min_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinSVecI8x16'], left, right);
    },
    'min_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinUVecI8x16'], left, right);
    },
    'max_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxSVecI8x16'], left, right);
    },
    'max_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxUVecI8x16'], left, right);
    },
    'avgr_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AvgrUVecI8x16'], left, right);
    },
    'narrow_i16x8_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NarrowSVecI16x8ToVecI8x16'], left, right);
    },
    'narrow_i16x8_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NarrowUVecI16x8ToVecI8x16'], left, right);
    },
  };

  self['i16x8'] = {
    'splat'(value) {
      return Module['_BinaryenUnary'](module, Module['SplatVecI16x8'], value);
    },
    'extract_lane_s'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneSVecI16x8'], vec, index);
    },
    'extract_lane_u'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneUVecI16x8'], vec, index);
    },
    'replace_lane'(vec, index, value) {
      return Module['_BinaryenSIMDReplace'](module, Module['ReplaceLaneVecI16x8'], vec, index, value);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqVecI16x8'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeVecI16x8'], left, right);
    },
    'lt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtSVecI16x8'], left, right);
    },
    'lt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtUVecI16x8'], left, right);
    },
    'gt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtSVecI16x8'], left, right);
    },
    'gt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtUVecI16x8'], left, right);
    },
    'le_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeSVecI16x8'], left, right);
    },
    'le_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeUVecI16x8'], left, right);
    },
    'ge_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeSVecI16x8'], left, right);
    },
    'ge_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeUVecI16x8'], left, right);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsVecI16x8'], value);
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegVecI16x8'], value);
    },
    'all_true'(value) {
      return Module['_BinaryenUnary'](module, Module['AllTrueVecI16x8'], value);
    },
    'bitmask'(value) {
      return Module['_BinaryenUnary'](module, Module['BitmaskVecI16x8'], value);
    },
    'shl'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShlVecI16x8'], vec, shift);
    },
    'shr_s'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrSVecI16x8'], vec, shift);
    },
    'shr_u'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrUVecI16x8'], vec, shift);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddVecI16x8'], left, right);
    },
    'add_saturate_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddSatSVecI16x8'], left, right);
    },
    'add_saturate_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddSatUVecI16x8'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubVecI16x8'], left, right);
    },
    'sub_saturate_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubSatSVecI16x8'], left, right);
    },
    'sub_saturate_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubSatUVecI16x8'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulVecI16x8'], left, right);
    },
    'min_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinSVecI16x8'], left, right);
    },
    'min_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinUVecI16x8'], left, right);
    },
    'max_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxSVecI16x8'], left, right);
    },
    'max_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxUVecI16x8'], left, right);
    },
    'avgr_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AvgrUVecI16x8'], left, right);
    },
    'q15mulr_sat_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['Q15MulrSatSVecI16x8'], left, right);
    },
    'extmul_low_i8x16_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulLowSVecI16x8'], left, right);
    },
    'extmul_high_i8x16_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulHighSVecI16x8'], left, right);
    },
    'extmul_low_i8x16_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulLowUVecI16x8'], left, right);
    },
    'extmul_high_i8x16_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulHighUVecI16x8'], left, right);
    },
    'extadd_pairwise_i8x16_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtAddPairwiseSVecI8x16ToI16x8'], value);
    },
    'extadd_pairwise_i8x16_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtAddPairwiseUVecI8x16ToI16x8'], value);
    },
    'narrow_i32x4_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NarrowSVecI32x4ToVecI16x8'], left, right);
    },
    'narrow_i32x4_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NarrowUVecI32x4ToVecI16x8'], left, right);
    },
    'extend_low_i8x16_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendLowSVecI8x16ToVecI16x8'], value);
    },
    'extend_high_i8x16_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendHighSVecI8x16ToVecI16x8'], value);
    },
    'extend_low_i8x16_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendLowUVecI8x16ToVecI16x8'], value);
    },
    'extend_high_i8x16_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendHighUVecI8x16ToVecI16x8'], value);
    },
  };

  self['i32x4'] = {
    'splat'(value) {
      return Module['_BinaryenUnary'](module, Module['SplatVecI32x4'], value);
    },
    'extract_lane'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneVecI32x4'], vec, index);
    },
    'replace_lane'(vec, index, value) {
      return Module['_BinaryenSIMDReplace'](module, Module['ReplaceLaneVecI32x4'], vec, index, value);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqVecI32x4'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeVecI32x4'], left, right);
    },
    'lt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtSVecI32x4'], left, right);
    },
    'lt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtUVecI32x4'], left, right);
    },
    'gt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtSVecI32x4'], left, right);
    },
    'gt_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtUVecI32x4'], left, right);
    },
    'le_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeSVecI32x4'], left, right);
    },
    'le_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeUVecI32x4'], left, right);
    },
    'ge_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeSVecI32x4'], left, right);
    },
    'ge_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeUVecI32x4'], left, right);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsVecI32x4'], value);
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegVecI32x4'], value);
    },
    'all_true'(value) {
      return Module['_BinaryenUnary'](module, Module['AllTrueVecI32x4'], value);
    },
    'bitmask'(value) {
      return Module['_BinaryenUnary'](module, Module['BitmaskVecI32x4'], value);
    },
    'shl'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShlVecI32x4'], vec, shift);
    },
    'shr_s'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrSVecI32x4'], vec, shift);
    },
    'shr_u'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrUVecI32x4'], vec, shift);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddVecI32x4'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubVecI32x4'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulVecI32x4'], left, right);
    },
    'min_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinSVecI32x4'], left, right);
    },
    'min_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinUVecI32x4'], left, right);
    },
    'max_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxSVecI32x4'], left, right);
    },
    'max_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxUVecI32x4'], left, right);
    },
    'dot_i16x8_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DotSVecI16x8ToVecI32x4'], left, right);
    },
    'extmul_low_i16x8_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulLowSVecI32x4'], left, right);
    },
    'extmul_high_i16x8_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulHighSVecI32x4'], left, right);
    },
    'extmul_low_i16x8_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulLowUVecI32x4'], left, right);
    },
    'extmul_high_i16x8_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulHighUVecI32x4'], left, right);
    },
    'extadd_pairwise_i16x8_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtAddPairwiseSVecI16x8ToI32x4'], value);
    },
    'extadd_pairwise_i16x8_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtAddPairwiseUVecI16x8ToI32x4'], value);
    },
    'trunc_sat_f32x4_s'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncSatSVecF32x4ToVecI32x4'], value);
    },
    'trunc_sat_f32x4_u'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncSatUVecF32x4ToVecI32x4'], value);
    },
    'extend_low_i16x8_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendLowSVecI16x8ToVecI32x4'], value);
    },
    'extend_high_i16x8_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendHighSVecI16x8ToVecI32x4'], value);
    },
    'extend_low_i16x8_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendLowUVecI16x8ToVecI32x4'], value);
    },
    'extend_high_i16x8_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendHighUVecI16x8ToVecI32x4'], value);
    },
    'trunc_sat_f64x2_s_zero'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncSatZeroSVecF64x2ToVecI32x4'], value);
    },
    'trunc_sat_f64x2_u_zero'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncSatZeroUVecF64x2ToVecI32x4'], value);
    },
  };

  self['i64x2'] = {
    'splat'(value) {
      return Module['_BinaryenUnary'](module, Module['SplatVecI64x2'], value);
    },
    'extract_lane'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneVecI64x2'], vec, index);
    },
    'replace_lane'(vec, index, value) {
      return Module['_BinaryenSIMDReplace'](module, Module['ReplaceLaneVecI64x2'], vec, index, value);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqVecI64x2'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeVecI64x2'], left, right);
    },
    'lt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtSVecI64x2'], left, right);
    },
    'gt_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtSVecI64x2'], left, right);
    },
    'le_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeSVecI64x2'], left, right);
    },
    'ge_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeSVecI64x2'], left, right);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsVecI64x2'], value);
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegVecI64x2'], value);
    },
    'all_true'(value) {
      return Module['_BinaryenUnary'](module, Module['AllTrueVecI64x2'], value);
    },
    'bitmask'(value) {
      return Module['_BinaryenUnary'](module, Module['BitmaskVecI64x2'], value);
    },
    'shl'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShlVecI64x2'], vec, shift);
    },
    'shr_s'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrSVecI64x2'], vec, shift);
    },
    'shr_u'(vec, shift) {
      return Module['_BinaryenSIMDShift'](module, Module['ShrUVecI64x2'], vec, shift);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddVecI64x2'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubVecI64x2'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulVecI64x2'], left, right);
    },
    'extmul_low_i32x4_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulLowSVecI64x2'], left, right);
    },
    'extmul_high_i32x4_s'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulHighSVecI64x2'], left, right);
    },
    'extmul_low_i32x4_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulLowUVecI64x2'], left, right);
    },
    'extmul_high_i32x4_u'(left, right) {
      return Module['_BinaryenBinary'](module, Module['ExtMulHighUVecI64x2'], left, right);
    },
    'extend_low_i32x4_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendLowSVecI32x4ToVecI64x2'], value);
    },
    'extend_high_i32x4_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendHighSVecI32x4ToVecI64x2'], value);
    },
    'extend_low_i32x4_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendLowUVecI32x4ToVecI64x2'], value);
    },
    'extend_high_i32x4_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ExtendHighUVecI32x4ToVecI64x2'], value);
    },
  };

  self['f32x4'] = {
    'splat'(value) {
      return Module['_BinaryenUnary'](module, Module['SplatVecF32x4'], value);
    },
    'extract_lane'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneVecF32x4'], vec, index);
    },
    'replace_lane'(vec, index, value) {
      return Module['_BinaryenSIMDReplace'](module, Module['ReplaceLaneVecF32x4'], vec, index, value);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqVecF32x4'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeVecF32x4'], left, right);
    },
    'lt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtVecF32x4'], left, right);
    },
    'gt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtVecF32x4'], left, right);
    },
    'le'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeVecF32x4'], left, right);
    },
    'ge'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeVecF32x4'], left, right);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsVecF32x4'], value);
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegVecF32x4'], value);
    },
    'sqrt'(value) {
      return Module['_BinaryenUnary'](module, Module['SqrtVecF32x4'], value);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddVecF32x4'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubVecF32x4'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulVecF32x4'], left, right);
    },
    'div'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivVecF32x4'], left, right);
    },
    'min'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinVecF32x4'], left, right);
    },
    'max'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxVecF32x4'], left, right);
    },
    'pmin'(left, right) {
      return Module['_BinaryenBinary'](module, Module['PMinVecF32x4'], left, right);
    },
    'pmax'(left, right) {
      return Module['_BinaryenBinary'](module, Module['PMaxVecF32x4'], left, right);
    },
    'ceil'(value) {
      return Module['_BinaryenUnary'](module, Module['CeilVecF32x4'], value);
    },
    'floor'(value) {
      return Module['_BinaryenUnary'](module, Module['FloorVecF32x4'], value);
    },
    'trunc'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncVecF32x4'], value);
    },
    'nearest'(value) {
      return Module['_BinaryenUnary'](module, Module['NearestVecF32x4'], value);
    },
    'convert_i32x4_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ConvertSVecI32x4ToVecF32x4'], value);
    },
    'convert_i32x4_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ConvertUVecI32x4ToVecF32x4'], value);
    },
    'demote_f64x2_zero'(value) {
      return Module['_BinaryenUnary'](module, Module['DemoteZeroVecF64x2ToVecF32x4'], value);
    },
  };

  self['f64x2'] = {
    'splat'(value) {
      return Module['_BinaryenUnary'](module, Module['SplatVecF64x2'], value);
    },
    'extract_lane'(vec, index) {
      return Module['_BinaryenSIMDExtract'](module, Module['ExtractLaneVecF64x2'], vec, index);
    },
    'replace_lane'(vec, index, value) {
      return Module['_BinaryenSIMDReplace'](module, Module['ReplaceLaneVecF64x2'], vec, index, value);
    },
    'eq'(left, right) {
      return Module['_BinaryenBinary'](module, Module['EqVecF64x2'], left, right);
    },
    'ne'(left, right) {
      return Module['_BinaryenBinary'](module, Module['NeVecF64x2'], left, right);
    },
    'lt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LtVecF64x2'], left, right);
    },
    'gt'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GtVecF64x2'], left, right);
    },
    'le'(left, right) {
      return Module['_BinaryenBinary'](module, Module['LeVecF64x2'], left, right);
    },
    'ge'(left, right) {
      return Module['_BinaryenBinary'](module, Module['GeVecF64x2'], left, right);
    },
    'abs'(value) {
      return Module['_BinaryenUnary'](module, Module['AbsVecF64x2'], value);
    },
    'neg'(value) {
      return Module['_BinaryenUnary'](module, Module['NegVecF64x2'], value);
    },
    'sqrt'(value) {
      return Module['_BinaryenUnary'](module, Module['SqrtVecF64x2'], value);
    },
    'add'(left, right) {
      return Module['_BinaryenBinary'](module, Module['AddVecF64x2'], left, right);
    },
    'sub'(left, right) {
      return Module['_BinaryenBinary'](module, Module['SubVecF64x2'], left, right);
    },
    'mul'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MulVecF64x2'], left, right);
    },
    'div'(left, right) {
      return Module['_BinaryenBinary'](module, Module['DivVecF64x2'], left, right);
    },
    'min'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MinVecF64x2'], left, right);
    },
    'max'(left, right) {
      return Module['_BinaryenBinary'](module, Module['MaxVecF64x2'], left, right);
    },
    'pmin'(left, right) {
      return Module['_BinaryenBinary'](module, Module['PMinVecF64x2'], left, right);
    },
    'pmax'(left, right) {
      return Module['_BinaryenBinary'](module, Module['PMaxVecF64x2'], left, right);
    },
    'ceil'(value) {
      return Module['_BinaryenUnary'](module, Module['CeilVecF64x2'], value);
    },
    'floor'(value) {
      return Module['_BinaryenUnary'](module, Module['FloorVecF64x2'], value);
    },
    'trunc'(value) {
      return Module['_BinaryenUnary'](module, Module['TruncVecF64x2'], value);
    },
    'nearest'(value) {
      return Module['_BinaryenUnary'](module, Module['NearestVecF64x2'], value);
    },
    'convert_low_i32x4_s'(value) {
      return Module['_BinaryenUnary'](module, Module['ConvertLowSVecI32x4ToVecF64x2'], value);
    },
    'convert_low_i32x4_u'(value) {
      return Module['_BinaryenUnary'](module, Module['ConvertLowUVecI32x4ToVecF64x2'], value);
    },
    'promote_low_f32x4'(value) {
      return Module['_BinaryenUnary'](module, Module['PromoteLowVecF32x4ToVecF64x2'], value);
    },
  };

  self['funcref'] = {
    'pop'() {
      return Module['_BinaryenPop'](module, Module['funcref']);
    }
  };

  self['externref'] = {
    'pop'() {
      return Module['_BinaryenPop'](module, Module['externref']);
    }
  };

  self['anyref'] = {
    'pop'() {
      return Module['_BinaryenPop'](module, Module['anyref']);
    }
  };

  self['eqref'] = {
    'pop'() {
      return Module['_BinaryenPop'](module, Module['eqref']);
    }
  };

  self['i31ref'] = {
    'pop'() {
      return Module['_BinaryenPop'](module, Module['i31ref']);
    }
  };

  self['dataref'] = {
    'pop'() {
      return Module['_BinaryenPop'](module, Module['dataref']);
    }
  };

  self['ref'] = {
    'null'(type) {
      return Module['_BinaryenRefNull'](module, type);
    },
    'is_null'(value) {
      return Module['_BinaryenRefIs'](module, Module['RefIsNull'], value);
    },
    'is_func'(value) {
      return Module['_BinaryenRefIs'](module, Module['RefIsFunc'], value);
    },
    'is_data'(value) {
      return Module['_BinaryenRefIs'](module, Module['RefIsData'], value);
    },
    'is_i31'(value) {
      return Module['_BinaryenRefIs'](module, Module['RefIsI31'], value);
    },
    'as_non_null'(value) {
      return Module['_BinaryenRefAs'](module, Module['RefAsNonNull'], value);
    },
    'as_func'(value) {
      return Module['_BinaryenRefAs'](module, Module['RefAsFunc'], value);
    },
    'as_data'(value) {
      return Module['_BinaryenRefAs'](module, Module['RefAsData'], value);
    },
    'as_i31'(value) {
      return Module['_BinaryenRefAs'](module, Module['RefAsI31'], value);
    },
    'func'(func, type) {
      return preserveStack(() => Module['_BinaryenRefFunc'](module, strToStack(func), type));
    },
    'eq'(left, right) {
      return Module['_BinaryenRefEq'](module, left, right);
    }
  };

  self['select'] = function(condition, ifTrue, ifFalse, type) {
    return Module['_BinaryenSelect'](module, condition, ifTrue, ifFalse, typeof type !== 'undefined' ? type : Module['auto']);
  };
  self['drop'] = function(value) {
    return Module['_BinaryenDrop'](module, value);
  };
  self['return'] = function(value) {
    return Module['_BinaryenReturn'](module, value);
  };
  self['nop'] = function() {
    return Module['_BinaryenNop'](module);
  };
  self['unreachable'] = function() {
    return Module['_BinaryenUnreachable'](module);
  };

  self['atomic'] = {
    'fence'() {
      return Module['_BinaryenAtomicFence'](module);
    }
  };

  self['try'] = function(name, body, catchEvents, catchBodies, delegateTarget) {
    return preserveStack(() =>
      Module['_BinaryenTry'](module, name ? strToStack(name) : 0, body, i32sToStack(catchEvents.map(strToStack)), catchEvents.length, i32sToStack(catchBodies), catchBodies.length, delegateTarget ? strToStack(delegateTarget) : 0));
  };
  self['throw'] = function(event_, operands) {
    return preserveStack(() => Module['_BinaryenThrow'](module, strToStack(event_), i32sToStack(operands), operands.length));
  };
  self['rethrow'] = function(target) {
    return Module['_BinaryenRethrow'](module, strToStack(target));
  };

  self['tuple'] = {
    'make'(elements) {
      return preserveStack(() => Module['_BinaryenTupleMake'](module, i32sToStack(elements), elements.length));
    },
    'extract'(tuple, index) {
      return Module['_BinaryenTupleExtract'](module, tuple, index);
    }
  };

  self['i31'] = {
    'new'(value) {
      return Module['_BinaryenI31New'](module, value);
    },
    'get_s'(i31) {
      return Module['_BinaryenI31Get'](module, i31, 1);
    },
    'get_u'(i31) {
      return Module['_BinaryenI31Get'](module, i31, 0);
    }
  };

  // 'Module' operations
  self['addFunction'] = function(name, params, results, varTypes, body) {
    return preserveStack(() =>
      Module['_BinaryenAddFunction'](module, strToStack(name), params, results, i32sToStack(varTypes), varTypes.length, body)
    );
  };
  self['getFunction'] = function(name) {
    return preserveStack(() => Module['_BinaryenGetFunction'](module, strToStack(name)));
  };
  self['removeFunction'] = function(name) {
    return preserveStack(() => Module['_BinaryenRemoveFunction'](module, strToStack(name)));
  };
  self['addGlobal'] = function(name, type, mutable, init) {
    return preserveStack(() => Module['_BinaryenAddGlobal'](module, strToStack(name), type, mutable, init));
  }
  self['getGlobal'] = function(name) {
    return preserveStack(() => Module['_BinaryenGetGlobal'](module, strToStack(name)));
  };
  self['addTable'] = function(table, initial, maximum) {
    return preserveStack(() => Module['_BinaryenAddTable'](module, strToStack(table), initial, maximum));
  }
  self['getTable'] = function(name) {
    return preserveStack(() => Module['_BinaryenGetTable'](module, strToStack(name)));
  };
  self['addActiveElementSegment'] = function(table, name, funcNames, offset = self['i32']['const'](0)) {
    return preserveStack(() => Module['_BinaryenAddActiveElementSegment'](
      module,
      strToStack(table),
      strToStack(name),
      i32sToStack(funcNames.map(strToStack)),
      funcNames.length,
      offset
    ));
  };
  self['addPassiveElementSegment'] = function(name, funcNames) {
    return preserveStack(() => Module['_BinaryenAddPassiveElementSegment'](
      module,
      strToStack(name),
      i32sToStack(funcNames.map(strToStack)),
      funcNames.length
    ));
  };
  self['getElementSegment'] = function(name) {
    return preserveStack(() => Module['_BinaryenGetElementSegment'](module, strToStack(name)));
  };
  self['getTableSegments'] = function(table) {
    var numElementSegments = Module['_BinaryenGetNumElementSegments'](module);
    var tableName = UTF8ToString(Module['_BinaryenTableGetName'](table));
    var ret = [];
    for (var i = 0; i < numElementSegments; i++) {
      var segment = Module['_BinaryenGetElementSegmentByIndex'](module, i);
      var elemTableName = UTF8ToString(Module['_BinaryenElementSegmentGetTable'](segment));
      if (tableName === elemTableName) {
        ret.push(segment);
      }
    }
    return ret;
  }
  self['removeGlobal'] = function(name) {
    return preserveStack(() => Module['_BinaryenRemoveGlobal'](module, strToStack(name)));
  }
  self['removeTable'] = function(name) {
    return preserveStack(() => Module['_BinaryenRemoveTable'](module, strToStack(name)));
  };
  self['removeElementSegment'] = function(name) {
    return preserveStack(() => Module['_BinaryenRemoveElementSegment'](module, strToStack(name)));
  };
  self['addEvent'] = function(name, attribute, params, results) {
    return preserveStack(() => Module['_BinaryenAddEvent'](module, strToStack(name), attribute, params, results));
  };
  self['getEvent'] = function(name) {
    return preserveStack(() => Module['_BinaryenGetEvent'](module, strToStack(name)));
  };
  self['removeEvent'] = function(name) {
    return preserveStack(() => Module['_BinaryenRemoveEvent'](module, strToStack(name)));
  };
  self['addFunctionImport'] = function(internalName, externalModuleName, externalBaseName, params, results) {
    return preserveStack(() =>
      Module['_BinaryenAddFunctionImport'](module, strToStack(internalName), strToStack(externalModuleName), strToStack(externalBaseName), params, results)
    );
  };
  self['addTableImport'] = function(internalName, externalModuleName, externalBaseName) {
    return preserveStack(() =>
      Module['_BinaryenAddTableImport'](module, strToStack(internalName), strToStack(externalModuleName), strToStack(externalBaseName))
    );
  };
  self['addMemoryImport'] = function(internalName, externalModuleName, externalBaseName, shared) {
    return preserveStack(() =>
      Module['_BinaryenAddMemoryImport'](module, strToStack(internalName), strToStack(externalModuleName), strToStack(externalBaseName), shared)
    );
  };
  self['addGlobalImport'] = function(internalName, externalModuleName, externalBaseName, globalType, mutable) {
    return preserveStack(() =>
      Module['_BinaryenAddGlobalImport'](module, strToStack(internalName), strToStack(externalModuleName), strToStack(externalBaseName), globalType, mutable)
    );
  };
  self['addEventImport'] = function(internalName, externalModuleName, externalBaseName, attribute, params, results) {
    return preserveStack(() =>
      Module['_BinaryenAddEventImport'](module, strToStack(internalName), strToStack(externalModuleName), strToStack(externalBaseName), attribute, params, results)
    );
  };
  self['addExport'] = // deprecated
  self['addFunctionExport'] = function(internalName, externalName) {
    return preserveStack(() => Module['_BinaryenAddFunctionExport'](module, strToStack(internalName), strToStack(externalName)));
  };
  self['addTableExport'] = function(internalName, externalName) {
    return preserveStack(() => Module['_BinaryenAddTableExport'](module, strToStack(internalName), strToStack(externalName)));
  };
  self['addMemoryExport'] = function(internalName, externalName) {
    return preserveStack(() => Module['_BinaryenAddMemoryExport'](module, strToStack(internalName), strToStack(externalName)));
  };
  self['addGlobalExport'] = function(internalName, externalName) {
    return preserveStack(() => Module['_BinaryenAddGlobalExport'](module, strToStack(internalName), strToStack(externalName)));
  };
  self['addEventExport'] = function(internalName, externalName) {
    return preserveStack(() => Module['_BinaryenAddEventExport'](module, strToStack(internalName), strToStack(externalName)));
  };
  self['removeExport'] = function(externalName) {
    return preserveStack(() => Module['_BinaryenRemoveExport'](module, strToStack(externalName)));
  };
  self['setMemory'] = function(initial, maximum, exportName, segments = [], shared = false) {
    // segments are assumed to be { passive: bool, offset: expression ref, data: array of 8-bit data }
    return preserveStack(() => {
      const segmentsLen = segments.length;
      const segmentData = new Array(segmentsLen);
      const segmentDataLen = new Array(segmentsLen);
      const segmentPassive = new Array(segmentsLen);
      const segmentOffset = new Array(segmentsLen);
      for (let i = 0; i < segmentsLen; i++) {
        const { data, offset, passive } = segments[i];
        segmentData[i] = allocate(data, ALLOC_STACK);
        segmentDataLen[i] = data.length;
        segmentPassive[i] = passive;
        segmentOffset[i] = offset;
      }
      return Module['_BinaryenSetMemory'](
        module, initial, maximum, strToStack(exportName),
        i32sToStack(segmentData),
        i8sToStack(segmentPassive),
        i32sToStack(segmentOffset),
        i32sToStack(segmentDataLen),
        segmentsLen,
        shared
      );
    });
  };
  self['getNumMemorySegments'] = function() {
    return Module['_BinaryenGetNumMemorySegments'](module);
  }
  self['getMemorySegmentInfoByIndex'] = function(id) {
    return {
      'offset': Module['_BinaryenGetMemorySegmentByteOffset'](module, id),
      'data': (function(){
        const size = Module['_BinaryenGetMemorySegmentByteLength'](module, id);
        const ptr = _malloc(size);
        Module['_BinaryenCopyMemorySegmentData'](module, id, ptr);
        const res = new Uint8Array(size);
        res.set(new Uint8Array(buffer, ptr, size));
        _free(ptr);
        return res.buffer;
      })(),
      'passive': Boolean(Module['_BinaryenGetMemorySegmentPassive'](module, id))
    };
  }
  self['setStart'] = function(start) {
    return Module['_BinaryenSetStart'](module, start);
  };
  self['getFeatures'] = function() {
    return Module['_BinaryenModuleGetFeatures'](module);
  };
  self['setFeatures'] = function(features) {
    Module['_BinaryenModuleSetFeatures'](module, features);
  };
  self['addCustomSection'] = function(name, contents) {
    return preserveStack(() =>
      Module['_BinaryenAddCustomSection'](module, strToStack(name), i8sToStack(contents), contents.length)
    );
  };
  self['getExport'] = function(externalName) {
    return preserveStack(() => Module['_BinaryenGetExport'](module, strToStack(externalName)));
  };
  self['getNumExports'] = function() {
    return Module['_BinaryenGetNumExports'](module);
  };
  self['getExportByIndex'] = function(index) {
    return Module['_BinaryenGetExportByIndex'](module, index);
  };
  self['getNumFunctions'] = function() {
    return Module['_BinaryenGetNumFunctions'](module);
  };
  self['getFunctionByIndex'] = function(index) {
    return Module['_BinaryenGetFunctionByIndex'](module, index);
  };
  self['getNumGlobals'] = function() {
    return Module['_BinaryenGetNumGlobals'](module);
  };
  self['getNumTables'] = function() {
    return Module['_BinaryenGetNumTables'](module);
  };
  self['getNumElementSegments'] = function() {
    return Module['_BinaryenGetNumElementSegments'](module);
  };
  self['getGlobalByIndex'] = function(index) {
    return Module['_BinaryenGetGlobalByIndex'](module, index);
  };
  self['getTableByIndex'] = function(index) {
    return Module['_BinaryenGetTableByIndex'](module, index);
  };
  self['getElementSegmentByIndex'] = function(index) {
    return Module['_BinaryenGetElementSegmentByIndex'](module, index);
  };
  self['emitText'] = function() {
    const old = out;
    let ret = '';
    out = x => { ret += x + '\n' };
    Module['_BinaryenModulePrint'](module);
    out = old;
    return ret;
  };
  self['emitStackIR'] = function(optimize) {
    self['runPasses'](['generate-stack-ir']);
    if (optimize) self['runPasses'](['optimize-stack-ir']);
    const old = out;
    let ret = '';
    out = x => { ret += x + '\n' };
    self['runPasses'](['print-stack-ir']);
    out = old;
    return ret;
  };
  self['emitAsmjs'] = function() {
    const old = out;
    let ret = '';
    out = x => { ret += x + '\n' };
    Module['_BinaryenModulePrintAsmjs'](module);
    out = old;
    return ret;
  };
  self['validate'] = function() {
    return Module['_BinaryenModuleValidate'](module);
  };
  self['optimize'] = function() {
    return Module['_BinaryenModuleOptimize'](module);
  };
  self['optimizeFunction'] = function(func) {
    if (typeof func === 'string') func = self['getFunction'](func);
    return Module['_BinaryenFunctionOptimize'](func, module);
  };
  self['runPasses'] = function(passes) {
    return preserveStack(() =>
      Module['_BinaryenModuleRunPasses'](module, i32sToStack(passes.map(strToStack)), passes.length)
    );
  };
  self['runPassesOnFunction'] = function(func, passes) {
    if (typeof func === 'string') func = self['getFunction'](func);
    return preserveStack(() =>
      Module['_BinaryenFunctionRunPasses'](func, module, i32sToStack(passes.map(strToStack)), passes.length)
    );
  };
  self['autoDrop'] = function() {
    return Module['_BinaryenModuleAutoDrop'](module);
  };
  self['dispose'] = function() {
    Module['_BinaryenModuleDispose'](module);
  };
  self['emitBinary'] = function(sourceMapUrl) {
    return preserveStack(() => {
      const tempBuffer = stackAlloc(_BinaryenSizeofAllocateAndWriteResult());
      Module['_BinaryenModuleAllocateAndWrite'](tempBuffer, module, strToStack(sourceMapUrl));
      const binaryPtr    = HEAPU32[ tempBuffer >>> 2     ];
      const binaryBytes  = HEAPU32[(tempBuffer >>> 2) + 1];
      const sourceMapPtr = HEAPU32[(tempBuffer >>> 2) + 2];
      try {
        const buffer = new Uint8Array(binaryBytes);
        buffer.set(HEAPU8.subarray(binaryPtr, binaryPtr + binaryBytes));
        return typeof sourceMapUrl === 'undefined'
          ? buffer
          : { 'binary': buffer, 'sourceMap': UTF8ToString(sourceMapPtr) };
      } finally {
        _free(binaryPtr);
        if (sourceMapPtr) _free(sourceMapPtr);
      }
    });
  };
  self['interpret'] = function() {
    return Module['_BinaryenModuleInterpret'](module);
  };
  self['addDebugInfoFileName'] = function(filename) {
    return preserveStack(() => Module['_BinaryenModuleAddDebugInfoFileName'](module, strToStack(filename)));
  };
  self['getDebugInfoFileName'] = function(index) {
    return UTF8ToString(Module['_BinaryenModuleGetDebugInfoFileName'](module, index));
  };
  self['setDebugLocation'] = function(func, expr, fileIndex, lineNumber, columnNumber) {
    return Module['_BinaryenFunctionSetDebugLocation'](func, expr, fileIndex, lineNumber, columnNumber);
  };
  self['copyExpression'] = function(expr) {
    return Module['_BinaryenExpressionCopy'](expr, module);
  };

  return self;
}
Module['wrapModule'] = wrapModule;

// 'Relooper' interface
Module['Relooper'] = function(module) {
  assert(module && typeof module === 'object' && module['ptr'] && module['block'] && module['if']); // guard against incorrect old API usage
  const relooper = Module['_RelooperCreate'](module['ptr']);
  this['ptr'] = relooper;

  this['addBlock'] = function(code) {
    return Module['_RelooperAddBlock'](relooper, code);
  };
  this['addBranch'] = function(from, to, condition, code) {
    return Module['_RelooperAddBranch'](from, to, condition, code);
  };
  this['addBlockWithSwitch'] = function(code, condition) {
    return Module['_RelooperAddBlockWithSwitch'](relooper, code, condition);
  };
  this['addBranchForSwitch'] = function(from, to, indexes, code) {
    return preserveStack(() => Module['_RelooperAddBranchForSwitch'](from, to, i32sToStack(indexes), indexes.length, code));
  };
  this['renderAndDispose'] = function(entry, labelHelper) {
    return Module['_RelooperRenderAndDispose'](relooper, entry, labelHelper);
  };
};

// 'ExpressionRunner' interface
Module['ExpressionRunner'] = function(module, flags, maxDepth, maxLoopIterations) {
  const runner = Module['_ExpressionRunnerCreate'](module['ptr'], flags, maxDepth, maxLoopIterations);
  this['ptr'] = runner;

  this['setLocalValue'] = function(index, valueExpr) {
    return Boolean(Module['_ExpressionRunnerSetLocalValue'](runner, index, valueExpr));
  };
  this['setGlobalValue'] = function(name, valueExpr) {
    return preserveStack(() => Boolean(Module['_ExpressionRunnerSetGlobalValue'](runner, strToStack(name), valueExpr)));
  };
  this['runAndDispose'] = function(expr) {
    return Module['_ExpressionRunnerRunAndDispose'](runner, expr);
  };
};

function getAllNested(ref, numFn, getFn) {
  const num = numFn(ref);
  const ret = new Array(num);
  for (let i = 0; i < num; ++i) ret[i] = getFn(ref, i);
  return ret;
}

function setAllNested(ref, values, numFn, setFn, appendFn, removeFn) {
  const num = values.length;
  let prevNum = numFn(ref);
  let index = 0;
  while (index < num) {
    if (index < prevNum) {
      setFn(ref, index, values[index]);
    } else {
      appendFn(ref, values[index]);
    }
    ++index;
  }
  while (prevNum > index) {
    removeFn(ref, --prevNum);
  }
}

// Gets the specific id of an 'Expression'
Module['getExpressionId'] = function(expr) {
  return Module['_BinaryenExpressionGetId'](expr);
};

// Gets the result type of an 'Expression'
Module['getExpressionType'] = function(expr) {
  return Module['_BinaryenExpressionGetType'](expr);
};

// Obtains information about an 'Expression'
Module['getExpressionInfo'] = function(expr) {
  const id = Module['_BinaryenExpressionGetId'](expr);
  const type = Module['_BinaryenExpressionGetType'](expr);
  switch (id) {
    case Module['BlockId']:
      return {
        'id': id,
        'type': type,
        'name': UTF8ToString(Module['_BinaryenBlockGetName'](expr)),
        'children': getAllNested(expr, Module['_BinaryenBlockGetNumChildren'], Module['_BinaryenBlockGetChildAt'])
      };
    case Module['IfId']:
      return {
        'id': id,
        'type': type,
        'condition': Module['_BinaryenIfGetCondition'](expr),
        'ifTrue': Module['_BinaryenIfGetIfTrue'](expr),
        'ifFalse': Module['_BinaryenIfGetIfFalse'](expr)
      };
    case Module['LoopId']:
      return {
        'id': id,
        'type': type,
        'name': UTF8ToString(Module['_BinaryenLoopGetName'](expr)),
        'body': Module['_BinaryenLoopGetBody'](expr)
      };
    case Module['BreakId']:
      return {
        'id': id,
        'type': type,
        'name': UTF8ToString(Module['_BinaryenBreakGetName'](expr)),
        'condition': Module['_BinaryenBreakGetCondition'](expr),
        'value': Module['_BinaryenBreakGetValue'](expr)
      };
    case Module['SwitchId']:
      return {
        'id': id,
        'type': type,
         // Do not pass the index as the second parameter to UTF8ToString as that will cut off the string.
        'names': getAllNested(expr, Module['_BinaryenSwitchGetNumNames'], Module['_BinaryenSwitchGetNameAt']).map(p => UTF8ToString(p)),
        'defaultName': UTF8ToString(Module['_BinaryenSwitchGetDefaultName'](expr)),
        'condition': Module['_BinaryenSwitchGetCondition'](expr),
        'value': Module['_BinaryenSwitchGetValue'](expr)
      };
    case Module['CallId']:
      return {
        'id': id,
        'type': type,
        'isReturn': Boolean(Module['_BinaryenCallIsReturn'](expr)),
        'target': UTF8ToString(Module['_BinaryenCallGetTarget'](expr)),
        'operands': getAllNested(expr, Module[ '_BinaryenCallGetNumOperands'], Module['_BinaryenCallGetOperandAt'])
      };
    case Module['CallIndirectId']:
      return {
        'id': id,
        'type': type,
        'isReturn': Boolean(Module['_BinaryenCallIndirectIsReturn'](expr)),
        'target': Module['_BinaryenCallIndirectGetTarget'](expr),
        'table': Module['_BinaryenCallIndirectGetTable'](expr),
        'operands': getAllNested(expr, Module['_BinaryenCallIndirectGetNumOperands'], Module['_BinaryenCallIndirectGetOperandAt'])
      };
    case Module['LocalGetId']:
      return {
        'id': id,
        'type': type,
        'index': Module['_BinaryenLocalGetGetIndex'](expr)
      };
    case Module['LocalSetId']:
      return {
        'id': id,
        'type': type,
        'isTee': Boolean(Module['_BinaryenLocalSetIsTee'](expr)),
        'index': Module['_BinaryenLocalSetGetIndex'](expr),
        'value': Module['_BinaryenLocalSetGetValue'](expr)
      };
    case Module['GlobalGetId']:
      return {
        'id': id,
        'type': type,
        'name': UTF8ToString(Module['_BinaryenGlobalGetGetName'](expr))
      };
    case Module['GlobalSetId']:
      return {
        'id': id,
        'type': type,
        'name': UTF8ToString(Module['_BinaryenGlobalSetGetName'](expr)),
        'value': Module['_BinaryenGlobalSetGetValue'](expr)
      };
    case Module['LoadId']:
      return {
        'id': id,
        'type': type,
        'isAtomic': Boolean(Module['_BinaryenLoadIsAtomic'](expr)),
        'isSigned': Boolean(Module['_BinaryenLoadIsSigned'](expr)),
        'offset': Module['_BinaryenLoadGetOffset'](expr),
        'bytes': Module['_BinaryenLoadGetBytes'](expr),
        'align': Module['_BinaryenLoadGetAlign'](expr),
        'ptr': Module['_BinaryenLoadGetPtr'](expr)
      };
    case Module['StoreId']:
      return {
        'id': id,
        'type': type,
        'isAtomic': Boolean(Module['_BinaryenStoreIsAtomic'](expr)),
        'offset': Module['_BinaryenStoreGetOffset'](expr),
        'bytes': Module['_BinaryenStoreGetBytes'](expr),
        'align': Module['_BinaryenStoreGetAlign'](expr),
        'ptr': Module['_BinaryenStoreGetPtr'](expr),
        'value': Module['_BinaryenStoreGetValue'](expr)
      };
    case Module['ConstId']: {
      let value;
      switch (type) {
        case Module['i32']: value = Module['_BinaryenConstGetValueI32'](expr); break;
        case Module['i64']: value = {
          'low':  Module['_BinaryenConstGetValueI64Low'](expr),
          'high': Module['_BinaryenConstGetValueI64High'](expr)
        }; break;
        case Module['f32']: value = Module['_BinaryenConstGetValueF32'](expr); break;
        case Module['f64']: value = Module['_BinaryenConstGetValueF64'](expr); break;
        case Module['v128']: {
          preserveStack(() => {
            const tempBuffer = stackAlloc(16);
            Module['_BinaryenConstGetValueV128'](expr, tempBuffer);
            value = new Array(16);
            for (let i = 0; i < 16; i++) {
              value[i] = HEAPU8[tempBuffer + i];
            }
          });
          break;
        }
        default: throw Error('unexpected type: ' + type);
      }
      return {
        'id': id,
        'type': type,
        'value': value
      };
    }
    case Module['UnaryId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenUnaryGetOp'](expr),
        'value': Module['_BinaryenUnaryGetValue'](expr)
      };
    case Module['BinaryId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenBinaryGetOp'](expr),
        'left': Module['_BinaryenBinaryGetLeft'](expr),
        'right':  Module['_BinaryenBinaryGetRight'](expr)
      };
    case Module['SelectId']:
      return {
        'id': id,
        'type': type,
        'ifTrue': Module['_BinaryenSelectGetIfTrue'](expr),
        'ifFalse': Module['_BinaryenSelectGetIfFalse'](expr),
        'condition': Module['_BinaryenSelectGetCondition'](expr)
      };
    case Module['DropId']:
      return {
        'id': id,
        'type': type,
        'value': Module['_BinaryenDropGetValue'](expr)
      };
    case Module['ReturnId']:
      return {
        'id': id,
        'type': type,
        'value': Module['_BinaryenReturnGetValue'](expr)
      };
    case Module['NopId']:
    case Module['UnreachableId']:
    case Module['PopId']:
      return {
        'id': id,
        'type': type
      };
    case Module['MemorySizeId']:
      return {
        'id': id,
        'type': type
      };
    case Module['MemoryGrowId']:
      return {
        'id': id,
        'type': type,
        'delta': Module['_BinaryenMemoryGrowGetDelta'](expr)
      }
    case Module['AtomicRMWId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenAtomicRMWGetOp'](expr),
        'bytes': Module['_BinaryenAtomicRMWGetBytes'](expr),
        'offset': Module['_BinaryenAtomicRMWGetOffset'](expr),
        'ptr': Module['_BinaryenAtomicRMWGetPtr'](expr),
        'value': Module['_BinaryenAtomicRMWGetValue'](expr)
      };
    case Module['AtomicCmpxchgId']:
      return {
        'id': id,
        'type': type,
        'bytes': Module['_BinaryenAtomicCmpxchgGetBytes'](expr),
        'offset': Module['_BinaryenAtomicCmpxchgGetOffset'](expr),
        'ptr': Module['_BinaryenAtomicCmpxchgGetPtr'](expr),
        'expected': Module['_BinaryenAtomicCmpxchgGetExpected'](expr),
        'replacement': Module['_BinaryenAtomicCmpxchgGetReplacement'](expr)
      };
    case Module['AtomicWaitId']:
      return {
        'id': id,
        'type': type,
        'ptr': Module['_BinaryenAtomicWaitGetPtr'](expr),
        'expected': Module['_BinaryenAtomicWaitGetExpected'](expr),
        'timeout': Module['_BinaryenAtomicWaitGetTimeout'](expr),
        'expectedType': Module['_BinaryenAtomicWaitGetExpectedType'](expr)
      };
    case Module['AtomicNotifyId']:
      return {
        'id': id,
        'type': type,
        'ptr': Module['_BinaryenAtomicNotifyGetPtr'](expr),
        'notifyCount': Module['_BinaryenAtomicNotifyGetNotifyCount'](expr)
      };
    case Module['AtomicFenceId']:
      return {
        'id': id,
        'type': type,
        'order': Module['_BinaryenAtomicFenceGetOrder'](expr)
      };
    case Module['SIMDExtractId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenSIMDExtractGetOp'](expr),
        'vec': Module['_BinaryenSIMDExtractGetVec'](expr),
        'index': Module['_BinaryenSIMDExtractGetIndex'](expr)
      };
    case Module['SIMDReplaceId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenSIMDReplaceGetOp'](expr),
        'vec': Module['_BinaryenSIMDReplaceGetVec'](expr),
        'index': Module['_BinaryenSIMDReplaceGetIndex'](expr),
        'value': Module['_BinaryenSIMDReplaceGetValue'](expr)
      };
    case Module['SIMDShuffleId']:
      return preserveStack(() => {
        const tempBuffer = stackAlloc(16);
        Module['_BinaryenSIMDShuffleGetMask'](expr, tempBuffer);
        const mask = new Array(16);
        for (let i = 0; i < 16; i++) {
          mask[i] = HEAPU8[tempBuffer + i];
        }
        return {
          'id': id,
          'type': type,
          'left': Module['_BinaryenSIMDShuffleGetLeft'](expr),
          'right': Module['_BinaryenSIMDShuffleGetRight'](expr),
          'mask': mask
        };
      });
    case Module['SIMDTernaryId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenSIMDTernaryGetOp'](expr),
        'a': Module['_BinaryenSIMDTernaryGetA'](expr),
        'b': Module['_BinaryenSIMDTernaryGetB'](expr),
        'c': Module['_BinaryenSIMDTernaryGetC'](expr)
      };
    case Module['SIMDShiftId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenSIMDShiftGetOp'](expr),
        'vec': Module['_BinaryenSIMDShiftGetVec'](expr),
        'shift': Module['_BinaryenSIMDShiftGetShift'](expr)
      };
    case Module['SIMDLoadId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenSIMDLoadGetOp'](expr),
        'offset': Module['_BinaryenSIMDLoadGetOffset'](expr),
        'align': Module['_BinaryenSIMDLoadGetAlign'](expr),
        'ptr': Module['_BinaryenSIMDLoadGetPtr'](expr)
      };
    case Module['SIMDLoadStoreLaneId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenSIMDLoadStoreLaneGetOp'](expr),
        'offset': Module['_BinaryenSIMDLoadStoreLaneGetOffset'](expr),
        'align': Module['_BinaryenSIMDLoadStoreLaneGetAlign'](expr),
        'index': Module['_BinaryenSIMDLoadStoreLaneGetIndex'](expr),
        'ptr': Module['_BinaryenSIMDLoadStoreLaneGetPtr'](expr),
        'vec': Module['_BinaryenSIMDLoadStoreLaneGetVec'](expr)
      };
    case Module['MemoryInitId']:
      return {
        'id': id,
        'segment': Module['_BinaryenMemoryInitGetSegment'](expr),
        'dest': Module['_BinaryenMemoryInitGetDest'](expr),
        'offset': Module['_BinaryenMemoryInitGetOffset'](expr),
        'size': Module['_BinaryenMemoryInitGetSize'](expr)
      };
    case Module['DataDropId']:
      return {
        'id': id,
        'segment': Module['_BinaryenDataDropGetSegment'](expr),
      };
    case Module['MemoryCopyId']:
      return {
        'id': id,
        'dest': Module['_BinaryenMemoryCopyGetDest'](expr),
        'source': Module['_BinaryenMemoryCopyGetSource'](expr),
        'size': Module['_BinaryenMemoryCopyGetSize'](expr)
      };
    case Module['MemoryFillId']:
      return {
        'id': id,
        'dest': Module['_BinaryenMemoryFillGetDest'](expr),
        'value': Module['_BinaryenMemoryFillGetValue'](expr),
        'size': Module['_BinaryenMemoryFillGetSize'](expr)
      };
    case Module['RefNullId']:
      return {
        'id': id,
        'type': type
      };
    case Module['RefIsId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenRefIsGetOp'](expr),
        'value': Module['_BinaryenRefIsGetValue'](expr)
      };
    case Module['RefAsId']:
      return {
        'id': id,
        'type': type,
        'op': Module['_BinaryenRefAsGetOp'](expr),
        'value': Module['_BinaryenRefAsGetValue'](expr)
      };
    case Module['RefFuncId']:
      return {
        'id': id,
        'type': type,
        'func': UTF8ToString(Module['_BinaryenRefFuncGetFunc'](expr)),
      };
    case Module['RefEqId']:
      return {
        'id': id,
        'type': type,
        'left': Module['_BinaryenRefEqGetLeft'](expr),
        'right': Module['_BinaryenRefEqGetRight'](expr)
      };
    case Module['TryId']:
      return {
        'id': id,
        'type': type,
        'name': UTF8ToString(Module['_BinaryenTryGetName'](expr)),
        'body': Module['_BinaryenTryGetBody'](expr),
        'catchEvents': getAllNested(expr, Module['_BinaryenTryGetNumCatchEvents'], Module['_BinaryenTryGetCatchEventAt']),
        'catchBodies': getAllNested(expr, Module['_BinaryenTryGetNumCatchBodies'], Module['_BinaryenTryGetCatchBodyAt']),
        'hasCatchAll': Module['_BinaryenTryHasCatchAll'](expr),
        'delegateTarget': UTF8ToString(Module['_BinaryenTryGetDelegateTarget'](expr)),
        'isDelegate': Module['_BinaryenTryIsDelegate'](expr)
      };
    case Module['ThrowId']:
      return {
        'id': id,
        'type': type,
        'event': UTF8ToString(Module['_BinaryenThrowGetEvent'](expr)),
        'operands': getAllNested(expr, Module['_BinaryenThrowGetNumOperands'], Module['_BinaryenThrowGetOperandAt'])
      };
    case Module['RethrowId']:
      return {
        'id': id,
        'type': type,
        'target': UTF8ToString(Module['_BinaryenRethrowGetTarget'](expr))
      };
    case Module['TupleMakeId']:
      return {
        'id': id,
        'type': type,
        'operands': getAllNested(expr, Module['_BinaryenTupleMakeGetNumOperands'], Module['_BinaryenTupleMakeGetOperandAt'])
      };
    case Module['TupleExtractId']:
      return {
        'id': id,
        'type': type,
        'tuple': Module['_BinaryenTupleExtractGetTuple'](expr),
        'index': Module['_BinaryenTupleExtractGetIndex'](expr)
      };
    case Module['I31NewId']:
      return {
        'id': id,
        'type': type,
        'value': Module['_BinaryenI31NewGetValue'](expr)
      };
    case Module['I31GetId']:
      return {
        'id': id,
        'type': type,
        'i31': Module['_BinaryenI31GetGetI31'](expr),
        'isSigned': Boolean(Module['_BinaryenI31GetIsSigned'](expr))
      };

    default:
      throw Error('unexpected id: ' + id);
  }
};

// Gets the side effects of the specified expression
Module['getSideEffects'] = function(expr, features) {
  return Module['_BinaryenExpressionGetSideEffects'](expr, features);
};

Module['createType'] = function(types) {
  return preserveStack(() => Module['_BinaryenTypeCreate'](i32sToStack(types), types.length));
};

Module['expandType'] = function(ty) {
  return preserveStack(() => {
    const numTypes = Module['_BinaryenTypeArity'](ty);
    const array = stackAlloc(numTypes << 2);
    Module['_BinaryenTypeExpand'](ty, array);
    const types = new Array(numTypes);
    for (let i = 0; i < numTypes; i++) {
      types[i] = HEAPU32[(array >>> 2) + i];
    }
    return types;
  });
};

// Obtains information about a 'Function'
Module['getFunctionInfo'] = function(func) {
  return {
    'name': UTF8ToString(Module['_BinaryenFunctionGetName'](func)),
    'module': UTF8ToString(Module['_BinaryenFunctionImportGetModule'](func)),
    'base': UTF8ToString(Module['_BinaryenFunctionImportGetBase'](func)),
    'params': Module['_BinaryenFunctionGetParams'](func),
    'results': Module['_BinaryenFunctionGetResults'](func),
    'vars': getAllNested(func, Module['_BinaryenFunctionGetNumVars'], Module['_BinaryenFunctionGetVar']),
    'body': Module['_BinaryenFunctionGetBody'](func)
  };
};

// Obtains information about a 'Global'
Module['getGlobalInfo'] = function(global) {
  return {
    'name': UTF8ToString(Module['_BinaryenGlobalGetName'](global)),
    'module': UTF8ToString(Module['_BinaryenGlobalImportGetModule'](global)),
    'base': UTF8ToString(Module['_BinaryenGlobalImportGetBase'](global)),
    'type': Module['_BinaryenGlobalGetType'](global),
    'mutable': Boolean(Module['_BinaryenGlobalIsMutable'](global)),
    'init': Module['_BinaryenGlobalGetInitExpr'](global)
  };
};

// Obtains information about a 'Table'
Module['getTableInfo'] = function(table) {
  var hasMax = Boolean(Module['_BinaryenTableHasMax'](table));
  var tableInfo = {
    'name': UTF8ToString(Module['_BinaryenTableGetName'](table)),
    'module': UTF8ToString(Module['_BinaryenTableImportGetModule'](table)),
    'base': UTF8ToString(Module['_BinaryenTableImportGetBase'](table)),
    'initial': Module['_BinaryenTableGetInitial'](table),
  }

  if (hasMax) {
    tableInfo.max = Module['_BinaryenTableGetMax'](table);
  }

  return tableInfo;
};

Module['getElementSegmentInfo'] = function(segment) {
  var segmentLength = Module['_BinaryenElementSegmentGetLength'](segment);
  var names = new Array(segmentLength);
  for (let j = 0; j !== segmentLength; ++j) {
    var ptr = Module['_BinaryenElementSegmentGetData'](segment, j);
    names[j] = UTF8ToString(ptr);
  }

  return {
    'name': UTF8ToString(Module['_BinaryenElementSegmentGetName'](segment)),
    'table': UTF8ToString(Module['_BinaryenElementSegmentGetTable'](segment)),
    'offset': Module['_BinaryenElementSegmentGetOffset'](segment),
    'data': names
  }
}

// Obtains information about a 'Event'
Module['getEventInfo'] = function(event_) {
  return {
    'name': UTF8ToString(Module['_BinaryenEventGetName'](event_)),
    'module': UTF8ToString(Module['_BinaryenEventImportGetModule'](event_)),
    'base': UTF8ToString(Module['_BinaryenEventImportGetBase'](event_)),
    'attribute': Module['_BinaryenEventGetAttribute'](event_),
    'params': Module['_BinaryenEventGetParams'](event_),
    'results': Module['_BinaryenEventGetResults'](event_)
  };
};

// Obtains information about an 'Export'
Module['getExportInfo'] = function(export_) {
  return {
    'kind': Module['_BinaryenExportGetKind'](export_),
    'name': UTF8ToString(Module['_BinaryenExportGetName'](export_)),
    'value': UTF8ToString(Module['_BinaryenExportGetValue'](export_))
  };
};

// Emits text format of an expression or a module
Module['emitText'] = function(expr) {
  if (typeof expr === 'object') {
    return expr.emitText();
  }
  const old = out;
  let ret = '';
  out = x => { ret += x + '\n' };
  Module['_BinaryenExpressionPrint'](expr);
  out = old;
  return ret;
};

// Parses a binary to a module

// If building with Emscripten ASSERTIONS, there is a property added to
// Module to guard against users mistakening using the removed readBinary()
// API. We must defuse that carefully.
Object.defineProperty(Module, 'readBinary', { writable: true });

Module['readBinary'] = function(data) {
  const buffer = allocate(data, ALLOC_NORMAL);
  const ptr = Module['_BinaryenModuleRead'](buffer, data.length);
  _free(buffer);
  return wrapModule(ptr);
};

// Parses text format to a module
Module['parseText'] = function(text) {
  const buffer = _malloc(text.length + 1);
  writeAsciiToMemory(text, buffer);
  const ptr = Module['_BinaryenModuleParse'](buffer);
  _free(buffer);
  return wrapModule(ptr);
};

// Gets the currently set optimize level. 0, 1, 2 correspond to -O0, -O1, -O2, etc.
Module['getOptimizeLevel'] = function() {
  return Module['_BinaryenGetOptimizeLevel']();
};

// Sets the optimization level to use. 0, 1, 2 correspond to -O0, -O1, -O2, etc.
Module['setOptimizeLevel'] = function(level) {
  Module['_BinaryenSetOptimizeLevel'](level);
};

// Gets the currently set shrink level. 0, 1, 2 correspond to -O0, -Os, -Oz.
Module['getShrinkLevel'] = function() {
  return Module['_BinaryenGetShrinkLevel']();
};

// Sets the shrink level to use. 0, 1, 2 correspond to -O0, -Os, -Oz.
Module['setShrinkLevel'] = function(level) {
  Module['_BinaryenSetShrinkLevel'](level);
};

// Gets whether generating debug information is currently enabled or not.
Module['getDebugInfo'] = function() {
  return Boolean(Module['_BinaryenGetDebugInfo']());
};

// Enables or disables debug information in emitted binaries.
Module['setDebugInfo'] = function(on) {
  Module['_BinaryenSetDebugInfo'](on);
};

// Gets whether the low 1K of memory can be considered unused when optimizing.
Module['getLowMemoryUnused'] = function() {
  return Boolean(Module['_BinaryenGetLowMemoryUnused']());
};

// Enables or disables whether the low 1K of memory can be considered unused
// when optimizing.
Module['setLowMemoryUnused'] = function(on) {
  Module['_BinaryenSetLowMemoryUnused'](on);
};

// Gets whether fast math optimizations are enabled, ignoring for example
// corner cases of floating-point math like NaN changes.
Module['getFastMath'] = function() {
  return Boolean(Module['_BinaryenGetFastMath']());
};

// Enables or disables fast math optimizations, ignoring for example
// corner cases of floating-point math like NaN changes.
Module['setFastMath'] = function(value) {
  Module['_BinaryenSetFastMath'](value);
};

// Gets the value of the specified arbitrary pass argument.
Module['getPassArgument'] = function(key) {
  return preserveStack(() => {
    const ret = Module['_BinaryenGetPassArgument'](strToStack(key));
    return ret !== 0 ? UTF8ToString(ret) : null;
  });
};

// Sets the value of the specified arbitrary pass argument. Removes the
// respective argument if `value` is NULL.
Module['setPassArgument'] = function (key, value) {
  preserveStack(() => { Module['_BinaryenSetPassArgument'](strToStack(key), strToStack(value)) });
};

// Clears all arbitrary pass arguments.
Module['clearPassArguments'] = function() {
  Module['_BinaryenClearPassArguments']();
};

// Gets the function size at which we always inline.
Module['getAlwaysInlineMaxSize'] = function() {
  return Module['_BinaryenGetAlwaysInlineMaxSize']();
};

// Sets the function size at which we always inline.
Module['setAlwaysInlineMaxSize'] = function(size) {
  Module['_BinaryenSetAlwaysInlineMaxSize'](size);
};

// Gets the function size which we inline when functions are lightweight.
Module['getFlexibleInlineMaxSize'] = function() {
  return Module['_BinaryenGetFlexibleInlineMaxSize']();
};

// Sets the function size which we inline when functions are lightweight.
Module['setFlexibleInlineMaxSize'] = function(size) {
  Module['_BinaryenSetFlexibleInlineMaxSize'](size);
};

// Gets the function size which we inline when there is only one caller.
Module['getOneCallerInlineMaxSize'] = function() {
  return Module['_BinaryenGetOneCallerInlineMaxSize']();
};

// Sets the function size which we inline when there is only one caller.
Module['setOneCallerInlineMaxSize'] = function(size) {
  Module['_BinaryenSetOneCallerInlineMaxSize'](size);
};

// Gets the value which allow inline functions that are not "lightweight".
Module['getAllowInliningFunctionsWithLoops'] = function() {
  return Boolean(Module['_BinaryenGetAllowInliningFunctionsWithLoops']());
};

// Sets the value which allow inline functions that are not "lightweight".
Module['setAllowInliningFunctionsWithLoops'] = function(value) {
  Module['_BinaryenSetAllowInliningFunctionsWithLoops'](value);
};

// Expression wrappers

// Private symbol used to store the underlying C-API pointer of a wrapped object.
const thisPtr = Symbol();

// Makes a specific expression wrapper class with the specified static members
// while automatically deriving instance methods and accessors.
function makeExpressionWrapper(ownStaticMembers) {
  function SpecificExpression(expr) {
    // can call the constructor without `new`
    if (!(this instanceof SpecificExpression)) {
      if (!expr) return null;
      return new SpecificExpression(expr);
    }
    Expression.call(this, expr);
  }
  // inherit static members of Expression
  Object.assign(SpecificExpression, Expression);
  // add own static members
  Object.assign(SpecificExpression, ownStaticMembers);
  // inherit from Expression
  (SpecificExpression.prototype = Object.create(Expression.prototype)).constructor = SpecificExpression;
  // derive own instance members
  deriveWrapperInstanceMembers(SpecificExpression.prototype, ownStaticMembers);
  return SpecificExpression;
}

// Derives the instance members of a wrapper class from the given static
// members.
function deriveWrapperInstanceMembers(prototype, staticMembers) {
  // Given a static member `getName(ptr)` for example, an instance method
  // `getName()` and a `name` accessor with the `this` argument bound will be
  // derived and added to the wrapper's prototype. If a corresponding static
  // `setName(ptr)` is present, a setter for the `name` accessor will be added
  // as well.
  Object.keys(staticMembers).forEach(memberName => {
    const member = staticMembers[memberName];
    if (typeof member === "function") {
      // Instance method calls the respective static method with `this` bound.
      prototype[memberName] = function(...args) {
        return this.constructor[memberName](this[thisPtr], ...args);
      };
      // Instance accessors call the respective static methods. Accessors are
      // derived only if the respective underlying static method takes exactly
      // one argument, the `this` argument, e.g. `getChild(ptr, idx)` does not
      // trigger an accessor.
      let match;
      if (member.length === 1 && (match = memberName.match(/^(get|is)/))) {
        const index = match[1].length;
        const propertyName = memberName.charAt(index).toLowerCase() + memberName.substring(index + 1);
        const setterIfAny = staticMembers["set" + memberName.substring(index)];
        Object.defineProperty(prototype, propertyName, {
          get() {
            return member(this[thisPtr]);
          },
          set(value) {
            if (setterIfAny) setterIfAny(this[thisPtr], value);
            else throw Error("property '" + propertyName + "' has no setter");
          }
        });
      }
    }
  });
}

// Base class of all expression wrappers
function Expression(expr) {
  if (!expr) throw Error("expression reference must not be null");
  this[thisPtr] = expr;
}
Expression['getId'] = function(expr) {
  return Module['_BinaryenExpressionGetId'](expr);
};
Expression['getType'] = function(expr) {
  return Module['_BinaryenExpressionGetType'](expr);
};
Expression['setType'] = function(expr, type) {
  Module['_BinaryenExpressionSetType'](expr, type);
};
Expression['finalize'] = function(expr) {
  return Module['_BinaryenExpressionFinalize'](expr);
};
Expression['toText'] = function(expr) {
  return Module['emitText'](expr);
};
deriveWrapperInstanceMembers(Expression.prototype, Expression);
Expression.prototype['valueOf'] = function() {
  return this[thisPtr];
};

Module['Expression'] = Expression;

Module['Block'] = makeExpressionWrapper({
  'getName'(expr) {
    const name = Module['_BinaryenBlockGetName'](expr);
    return name ? UTF8ToString(name) : null;
  },
  'setName'(expr, name) {
    preserveStack(() => { Module['_BinaryenBlockSetName'](expr, strToStack(name)) });
  },
  'getNumChildren'(expr) {
    return Module['_BinaryenBlockGetNumChildren'](expr);
  },
  'getChildren'(expr) {
    return getAllNested(expr, Module['_BinaryenBlockGetNumChildren'], Module['_BinaryenBlockGetChildAt']);
  },
  'setChildren'(expr, children) {
    setAllNested(expr, children, Module['_BinaryenBlockGetNumChildren'], Module['_BinaryenBlockSetChildAt'], Module['_BinaryenBlockAppendChild'], Module['_BinaryenBlockRemoveChildAt']);
  },
  'getChildAt'(expr, index) {
    return Module['_BinaryenBlockGetChildAt'](expr, index);
  },
  'setChildAt'(expr, index, childExpr) {
    Module['_BinaryenBlockSetChildAt'](expr, index, childExpr);
  },
  'appendChild'(expr, childExpr) {
    return Module['_BinaryenBlockAppendChild'](expr, childExpr);
  },
  'insertChildAt'(expr, index, childExpr) {
    Module['_BinaryenBlockInsertChildAt'](expr, index, childExpr);
  },
  'removeChildAt'(expr, index) {
    return Module['_BinaryenBlockRemoveChildAt'](expr, index);
  }
});

Module['If'] = makeExpressionWrapper({
  'getCondition'(expr) {
    return Module['_BinaryenIfGetCondition'](expr);
  },
  'setCondition'(expr, condExpr) {
    Module['_BinaryenIfSetCondition'](expr, condExpr);
  },
  'getIfTrue'(expr) {
    return Module['_BinaryenIfGetIfTrue'](expr);
  },
  'setIfTrue'(expr, ifTrueExpr) {
    Module['_BinaryenIfSetIfTrue'](expr, ifTrueExpr);
  },
  'getIfFalse'(expr) {
    return Module['_BinaryenIfGetIfFalse'](expr);
  },
  'setIfFalse'(expr, ifFalseExpr) {
    Module['_BinaryenIfSetIfFalse'](expr, ifFalseExpr);
  }
});

Module['Loop'] = makeExpressionWrapper({
  'getName'(expr) {
    const name = Module['_BinaryenLoopGetName'](expr);
    return name ? UTF8ToString(name) : null;
  },
  'setName'(expr, name) {
    preserveStack(() => { Module['_BinaryenLoopSetName'](expr, strToStack(name)) });
  },
  'getBody'(expr) {
    return Module['_BinaryenLoopGetBody'](expr);
  },
  'setBody'(expr, bodyExpr) {
    Module['_BinaryenLoopSetBody'](expr, bodyExpr);
  }
});

Module['Break'] = makeExpressionWrapper({
  'getName'(expr) {
    const name = Module['_BinaryenBreakGetName'](expr);
    return name ? UTF8ToString(name) : null;
  },
  'setName'(expr, name) {
    preserveStack(() => { Module['_BinaryenBreakSetName'](expr, strToStack(name)) });
  },
  'getCondition'(expr) {
    return Module['_BinaryenBreakGetCondition'](expr);
  },
  'setCondition'(expr, condExpr) {
    Module['_BinaryenBreakSetCondition'](expr, condExpr);
  },
  'getValue'(expr) {
    return Module['_BinaryenBreakGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenBreakSetValue'](expr, valueExpr);
  }
});

Module['Switch'] = makeExpressionWrapper({
  'getNumNames'(expr) {
    return Module['_BinaryenSwitchGetNumNames'](expr);
  },
  'getNames'(expr) {
    return getAllNested(expr, Module['_BinaryenSwitchGetNumNames'], Module['_BinaryenSwitchGetNameAt']).map(p => UTF8ToString(p));
  },
  'setNames'(expr, names) {
    preserveStack(() => {
      setAllNested(expr, names.map(strToStack), Module['_BinaryenSwitchGetNumNames'], Module['_BinaryenSwitchSetNameAt'], Module['_BinaryenSwitchAppendName'], Module['_BinaryenSwitchRemoveNameAt']);
    });
  },
  'getDefaultName'(expr) {
    const name = Module['_BinaryenSwitchGetDefaultName'](expr);
    return name ? UTF8ToString(name) : null;
  },
  'setDefaultName'(expr, defaultName) {
    preserveStack(() => { Module['_BinaryenSwitchSetDefaultName'](expr, strToStack(defaultName)) });
  },
  'getCondition'(expr) {
    return Module['_BinaryenSwitchGetCondition'](expr);
  },
  'setCondition'(expr, condExpr) {
    Module['_BinaryenSwitchSetCondition'](expr, condExpr);
  },
  'getValue'(expr) {
    return Module['_BinaryenSwitchGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenSwitchSetValue'](expr, valueExpr);
  },
  'getNameAt'(expr, index) {
    return UTF8ToString(Module['_BinaryenSwitchGetNameAt'](expr, index));
  },
  'setNameAt'(expr, index, name) {
    preserveStack(() => { Module['_BinaryenSwitchSetNameAt'](expr, index, strToStack(name)) });
  },
  'appendName'(expr, name) {
    preserveStack(() => Module['_BinaryenSwitchAppendName'](expr, strToStack(name)));
  },
  'insertNameAt'(expr, index, name) {
    preserveStack(() => { Module['_BinaryenSwitchInsertNameAt'](expr, index, strToStack(name)) });
  },
  'removeNameAt'(expr, index) {
    return UTF8ToString(Module['_BinaryenSwitchRemoveNameAt'](expr, index));
  },
});

Module['Call'] = makeExpressionWrapper({
  'getTarget'(expr) {
    return UTF8ToString(Module['_BinaryenCallGetTarget'](expr));
  },
  'setTarget'(expr, targetName) {
    preserveStack(() => { Module['_BinaryenCallSetTarget'](expr, strToStack(targetName)) });
  },
  'getNumOperands'(expr) {
    return Module['_BinaryenCallGetNumOperands'](expr);
  },
  'getOperands'(expr) {
    return getAllNested(expr, Module['_BinaryenCallGetNumOperands'], Module['_BinaryenCallGetOperandAt']);
  },
  'setOperands'(expr, operands) {
    setAllNested(expr, operands, Module['_BinaryenCallGetNumOperands'], Module['_BinaryenCallSetOperandAt'], Module['_BinaryenCallAppendOperand'], Module['_BinaryenCallRemoveOperandAt']);
  },
  'getOperandAt'(expr, index) {
    return Module['_BinaryenCallGetOperandAt'](expr, index);
  },
  'setOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenCallSetOperandAt'](expr, index, operandExpr);
  },
  'appendOperand'(expr, operandExpr) {
    return Module['_BinaryenCallAppendOperand'](expr, operandExpr);
  },
  'insertOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenCallInsertOperandAt'](expr, index, operandExpr);
  },
  'removeOperandAt'(expr, index) {
    return Module['_BinaryenCallRemoveOperandAt'](expr, index);
  },
  'isReturn'(expr) {
    return Boolean(Module['_BinaryenCallIsReturn'](expr));
  },
  'setReturn'(expr, isReturn) {
    Module['_BinaryenCallSetReturn'](expr, isReturn);
  }
});

Module['CallIndirect'] = makeExpressionWrapper({
  'getTarget'(expr) {
    return Module['_BinaryenCallIndirectGetTarget'](expr);
  },
  'setTarget'(expr, targetExpr) {
    Module['_BinaryenCallIndirectSetTarget'](expr, targetExpr);
  },
  'getTable'(expr) {
    return UTF8ToString(Module['_BinaryenCallIndirectGetTable'](expr));
  },
  'setTable'(expr, table) {
    preserveStack(() => { Module['_BinaryenCallIndirectSetTable'](expr, strToStack(table)) });
  },
  'getNumOperands'(expr) {
    return Module['_BinaryenCallIndirectGetNumOperands'](expr);
  },
  'getOperands'(expr) {
    return getAllNested(expr, Module['_BinaryenCallIndirectGetNumOperands'], Module['_BinaryenCallIndirectGetOperandAt']);
  },
  'setOperands'(expr, operands) {
    setAllNested(expr, operands, Module['_BinaryenCallIndirectGetNumOperands'], Module['_BinaryenCallIndirectSetOperandAt'], Module['_BinaryenCallIndirectAppendOperand'], Module['_BinaryenCallIndirectRemoveOperandAt']);
  },
  'getOperandAt'(expr, index) {
    return Module['_BinaryenCallIndirectGetOperandAt'](expr, index);
  },
  'setOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenCallIndirectSetOperandAt'](expr, index, operandExpr);
  },
  'appendOperand'(expr, operandExpr) {
    return Module['_BinaryenCallIndirectAppendOperand'](expr, operandExpr);
  },
  'insertOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenCallIndirectInsertOperandAt'](expr, index, operandExpr);
  },
  'removeOperandAt'(expr, index) {
    return Module['_BinaryenCallIndirectRemoveOperandAt'](expr, index);
  },
  'isReturn'(expr) {
    return Boolean(Module['_BinaryenCallIndirectIsReturn'](expr));
  },
  'setReturn'(expr, isReturn) {
    Module['_BinaryenCallIndirectSetReturn'](expr, isReturn);
  },
  'getParams'(expr) {
    return Module['_BinaryenCallIndirectGetParams'](expr);
  },
  'setParams'(expr, params) {
    Module['_BinaryenCallIndirectSetParams'](expr, params);
  },
  'getResults'(expr) {
    return Module['_BinaryenCallIndirectGetResults'](expr);
  },
  'setResults'(expr, results) {
    Module['_BinaryenCallIndirectSetResults'](expr, results);
  }
});

Module['LocalGet'] = makeExpressionWrapper({
  'getIndex'(expr) {
    return Module['_BinaryenLocalGetGetIndex'](expr);
  },
  'setIndex'(expr, index) {
    Module['_BinaryenLocalGetSetIndex'](expr, index);
  }
});

Module['LocalSet'] = makeExpressionWrapper({
  'getIndex'(expr) {
    return Module['_BinaryenLocalSetGetIndex'](expr);
  },
  'setIndex'(expr, index) {
    Module['_BinaryenLocalSetSetIndex'](expr, index);
  },
  'isTee'(expr) {
    return Boolean(Module['_BinaryenLocalSetIsTee'](expr));
  },
  'getValue'(expr) {
    return Module['_BinaryenLocalSetGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenLocalSetSetValue'](expr, valueExpr);
  }
});

Module['GlobalGet'] = makeExpressionWrapper({
  'getName'(expr) {
    return UTF8ToString(Module['_BinaryenGlobalGetGetName'](expr));
  },
  'setName'(expr, name) {
    preserveStack(() => { Module['_BinaryenGlobalGetSetName'](expr, strToStack(name)) });
  }
});

Module['GlobalSet'] = makeExpressionWrapper({
  'getName'(expr) {
    return UTF8ToString(Module['_BinaryenGlobalSetGetName'](expr));
  },
  'setName'(expr, name) {
    preserveStack(() => { Module['_BinaryenGlobalSetSetName'](expr, strToStack(name)) });
  },
  'getValue'(expr) {
    return Module['_BinaryenGlobalSetGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenGlobalSetSetValue'](expr, valueExpr);
  }
});

Module['MemorySize'] = makeExpressionWrapper({});

Module['MemoryGrow'] = makeExpressionWrapper({
  'getDelta'(expr) {
    return Module['_BinaryenMemoryGrowGetDelta'](expr);
  },
  'setDelta'(expr, deltaExpr) {
    Module['_BinaryenMemoryGrowSetDelta'](expr, deltaExpr);
  }
});

Module['Load'] = makeExpressionWrapper({
  'isAtomic'(expr) {
    return Boolean(Module['_BinaryenLoadIsAtomic'](expr));
  },
  'setAtomic'(expr, isAtomic) {
    Module['_BinaryenLoadSetAtomic'](expr, isAtomic);
  },
  'isSigned'(expr) {
    return Boolean(Module['_BinaryenLoadIsSigned'](expr));
  },
  'setSigned'(expr, isSigned) {
    Module['_BinaryenLoadSetSigned'](expr, isSigned);
  },
  'getOffset'(expr) {
    return Module['_BinaryenLoadGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenLoadSetOffset'](expr, offset);
  },
  'getBytes'(expr) {
    return Module['_BinaryenLoadGetBytes'](expr);
  },
  'setBytes'(expr, bytes) {
    Module['_BinaryenLoadSetBytes'](expr, bytes);
  },
  'getAlign'(expr) {
    return Module['_BinaryenLoadGetAlign'](expr);
  },
  'setAlign'(expr, align) {
    Module['_BinaryenLoadSetAlign'](expr, align);
  },
  'getPtr'(expr) {
    return Module['_BinaryenLoadGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenLoadSetPtr'](expr, ptrExpr);
  }
});

Module['Store'] = makeExpressionWrapper({
  'isAtomic'(expr) {
    return Boolean(Module['_BinaryenStoreIsAtomic'](expr));
  },
  'setAtomic'(expr, isAtomic) {
    Module['_BinaryenStoreSetAtomic'](expr, isAtomic);
  },
  'getBytes'(expr) {
    return Module['_BinaryenStoreGetBytes'](expr);
  },
  'setBytes'(expr, bytes) {
    Module['_BinaryenStoreSetBytes'](expr, bytes);
  },
  'getOffset'(expr) {
    return Module['_BinaryenStoreGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenStoreSetOffset'](expr, offset);
  },
  'getAlign'(expr) {
    return Module['_BinaryenStoreGetAlign'](expr);
  },
  'setAlign'(expr, align) {
    Module['_BinaryenStoreSetAlign'](expr, align);
  },
  'getPtr'(expr) {
    return Module['_BinaryenStoreGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenStoreSetPtr'](expr, ptrExpr);
  },
  'getValue'(expr) {
    return Module['_BinaryenStoreGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenStoreSetValue'](expr, valueExpr);
  },
  'getValueType'(expr) {
    return Module['_BinaryenStoreGetValueType'](expr);
  },
  'setValueType'(expr, valueType) {
    Module['_BinaryenStoreSetValueType'](expr, valueType);
  }
});

Module['Const'] = makeExpressionWrapper({
  'getValueI32'(expr) {
    return Module['_BinaryenConstGetValueI32'](expr);
  },
  'setValueI32'(expr, value) {
    Module['_BinaryenConstSetValueI32'](expr, value);
  },
  'getValueI64Low'(expr) {
    return Module['_BinaryenConstGetValueI64Low'](expr);
  },
  'setValueI64Low'(expr, value) {
    Module['_BinaryenConstSetValueI64Low'](expr, value);
  },
  'getValueI64High'(expr) {
    return Module['_BinaryenConstGetValueI64High'](expr);
  },
  'setValueI64High'(expr, value) {
    Module['_BinaryenConstSetValueI64High'](expr, value);
  },
  'getValueF32'(expr) {
    return Module['_BinaryenConstGetValueF32'](expr);
  },
  'setValueF32'(expr, value) {
    Module['_BinaryenConstSetValueF32'](expr, value);
  },
  'getValueF64'(expr) {
    return Module['_BinaryenConstGetValueF64'](expr);
  },
  'setValueF64'(expr, value) {
    Module['_BinaryenConstSetValueF64'](expr, value);
  },
  'getValueV128'(expr) {
    let value;
    preserveStack(() => {
      const tempBuffer = stackAlloc(16);
      Module['_BinaryenConstGetValueV128'](expr, tempBuffer);
      value = new Array(16);
      for (let i = 0 ; i < 16; ++i) {
        value[i] = HEAPU8[tempBuffer + i];
      }
    });
    return value;
  },
  'setValueV128'(expr, value) {
    preserveStack(() => {
      const tempBuffer = stackAlloc(16);
      for (let i = 0 ; i < 16; ++i) {
        HEAPU8[tempBuffer + i] = value[i];
      }
      Module['_BinaryenConstSetValueV128'](expr, tempBuffer);
    });
  }
});

Module['Unary'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenUnaryGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenUnarySetOp'](expr, op);
  },
  'getValue'(expr) {
    return Module['_BinaryenUnaryGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenUnarySetValue'](expr, valueExpr);
  }
});

Module['Binary'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenBinaryGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenBinarySetOp'](expr, op);
  },
  'getLeft'(expr) {
    return Module['_BinaryenBinaryGetLeft'](expr);
  },
  'setLeft'(expr, leftExpr) {
    Module['_BinaryenBinarySetLeft'](expr, leftExpr);
  },
  'getRight'(expr) {
    return Module['_BinaryenBinaryGetRight'](expr);
  },
  'setRight'(expr, rightExpr) {
    Module['_BinaryenBinarySetRight'](expr, rightExpr);
  }
});

Module['Select'] = makeExpressionWrapper({
  'getIfTrue'(expr) {
    return Module['_BinaryenSelectGetIfTrue'](expr);
  },
  'setIfTrue'(expr, ifTrueExpr) {
    Module['_BinaryenSelectSetIfTrue'](expr, ifTrueExpr);
  },
  'getIfFalse'(expr) {
    return Module['_BinaryenSelectGetIfFalse'](expr);
  },
  'setIfFalse'(expr, ifFalseExpr) {
    Module['_BinaryenSelectSetIfFalse'](expr, ifFalseExpr);
  },
  'getCondition'(expr) {
    return Module['_BinaryenSelectGetCondition'](expr);
  },
  'setCondition'(expr, condExpr) {
    Module['_BinaryenSelectSetCondition'](expr, condExpr);
  }
});

Module['Drop'] = makeExpressionWrapper({
  'getValue'(expr) {
    return Module['_BinaryenDropGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenDropSetValue'](expr, valueExpr);
  }
});

Module['Return'] = makeExpressionWrapper({
  'getValue'(expr) {
    return Module['_BinaryenReturnGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenReturnSetValue'](expr, valueExpr);
  }
});

Module['AtomicRMW'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenAtomicRMWGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenAtomicRMWSetOp'](expr, op);
  },
  'getBytes'(expr) {
    return Module['_BinaryenAtomicRMWGetBytes'](expr);
  },
  'setBytes'(expr, bytes) {
    Module['_BinaryenAtomicRMWSetBytes'](expr, bytes);
  },
  'getOffset'(expr) {
    return Module['_BinaryenAtomicRMWGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenAtomicRMWSetOffset'](expr, offset);
  },
  'getPtr'(expr) {
    return Module['_BinaryenAtomicRMWGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenAtomicRMWSetPtr'](expr, ptrExpr);
  },
  'getValue'(expr) {
    return Module['_BinaryenAtomicRMWGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenAtomicRMWSetValue'](expr, valueExpr);
  }
});

Module['AtomicCmpxchg'] = makeExpressionWrapper({
  'getBytes'(expr) {
    return Module['_BinaryenAtomicCmpxchgGetBytes'](expr);
  },
  'setBytes'(expr, bytes) {
    Module['_BinaryenAtomicCmpxchgSetBytes'](expr, bytes);
  },
  'getOffset'(expr) {
    return Module['_BinaryenAtomicCmpxchgGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenAtomicCmpxchgSetOffset'](expr, offset);
  },
  'getPtr'(expr) {
    return Module['_BinaryenAtomicCmpxchgGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenAtomicCmpxchgSetPtr'](expr, ptrExpr);
  },
  'getExpected'(expr) {
    return Module['_BinaryenAtomicCmpxchgGetExpected'](expr);
  },
  'setExpected'(expr, expectedExpr) {
    Module['_BinaryenAtomicCmpxchgSetExpected'](expr, expectedExpr);
  },
  'getReplacement'(expr) {
    return Module['_BinaryenAtomicCmpxchgGetReplacement'](expr);
  },
  'setReplacement'(expr, replacementExpr) {
    Module['_BinaryenAtomicCmpxchgSetReplacement'](expr, replacementExpr);
  }
});

Module['AtomicWait'] = makeExpressionWrapper({
  'getPtr'(expr) {
    return Module['_BinaryenAtomicWaitGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenAtomicWaitSetPtr'](expr, ptrExpr);
  },
  'getExpected'(expr) {
    return Module['_BinaryenAtomicWaitGetExpected'](expr);
  },
  'setExpected'(expr, expectedExpr) {
    Module['_BinaryenAtomicWaitSetExpected'](expr, expectedExpr);
  },
  'getTimeout'(expr) {
    return Module['_BinaryenAtomicWaitGetTimeout'](expr);
  },
  'setTimeout'(expr, timeoutExpr) {
    Module['_BinaryenAtomicWaitSetTimeout'](expr, timeoutExpr);
  },
  'getExpectedType'(expr) {
    return Module['_BinaryenAtomicWaitGetExpectedType'](expr);
  },
  'setExpectedType'(expr, expectedType) {
    Module['_BinaryenAtomicWaitSetExpectedType'](expr, expectedType);
  }
});

Module['AtomicNotify'] = makeExpressionWrapper({
  'getPtr'(expr) {
    return Module['_BinaryenAtomicNotifyGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenAtomicNotifySetPtr'](expr, ptrExpr);
  },
  'getNotifyCount'(expr) {
    return Module['_BinaryenAtomicNotifyGetNotifyCount'](expr);
  },
  'setNotifyCount'(expr, notifyCountExpr) {
    Module['_BinaryenAtomicNotifySetNotifyCount'](expr, notifyCountExpr);
  }
});

Module['AtomicFence'] = makeExpressionWrapper({
  'getOrder'(expr) {
    return Module['_BinaryenAtomicFenceGetOrder'](expr);
  },
  'setOrder'(expr, order) {
    Module['_BinaryenAtomicFenceSetOrder'](expr, order);
  }
});

Module['SIMDExtract'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenSIMDExtractGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenSIMDExtractSetOp'](expr, op);
  },
  'getVec'(expr) {
    return Module['_BinaryenSIMDExtractGetVec'](expr);
  },
  'setVec'(expr, vecExpr) {
    Module['_BinaryenSIMDExtractSetVec'](expr, vecExpr);
  },
  'getIndex'(expr) {
    return Module['_BinaryenSIMDExtractGetIndex'](expr);
  },
  'setIndex'(expr, index) {
    Module['_BinaryenSIMDExtractSetIndex'](expr, index)
  }
});

Module['SIMDReplace'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenSIMDReplaceGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenSIMDReplaceSetOp'](expr, op);
  },
  'getVec'(expr) {
    return Module['_BinaryenSIMDReplaceGetVec'](expr);
  },
  'setVec'(expr, vecExpr) {
    Module['_BinaryenSIMDReplaceSetVec'](expr, vecExpr);
  },
  'getIndex'(expr) {
    return Module['_BinaryenSIMDReplaceGetIndex'](expr);
  },
  'setIndex'(expr, index) {
    Module['_BinaryenSIMDReplaceSetIndex'](expr, index);
  },
  'getValue'(expr) {
    return Module['_BinaryenSIMDReplaceGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenSIMDReplaceSetValue'](expr, valueExpr);
  }
});

Module['SIMDShuffle'] = makeExpressionWrapper({
  'getLeft'(expr) {
    return Module['_BinaryenSIMDShuffleGetLeft'](expr);
  },
  'setLeft'(expr, leftExpr) {
    Module['_BinaryenSIMDShuffleSetLeft'](expr, leftExpr)
  },
  'getRight'(expr) {
    return Module['_BinaryenSIMDShuffleGetRight'](expr);
  },
  'setRight'(expr, rightExpr) {
    Module['_BinaryenSIMDShuffleSetRight'](expr, rightExpr);
  },
  'getMask'(expr) {
    let mask;
    preserveStack(() => {
      const tempBuffer = stackAlloc(16);
      Module['_BinaryenSIMDShuffleGetMask'](expr, tempBuffer);
      mask = new Array(16);
      for (let i = 0 ; i < 16; ++i) {
        mask[i] = HEAPU8[tempBuffer + i];
      }
    });
    return mask;
  },
  'setMask'(expr, mask) {
    preserveStack(() => {
      const tempBuffer = stackAlloc(16);
      for (let i = 0 ; i < 16; ++i) {
        HEAPU8[tempBuffer + i] = mask[i];
      }
      Module['_BinaryenSIMDShuffleSetMask'](expr, tempBuffer);
    });
  }
});

Module['SIMDTernary'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenSIMDTernaryGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenSIMDTernarySetOp'](expr, op);
  },
  'getA'(expr) {
    return Module['_BinaryenSIMDTernaryGetA'](expr);
  },
  'setA'(expr, aExpr) {
    Module['_BinaryenSIMDTernarySetA'](expr, aExpr);
  },
  'getB'(expr) {
    return Module['_BinaryenSIMDTernaryGetB'](expr);
  },
  'setB'(expr, bExpr) {
    Module['_BinaryenSIMDTernarySetB'](expr, bExpr);
  },
  'getC'(expr) {
    return Module['_BinaryenSIMDTernaryGetC'](expr);
  },
  'setC'(expr, cExpr) {
    Module['_BinaryenSIMDTernarySetC'](expr, cExpr);
  }
});

Module['SIMDShift'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenSIMDShiftGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenSIMDShiftSetOp'](expr, op);
  },
  'getVec'(expr) {
    return Module['_BinaryenSIMDShiftGetVec'](expr);
  },
  'setVec'(expr, vecExpr) {
    Module['_BinaryenSIMDShiftSetVec'](expr, vecExpr);
  },
  'getShift'(expr) {
    return Module['_BinaryenSIMDShiftGetShift'](expr);
  },
  'setShift'(expr, shiftExpr) {
    Module['_BinaryenSIMDShiftSetShift'](expr, shiftExpr);
  }
});

Module['SIMDLoad'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenSIMDLoadGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenSIMDLoadSetOp'](expr, op);
  },
  'getOffset'(expr) {
    return Module['_BinaryenSIMDLoadGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenSIMDLoadSetOffset'](expr, offset);
  },
  'getAlign'(expr) {
    return Module['_BinaryenSIMDLoadGetAlign'](expr);
  },
  'setAlign'(expr, align) {
    Module['_BinaryenSIMDLoadSetAlign'](expr, align);
  },
  'getPtr'(expr) {
    return Module['_BinaryenSIMDLoadGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenSIMDLoadSetPtr'](expr, ptrExpr);
  }
});

Module['SIMDLoadStoreLane'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenSIMDLoadStoreLaneGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenSIMDLoadStoreLaneSetOp'](expr, op);
  },
  'getOffset'(expr) {
    return Module['_BinaryenSIMDLoadStoreLaneGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenSIMDLoadStoreLaneSetOffset'](expr, offset);
  },
  'getAlign'(expr) {
    return Module['_BinaryenSIMDLoadStoreLaneGetAlign'](expr);
  },
  'setAlign'(expr, align) {
    Module['_BinaryenSIMDLoadStoreLaneSetAlign'](expr, align);
  },
  'getIndex'(expr) {
    return Module['_BinaryenSIMDLoadStoreLaneGetIndex'](expr);
  },
  'setIndex'(expr, align) {
    Module['_BinaryenSIMDLoadStoreLaneSetIndex'](expr, align);
  },
  'getPtr'(expr) {
    return Module['_BinaryenSIMDLoadStoreLaneGetPtr'](expr);
  },
  'setPtr'(expr, ptrExpr) {
    Module['_BinaryenSIMDLoadStoreLaneSetPtr'](expr, ptrExpr);
  },
  'getVec'(expr) {
    return Module['_BinaryenSIMDLoadStoreLaneGetVec'](expr);
  },
  'setVec'(expr, ptrExpr) {
    Module['_BinaryenSIMDLoadStoreLaneSetVec'](expr, ptrExpr);
  },
  'isStore'(expr) {
    return Boolean(Module['_BinaryenSIMDLoadStoreLaneIsStore'](expr));
  }
});

Module['MemoryInit'] = makeExpressionWrapper({
  'getSegment'(expr) {
    return Module['_BinaryenMemoryInitGetSegment'](expr);
  },
  'setSegment'(expr, segmentIndex) {
    Module['_BinaryenMemoryInitSetSegment'](expr, segmentIndex);
  },
  'getDest'(expr) {
    return Module['_BinaryenMemoryInitGetDest'](expr);
  },
  'setDest'(expr, destExpr) {
    Module['_BinaryenMemoryInitSetDest'](expr, destExpr);
  },
  'getOffset'(expr) {
    return Module['_BinaryenMemoryInitGetOffset'](expr);
  },
  'setOffset'(expr, offset) {
    Module['_BinaryenMemoryInitSetOffset'](expr, offset);
  },
  'getSize'(expr) {
    return Module['_BinaryenMemoryInitGetSize'](expr);
  },
  'setSize'(expr, sizeExpr) {
    Module['_BinaryenMemoryInitSetSize'](expr, sizeExpr);
  }
});

Module['DataDrop'] = makeExpressionWrapper({
  'getSegment'(expr) {
    return Module['_BinaryenDataDropGetSegment'](expr);
  },
  'setSegment'(expr, segmentIndex) {
    Module['_BinaryenDataDropSetSegment'](expr, segmentIndex);
  }
});

Module['MemoryCopy'] = makeExpressionWrapper({
  'getDest'(expr) {
    return Module['_BinaryenMemoryCopyGetDest'](expr);
  },
  'setDest'(expr, destExpr) {
    Module['_BinaryenMemoryCopySetDest'](expr, destExpr);
  },
  'getSource'(expr) {
    return Module['_BinaryenMemoryCopyGetSource'](expr);
  },
  'setSource'(expr, sourceExpr) {
    Module['_BinaryenMemoryCopySetSource'](expr, sourceExpr);
  },
  'getSize'(expr) {
    return Module['_BinaryenMemoryCopyGetSize'](expr);
  },
  'setSize'(expr, sizeExpr) {
    Module['_BinaryenMemoryCopySetSize'](expr, sizeExpr);
  }
});

Module['MemoryFill'] = makeExpressionWrapper({
  'getDest'(expr) {
    return Module['_BinaryenMemoryFillGetDest'](expr);
  },
  'setDest'(expr, destExpr) {
    Module['_BinaryenMemoryFillSetDest'](expr, destExpr);
  },
  'getValue'(expr) {
    return Module['_BinaryenMemoryFillGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenMemoryFillSetValue'](expr, valueExpr);
  },
  'getSize'(expr) {
    return Module['_BinaryenMemoryFillGetSize'](expr);
  },
  'setSize'(expr, sizeExpr) {
    Module['_BinaryenMemoryFillSetSize'](expr, sizeExpr);
  }
});

Module['RefIs'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenRefIsGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenRefIsSetOp'](expr, op);
  },
  'getValue'(expr) {
    return Module['_BinaryenRefIsGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenRefIsSetValue'](expr, valueExpr);
  }
});

Module['RefAs'] = makeExpressionWrapper({
  'getOp'(expr) {
    return Module['_BinaryenRefAsGetOp'](expr);
  },
  'setOp'(expr, op) {
    Module['_BinaryenRefAsSetOp'](expr, op);
  },
  'getValue'(expr) {
    return Module['_BinaryenRefAsGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenRefAsSetValue'](expr, valueExpr);
  }
});

Module['RefFunc'] = makeExpressionWrapper({
  'getFunc'(expr) {
    return UTF8ToString(Module['_BinaryenRefFuncGetFunc'](expr));
  },
  'setFunc'(expr, funcName) {
    preserveStack(() => { Module['_BinaryenRefFuncSetFunc'](expr, strToStack(funcName)) });
  }
});

Module['RefEq'] = makeExpressionWrapper({
  'getLeft'(expr) {
    return Module['_BinaryenRefEqGetLeft'](expr);
  },
  'setLeft'(expr, leftExpr) {
    return Module['_BinaryenRefEqSetLeft'](expr, leftExpr);
  },
  'getRight'(expr) {
    return Module['_BinaryenRefEqGetRight'](expr);
  },
  'setRight'(expr, rightExpr) {
    return Module['_BinaryenRefEqSetRight'](expr, rightExpr);
  }
});

Module['Try'] = makeExpressionWrapper({
  'getName'(expr) {
    const name = Module['_BinaryenTryGetName'](expr);
    return name ? UTF8ToString(name) : null;
  },
  'setName'(expr, name) {
    preserveStack(() => { Module['_BinaryenTrySetName'](expr, strToStack(name)) });
  },
  'getBody'(expr) {
    return Module['_BinaryenTryGetBody'](expr);
  },
  'setBody'(expr, bodyExpr) {
    Module['_BinaryenTrySetBody'](expr, bodyExpr);
  },
  'getNumCatchEvents'(expr) {
    return Module['_BinaryenTryGetNumCatchEvents'](expr);
  },
  'getCatchEvents'(expr) {
    return getAllNested(expr, Module['_BinaryenTryGetNumCatchEvents'], Module['_BinaryenTryGetCatchEventAt']).map(p => UTF8ToString(p));
  },
  'setCatchEvents'(expr, catchEvents) {
    preserveStack(() => {
      setAllNested(expr, catchEvents.map(strToStack), Module['_BinaryenTryGetNumCatchEvents'], Module['_BinaryenTrySetCatchEventAt'], Module['_BinaryenTryAppendCatchEvent'], Module['_BinaryenTryRemoveCatchEventAt']);
    });
  },
  'getCatchEventAt'(expr, index) {
    return UTF8ToString(Module['_BinaryenTryGetCatchEventAt'](expr, index));
  },
  'setCatchEventAt'(expr, index, catchEvent) {
    preserveStack(() => { Module['_BinaryenTrySetCatchEventAt'](expr, index, strToStack(catchEvent)) });
  },
  'appendCatchEvent'(expr, catchEvent) {
    preserveStack(() => Module['_BinaryenTryAppendCatchEvent'](expr, strToStack(catchEvent)));
  },
  'insertCatchEventAt'(expr, index, catchEvent) {
    preserveStack(() => { Module['_BinaryenTryInsertCatchEventAt'](expr, index, strToStack(catchEvent)) });
  },
  'removeCatchEventAt'(expr, index) {
    return UTF8ToString(Module['_BinaryenTryRemoveCatchEventAt'](expr, index));
  },
  'getNumCatchBodies'(expr) {
    return Module['_BinaryenTryGetNumCatchBodies'](expr);
  },
  'getCatchBodies'(expr) {
    return getAllNested(expr, Module['_BinaryenTryGetNumCatchBodies'], Module['_BinaryenTryGetCatchBodyAt']);
  },
  'setCatchBodies'(expr, catchBodies) {
    setAllNested(expr, catchBodies, Module['_BinaryenTryGetNumCatchBodies'], Module['_BinaryenTrySetCatchBodyAt'], Module['_BinaryenTryAppendCatchBody'], Module['_BinaryenTryRemoveCatchBodyAt']);
  },
  'getCatchBodyAt'(expr, index) {
    return Module['_BinaryenTryGetCatchBodyAt'](expr, index);
  },
  'setCatchBodyAt'(expr, index, catchExpr) {
    Module['_BinaryenTrySetCatchBodyAt'](expr, index, catchExpr);
  },
  'appendCatchBody'(expr, catchExpr) {
    return Module['_BinaryenTryAppendCatchBody'](expr, catchExpr);
  },
  'insertCatchBodyAt'(expr, index, catchExpr) {
    Module['_BinaryenTryInsertCatchBodyAt'](expr, index, catchExpr);
  },
  'removeCatchBodyAt'(expr, index) {
    return Module['_BinaryenTryRemoveCatchBodyAt'](expr, index);
  },
  'hasCatchAll'(expr) {
    return Boolean(Module['_BinaryenTryHasCatchAll'](expr));
  },
  'getDelegateTarget'(expr) {
    const name = Module['_BinaryenTryGetDelegateTarget'](expr);
    return name ? UTF8ToString(name) : null;
  },
  'setDelegateTarget'(expr, name) {
    preserveStack(() => { Module['_BinaryenTrySetDelegateTarget'](expr, strToStack(name)) });
  },
  'isDelegate'(expr) {
    return Boolean(Module['_BinaryenTryIsDelegate'](expr));
  }
});

Module['Throw'] = makeExpressionWrapper({
  'getEvent'(expr) {
    return UTF8ToString(Module['_BinaryenThrowGetEvent'](expr));
  },
  'setEvent'(expr, eventName) {
    preserveStack(() => { Module['_BinaryenThrowSetEvent'](expr, strToStack(eventName)) });
  },
  'getNumOperands'(expr) {
    return Module['_BinaryenThrowGetNumOperands'](expr);
  },
  'getOperands'(expr) {
    return getAllNested(expr, Module['_BinaryenThrowGetNumOperands'], Module['_BinaryenThrowGetOperandAt']);
  },
  'setOperands'(expr, operands) {
    setAllNested(expr, operands, Module['_BinaryenThrowGetNumOperands'], Module['_BinaryenThrowSetOperandAt'], Module['_BinaryenThrowAppendOperand'], Module['_BinaryenThrowRemoveOperandAt']);
  },
  'getOperandAt'(expr, index) {
    return Module['_BinaryenThrowGetOperandAt'](expr, index);
  },
  'setOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenThrowSetOperandAt'](expr, index, operandExpr);
  },
  'appendOperand'(expr, operandExpr) {
    return Module['_BinaryenThrowAppendOperand'](expr, operandExpr);
  },
  'insertOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenThrowInsertOperandAt'](expr, index, operandExpr);
  },
  'removeOperandAt'(expr, index) {
    return Module['_BinaryenThrowRemoveOperandAt'](expr, index);
  },
});

Module['Rethrow'] = makeExpressionWrapper({
  'getTarget'(expr) {
    const target = Module['_BinaryenRethrowGetTarget'](expr);
    return target ? UTF8ToString(target) : null;
  },
  'setTarget'(expr, target) {
    preserveStack(() => { Module['_BinaryenRethrowSetTarget'](expr, strToStack(target)) });
  }
});

Module['TupleMake'] = makeExpressionWrapper({
  'getNumOperands'(expr) {
    return Module['_BinaryenTupleMakeGetNumOperands'](expr);
  },
  'getOperands'(expr) {
    return getAllNested(expr, Module['_BinaryenTupleMakeGetNumOperands'], Module['_BinaryenTupleMakeGetOperandAt']);
  },
  'setOperands'(expr, operands) {
    setAllNested(expr, operands, Module['_BinaryenTupleMakeGetNumOperands'], Module['_BinaryenTupleMakeSetOperandAt'], Module['_BinaryenTupleMakeAppendOperand'], Module['_BinaryenTupleMakeRemoveOperandAt']);
  },
  'getOperandAt'(expr, index) {
    return Module['_BinaryenTupleMakeGetOperandAt'](expr, index);
  },
  'setOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenTupleMakeSetOperandAt'](expr, index, operandExpr);
  },
  'appendOperand'(expr, operandExpr) {
    return Module['_BinaryenTupleMakeAppendOperand'](expr, operandExpr);
  },
  'insertOperandAt'(expr, index, operandExpr) {
    Module['_BinaryenTupleMakeInsertOperandAt'](expr, index, operandExpr);
  },
  'removeOperandAt'(expr, index) {
    return Module['_BinaryenTupleMakeRemoveOperandAt'](expr, index);
  }
});

Module['TupleExtract'] = makeExpressionWrapper({
  'getTuple'(expr) {
    return Module['_BinaryenTupleExtractGetTuple'](expr);
  },
  'setTuple'(expr, tupleExpr) {
    Module['_BinaryenTupleExtractSetTuple'](expr, tupleExpr);
  },
  'getIndex'(expr) {
    return Module['_BinaryenTupleExtractGetIndex'](expr);
  },
  'setIndex'(expr, index) {
    Module['_BinaryenTupleExtractSetIndex'](expr, index);
  }
});

Module['I31New'] = makeExpressionWrapper({
  'getValue'(expr) {
    return Module['_BinaryenI31NewGetValue'](expr);
  },
  'setValue'(expr, valueExpr) {
    Module['_BinaryenI31NewSetValue'](expr, valueExpr);
  }
});

Module['I31Get'] = makeExpressionWrapper({
  'getI31'(expr) {
    return Module['_BinaryenI31GetGetI31'](expr);
  },
  'setI31'(expr, i31Expr) {
    Module['_BinaryenI31GetSetI31'](expr, i31Expr);
  },
  'isSigned'(expr) {
    return Boolean(Module['_BinaryenI31GetIsSigned'](expr));
  },
  'setSigned'(expr, isSigned) {
    Module['_BinaryenI31GetSetSigned'](expr, isSigned);
  }
});

// Function wrapper

Module['Function'] = (() => {
  // Closure compiler doesn't allow multiple `Function`s at top-level, so:
  function Function(func) {
    if (!(this instanceof Function)) {
      if (!func) return null;
      return new Function(func);
    }
    if (!func) throw Error("function reference must not be null");
    this[thisPtr] = func;
  }
  Function['getName'] = function(func) {
    return UTF8ToString(Module['_BinaryenFunctionGetName'](func));
  };
  Function['getParams'] = function(func) {
    return Module['_BinaryenFunctionGetParams'](func);
  };
  Function['getResults'] = function(func) {
    return Module['_BinaryenFunctionGetResults'](func);
  };
  Function['getNumVars'] = function(func) {
    return Module['_BinaryenFunctionGetNumVars'](func);
  };
  Function['getVar'] = function(func, index) {
    return Module['_BinaryenFunctionGetVar'](func, index);
  };
  Function['getNumLocals'] = function(func) {
    return Module['_BinaryenFunctionGetNumLocals'](func);
  };
  Function['hasLocalName'] = function(func, index) {
    return Boolean(Module['_BinaryenFunctionHasLocalName'](func, index));
  };
  Function['getLocalName'] = function(func, index) {
    return UTF8ToString(Module['_BinaryenFunctionGetLocalName'](func, index));
  };
  Function['setLocalName'] = function(func, index, name) {
    preserveStack(() => {
      Module['_BinaryenFunctionSetLocalName'](func, index, strToStack(name));
    });
  };
  Function['getBody'] = function(func) {
    return Module['_BinaryenFunctionGetBody'](func);
  };
  Function['setBody'] = function(func, bodyExpr) {
    Module['_BinaryenFunctionSetBody'](func, bodyExpr);
  };
  deriveWrapperInstanceMembers(Function.prototype, Function);
  Function.prototype['valueOf'] = function() {
    return this[thisPtr];
  };
  return Function;
})();

// Additional customizations

Module['exit'] = function(status) {
  // Instead of exiting silently on errors, always show an error with
  // a stack trace, for debuggability.
  if (status != 0) throw new Error('exiting due to error: ' + status);
};

// Indicates if Binaryen has been loaded and is ready
Module['isReady'] = runtimeInitialized;

// Provide a mechanism to tell when the module is ready
//
// if (!binaryen.isReady) await binaryen.ready;
// ...
//
let pendingPromises = [];
let initializeError = null;

Object.defineProperty(Module, 'ready', {
  get() {
    return new Promise((resolve, reject) => {
      if (initializeError) {
        reject(initializeError);
      } else if (runtimeInitialized) {
        resolve(Module);
      } else {
        pendingPromises.push({ resolve, reject });
      }
    });
  }
});

// Intercept the onRuntimeInitialized hook if necessary
if (runtimeInitialized) {
  initializeConstants();
} else {
  Module['onRuntimeInitialized'] = (super_ => () => {
    try {
      initializeConstants();
      if (super_) super_();
      Module['isReady'] = true;
      pendingPromises.forEach(p => { p.resolve(Module) });
    } catch (e) {
      initializeError = e;
      pendingPromises.forEach(p => { p.reject(e) });
    } finally {
      pendingPromises = [];
    }
  })(Module['onRuntimeInitialized']);
}
