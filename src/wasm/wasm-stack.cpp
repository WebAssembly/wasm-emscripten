/*
 * Copyright 2019 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wasm-stack.h"
#include "ir/find_all.h"
#include "ir/properties.h"
#include "wasm-binary.h"
#include "wasm-debug.h"

namespace wasm {

static Name IMPOSSIBLE_CONTINUE("impossible-continue");

void BinaryInstWriter::emitResultType(Type type) {
  if (type == Type::unreachable) {
    parent.writeType(Type::none);
  } else if (type.isTuple()) {
    o << S32LEB(parent.getSignatureIndex(Signature(Type::none, type)));
  } else {
    parent.writeType(type);
  }
}

void BinaryInstWriter::visitBlock(Block* curr) {
  breakStack.push_back(curr->name);
  o << int8_t(BinaryConsts::Block);
  emitResultType(curr->type);
}

void BinaryInstWriter::visitIf(If* curr) {
  // the binary format requires this; we have a block if we need one
  // TODO: optimize this in Stack IR (if child is a block, we may break to this
  // instead)
  breakStack.emplace_back(IMPOSSIBLE_CONTINUE);
  o << int8_t(BinaryConsts::If);
  emitResultType(curr->type);
}

void BinaryInstWriter::emitIfElse(If* curr) {
  if (func && !sourceMap) {
    parent.writeExtraDebugLocation(curr, func, BinaryLocations::Else);
  }
  o << int8_t(BinaryConsts::Else);
}

void BinaryInstWriter::visitLoop(Loop* curr) {
  breakStack.push_back(curr->name);
  o << int8_t(BinaryConsts::Loop);
  emitResultType(curr->type);
}

void BinaryInstWriter::visitBreak(Break* curr) {
  o << int8_t(curr->condition ? BinaryConsts::BrIf : BinaryConsts::Br)
    << U32LEB(getBreakIndex(curr->name));

  // See comment on |brIfsNeedingHandling| for the extra casts we need to emit
  // here for certain br_ifs.
  auto iter = brIfsNeedingHandling.find(curr);
  if (iter != brIfsNeedingHandling.end()) {
    auto unrefinedType = iter->second;
    auto type = curr->type;
    assert(type.size() == unrefinedType.size());

    assert(curr->type.hasRef());

    auto emitCast = [&](Type to) {
      // Shim a tiny bit of IR, just enough to get visitRefCast to see what we
      // are casting, and to emit the proper thing.
      RefCast cast;
      cast.type = to;
      cast.ref = nullptr;
      visitRefCast(&cast);
    };

    if (!type.isTuple()) {
      // Simple: Just emit a cast, and then the type matches Binaryen IR's.
      emitCast(type);
    } else {
      // Tuples are trickier to handle, and we need to use scratch locals. Stash
      // all the values on the stack to those locals, then reload them, casting
      // as we go.
      //
      // We must track how many scratch locals we've used from each type as we
      // go, as a type might appear multiple times in the tuple. We allocated
      // enough for each, in a contiguous range, so we just increment as we go.
      std::unordered_map<Type, Index> scratchTypeUses;
      for (Index i = 0; i < unrefinedType.size(); i++) {
        auto t = unrefinedType[unrefinedType.size() - i - 1];
        assert(scratchLocals.find(t) != scratchLocals.end());
        auto localIndex = scratchLocals[t] + scratchTypeUses[t]++;
        o << int8_t(BinaryConsts::LocalSet) << U32LEB(localIndex);
      }
      for (Index i = 0; i < unrefinedType.size(); i++) {
        auto t = unrefinedType[i];
        auto localIndex = scratchLocals[t] + --scratchTypeUses[t];
        o << int8_t(BinaryConsts::LocalGet) << U32LEB(localIndex);
        if (t.isRef()) {
          // Note that we cast all types here, when perhaps only some of the
          // tuple's lanes need that. This is simpler.
          emitCast(type[i]);
        }
      }
    }
  }
}

void BinaryInstWriter::visitSwitch(Switch* curr) {
  o << int8_t(BinaryConsts::BrTable) << U32LEB(curr->targets.size());
  for (auto target : curr->targets) {
    o << U32LEB(getBreakIndex(target));
  }
  o << U32LEB(getBreakIndex(curr->default_));
}

void BinaryInstWriter::visitCall(Call* curr) {
  int8_t op =
    curr->isReturn ? BinaryConsts::RetCallFunction : BinaryConsts::CallFunction;
  o << op << U32LEB(parent.getFunctionIndex(curr->target));
}

void BinaryInstWriter::visitCallIndirect(CallIndirect* curr) {
  Index tableIdx = parent.getTableIndex(curr->table);
  int8_t op =
    curr->isReturn ? BinaryConsts::RetCallIndirect : BinaryConsts::CallIndirect;
  o << op << U32LEB(parent.getTypeIndex(curr->heapType)) << U32LEB(tableIdx);
}

void BinaryInstWriter::visitLocalGet(LocalGet* curr) {
  if (deferredGets.count(curr)) {
    // This local.get will be emitted as part of the instruction that consumes
    // it.
    return;
  }
  if (auto it = extractedGets.find(curr); it != extractedGets.end()) {
    // We have a tuple of locals to get, but we will only end up using one of
    // them, so we can just emit that one.
    o << int8_t(BinaryConsts::LocalGet)
      << U32LEB(mappedLocals[std::make_pair(curr->index, it->second)]);
    return;
  }
  size_t numValues = func->getLocalType(curr->index).size();
  for (Index i = 0; i < numValues; ++i) {
    o << int8_t(BinaryConsts::LocalGet)
      << U32LEB(mappedLocals[std::make_pair(curr->index, i)]);
  }
}

void BinaryInstWriter::visitLocalSet(LocalSet* curr) {
  size_t numValues = func->getLocalType(curr->index).size();
  // If this is a tuple, set all the elements with nonzero index.
  for (Index i = numValues - 1; i >= 1; --i) {
    o << int8_t(BinaryConsts::LocalSet)
      << U32LEB(mappedLocals[std::make_pair(curr->index, i)]);
  }
  if (!curr->isTee()) {
    // This is not a tee, so just finish setting the values.
    o << int8_t(BinaryConsts::LocalSet)
      << U32LEB(mappedLocals[std::make_pair(curr->index, 0)]);
  } else if (auto it = extractedGets.find(curr); it != extractedGets.end()) {
    // We only need to get the single extracted value.
    if (it->second == 0) {
      o << int8_t(BinaryConsts::LocalTee)
        << U32LEB(mappedLocals[std::make_pair(curr->index, 0)]);
    } else {
      o << int8_t(BinaryConsts::LocalSet)
        << U32LEB(mappedLocals[std::make_pair(curr->index, 0)]);
      o << int8_t(BinaryConsts::LocalGet)
        << U32LEB(mappedLocals[std::make_pair(curr->index, it->second)]);
    }
  } else {
    // We need to get all the values.
    o << int8_t(BinaryConsts::LocalTee)
      << U32LEB(mappedLocals[std::make_pair(curr->index, 0)]);
    for (Index i = 1; i < numValues; ++i) {
      o << int8_t(BinaryConsts::LocalGet)
        << U32LEB(mappedLocals[std::make_pair(curr->index, i)]);
    }
  }
}

void BinaryInstWriter::visitGlobalGet(GlobalGet* curr) {
  Index index = parent.getGlobalIndex(curr->name);
  if (auto it = extractedGets.find(curr); it != extractedGets.end()) {
    // We have a tuple of globals to get, but we will only end up using one of
    // them, so we can just emit that one.
    o << int8_t(BinaryConsts::GlobalGet) << U32LEB(index + it->second);
    return;
  }
  // Emit a global.get for each element if this is a tuple global
  size_t numValues = curr->type.size();
  for (Index i = 0; i < numValues; ++i) {
    o << int8_t(BinaryConsts::GlobalGet) << U32LEB(index + i);
  }
}

void BinaryInstWriter::visitGlobalSet(GlobalSet* curr) {
  // Emit a global.set for each element if this is a tuple global
  Index index = parent.getGlobalIndex(curr->name);
  size_t numValues = parent.getModule()->getGlobal(curr->name)->type.size();
  for (int i = numValues - 1; i >= 0; --i) {
    o << int8_t(BinaryConsts::GlobalSet) << U32LEB(index + i);
  }
}

void BinaryInstWriter::visitLoad(Load* curr) {
  if (!curr->isAtomic) {
    switch (curr->type.getBasic()) {
      case Type::i32: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(curr->signed_ ? BinaryConsts::I32LoadMem8S
                                      : BinaryConsts::I32LoadMem8U);
            break;
          case 2:
            o << int8_t(curr->signed_ ? BinaryConsts::I32LoadMem16S
                                      : BinaryConsts::I32LoadMem16U);
            break;
          case 4:
            o << int8_t(BinaryConsts::I32LoadMem);
            break;
          default:
            abort();
        }
        break;
      }
      case Type::i64: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(curr->signed_ ? BinaryConsts::I64LoadMem8S
                                      : BinaryConsts::I64LoadMem8U);
            break;
          case 2:
            o << int8_t(curr->signed_ ? BinaryConsts::I64LoadMem16S
                                      : BinaryConsts::I64LoadMem16U);
            break;
          case 4:
            o << int8_t(curr->signed_ ? BinaryConsts::I64LoadMem32S
                                      : BinaryConsts::I64LoadMem32U);
            break;
          case 8:
            o << int8_t(BinaryConsts::I64LoadMem);
            break;
          default:
            abort();
        }
        break;
      }
      case Type::f32: {
        switch (curr->bytes) {
          case 2:
            o << int8_t(BinaryConsts::MiscPrefix)
              << U32LEB(BinaryConsts::F32_F16LoadMem);
            break;
          case 4:
            o << int8_t(BinaryConsts::F32LoadMem);
            break;
          default:
            WASM_UNREACHABLE("invalid load size");
        }
        break;
      }
      case Type::f64:
        o << int8_t(BinaryConsts::F64LoadMem);
        break;
      case Type::v128:
        o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128Load);
        break;
      case Type::unreachable:
        // the pointer is unreachable, so we are never reached; just don't emit
        // a load
        return;
      case Type::none:
        WASM_UNREACHABLE("unexpected type");
    }
  } else {
    o << int8_t(BinaryConsts::AtomicPrefix);
    switch (curr->type.getBasic()) {
      case Type::i32: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(BinaryConsts::I32AtomicLoad8U);
            break;
          case 2:
            o << int8_t(BinaryConsts::I32AtomicLoad16U);
            break;
          case 4:
            o << int8_t(BinaryConsts::I32AtomicLoad);
            break;
          default:
            WASM_UNREACHABLE("invalid load size");
        }
        break;
      }
      case Type::i64: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(BinaryConsts::I64AtomicLoad8U);
            break;
          case 2:
            o << int8_t(BinaryConsts::I64AtomicLoad16U);
            break;
          case 4:
            o << int8_t(BinaryConsts::I64AtomicLoad32U);
            break;
          case 8:
            o << int8_t(BinaryConsts::I64AtomicLoad);
            break;
          default:
            WASM_UNREACHABLE("invalid load size");
        }
        break;
      }
      case Type::unreachable:
        return;
      default:
        WASM_UNREACHABLE("unexpected type");
    }
  }
  emitMemoryAccess(curr->align, curr->bytes, curr->offset, curr->memory);
}

void BinaryInstWriter::visitStore(Store* curr) {
  if (!curr->isAtomic) {
    switch (curr->valueType.getBasic()) {
      case Type::i32: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(BinaryConsts::I32StoreMem8);
            break;
          case 2:
            o << int8_t(BinaryConsts::I32StoreMem16);
            break;
          case 4:
            o << int8_t(BinaryConsts::I32StoreMem);
            break;
          default:
            abort();
        }
        break;
      }
      case Type::i64: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(BinaryConsts::I64StoreMem8);
            break;
          case 2:
            o << int8_t(BinaryConsts::I64StoreMem16);
            break;
          case 4:
            o << int8_t(BinaryConsts::I64StoreMem32);
            break;
          case 8:
            o << int8_t(BinaryConsts::I64StoreMem);
            break;
          default:
            abort();
        }
        break;
      }
      case Type::f32: {
        switch (curr->bytes) {
          case 2:
            o << int8_t(BinaryConsts::MiscPrefix)
              << U32LEB(BinaryConsts::F32_F16StoreMem);
            break;
          case 4:
            o << int8_t(BinaryConsts::F32StoreMem);
            break;
          default:
            WASM_UNREACHABLE("invalid store size");
        }
        break;
      }
      case Type::f64:
        o << int8_t(BinaryConsts::F64StoreMem);
        break;
      case Type::v128:
        o << int8_t(BinaryConsts::SIMDPrefix)
          << U32LEB(BinaryConsts::V128Store);
        break;
      case Type::none:
      case Type::unreachable:
        WASM_UNREACHABLE("unexpected type");
    }
  } else {
    o << int8_t(BinaryConsts::AtomicPrefix);
    switch (curr->valueType.getBasic()) {
      case Type::i32: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(BinaryConsts::I32AtomicStore8);
            break;
          case 2:
            o << int8_t(BinaryConsts::I32AtomicStore16);
            break;
          case 4:
            o << int8_t(BinaryConsts::I32AtomicStore);
            break;
          default:
            WASM_UNREACHABLE("invalid store size");
        }
        break;
      }
      case Type::i64: {
        switch (curr->bytes) {
          case 1:
            o << int8_t(BinaryConsts::I64AtomicStore8);
            break;
          case 2:
            o << int8_t(BinaryConsts::I64AtomicStore16);
            break;
          case 4:
            o << int8_t(BinaryConsts::I64AtomicStore32);
            break;
          case 8:
            o << int8_t(BinaryConsts::I64AtomicStore);
            break;
          default:
            WASM_UNREACHABLE("invalid store size");
        }
        break;
      }
      default:
        WASM_UNREACHABLE("unexpected type");
    }
  }
  emitMemoryAccess(curr->align, curr->bytes, curr->offset, curr->memory);
}

void BinaryInstWriter::visitAtomicRMW(AtomicRMW* curr) {
  o << int8_t(BinaryConsts::AtomicPrefix);

#define CASE_FOR_OP(Op)                                                        \
  case RMW##Op:                                                                \
    switch (curr->type.getBasic()) {                                           \
      case Type::i32:                                                          \
        switch (curr->bytes) {                                                 \
          case 1:                                                              \
            o << int8_t(BinaryConsts::I32AtomicRMW##Op##8U);                   \
            break;                                                             \
          case 2:                                                              \
            o << int8_t(BinaryConsts::I32AtomicRMW##Op##16U);                  \
            break;                                                             \
          case 4:                                                              \
            o << int8_t(BinaryConsts::I32AtomicRMW##Op);                       \
            break;                                                             \
          default:                                                             \
            WASM_UNREACHABLE("invalid rmw size");                              \
        }                                                                      \
        break;                                                                 \
      case Type::i64:                                                          \
        switch (curr->bytes) {                                                 \
          case 1:                                                              \
            o << int8_t(BinaryConsts::I64AtomicRMW##Op##8U);                   \
            break;                                                             \
          case 2:                                                              \
            o << int8_t(BinaryConsts::I64AtomicRMW##Op##16U);                  \
            break;                                                             \
          case 4:                                                              \
            o << int8_t(BinaryConsts::I64AtomicRMW##Op##32U);                  \
            break;                                                             \
          case 8:                                                              \
            o << int8_t(BinaryConsts::I64AtomicRMW##Op);                       \
            break;                                                             \
          default:                                                             \
            WASM_UNREACHABLE("invalid rmw size");                              \
        }                                                                      \
        break;                                                                 \
      default:                                                                 \
        WASM_UNREACHABLE("unexpected type");                                   \
    }                                                                          \
    break

  switch (curr->op) {
    CASE_FOR_OP(Add);
    CASE_FOR_OP(Sub);
    CASE_FOR_OP(And);
    CASE_FOR_OP(Or);
    CASE_FOR_OP(Xor);
    CASE_FOR_OP(Xchg);
    default:
      WASM_UNREACHABLE("unexpected op");
  }
#undef CASE_FOR_OP

  emitMemoryAccess(curr->bytes, curr->bytes, curr->offset, curr->memory);
}

void BinaryInstWriter::visitAtomicCmpxchg(AtomicCmpxchg* curr) {
  o << int8_t(BinaryConsts::AtomicPrefix);
  switch (curr->type.getBasic()) {
    case Type::i32:
      switch (curr->bytes) {
        case 1:
          o << int8_t(BinaryConsts::I32AtomicCmpxchg8U);
          break;
        case 2:
          o << int8_t(BinaryConsts::I32AtomicCmpxchg16U);
          break;
        case 4:
          o << int8_t(BinaryConsts::I32AtomicCmpxchg);
          break;
        default:
          WASM_UNREACHABLE("invalid size");
      }
      break;
    case Type::i64:
      switch (curr->bytes) {
        case 1:
          o << int8_t(BinaryConsts::I64AtomicCmpxchg8U);
          break;
        case 2:
          o << int8_t(BinaryConsts::I64AtomicCmpxchg16U);
          break;
        case 4:
          o << int8_t(BinaryConsts::I64AtomicCmpxchg32U);
          break;
        case 8:
          o << int8_t(BinaryConsts::I64AtomicCmpxchg);
          break;
        default:
          WASM_UNREACHABLE("invalid size");
      }
      break;
    default:
      WASM_UNREACHABLE("unexpected type");
  }
  emitMemoryAccess(curr->bytes, curr->bytes, curr->offset, curr->memory);
}

void BinaryInstWriter::visitAtomicWait(AtomicWait* curr) {
  o << int8_t(BinaryConsts::AtomicPrefix);
  switch (curr->expectedType.getBasic()) {
    case Type::i32: {
      o << int8_t(BinaryConsts::I32AtomicWait);
      emitMemoryAccess(4, 4, curr->offset, curr->memory);
      break;
    }
    case Type::i64: {
      o << int8_t(BinaryConsts::I64AtomicWait);
      emitMemoryAccess(8, 8, curr->offset, curr->memory);
      break;
    }
    default:
      WASM_UNREACHABLE("unexpected type");
  }
}

void BinaryInstWriter::visitAtomicNotify(AtomicNotify* curr) {
  o << int8_t(BinaryConsts::AtomicPrefix) << int8_t(BinaryConsts::AtomicNotify);
  emitMemoryAccess(4, 4, curr->offset, curr->memory);
}

void BinaryInstWriter::visitAtomicFence(AtomicFence* curr) {
  o << int8_t(BinaryConsts::AtomicPrefix) << int8_t(BinaryConsts::AtomicFence)
    << int8_t(curr->order);
}

void BinaryInstWriter::visitSIMDExtract(SIMDExtract* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix);
  switch (curr->op) {
    case ExtractLaneSVecI8x16:
      o << U32LEB(BinaryConsts::I8x16ExtractLaneS);
      break;
    case ExtractLaneUVecI8x16:
      o << U32LEB(BinaryConsts::I8x16ExtractLaneU);
      break;
    case ExtractLaneSVecI16x8:
      o << U32LEB(BinaryConsts::I16x8ExtractLaneS);
      break;
    case ExtractLaneUVecI16x8:
      o << U32LEB(BinaryConsts::I16x8ExtractLaneU);
      break;
    case ExtractLaneVecI32x4:
      o << U32LEB(BinaryConsts::I32x4ExtractLane);
      break;
    case ExtractLaneVecI64x2:
      o << U32LEB(BinaryConsts::I64x2ExtractLane);
      break;
    case ExtractLaneVecF16x8:
      o << U32LEB(BinaryConsts::F16x8ExtractLane);
      break;
    case ExtractLaneVecF32x4:
      o << U32LEB(BinaryConsts::F32x4ExtractLane);
      break;
    case ExtractLaneVecF64x2:
      o << U32LEB(BinaryConsts::F64x2ExtractLane);
      break;
  }
  o << uint8_t(curr->index);
}

void BinaryInstWriter::visitSIMDReplace(SIMDReplace* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix);
  switch (curr->op) {
    case ReplaceLaneVecI8x16:
      o << U32LEB(BinaryConsts::I8x16ReplaceLane);
      break;
    case ReplaceLaneVecI16x8:
      o << U32LEB(BinaryConsts::I16x8ReplaceLane);
      break;
    case ReplaceLaneVecI32x4:
      o << U32LEB(BinaryConsts::I32x4ReplaceLane);
      break;
    case ReplaceLaneVecI64x2:
      o << U32LEB(BinaryConsts::I64x2ReplaceLane);
      break;
    case ReplaceLaneVecF16x8:
      o << U32LEB(BinaryConsts::F16x8ReplaceLane);
      break;
    case ReplaceLaneVecF32x4:
      o << U32LEB(BinaryConsts::F32x4ReplaceLane);
      break;
    case ReplaceLaneVecF64x2:
      o << U32LEB(BinaryConsts::F64x2ReplaceLane);
      break;
  }
  assert(curr->index < 16);
  o << uint8_t(curr->index);
}

void BinaryInstWriter::visitSIMDShuffle(SIMDShuffle* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Shuffle);
  for (uint8_t m : curr->mask) {
    o << m;
  }
}

void BinaryInstWriter::visitSIMDTernary(SIMDTernary* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix);
  switch (curr->op) {
    case Bitselect:
      o << U32LEB(BinaryConsts::V128Bitselect);
      break;
    case LaneselectI8x16:
      o << U32LEB(BinaryConsts::I8x16Laneselect);
      break;
    case LaneselectI16x8:
      o << U32LEB(BinaryConsts::I16x8Laneselect);
      break;
    case LaneselectI32x4:
      o << U32LEB(BinaryConsts::I32x4Laneselect);
      break;
    case LaneselectI64x2:
      o << U32LEB(BinaryConsts::I64x2Laneselect);
      break;
    case RelaxedMaddVecF16x8:
      o << U32LEB(BinaryConsts::F16x8RelaxedMadd);
      break;
    case RelaxedNmaddVecF16x8:
      o << U32LEB(BinaryConsts::F16x8RelaxedNmadd);
      break;
    case RelaxedMaddVecF32x4:
      o << U32LEB(BinaryConsts::F32x4RelaxedMadd);
      break;
    case RelaxedNmaddVecF32x4:
      o << U32LEB(BinaryConsts::F32x4RelaxedNmadd);
      break;
    case RelaxedMaddVecF64x2:
      o << U32LEB(BinaryConsts::F64x2RelaxedMadd);
      break;
    case RelaxedNmaddVecF64x2:
      o << U32LEB(BinaryConsts::F64x2RelaxedNmadd);
      break;
    case DotI8x16I7x16AddSToVecI32x4:
      o << U32LEB(BinaryConsts::I32x4DotI8x16I7x16AddS);
      break;
  }
}

void BinaryInstWriter::visitSIMDShift(SIMDShift* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix);
  switch (curr->op) {
    case ShlVecI8x16:
      o << U32LEB(BinaryConsts::I8x16Shl);
      break;
    case ShrSVecI8x16:
      o << U32LEB(BinaryConsts::I8x16ShrS);
      break;
    case ShrUVecI8x16:
      o << U32LEB(BinaryConsts::I8x16ShrU);
      break;
    case ShlVecI16x8:
      o << U32LEB(BinaryConsts::I16x8Shl);
      break;
    case ShrSVecI16x8:
      o << U32LEB(BinaryConsts::I16x8ShrS);
      break;
    case ShrUVecI16x8:
      o << U32LEB(BinaryConsts::I16x8ShrU);
      break;
    case ShlVecI32x4:
      o << U32LEB(BinaryConsts::I32x4Shl);
      break;
    case ShrSVecI32x4:
      o << U32LEB(BinaryConsts::I32x4ShrS);
      break;
    case ShrUVecI32x4:
      o << U32LEB(BinaryConsts::I32x4ShrU);
      break;
    case ShlVecI64x2:
      o << U32LEB(BinaryConsts::I64x2Shl);
      break;
    case ShrSVecI64x2:
      o << U32LEB(BinaryConsts::I64x2ShrS);
      break;
    case ShrUVecI64x2:
      o << U32LEB(BinaryConsts::I64x2ShrU);
      break;
  }
}

void BinaryInstWriter::visitSIMDLoad(SIMDLoad* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix);
  switch (curr->op) {
    case Load8SplatVec128:
      o << U32LEB(BinaryConsts::V128Load8Splat);
      break;
    case Load16SplatVec128:
      o << U32LEB(BinaryConsts::V128Load16Splat);
      break;
    case Load32SplatVec128:
      o << U32LEB(BinaryConsts::V128Load32Splat);
      break;
    case Load64SplatVec128:
      o << U32LEB(BinaryConsts::V128Load64Splat);
      break;
    case Load8x8SVec128:
      o << U32LEB(BinaryConsts::V128Load8x8S);
      break;
    case Load8x8UVec128:
      o << U32LEB(BinaryConsts::V128Load8x8U);
      break;
    case Load16x4SVec128:
      o << U32LEB(BinaryConsts::V128Load16x4S);
      break;
    case Load16x4UVec128:
      o << U32LEB(BinaryConsts::V128Load16x4U);
      break;
    case Load32x2SVec128:
      o << U32LEB(BinaryConsts::V128Load32x2S);
      break;
    case Load32x2UVec128:
      o << U32LEB(BinaryConsts::V128Load32x2U);
      break;
    case Load32ZeroVec128:
      o << U32LEB(BinaryConsts::V128Load32Zero);
      break;
    case Load64ZeroVec128:
      o << U32LEB(BinaryConsts::V128Load64Zero);
      break;
  }
  assert(curr->align);
  emitMemoryAccess(
    curr->align, /*(unused) bytes=*/0, curr->offset, curr->memory);
}

