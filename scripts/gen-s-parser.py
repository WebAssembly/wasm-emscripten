#!/usr/bin/env python3
#
# Copyright 2018 WebAssembly Community Group participants
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys

instructions = [
    ("unreachable",    "makeUnreachable()"),
    ("nop",            "makeNop()"),
    ("block",          "makeBlock()"),
    ("loop",           "makeLoop()"),
    ("if",             "makeIf()"),
    ("then",           "makeThenOrElse()"),
    ("else",           "makeThenOrElse()"),
    ("br",             "makeBreak(false)"),
    ("br_if",          "makeBreak(true)"),
    ("br_table",       "makeBreakTable()"),
    ("return",         "makeReturn()"),
    ("call",           "makeCall(/*isReturn=*/false)"),
    ("call_indirect",  "makeCallIndirect(/*isReturn=*/false)"),
    ("return_call",    "makeCall(/*isReturn=*/true)"),
    ("return_call_indirect", "makeCallIndirect(/*isReturn=*/true)"),
    ("drop",           "makeDrop()"),
    ("select",         "makeSelect()"),
    ("local.get",      "makeLocalGet()"),
    ("local.set",      "makeLocalSet()"),
    ("local.tee",      "makeLocalTee()"),
    ("global.get",     "makeGlobalGet()"),
    ("global.set",     "makeGlobalSet()"),
    ("memory.init",    "makeMemoryInit()"),
    ("data.drop",      "makeDataDrop()"),
    ("memory.copy",    "makeMemoryCopy()"),
    ("memory.fill",    "makeMemoryFill()"),
    ("i32.load",       "makeLoad(Type::i32, /*signed=*/false, 4, /*isAtomic=*/false)"),
    ("i64.load",       "makeLoad(Type::i64, /*signed=*/false, 8, /*isAtomic=*/false)"),
    ("f32.load",       "makeLoad(Type::f32, /*signed=*/false, 4, /*isAtomic=*/false)"),
    ("f32.load_f16",   "makeLoad(Type::f32, /*signed=*/false, 2, /*isAtomic=*/false)"),
    ("f64.load",       "makeLoad(Type::f64, /*signed=*/false, 8, /*isAtomic=*/false)"),
    ("i32.load8_s",    "makeLoad(Type::i32, /*signed=*/true, 1, /*isAtomic=*/false)"),
    ("i32.load8_u",    "makeLoad(Type::i32, /*signed=*/false, 1, /*isAtomic=*/false)"),
    ("i32.load16_s",   "makeLoad(Type::i32, /*signed=*/true, 2, /*isAtomic=*/false)"),
    ("i32.load16_u",   "makeLoad(Type::i32, /*signed=*/false, 2, /*isAtomic=*/false)"),
    ("i64.load8_s",    "makeLoad(Type::i64, /*signed=*/true, 1, /*isAtomic=*/false)"),
    ("i64.load8_u",    "makeLoad(Type::i64, /*signed=*/false, 1, /*isAtomic=*/false)"),
    ("i64.load16_s",   "makeLoad(Type::i64, /*signed=*/true, 2, /*isAtomic=*/false)"),
    ("i64.load16_u",   "makeLoad(Type::i64, /*signed=*/false, 2, /*isAtomic=*/false)"),
    ("i64.load32_s",   "makeLoad(Type::i64, /*signed=*/true, 4, /*isAtomic=*/false)"),
    ("i64.load32_u",   "makeLoad(Type::i64, /*signed=*/false, 4, /*isAtomic=*/false)"),
    ("i32.store",      "makeStore(Type::i32, 4, /*isAtomic=*/false)"),
    ("i64.store",      "makeStore(Type::i64, 8, /*isAtomic=*/false)"),
    ("f32.store",      "makeStore(Type::f32, 4, /*isAtomic=*/false)"),
    ("f32.store_f16",  "makeStore(Type::f32, 2, /*isAtomic=*/false)"),
    ("f64.store",      "makeStore(Type::f64, 8, /*isAtomic=*/false)"),
    ("i32.store8",     "makeStore(Type::i32, 1, /*isAtomic=*/false)"),
    ("i32.store16",    "makeStore(Type::i32, 2, /*isAtomic=*/false)"),
    ("i64.store8",     "makeStore(Type::i64, 1, /*isAtomic=*/false)"),
    ("i64.store16",    "makeStore(Type::i64, 2, /*isAtomic=*/false)"),
    ("i64.store32",    "makeStore(Type::i64, 4, /*isAtomic=*/false)"),
    ("memory.size",    "makeMemorySize()"),
    ("memory.grow",    "makeMemoryGrow()"),
    ("i32.const",      "makeConst(Type::i32)"),
    ("i64.const",      "makeConst(Type::i64)"),
    ("f32.const",      "makeConst(Type::f32)"),
    ("f64.const",      "makeConst(Type::f64)"),
    ("i32.eqz",        "makeUnary(UnaryOp::EqZInt32)"),
    ("i32.eq",         "makeBinary(BinaryOp::EqInt32)"),
    ("i32.ne",         "makeBinary(BinaryOp::NeInt32)"),
    ("i32.lt_s",       "makeBinary(BinaryOp::LtSInt32)"),
    ("i32.lt_u",       "makeBinary(BinaryOp::LtUInt32)"),
    ("i32.gt_s",       "makeBinary(BinaryOp::GtSInt32)"),
    ("i32.gt_u",       "makeBinary(BinaryOp::GtUInt32)"),
    ("i32.le_s",       "makeBinary(BinaryOp::LeSInt32)"),
    ("i32.le_u",       "makeBinary(BinaryOp::LeUInt32)"),
    ("i32.ge_s",       "makeBinary(BinaryOp::GeSInt32)"),
    ("i32.ge_u",       "makeBinary(BinaryOp::GeUInt32)"),
    ("i64.eqz",        "makeUnary(UnaryOp::EqZInt64)"),
    ("i64.eq",         "makeBinary(BinaryOp::EqInt64)"),
    ("i64.ne",         "makeBinary(BinaryOp::NeInt64)"),
    ("i64.lt_s",       "makeBinary(BinaryOp::LtSInt64)"),
    ("i64.lt_u",       "makeBinary(BinaryOp::LtUInt64)"),
    ("i64.gt_s",       "makeBinary(BinaryOp::GtSInt64)"),
    ("i64.gt_u",       "makeBinary(BinaryOp::GtUInt64)"),
    ("i64.le_s",       "makeBinary(BinaryOp::LeSInt64)"),
    ("i64.le_u",       "makeBinary(BinaryOp::LeUInt64)"),
    ("i64.ge_s",       "makeBinary(BinaryOp::GeSInt64)"),
    ("i64.ge_u",       "makeBinary(BinaryOp::GeUInt64)"),
    ("f32.eq",         "makeBinary(BinaryOp::EqFloat32)"),
    ("f32.ne",         "makeBinary(BinaryOp::NeFloat32)"),
    ("f32.lt",         "makeBinary(BinaryOp::LtFloat32)"),
    ("f32.gt",         "makeBinary(BinaryOp::GtFloat32)"),
    ("f32.le",         "makeBinary(BinaryOp::LeFloat32)"),
    ("f32.ge",         "makeBinary(BinaryOp::GeFloat32)"),
    ("f64.eq",         "makeBinary(BinaryOp::EqFloat64)"),
    ("f64.ne",         "makeBinary(BinaryOp::NeFloat64)"),
    ("f64.lt",         "makeBinary(BinaryOp::LtFloat64)"),
    ("f64.gt",         "makeBinary(BinaryOp::GtFloat64)"),
    ("f64.le",         "makeBinary(BinaryOp::LeFloat64)"),
    ("f64.ge",         "makeBinary(BinaryOp::GeFloat64)"),
    ("i32.clz",        "makeUnary(UnaryOp::ClzInt32)"),
    ("i32.ctz",        "makeUnary(UnaryOp::CtzInt32)"),
    ("i32.popcnt",     "makeUnary(UnaryOp::PopcntInt32)"),
    ("i32.add",        "makeBinary(BinaryOp::AddInt32)"),
    ("i32.sub",        "makeBinary(BinaryOp::SubInt32)"),
    ("i32.mul",        "makeBinary(BinaryOp::MulInt32)"),
    ("i32.div_s",      "makeBinary(BinaryOp::DivSInt32)"),
    ("i32.div_u",      "makeBinary(BinaryOp::DivUInt32)"),
    ("i32.rem_s",      "makeBinary(BinaryOp::RemSInt32)"),
    ("i32.rem_u",      "makeBinary(BinaryOp::RemUInt32)"),
    ("i32.and",        "makeBinary(BinaryOp::AndInt32)"),
    ("i32.or",         "makeBinary(BinaryOp::OrInt32)"),
    ("i32.xor",        "makeBinary(BinaryOp::XorInt32)"),
    ("i32.shl",        "makeBinary(BinaryOp::ShlInt32)"),
    ("i32.shr_s",      "makeBinary(BinaryOp::ShrSInt32)"),
    ("i32.shr_u",      "makeBinary(BinaryOp::ShrUInt32)"),
    ("i32.rotl",       "makeBinary(BinaryOp::RotLInt32)"),
    ("i32.rotr",       "makeBinary(BinaryOp::RotRInt32)"),
    ("i64.clz",        "makeUnary(UnaryOp::ClzInt64)"),
    ("i64.ctz",        "makeUnary(UnaryOp::CtzInt64)"),
    ("i64.popcnt",     "makeUnary(UnaryOp::PopcntInt64)"),
    ("i64.add",        "makeBinary(BinaryOp::AddInt64)"),
    ("i64.sub",        "makeBinary(BinaryOp::SubInt64)"),
    ("i64.mul",        "makeBinary(BinaryOp::MulInt64)"),
    ("i64.div_s",      "makeBinary(BinaryOp::DivSInt64)"),
    ("i64.div_u",      "makeBinary(BinaryOp::DivUInt64)"),
    ("i64.rem_s",      "makeBinary(BinaryOp::RemSInt64)"),
    ("i64.rem_u",      "makeBinary(BinaryOp::RemUInt64)"),
    ("i64.and",        "makeBinary(BinaryOp::AndInt64)"),
    ("i64.or",         "makeBinary(BinaryOp::OrInt64)"),
    ("i64.xor",        "makeBinary(BinaryOp::XorInt64)"),
    ("i64.shl",        "makeBinary(BinaryOp::ShlInt64)"),
    ("i64.shr_s",      "makeBinary(BinaryOp::ShrSInt64)"),
    ("i64.shr_u",      "makeBinary(BinaryOp::ShrUInt64)"),
    ("i64.rotl",       "makeBinary(BinaryOp::RotLInt64)"),
    ("i64.rotr",       "makeBinary(BinaryOp::RotRInt64)"),
    ("f32.abs",        "makeUnary(UnaryOp::AbsFloat32)"),
    ("f32.neg",        "makeUnary(UnaryOp::NegFloat32)"),
    ("f32.ceil",       "makeUnary(UnaryOp::CeilFloat32)"),
    ("f32.floor",      "makeUnary(UnaryOp::FloorFloat32)"),
    ("f32.trunc",      "makeUnary(UnaryOp::TruncFloat32)"),
    ("f32.nearest",    "makeUnary(UnaryOp::NearestFloat32)"),
    ("f32.sqrt",       "makeUnary(UnaryOp::SqrtFloat32)"),
    ("f32.add",        "makeBinary(BinaryOp::AddFloat32)"),
    ("f32.sub",        "makeBinary(BinaryOp::SubFloat32)"),
    ("f32.mul",        "makeBinary(BinaryOp::MulFloat32)"),
    ("f32.div",        "makeBinary(BinaryOp::DivFloat32)"),
    ("f32.min",        "makeBinary(BinaryOp::MinFloat32)"),
    ("f32.max",        "makeBinary(BinaryOp::MaxFloat32)"),
    ("f32.copysign",   "makeBinary(BinaryOp::CopySignFloat32)"),
    ("f64.abs",        "makeUnary(UnaryOp::AbsFloat64)"),
    ("f64.neg",        "makeUnary(UnaryOp::NegFloat64)"),
    ("f64.ceil",       "makeUnary(UnaryOp::CeilFloat64)"),
    ("f64.floor",      "makeUnary(UnaryOp::FloorFloat64)"),
    ("f64.trunc",      "makeUnary(UnaryOp::TruncFloat64)"),
    ("f64.nearest",    "makeUnary(UnaryOp::NearestFloat64)"),
    ("f64.sqrt",       "makeUnary(UnaryOp::SqrtFloat64)"),
    ("f64.add",        "makeBinary(BinaryOp::AddFloat64)"),
    ("f64.sub",        "makeBinary(BinaryOp::SubFloat64)"),
    ("f64.mul",        "makeBinary(BinaryOp::MulFloat64)"),
    ("f64.div",        "makeBinary(BinaryOp::DivFloat64)"),
    ("f64.min",        "makeBinary(BinaryOp::MinFloat64)"),
    ("f64.max",        "makeBinary(BinaryOp::MaxFloat64)"),
    ("f64.copysign",   "makeBinary(BinaryOp::CopySignFloat64)"),
    ("i32.wrap_i64",   "makeUnary(UnaryOp::WrapInt64)"),
    ("i32.trunc_f32_s",     "makeUnary(UnaryOp::TruncSFloat32ToInt32)"),
    ("i32.trunc_f32_u",     "makeUnary(UnaryOp::TruncUFloat32ToInt32)"),
    ("i32.trunc_f64_s",     "makeUnary(UnaryOp::TruncSFloat64ToInt32)"),
    ("i32.trunc_f64_u",     "makeUnary(UnaryOp::TruncUFloat64ToInt32)"),
    ("i64.extend_i32_s",    "makeUnary(UnaryOp::ExtendSInt32)"),
    ("i64.extend_i32_u",    "makeUnary(UnaryOp::ExtendUInt32)"),
    ("i64.trunc_f32_s",     "makeUnary(UnaryOp::TruncSFloat32ToInt64)"),
    ("i64.trunc_f32_u",     "makeUnary(UnaryOp::TruncUFloat32ToInt64)"),
    ("i64.trunc_f64_s",     "makeUnary(UnaryOp::TruncSFloat64ToInt64)"),
    ("i64.trunc_f64_u",     "makeUnary(UnaryOp::TruncUFloat64ToInt64)"),
    ("f32.convert_i32_s",   "makeUnary(UnaryOp::ConvertSInt32ToFloat32)"),
    ("f32.convert_i32_u",   "makeUnary(UnaryOp::ConvertUInt32ToFloat32)"),
    ("f32.convert_i64_s",   "makeUnary(UnaryOp::ConvertSInt64ToFloat32)"),
    ("f32.convert_i64_u",   "makeUnary(UnaryOp::ConvertUInt64ToFloat32)"),
    ("f32.demote_f64",      "makeUnary(UnaryOp::DemoteFloat64)"),
    ("f64.convert_i32_s",   "makeUnary(UnaryOp::ConvertSInt32ToFloat64)"),
    ("f64.convert_i32_u",   "makeUnary(UnaryOp::ConvertUInt32ToFloat64)"),
    ("f64.convert_i64_s",   "makeUnary(UnaryOp::ConvertSInt64ToFloat64)"),
    ("f64.convert_i64_u",   "makeUnary(UnaryOp::ConvertUInt64ToFloat64)"),
    ("f64.promote_f32",     "makeUnary(UnaryOp::PromoteFloat32)"),
    ("i32.reinterpret_f32", "makeUnary(UnaryOp::ReinterpretFloat32)"),
    ("i64.reinterpret_f64", "makeUnary(UnaryOp::ReinterpretFloat64)"),
    ("f32.reinterpret_i32", "makeUnary(UnaryOp::ReinterpretInt32)"),
    ("f64.reinterpret_i64", "makeUnary(UnaryOp::ReinterpretInt64)"),
    ("i32.extend8_s",       "makeUnary(UnaryOp::ExtendS8Int32)"),
    ("i32.extend16_s",      "makeUnary(UnaryOp::ExtendS16Int32)"),
    ("i64.extend8_s",       "makeUnary(UnaryOp::ExtendS8Int64)"),
    ("i64.extend16_s",      "makeUnary(UnaryOp::ExtendS16Int64)"),
    ("i64.extend32_s",      "makeUnary(UnaryOp::ExtendS32Int64)"),
    # atomic instructions
    ("memory.atomic.notify",    "makeAtomicNotify()"),
    ("memory.atomic.wait32",    "makeAtomicWait(Type::i32)"),
    ("memory.atomic.wait64",    "makeAtomicWait(Type::i64)"),
    ("atomic.fence",            "makeAtomicFence()"),
    ("i32.atomic.load8_u",      "makeLoad(Type::i32, /*signed=*/false, 1, /*isAtomic=*/true)"),
    ("i32.atomic.load16_u",     "makeLoad(Type::i32, /*signed=*/false, 2, /*isAtomic=*/true)"),
    ("i32.atomic.load",         "makeLoad(Type::i32, /*signed=*/false, 4, /*isAtomic=*/true)"),
    ("i64.atomic.load8_u",      "makeLoad(Type::i64, /*signed=*/false, 1, /*isAtomic=*/true)"),
    ("i64.atomic.load16_u",     "makeLoad(Type::i64, /*signed=*/false, 2, /*isAtomic=*/true)"),
    ("i64.atomic.load32_u",     "makeLoad(Type::i64, /*signed=*/false, 4, /*isAtomic=*/true)"),
    ("i64.atomic.load",         "makeLoad(Type::i64, /*signed=*/false, 8, /*isAtomic=*/true)"),
    ("i32.atomic.store8",       "makeStore(Type::i32, 1, /*isAtomic=*/true)"),
    ("i32.atomic.store16",      "makeStore(Type::i32, 2, /*isAtomic=*/true)"),
    ("i32.atomic.store",        "makeStore(Type::i32, 4, /*isAtomic=*/true)"),
    ("i64.atomic.store8",       "makeStore(Type::i64, 1, /*isAtomic=*/true)"),
    ("i64.atomic.store16",      "makeStore(Type::i64, 2, /*isAtomic=*/true)"),
    ("i64.atomic.store32",      "makeStore(Type::i64, 4, /*isAtomic=*/true)"),
    ("i64.atomic.store",        "makeStore(Type::i64, 8, /*isAtomic=*/true)"),
    ("i32.atomic.rmw8.add_u",   "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i32, 1)"),
    ("i32.atomic.rmw16.add_u",  "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i32, 2)"),
    ("i32.atomic.rmw.add",      "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i32, 4)"),
    ("i64.atomic.rmw8.add_u",   "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i64, 1)"),
    ("i64.atomic.rmw16.add_u",  "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i64, 2)"),
    ("i64.atomic.rmw32.add_u",  "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i64, 4)"),
    ("i64.atomic.rmw.add",      "makeAtomicRMW(AtomicRMWOp::RMWAdd, Type::i64, 8)"),
    ("i32.atomic.rmw8.sub_u",   "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i32, 1)"),
    ("i32.atomic.rmw16.sub_u",  "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i32, 2)"),
    ("i32.atomic.rmw.sub",      "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i32, 4)"),
    ("i64.atomic.rmw8.sub_u",   "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i64, 1)"),
    ("i64.atomic.rmw16.sub_u",  "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i64, 2)"),
    ("i64.atomic.rmw32.sub_u",  "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i64, 4)"),
    ("i64.atomic.rmw.sub",      "makeAtomicRMW(AtomicRMWOp::RMWSub, Type::i64, 8)"),
    ("i32.atomic.rmw8.and_u",   "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i32, 1)"),
    ("i32.atomic.rmw16.and_u",  "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i32, 2)"),
    ("i32.atomic.rmw.and",      "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i32, 4)"),
    ("i64.atomic.rmw8.and_u",   "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i64, 1)"),
    ("i64.atomic.rmw16.and_u",  "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i64, 2)"),
    ("i64.atomic.rmw32.and_u",  "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i64, 4)"),
    ("i64.atomic.rmw.and",      "makeAtomicRMW(AtomicRMWOp::RMWAnd, Type::i64, 8)"),
    ("i32.atomic.rmw8.or_u",    "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i32, 1)"),
    ("i32.atomic.rmw16.or_u",   "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i32, 2)"),
    ("i32.atomic.rmw.or",       "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i32, 4)"),
    ("i64.atomic.rmw8.or_u",    "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i64, 1)"),
    ("i64.atomic.rmw16.or_u",   "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i64, 2)"),
    ("i64.atomic.rmw32.or_u",   "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i64, 4)"),
    ("i64.atomic.rmw.or",       "makeAtomicRMW(AtomicRMWOp::RMWOr, Type::i64, 8)"),
    ("i32.atomic.rmw8.xor_u",   "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i32, 1)"),
    ("i32.atomic.rmw16.xor_u",  "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i32, 2)"),
    ("i32.atomic.rmw.xor",      "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i32, 4)"),
    ("i64.atomic.rmw8.xor_u",   "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i64, 1)"),
    ("i64.atomic.rmw16.xor_u",  "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i64, 2)"),
    ("i64.atomic.rmw32.xor_u",  "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i64, 4)"),
    ("i64.atomic.rmw.xor",      "makeAtomicRMW(AtomicRMWOp::RMWXor, Type::i64, 8)"),
    ("i32.atomic.rmw8.xchg_u",  "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i32, 1)"),
    ("i32.atomic.rmw16.xchg_u", "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i32, 2)"),
    ("i32.atomic.rmw.xchg",     "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i32, 4)"),
    ("i64.atomic.rmw8.xchg_u",  "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i64, 1)"),
    ("i64.atomic.rmw16.xchg_u", "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i64, 2)"),
    ("i64.atomic.rmw32.xchg_u", "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i64, 4)"),
    ("i64.atomic.rmw.xchg",     "makeAtomicRMW(AtomicRMWOp::RMWXchg, Type::i64, 8)"),
    ("i32.atomic.rmw8.cmpxchg_u",  "makeAtomicCmpxchg(Type::i32, 1)"),
    ("i32.atomic.rmw16.cmpxchg_u", "makeAtomicCmpxchg(Type::i32, 2)"),
    ("i32.atomic.rmw.cmpxchg",     "makeAtomicCmpxchg(Type::i32, 4)"),
    ("i64.atomic.rmw8.cmpxchg_u",  "makeAtomicCmpxchg(Type::i64, 1)"),
    ("i64.atomic.rmw16.cmpxchg_u", "makeAtomicCmpxchg(Type::i64, 2)"),
    ("i64.atomic.rmw32.cmpxchg_u", "makeAtomicCmpxchg(Type::i64, 4)"),
    ("i64.atomic.rmw.cmpxchg",     "makeAtomicCmpxchg(Type::i64, 8)"),
    # nontrapping float-to-int instructions
    ("i32.trunc_sat_f32_s", "makeUnary(UnaryOp::TruncSatSFloat32ToInt32)"),
    ("i32.trunc_sat_f32_u", "makeUnary(UnaryOp::TruncSatUFloat32ToInt32)"),
    ("i32.trunc_sat_f64_s", "makeUnary(UnaryOp::TruncSatSFloat64ToInt32)"),
    ("i32.trunc_sat_f64_u", "makeUnary(UnaryOp::TruncSatUFloat64ToInt32)"),
    ("i64.trunc_sat_f32_s", "makeUnary(UnaryOp::TruncSatSFloat32ToInt64)"),
    ("i64.trunc_sat_f32_u", "makeUnary(UnaryOp::TruncSatUFloat32ToInt64)"),
    ("i64.trunc_sat_f64_s", "makeUnary(UnaryOp::TruncSatSFloat64ToInt64)"),
    ("i64.trunc_sat_f64_u", "makeUnary(UnaryOp::TruncSatUFloat64ToInt64)"),
    # SIMD ops
    ("v128.load",            "makeLoad(Type::v128, /*signed=*/false, 16, /*isAtomic=*/false)"),
    ("v128.store",           "makeStore(Type::v128, 16, /*isAtomic=*/false)"),
    ("v128.const",           "makeConst(Type::v128)"),
    ("i8x16.shuffle",        "makeSIMDShuffle()"),
    ("i8x16.splat",          "makeUnary(UnaryOp::SplatVecI8x16)"),
    ("i8x16.extract_lane_s", "makeSIMDExtract(SIMDExtractOp::ExtractLaneSVecI8x16, 16)"),
    ("i8x16.extract_lane_u", "makeSIMDExtract(SIMDExtractOp::ExtractLaneUVecI8x16, 16)"),
    ("i8x16.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecI8x16, 16)"),
    ("i16x8.splat",          "makeUnary(UnaryOp::SplatVecI16x8)"),
    ("i16x8.extract_lane_s", "makeSIMDExtract(SIMDExtractOp::ExtractLaneSVecI16x8, 8)"),
    ("i16x8.extract_lane_u", "makeSIMDExtract(SIMDExtractOp::ExtractLaneUVecI16x8, 8)"),
    ("i16x8.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecI16x8, 8)"),
    ("i32x4.splat",          "makeUnary(UnaryOp::SplatVecI32x4)"),
    ("i32x4.extract_lane",   "makeSIMDExtract(SIMDExtractOp::ExtractLaneVecI32x4, 4)"),
    ("i32x4.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecI32x4, 4)"),
    ("i64x2.splat",          "makeUnary(UnaryOp::SplatVecI64x2)"),
    ("i64x2.extract_lane",   "makeSIMDExtract(SIMDExtractOp::ExtractLaneVecI64x2, 2)"),
    ("i64x2.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecI64x2, 2)"),
    ("f16x8.splat",          "makeUnary(UnaryOp::SplatVecF16x8)"),
    ("f16x8.extract_lane",   "makeSIMDExtract(SIMDExtractOp::ExtractLaneVecF16x8, 8)"),
    ("f16x8.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecF16x8, 8)"),
    ("f32x4.splat",          "makeUnary(UnaryOp::SplatVecF32x4)"),
    ("f32x4.extract_lane",   "makeSIMDExtract(SIMDExtractOp::ExtractLaneVecF32x4, 4)"),
    ("f32x4.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecF32x4, 4)"),
    ("f64x2.splat",          "makeUnary(UnaryOp::SplatVecF64x2)"),
    ("f64x2.extract_lane",   "makeSIMDExtract(SIMDExtractOp::ExtractLaneVecF64x2, 2)"),
    ("f64x2.replace_lane",   "makeSIMDReplace(SIMDReplaceOp::ReplaceLaneVecF64x2, 2)"),
    ("i8x16.eq",             "makeBinary(BinaryOp::EqVecI8x16)"),
    ("i8x16.ne",             "makeBinary(BinaryOp::NeVecI8x16)"),
    ("i8x16.lt_s",           "makeBinary(BinaryOp::LtSVecI8x16)"),
    ("i8x16.lt_u",           "makeBinary(BinaryOp::LtUVecI8x16)"),
    ("i8x16.gt_s",           "makeBinary(BinaryOp::GtSVecI8x16)"),
    ("i8x16.gt_u",           "makeBinary(BinaryOp::GtUVecI8x16)"),
    ("i8x16.le_s",           "makeBinary(BinaryOp::LeSVecI8x16)"),
    ("i8x16.le_u",           "makeBinary(BinaryOp::LeUVecI8x16)"),
    ("i8x16.ge_s",           "makeBinary(BinaryOp::GeSVecI8x16)"),
    ("i8x16.ge_u",           "makeBinary(BinaryOp::GeUVecI8x16)"),
    ("i16x8.eq",             "makeBinary(BinaryOp::EqVecI16x8)"),
    ("i16x8.ne",             "makeBinary(BinaryOp::NeVecI16x8)"),
    ("i16x8.lt_s",           "makeBinary(BinaryOp::LtSVecI16x8)"),
    ("i16x8.lt_u",           "makeBinary(BinaryOp::LtUVecI16x8)"),
    ("i16x8.gt_s",           "makeBinary(BinaryOp::GtSVecI16x8)"),
    ("i16x8.gt_u",           "makeBinary(BinaryOp::GtUVecI16x8)"),
    ("i16x8.le_s",           "makeBinary(BinaryOp::LeSVecI16x8)"),
    ("i16x8.le_u",           "makeBinary(BinaryOp::LeUVecI16x8)"),
    ("i16x8.ge_s",           "makeBinary(BinaryOp::GeSVecI16x8)"),
    ("i16x8.ge_u",           "makeBinary(BinaryOp::GeUVecI16x8)"),
    ("i32x4.eq",             "makeBinary(BinaryOp::EqVecI32x4)"),
    ("i32x4.ne",             "makeBinary(BinaryOp::NeVecI32x4)"),
    ("i32x4.lt_s",           "makeBinary(BinaryOp::LtSVecI32x4)"),
    ("i32x4.lt_u",           "makeBinary(BinaryOp::LtUVecI32x4)"),
    ("i32x4.gt_s",           "makeBinary(BinaryOp::GtSVecI32x4)"),
    ("i32x4.gt_u",           "makeBinary(BinaryOp::GtUVecI32x4)"),
    ("i32x4.le_s",           "makeBinary(BinaryOp::LeSVecI32x4)"),
    ("i32x4.le_u",           "makeBinary(BinaryOp::LeUVecI32x4)"),
    ("i32x4.ge_s",           "makeBinary(BinaryOp::GeSVecI32x4)"),
    ("i32x4.ge_u",           "makeBinary(BinaryOp::GeUVecI32x4)"),
    ("i64x2.eq",             "makeBinary(BinaryOp::EqVecI64x2)"),
    ("i64x2.ne",             "makeBinary(BinaryOp::NeVecI64x2)"),
    ("i64x2.lt_s",           "makeBinary(BinaryOp::LtSVecI64x2)"),
    ("i64x2.gt_s",           "makeBinary(BinaryOp::GtSVecI64x2)"),
    ("i64x2.le_s",           "makeBinary(BinaryOp::LeSVecI64x2)"),
    ("i64x2.ge_s",           "makeBinary(BinaryOp::GeSVecI64x2)"),
    ("f16x8.eq",             "makeBinary(BinaryOp::EqVecF16x8)"),
    ("f16x8.ne",             "makeBinary(BinaryOp::NeVecF16x8)"),
    ("f16x8.lt",             "makeBinary(BinaryOp::LtVecF16x8)"),
    ("f16x8.gt",             "makeBinary(BinaryOp::GtVecF16x8)"),
    ("f16x8.le",             "makeBinary(BinaryOp::LeVecF16x8)"),
    ("f16x8.ge",             "makeBinary(BinaryOp::GeVecF16x8)"),
    ("f32x4.eq",             "makeBinary(BinaryOp::EqVecF32x4)"),
    ("f32x4.ne",             "makeBinary(BinaryOp::NeVecF32x4)"),
    ("f32x4.lt",             "makeBinary(BinaryOp::LtVecF32x4)"),
    ("f32x4.gt",             "makeBinary(BinaryOp::GtVecF32x4)"),
    ("f32x4.le",             "makeBinary(BinaryOp::LeVecF32x4)"),
    ("f32x4.ge",             "makeBinary(BinaryOp::GeVecF32x4)"),
    ("f64x2.eq",             "makeBinary(BinaryOp::EqVecF64x2)"),
    ("f64x2.ne",             "makeBinary(BinaryOp::NeVecF64x2)"),
    ("f64x2.lt",             "makeBinary(BinaryOp::LtVecF64x2)"),
    ("f64x2.gt",             "makeBinary(BinaryOp::GtVecF64x2)"),
    ("f64x2.le",             "makeBinary(BinaryOp::LeVecF64x2)"),
    ("f64x2.ge",             "makeBinary(BinaryOp::GeVecF64x2)"),
    ("v128.not",             "makeUnary(UnaryOp::NotVec128)"),
    ("v128.and",             "makeBinary(BinaryOp::AndVec128)"),
    ("v128.or",              "makeBinary(BinaryOp::OrVec128)"),
    ("v128.xor",             "makeBinary(BinaryOp::XorVec128)"),
    ("v128.andnot",          "makeBinary(BinaryOp::AndNotVec128)"),
    ("v128.any_true",        "makeUnary(UnaryOp::AnyTrueVec128)"),
    ("v128.bitselect",       "makeSIMDTernary(SIMDTernaryOp::Bitselect)"),
    ("v128.load8_lane",      "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Load8LaneVec128, 1)"),
    ("v128.load16_lane",     "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Load16LaneVec128, 2)"),
    ("v128.load32_lane",     "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Load32LaneVec128, 4)"),
    ("v128.load64_lane",     "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Load64LaneVec128, 8)"),
    ("v128.store8_lane",     "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Store8LaneVec128, 1)"),
    ("v128.store16_lane",    "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Store16LaneVec128, 2)"),
    ("v128.store32_lane",    "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Store32LaneVec128, 4)"),
    ("v128.store64_lane",    "makeSIMDLoadStoreLane(SIMDLoadStoreLaneOp::Store64LaneVec128, 8)"),
    ("i8x16.popcnt",         "makeUnary(UnaryOp::PopcntVecI8x16)"),
    ("i8x16.abs",            "makeUnary(UnaryOp::AbsVecI8x16)"),
    ("i8x16.neg",            "makeUnary(UnaryOp::NegVecI8x16)"),
    ("i8x16.all_true",       "makeUnary(UnaryOp::AllTrueVecI8x16)"),
    ("i8x16.bitmask",        "makeUnary(UnaryOp::BitmaskVecI8x16)"),
    ("i8x16.shl",            "makeSIMDShift(SIMDShiftOp::ShlVecI8x16)"),
    ("i8x16.shr_s",          "makeSIMDShift(SIMDShiftOp::ShrSVecI8x16)"),
    ("i8x16.shr_u",          "makeSIMDShift(SIMDShiftOp::ShrUVecI8x16)"),
    ("i8x16.add",            "makeBinary(BinaryOp::AddVecI8x16)"),
    ("i8x16.add_sat_s",      "makeBinary(BinaryOp::AddSatSVecI8x16)"),
    ("i8x16.add_sat_u",      "makeBinary(BinaryOp::AddSatUVecI8x16)"),
    ("i8x16.sub",            "makeBinary(BinaryOp::SubVecI8x16)"),
    ("i8x16.sub_sat_s",      "makeBinary(BinaryOp::SubSatSVecI8x16)"),
    ("i8x16.sub_sat_u",      "makeBinary(BinaryOp::SubSatUVecI8x16)"),
    ("i8x16.min_s",          "makeBinary(BinaryOp::MinSVecI8x16)"),
    ("i8x16.min_u",          "makeBinary(BinaryOp::MinUVecI8x16)"),
    ("i8x16.max_s",          "makeBinary(BinaryOp::MaxSVecI8x16)"),
    ("i8x16.max_u",          "makeBinary(BinaryOp::MaxUVecI8x16)"),
    ("i8x16.avgr_u",         "makeBinary(BinaryOp::AvgrUVecI8x16)"),
    ("i16x8.abs",            "makeUnary(UnaryOp::AbsVecI16x8)"),
    ("i16x8.neg",            "makeUnary(UnaryOp::NegVecI16x8)"),
    ("i16x8.all_true",       "makeUnary(UnaryOp::AllTrueVecI16x8)"),
    ("i16x8.bitmask",        "makeUnary(UnaryOp::BitmaskVecI16x8)"),
    ("i16x8.shl",            "makeSIMDShift(SIMDShiftOp::ShlVecI16x8)"),
    ("i16x8.shr_s",          "makeSIMDShift(SIMDShiftOp::ShrSVecI16x8)"),
    ("i16x8.shr_u",          "makeSIMDShift(SIMDShiftOp::ShrUVecI16x8)"),
    ("i16x8.add",            "makeBinary(BinaryOp::AddVecI16x8)"),
    ("i16x8.add_sat_s",      "makeBinary(BinaryOp::AddSatSVecI16x8)"),
    ("i16x8.add_sat_u",      "makeBinary(BinaryOp::AddSatUVecI16x8)"),
    ("i16x8.sub",            "makeBinary(BinaryOp::SubVecI16x8)"),
    ("i16x8.sub_sat_s",      "makeBinary(BinaryOp::SubSatSVecI16x8)"),
    ("i16x8.sub_sat_u",      "makeBinary(BinaryOp::SubSatUVecI16x8)"),
    ("i16x8.mul",            "makeBinary(BinaryOp::MulVecI16x8)"),
    ("i16x8.min_s",          "makeBinary(BinaryOp::MinSVecI16x8)"),
    ("i16x8.min_u",          "makeBinary(BinaryOp::MinUVecI16x8)"),
    ("i16x8.max_s",          "makeBinary(BinaryOp::MaxSVecI16x8)"),
    ("i16x8.max_u",          "makeBinary(BinaryOp::MaxUVecI16x8)"),
    ("i16x8.avgr_u",         "makeBinary(BinaryOp::AvgrUVecI16x8)"),
    ("i16x8.q15mulr_sat_s",  "makeBinary(BinaryOp::Q15MulrSatSVecI16x8)"),
    ("i16x8.extmul_low_i8x16_s", "makeBinary(BinaryOp::ExtMulLowSVecI16x8)"),
    ("i16x8.extmul_high_i8x16_s", "makeBinary(BinaryOp::ExtMulHighSVecI16x8)"),
    ("i16x8.extmul_low_i8x16_u", "makeBinary(BinaryOp::ExtMulLowUVecI16x8)"),
    ("i16x8.extmul_high_i8x16_u", "makeBinary(BinaryOp::ExtMulHighUVecI16x8)"),
    ("i32x4.abs",            "makeUnary(UnaryOp::AbsVecI32x4)"),
    ("i32x4.neg",            "makeUnary(UnaryOp::NegVecI32x4)"),
    ("i32x4.all_true",       "makeUnary(UnaryOp::AllTrueVecI32x4)"),
    ("i32x4.bitmask",        "makeUnary(UnaryOp::BitmaskVecI32x4)"),
    ("i32x4.shl",            "makeSIMDShift(SIMDShiftOp::ShlVecI32x4)"),
    ("i32x4.shr_s",          "makeSIMDShift(SIMDShiftOp::ShrSVecI32x4)"),
    ("i32x4.shr_u",          "makeSIMDShift(SIMDShiftOp::ShrUVecI32x4)"),
    ("i32x4.add",            "makeBinary(BinaryOp::AddVecI32x4)"),
    ("i32x4.sub",            "makeBinary(BinaryOp::SubVecI32x4)"),
    ("i32x4.mul",            "makeBinary(BinaryOp::MulVecI32x4)"),
    ("i32x4.min_s",          "makeBinary(BinaryOp::MinSVecI32x4)"),
    ("i32x4.min_u",          "makeBinary(BinaryOp::MinUVecI32x4)"),
    ("i32x4.max_s",          "makeBinary(BinaryOp::MaxSVecI32x4)"),
    ("i32x4.max_u",          "makeBinary(BinaryOp::MaxUVecI32x4)"),
    ("i32x4.dot_i16x8_s",    "makeBinary(BinaryOp::DotSVecI16x8ToVecI32x4)"),
    ("i32x4.extmul_low_i16x8_s", "makeBinary(BinaryOp::ExtMulLowSVecI32x4)"),
    ("i32x4.extmul_high_i16x8_s", "makeBinary(BinaryOp::ExtMulHighSVecI32x4)"),
    ("i32x4.extmul_low_i16x8_u", "makeBinary(BinaryOp::ExtMulLowUVecI32x4)"),
    ("i32x4.extmul_high_i16x8_u", "makeBinary(BinaryOp::ExtMulHighUVecI32x4)"),
    ("i64x2.abs",            "makeUnary(UnaryOp::AbsVecI64x2)"),
    ("i64x2.neg",            "makeUnary(UnaryOp::NegVecI64x2)"),
    ("i64x2.all_true",       "makeUnary(UnaryOp::AllTrueVecI64x2)"),
    ("i64x2.bitmask",        "makeUnary(UnaryOp::BitmaskVecI64x2)"),
    ("i64x2.shl",            "makeSIMDShift(SIMDShiftOp::ShlVecI64x2)"),
    ("i64x2.shr_s",          "makeSIMDShift(SIMDShiftOp::ShrSVecI64x2)"),
    ("i64x2.shr_u",          "makeSIMDShift(SIMDShiftOp::ShrUVecI64x2)"),
    ("i64x2.add",            "makeBinary(BinaryOp::AddVecI64x2)"),
    ("i64x2.sub",            "makeBinary(BinaryOp::SubVecI64x2)"),
    ("i64x2.mul",            "makeBinary(BinaryOp::MulVecI64x2)"),
    ("i64x2.extmul_low_i32x4_s", "makeBinary(BinaryOp::ExtMulLowSVecI64x2)"),
    ("i64x2.extmul_high_i32x4_s", "makeBinary(BinaryOp::ExtMulHighSVecI64x2)"),
    ("i64x2.extmul_low_i32x4_u", "makeBinary(BinaryOp::ExtMulLowUVecI64x2)"),
    ("i64x2.extmul_high_i32x4_u", "makeBinary(BinaryOp::ExtMulHighUVecI64x2)"),
    ("f16x8.abs",            "makeUnary(UnaryOp::AbsVecF16x8)"),
    ("f16x8.neg",            "makeUnary(UnaryOp::NegVecF16x8)"),
    ("f16x8.sqrt",           "makeUnary(UnaryOp::SqrtVecF16x8)"),
    ("f16x8.add",            "makeBinary(BinaryOp::AddVecF16x8)"),
    ("f16x8.sub",            "makeBinary(BinaryOp::SubVecF16x8)"),
    ("f16x8.mul",            "makeBinary(BinaryOp::MulVecF16x8)"),
    ("f16x8.div",            "makeBinary(BinaryOp::DivVecF16x8)"),
    ("f16x8.min",            "makeBinary(BinaryOp::MinVecF16x8)"),
    ("f16x8.max",            "makeBinary(BinaryOp::MaxVecF16x8)"),
    ("f16x8.pmin",           "makeBinary(BinaryOp::PMinVecF16x8)"),
    ("f16x8.pmax",           "makeBinary(BinaryOp::PMaxVecF16x8)"),
    ("f16x8.ceil",           "makeUnary(UnaryOp::CeilVecF16x8)"),
    ("f16x8.floor",          "makeUnary(UnaryOp::FloorVecF16x8)"),
    ("f16x8.trunc",          "makeUnary(UnaryOp::TruncVecF16x8)"),
    ("f16x8.nearest",        "makeUnary(UnaryOp::NearestVecF16x8)"),
    ("f32x4.abs",            "makeUnary(UnaryOp::AbsVecF32x4)"),
    ("f32x4.neg",            "makeUnary(UnaryOp::NegVecF32x4)"),
    ("f32x4.sqrt",           "makeUnary(UnaryOp::SqrtVecF32x4)"),
    ("f32x4.add",            "makeBinary(BinaryOp::AddVecF32x4)"),
    ("f32x4.sub",            "makeBinary(BinaryOp::SubVecF32x4)"),
    ("f32x4.mul",            "makeBinary(BinaryOp::MulVecF32x4)"),
    ("f32x4.div",            "makeBinary(BinaryOp::DivVecF32x4)"),
    ("f32x4.min",            "makeBinary(BinaryOp::MinVecF32x4)"),
    ("f32x4.max",            "makeBinary(BinaryOp::MaxVecF32x4)"),
    ("f32x4.pmin",           "makeBinary(BinaryOp::PMinVecF32x4)"),
    ("f32x4.pmax",           "makeBinary(BinaryOp::PMaxVecF32x4)"),
    ("f32x4.ceil",           "makeUnary(UnaryOp::CeilVecF32x4)"),
    ("f32x4.floor",          "makeUnary(UnaryOp::FloorVecF32x4)"),
    ("f32x4.trunc",          "makeUnary(UnaryOp::TruncVecF32x4)"),
    ("f32x4.nearest",        "makeUnary(UnaryOp::NearestVecF32x4)"),
    ("f64x2.abs",            "makeUnary(UnaryOp::AbsVecF64x2)"),
    ("f64x2.neg",            "makeUnary(UnaryOp::NegVecF64x2)"),
    ("f64x2.sqrt",           "makeUnary(UnaryOp::SqrtVecF64x2)"),
    ("f64x2.add",            "makeBinary(BinaryOp::AddVecF64x2)"),
    ("f64x2.sub",            "makeBinary(BinaryOp::SubVecF64x2)"),
    ("f64x2.mul",            "makeBinary(BinaryOp::MulVecF64x2)"),
    ("f64x2.div",            "makeBinary(BinaryOp::DivVecF64x2)"),
    ("f64x2.min",            "makeBinary(BinaryOp::MinVecF64x2)"),
    ("f64x2.max",            "makeBinary(BinaryOp::MaxVecF64x2)"),
    ("f64x2.pmin",           "makeBinary(BinaryOp::PMinVecF64x2)"),
    ("f64x2.pmax",           "makeBinary(BinaryOp::PMaxVecF64x2)"),
    ("f64x2.ceil",           "makeUnary(UnaryOp::CeilVecF64x2)"),
    ("f64x2.floor",          "makeUnary(UnaryOp::FloorVecF64x2)"),
    ("f64x2.trunc",          "makeUnary(UnaryOp::TruncVecF64x2)"),
    ("f64x2.nearest",        "makeUnary(UnaryOp::NearestVecF64x2)"),
    ("i32x4.trunc_sat_f32x4_s",  "makeUnary(UnaryOp::TruncSatSVecF32x4ToVecI32x4)"),
    ("i32x4.trunc_sat_f32x4_u",  "makeUnary(UnaryOp::TruncSatUVecF32x4ToVecI32x4)"),
    ("f32x4.convert_i32x4_s",    "makeUnary(UnaryOp::ConvertSVecI32x4ToVecF32x4)"),
    ("f32x4.convert_i32x4_u",    "makeUnary(UnaryOp::ConvertUVecI32x4ToVecF32x4)"),
    ("v128.load8_splat",         "makeSIMDLoad(SIMDLoadOp::Load8SplatVec128, 1)"),
    ("v128.load16_splat",        "makeSIMDLoad(SIMDLoadOp::Load16SplatVec128, 2)"),
    ("v128.load32_splat",        "makeSIMDLoad(SIMDLoadOp::Load32SplatVec128, 4)"),
    ("v128.load64_splat",        "makeSIMDLoad(SIMDLoadOp::Load64SplatVec128, 8)"),
    ("v128.load8x8_s",           "makeSIMDLoad(SIMDLoadOp::Load8x8SVec128, 8)"),
    ("v128.load8x8_u",           "makeSIMDLoad(SIMDLoadOp::Load8x8UVec128, 8)"),
    ("v128.load16x4_s",          "makeSIMDLoad(SIMDLoadOp::Load16x4SVec128, 8)"),
    ("v128.load16x4_u",          "makeSIMDLoad(SIMDLoadOp::Load16x4UVec128, 8)"),
    ("v128.load32x2_s",          "makeSIMDLoad(SIMDLoadOp::Load32x2SVec128, 8)"),
    ("v128.load32x2_u",          "makeSIMDLoad(SIMDLoadOp::Load32x2UVec128, 8)"),
    ("v128.load32_zero",         "makeSIMDLoad(SIMDLoadOp::Load32ZeroVec128, 4)"),
    ("v128.load64_zero",         "makeSIMDLoad(SIMDLoadOp::Load64ZeroVec128, 8)"),
    ("i8x16.narrow_i16x8_s",     "makeBinary(BinaryOp::NarrowSVecI16x8ToVecI8x16)"),
    ("i8x16.narrow_i16x8_u",     "makeBinary(BinaryOp::NarrowUVecI16x8ToVecI8x16)"),
    ("i16x8.narrow_i32x4_s",     "makeBinary(BinaryOp::NarrowSVecI32x4ToVecI16x8)"),
    ("i16x8.narrow_i32x4_u",     "makeBinary(BinaryOp::NarrowUVecI32x4ToVecI16x8)"),
    ("i16x8.extend_low_i8x16_s",  "makeUnary(UnaryOp::ExtendLowSVecI8x16ToVecI16x8)"),
    ("i16x8.extend_high_i8x16_s", "makeUnary(UnaryOp::ExtendHighSVecI8x16ToVecI16x8)"),
    ("i16x8.extend_low_i8x16_u",  "makeUnary(UnaryOp::ExtendLowUVecI8x16ToVecI16x8)"),
    ("i16x8.extend_high_i8x16_u", "makeUnary(UnaryOp::ExtendHighUVecI8x16ToVecI16x8)"),
    ("i32x4.extend_low_i16x8_s",  "makeUnary(UnaryOp::ExtendLowSVecI16x8ToVecI32x4)"),
    ("i32x4.extend_high_i16x8_s", "makeUnary(UnaryOp::ExtendHighSVecI16x8ToVecI32x4)"),
    ("i32x4.extend_low_i16x8_u",  "makeUnary(UnaryOp::ExtendLowUVecI16x8ToVecI32x4)"),
    ("i32x4.extend_high_i16x8_u", "makeUnary(UnaryOp::ExtendHighUVecI16x8ToVecI32x4)"),
    ("i64x2.extend_low_i32x4_s",  "makeUnary(UnaryOp::ExtendLowSVecI32x4ToVecI64x2)"),
    ("i64x2.extend_high_i32x4_s", "makeUnary(UnaryOp::ExtendHighSVecI32x4ToVecI64x2)"),
    ("i64x2.extend_low_i32x4_u",  "makeUnary(UnaryOp::ExtendLowUVecI32x4ToVecI64x2)"),
    ("i64x2.extend_high_i32x4_u", "makeUnary(UnaryOp::ExtendHighUVecI32x4ToVecI64x2)"),
    ("i8x16.swizzle",             "makeBinary(BinaryOp::SwizzleVecI8x16)"),
    ("i16x8.extadd_pairwise_i8x16_s", "makeUnary(UnaryOp::ExtAddPairwiseSVecI8x16ToI16x8)"),
    ("i16x8.extadd_pairwise_i8x16_u", "makeUnary(UnaryOp::ExtAddPairwiseUVecI8x16ToI16x8)"),
    ("i32x4.extadd_pairwise_i16x8_s", "makeUnary(UnaryOp::ExtAddPairwiseSVecI16x8ToI32x4)"),
    ("i32x4.extadd_pairwise_i16x8_u", "makeUnary(UnaryOp::ExtAddPairwiseUVecI16x8ToI32x4)"),
    ("f64x2.convert_low_i32x4_s",     "makeUnary(UnaryOp::ConvertLowSVecI32x4ToVecF64x2)"),
    ("f64x2.convert_low_i32x4_u",     "makeUnary(UnaryOp::ConvertLowUVecI32x4ToVecF64x2)"),
    ("i32x4.trunc_sat_f64x2_s_zero",  "makeUnary(UnaryOp::TruncSatZeroSVecF64x2ToVecI32x4)"),
    ("i32x4.trunc_sat_f64x2_u_zero",  "makeUnary(UnaryOp::TruncSatZeroUVecF64x2ToVecI32x4)"),
    ("f32x4.demote_f64x2_zero",       "makeUnary(UnaryOp::DemoteZeroVecF64x2ToVecF32x4)"),
    ("f64x2.promote_low_f32x4",       "makeUnary(UnaryOp::PromoteLowVecF32x4ToVecF64x2)"),
    ("i16x8.trunc_sat_f16x8_s",  "makeUnary(UnaryOp::TruncSatSVecF16x8ToVecI16x8)"),
    ("i16x8.trunc_sat_f16x8_u",  "makeUnary(UnaryOp::TruncSatUVecF16x8ToVecI16x8)"),
    ("f16x8.convert_i16x8_s",    "makeUnary(UnaryOp::ConvertSVecI16x8ToVecF16x8)"),
    ("f16x8.convert_i16x8_u",    "makeUnary(UnaryOp::ConvertUVecI16x8ToVecF16x8)"),

    # relaxed SIMD ops
    ("i8x16.relaxed_swizzle", "makeBinary(BinaryOp::RelaxedSwizzleVecI8x16)"),
    ("i32x4.relaxed_trunc_f32x4_s", "makeUnary(UnaryOp::RelaxedTruncSVecF32x4ToVecI32x4)"),
    ("i32x4.relaxed_trunc_f32x4_u", "makeUnary(UnaryOp::RelaxedTruncUVecF32x4ToVecI32x4)"),
    ("i32x4.relaxed_trunc_f64x2_s_zero", "makeUnary(UnaryOp::RelaxedTruncZeroSVecF64x2ToVecI32x4)"),
    ("i32x4.relaxed_trunc_f64x2_u_zero", "makeUnary(UnaryOp::RelaxedTruncZeroUVecF64x2ToVecI32x4)"),
    ("f16x8.relaxed_madd", "makeSIMDTernary(SIMDTernaryOp::RelaxedMaddVecF16x8)"),
    ("f16x8.relaxed_nmadd", "makeSIMDTernary(SIMDTernaryOp::RelaxedNmaddVecF16x8)"),
    ("f32x4.relaxed_madd", "makeSIMDTernary(SIMDTernaryOp::RelaxedMaddVecF32x4)"),
    ("f32x4.relaxed_nmadd", "makeSIMDTernary(SIMDTernaryOp::RelaxedNmaddVecF32x4)"),
    ("f64x2.relaxed_madd", "makeSIMDTernary(SIMDTernaryOp::RelaxedMaddVecF64x2)"),
    ("f64x2.relaxed_nmadd", "makeSIMDTernary(SIMDTernaryOp::RelaxedNmaddVecF64x2)"),
    ("i8x16.laneselect", "makeSIMDTernary(SIMDTernaryOp::LaneselectI8x16)"),
    ("i16x8.laneselect", "makeSIMDTernary(SIMDTernaryOp::LaneselectI16x8)"),
    ("i32x4.laneselect", "makeSIMDTernary(SIMDTernaryOp::LaneselectI32x4)"),
    ("i64x2.laneselect", "makeSIMDTernary(SIMDTernaryOp::LaneselectI64x2)"),
    ("f32x4.relaxed_min", "makeBinary(BinaryOp::RelaxedMinVecF32x4)"),
    ("f32x4.relaxed_max", "makeBinary(BinaryOp::RelaxedMaxVecF32x4)"),
    ("f64x2.relaxed_min", "makeBinary(BinaryOp::RelaxedMinVecF64x2)"),
    ("f64x2.relaxed_max", "makeBinary(BinaryOp::RelaxedMaxVecF64x2)"),
    ("i16x8.relaxed_q15mulr_s", "makeBinary(BinaryOp::RelaxedQ15MulrSVecI16x8)"),
    ("i16x8.dot_i8x16_i7x16_s", "makeBinary(BinaryOp::DotI8x16I7x16SToVecI16x8)"),
    ("i32x4.dot_i8x16_i7x16_add_s", "makeSIMDTernary(SIMDTernaryOp::DotI8x16I7x16AddSToVecI32x4)"),

    # reference types instructions
    ("ref.null",             "makeRefNull()"),
    ("ref.is_null",          "makeRefIsNull()"),
    ("ref.func",             "makeRefFunc()"),
    ("ref.eq",               "makeRefEq()"),
    # table instructions
    ("table.get",            "makeTableGet()"),
    ("table.set",            "makeTableSet()"),
    ("table.size",           "makeTableSize()"),
    ("table.grow",           "makeTableGrow()"),
    ("table.fill",           "makeTableFill()"),
    ("table.copy",           "makeTableCopy()"),
    ("table.init",           "makeTableInit()"),
    # exception handling instructions
    ("try",                  "makeTry()"),
    ("try_table",            "makeTryTable()"),
    ("throw",                "makeThrow()"),
    ("rethrow",              "makeRethrow()"),
    ("throw_ref",            "makeThrowRef()"),
    # Multivalue pseudoinstructions
    ("tuple.make",           "makeTupleMake()"),
    ("tuple.extract",        "makeTupleExtract()"),
    ("tuple.drop",           "makeTupleDrop()"),
    ("pop",                  "makePop()"),
    # Typed function references instructions
    ("call_ref",             "makeCallRef(/*isReturn=*/false)"),
    ("return_call_ref",      "makeCallRef(/*isReturn=*/true)"),
    # Stack switching instructions
    ("cont.new",             "makeContNew()"),
    ("cont.bind",            "makeContBind()"),
    ("suspend",              "makeSuspend()"),
    ("resume",               "makeResume()"),
    ("resume_throw",         "makeResumeThrow()"),
    ("switch",               "makeStackSwitch()"),
    # GC
    ("ref.i31",              "makeRefI31(Unshared)"),
    ("ref.i31_shared",       "makeRefI31(Shared)"),
    ("i31.get_s",            "makeI31Get(true)"),
    ("i31.get_u",            "makeI31Get(false)"),
    ("ref.test",             "makeRefTest()"),
    ("ref.cast",             "makeRefCast()"),
    ("br_on_null",           "makeBrOnNull()"),
    ("br_on_non_null",       "makeBrOnNull(true)"),
    ("br_on_cast",           "makeBrOnCast()"),
    ("br_on_cast_fail",      "makeBrOnCast(true)"),
    ("struct.new",           "makeStructNew(false)"),
    ("struct.new_default",   "makeStructNew(true)"),
    ("struct.get",           "makeStructGet()"),
    ("struct.get_s",         "makeStructGet(true)"),
    ("struct.get_u",         "makeStructGet(false)"),
    ("struct.set",           "makeStructSet()"),
    ("array.new",            "makeArrayNew(false)"),
    ("array.new_default",    "makeArrayNew(true)"),
    ("array.new_data",       "makeArrayNewData()"),
    ("array.new_elem",       "makeArrayNewElem()"),
    ("array.new_fixed",      "makeArrayNewFixed()"),
    ("array.get",            "makeArrayGet()"),
    ("array.get_s",          "makeArrayGet(true)"),
    ("array.get_u",          "makeArrayGet(false)"),
    ("array.set",            "makeArraySet()"),
    ("array.len",            "makeArrayLen()"),
    ("array.copy",           "makeArrayCopy()"),
    ("array.fill",           "makeArrayFill()"),
    ("array.init_data",      "makeArrayInitData()"),
    ("array.init_elem",      "makeArrayInitElem()"),
    ("ref.as_non_null",      "makeRefAs(RefAsNonNull)"),
    ("extern.internalize",   "makeRefAs(AnyConvertExtern)"),  # Deprecated
    ("extern.externalize",   "makeRefAs(ExternConvertAny)"),  # Deprecated
    ("any.convert_extern",   "makeRefAs(AnyConvertExtern)"),
    ("extern.convert_any",   "makeRefAs(ExternConvertAny)"),
    ("string.new_lossy_utf8_array",  "makeStringNew(StringNewLossyUTF8Array)"),
    ("string.new_wtf16_array", "makeStringNew(StringNewWTF16Array)"),
    ("string.from_code_point", "makeStringNew(StringNewFromCodePoint)"),
    ("string.const",         "makeStringConst()"),
    ("string.measure_utf8",  "makeStringMeasure(StringMeasureUTF8)"),
    ("string.measure_wtf16", "makeStringMeasure(StringMeasureWTF16)"),
    ("stringview_wtf16.length", "makeStringMeasure(StringMeasureWTF16)"),
    ("string.encode_lossy_utf8_array",   "makeStringEncode(StringEncodeLossyUTF8Array)"),
    ("string.encode_wtf16_array",  "makeStringEncode(StringEncodeWTF16Array)"),
    ("string.concat",        "makeStringConcat()"),
    ("string.eq",            "makeStringEq(StringEqEqual)"),
    ("string.compare",       "makeStringEq(StringEqCompare)"),
    ("stringview_wtf16.get_codeunit", "makeStringWTF16Get()"),
    ("stringview_wtf16.slice",        "makeStringSliceWTF()"),
    # Ignored in input
    ("string.as_wtf16",               "ignore()"),
]


