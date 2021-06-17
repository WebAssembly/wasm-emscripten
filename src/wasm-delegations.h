/*
 * Copyright 2020 WebAssembly Community Group participants
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

DELEGATE(Nop);
DELEGATE(Block);
DELEGATE(If);
DELEGATE(Loop);
DELEGATE(Break);
DELEGATE(Switch);
DELEGATE(Call);
DELEGATE(CallIndirect);
DELEGATE(LocalGet);
DELEGATE(LocalSet);
DELEGATE(GlobalGet);
DELEGATE(GlobalSet);
DELEGATE(Load);
DELEGATE(Store);
DELEGATE(AtomicRMW);
DELEGATE(AtomicCmpxchg);
DELEGATE(AtomicWait);
DELEGATE(AtomicNotify);
DELEGATE(AtomicFence);
DELEGATE(SIMDExtract);
DELEGATE(SIMDReplace);
DELEGATE(SIMDShuffle);
DELEGATE(SIMDTernary);
DELEGATE(SIMDShift);
DELEGATE(SIMDLoad);
DELEGATE(SIMDLoadStoreLane);
DELEGATE(MemoryInit);
DELEGATE(DataDrop);
DELEGATE(MemoryCopy);
DELEGATE(MemoryFill);
DELEGATE(Const);
DELEGATE(Unary);
DELEGATE(Binary);
DELEGATE(Select);
DELEGATE(Drop);
DELEGATE(Return);
DELEGATE(MemorySize);
DELEGATE(MemoryGrow);
DELEGATE(TableGet);
DELEGATE(TableSet);
DELEGATE(TableSize);
DELEGATE(TableGrow);
DELEGATE(TableFill);
DELEGATE(TableCopy);
DELEGATE(TableInit);
DELEGATE(ElemDrop);
DELEGATE(Unreachable);
DELEGATE(Pop);
DELEGATE(RefNull);
DELEGATE(RefIs);
DELEGATE(RefFunc);
DELEGATE(RefEq);
DELEGATE(Try);
DELEGATE(Throw);
DELEGATE(Rethrow);
DELEGATE(TupleMake);
DELEGATE(TupleExtract);
DELEGATE(I31New);
DELEGATE(I31Get);
DELEGATE(CallRef);
DELEGATE(RefTest);
DELEGATE(RefCast);
DELEGATE(BrOn);
DELEGATE(RttCanon);
DELEGATE(RttSub);
DELEGATE(StructNew);
DELEGATE(StructGet);
DELEGATE(StructSet);
DELEGATE(ArrayNew);
DELEGATE(ArrayGet);
DELEGATE(ArraySet);
DELEGATE(ArrayLen);
DELEGATE(ArrayCopy);
DELEGATE(RefAs);

#undef DELEGATE