void BinaryInstWriter::visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
  o << int8_t(BinaryConsts::SIMDPrefix);
  switch (curr->op) {
    case Load8LaneVec128:
      o << U32LEB(BinaryConsts::V128Load8Lane);
      break;
    case Load16LaneVec128:
      o << U32LEB(BinaryConsts::V128Load16Lane);
      break;
    case Load32LaneVec128:
      o << U32LEB(BinaryConsts::V128Load32Lane);
      break;
    case Load64LaneVec128:
      o << U32LEB(BinaryConsts::V128Load64Lane);
      break;
    case Store8LaneVec128:
      o << U32LEB(BinaryConsts::V128Store8Lane);
      break;
    case Store16LaneVec128:
      o << U32LEB(BinaryConsts::V128Store16Lane);
      break;
    case Store32LaneVec128:
      o << U32LEB(BinaryConsts::V128Store32Lane);
      break;
    case Store64LaneVec128:
      o << U32LEB(BinaryConsts::V128Store64Lane);
      break;
  }
  assert(curr->align);
  emitMemoryAccess(
    curr->align, /*(unused) bytes=*/0, curr->offset, curr->memory);
  o << curr->index;
}

void BinaryInstWriter::visitMemoryInit(MemoryInit* curr) {
  o << int8_t(BinaryConsts::MiscPrefix);
  o << U32LEB(BinaryConsts::MemoryInit);
  o << U32LEB(parent.getDataSegmentIndex(curr->segment));
  o << U32LEB(parent.getMemoryIndex(curr->memory));
}