class CodePrinter:
    indents = 0

    def __enter__(self):
        CodePrinter.indents += 1

    def __exit__(self, *args):
        CodePrinter.indents -= 1

    def indent(self):
        # call in a 'with' statement
        return self

    def print_line(self, line):
        print("  " * CodePrinter.indents + line)


class Node:
    def __init__(self, expr=None, children=None, inst=None):
        # the expression to return if this is the string has ended
        self.expr = expr
        # map unique strings to children nodes
        self.children = children if children else {}
        # full instruction leading to this node
        self.inst = inst

    def _common_prefix(a, b):
        """Return the common prefix of two strings."""
        prefix = []
        while a and b and a[0] == b[0]:
            prefix.append(a[0])
            a = a[1:]
            b = b[1:]
        return "".join(prefix)

    def do_insert(self, full_inst, inst, expr):
        if not inst:
            assert self.expr is None, "Repeated instruction " + full_inst
            self.expr = expr
            self.inst = full_inst
            return
        # find key with shared prefix
        prefix, key = "", None
        for k in self.children:
            prefix = Node._common_prefix(inst, k)
            if prefix:
                key = k
                break
        if key is None:
            # unique prefix, insert and stop
            self.children[inst] = Node(expr, inst=full_inst)
            return
        key_remainder = key[len(prefix):]
        if key_remainder:
            # split key and move everything after the prefix to a new node
            child = self.children.pop(key)
            self.children[prefix] = Node(children={key_remainder: child})
            # update key for recursive insert
            key = prefix
        # chop off prefix and recurse
        self.children[key].do_insert(full_inst, inst[len(key):], expr)

    def insert(self, inst, expr):
        self.do_insert(inst, inst, expr)


