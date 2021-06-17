/*
 * Copyright 2015 WebAssembly Community Group participants
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

//
// Parses and emits WebAssembly binary code
//

#ifndef wasm_wasm_binary_h
#define wasm_wasm_binary_h

#include <cassert>
#include <ostream>
#include <type_traits>

#include "ir/import-utils.h"
#include "ir/module-utils.h"
#include "parsing.h"
#include "support/debug.h"
#include "wasm-builder.h"
#include "wasm-traversal.h"
#include "wasm-validator.h"
#include "wasm.h"

#define DEBUG_TYPE "binary"

namespace wasm {

enum {
  // the maximum amount of bytes we emit per LEB
  MaxLEB32Bytes = 5,
};

// wasm VMs on the web have decided to impose some limits on what they
// accept
enum WebLimitations : uint32_t {
  MaxDataSegments = 100 * 1000,
  MaxFunctionBodySize = 128 * 1024,
  MaxFunctionLocals = 50 * 1000
};

template<typename T, typename MiniT> struct LEB {
  static_assert(sizeof(MiniT) == 1, "MiniT must be a byte");

  T value;

  LEB() = default;
  LEB(T value) : value(value) {}

  bool hasMore(T temp, MiniT byte) {
    // for signed, we must ensure the last bit has the right sign, as it will
    // zero extend
    return std::is_signed<T>::value
             ? (temp != 0 && temp != T(-1)) || (value >= 0 && (byte & 64)) ||
                 (value < 0 && !(byte & 64))
             : (temp != 0);
  }

  void write(std::vector<uint8_t>* out) {
    T temp = value;
    bool more;
    do {
      uint8_t byte = temp & 127;
      temp >>= 7;
      more = hasMore(temp, byte);
      if (more) {
        byte = byte | 128;
      }
      out->push_back(byte);
    } while (more);
  }

  // @minimum: a minimum number of bytes to write, padding as necessary
  // returns the number of bytes written
  size_t writeAt(std::vector<uint8_t>* out, size_t at, size_t minimum = 0) {
    T temp = value;
    size_t offset = 0;
    bool more;
    do {
      uint8_t byte = temp & 127;
      temp >>= 7;
      more = hasMore(temp, byte) || offset + 1 < minimum;
      if (more) {
        byte = byte | 128;
      }
      (*out)[at + offset] = byte;
      offset++;
    } while (more);
    return offset;
  }

  LEB<T, MiniT>& read(std::function<MiniT()> get) {
    value = 0;
    T shift = 0;
    MiniT byte;
    while (1) {
      byte = get();
      bool last = !(byte & 128);
      T payload = byte & 127;
      typedef typename std::make_unsigned<T>::type mask_type;
      auto shift_mask = 0 == shift
                          ? ~mask_type(0)
                          : ((mask_type(1) << (sizeof(T) * 8 - shift)) - 1u);
      T significant_payload = payload & shift_mask;
      if (significant_payload != payload) {
        if (!(std::is_signed<T>::value && last)) {
          throw ParseException("LEB dropped bits only valid for signed LEB");
        }
      }
      value |= significant_payload << shift;
      if (last) {
        break;
      }
      shift += 7;
      if (size_t(shift) >= sizeof(T) * 8) {
        throw ParseException("LEB overflow");
      }
    }
    // If signed LEB, then we might need to sign-extend. (compile should
    // optimize this out if not needed).
    if (std::is_signed<T>::value) {
      shift += 7;
      if ((byte & 64) && size_t(shift) < 8 * sizeof(T)) {
        size_t sext_bits = 8 * sizeof(T) - size_t(shift);
        value <<= sext_bits;
        value >>= sext_bits;
        if (value >= 0) {
          throw ParseException(
            " LEBsign-extend should produce a negative value");
        }
      }
    }
    return *this;
  }
};

typedef LEB<uint32_t, uint8_t> U32LEB;
typedef LEB<uint64_t, uint8_t> U64LEB;
typedef LEB<int32_t, int8_t> S32LEB;
typedef LEB<int64_t, int8_t> S64LEB;

//
// We mostly stream into a buffer as we create the binary format, however,
// sometimes we need to backtrack and write to a location behind us - wasm
// is optimized for reading, not writing.
//
class BufferWithRandomAccess : public std::vector<uint8_t> {
public:
  BufferWithRandomAccess() = default;

  BufferWithRandomAccess& operator<<(int8_t x) {
    BYN_TRACE("writeInt8: " << (int)(uint8_t)x << " (at " << size() << ")\n");
    push_back(x);
    return *this;
  }
  BufferWithRandomAccess& operator<<(int16_t x) {
    BYN_TRACE("writeInt16: " << x << " (at " << size() << ")\n");
    push_back(x & 0xff);
    push_back(x >> 8);
    return *this;
  }
  BufferWithRandomAccess& operator<<(int32_t x) {
    BYN_TRACE("writeInt32: " << x << " (at " << size() << ")\n");
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    return *this;
  }
  BufferWithRandomAccess& operator<<(int64_t x) {
    BYN_TRACE("writeInt64: " << x << " (at " << size() << ")\n");
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    x >>= 8;
    push_back(x & 0xff);
    return *this;
  }
  BufferWithRandomAccess& operator<<(U32LEB x) {
    size_t before = -1;
    WASM_UNUSED(before);
    BYN_DEBUG(before = size(); std::cerr << "writeU32LEB: " << x.value
                                         << " (at " << before << ")"
                                         << std::endl;);
    x.write(this);
    BYN_DEBUG(for (size_t i = before; i < size(); i++) {
      std::cerr << "  " << (int)at(i) << " (at " << i << ")\n";
    });
    return *this;
  }
  BufferWithRandomAccess& operator<<(U64LEB x) {
    size_t before = -1;
    WASM_UNUSED(before);
    BYN_DEBUG(before = size(); std::cerr << "writeU64LEB: " << x.value
                                         << " (at " << before << ")"
                                         << std::endl;);
    x.write(this);
    BYN_DEBUG(for (size_t i = before; i < size(); i++) {
      std::cerr << "  " << (int)at(i) << " (at " << i << ")\n";
    });
    return *this;
  }
  BufferWithRandomAccess& operator<<(S32LEB x) {
    size_t before = -1;
    WASM_UNUSED(before);
    BYN_DEBUG(before = size(); std::cerr << "writeS32LEB: " << x.value
                                         << " (at " << before << ")"
                                         << std::endl;);
    x.write(this);
    BYN_DEBUG(for (size_t i = before; i < size(); i++) {
      std::cerr << "  " << (int)at(i) << " (at " << i << ")\n";
    });
    return *this;
  }
  BufferWithRandomAccess& operator<<(S64LEB x) {
    size_t before = -1;
    WASM_UNUSED(before);
    BYN_DEBUG(before = size(); std::cerr << "writeS64LEB: " << x.value
                                         << " (at " << before << ")"
                                         << std::endl;);
    x.write(this);
    BYN_DEBUG(for (size_t i = before; i < size(); i++) {
      std::cerr << "  " << (int)at(i) << " (at " << i << ")\n";
    });
    return *this;
  }

  BufferWithRandomAccess& operator<<(uint8_t x) { return *this << (int8_t)x; }
  BufferWithRandomAccess& operator<<(uint16_t x) { return *this << (int16_t)x; }
  BufferWithRandomAccess& operator<<(uint32_t x) { return *this << (int32_t)x; }
  BufferWithRandomAccess& operator<<(uint64_t x) { return *this << (int64_t)x; }

  BufferWithRandomAccess& operator<<(float x) {
    BYN_TRACE("writeFloat32: " << x << " (at " << size() << ")\n");
    return *this << Literal(x).reinterpreti32();
  }
  BufferWithRandomAccess& operator<<(double x) {
    BYN_TRACE("writeFloat64: " << x << " (at " << size() << ")\n");
    return *this << Literal(x).reinterpreti64();
  }

  void writeAt(size_t i, uint16_t x) {
    BYN_TRACE("backpatchInt16: " << x << " (at " << i << ")\n");
    (*this)[i] = x & 0xff;
    (*this)[i + 1] = x >> 8;
  }
  void writeAt(size_t i, uint32_t x) {
    BYN_TRACE("backpatchInt32: " << x << " (at " << i << ")\n");
    (*this)[i] = x & 0xff;
    x >>= 8;
    (*this)[i + 1] = x & 0xff;
    x >>= 8;
    (*this)[i + 2] = x & 0xff;
    x >>= 8;
    (*this)[i + 3] = x & 0xff;
  }

  // writes out an LEB to an arbitrary location. this writes the LEB as a full
  // 5 bytes, the fixed amount that can easily be set aside ahead of time
  void writeAtFullFixedSize(size_t i, U32LEB x) {
    BYN_TRACE("backpatchU32LEB: " << x.value << " (at " << i << ")\n");
    // fill all 5 bytes, we have to do this when backpatching
    x.writeAt(this, i, MaxLEB32Bytes);
  }
  // writes out an LEB of normal size
  // returns how many bytes were written
  size_t writeAt(size_t i, U32LEB x) {
    BYN_TRACE("writeAtU32LEB: " << x.value << " (at " << i << ")\n");
    return x.writeAt(this, i);
  }

  template<typename T> void writeTo(T& o) {
    for (auto c : *this) {
      o << c;
    }
  }

  std::vector<char> getAsChars() {
    std::vector<char> ret;
    ret.resize(size());
    std::copy(begin(), end(), ret.begin());
    return ret;
  }
};

namespace BinaryConsts {

enum Meta { Magic = 0x6d736100, Version = 0x01 };

enum Section {
  User = 0,
  Type = 1,
  Import = 2,
  Function = 3,
  Table = 4,
  Memory = 5,
  Global = 6,
  Export = 7,
  Start = 8,
  Element = 9,
  Code = 10,
  Data = 11,
  DataCount = 12,
  Event = 13
};

// A passive segment is a segment that will not be automatically copied into a
//   memory or table on instantiation, and must instead be applied manually
//   using the instructions memory.init or table.init.
// An active segment is equivalent to a passive segment, but with an implicit
//   memory.init followed by a data.drop (or table.init followed by a elem.drop)
//   that is prepended to the module's start function.
// A declarative element segment is not available at runtime but merely serves
//   to forward-declare references that are formed in code with instructions
//   like ref.func.
enum SegmentFlag {
  // Bit 0: 0 = active, 1 = passive
  IsPassive = 1 << 0,
  // Bit 1 if passive: 0 = passive, 1 = declarative
  IsDeclarative = 1 << 1,
  // Bit 1 if active: 0 = index 0, 1 = index given
  HasIndex = 1 << 1,
  // Table element segments only:
  // Bit 2: 0 = elemType is funcref and vector of func indexes given
  //        1 = elemType is given and vector of ref expressions is given
  UsesExpressions = 1 << 2
};

enum EncodedType {
  // value_type
  i32 = -0x1,  // 0x7f
  i64 = -0x2,  // 0x7e
  f32 = -0x3,  // 0x7d
  f64 = -0x4,  // 0x7c
  v128 = -0x5, // 0x7b
  i8 = -0x6,   // 0x7a
  i16 = -0x7,  // 0x79
  // function reference type
  funcref = -0x10, // 0x70
  // opaque host reference type
  externref = -0x11, // 0x6f
  // any reference type
  anyref = -0x12, // 0x6e
  // comparable reference type
  eqref = -0x13, // 0x6d
  // nullable typed function reference type, with parameter
  nullable = -0x14, // 0x6c
  // non-nullable typed function reference type, with parameter
  nonnullable = -0x15, // 0x6b
  // integer reference type
  i31ref = -0x16, // 0x6a
  // run-time type info type, with depth index n
  rtt_n = -0x17, // 0x69
  // run-time type info type, without depth index n
  rtt = -0x18,     // 0x68
  dataref = -0x19, // 0x67
  // func_type form
  Func = -0x20,            // 0x60
  Struct = -0x21,          // 0x5f
  Array = -0x22,           // 0x5e
  FuncExtending = -0x23,   // 0x5d
  StructExtending = -0x24, // 0x5c
  ArrayExtending = -0x25,  // 0x5b
  // block_type
  Empty = -0x40 // 0x40
};

enum EncodedHeapType {
  func = -0x10,    // 0x70
  extern_ = -0x11, // 0x6f
  any = -0x12,     // 0x6e
  eq = -0x13,      // 0x6d
  i31 = -0x16,     // 0x6a
  data = -0x19,    // 0x67
};

namespace UserSections {
extern const char* Name;
extern const char* SourceMapUrl;
extern const char* Dylink;
extern const char* Linking;
extern const char* Producers;
extern const char* TargetFeatures;

extern const char* AtomicsFeature;
extern const char* BulkMemoryFeature;
extern const char* ExceptionHandlingFeature;
extern const char* MutableGlobalsFeature;
extern const char* TruncSatFeature;
extern const char* SignExtFeature;
extern const char* SIMD128Feature;
extern const char* ExceptionHandlingFeature;
extern const char* TailCallFeature;
extern const char* ReferenceTypesFeature;
extern const char* MultivalueFeature;
extern const char* GCFeature;
extern const char* Memory64Feature;
extern const char* TypedFunctionReferencesFeature;

enum Subsection {
  NameModule = 0,
  NameFunction = 1,
  NameLocal = 2,
  // see: https://github.com/WebAssembly/extended-name-section
  NameLabel = 3,
  NameType = 4,
  NameTable = 5,
  NameMemory = 6,
  NameGlobal = 7,
  NameElem = 8,
  NameData = 9,
  // see: https://github.com/WebAssembly/gc/issues/193
  NameField = 10
};

} // namespace UserSections

enum ASTNodes {
  Unreachable = 0x00,
  Nop = 0x01,
  Block = 0x02,
  Loop = 0x03,
  If = 0x04,
  Else = 0x05,

  End = 0x0b,
  Br = 0x0c,
  BrIf = 0x0d,
  BrTable = 0x0e,
  Return = 0x0f,

  CallFunction = 0x10,
  CallIndirect = 0x11,
  RetCallFunction = 0x12,
  RetCallIndirect = 0x13,

  Drop = 0x1a,
  Select = 0x1b,
  SelectWithType = 0x1c, // added in reference types proposal

  LocalGet = 0x20,
  LocalSet = 0x21,
  LocalTee = 0x22,
  GlobalGet = 0x23,
  GlobalSet = 0x24,

  I32LoadMem = 0x28,
  I64LoadMem = 0x29,
  F32LoadMem = 0x2a,
  F64LoadMem = 0x2b,

  I32LoadMem8S = 0x2c,
  I32LoadMem8U = 0x2d,
  I32LoadMem16S = 0x2e,
  I32LoadMem16U = 0x2f,
  I64LoadMem8S = 0x30,
  I64LoadMem8U = 0x31,
  I64LoadMem16S = 0x32,
  I64LoadMem16U = 0x33,
  I64LoadMem32S = 0x34,
  I64LoadMem32U = 0x35,

  I32StoreMem = 0x36,
  I64StoreMem = 0x37,
  F32StoreMem = 0x38,
  F64StoreMem = 0x39,

  I32StoreMem8 = 0x3a,
  I32StoreMem16 = 0x3b,
  I64StoreMem8 = 0x3c,
  I64StoreMem16 = 0x3d,
  I64StoreMem32 = 0x3e,

  MemorySize = 0x3f,
  MemoryGrow = 0x40,

  TableGet = 0x25,
  TableSet = 0x26,
  TableSize = 0x10,
  TableGrow = 0x0f,
  TableFill = 0x11,
  TableCopy = 0x0e,
  TableInit = 0x0c,
  ElemDrop = 0x0d,

  I32Const = 0x41,
  I64Const = 0x42,
  F32Const = 0x43,
  F64Const = 0x44,

  I32EqZ = 0x45,
  I32Eq = 0x46,
  I32Ne = 0x47,
  I32LtS = 0x48,
  I32LtU = 0x49,
  I32GtS = 0x4a,
  I32GtU = 0x4b,
  I32LeS = 0x4c,
  I32LeU = 0x4d,
  I32GeS = 0x4e,
  I32GeU = 0x4f,
  I64EqZ = 0x50,
  I64Eq = 0x51,
  I64Ne = 0x52,
  I64LtS = 0x53,
  I64LtU = 0x54,
  I64GtS = 0x55,
  I64GtU = 0x56,
  I64LeS = 0x57,
  I64LeU = 0x58,
  I64GeS = 0x59,
  I64GeU = 0x5a,
  F32Eq = 0x5b,
  F32Ne = 0x5c,
  F32Lt = 0x5d,
  F32Gt = 0x5e,
  F32Le = 0x5f,
  F32Ge = 0x60,
  F64Eq = 0x61,
  F64Ne = 0x62,
  F64Lt = 0x63,
  F64Gt = 0x64,
  F64Le = 0x65,
  F64Ge = 0x66,

  I32Clz = 0x67,
  I32Ctz = 0x68,
  I32Popcnt = 0x69,
  I32Add = 0x6a,
  I32Sub = 0x6b,
  I32Mul = 0x6c,
  I32DivS = 0x6d,
  I32DivU = 0x6e,
  I32RemS = 0x6f,
  I32RemU = 0x70,
  I32And = 0x71,
  I32Or = 0x72,
  I32Xor = 0x73,
  I32Shl = 0x74,
  I32ShrS = 0x75,
  I32ShrU = 0x76,
  I32RotL = 0x77,
  I32RotR = 0x78,

  I64Clz = 0x79,
  I64Ctz = 0x7a,
  I64Popcnt = 0x7b,
  I64Add = 0x7c,
  I64Sub = 0x7d,
  I64Mul = 0x7e,
  I64DivS = 0x7f,
  I64DivU = 0x80,
  I64RemS = 0x81,
  I64RemU = 0x82,
  I64And = 0x83,
  I64Or = 0x84,
  I64Xor = 0x85,
  I64Shl = 0x86,
  I64ShrS = 0x87,
  I64ShrU = 0x88,
  I64RotL = 0x89,
  I64RotR = 0x8a,

  F32Abs = 0x8b,
  F32Neg = 0x8c,
  F32Ceil = 0x8d,
  F32Floor = 0x8e,
  F32Trunc = 0x8f,
  F32NearestInt = 0x90,
  F32Sqrt = 0x91,
  F32Add = 0x92,
  F32Sub = 0x93,
  F32Mul = 0x94,
  F32Div = 0x95,
  F32Min = 0x96,
  F32Max = 0x97,
  F32CopySign = 0x98,

  F64Abs = 0x99,
  F64Neg = 0x9a,
  F64Ceil = 0x9b,
  F64Floor = 0x9c,
  F64Trunc = 0x9d,
  F64NearestInt = 0x9e,
  F64Sqrt = 0x9f,
  F64Add = 0xa0,
  F64Sub = 0xa1,
  F64Mul = 0xa2,
  F64Div = 0xa3,
  F64Min = 0xa4,
  F64Max = 0xa5,
  F64CopySign = 0xa6,

  I32WrapI64 = 0xa7,
  I32STruncF32 = 0xa8,
  I32UTruncF32 = 0xa9,
  I32STruncF64 = 0xaa,
  I32UTruncF64 = 0xab,
  I64SExtendI32 = 0xac,
  I64UExtendI32 = 0xad,
  I64STruncF32 = 0xae,
  I64UTruncF32 = 0xaf,
  I64STruncF64 = 0xb0,
  I64UTruncF64 = 0xb1,
  F32SConvertI32 = 0xb2,
  F32UConvertI32 = 0xb3,
  F32SConvertI64 = 0xb4,
  F32UConvertI64 = 0xb5,
  F32DemoteI64 = 0xb6,
  F64SConvertI32 = 0xb7,
  F64UConvertI32 = 0xb8,
  F64SConvertI64 = 0xb9,
  F64UConvertI64 = 0xba,
  F64PromoteF32 = 0xbb,

  I32ReinterpretF32 = 0xbc,
  I64ReinterpretF64 = 0xbd,
  F32ReinterpretI32 = 0xbe,
  F64ReinterpretI64 = 0xbf,

  I32ExtendS8 = 0xc0,
  I32ExtendS16 = 0xc1,
  I64ExtendS8 = 0xc2,
  I64ExtendS16 = 0xc3,
  I64ExtendS32 = 0xc4,

  // prefixes

  GCPrefix = 0xfb,
  MiscPrefix = 0xfc,
  SIMDPrefix = 0xfd,
  AtomicPrefix = 0xfe,

  // atomic opcodes

  AtomicNotify = 0x00,
  I32AtomicWait = 0x01,
  I64AtomicWait = 0x02,
  AtomicFence = 0x03,

  I32AtomicLoad = 0x10,
  I64AtomicLoad = 0x11,
  I32AtomicLoad8U = 0x12,
  I32AtomicLoad16U = 0x13,
  I64AtomicLoad8U = 0x14,
  I64AtomicLoad16U = 0x15,
  I64AtomicLoad32U = 0x16,
  I32AtomicStore = 0x17,
  I64AtomicStore = 0x18,
  I32AtomicStore8 = 0x19,
  I32AtomicStore16 = 0x1a,
  I64AtomicStore8 = 0x1b,
  I64AtomicStore16 = 0x1c,
  I64AtomicStore32 = 0x1d,

  AtomicRMWOps_Begin = 0x1e,
  I32AtomicRMWAdd = 0x1e,
  I64AtomicRMWAdd = 0x1f,
  I32AtomicRMWAdd8U = 0x20,
  I32AtomicRMWAdd16U = 0x21,
  I64AtomicRMWAdd8U = 0x22,
  I64AtomicRMWAdd16U = 0x23,
  I64AtomicRMWAdd32U = 0x24,
  I32AtomicRMWSub = 0x25,
  I64AtomicRMWSub = 0x26,
  I32AtomicRMWSub8U = 0x27,
  I32AtomicRMWSub16U = 0x28,
  I64AtomicRMWSub8U = 0x29,
  I64AtomicRMWSub16U = 0x2a,
  I64AtomicRMWSub32U = 0x2b,
  I32AtomicRMWAnd = 0x2c,
  I64AtomicRMWAnd = 0x2d,
  I32AtomicRMWAnd8U = 0x2e,
  I32AtomicRMWAnd16U = 0x2f,
  I64AtomicRMWAnd8U = 0x30,
  I64AtomicRMWAnd16U = 0x31,
  I64AtomicRMWAnd32U = 0x32,
  I32AtomicRMWOr = 0x33,
  I64AtomicRMWOr = 0x34,
  I32AtomicRMWOr8U = 0x35,
  I32AtomicRMWOr16U = 0x36,
  I64AtomicRMWOr8U = 0x37,
  I64AtomicRMWOr16U = 0x38,
  I64AtomicRMWOr32U = 0x39,
  I32AtomicRMWXor = 0x3a,
  I64AtomicRMWXor = 0x3b,
  I32AtomicRMWXor8U = 0x3c,
  I32AtomicRMWXor16U = 0x3d,
  I64AtomicRMWXor8U = 0x3e,
  I64AtomicRMWXor16U = 0x3f,
  I64AtomicRMWXor32U = 0x40,
  I32AtomicRMWXchg = 0x41,
  I64AtomicRMWXchg = 0x42,
  I32AtomicRMWXchg8U = 0x43,
  I32AtomicRMWXchg16U = 0x44,
  I64AtomicRMWXchg8U = 0x45,
  I64AtomicRMWXchg16U = 0x46,
  I64AtomicRMWXchg32U = 0x47,
  AtomicRMWOps_End = 0x47,

  AtomicCmpxchgOps_Begin = 0x48,
  I32AtomicCmpxchg = 0x48,
  I64AtomicCmpxchg = 0x49,
  I32AtomicCmpxchg8U = 0x4a,
  I32AtomicCmpxchg16U = 0x4b,
  I64AtomicCmpxchg8U = 0x4c,
  I64AtomicCmpxchg16U = 0x4d,
  I64AtomicCmpxchg32U = 0x4e,
  AtomicCmpxchgOps_End = 0x4e,

  // truncsat opcodes

  I32STruncSatF32 = 0x00,
  I32UTruncSatF32 = 0x01,
  I32STruncSatF64 = 0x02,
  I32UTruncSatF64 = 0x03,
  I64STruncSatF32 = 0x04,
  I64UTruncSatF32 = 0x05,
  I64STruncSatF64 = 0x06,
  I64UTruncSatF64 = 0x07,

  // SIMD opcodes

  V128Load = 0x00,
  V128Load8x8S = 0x01,
  V128Load8x8U = 0x02,
  V128Load16x4S = 0x03,
  V128Load16x4U = 0x04,
  V128Load32x2S = 0x05,
  V128Load32x2U = 0x06,
  V128Load8Splat = 0x07,
  V128Load16Splat = 0x08,
  V128Load32Splat = 0x09,
  V128Load64Splat = 0x0a,
  V128Store = 0x0b,

  V128Const = 0x0c,
  I8x16Shuffle = 0x0d,
  I8x16Swizzle = 0x0e,

  I8x16Splat = 0x0f,
  I16x8Splat = 0x10,
  I32x4Splat = 0x11,
  I64x2Splat = 0x12,
  F32x4Splat = 0x13,
  F64x2Splat = 0x14,

  I8x16ExtractLaneS = 0x15,
  I8x16ExtractLaneU = 0x16,
  I8x16ReplaceLane = 0x17,
  I16x8ExtractLaneS = 0x18,
  I16x8ExtractLaneU = 0x19,
  I16x8ReplaceLane = 0x1a,
  I32x4ExtractLane = 0x1b,
  I32x4ReplaceLane = 0x1c,
  I64x2ExtractLane = 0x1d,
  I64x2ReplaceLane = 0x1e,
  F32x4ExtractLane = 0x1f,
  F32x4ReplaceLane = 0x20,
  F64x2ExtractLane = 0x21,
  F64x2ReplaceLane = 0x22,

  I8x16Eq = 0x23,
  I8x16Ne = 0x24,
  I8x16LtS = 0x25,
  I8x16LtU = 0x26,
  I8x16GtS = 0x27,
  I8x16GtU = 0x28,
  I8x16LeS = 0x29,
  I8x16LeU = 0x2a,
  I8x16GeS = 0x2b,
  I8x16GeU = 0x2c,
  I16x8Eq = 0x2d,
  I16x8Ne = 0x2e,
  I16x8LtS = 0x2f,
  I16x8LtU = 0x30,
  I16x8GtS = 0x31,
  I16x8GtU = 0x32,
  I16x8LeS = 0x33,
  I16x8LeU = 0x34,
  I16x8GeS = 0x35,
  I16x8GeU = 0x36,
  I32x4Eq = 0x37,
  I32x4Ne = 0x38,
  I32x4LtS = 0x39,
  I32x4LtU = 0x3a,
  I32x4GtS = 0x3b,
  I32x4GtU = 0x3c,
  I32x4LeS = 0x3d,
  I32x4LeU = 0x3e,
  I32x4GeS = 0x3f,
  I32x4GeU = 0x40,
  F32x4Eq = 0x41,
  F32x4Ne = 0x42,
  F32x4Lt = 0x43,
  F32x4Gt = 0x44,
  F32x4Le = 0x45,
  F32x4Ge = 0x46,
  F64x2Eq = 0x47,
  F64x2Ne = 0x48,
  F64x2Lt = 0x49,
  F64x2Gt = 0x4a,
  F64x2Le = 0x4b,
  F64x2Ge = 0x4c,

  V128Not = 0x4d,
  V128And = 0x4e,
  V128Andnot = 0x4f,
  V128Or = 0x50,
  V128Xor = 0x51,
  V128Bitselect = 0x52,
  V128AnyTrue = 0x53,

  V128Load8Lane = 0x54,
  V128Load16Lane = 0x55,
  V128Load32Lane = 0x56,
  V128Load64Lane = 0x57,
  V128Store8Lane = 0x58,
  V128Store16Lane = 0x59,
  V128Store32Lane = 0x5a,
  V128Store64Lane = 0x5b,
  V128Load32Zero = 0x5c,
  V128Load64Zero = 0x5d,

  F32x4DemoteF64x2Zero = 0x5e,
  F64x2PromoteLowF32x4 = 0x5f,

  I8x16Abs = 0x60,
  I8x16Neg = 0x61,
  I8x16Popcnt = 0x62,
  I8x16AllTrue = 0x63,
  I8x16Bitmask = 0x64,
  I8x16NarrowI16x8S = 0x65,
  I8x16NarrowI16x8U = 0x66,
  F32x4Ceil = 0x67,
  F32x4Floor = 0x68,
  F32x4Trunc = 0x69,
  F32x4Nearest = 0x6a,
  I8x16Shl = 0x6b,
  I8x16ShrS = 0x6c,
  I8x16ShrU = 0x6d,
  I8x16Add = 0x6e,
  I8x16AddSatS = 0x6f,
  I8x16AddSatU = 0x70,
  I8x16Sub = 0x71,
  I8x16SubSatS = 0x72,
  I8x16SubSatU = 0x73,
  F64x2Ceil = 0x74,
  F64x2Floor = 0x75,
  I8x16MinS = 0x76,
  I8x16MinU = 0x77,
  I8x16MaxS = 0x78,
  I8x16MaxU = 0x79,
  F64x2Trunc = 0x7a,
  I8x16AvgrU = 0x7b,
  I16x8ExtaddPairwiseI8x16S = 0x7c,
  I16x8ExtaddPairwiseI8x16U = 0x7d,
  I32x4ExtaddPairwiseI16x8S = 0x7e,
  I32x4ExtaddPairwiseI16x8U = 0x7f,

  I16x8Abs = 0x80,
  I16x8Neg = 0x81,
  I16x8Q15mulrSatS = 0x82,
  I16x8AllTrue = 0x83,
  I16x8Bitmask = 0x84,
  I16x8NarrowI32x4S = 0x85,
  I16x8NarrowI32x4U = 0x86,
  I16x8ExtendLowI8x16S = 0x87,
  I16x8ExtendHighI8x16S = 0x88,
  I16x8ExtendLowI8x16U = 0x89,
  I16x8ExtendHighI8x16U = 0x8a,
  I16x8Shl = 0x8b,
  I16x8ShrS = 0x8c,
  I16x8ShrU = 0x8d,
  I16x8Add = 0x8e,
  I16x8AddSatS = 0x8f,
  I16x8AddSatU = 0x90,
  I16x8Sub = 0x91,
  I16x8SubSatS = 0x92,
  I16x8SubSatU = 0x93,
  F64x2Nearest = 0x94,
  I16x8Mul = 0x95,
  I16x8MinS = 0x96,
  I16x8MinU = 0x97,
  I16x8MaxS = 0x98,
  I16x8MaxU = 0x99,
  // 0x9a unused
  I16x8AvgrU = 0x9b,
  I16x8ExtmulLowI8x16S = 0x9c,
  I16x8ExtmulHighI8x16S = 0x9d,
  I16x8ExtmulLowI8x16U = 0x9e,
  I16x8ExtmulHighI8x16U = 0x9f,

  I32x4Abs = 0xa0,
  I32x4Neg = 0xa1,
  // 0xa2 unused
  I32x4AllTrue = 0xa3,
  I32x4Bitmask = 0xa4,
  // 0xa5 unused
  // 0xa6 unused
  I32x4ExtendLowI16x8S = 0xa7,
  I32x4ExtendHighI16x8S = 0xa8,
  I32x4ExtendLowI16x8U = 0xa9,
  I32x4ExtendHighI16x8U = 0xaa,
  I32x4Shl = 0xab,
  I32x4ShrS = 0xac,
  I32x4ShrU = 0xad,
  I32x4Add = 0xae,
  // 0xaf unused
  // 0xb0 unused
  I32x4Sub = 0xb1,
  // 0xb2 unused
  // 0xb3 unused
  // 0xb4 unused
  I32x4Mul = 0xb5,
  I32x4MinS = 0xb6,
  I32x4MinU = 0xb7,
  I32x4MaxS = 0xb8,
  I32x4MaxU = 0xb9,
  I32x4DotI16x8S = 0xba,
  // 0xbb unused
  I32x4ExtmulLowI16x8S = 0xbc,
  I32x4ExtmulHighI16x8S = 0xbd,
  I32x4ExtmulLowI16x8U = 0xbe,
  I32x4ExtmulHighI16x8U = 0xbf,

  I64x2Abs = 0xc0,
  I64x2Neg = 0xc1,
  // 0xc2 unused
  I64x2AllTrue = 0xc3,
  I64x2Bitmask = 0xc4,
  // 0xc5 unused
  // 0xc6 unused
  I64x2ExtendLowI32x4S = 0xc7,
  I64x2ExtendHighI32x4S = 0xc8,
  I64x2ExtendLowI32x4U = 0xc9,
  I64x2ExtendHighI32x4U = 0xca,
  I64x2Shl = 0xcb,
  I64x2ShrS = 0xcc,
  I64x2ShrU = 0xcd,
  I64x2Add = 0xce,
  // 0xcf unused
  // 0xd0 unused
  I64x2Sub = 0xd1,
  // 0xd2 unused
  // 0xd3 unused
  // 0xd4 unused
  I64x2Mul = 0xd5,
  I64x2Eq = 0xd6,
  I64x2Ne = 0xd7,
  I64x2LtS = 0xd8,
  I64x2GtS = 0xd9,
  I64x2LeS = 0xda,
  I64x2GeS = 0xdb,
  I64x2ExtmulLowI32x4S = 0xdc,
  I64x2ExtmulHighI32x4S = 0xdd,
  I64x2ExtmulLowI32x4U = 0xde,
  I64x2ExtmulHighI32x4U = 0xdf,

  F32x4Abs = 0xe0,
  F32x4Neg = 0xe1,
  // 0xe2 unused
  F32x4Sqrt = 0xe3,
  F32x4Add = 0xe4,
  F32x4Sub = 0xe5,
  F32x4Mul = 0xe6,
  F32x4Div = 0xe7,
  F32x4Min = 0xe8,
  F32x4Max = 0xe9,
  F32x4Pmin = 0xea,
  F32x4Pmax = 0xeb,

  F64x2Abs = 0xec,
  F64x2Neg = 0xed,
  // 0xee unused
  F64x2Sqrt = 0xef,
  F64x2Add = 0xf0,
  F64x2Sub = 0xf1,
  F64x2Mul = 0xf2,
  F64x2Div = 0xf3,
  F64x2Min = 0xf4,
  F64x2Max = 0xf5,
  F64x2Pmin = 0xf6,
  F64x2Pmax = 0xf7,

  I32x4TruncSatF32x4S = 0xf8,
  I32x4TruncSatF32x4U = 0xf9,
  F32x4ConvertI32x4S = 0xfa,
  F32x4ConvertI32x4U = 0xfb,
  I32x4TruncSatF64x2SZero = 0xfc,
  I32x4TruncSatF64x2UZero = 0xfd,
  F64x2ConvertLowI32x4S = 0xfe,
  F64x2ConvertLowI32x4U = 0xff,

  // bulk memory opcodes

  MemoryInit = 0x08,
  DataDrop = 0x09,
  MemoryCopy = 0x0a,
  MemoryFill = 0x0b,

  // reference types opcodes

  RefNull = 0xd0,
  RefIsNull = 0xd1,
  RefFunc = 0xd2,
  RefAsNonNull = 0xd3,
  BrOnNull = 0xd4,
  BrOnNonNull = 0xd6,

  // exception handling opcodes

  Try = 0x06,
  Catch = 0x07,
  CatchAll = 0x19,
  Delegate = 0x18,
  Throw = 0x08,
  Rethrow = 0x09,

  // typed function references opcodes

  CallRef = 0x14,
  RetCallRef = 0x15,
  Let = 0x17,

  // gc opcodes

  RefEq = 0xd5,
  StructNewWithRtt = 0x01,
  StructNewDefaultWithRtt = 0x02,
  StructGet = 0x03,
  StructGetS = 0x04,
  StructGetU = 0x05,
  StructSet = 0x06,
  ArrayNewWithRtt = 0x11,
  ArrayNewDefaultWithRtt = 0x12,
  ArrayGet = 0x13,
  ArrayGetS = 0x14,
  ArrayGetU = 0x15,
  ArraySet = 0x16,
  ArrayLen = 0x17,
  ArrayCopy = 0x18,
  I31New = 0x20,
  I31GetS = 0x21,
  I31GetU = 0x22,
  RttCanon = 0x30,
  RttSub = 0x31,
  RttFreshSub = 0x32,
  RefTest = 0x40,
  RefCast = 0x41,
  BrOnCast = 0x42,
  BrOnCastFail = 0x43,
  RefIsFunc = 0x50,
  RefIsData = 0x51,
  RefIsI31 = 0x52,
  RefAsFunc = 0x58,
  RefAsData = 0x59,
  RefAsI31 = 0x5a,
  BrOnFunc = 0x60,
  BrOnData = 0x61,
  BrOnI31 = 0x62,
  BrOnNonFunc = 0x63,
  BrOnNonData = 0x64,
  BrOnNonI31 = 0x65,
};

enum MemoryAccess {
  Offset = 0x10,    // bit 4
  Alignment = 0x80, // bit 7
  NaturalAlignment = 0
};

enum MemoryFlags { HasMaximum = 1 << 0, IsShared = 1 << 1, Is64 = 1 << 2 };

enum FeaturePrefix {
  FeatureUsed = '+',
  FeatureRequired = '=',
  FeatureDisallowed = '-'
};

} // namespace BinaryConsts

// (local index in IR, tuple index) => binary local index
using MappedLocals = std::unordered_map<std::pair<Index, Index>, size_t>;

// Writes out wasm to the binary format

class WasmBinaryWriter {
  // Computes the indexes in a wasm binary, i.e., with function imports
  // and function implementations sharing a single index space, etc.,
  // and with the imports first (the Module's functions and globals
  // arrays are not assumed to be in a particular order, so we can't
  // just use them directly).
  struct BinaryIndexes {
    std::unordered_map<Name, Index> functionIndexes;
    std::unordered_map<Name, Index> eventIndexes;
    std::unordered_map<Name, Index> globalIndexes;
    std::unordered_map<Name, Index> tableIndexes;
    std::unordered_map<Name, Index> elemIndexes;

    BinaryIndexes(Module& wasm) {
      auto addIndexes = [&](auto& source, auto& indexes) {
        auto addIndex = [&](auto* curr) {
          auto index = indexes.size();
          indexes[curr->name] = index;
        };
        for (auto& curr : source) {
          if (curr->imported()) {
            addIndex(curr.get());
          }
        }
        for (auto& curr : source) {
          if (!curr->imported()) {
            addIndex(curr.get());
          }
        }
      };
      addIndexes(wasm.functions, functionIndexes);
      addIndexes(wasm.events, eventIndexes);
      addIndexes(wasm.tables, tableIndexes);

      for (auto& curr : wasm.elementSegments) {
        auto index = elemIndexes.size();
        elemIndexes[curr->name] = index;
      }

      // Globals may have tuple types in the IR, in which case they lower to
      // multiple globals, one for each tuple element, in the binary. Tuple
      // globals therefore occupy multiple binary indices, and we have to take
      // that into account when calculating indices.
      Index globalCount = 0;
      auto addGlobal = [&](auto* curr) {
        globalIndexes[curr->name] = globalCount;
        globalCount += curr->type.size();
      };
      for (auto& curr : wasm.globals) {
        if (curr->imported()) {
          addGlobal(curr.get());
        }
      }
      for (auto& curr : wasm.globals) {
        if (!curr->imported()) {
          addGlobal(curr.get());
        }
      }
    }
  };

public:
  WasmBinaryWriter(Module* input, BufferWithRandomAccess& o)
    : wasm(input), o(o), indexes(*input) {
    prepare();
  }

  // locations in the output binary for the various parts of the module
  struct TableOfContents {
    struct Entry {
      Name name;
      size_t offset; // where the entry starts
      size_t size;   // the size of the entry
      Entry(Name name, size_t offset, size_t size)
        : name(name), offset(offset), size(size) {}
    };
    std::vector<Entry> functionBodies;
  } tableOfContents;

  void setNamesSection(bool set) {
    debugInfo = set;
    emitModuleName = set;
  }
  void setEmitModuleName(bool set) { emitModuleName = set; }
  void setSourceMap(std::ostream* set, std::string url) {
    sourceMap = set;
    sourceMapUrl = url;
  }
  void setSymbolMap(std::string set) { symbolMap = set; }

  void write();
  void writeHeader();
  int32_t writeU32LEBPlaceholder();
  void writeResizableLimits(
    Address initial, Address maximum, bool hasMaximum, bool shared, bool is64);
  template<typename T> int32_t startSection(T code);
  void finishSection(int32_t start);
  int32_t startSubsection(BinaryConsts::UserSections::Subsection code);
  void finishSubsection(int32_t start);
  void writeStart();
  void writeMemory();
  void writeTypes();
  void writeImports();

  void writeFunctionSignatures();
  void writeExpression(Expression* curr);
  void writeFunctions();
  void writeGlobals();
  void writeExports();
  void writeDataCount();
  void writeDataSegments();
  void writeEvents();

  uint32_t getFunctionIndex(Name name) const;
  uint32_t getTableIndex(Name name) const;
  uint32_t getElementSegmentIndex(Name name) const;
  uint32_t getGlobalIndex(Name name) const;
  uint32_t getEventIndex(Name name) const;
  uint32_t getTypeIndex(HeapType type) const;

  void writeTableDeclarations();
  void writeElementSegments();
  void writeNames();
  void writeSourceMapUrl();
  void writeSymbolMap();
  void writeLateUserSections();
  void writeUserSection(const UserSection& section);
  void writeFeaturesSection();
  void writeDylinkSection();

  void initializeDebugInfo();
  void writeSourceMapProlog();
  void writeSourceMapEpilog();
  void writeDebugLocation(const Function::DebugLocation& loc);
  void writeDebugLocation(Expression* curr, Function* func);
  void writeDebugLocationEnd(Expression* curr, Function* func);
  void writeExtraDebugLocation(Expression* curr, Function* func, size_t id);

  // helpers
  void writeInlineString(const char* name);
  void writeEscapedName(const char* name);
  void writeInlineBuffer(const char* data, size_t size);

  struct Buffer {
    const char* data;
    size_t size;
    size_t pointerLocation;
    Buffer(const char* data, size_t size, size_t pointerLocation)
      : data(data), size(size), pointerLocation(pointerLocation) {}
  };

  std::vector<Buffer> buffersToWrite;

  void emitBuffer(const char* data, size_t size);
  void emitString(const char* str);
  void finishUp();

  Module* getModule() { return wasm; }

  void writeType(Type type);

  // Writes an arbitrary heap type, which may be indexed or one of the
  // basic types like funcref.
  void writeHeapType(HeapType type);
  // Writes an indexed heap type. Note that this is encoded differently than a
  // general heap type because it does not allow negative values for basic heap
  // types.
  void writeIndexedHeapType(HeapType type);

  void writeField(const Field& field);

private:
  Module* wasm;
  BufferWithRandomAccess& o;
  BinaryIndexes indexes;
  std::unordered_map<HeapType, Index> typeIndices;
  std::vector<HeapType> types;

  bool debugInfo = true;

  // TODO: Remove `emitModuleName` in the future once there are better ways to
  // ensure modules have meaningful names in stack traces.For example, using
  // ObjectURLs works in FireFox, but not Chrome. See
  // https://bugs.chromium.org/p/v8/issues/detail?id=11808.
  bool emitModuleName = true;

  std::ostream* sourceMap = nullptr;
  std::string sourceMapUrl;
  std::string symbolMap;

  MixedArena allocator;

  // storage of source map locations until the section is placed at its final
  // location (shrinking LEBs may cause changes there)
  std::vector<std::pair<size_t, const Function::DebugLocation*>>
    sourceMapLocations;
  size_t sourceMapLocationsSizeAtSectionStart;
  Function::DebugLocation lastDebugLocation;

  std::unique_ptr<ImportInfo> importInfo;

  // General debugging info: track locations as we write.
  BinaryLocations binaryLocations;
  size_t binaryLocationsSizeAtSectionStart;
  // Track the expressions that we added for the current function being
  // written, so that we can update those specific binary locations when
  // the function is written out.
  std::vector<Expression*> binaryLocationTrackedExpressionsForFunc;

  // Maps function names to their mapped locals. This is used when we emit the
  // local names section: we map the locals when writing the function, save that
  // info here, and then use it when writing the names.
  std::unordered_map<Name, MappedLocals> funcMappedLocals;

  void prepare();
};

class WasmBinaryBuilder {
  Module& wasm;
  MixedArena& allocator;
  const std::vector<char>& input;
  std::istream* sourceMap;
  std::pair<uint32_t, Function::DebugLocation> nextDebugLocation;
  bool debugInfo = true;
  bool DWARF = false;
  bool skipFunctionBodies = false;

  size_t pos = 0;
  Index startIndex = -1;
  std::set<Function::DebugLocation> debugLocation;
  size_t codeSectionLocation;

  std::set<BinaryConsts::Section> seenSections;

  // All types defined in the type section
  std::vector<HeapType> types;

public:
  WasmBinaryBuilder(Module& wasm, const std::vector<char>& input)
    : wasm(wasm), allocator(wasm.allocator), input(input), sourceMap(nullptr),
      nextDebugLocation(0, {0, 0, 0}), debugLocation() {}

  void setDebugInfo(bool value) { debugInfo = value; }
  void setDWARF(bool value) { DWARF = value; }
  void setSkipFunctionBodies(bool skipFunctionBodies_) {
    skipFunctionBodies = skipFunctionBodies_;
  }
  void read();
  void readUserSection(size_t payloadLen);

  bool more() { return pos < input.size(); }

  std::pair<const char*, const char*> getByteView(size_t size);
  uint8_t getInt8();
  uint16_t getInt16();
  uint32_t getInt32();
  uint64_t getInt64();
  uint8_t getLaneIndex(size_t lanes);
  // it is unsafe to return a float directly, due to ABI issues with the
  // signalling bit
  Literal getFloat32Literal();
  Literal getFloat64Literal();
  Literal getVec128Literal();
  uint32_t getU32LEB();
  uint64_t getU64LEB();
  int32_t getS32LEB();
  int64_t getS64LEB();
  uint64_t getUPtrLEB();

  bool getBasicType(int32_t code, Type& out);
  bool getBasicHeapType(int64_t code, HeapType& out);
  // Read a value and get a type for it.
  Type getType();
  // Get a type given the initial S32LEB has already been read, and is provided.
  Type getType(int initial);
  HeapType getHeapType();
  HeapType getIndexedHeapType();

  Type getConcreteType();
  Name getInlineString();
  void verifyInt8(int8_t x);
  void verifyInt16(int16_t x);
  void verifyInt32(int32_t x);
  void verifyInt64(int64_t x);
  void readHeader();
  void readStart();
  void readMemory();
  void readTypes();

  // gets a name in the combined import+defined space
  Name getFunctionName(Index index);
  Name getTableName(Index index);
  Name getElementSegmentName(Index index);
  Name getGlobalName(Index index);
  Name getEventName(Index index);

  Table* getTable(Index index);
  ElementSegment* getElementSegment(Index index);

  void getResizableLimits(Address& initial,
                          Address& max,
                          bool& shared,
                          Type& indexType,
                          Address defaultIfNoMax);
  void readImports();

  // The signatures of each function, including imported functions, given in the
  // import and function sections. Store HeapTypes instead of Signatures because
  // reconstructing the HeapTypes from the Signatures is expensive.
  std::vector<HeapType> functionTypes;

  void readFunctionSignatures();
  HeapType getTypeByIndex(Index index);
  HeapType getTypeByFunctionIndex(Index index);
  Signature getSignatureByTypeIndex(Index index);
  Signature getSignatureByFunctionIndex(Index index);

  size_t nextLabel;

  Name getNextLabel();

  // We read functions and globals before we know their names, so we need to
  // backpatch the names later

  // we store functions here before wasm.addFunction after we know their names
  std::vector<Function*> functions;
  // we store function imports here before wasm.addFunctionImport after we know
  // their names
  std::vector<Function*> functionImports;
  // at index i we have all refs to the function i
  std::map<Index, std::vector<Expression*>> functionRefs;
  Function* currFunction = nullptr;
  // before we see a function (like global init expressions), there is no end of
  // function to check
  Index endOfFunction = -1;

  // we store tables here before wasm.addTable after we know their names
  std::vector<std::unique_ptr<Table>> tables;
  // we store table imports here before wasm.addTableImport after we know
  // their names
  std::vector<Table*> tableImports;
  // at index i we have all references to the table i
  std::map<Index, std::vector<Name*>> tableRefs;

  std::map<Index, Name> elemTables;

  // we store elems here after being read from binary, until when we know their
  // names
  std::vector<std::unique_ptr<ElementSegment>> elementSegments;
  // at index i we have all references to the element segment i
  std::map<Index, std::vector<Expression*>> elemRefs;

  // we store globals here before wasm.addGlobal after we know their names
  std::vector<std::unique_ptr<Global>> globals;
  // we store global imports here before wasm.addGlobalImport after we know
  // their names
  std::vector<Global*> globalImports;
  // at index i we have all refs to the global i
  std::map<Index, std::vector<Expression*>> globalRefs;

  // Throws a parsing error if we are not in a function context
  void requireFunctionContext(const char* error);

  void readFunctions();
  void readVars();

  std::map<Export*, Index> exportIndices;
  std::vector<Export*> exportOrder;
  void readExports();

  Expression* readExpression();
  void readGlobals();

  struct BreakTarget {
    Name name;
    Type type;
    BreakTarget(Name name, Type type) : name(name), type(type) {}
  };
  std::vector<BreakTarget> breakStack;
  // the names that breaks target. this lets us know if a block has breaks to it
  // or not.
  std::unordered_set<Name> breakTargetNames;
  // the names that delegates target.
  std::unordered_set<Name> exceptionTargetNames;

  std::vector<Expression*> expressionStack;

  // Each let block in the binary adds new locals to the bottom of the index
  // space. That is, all previously-existing indexes are bumped to higher
  // indexes. getAbsoluteLocalIndex does this computation.
  // Note that we must track not just the number of locals added in each let,
  // but also the absolute index from which they were allocated, as binaryen
  // will add new locals as it goes for things like stacky code and tuples (so
  // there isn't a simple way to get to the absolute index from a relative one).
  // Hence each entry here is a pair of the number of items, and the absolute
  // index they begin at.
  struct LetData {
    // How many items are defined in this let.
    Index num;
    // The absolute index from which they are allocated from. That is, if num is
    // 5 and absoluteStart is 10, then we use indexes 10-14.
    Index absoluteStart;
  };
  std::vector<LetData> letStack;

  // Given a relative index of a local (the one used in the wasm binary), get
  // the absolute one which takes into account lets, and is the one used in
  // Binaryen IR.
  Index getAbsoluteLocalIndex(Index index);

  // Control flow structure parsing: these have not just the normal binary
  // data for an instruction, but also some bytes later on like "end" or "else".
  // We must be aware of the connection between those things, for debug info.
  std::vector<Expression*> controlFlowStack;

  // Called when we parse the beginning of a control flow structure.
  void startControlFlow(Expression* curr);

  // set when we know code is unreachable in the sense of the wasm spec: we are
  // in a block and after an unreachable element. this helps parse stacky wasm
  // code, which can be unsuitable for our IR when unreachable.
  bool unreachableInTheWasmSense;

  // set when the current code being processed will not be emitted in the
  // output, which is the case when it is literally unreachable, for example,
  // (block $a
  //   (unreachable)
  //   (block $b
  //     ;; code here is reachable in the wasm sense, even though $b as a whole
  //     ;; is not
  //     (unreachable)
  //     ;; code here is unreachable in the wasm sense
  //   )
  // )
  bool willBeIgnored;

  BinaryConsts::ASTNodes lastSeparator = BinaryConsts::End;

  // process a block-type scope, until an end or else marker, or the end of the
  // function
  void processExpressions();
  void skipUnreachableCode();

  void pushExpression(Expression* curr);
  Expression* popExpression();
  Expression* popNonVoidExpression();
  Expression* popTuple(size_t numElems);
  Expression* popTypedExpression(Type type);

  void validateBinary(); // validations that cannot be performed on the Module
  void processNames();

  size_t dataCount = 0;
  bool hasDataCount = false;

  void readDataSegments();
  void readDataCount();

  void readTableDeclarations();
  void readElementSegments();

  void readEvents();

  static Name escape(Name name);
  void readNames(size_t);
  void readFeatures(size_t);
  void readDylink(size_t);

  // Debug information reading helpers
  void setDebugLocations(std::istream* sourceMap_) { sourceMap = sourceMap_; }
  std::unordered_map<std::string, Index> debugInfoFileIndices;
  void readNextDebugLocation();
  void readSourceMapHeader();

  // AST reading
  int depth = 0; // only for debugging

  BinaryConsts::ASTNodes readExpression(Expression*& curr);
  void pushBlockElements(Block* curr, Type type, size_t start);
  void visitBlock(Block* curr);

  // Gets a block of expressions. If it's just one, return that singleton.
  Expression* getBlockOrSingleton(Type type);

  BreakTarget getBreakTarget(int32_t offset);
  Name getExceptionTargetName(int32_t offset);

  void readMemoryAccess(Address& alignment, Address& offset);

  void visitIf(If* curr);
  void visitLoop(Loop* curr);
  void visitBreak(Break* curr, uint8_t code);
  void visitSwitch(Switch* curr);
  void visitCall(Call* curr);
  void visitCallIndirect(CallIndirect* curr);
  void visitLocalGet(LocalGet* curr);
  void visitLocalSet(LocalSet* curr, uint8_t code);
  void visitGlobalGet(GlobalGet* curr);
  void visitGlobalSet(GlobalSet* curr);
  bool maybeVisitLoad(Expression*& out, uint8_t code, bool isAtomic);
  bool maybeVisitStore(Expression*& out, uint8_t code, bool isAtomic);
  bool maybeVisitNontrappingTrunc(Expression*& out, uint32_t code);
  bool maybeVisitAtomicRMW(Expression*& out, uint8_t code);
  bool maybeVisitAtomicCmpxchg(Expression*& out, uint8_t code);
  bool maybeVisitAtomicWait(Expression*& out, uint8_t code);
  bool maybeVisitAtomicNotify(Expression*& out, uint8_t code);
  bool maybeVisitAtomicFence(Expression*& out, uint8_t code);
  bool maybeVisitConst(Expression*& out, uint8_t code);
  bool maybeVisitUnary(Expression*& out, uint8_t code);
  bool maybeVisitBinary(Expression*& out, uint8_t code);
  bool maybeVisitTruncSat(Expression*& out, uint32_t code);
  bool maybeVisitSIMDBinary(Expression*& out, uint32_t code);
  bool maybeVisitSIMDUnary(Expression*& out, uint32_t code);
  bool maybeVisitSIMDConst(Expression*& out, uint32_t code);
  bool maybeVisitSIMDStore(Expression*& out, uint32_t code);
  bool maybeVisitSIMDExtract(Expression*& out, uint32_t code);
  bool maybeVisitSIMDReplace(Expression*& out, uint32_t code);
  bool maybeVisitSIMDShuffle(Expression*& out, uint32_t code);
  bool maybeVisitSIMDTernary(Expression*& out, uint32_t code);
  bool maybeVisitSIMDShift(Expression*& out, uint32_t code);
  bool maybeVisitSIMDLoad(Expression*& out, uint32_t code);
  bool maybeVisitSIMDLoadStoreLane(Expression*& out, uint32_t code);
  bool maybeVisitTableSize(Expression*& out, uint32_t code);
  bool maybeVisitTableGrow(Expression*& out, uint32_t code);
  bool maybeVisitTableFill(Expression*& out, uint32_t code);
  bool maybeVisitTableCopy(Expression*& out, uint32_t code);
  bool maybeVisitTableInit(Expression*& out, uint32_t code);
  bool maybeVisitElemDrop(Expression*& out, uint32_t code);
  bool maybeVisitMemoryInit(Expression*& out, uint32_t code);
  bool maybeVisitDataDrop(Expression*& out, uint32_t code);
  bool maybeVisitMemoryCopy(Expression*& out, uint32_t code);
  bool maybeVisitMemoryFill(Expression*& out, uint32_t code);
  bool maybeVisitI31New(Expression*& out, uint32_t code);
  bool maybeVisitI31Get(Expression*& out, uint32_t code);
  bool maybeVisitRefTest(Expression*& out, uint32_t code);
  bool maybeVisitRefCast(Expression*& out, uint32_t code);
  bool maybeVisitBrOn(Expression*& out, uint32_t code);
  bool maybeVisitRttCanon(Expression*& out, uint32_t code);
  bool maybeVisitRttSub(Expression*& out, uint32_t code);
  bool maybeVisitStructNew(Expression*& out, uint32_t code);
  bool maybeVisitStructGet(Expression*& out, uint32_t code);
  bool maybeVisitStructSet(Expression*& out, uint32_t code);
  bool maybeVisitArrayNew(Expression*& out, uint32_t code);
  bool maybeVisitArrayGet(Expression*& out, uint32_t code);
  bool maybeVisitArraySet(Expression*& out, uint32_t code);
  bool maybeVisitArrayLen(Expression*& out, uint32_t code);
  bool maybeVisitArrayCopy(Expression*& out, uint32_t code);
  void visitSelect(Select* curr, uint8_t code);
  void visitReturn(Return* curr);
  void visitTableGet(TableGet* curr);
  void visitTableSet(TableSet* curr);
  void visitMemorySize(MemorySize* curr);
  void visitMemoryGrow(MemoryGrow* curr);
  void visitNop(Nop* curr);
  void visitUnreachable(Unreachable* curr);
  void visitDrop(Drop* curr);
  void visitRefNull(RefNull* curr);
  void visitRefIs(RefIs* curr, uint8_t code);
  void visitRefFunc(RefFunc* curr);
  void visitRefEq(RefEq* curr);
  void visitTryOrTryInBlock(Expression*& out);
  void visitThrow(Throw* curr);
  void visitRethrow(Rethrow* curr);
  void visitCallRef(CallRef* curr);
  void visitRefAs(RefAs* curr, uint8_t code);
  // Let is lowered into a block.
  void visitLet(Block* curr);

  void throwError(std::string text);

  // Struct/Array instructions have an unnecessary heap type that is just for
  // validation (except for the case of unreachability, but that's not a problem
  // anyhow, we can ignore it there). That is, we also have a reference / rtt
  // child from which we can infer the type anyhow, and we just need to check
  // that type is the same.
  void validateHeapTypeUsingChild(Expression* child, HeapType heapType);

private:
  bool hasDWARFSections();
};

} // namespace wasm

#undef DEBUG_TYPE

#endif // wasm_wasm_binary_h