void BinaryInstWriter::visitDataDrop(DataDrop* curr) {
  o << int8_t(BinaryConsts::MiscPrefix);
  o << U32LEB(BinaryConsts::DataDrop);
  o << U32LEB(parent.getDataSegmentIndex(curr->segment));
}

void BinaryInstWriter::visitMemoryCopy(MemoryCopy* curr) {
  o << int8_t(BinaryConsts::MiscPrefix);
  o << U32LEB(BinaryConsts::MemoryCopy);
  o << U32LEB(parent.getMemoryIndex(curr->destMemory));
  o << U32LEB(parent.getMemoryIndex(curr->sourceMemory));
}

void BinaryInstWriter::visitMemoryFill(MemoryFill* curr) {
  o << int8_t(BinaryConsts::MiscPrefix);
  o << U32LEB(BinaryConsts::MemoryFill);
  o << U32LEB(parent.getMemoryIndex(curr->memory));
}

void BinaryInstWriter::visitConst(Const* curr) {
  switch (curr->type.getBasic()) {
    case Type::i32: {
      o << int8_t(BinaryConsts::I32Const) << S32LEB(curr->value.geti32());
      break;
    }
    case Type::i64: {
      o << int8_t(BinaryConsts::I64Const) << S64LEB(curr->value.geti64());
      break;
    }
    case Type::f32: {
      o << int8_t(BinaryConsts::F32Const) << curr->value.reinterpreti32();
      break;
    }
    case Type::f64: {
      o << int8_t(BinaryConsts::F64Const) << curr->value.reinterpreti64();
      break;
    }
    case Type::v128: {
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128Const);
      std::array<uint8_t, 16> v = curr->value.getv128();
      for (size_t i = 0; i < 16; ++i) {
        o << uint8_t(v[i]);
      }
      break;
    }
    case Type::none:
    case Type::unreachable:
      WASM_UNREACHABLE("unexpected type");
  }
}

void BinaryInstWriter::visitUnary(Unary* curr) {
  switch (curr->op) {
    case ClzInt32:
      o << int8_t(BinaryConsts::I32Clz);
      break;
    case CtzInt32:
      o << int8_t(BinaryConsts::I32Ctz);
      break;
    case PopcntInt32:
      o << int8_t(BinaryConsts::I32Popcnt);
      break;
    case EqZInt32:
      o << int8_t(BinaryConsts::I32EqZ);
      break;
    case ClzInt64:
      o << int8_t(BinaryConsts::I64Clz);
      break;
    case CtzInt64:
      o << int8_t(BinaryConsts::I64Ctz);
      break;
    case PopcntInt64:
      o << int8_t(BinaryConsts::I64Popcnt);
      break;
    case EqZInt64:
      o << int8_t(BinaryConsts::I64EqZ);
      break;
    case NegFloat32:
      o << int8_t(BinaryConsts::F32Neg);
      break;
    case AbsFloat32:
      o << int8_t(BinaryConsts::F32Abs);
      break;
    case CeilFloat32:
      o << int8_t(BinaryConsts::F32Ceil);
      break;
    case FloorFloat32:
      o << int8_t(BinaryConsts::F32Floor);
      break;
    case TruncFloat32:
      o << int8_t(BinaryConsts::F32Trunc);
      break;
    case NearestFloat32:
      o << int8_t(BinaryConsts::F32Nearest);
      break;
    case SqrtFloat32:
      o << int8_t(BinaryConsts::F32Sqrt);
      break;
    case NegFloat64:
      o << int8_t(BinaryConsts::F64Neg);
      break;
    case AbsFloat64:
      o << int8_t(BinaryConsts::F64Abs);
      break;
    case CeilFloat64:
      o << int8_t(BinaryConsts::F64Ceil);
      break;
    case FloorFloat64:
      o << int8_t(BinaryConsts::F64Floor);
      break;
    case TruncFloat64:
      o << int8_t(BinaryConsts::F64Trunc);
      break;
    case NearestFloat64:
      o << int8_t(BinaryConsts::F64Nearest);
      break;
    case SqrtFloat64:
      o << int8_t(BinaryConsts::F64Sqrt);
      break;
    case ExtendSInt32:
      o << int8_t(BinaryConsts::I64SExtendI32);
      break;
    case ExtendUInt32:
      o << int8_t(BinaryConsts::I64UExtendI32);
      break;
    case WrapInt64:
      o << int8_t(BinaryConsts::I32WrapI64);
      break;
    case TruncUFloat32ToInt32:
      o << int8_t(BinaryConsts::I32UTruncF32);
      break;
    case TruncUFloat32ToInt64:
      o << int8_t(BinaryConsts::I64UTruncF32);
      break;
    case TruncSFloat32ToInt32:
      o << int8_t(BinaryConsts::I32STruncF32);
      break;
    case TruncSFloat32ToInt64:
      o << int8_t(BinaryConsts::I64STruncF32);
      break;
    case TruncUFloat64ToInt32:
      o << int8_t(BinaryConsts::I32UTruncF64);
      break;
    case TruncUFloat64ToInt64:
      o << int8_t(BinaryConsts::I64UTruncF64);
      break;
    case TruncSFloat64ToInt32:
      o << int8_t(BinaryConsts::I32STruncF64);
      break;
    case TruncSFloat64ToInt64:
      o << int8_t(BinaryConsts::I64STruncF64);
      break;
    case ConvertUInt32ToFloat32:
      o << int8_t(BinaryConsts::F32UConvertI32);
      break;
    case ConvertUInt32ToFloat64:
      o << int8_t(BinaryConsts::F64UConvertI32);
      break;
    case ConvertSInt32ToFloat32:
      o << int8_t(BinaryConsts::F32SConvertI32);
      break;
    case ConvertSInt32ToFloat64:
      o << int8_t(BinaryConsts::F64SConvertI32);
      break;
    case ConvertUInt64ToFloat32:
      o << int8_t(BinaryConsts::F32UConvertI64);
      break;
    case ConvertUInt64ToFloat64:
      o << int8_t(BinaryConsts::F64UConvertI64);
      break;
    case ConvertSInt64ToFloat32:
      o << int8_t(BinaryConsts::F32SConvertI64);
      break;
    case ConvertSInt64ToFloat64:
      o << int8_t(BinaryConsts::F64SConvertI64);
      break;
    case DemoteFloat64:
      o << int8_t(BinaryConsts::F32DemoteI64);
      break;
    case PromoteFloat32:
      o << int8_t(BinaryConsts::F64PromoteF32);
      break;
    case ReinterpretFloat32:
      o << int8_t(BinaryConsts::I32ReinterpretF32);
      break;
    case ReinterpretFloat64:
      o << int8_t(BinaryConsts::I64ReinterpretF64);
      break;
    case ReinterpretInt32:
      o << int8_t(BinaryConsts::F32ReinterpretI32);
      break;
    case ReinterpretInt64:
      o << int8_t(BinaryConsts::F64ReinterpretI64);
      break;
    case ExtendS8Int32:
      o << int8_t(BinaryConsts::I32ExtendS8);
      break;
    case ExtendS16Int32:
      o << int8_t(BinaryConsts::I32ExtendS16);
      break;
    case ExtendS8Int64:
      o << int8_t(BinaryConsts::I64ExtendS8);
      break;
    case ExtendS16Int64:
      o << int8_t(BinaryConsts::I64ExtendS16);
      break;
    case ExtendS32Int64:
      o << int8_t(BinaryConsts::I64ExtendS32);
      break;
    case TruncSatSFloat32ToInt32:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I32STruncSatF32);
      break;
    case TruncSatUFloat32ToInt32:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I32UTruncSatF32);
      break;
    case TruncSatSFloat64ToInt32:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I32STruncSatF64);
      break;
    case TruncSatUFloat64ToInt32:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I32UTruncSatF64);
      break;
    case TruncSatSFloat32ToInt64:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I64STruncSatF32);
      break;
    case TruncSatUFloat32ToInt64:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I64UTruncSatF32);
      break;
    case TruncSatSFloat64ToInt64:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I64STruncSatF64);
      break;
    case TruncSatUFloat64ToInt64:
      o << int8_t(BinaryConsts::MiscPrefix)
        << U32LEB(BinaryConsts::I64UTruncSatF64);
      break;
    case SplatVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Splat);
      break;
    case SplatVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Splat);
      break;
    case SplatVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Splat);
      break;
    case SplatVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Splat);
      break;
    case SplatVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Splat);
      break;
    case SplatVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Splat);
      break;
    case SplatVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Splat);
      break;
    case NotVec128:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128Not);
      break;
    case AnyTrueVec128:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::V128AnyTrue);
      break;
    case AbsVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Abs);
      break;
    case NegVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Neg);
      break;
    case AllTrueVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16AllTrue);
      break;
    case BitmaskVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16Bitmask);
      break;
    case PopcntVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16Popcnt);
      break;
    case AbsVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Abs);
      break;
    case NegVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Neg);
      break;
    case AllTrueVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8AllTrue);
      break;
    case BitmaskVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8Bitmask);
      break;
    case AbsVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Abs);
      break;
    case NegVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Neg);
      break;
    case AllTrueVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4AllTrue);
      break;
    case BitmaskVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4Bitmask);
      break;
    case AbsVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Abs);
      break;
    case NegVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Neg);
      break;
    case AllTrueVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2AllTrue);
      break;
    case BitmaskVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2Bitmask);
      break;
    case AbsVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Abs);
      break;
    case NegVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Neg);
      break;
    case SqrtVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Sqrt);
      break;
    case CeilVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Ceil);
      break;
    case FloorVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Floor);
      break;
    case TruncVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Trunc);
      break;
    case NearestVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F16x8Nearest);
      break;
    case AbsVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Abs);
      break;
    case NegVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Neg);
      break;
    case SqrtVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Sqrt);
      break;
    case CeilVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Ceil);
      break;
    case FloorVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Floor);
      break;
    case TruncVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Trunc);
      break;
    case NearestVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F32x4Nearest);
      break;
    case AbsVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Abs);
      break;
    case NegVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Neg);
      break;
    case SqrtVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Sqrt);
      break;
    case CeilVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Ceil);
      break;
    case FloorVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Floor);
      break;
    case TruncVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Trunc);
      break;
    case NearestVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F64x2Nearest);
      break;
    case ExtAddPairwiseSVecI8x16ToI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtaddPairwiseI8x16S);
      break;
    case ExtAddPairwiseUVecI8x16ToI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtaddPairwiseI8x16U);
      break;
    case ExtAddPairwiseSVecI16x8ToI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtaddPairwiseI16x8S);
      break;
    case ExtAddPairwiseUVecI16x8ToI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtaddPairwiseI16x8U);
      break;
    case TruncSatSVecF32x4ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4TruncSatF32x4S);
      break;
    case TruncSatUVecF32x4ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4TruncSatF32x4U);
      break;
    case ConvertSVecI32x4ToVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F32x4ConvertI32x4S);
      break;
    case ConvertUVecI32x4ToVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F32x4ConvertI32x4U);
      break;
    case ExtendLowSVecI8x16ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtendLowI8x16S);
      break;
    case ExtendHighSVecI8x16ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtendHighI8x16S);
      break;
    case ExtendLowUVecI8x16ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtendLowI8x16U);
      break;
    case ExtendHighUVecI8x16ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtendHighI8x16U);
      break;
    case ExtendLowSVecI16x8ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtendLowI16x8S);
      break;
    case ExtendHighSVecI16x8ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtendHighI16x8S);
      break;
    case ExtendLowUVecI16x8ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtendLowI16x8U);
      break;
    case ExtendHighUVecI16x8ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtendHighI16x8U);
      break;
    case ExtendLowSVecI32x4ToVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtendLowI32x4S);
      break;
    case ExtendHighSVecI32x4ToVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtendHighI32x4S);
      break;
    case ExtendLowUVecI32x4ToVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtendLowI32x4U);
      break;
    case ExtendHighUVecI32x4ToVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtendHighI32x4U);
      break;
    case ConvertLowSVecI32x4ToVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F64x2ConvertLowI32x4S);
      break;
    case ConvertLowUVecI32x4ToVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F64x2ConvertLowI32x4U);
      break;
    case TruncSatZeroSVecF64x2ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4TruncSatF64x2SZero);
      break;
    case TruncSatZeroUVecF64x2ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4TruncSatF64x2UZero);
      break;
    case DemoteZeroVecF64x2ToVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F32x4DemoteF64x2Zero);
      break;
    case PromoteLowVecF32x4ToVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F64x2PromoteLowF32x4);
      break;
    case RelaxedTruncSVecF32x4ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4RelaxedTruncF32x4S);
      break;
    case RelaxedTruncUVecF32x4ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4RelaxedTruncF32x4U);
      break;
    case RelaxedTruncZeroSVecF64x2ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4RelaxedTruncF64x2SZero);
      break;
    case RelaxedTruncZeroUVecF64x2ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4RelaxedTruncF64x2UZero);
      break;
    case TruncSatSVecF16x8ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8TruncSatF16x8S);
      break;
    case TruncSatUVecF16x8ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8TruncSatF16x8U);
      break;
    case ConvertSVecI16x8ToVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F16x8ConvertI16x8S);
      break;
    case ConvertUVecI16x8ToVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F16x8ConvertI16x8U);
      break;
    case InvalidUnary:
      WASM_UNREACHABLE("invalid unary op");
  }
}