def instruction_parser():
    """Build a trie out of all the instructions, then emit it as C++ code."""
    global instructions
    trie = Node()
    inst_length = 0
    for inst, expr in instructions:
        if inst in {"block", "loop", "if", "try", "then",
                    "else", "try_table"}:
            # These are either control flow handled manually or not real
            # instructions. Skip them.
            continue
        inst_length = max(inst_length, len(inst))
        trie.insert(inst, expr)

    printer = CodePrinter()

    printer.print_line("auto op = *keyword;")
    printer.print_line("char buf[{}] = {{}};".format(inst_length + 1))
    printer.print_line("// Ensure we do not copy more than the buffer can hold")
    printer.print_line("if (op.size() >= sizeof(buf)) {")
    printer.print_line("  goto parse_error;")
    printer.print_line("}")
    printer.print_line("memcpy(buf, op.data(), op.size());")

    def print_leaf(expr, inst):
        if "()" in expr:
            expr = expr.replace("()", "(ctx, pos, annotations)")
        else:
            expr = expr.replace("(", "(ctx, pos, annotations, ")
        printer.print_line("if (op == \"{inst}\"sv) {{".format(inst=inst))
        with printer.indent():
            printer.print_line("CHECK_ERR({expr});".format(expr=expr))
            printer.print_line("return Ok{};")
        printer.print_line("}")
        printer.print_line("goto parse_error;")

    def emit(node, idx=0):
        assert node.children
        printer.print_line("switch (buf[{}]) {{".format(idx))
        with printer.indent():
            if node.expr:
                printer.print_line("case '\\0':")
                with printer.indent():
                    print_leaf(node.expr, node.inst)
            children = sorted(node.children.items(), key=lambda pair: pair[0])
            for prefix, child in children:
                if child.children:
                    printer.print_line("case '{}': {{".format(prefix[0]))
                    with printer.indent():
                        emit(child, idx + len(prefix))
                    printer.print_line("}")
                else:
                    assert child.expr
                    printer.print_line("case '{}':".format(prefix[0]))
                    with printer.indent():
                        print_leaf(child.expr, child.inst)
            printer.print_line("default: goto parse_error;")
        printer.print_line("}")

    emit(trie)
    printer.print_line("parse_error:")
    with printer.indent():
        printer.print_line("return ctx.in.err(pos, \"unrecognized instruction\");")


def print_header():
    print("// DO NOT EDIT! This file generated by scripts/gen-s-parser.py\n")
    print("// clang-format off\n")
    print("// NOLINTBEGIN\n")


def print_footer():
    print("\n// NOLINTEND")
    print("\n// clang-format on")


def main():
    if sys.version_info.major != 3:
        import datetime
        print("It's " + str(datetime.datetime.now().year) + "! Use Python 3!")
        sys.exit(1)
    print_header()
    instruction_parser()
    print_footer()


if __name__ == "__main__":
    main()