void BinaryInstWriter::visitBinary(Binary* curr) {
  switch (curr->op) {
    case AddInt32:
      o << int8_t(BinaryConsts::I32Add);
      break;
    case SubInt32:
      o << int8_t(BinaryConsts::I32Sub);
      break;
    case MulInt32:
      o << int8_t(BinaryConsts::I32Mul);
      break;
    case DivSInt32:
      o << int8_t(BinaryConsts::I32DivS);
      break;
    case DivUInt32:
      o << int8_t(BinaryConsts::I32DivU);
      break;
    case RemSInt32:
      o << int8_t(BinaryConsts::I32RemS);
      break;
    case RemUInt32:
      o << int8_t(BinaryConsts::I32RemU);
      break;
    case AndInt32:
      o << int8_t(BinaryConsts::I32And);
      break;
    case OrInt32:
      o << int8_t(BinaryConsts::I32Or);
      break;
    case XorInt32:
      o << int8_t(BinaryConsts::I32Xor);
      break;
    case ShlInt32:
      o << int8_t(BinaryConsts::I32Shl);
      break;
    case ShrUInt32:
      o << int8_t(BinaryConsts::I32ShrU);
      break;
    case ShrSInt32:
      o << int8_t(BinaryConsts::I32ShrS);
      break;
    case RotLInt32:
      o << int8_t(BinaryConsts::I32RotL);
      break;
    case RotRInt32:
      o << int8_t(BinaryConsts::I32RotR);
      break;
    case EqInt32:
      o << int8_t(BinaryConsts::I32Eq);
      break;
    case NeInt32:
      o << int8_t(BinaryConsts::I32Ne);
      break;
    case LtSInt32:
      o << int8_t(BinaryConsts::I32LtS);
      break;
    case LtUInt32:
      o << int8_t(BinaryConsts::I32LtU);
      break;
    case LeSInt32:
      o << int8_t(BinaryConsts::I32LeS);
      break;
    case LeUInt32:
      o << int8_t(BinaryConsts::I32LeU);
      break;
    case GtSInt32:
      o << int8_t(BinaryConsts::I32GtS);
      break;
    case GtUInt32:
      o << int8_t(BinaryConsts::I32GtU);
      break;
    case GeSInt32:
      o << int8_t(BinaryConsts::I32GeS);
      break;
    case GeUInt32:
      o << int8_t(BinaryConsts::I32GeU);
      break;

    case AddInt64:
      o << int8_t(BinaryConsts::I64Add);
      break;
    case SubInt64:
      o << int8_t(BinaryConsts::I64Sub);
      break;
    case MulInt64:
      o << int8_t(BinaryConsts::I64Mul);
      break;
    case DivSInt64:
      o << int8_t(BinaryConsts::I64DivS);
      break;
    case DivUInt64:
      o << int8_t(BinaryConsts::I64DivU);
      break;
    case RemSInt64:
      o << int8_t(BinaryConsts::I64RemS);
      break;
    case RemUInt64:
      o << int8_t(BinaryConsts::I64RemU);
      break;
    case AndInt64:
      o << int8_t(BinaryConsts::I64And);
      break;
    case OrInt64:
      o << int8_t(BinaryConsts::I64Or);
      break;
    case XorInt64:
      o << int8_t(BinaryConsts::I64Xor);
      break;
    case ShlInt64:
      o << int8_t(BinaryConsts::I64Shl);
      break;
    case ShrUInt64:
      o << int8_t(BinaryConsts::I64ShrU);
      break;
    case ShrSInt64:
      o << int8_t(BinaryConsts::I64ShrS);
      break;
    case RotLInt64:
      o << int8_t(BinaryConsts::I64RotL);
      break;
    case RotRInt64:
      o << int8_t(BinaryConsts::I64RotR);
      break;
    case EqInt64:
      o << int8_t(BinaryConsts::I64Eq);
      break;
    case NeInt64:
      o << int8_t(BinaryConsts::I64Ne);
      break;
    case LtSInt64:
      o << int8_t(BinaryConsts::I64LtS);
      break;
    case LtUInt64:
      o << int8_t(BinaryConsts::I64LtU);
      break;
    case LeSInt64:
      o << int8_t(BinaryConsts::I64LeS);
      break;
    case LeUInt64:
      o << int8_t(BinaryConsts::I64LeU);
      break;
    case GtSInt64:
      o << int8_t(BinaryConsts::I64GtS);
      break;
    case GtUInt64:
      o << int8_t(BinaryConsts::I64GtU);
      break;
    case GeSInt64:
      o << int8_t(BinaryConsts::I64GeS);
      break;
    case GeUInt64:
      o << int8_t(BinaryConsts::I64GeU);
      break;

    case AddFloat32:
      o << int8_t(BinaryConsts::F32Add);
      break;
    case SubFloat32:
      o << int8_t(BinaryConsts::F32Sub);
      break;
    case MulFloat32:
      o << int8_t(BinaryConsts::F32Mul);
      break;
    case DivFloat32:
      o << int8_t(BinaryConsts::F32Div);
      break;
    case CopySignFloat32:
      o << int8_t(BinaryConsts::F32CopySign);
      break;
    case MinFloat32:
      o << int8_t(BinaryConsts::F32Min);
      break;
    case MaxFloat32:
      o << int8_t(BinaryConsts::F32Max);
      break;
    case EqFloat32:
      o << int8_t(BinaryConsts::F32Eq);
      break;
    case NeFloat32:
      o << int8_t(BinaryConsts::F32Ne);
      break;
    case LtFloat32:
      o << int8_t(BinaryConsts::F32Lt);
      break;
    case LeFloat32:
      o << int8_t(BinaryConsts::F32Le);
      break;
    case GtFloat32:
      o << int8_t(BinaryConsts::F32Gt);
      break;
    case GeFloat32:
      o << int8_t(BinaryConsts::F32Ge);
      break;

    case AddFloat64:
      o << int8_t(BinaryConsts::F64Add);
      break;
    case SubFloat64:
      o << int8_t(BinaryConsts::F64Sub);
      break;
    case MulFloat64:
      o << int8_t(BinaryConsts::F64Mul);
      break;
    case DivFloat64:
      o << int8_t(BinaryConsts::F64Div);
      break;
    case CopySignFloat64:
      o << int8_t(BinaryConsts::F64CopySign);
      break;
    case MinFloat64:
      o << int8_t(BinaryConsts::F64Min);
      break;
    case MaxFloat64:
      o << int8_t(BinaryConsts::F64Max);
      break;
    case EqFloat64:
      o << int8_t(BinaryConsts::F64Eq);
      break;
    case NeFloat64:
      o << int8_t(BinaryConsts::F64Ne);
      break;
    case LtFloat64:
      o << int8_t(BinaryConsts::F64Lt);
      break;
    case LeFloat64:
      o << int8_t(BinaryConsts::F64Le);
      break;
    case GtFloat64:
      o << int8_t(BinaryConsts::F64Gt);
      break;
    case GeFloat64:
      o << int8_t(BinaryConsts::F64Ge);
      break;

    case EqVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Eq);
      break;
    case NeVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Ne);
      break;
    case LtSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16LtS);
      break;
    case LtUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16LtU);
      break;
    case GtSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16GtS);
      break;
    case GtUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16GtU);
      break;
    case LeSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16LeS);
      break;
    case LeUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16LeU);
      break;
    case GeSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16GeS);
      break;
    case GeUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16GeU);
      break;
    case EqVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Eq);
      break;
    case NeVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Ne);
      break;
    case LtSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8LtS);
      break;
    case LtUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8LtU);
      break;
    case GtSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8GtS);
      break;
    case GtUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8GtU);
      break;
    case LeSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8LeS);
      break;
    case LeUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8LeU);
      break;
    case GeSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8GeS);
      break;
    case GeUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8GeU);
      break;
    case EqVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Eq);
      break;
    case NeVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Ne);
      break;
    case LtSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4LtS);
      break;
    case LtUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4LtU);
      break;
    case GtSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4GtS);
      break;
    case GtUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4GtU);
      break;
    case LeSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4LeS);
      break;
    case LeUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4LeU);
      break;
    case GeSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4GeS);
      break;
    case GeUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4GeU);
      break;
    case EqVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Eq);
      break;
    case NeVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Ne);
      break;
    case LtSVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2LtS);
      break;
    case GtSVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2GtS);
      break;
    case LeSVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2LeS);
      break;
    case GeSVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2GeS);
      break;
    case EqVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Eq);
      break;
    case NeVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Ne);
      break;
    case LtVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Lt);
      break;
    case GtVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Gt);
      break;
    case LeVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Le);
      break;
    case GeVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Ge);
      break;
    case EqVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Eq);
      break;
    case NeVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Ne);
      break;
    case LtVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Lt);
      break;
    case GtVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Gt);
      break;
    case LeVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Le);
      break;
    case GeVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Ge);
      break;
    case EqVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Eq);
      break;
    case NeVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Ne);
      break;
    case LtVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Lt);
      break;
    case GtVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Gt);
      break;
    case LeVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Le);
      break;
    case GeVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Ge);
      break;
    case AndVec128:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128And);
      break;
    case OrVec128:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128Or);
      break;
    case XorVec128:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128Xor);
      break;
    case AndNotVec128:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::V128Andnot);
      break;
    case AddVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Add);
      break;
    case AddSatSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16AddSatS);
      break;
    case AddSatUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16AddSatU);
      break;
    case SubVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16Sub);
      break;
    case SubSatSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16SubSatS);
      break;
    case SubSatUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16SubSatU);
      break;
    case MinSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16MinS);
      break;
    case MinUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16MinU);
      break;
    case MaxSVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16MaxS);
      break;
    case MaxUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16MaxU);
      break;
    case AvgrUVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I8x16AvgrU);
      break;
    case AddVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Add);
      break;
    case AddSatSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8AddSatS);
      break;
    case AddSatUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8AddSatU);
      break;
    case SubVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Sub);
      break;
    case SubSatSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8SubSatS);
      break;
    case SubSatUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8SubSatU);
      break;
    case MulVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8Mul);
      break;
    case MinSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8MinS);
      break;
    case MinUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8MinU);
      break;
    case MaxSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8MaxS);
      break;
    case MaxUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8MaxU);
      break;
    case AvgrUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I16x8AvgrU);
      break;
    case Q15MulrSatSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8Q15MulrSatS);
      break;
    case ExtMulLowSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtmulLowI8x16S);
      break;
    case ExtMulHighSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtmulHighI8x16S);
      break;
    case ExtMulLowUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtmulLowI8x16U);
      break;
    case ExtMulHighUVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8ExtmulHighI8x16U);
      break;
    case AddVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Add);
      break;
    case SubVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Sub);
      break;
    case MulVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4Mul);
      break;
    case MinSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4MinS);
      break;
    case MinUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4MinU);
      break;
    case MaxSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4MaxS);
      break;
    case MaxUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I32x4MaxU);
      break;
    case DotSVecI16x8ToVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4DotI16x8S);
      break;
    case ExtMulLowSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtmulLowI16x8S);
      break;
    case ExtMulHighSVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtmulHighI16x8S);
      break;
    case ExtMulLowUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtmulLowI16x8U);
      break;
    case ExtMulHighUVecI32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I32x4ExtmulHighI16x8U);
      break;
    case AddVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Add);
      break;
    case SubVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Sub);
      break;
    case MulVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::I64x2Mul);
      break;
    case ExtMulLowSVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtmulLowI32x4S);
      break;
    case ExtMulHighSVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtmulHighI32x4S);
      break;
    case ExtMulLowUVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtmulLowI32x4U);
      break;
    case ExtMulHighUVecI64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I64x2ExtmulHighI32x4U);
      break;

    case AddVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Add);
      break;
    case SubVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Sub);
      break;
    case MulVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Mul);
      break;
    case DivVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Div);
      break;
    case MinVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Min);
      break;
    case MaxVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Max);
      break;
    case PMinVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Pmin);
      break;
    case PMaxVecF16x8:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F16x8Pmax);
      break;
    case AddVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Add);
      break;
    case SubVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Sub);
      break;
    case MulVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Mul);
      break;
    case DivVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Div);
      break;
    case MinVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Min);
      break;
    case MaxVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Max);
      break;
    case PMinVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Pmin);
      break;
    case PMaxVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F32x4Pmax);
      break;
    case AddVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Add);
      break;
    case SubVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Sub);
      break;
    case MulVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Mul);
      break;
    case DivVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Div);
      break;
    case MinVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Min);
      break;
    case MaxVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Max);
      break;
    case PMinVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Pmin);
      break;
    case PMaxVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix) << U32LEB(BinaryConsts::F64x2Pmax);
      break;

    case NarrowSVecI16x8ToVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16NarrowI16x8S);
      break;
    case NarrowUVecI16x8ToVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16NarrowI16x8U);
      break;
    case NarrowSVecI32x4ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8NarrowI32x4S);
      break;
    case NarrowUVecI32x4ToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8NarrowI32x4U);
      break;

    case SwizzleVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16Swizzle);
      break;

    case RelaxedSwizzleVecI8x16:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I8x16RelaxedSwizzle);
      break;
    case RelaxedMinVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F32x4RelaxedMin);
      break;
    case RelaxedMaxVecF32x4:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F32x4RelaxedMax);
      break;
    case RelaxedMinVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F64x2RelaxedMin);
      break;
    case RelaxedMaxVecF64x2:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::F64x2RelaxedMax);
      break;
    case RelaxedQ15MulrSVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8RelaxedQ15MulrS);
      break;
    case DotI8x16I7x16SToVecI16x8:
      o << int8_t(BinaryConsts::SIMDPrefix)
        << U32LEB(BinaryConsts::I16x8DotI8x16I7x16S);
      break;

    case InvalidBinary:
      WASM_UNREACHABLE("invalid binary op");
  }
}

void BinaryInstWriter::visitSelect(Select* curr) {
  if (curr->type.isRef()) {
    o << int8_t(BinaryConsts::SelectWithType) << U32LEB(curr->type.size());
    for (size_t i = 0; i < curr->type.size(); i++) {
      parent.writeType(curr->type != Type::unreachable ? curr->type
                                                       : Type::none);
    }
  } else {
    o << int8_t(BinaryConsts::Select);
  }
}

void BinaryInstWriter::visitReturn(Return* curr) {
  o << int8_t(BinaryConsts::Return);
}

void BinaryInstWriter::visitMemorySize(MemorySize* curr) {
  o << int8_t(BinaryConsts::MemorySize);
  o << U32LEB(parent.getMemoryIndex(curr->memory));
}

void BinaryInstWriter::visitMemoryGrow(MemoryGrow* curr) {
  o << int8_t(BinaryConsts::MemoryGrow);
  o << U32LEB(parent.getMemoryIndex(curr->memory));
}

void BinaryInstWriter::visitRefNull(RefNull* curr) {
  o << int8_t(BinaryConsts::RefNull);
  parent.writeHeapType(curr->type.getHeapType());
}

void BinaryInstWriter::visitRefIsNull(RefIsNull* curr) {
  o << int8_t(BinaryConsts::RefIsNull);
}

void BinaryInstWriter::visitRefFunc(RefFunc* curr) {
  o << int8_t(BinaryConsts::RefFunc)
    << U32LEB(parent.getFunctionIndex(curr->func));
}

void BinaryInstWriter::visitRefEq(RefEq* curr) {
  o << int8_t(BinaryConsts::RefEq);
}

void BinaryInstWriter::visitTableGet(TableGet* curr) {
  o << int8_t(BinaryConsts::TableGet);
  o << U32LEB(parent.getTableIndex(curr->table));
}

void BinaryInstWriter::visitTableSet(TableSet* curr) {
  o << int8_t(BinaryConsts::TableSet);
  o << U32LEB(parent.getTableIndex(curr->table));
}

void BinaryInstWriter::visitTableSize(TableSize* curr) {
  o << int8_t(BinaryConsts::MiscPrefix) << U32LEB(BinaryConsts::TableSize);
  o << U32LEB(parent.getTableIndex(curr->table));
}

void BinaryInstWriter::visitTableGrow(TableGrow* curr) {
  o << int8_t(BinaryConsts::MiscPrefix) << U32LEB(BinaryConsts::TableGrow);
  o << U32LEB(parent.getTableIndex(curr->table));
}

void BinaryInstWriter::visitTableFill(TableFill* curr) {
  o << int8_t(BinaryConsts::MiscPrefix) << U32LEB(BinaryConsts::TableFill);
  o << U32LEB(parent.getTableIndex(curr->table));
}

void BinaryInstWriter::visitTableCopy(TableCopy* curr) {
  o << int8_t(BinaryConsts::MiscPrefix) << U32LEB(BinaryConsts::TableCopy);
  o << U32LEB(parent.getTableIndex(curr->destTable));
  o << U32LEB(parent.getTableIndex(curr->sourceTable));
}

void BinaryInstWriter::visitTableInit(TableInit* curr) {
  o << int8_t(BinaryConsts::MiscPrefix) << U32LEB(BinaryConsts::TableInit);
  o << U32LEB(parent.getElementSegmentIndex(curr->segment));
  o << U32LEB(parent.getTableIndex(curr->table));
}

void BinaryInstWriter::visitTry(Try* curr) {
  breakStack.push_back(curr->name);
  o << int8_t(BinaryConsts::Try);
  emitResultType(curr->type);
}

void BinaryInstWriter::visitTryTable(TryTable* curr) {
  o << int8_t(BinaryConsts::TryTable);
  emitResultType(curr->type);
  o << U32LEB(curr->catchTags.size());
  for (Index i = 0; i < curr->catchTags.size(); i++) {
    if (curr->catchTags[i]) {
      o << (curr->catchRefs[i] ? int8_t(BinaryConsts::CatchRef)
                               : int8_t(BinaryConsts::Catch));
      o << U32LEB(parent.getTagIndex(curr->catchTags[i]));
    } else {
      o << (curr->catchRefs[i] ? int8_t(BinaryConsts::CatchAllRef)
                               : int8_t(BinaryConsts::CatchAll));
    }
    o << U32LEB(getBreakIndex(curr->catchDests[i]));
  }
  // the binary format requires this; we have a block if we need one
  // catch_*** clauses should refer to block labels without entering the try
  // scope. So we do this at the end.
  breakStack.emplace_back(IMPOSSIBLE_CONTINUE);
}

void BinaryInstWriter::emitCatch(Try* curr, Index i) {
  if (func && !sourceMap) {
    parent.writeExtraDebugLocation(curr, func, i);
  }
  o << int8_t(BinaryConsts::Catch_Legacy)
    << U32LEB(parent.getTagIndex(curr->catchTags[i]));
}

void BinaryInstWriter::emitCatchAll(Try* curr) {
  if (func && !sourceMap) {
    parent.writeExtraDebugLocation(curr, func, curr->catchBodies.size());
  }
  o << int8_t(BinaryConsts::CatchAll_Legacy);
}

void BinaryInstWriter::emitDelegate(Try* curr) {
  // The delegate ends the scope in effect, and pops the try's name. Note that
  // the getBreakIndex is intentionally after that pop, as the delegate cannot
  // target its own try.
  assert(!breakStack.empty());
  breakStack.pop_back();
  o << int8_t(BinaryConsts::Delegate)
    << U32LEB(getBreakIndex(curr->delegateTarget));
}

void BinaryInstWriter::visitThrow(Throw* curr) {
  o << int8_t(BinaryConsts::Throw) << U32LEB(parent.getTagIndex(curr->tag));
}

void BinaryInstWriter::visitRethrow(Rethrow* curr) {
  o << int8_t(BinaryConsts::Rethrow) << U32LEB(getBreakIndex(curr->target));
}

void BinaryInstWriter::visitThrowRef(ThrowRef* curr) {
  o << int8_t(BinaryConsts::ThrowRef);
}

void BinaryInstWriter::visitNop(Nop* curr) { o << int8_t(BinaryConsts::Nop); }

void BinaryInstWriter::visitUnreachable(Unreachable* curr) {
  o << int8_t(BinaryConsts::Unreachable);
}

void BinaryInstWriter::visitDrop(Drop* curr) {
  size_t numValues = curr->value->type.size();
  for (size_t i = 0; i < numValues; i++) {
    o << int8_t(BinaryConsts::Drop);
  }
}

void BinaryInstWriter::visitPop(Pop* curr) {
  // Turns into nothing in the binary format
}

void BinaryInstWriter::visitTupleMake(TupleMake* curr) {
  // Turns into nothing in the binary format
}

void BinaryInstWriter::visitTupleExtract(TupleExtract* curr) {
  if (extractedGets.count(curr->tuple)) {
    // We already have just the extracted value on the stack.
    return;
  }
  size_t numVals = curr->tuple->type.size();
  // Drop all values after the one we want
  for (size_t i = curr->index + 1; i < numVals; ++i) {
    o << int8_t(BinaryConsts::Drop);
  }
  // If the extracted value is the only one left, we're done
  if (curr->index == 0) {
    return;
  }
  // Otherwise, save it to a scratch local, drop the others, then retrieve it
  assert(scratchLocals.find(curr->type) != scratchLocals.end());
  auto scratch = scratchLocals[curr->type];
  o << int8_t(BinaryConsts::LocalSet) << U32LEB(scratch);
  for (size_t i = 0; i < curr->index; ++i) {
    o << int8_t(BinaryConsts::Drop);
  }
  o << int8_t(BinaryConsts::LocalGet) << U32LEB(scratch);
}

void BinaryInstWriter::visitRefI31(RefI31* curr) {
  o << int8_t(BinaryConsts::GCPrefix)
    << U32LEB(curr->type.getHeapType().isShared() ? BinaryConsts::RefI31Shared
                                                  : BinaryConsts::RefI31);
}

void BinaryInstWriter::visitI31Get(I31Get* curr) {
  o << int8_t(BinaryConsts::GCPrefix)
    << U32LEB(curr->signed_ ? BinaryConsts::I31GetS : BinaryConsts::I31GetU);
}

void BinaryInstWriter::visitCallRef(CallRef* curr) {
  assert(curr->target->type != Type::unreachable);
  if (curr->target->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(curr->isReturn ? BinaryConsts::RetCallRef
                             : BinaryConsts::CallRef);
  parent.writeIndexedHeapType(curr->target->type.getHeapType());
}

void BinaryInstWriter::visitRefTest(RefTest* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  if (curr->castType.isNullable()) {
    o << U32LEB(BinaryConsts::RefTestNull);
  } else {
    o << U32LEB(BinaryConsts::RefTest);
  }
  parent.writeHeapType(curr->castType.getHeapType());
}

void BinaryInstWriter::visitRefCast(RefCast* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  if (curr->type.isNullable()) {
    o << U32LEB(BinaryConsts::RefCastNull);
  } else {
    o << U32LEB(BinaryConsts::RefCast);
  }
  parent.writeHeapType(curr->type.getHeapType());
}

void BinaryInstWriter::visitBrOn(BrOn* curr) {
  switch (curr->op) {
    case BrOnNull:
      o << int8_t(BinaryConsts::BrOnNull);
      o << U32LEB(getBreakIndex(curr->name));
      return;
    case BrOnNonNull:
      o << int8_t(BinaryConsts::BrOnNonNull);
      o << U32LEB(getBreakIndex(curr->name));
      return;
    case BrOnCast:
    case BrOnCastFail: {
      o << int8_t(BinaryConsts::GCPrefix);
      if (curr->op == BrOnCast) {
        o << U32LEB(BinaryConsts::BrOnCast);
      } else {
        o << U32LEB(BinaryConsts::BrOnCastFail);
      }
      assert(curr->ref->type.isRef());
      assert(Type::isSubType(curr->castType, curr->ref->type));
      uint8_t flags = (curr->ref->type.isNullable() ? 1 : 0) |
                      (curr->castType.isNullable() ? 2 : 0);
      o << flags;
      o << U32LEB(getBreakIndex(curr->name));
      parent.writeHeapType(curr->ref->type.getHeapType());
      parent.writeHeapType(curr->castType.getHeapType());
      return;
    }
  }
  WASM_UNREACHABLE("invalid br_on_*");
}

void BinaryInstWriter::visitStructNew(StructNew* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  if (curr->isWithDefault()) {
    o << U32LEB(BinaryConsts::StructNewDefault);
  } else {
    o << U32LEB(BinaryConsts::StructNew);
  }
  parent.writeIndexedHeapType(curr->type.getHeapType());
}

void BinaryInstWriter::visitStructGet(StructGet* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  const auto& heapType = curr->ref->type.getHeapType();
  const auto& field = heapType.getStruct().fields[curr->index];
  int8_t op;
  if (field.type != Type::i32 || field.packedType == Field::not_packed) {
    op = BinaryConsts::StructGet;
  } else if (curr->signed_) {
    op = BinaryConsts::StructGetS;
  } else {
    op = BinaryConsts::StructGetU;
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(op);
  parent.writeIndexedHeapType(heapType);
  o << U32LEB(curr->index);
}

void BinaryInstWriter::visitStructSet(StructSet* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::StructSet);
  parent.writeIndexedHeapType(curr->ref->type.getHeapType());
  o << U32LEB(curr->index);
}

void BinaryInstWriter::visitArrayNew(ArrayNew* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  if (curr->isWithDefault()) {
    o << U32LEB(BinaryConsts::ArrayNewDefault);
  } else {
    o << U32LEB(BinaryConsts::ArrayNew);
  }
  parent.writeIndexedHeapType(curr->type.getHeapType());
}

void BinaryInstWriter::visitArrayNewData(ArrayNewData* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  o << U32LEB(BinaryConsts::ArrayNewData);
  parent.writeIndexedHeapType(curr->type.getHeapType());
  o << U32LEB(parent.getDataSegmentIndex(curr->segment));
}

void BinaryInstWriter::visitArrayNewElem(ArrayNewElem* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  o << U32LEB(BinaryConsts::ArrayNewElem);
  parent.writeIndexedHeapType(curr->type.getHeapType());
  o << U32LEB(parent.getElementSegmentIndex(curr->segment));
}

void BinaryInstWriter::visitArrayNewFixed(ArrayNewFixed* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  o << U32LEB(BinaryConsts::ArrayNewFixed);
  parent.writeIndexedHeapType(curr->type.getHeapType());
  o << U32LEB(curr->values.size());
}

void BinaryInstWriter::visitArrayGet(ArrayGet* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  auto heapType = curr->ref->type.getHeapType();
  const auto& field = heapType.getArray().element;
  int8_t op;
  if (field.type != Type::i32 || field.packedType == Field::not_packed) {
    op = BinaryConsts::ArrayGet;
  } else if (curr->signed_) {
    op = BinaryConsts::ArrayGetS;
  } else {
    op = BinaryConsts::ArrayGetU;
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(op);
  parent.writeIndexedHeapType(heapType);
}

void BinaryInstWriter::visitArraySet(ArraySet* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::ArraySet);
  parent.writeIndexedHeapType(curr->ref->type.getHeapType());
}

void BinaryInstWriter::visitArrayLen(ArrayLen* curr) {
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::ArrayLen);
}

void BinaryInstWriter::visitArrayCopy(ArrayCopy* curr) {
  if (curr->srcRef->type.isNull() || curr->destRef->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::ArrayCopy);
  parent.writeIndexedHeapType(curr->destRef->type.getHeapType());
  parent.writeIndexedHeapType(curr->srcRef->type.getHeapType());
}

void BinaryInstWriter::visitArrayFill(ArrayFill* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::ArrayFill);
  parent.writeIndexedHeapType(curr->ref->type.getHeapType());
}

void BinaryInstWriter::visitArrayInitData(ArrayInitData* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix);
  o << U32LEB(BinaryConsts::ArrayInitData);
  parent.writeIndexedHeapType(curr->ref->type.getHeapType());
  o << U32LEB(parent.getDataSegmentIndex(curr->segment));
}

void BinaryInstWriter::visitArrayInitElem(ArrayInitElem* curr) {
  if (curr->ref->type.isNull()) {
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix);
  o << U32LEB(BinaryConsts::ArrayInitElem);
  parent.writeIndexedHeapType(curr->ref->type.getHeapType());
  o << U32LEB(parent.getElementSegmentIndex(curr->segment));
}

void BinaryInstWriter::visitRefAs(RefAs* curr) {
  switch (curr->op) {
    case RefAsNonNull:
      o << int8_t(BinaryConsts::RefAsNonNull);
      break;
    case AnyConvertExtern:
      o << int8_t(BinaryConsts::GCPrefix)
        << U32LEB(BinaryConsts::AnyConvertExtern);
      break;
    case ExternConvertAny:
      o << int8_t(BinaryConsts::GCPrefix)
        << U32LEB(BinaryConsts::ExternConvertAny);
      break;
    default:
      WASM_UNREACHABLE("invalid ref.as_*");
  }
}

void BinaryInstWriter::visitStringNew(StringNew* curr) {
  if (curr->ref->type.isNull()) {
    // This is a bottom type, so this is an array-receiving operation that does
    // not receive an array. The spec allows this, but V8 does not, see
    // https://github.com/WebAssembly/stringref/issues/66
    // For now, just emit an unreachable here as this will definitely trap.
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix);
  switch (curr->op) {
    case StringNewLossyUTF8Array:
      o << U32LEB(BinaryConsts::StringNewLossyUTF8Array);
      break;
    case StringNewWTF16Array:
      o << U32LEB(BinaryConsts::StringNewWTF16Array);
      break;
    case StringNewFromCodePoint:
      o << U32LEB(BinaryConsts::StringFromCodePoint);
      break;
    default:
      WASM_UNREACHABLE("invalid string.new*");
  }
}

void BinaryInstWriter::visitStringConst(StringConst* curr) {
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::StringConst)
    << U32LEB(parent.getStringIndex(curr->string));
}

void BinaryInstWriter::visitStringMeasure(StringMeasure* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  switch (curr->op) {
    case StringMeasureUTF8:
      o << U32LEB(BinaryConsts::StringMeasureUTF8);
      break;
    case StringMeasureWTF16:
      o << U32LEB(BinaryConsts::StringMeasureWTF16);
      break;
    default:
      WASM_UNREACHABLE("invalid string.new*");
  }
}

void BinaryInstWriter::visitStringEncode(StringEncode* curr) {
  if (curr->str->type.isNull()) {
    // See visitStringNew.
    emitUnreachable();
    return;
  }
  o << int8_t(BinaryConsts::GCPrefix);
  switch (curr->op) {
    case StringEncodeLossyUTF8Array:
      o << U32LEB(BinaryConsts::StringEncodeLossyUTF8Array);
      break;
    case StringEncodeWTF16Array:
      o << U32LEB(BinaryConsts::StringEncodeWTF16Array);
      break;
    default:
      WASM_UNREACHABLE("invalid string.new*");
  }
}

void BinaryInstWriter::visitStringConcat(StringConcat* curr) {
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::StringConcat);
}

void BinaryInstWriter::visitStringEq(StringEq* curr) {
  o << int8_t(BinaryConsts::GCPrefix);
  switch (curr->op) {
    case StringEqEqual:
      o << U32LEB(BinaryConsts::StringEq);
      break;
    case StringEqCompare:
      o << U32LEB(BinaryConsts::StringCompare);
      break;
    default:
      WASM_UNREACHABLE("invalid string.eq*");
  }
}

void BinaryInstWriter::visitStringWTF16Get(StringWTF16Get* curr) {
  // We need to convert the ref operand to a stringview, but it is under the pos
  // operand. Put the i32 in a scratch local, emit the conversion, then get the
  // i32 back onto the stack. If `pos` is a local.get anyway, then we can skip
  // the scratch local.
  bool posDeferred = false;
  Index posIndex;
  if (auto* get = curr->pos->dynCast<LocalGet>()) {
    assert(deferredGets.count(get));
    posDeferred = true;
    posIndex = mappedLocals[{get->index, 0}];
  } else {
    posIndex = scratchLocals[Type::i32];
  }

  if (!posDeferred) {
    o << int8_t(BinaryConsts::LocalSet) << U32LEB(posIndex);
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::StringAsWTF16);
  o << int8_t(BinaryConsts::LocalGet) << U32LEB(posIndex);
  o << int8_t(BinaryConsts::GCPrefix)
    << U32LEB(BinaryConsts::StringViewWTF16GetCodePoint);
}

void BinaryInstWriter::visitStringSliceWTF(StringSliceWTF* curr) {
  // We need to convert the ref operand to a stringview, but it is buried under
  // the start and end operands. Put the i32s in scratch locals, emit the
  // conversion, then get the i32s back onto the stack. If both `start` and
  // `end` are already local.gets, then we can skip the scratch locals.
  bool deferred = false;
  Index startIndex, endIndex;
  auto* startGet = curr->start->dynCast<LocalGet>();
  auto* endGet = curr->end->dynCast<LocalGet>();
  if (startGet && endGet) {
    assert(deferredGets.count(startGet));
    assert(deferredGets.count(endGet));
    deferred = true;
    startIndex = mappedLocals[{startGet->index, 0}];
    endIndex = mappedLocals[{endGet->index, 0}];
  } else {
    startIndex = scratchLocals[Type::i32];
    endIndex = startIndex + 1;
  }

  if (!deferred) {
    o << int8_t(BinaryConsts::LocalSet) << U32LEB(endIndex);
    o << int8_t(BinaryConsts::LocalSet) << U32LEB(startIndex);
  }
  o << int8_t(BinaryConsts::GCPrefix) << U32LEB(BinaryConsts::StringAsWTF16);
  o << int8_t(BinaryConsts::LocalGet) << U32LEB(startIndex);
  o << int8_t(BinaryConsts::LocalGet) << U32LEB(endIndex);
  o << int8_t(BinaryConsts::GCPrefix)
    << U32LEB(BinaryConsts::StringViewWTF16Slice);
}

void BinaryInstWriter::visitContNew(ContNew* curr) {
  o << int8_t(BinaryConsts::ContNew);
  parent.writeIndexedHeapType(curr->type.getHeapType());
}

void BinaryInstWriter::visitSuspend(Suspend* curr) {
  o << int8_t(BinaryConsts::Suspend) << U32LEB(parent.getTagIndex(curr->tag));
}

void BinaryInstWriter::visitContBind(ContBind* curr) {
  assert(curr->cont->type.isContinuation() && curr->type.isContinuation());
  o << int8_t(BinaryConsts::ContBind);
  parent.writeIndexedHeapType(curr->cont->type.getHeapType());
  parent.writeIndexedHeapType(curr->type.getHeapType());
}

void BinaryInstWriter::visitResume(Resume* curr) {
  assert(curr->cont->type.isContinuation());
  o << int8_t(BinaryConsts::Resume);
  parent.writeIndexedHeapType(curr->cont->type.getHeapType());

  size_t handlerNum = curr->handlerTags.size();
  o << U32LEB(handlerNum);
  for (size_t i = 0; i < handlerNum; i++) {
    if (curr->handlerBlocks[i].isNull()) {
      // on switch
      o << int8_t(BinaryConsts::OnSwitch)
        << U32LEB(parent.getTagIndex(curr->handlerTags[i]));
    } else {
      // on label
      o << int8_t(BinaryConsts::OnLabel)
        << U32LEB(parent.getTagIndex(curr->handlerTags[i]))
        << U32LEB(getBreakIndex(curr->handlerBlocks[i]));
    }
  }
}

void BinaryInstWriter::visitResumeThrow(ResumeThrow* curr) {
  assert(curr->cont->type.isContinuation());
  o << int8_t(BinaryConsts::ResumeThrow);
  parent.writeIndexedHeapType(curr->cont->type.getHeapType());
  o << U32LEB(parent.getTagIndex(curr->tag));

  size_t handlerNum = curr->handlerTags.size();
  o << U32LEB(handlerNum);
  for (size_t i = 0; i < handlerNum; i++) {
    if (curr->handlerBlocks[i].isNull()) {
      // on switch
      o << int8_t(BinaryConsts::OnSwitch)
        << U32LEB(parent.getTagIndex(curr->handlerTags[i]));
    } else {
      // on label
      o << int8_t(BinaryConsts::OnLabel)
        << U32LEB(parent.getTagIndex(curr->handlerTags[i]))
        << U32LEB(getBreakIndex(curr->handlerBlocks[i]));
    }
  }
}

void BinaryInstWriter::visitStackSwitch(StackSwitch* curr) {
  assert(curr->cont->type.isContinuation());
  o << int8_t(BinaryConsts::Switch);
  parent.writeIndexedHeapType(curr->cont->type.getHeapType());
  o << U32LEB(parent.getTagIndex(curr->tag));
}

void BinaryInstWriter::emitScopeEnd(Expression* curr) {
  assert(!breakStack.empty());
  breakStack.pop_back();
  o << int8_t(BinaryConsts::End);
  if (func && !sourceMap) {
    parent.writeDebugLocationEnd(curr, func);
  }
}

void BinaryInstWriter::emitFunctionEnd() { o << int8_t(BinaryConsts::End); }

void BinaryInstWriter::emitUnreachable() {
  o << int8_t(BinaryConsts::Unreachable);
}

void BinaryInstWriter::mapLocalsAndEmitHeader() {
  assert(func && "BinaryInstWriter: function is not set");
  // Map params
  for (Index i = 0; i < func->getNumParams(); i++) {
    mappedLocals[std::make_pair(i, 0)] = i;
  }

  auto scratches = countScratchLocals();

  // Normally we map all locals of the same type into a range of adjacent
  // addresses, which is more compact. However, if we need to keep DWARF valid,
  // do not do any reordering at all - instead, do a trivial mapping that
  // keeps everything unmoved.
  //
  // Unless we have run DWARF-invalidating passes, all locals added during the
  // process that are not in DWARF info (tuple locals, tuple scratch locals,
  // locals to resolve stacky format, ..) have been all tacked on to the
  // existing locals and happen at the end, so as long as we print the local
  // types in order, we don't invalidate original local DWARF info here.
  if (DWARF) {
    Index mappedIndex = func->getVarIndexBase();
    for (Index i = func->getVarIndexBase(); i < func->getNumLocals(); i++) {
      size_t size = func->getLocalType(i).size();
      for (Index j = 0; j < size; j++) {
        mappedLocals[std::make_pair(i, j)] = mappedIndex++;
      }
    }

    size_t numBinaryLocals =
      mappedIndex - func->getVarIndexBase() + scratches.size();

    o << U32LEB(numBinaryLocals);

    for (Index i = func->getVarIndexBase(); i < func->getNumLocals(); i++) {
      for (const auto& type : func->getLocalType(i)) {
        o << U32LEB(1);
        parent.writeType(type);
      }
    }
    for (auto& [type, count] : scratches) {
      o << U32LEB(count);
      parent.writeType(type);
      scratchLocals[type] = mappedIndex;
      mappedIndex += count;
    }
    return;
  }

  for (auto type : func->vars) {
    for (const auto& t : type) {
      noteLocalType(t);
    }
  }

  for (auto& [type, count] : scratches) {
    noteLocalType(type, count);
  }

  if (parent.getModule()->features.hasReferenceTypes()) {
    // Sort local types in a way that keeps all MVP types together and all
    // reference types together. E.g. it is helpful to avoid a block of i32s in
    // between blocks of different reference types, since clearing out reference
    // types may require different work.
    //
    // See https://github.com/WebAssembly/binaryen/issues/4773
    //
    // In order to decide whether to put MVP types or reference types first,
    // look at the type of the first local. In an optimized binary we will have
    // sorted the locals by frequency of uses, so this way we'll keep the most
    // commonly-used local at the top, which should work well in many cases.
    bool refsFirst = !localTypes.empty() && localTypes[0].isRef();
    std::stable_sort(localTypes.begin(), localTypes.end(), [&](Type a, Type b) {
      if (refsFirst) {
        return a.isRef() && !b.isRef();
      } else {
        return !a.isRef() && b.isRef();
      }
    });
  }

  // Map IR (local index, tuple index) pairs to binary local indices. Since
  // locals are grouped by type, start by calculating the base indices for each
  // type.
  std::unordered_map<Type, Index> nextFreeIndex;
  Index baseIndex = func->getVarIndexBase();
  for (auto& type : localTypes) {
    nextFreeIndex[type] = baseIndex;
    baseIndex += numLocalsByType[type];
  }

  // Map the IR index pairs to indices.
  for (Index i = func->getVarIndexBase(); i < func->getNumLocals(); i++) {
    Index j = 0;
    for (const auto& type : func->getLocalType(i)) {
      mappedLocals[{i, j++}] = nextFreeIndex[type]++;
    }
  }

  // Map scratch locals to the remaining indices.
  for (auto& [type, _] : scratches) {
    scratchLocals[type] = nextFreeIndex[type];
  }

  o << U32LEB(numLocalsByType.size());
  for (auto& localType : localTypes) {
    o << U32LEB(numLocalsByType.at(localType));
    parent.writeType(localType);
  }
}

void BinaryInstWriter::noteLocalType(Type type, Index count) {
  auto& num = numLocalsByType[type];
  if (num == 0) {
    localTypes.push_back(type);
  }
  num += count;
}

InsertOrderedMap<Type, Index> BinaryInstWriter::countScratchLocals() {
  struct ScratchLocalFinder : PostWalker<ScratchLocalFinder> {
    BinaryInstWriter& parent;
    InsertOrderedMap<Type, Index> scratches;

    ScratchLocalFinder(BinaryInstWriter& parent) : parent(parent) {}

    void visitTupleExtract(TupleExtract* curr) {
      if (curr->type == Type::unreachable) {
        // We will not emit this instruction anyway.
        return;
      }
      // Extracts from locals or globals are optimizable and do not require
      // scratch locals. Record them.
      auto* tuple = curr->tuple;
      if (tuple->is<LocalGet>() || tuple->is<LocalSet>() ||
          tuple->is<GlobalGet>()) {
        parent.extractedGets.insert({tuple, curr->index});
        return;
      }
      // Include a scratch register for each type of tuple.extract with nonzero
      // index present.
      if (curr->index != 0) {
        auto& count = scratches[curr->type];
        count = std::max(count, 1u);
      }
    }

    void visitStringWTF16Get(StringWTF16Get* curr) {
      if (curr->type == Type::unreachable) {
        return;
      }
      // If `pos` already a local.get, we can defer emitting that local.get
      // instead of using a scratch local.
      if (auto* get = curr->pos->dynCast<LocalGet>()) {
        parent.deferredGets.insert(get);
        return;
      }
      // Scratch local to hold the `pos` value while we emit a stringview
      // conversion for the `ref` value.
      auto& count = scratches[Type::i32];
      count = std::max(count, 1u);
    }

    void visitStringSliceWTF(StringSliceWTF* curr) {
      if (curr->type == Type::unreachable) {
        return;
      }
      // If `start` and `end` are already local.gets, we can defer emitting
      // those gets instead of using scratch locals.
      auto* startGet = curr->start->dynCast<LocalGet>();
      auto* endGet = curr->end->dynCast<LocalGet>();
      if (startGet && endGet) {
        parent.deferredGets.insert(startGet);
        parent.deferredGets.insert(endGet);
        return;
      }
      // Scratch locals to hold the `start` and `end` values while we emit a
      // stringview conversion for the `ref` value.
      auto& count = scratches[Type::i32];
      count = std::max(count, 2u);
    }

    // As mentioned in BinaryInstWriter::visitBreak, the type of br_if with a
    // value may be more refined in Binaryen IR compared to the wasm spec, as we
    // give it the type of the value, while the spec gives it the type of the
    // block it targets. To avoid problems we must handle the case where a br_if
    // has a value, the value is more refined then the target, and the value is
    // not dropped (the last condition is very rare in real-world wasm, making
    // all of this a quite unusual situation). First, detect such situations by
    // seeing if we have br_ifs that return reference types at all. We do so by
    // counting them, and as we go we ignore ones that are dropped, since a
    // dropped value is not a problem for us.
    //
    // Note that we do not check all the conditions here, such as if the type
    // matches the break target, or if the parent is a cast, which we leave for
    // a more expensive analysis later, which we only run if we see something
    // suspicious here.
    Index numDangerousBrIfs = 0;

    void visitBreak(Break* curr) {
      if (curr->type.hasRef()) {
        numDangerousBrIfs++;
      }
    }

    void visitDrop(Drop* curr) {
      if (curr->value->is<Break>() && curr->value->type.hasRef()) {
        // The value is exactly a br_if of a ref, that we just visited before
        // us. Undo the ++ from there as it can be ignored.
        assert(numDangerousBrIfs > 0);
        numDangerousBrIfs--;
      }
    }
  } finder(*this);
  finder.walk(func->body);

  if (!finder.numDangerousBrIfs || !parent.getModule()->features.hasGC()) {
    // Nothing more to do: either no such br_ifs, or GC is not enabled.
    //
    // The explicit check for GC is here because if only reference types are
    // enabled then we still may seem to need a fixup here, e.g. if a ref.func
    // is br_if'd to a block of type funcref. But that only appears that way
    // because in Binaryen IR we allow non-nullable types even without GC (and
    // if GC is not enabled then we always emit nullable types in the binary).
    // That is, even if we see a type difference without GC, it will vanish in
    // the binary format; there is never a need to add any ref.casts without GC
    // being enabled.
    return std::move(finder.scratches);
  }

  // There are dangerous-looking br_ifs, so we must do the harder work to
  // actually investigate them in detail, including tracking block types. By
  // being fully precise here, we'll only emit casts when absolutely necessary,
  // which avoids repeated roundtrips adding more and more code.
  struct RefinementScanner : public ExpressionStackWalker<RefinementScanner> {
    BinaryInstWriter& writer;
    ScratchLocalFinder& finder;

    RefinementScanner(BinaryInstWriter& writer, ScratchLocalFinder& finder)
      : writer(writer), finder(finder) {}

    void visitBreak(Break* curr) {
      // See if this is one of the dangerous br_ifs we must handle.
      if (!curr->type.hasRef()) {
        // Not even a reference.
        return;
      }
      auto* parent = getParent();
      if (parent) {
        if (parent->is<Drop>()) {
          // It is dropped anyhow.
          return;
        }
        if (auto* cast = parent->dynCast<RefCast>()) {
          if (Type::isSubType(cast->type, curr->type)) {
            // It is cast to the same type or a better one. In particular this
            // handles the case of repeated roundtripping: After the first
            // roundtrip we emit a cast that we'll identify here, and not emit
            // an additional one.
            return;
          }
        }
      }
      auto* breakTarget = findBreakTarget(curr->name);
      auto unrefinedType = breakTarget->type;
      if (unrefinedType == curr->type) {
        // It has the proper type anyhow.
        return;
      }

      // Mark the br_if as needing handling, and add the type to the set of
      // types we need scratch tuple locals for (if relevant).
      writer.brIfsNeedingHandling[curr] = unrefinedType;

      if (unrefinedType.isTuple()) {
        // We must allocate enough scratch locals for this tuple. Note that we
        // may need more than one per type in the tuple, if a type appears more
        // than once, so we count their appearances.
        InsertOrderedMap<Type, Index> scratchTypeUses;
        for (auto t : unrefinedType) {
          scratchTypeUses[t]++;
        }
        for (auto& [type, uses] : scratchTypeUses) {
          auto& count = finder.scratches[type];
          count = std::max(count, uses);
        }
      }
    }
  } refinementScanner(*this, finder);
  refinementScanner.walk(func->body);

  return std::move(finder.scratches);
}

void BinaryInstWriter::emitMemoryAccess(size_t alignment,
                                        size_t bytes,
                                        uint64_t offset,
                                        Name memory) {
  uint32_t alignmentBits = Bits::log2(alignment ? alignment : bytes);
  uint32_t memoryIdx = parent.getMemoryIndex(memory);
  if (memoryIdx > 0) {
    // Set bit 6 in the alignment to indicate a memory index is present per:
    // https://github.com/WebAssembly/multi-memory/blob/main/proposals/multi-memory/Overview.md
    alignmentBits = alignmentBits | 1 << 6;
  }
  o << U32LEB(alignmentBits);
  if (memoryIdx > 0) {
    o << U32LEB(memoryIdx);
  }

  bool memory64 = parent.getModule()->getMemory(memory)->is64();
  if (memory64) {
    o << U64LEB(offset);
  } else {
    o << U32LEB(offset);
  }
}

int32_t BinaryInstWriter::getBreakIndex(Name name) { // -1 if not found
  if (name == DELEGATE_CALLER_TARGET) {
    return breakStack.size();
  }
  for (int i = breakStack.size() - 1; i >= 0; i--) {
    if (breakStack[i] == name) {
      return breakStack.size() - 1 - i;
    }
  }
  WASM_UNREACHABLE("break index not found");
}

// Queues the expressions linearly in Stack IR (SIR)
class StackIRGenerator : public BinaryenIRWriter<StackIRGenerator> {
public:
  StackIRGenerator(Module& module, Function* func)
    : BinaryenIRWriter<StackIRGenerator>(func), module(module) {}

  void emit(Expression* curr);
  void emitScopeEnd(Expression* curr);
  void emitHeader() {}
  void emitIfElse(If* curr) {
    stackIR.push_back(makeStackInst(StackInst::IfElse, curr));
  }
  void emitCatch(Try* curr, Index i) {
    stackIR.push_back(makeStackInst(StackInst::Catch, curr));
  }
  void emitCatchAll(Try* curr) {
    stackIR.push_back(makeStackInst(StackInst::CatchAll, curr));
  }
  void emitDelegate(Try* curr) {
    stackIR.push_back(makeStackInst(StackInst::Delegate, curr));
  }
  void emitFunctionEnd() {}
  void emitUnreachable() {
    stackIR.push_back(makeStackInst(Builder(module).makeUnreachable()));
  }
  void emitDebugLocation(Expression* curr) {}

  StackIR& getStackIR() { return stackIR; }

private:
  StackInst* makeStackInst(StackInst::Op op, Expression* origin);
  StackInst* makeStackInst(Expression* origin) {
    return makeStackInst(StackInst::Basic, origin);
  }

  Module& module;
  StackIR stackIR; // filled in write()
};

void StackIRGenerator::emit(Expression* curr) {
  StackInst* stackInst = nullptr;
  if (curr->is<Block>()) {
    stackInst = makeStackInst(StackInst::BlockBegin, curr);
  } else if (curr->is<If>()) {
    stackInst = makeStackInst(StackInst::IfBegin, curr);
  } else if (curr->is<Loop>()) {
    stackInst = makeStackInst(StackInst::LoopBegin, curr);
  } else if (curr->is<Try>()) {
    stackInst = makeStackInst(StackInst::TryBegin, curr);
  } else if (curr->is<TryTable>()) {
    stackInst = makeStackInst(StackInst::TryTableBegin, curr);
  } else {
    stackInst = makeStackInst(curr);
  }
  stackIR.push_back(stackInst);
}

void StackIRGenerator::emitScopeEnd(Expression* curr) {
  StackInst* stackInst = nullptr;
  if (curr->is<Block>()) {
    stackInst = makeStackInst(StackInst::BlockEnd, curr);
  } else if (curr->is<If>()) {
    stackInst = makeStackInst(StackInst::IfEnd, curr);
  } else if (curr->is<Loop>()) {
    stackInst = makeStackInst(StackInst::LoopEnd, curr);
  } else if (curr->is<Try>()) {
    stackInst = makeStackInst(StackInst::TryEnd, curr);
  } else if (curr->is<TryTable>()) {
    stackInst = makeStackInst(StackInst::TryTableEnd, curr);
  } else {
    WASM_UNREACHABLE("unexpected expr type");
  }
  stackIR.push_back(stackInst);
}

StackInst* StackIRGenerator::makeStackInst(StackInst::Op op,
                                           Expression* origin) {
  auto* ret = module.allocator.alloc<StackInst>();
  ret->op = op;
  ret->origin = origin;
  auto stackType = origin->type;
  if (Properties::isControlFlowStructure(origin)) {
    if (stackType == Type::unreachable) {
      // There are no unreachable blocks, loops, ifs, trys, or try_tables. we
      // emit extra unreachables to fix that up, so that they are valid as
      // having none type.
      stackType = Type::none;
    } else if (op != StackInst::BlockEnd && op != StackInst::IfEnd &&
               op != StackInst::LoopEnd && op != StackInst::TryEnd &&
               op != StackInst::TryTableEnd) {
      // If a concrete type is returned, we mark the end of the construct has
      // having that type (as it is pushed to the value stack at that point),
      // other parts are marked as none).
      stackType = Type::none;
    }
  }
  ret->type = stackType;
  return ret;
}

ModuleStackIR::ModuleStackIR(Module& wasm, const PassOptions& options)
  : analysis(wasm, [&](Function* func, StackIR& stackIR) {
      if (func->imported()) {
        return;
      }

      StackIRGenerator stackIRGen(wasm, func);
      stackIRGen.write();
      stackIR = std::move(stackIRGen.getStackIR());

      if (options.optimizeStackIR) {
        StackIROptimizer optimizer(func, stackIR, options, wasm.features);
        optimizer.run();
      }
    }) {}

void StackIRToBinaryWriter::write() {
  if (func->prologLocation) {
    parent.writeDebugLocation(*func->prologLocation);
  }
  writer.mapLocalsAndEmitHeader();
  // Stack to track indices of catches within a try
  SmallVector<Index, 4> catchIndexStack;
  for (auto* inst : stackIR) {
    if (!inst) {
      continue; // a nullptr is just something we can skip
    }
    switch (inst->op) {
      case StackInst::TryBegin:
        catchIndexStack.push_back(0);
        [[fallthrough]];
      case StackInst::Basic:
      case StackInst::BlockBegin:
      case StackInst::IfBegin:
      case StackInst::LoopBegin:
      case StackInst::TryTableBegin: {
        if (sourceMap) {
          parent.writeDebugLocation(inst->origin, func);
        }
        writer.visit(inst->origin);
        if (sourceMap) {
          parent.writeDebugLocationEnd(inst->origin, func);
        }
        break;
      }
      case StackInst::TryEnd:
        catchIndexStack.pop_back();
        [[fallthrough]];
      case StackInst::BlockEnd:
      case StackInst::IfEnd:
      case StackInst::LoopEnd:
      case StackInst::TryTableEnd: {
        writer.emitScopeEnd(inst->origin);
        break;
      }
      case StackInst::IfElse: {
        writer.emitIfElse(inst->origin->cast<If>());
        break;
      }
      case StackInst::Catch: {
        writer.emitCatch(inst->origin->cast<Try>(), catchIndexStack.back()++);
        break;
      }
      case StackInst::CatchAll: {
        writer.emitCatchAll(inst->origin->cast<Try>());
        break;
      }
      case StackInst::Delegate: {
        writer.emitDelegate(inst->origin->cast<Try>());
        // Delegates end the try, like a TryEnd.
        catchIndexStack.pop_back();
        break;
      }
      default:
        WASM_UNREACHABLE("unexpected op");
    }
  }
  // Indicate the debug location corresponding to the end opcode that
  // terminates the function code.
  if (func->epilogLocation) {
    parent.writeDebugLocation(*func->epilogLocation);
  } else {
    // The end opcode has no debug location.
    parent.writeNoDebugLocation();
  }
  writer.emitFunctionEnd();
}

} // namespace wasm
