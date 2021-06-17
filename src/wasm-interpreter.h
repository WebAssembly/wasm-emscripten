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
// Simple WebAssembly interpreter. This operates directly on the AST,
// for simplicity and clarity. A goal is for it to be possible for
// people to read this code and understand WebAssembly semantics.
//

#ifndef wasm_wasm_interpreter_h
#define wasm_wasm_interpreter_h

#include <cmath>
#include <limits.h>
#include <sstream>

#include "ir/module-utils.h"
#include "support/bits.h"
#include "support/safe_integer.h"
#include "wasm-builder.h"
#include "wasm-traversal.h"
#include "wasm.h"

#ifdef WASM_INTERPRETER_DEBUG
#include "wasm-printing.h"
#endif

namespace wasm {

struct WasmException {
  Name event;
  Literals values;
};
std::ostream& operator<<(std::ostream& o, const WasmException& exn);

using namespace cashew;

// Utilities

extern Name WASM, RETURN_FLOW, NONCONSTANT_FLOW;

// Stuff that flows around during executing expressions: a literal, or a change
// in control flow.
class Flow {
public:
  Flow() : values() {}
  Flow(Literal value) : values{value} { assert(value.type.isConcrete()); }
  Flow(Literals& values) : values(values) {}
  Flow(Literals&& values) : values(std::move(values)) {}
  Flow(Name breakTo) : values(), breakTo(breakTo) {}
  Flow(Name breakTo, Literal value) : values{value}, breakTo(breakTo) {}

  Literals values;
  Name breakTo; // if non-null, a break is going on

  // A helper function for the common case where there is only one value
  const Literal& getSingleValue() {
    assert(values.size() == 1);
    return values[0];
  }

  Type getType() { return values.getType(); }

  Expression* getConstExpression(Module& module) {
    assert(values.size() > 0);
    Builder builder(module);
    return builder.makeConstantExpression(values);
  }

  bool breaking() { return breakTo.is(); }

  void clearIf(Name target) {
    if (breakTo == target) {
      breakTo.clear();
    }
  }

  friend std::ostream& operator<<(std::ostream& o, const Flow& flow) {
    o << "(flow " << (flow.breakTo.is() ? flow.breakTo.str : "-") << " : {";
    for (size_t i = 0; i < flow.values.size(); ++i) {
      if (i > 0) {
        o << ", ";
      }
      o << flow.values[i];
    }
    o << "})";
    return o;
  }
};

// A list of literals, for function calls
typedef std::vector<Literal> LiteralList;

// Debugging helpers
#ifdef WASM_INTERPRETER_DEBUG
class Indenter {
  static int indentLevel;

  const char* entryName;

public:
  Indenter(const char* entry);
  ~Indenter();

  static void print();
};

#define NOTE_ENTER(x)                                                          \
  Indenter _int_blah(x);                                                       \
  {                                                                            \
    Indenter::print();                                                         \
    std::cout << "visit " << x << " : " << curr << "\n";                       \
  }
#define NOTE_ENTER_(x)                                                         \
  Indenter _int_blah(x);                                                       \
  {                                                                            \
    Indenter::print();                                                         \
    std::cout << "visit " << x << "\n";                                        \
  }
#define NOTE_NAME(p0)                                                          \
  {                                                                            \
    Indenter::print();                                                         \
    std::cout << "name " << '(' << Name(p0) << ")\n";                          \
  }
#define NOTE_EVAL1(p0)                                                         \
  {                                                                            \
    Indenter::print();                                                         \
    std::cout << "eval " #p0 " (" << p0 << ")\n";                              \
  }
#define NOTE_EVAL2(p0, p1)                                                     \
  {                                                                            \
    Indenter::print();                                                         \
    std::cout << "eval " #p0 " (" << p0 << "), " #p1 " (" << p1 << ")\n";      \
  }
#else // WASM_INTERPRETER_DEBUG
#define NOTE_ENTER(x)
#define NOTE_ENTER_(x)
#define NOTE_NAME(p0)
#define NOTE_EVAL1(p0)
#define NOTE_EVAL2(p0, p1)
#endif // WASM_INTERPRETER_DEBUG

// Execute an expression
template<typename SubType>
class ExpressionRunner : public OverriddenVisitor<SubType, Flow> {
protected:
  // Optional module context to search for globals and called functions. NULL if
  // we are not interested in any context.
  Module* module = nullptr;

  // Maximum depth before giving up.
  Index maxDepth;
  Index depth = 0;

  // Maximum iterations before giving up on a loop.
  Index maxLoopIterations;

  Flow generateArguments(const ExpressionList& operands,
                         LiteralList& arguments) {
    NOTE_ENTER_("generateArguments");
    arguments.reserve(operands.size());
    for (auto expression : operands) {
      Flow flow = this->visit(expression);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(flow.values);
      arguments.push_back(flow.getSingleValue());
    }
    return Flow();
  }

public:
  // Indicates no limit of maxDepth or maxLoopIterations.
  static const Index NO_LIMIT = 0;

  ExpressionRunner(Module* module = nullptr,
                   Index maxDepth = NO_LIMIT,
                   Index maxLoopIterations = NO_LIMIT)
    : module(module), maxDepth(maxDepth), maxLoopIterations(maxLoopIterations) {
  }
  virtual ~ExpressionRunner() = default;

  Flow visit(Expression* curr) {
    depth++;
    if (maxDepth != NO_LIMIT && depth > maxDepth) {
      hostLimit("interpreter recursion limit");
    }
    auto ret = OverriddenVisitor<SubType, Flow>::visit(curr);
    if (!ret.breaking()) {
      Type type = ret.getType();
      if (type.isConcrete() || curr->type.isConcrete()) {
#if 1 // def WASM_INTERPRETER_DEBUG
        if (!Type::isSubType(type, curr->type)) {
          std::cerr << "expected " << curr->type << ", seeing " << type
                    << " from\n"
                    << *curr << '\n';
        }
#endif
        assert(Type::isSubType(type, curr->type));
      }
    }
    depth--;
    return ret;
  }

  // Gets the module this runner is operating on.
  Module* getModule() { return module; }

  Flow visitBlock(Block* curr) {
    NOTE_ENTER("Block");
    // special-case Block, because Block nesting (in their first element) can be
    // incredibly deep
    std::vector<Block*> stack;
    stack.push_back(curr);
    while (curr->list.size() > 0 && curr->list[0]->is<Block>()) {
      curr = curr->list[0]->cast<Block>();
      stack.push_back(curr);
    }
    Flow flow;
    auto* top = stack.back();
    while (stack.size() > 0) {
      curr = stack.back();
      stack.pop_back();
      if (flow.breaking()) {
        flow.clearIf(curr->name);
        continue;
      }
      auto& list = curr->list;
      for (size_t i = 0; i < list.size(); i++) {
        if (curr != top && i == 0) {
          // one of the block recursions we already handled
          continue;
        }
        flow = visit(list[i]);
        if (flow.breaking()) {
          flow.clearIf(curr->name);
          break;
        }
      }
    }
    return flow;
  }
  Flow visitIf(If* curr) {
    NOTE_ENTER("If");
    Flow flow = visit(curr->condition);
    if (flow.breaking()) {
      return flow;
    }
    NOTE_EVAL1(flow.values);
    if (flow.getSingleValue().geti32()) {
      Flow flow = visit(curr->ifTrue);
      if (!flow.breaking() && !curr->ifFalse) {
        flow = Flow(); // if_else returns a value, but if does not
      }
      return flow;
    }
    if (curr->ifFalse) {
      return visit(curr->ifFalse);
    }
    return Flow();
  }
  Flow visitLoop(Loop* curr) {
    NOTE_ENTER("Loop");
    Index loopCount = 0;
    while (1) {
      Flow flow = visit(curr->body);
      if (flow.breaking()) {
        if (flow.breakTo == curr->name) {
          if (maxLoopIterations != NO_LIMIT &&
              ++loopCount >= maxLoopIterations) {
            return Flow(NONCONSTANT_FLOW);
          }
          continue; // lol
        }
      }
      // loop does not loop automatically, only continue achieves that
      return flow;
    }
  }
  Flow visitBreak(Break* curr) {
    NOTE_ENTER("Break");
    bool condition = true;
    Flow flow;
    if (curr->value) {
      flow = visit(curr->value);
      if (flow.breaking()) {
        return flow;
      }
    }
    if (curr->condition) {
      Flow conditionFlow = visit(curr->condition);
      if (conditionFlow.breaking()) {
        return conditionFlow;
      }
      condition = conditionFlow.getSingleValue().getInteger() != 0;
      if (!condition) {
        return flow;
      }
    }
    flow.breakTo = curr->name;
    return flow;
  }
  Flow visitSwitch(Switch* curr) {
    NOTE_ENTER("Switch");
    Flow flow;
    Literals values;
    if (curr->value) {
      flow = visit(curr->value);
      if (flow.breaking()) {
        return flow;
      }
      values = flow.values;
    }
    flow = visit(curr->condition);
    if (flow.breaking()) {
      return flow;
    }
    int64_t index = flow.getSingleValue().getInteger();
    Name target = curr->default_;
    if (index >= 0 && (size_t)index < curr->targets.size()) {
      target = curr->targets[(size_t)index];
    }
    flow.breakTo = target;
    flow.values = values;
    return flow;
  }

  Flow visitConst(Const* curr) {
    NOTE_ENTER("Const");
    NOTE_EVAL1(curr->value);
    return Flow(curr->value); // heh
  }

  // Unary and Binary nodes, the core math computations. We mostly just
  // delegate to the Literal::* methods, except we handle traps here.

  Flow visitUnary(Unary* curr) {
    NOTE_ENTER("Unary");
    Flow flow = visit(curr->value);
    if (flow.breaking()) {
      return flow;
    }
    Literal value = flow.getSingleValue();
    NOTE_EVAL1(value);
    switch (curr->op) {
      case ClzInt32:
      case ClzInt64:
        return value.countLeadingZeroes();
      case CtzInt32:
      case CtzInt64:
        return value.countTrailingZeroes();
      case PopcntInt32:
      case PopcntInt64:
        return value.popCount();
      case EqZInt32:
      case EqZInt64:
        return value.eqz();
      case ReinterpretInt32:
        return value.castToF32();
      case ReinterpretInt64:
        return value.castToF64();
      case ExtendSInt32:
        return value.extendToSI64();
      case ExtendUInt32:
        return value.extendToUI64();
      case WrapInt64:
        return value.wrapToI32();
      case ConvertUInt32ToFloat32:
      case ConvertUInt64ToFloat32:
        return value.convertUIToF32();
      case ConvertUInt32ToFloat64:
      case ConvertUInt64ToFloat64:
        return value.convertUIToF64();
      case ConvertSInt32ToFloat32:
      case ConvertSInt64ToFloat32:
        return value.convertSIToF32();
      case ConvertSInt32ToFloat64:
      case ConvertSInt64ToFloat64:
        return value.convertSIToF64();
      case ExtendS8Int32:
      case ExtendS8Int64:
        return value.extendS8();
      case ExtendS16Int32:
      case ExtendS16Int64:
        return value.extendS16();
      case ExtendS32Int64:
        return value.extendS32();

      case NegFloat32:
      case NegFloat64:
        return value.neg();
      case AbsFloat32:
      case AbsFloat64:
        return value.abs();
      case CeilFloat32:
      case CeilFloat64:
        return value.ceil();
      case FloorFloat32:
      case FloorFloat64:
        return value.floor();
      case TruncFloat32:
      case TruncFloat64:
        return value.trunc();
      case NearestFloat32:
      case NearestFloat64:
        return value.nearbyint();
      case SqrtFloat32:
      case SqrtFloat64:
        return value.sqrt();
      case TruncSFloat32ToInt32:
      case TruncSFloat64ToInt32:
      case TruncSFloat32ToInt64:
      case TruncSFloat64ToInt64:
        return truncSFloat(curr, value);
      case TruncUFloat32ToInt32:
      case TruncUFloat64ToInt32:
      case TruncUFloat32ToInt64:
      case TruncUFloat64ToInt64:
        return truncUFloat(curr, value);
      case TruncSatSFloat32ToInt32:
      case TruncSatSFloat64ToInt32:
        return value.truncSatToSI32();
      case TruncSatSFloat32ToInt64:
      case TruncSatSFloat64ToInt64:
        return value.truncSatToSI64();
      case TruncSatUFloat32ToInt32:
      case TruncSatUFloat64ToInt32:
        return value.truncSatToUI32();
      case TruncSatUFloat32ToInt64:
      case TruncSatUFloat64ToInt64:
        return value.truncSatToUI64();
      case ReinterpretFloat32:
        return value.castToI32();
      case PromoteFloat32:
        return value.extendToF64();
      case ReinterpretFloat64:
        return value.castToI64();
      case DemoteFloat64:
        return value.demote();
      case SplatVecI8x16:
        return value.splatI8x16();
      case SplatVecI16x8:
        return value.splatI16x8();
      case SplatVecI32x4:
        return value.splatI32x4();
      case SplatVecI64x2:
        return value.splatI64x2();
      case SplatVecF32x4:
        return value.splatF32x4();
      case SplatVecF64x2:
        return value.splatF64x2();
      case NotVec128:
        return value.notV128();
      case AnyTrueVec128:
        return value.anyTrueV128();
      case AbsVecI8x16:
        return value.absI8x16();
      case NegVecI8x16:
        return value.negI8x16();
      case AllTrueVecI8x16:
        return value.allTrueI8x16();
      case BitmaskVecI8x16:
        return value.bitmaskI8x16();
      case PopcntVecI8x16:
        return value.popcntI8x16();
      case AbsVecI16x8:
        return value.absI16x8();
      case NegVecI16x8:
        return value.negI16x8();
      case AllTrueVecI16x8:
        return value.allTrueI16x8();
      case BitmaskVecI16x8:
        return value.bitmaskI16x8();
      case AbsVecI32x4:
        return value.absI32x4();
      case NegVecI32x4:
        return value.negI32x4();
      case AllTrueVecI32x4:
        return value.allTrueI32x4();
      case BitmaskVecI32x4:
        return value.bitmaskI32x4();
      case AbsVecI64x2:
        return value.absI64x2();
      case NegVecI64x2:
        return value.negI64x2();
      case AllTrueVecI64x2:
        return value.allTrueI64x2();
      case BitmaskVecI64x2:
        WASM_UNREACHABLE("unimp");
      case AbsVecF32x4:
        return value.absF32x4();
      case NegVecF32x4:
        return value.negF32x4();
      case SqrtVecF32x4:
        return value.sqrtF32x4();
      case CeilVecF32x4:
        return value.ceilF32x4();
      case FloorVecF32x4:
        return value.floorF32x4();
      case TruncVecF32x4:
        return value.truncF32x4();
      case NearestVecF32x4:
        return value.nearestF32x4();
      case AbsVecF64x2:
        return value.absF64x2();
      case NegVecF64x2:
        return value.negF64x2();
      case SqrtVecF64x2:
        return value.sqrtF64x2();
      case CeilVecF64x2:
        return value.ceilF64x2();
      case FloorVecF64x2:
        return value.floorF64x2();
      case TruncVecF64x2:
        return value.truncF64x2();
      case NearestVecF64x2:
        return value.nearestF64x2();
      case ExtAddPairwiseSVecI8x16ToI16x8:
        WASM_UNREACHABLE("unimp");
      case ExtAddPairwiseUVecI8x16ToI16x8:
        WASM_UNREACHABLE("unimp");
      case ExtAddPairwiseSVecI16x8ToI32x4:
        WASM_UNREACHABLE("unimp");
      case ExtAddPairwiseUVecI16x8ToI32x4:
        WASM_UNREACHABLE("unimp");
      case TruncSatSVecF32x4ToVecI32x4:
        return value.truncSatToSI32x4();
      case TruncSatUVecF32x4ToVecI32x4:
        return value.truncSatToUI32x4();
      case ConvertSVecI32x4ToVecF32x4:
        return value.convertSToF32x4();
      case ConvertUVecI32x4ToVecF32x4:
        return value.convertUToF32x4();
      case ExtendLowSVecI8x16ToVecI16x8:
        return value.extendLowSToVecI16x8();
      case ExtendHighSVecI8x16ToVecI16x8:
        return value.extendHighSToVecI16x8();
      case ExtendLowUVecI8x16ToVecI16x8:
        return value.extendLowUToVecI16x8();
      case ExtendHighUVecI8x16ToVecI16x8:
        return value.extendHighUToVecI16x8();
      case ExtendLowSVecI16x8ToVecI32x4:
        return value.extendLowSToVecI32x4();
      case ExtendHighSVecI16x8ToVecI32x4:
        return value.extendHighSToVecI32x4();
      case ExtendLowUVecI16x8ToVecI32x4:
        return value.extendLowUToVecI32x4();
      case ExtendHighUVecI16x8ToVecI32x4:
        return value.extendHighUToVecI32x4();
      case ExtendLowSVecI32x4ToVecI64x2:
      case ExtendHighSVecI32x4ToVecI64x2:
      case ExtendLowUVecI32x4ToVecI64x2:
      case ExtendHighUVecI32x4ToVecI64x2:
      case ConvertLowSVecI32x4ToVecF64x2:
      case ConvertLowUVecI32x4ToVecF64x2:
      case TruncSatZeroSVecF64x2ToVecI32x4:
      case TruncSatZeroUVecF64x2ToVecI32x4:
      case DemoteZeroVecF64x2ToVecF32x4:
      case PromoteLowVecF32x4ToVecF64x2:
        WASM_UNREACHABLE("unimp");
      case InvalidUnary:
        WASM_UNREACHABLE("invalid unary op");
    }
    WASM_UNREACHABLE("invalid op");
  }
  Flow visitBinary(Binary* curr) {
    NOTE_ENTER("Binary");
    Flow flow = visit(curr->left);
    if (flow.breaking()) {
      return flow;
    }
    Literal left = flow.getSingleValue();
    flow = visit(curr->right);
    if (flow.breaking()) {
      return flow;
    }
    Literal right = flow.getSingleValue();
    NOTE_EVAL2(left, right);
    assert(curr->left->type.isConcrete() ? left.type == curr->left->type
                                         : true);
    assert(curr->right->type.isConcrete() ? right.type == curr->right->type
                                          : true);
    switch (curr->op) {
      case AddInt32:
      case AddInt64:
      case AddFloat32:
      case AddFloat64:
        return left.add(right);
      case SubInt32:
      case SubInt64:
      case SubFloat32:
      case SubFloat64:
        return left.sub(right);
      case MulInt32:
      case MulInt64:
      case MulFloat32:
      case MulFloat64:
        return left.mul(right);
      case DivSInt32: {
        if (right.getInteger() == 0) {
          trap("i32.div_s by 0");
        }
        if (left.getInteger() == std::numeric_limits<int32_t>::min() &&
            right.getInteger() == -1) {
          trap("i32.div_s overflow"); // signed division overflow
        }
        return left.divS(right);
      }
      case DivUInt32: {
        if (right.getInteger() == 0) {
          trap("i32.div_u by 0");
        }
        return left.divU(right);
      }
      case RemSInt32: {
        if (right.getInteger() == 0) {
          trap("i32.rem_s by 0");
        }
        if (left.getInteger() == std::numeric_limits<int32_t>::min() &&
            right.getInteger() == -1) {
          return Literal(int32_t(0));
        }
        return left.remS(right);
      }
      case RemUInt32: {
        if (right.getInteger() == 0) {
          trap("i32.rem_u by 0");
        }
        return left.remU(right);
      }
      case DivSInt64: {
        if (right.getInteger() == 0) {
          trap("i64.div_s by 0");
        }
        if (left.getInteger() == LLONG_MIN && right.getInteger() == -1LL) {
          trap("i64.div_s overflow"); // signed division overflow
        }
        return left.divS(right);
      }
      case DivUInt64: {
        if (right.getInteger() == 0) {
          trap("i64.div_u by 0");
        }
        return left.divU(right);
      }
      case RemSInt64: {
        if (right.getInteger() == 0) {
          trap("i64.rem_s by 0");
        }
        if (left.getInteger() == LLONG_MIN && right.getInteger() == -1LL) {
          return Literal(int64_t(0));
        }
        return left.remS(right);
      }
      case RemUInt64: {
        if (right.getInteger() == 0) {
          trap("i64.rem_u by 0");
        }
        return left.remU(right);
      }
      case DivFloat32:
      case DivFloat64:
        return left.div(right);
      case AndInt32:
      case AndInt64:
        return left.and_(right);
      case OrInt32:
      case OrInt64:
        return left.or_(right);
      case XorInt32:
      case XorInt64:
        return left.xor_(right);
      case ShlInt32:
      case ShlInt64:
        return left.shl(right);
      case ShrUInt32:
      case ShrUInt64:
        return left.shrU(right);
      case ShrSInt32:
      case ShrSInt64:
        return left.shrS(right);
      case RotLInt32:
      case RotLInt64:
        return left.rotL(right);
      case RotRInt32:
      case RotRInt64:
        return left.rotR(right);

      case EqInt32:
      case EqInt64:
      case EqFloat32:
      case EqFloat64:
        return left.eq(right);
      case NeInt32:
      case NeInt64:
      case NeFloat32:
      case NeFloat64:
        return left.ne(right);
      case LtSInt32:
      case LtSInt64:
        return left.ltS(right);
      case LtUInt32:
      case LtUInt64:
        return left.ltU(right);
      case LeSInt32:
      case LeSInt64:
        return left.leS(right);
      case LeUInt32:
      case LeUInt64:
        return left.leU(right);
      case GtSInt32:
      case GtSInt64:
        return left.gtS(right);
      case GtUInt32:
      case GtUInt64:
        return left.gtU(right);
      case GeSInt32:
      case GeSInt64:
        return left.geS(right);
      case GeUInt32:
      case GeUInt64:
        return left.geU(right);
      case LtFloat32:
      case LtFloat64:
        return left.lt(right);
      case LeFloat32:
      case LeFloat64:
        return left.le(right);
      case GtFloat32:
      case GtFloat64:
        return left.gt(right);
      case GeFloat32:
      case GeFloat64:
        return left.ge(right);

      case CopySignFloat32:
      case CopySignFloat64:
        return left.copysign(right);
      case MinFloat32:
      case MinFloat64:
        return left.min(right);
      case MaxFloat32:
      case MaxFloat64:
        return left.max(right);

      case EqVecI8x16:
        return left.eqI8x16(right);
      case NeVecI8x16:
        return left.neI8x16(right);
      case LtSVecI8x16:
        return left.ltSI8x16(right);
      case LtUVecI8x16:
        return left.ltUI8x16(right);
      case GtSVecI8x16:
        return left.gtSI8x16(right);
      case GtUVecI8x16:
        return left.gtUI8x16(right);
      case LeSVecI8x16:
        return left.leSI8x16(right);
      case LeUVecI8x16:
        return left.leUI8x16(right);
      case GeSVecI8x16:
        return left.geSI8x16(right);
      case GeUVecI8x16:
        return left.geUI8x16(right);
      case EqVecI16x8:
        return left.eqI16x8(right);
      case NeVecI16x8:
        return left.neI16x8(right);
      case LtSVecI16x8:
        return left.ltSI16x8(right);
      case LtUVecI16x8:
        return left.ltUI16x8(right);
      case GtSVecI16x8:
        return left.gtSI16x8(right);
      case GtUVecI16x8:
        return left.gtUI16x8(right);
      case LeSVecI16x8:
        return left.leSI16x8(right);
      case LeUVecI16x8:
        return left.leUI16x8(right);
      case GeSVecI16x8:
        return left.geSI16x8(right);
      case GeUVecI16x8:
        return left.geUI16x8(right);
      case EqVecI32x4:
        return left.eqI32x4(right);
      case NeVecI32x4:
        return left.neI32x4(right);
      case LtSVecI32x4:
        return left.ltSI32x4(right);
      case LtUVecI32x4:
        return left.ltUI32x4(right);
      case GtSVecI32x4:
        return left.gtSI32x4(right);
      case GtUVecI32x4:
        return left.gtUI32x4(right);
      case LeSVecI32x4:
        return left.leSI32x4(right);
      case LeUVecI32x4:
        return left.leUI32x4(right);
      case GeSVecI32x4:
        return left.geSI32x4(right);
      case GeUVecI32x4:
        return left.geUI32x4(right);
      case EqVecI64x2:
        return left.eqI64x2(right);
      case NeVecI64x2:
        return left.neI64x2(right);
      case LtSVecI64x2:
        return left.ltSI64x2(right);
      case GtSVecI64x2:
        return left.gtSI64x2(right);
      case LeSVecI64x2:
        return left.leSI64x2(right);
      case GeSVecI64x2:
        return left.geSI64x2(right);
      case EqVecF32x4:
        return left.eqF32x4(right);
      case NeVecF32x4:
        return left.neF32x4(right);
      case LtVecF32x4:
        return left.ltF32x4(right);
      case GtVecF32x4:
        return left.gtF32x4(right);
      case LeVecF32x4:
        return left.leF32x4(right);
      case GeVecF32x4:
        return left.geF32x4(right);
      case EqVecF64x2:
        return left.eqF64x2(right);
      case NeVecF64x2:
        return left.neF64x2(right);
      case LtVecF64x2:
        return left.ltF64x2(right);
      case GtVecF64x2:
        return left.gtF64x2(right);
      case LeVecF64x2:
        return left.leF64x2(right);
      case GeVecF64x2:
        return left.geF64x2(right);

      case AndVec128:
        return left.andV128(right);
      case OrVec128:
        return left.orV128(right);
      case XorVec128:
        return left.xorV128(right);
      case AndNotVec128:
        return left.andV128(right.notV128());

      case AddVecI8x16:
        return left.addI8x16(right);
      case AddSatSVecI8x16:
        return left.addSaturateSI8x16(right);
      case AddSatUVecI8x16:
        return left.addSaturateUI8x16(right);
      case SubVecI8x16:
        return left.subI8x16(right);
      case SubSatSVecI8x16:
        return left.subSaturateSI8x16(right);
      case SubSatUVecI8x16:
        return left.subSaturateUI8x16(right);
      case MinSVecI8x16:
        return left.minSI8x16(right);
      case MinUVecI8x16:
        return left.minUI8x16(right);
      case MaxSVecI8x16:
        return left.maxSI8x16(right);
      case MaxUVecI8x16:
        return left.maxUI8x16(right);
      case AvgrUVecI8x16:
        return left.avgrUI8x16(right);
      case AddVecI16x8:
        return left.addI16x8(right);
      case AddSatSVecI16x8:
        return left.addSaturateSI16x8(right);
      case AddSatUVecI16x8:
        return left.addSaturateUI16x8(right);
      case SubVecI16x8:
        return left.subI16x8(right);
      case SubSatSVecI16x8:
        return left.subSaturateSI16x8(right);
      case SubSatUVecI16x8:
        return left.subSaturateUI16x8(right);
      case MulVecI16x8:
        return left.mulI16x8(right);
      case MinSVecI16x8:
        return left.minSI16x8(right);
      case MinUVecI16x8:
        return left.minUI16x8(right);
      case MaxSVecI16x8:
        return left.maxSI16x8(right);
      case MaxUVecI16x8:
        return left.maxUI16x8(right);
      case AvgrUVecI16x8:
        return left.avgrUI16x8(right);
      case Q15MulrSatSVecI16x8:
        return left.q15MulrSatSI16x8(right);
      case ExtMulLowSVecI16x8:
        return left.extMulLowSI16x8(right);
      case ExtMulHighSVecI16x8:
        return left.extMulHighSI16x8(right);
      case ExtMulLowUVecI16x8:
        return left.extMulLowUI16x8(right);
      case ExtMulHighUVecI16x8:
        return left.extMulHighUI16x8(right);
      case AddVecI32x4:
        return left.addI32x4(right);
      case SubVecI32x4:
        return left.subI32x4(right);
      case MulVecI32x4:
        return left.mulI32x4(right);
      case MinSVecI32x4:
        return left.minSI32x4(right);
      case MinUVecI32x4:
        return left.minUI32x4(right);
      case MaxSVecI32x4:
        return left.maxSI32x4(right);
      case MaxUVecI32x4:
        return left.maxUI32x4(right);
      case DotSVecI16x8ToVecI32x4:
        return left.dotSI16x8toI32x4(right);
      case ExtMulLowSVecI32x4:
        return left.extMulLowSI32x4(right);
      case ExtMulHighSVecI32x4:
        return left.extMulHighSI32x4(right);
      case ExtMulLowUVecI32x4:
        return left.extMulLowUI32x4(right);
      case ExtMulHighUVecI32x4:
        return left.extMulHighUI32x4(right);
      case AddVecI64x2:
        return left.addI64x2(right);
      case SubVecI64x2:
        return left.subI64x2(right);
      case MulVecI64x2:
        return left.mulI64x2(right);
      case ExtMulLowSVecI64x2:
        return left.extMulLowSI64x2(right);
      case ExtMulHighSVecI64x2:
        return left.extMulHighSI64x2(right);
      case ExtMulLowUVecI64x2:
        return left.extMulLowUI64x2(right);
      case ExtMulHighUVecI64x2:
        return left.extMulHighUI64x2(right);

      case AddVecF32x4:
        return left.addF32x4(right);
      case SubVecF32x4:
        return left.subF32x4(right);
      case MulVecF32x4:
        return left.mulF32x4(right);
      case DivVecF32x4:
        return left.divF32x4(right);
      case MinVecF32x4:
        return left.minF32x4(right);
      case MaxVecF32x4:
        return left.maxF32x4(right);
      case PMinVecF32x4:
        return left.pminF32x4(right);
      case PMaxVecF32x4:
        return left.pmaxF32x4(right);
      case AddVecF64x2:
        return left.addF64x2(right);
      case SubVecF64x2:
        return left.subF64x2(right);
      case MulVecF64x2:
        return left.mulF64x2(right);
      case DivVecF64x2:
        return left.divF64x2(right);
      case MinVecF64x2:
        return left.minF64x2(right);
      case MaxVecF64x2:
        return left.maxF64x2(right);
      case PMinVecF64x2:
        return left.pminF64x2(right);
      case PMaxVecF64x2:
        return left.pmaxF64x2(right);

      case NarrowSVecI16x8ToVecI8x16:
        return left.narrowSToVecI8x16(right);
      case NarrowUVecI16x8ToVecI8x16:
        return left.narrowUToVecI8x16(right);
      case NarrowSVecI32x4ToVecI16x8:
        return left.narrowSToVecI16x8(right);
      case NarrowUVecI32x4ToVecI16x8:
        return left.narrowUToVecI16x8(right);

      case SwizzleVec8x16:
        return left.swizzleVec8x16(right);

      case InvalidBinary:
        WASM_UNREACHABLE("invalid binary op");
    }
    WASM_UNREACHABLE("invalid op");
  }
  Flow visitSIMDExtract(SIMDExtract* curr) {
    NOTE_ENTER("SIMDExtract");
    Flow flow = this->visit(curr->vec);
    if (flow.breaking()) {
      return flow;
    }
    Literal vec = flow.getSingleValue();
    switch (curr->op) {
      case ExtractLaneSVecI8x16:
        return vec.extractLaneSI8x16(curr->index);
      case ExtractLaneUVecI8x16:
        return vec.extractLaneUI8x16(curr->index);
      case ExtractLaneSVecI16x8:
        return vec.extractLaneSI16x8(curr->index);
      case ExtractLaneUVecI16x8:
        return vec.extractLaneUI16x8(curr->index);
      case ExtractLaneVecI32x4:
        return vec.extractLaneI32x4(curr->index);
      case ExtractLaneVecI64x2:
        return vec.extractLaneI64x2(curr->index);
      case ExtractLaneVecF32x4:
        return vec.extractLaneF32x4(curr->index);
      case ExtractLaneVecF64x2:
        return vec.extractLaneF64x2(curr->index);
    }
    WASM_UNREACHABLE("invalid op");
  }
  Flow visitSIMDReplace(SIMDReplace* curr) {
    NOTE_ENTER("SIMDReplace");
    Flow flow = this->visit(curr->vec);
    if (flow.breaking()) {
      return flow;
    }
    Literal vec = flow.getSingleValue();
    flow = this->visit(curr->value);
    if (flow.breaking()) {
      return flow;
    }
    Literal value = flow.getSingleValue();
    switch (curr->op) {
      case ReplaceLaneVecI8x16:
        return vec.replaceLaneI8x16(value, curr->index);
      case ReplaceLaneVecI16x8:
        return vec.replaceLaneI16x8(value, curr->index);
      case ReplaceLaneVecI32x4:
        return vec.replaceLaneI32x4(value, curr->index);
      case ReplaceLaneVecI64x2:
        return vec.replaceLaneI64x2(value, curr->index);
      case ReplaceLaneVecF32x4:
        return vec.replaceLaneF32x4(value, curr->index);
      case ReplaceLaneVecF64x2:
        return vec.replaceLaneF64x2(value, curr->index);
    }
    WASM_UNREACHABLE("invalid op");
  }
  Flow visitSIMDShuffle(SIMDShuffle* curr) {
    NOTE_ENTER("SIMDShuffle");
    Flow flow = this->visit(curr->left);
    if (flow.breaking()) {
      return flow;
    }
    Literal left = flow.getSingleValue();
    flow = this->visit(curr->right);
    if (flow.breaking()) {
      return flow;
    }
    Literal right = flow.getSingleValue();
    return left.shuffleV8x16(right, curr->mask);
  }
  Flow visitSIMDTernary(SIMDTernary* curr) {
    NOTE_ENTER("SIMDBitselect");
    Flow flow = this->visit(curr->a);
    if (flow.breaking()) {
      return flow;
    }
    Literal a = flow.getSingleValue();
    flow = this->visit(curr->b);
    if (flow.breaking()) {
      return flow;
    }
    Literal b = flow.getSingleValue();
    flow = this->visit(curr->c);
    if (flow.breaking()) {
      return flow;
    }
    Literal c = flow.getSingleValue();
    switch (curr->op) {
      case Bitselect:
        return c.bitselectV128(a, b);
      default:
        // TODO: implement qfma/qfms and signselect
        WASM_UNREACHABLE("not implemented");
    }
  }
  Flow visitSIMDShift(SIMDShift* curr) {
    NOTE_ENTER("SIMDShift");
    Flow flow = this->visit(curr->vec);
    if (flow.breaking()) {
      return flow;
    }
    Literal vec = flow.getSingleValue();
    flow = this->visit(curr->shift);
    if (flow.breaking()) {
      return flow;
    }
    Literal shift = flow.getSingleValue();
    switch (curr->op) {
      case ShlVecI8x16:
        return vec.shlI8x16(shift);
      case ShrSVecI8x16:
        return vec.shrSI8x16(shift);
      case ShrUVecI8x16:
        return vec.shrUI8x16(shift);
      case ShlVecI16x8:
        return vec.shlI16x8(shift);
      case ShrSVecI16x8:
        return vec.shrSI16x8(shift);
      case ShrUVecI16x8:
        return vec.shrUI16x8(shift);
      case ShlVecI32x4:
        return vec.shlI32x4(shift);
      case ShrSVecI32x4:
        return vec.shrSI32x4(shift);
      case ShrUVecI32x4:
        return vec.shrUI32x4(shift);
      case ShlVecI64x2:
        return vec.shlI64x2(shift);
      case ShrSVecI64x2:
        return vec.shrSI64x2(shift);
      case ShrUVecI64x2:
        return vec.shrUI64x2(shift);
    }
    WASM_UNREACHABLE("invalid op");
  }
  Flow visitSelect(Select* curr) {
    NOTE_ENTER("Select");
    Flow ifTrue = visit(curr->ifTrue);
    if (ifTrue.breaking()) {
      return ifTrue;
    }
    Flow ifFalse = visit(curr->ifFalse);
    if (ifFalse.breaking()) {
      return ifFalse;
    }
    Flow condition = visit(curr->condition);
    if (condition.breaking()) {
      return condition;
    }
    NOTE_EVAL1(condition.getSingleValue());
    return condition.getSingleValue().geti32() ? ifTrue : ifFalse; // ;-)
  }
  Flow visitDrop(Drop* curr) {
    NOTE_ENTER("Drop");
    Flow value = visit(curr->value);
    if (value.breaking()) {
      return value;
    }
    return Flow();
  }
  Flow visitReturn(Return* curr) {
    NOTE_ENTER("Return");
    Flow flow;
    if (curr->value) {
      flow = visit(curr->value);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(flow.getSingleValue());
    }
    flow.breakTo = RETURN_FLOW;
    return flow;
  }
  Flow visitNop(Nop* curr) {
    NOTE_ENTER("Nop");
    return Flow();
  }
  Flow visitUnreachable(Unreachable* curr) {
    NOTE_ENTER("Unreachable");
    trap("unreachable");
    WASM_UNREACHABLE("unreachable");
  }

  Literal truncSFloat(Unary* curr, Literal value) {
    double val = value.getFloat();
    if (std::isnan(val)) {
      trap("truncSFloat of nan");
    }
    if (curr->type == Type::i32) {
      if (value.type == Type::f32) {
        if (!isInRangeI32TruncS(value.reinterpreti32())) {
          trap("i32.truncSFloat overflow");
        }
      } else {
        if (!isInRangeI32TruncS(value.reinterpreti64())) {
          trap("i32.truncSFloat overflow");
        }
      }
      return Literal(int32_t(val));
    } else {
      if (value.type == Type::f32) {
        if (!isInRangeI64TruncS(value.reinterpreti32())) {
          trap("i64.truncSFloat overflow");
        }
      } else {
        if (!isInRangeI64TruncS(value.reinterpreti64())) {
          trap("i64.truncSFloat overflow");
        }
      }
      return Literal(int64_t(val));
    }
  }

  Literal truncUFloat(Unary* curr, Literal value) {
    double val = value.getFloat();
    if (std::isnan(val)) {
      trap("truncUFloat of nan");
    }
    if (curr->type == Type::i32) {
      if (value.type == Type::f32) {
        if (!isInRangeI32TruncU(value.reinterpreti32())) {
          trap("i32.truncUFloat overflow");
        }
      } else {
        if (!isInRangeI32TruncU(value.reinterpreti64())) {
          trap("i32.truncUFloat overflow");
        }
      }
      return Literal(uint32_t(val));
    } else {
      if (value.type == Type::f32) {
        if (!isInRangeI64TruncU(value.reinterpreti32())) {
          trap("i64.truncUFloat overflow");
        }
      } else {
        if (!isInRangeI64TruncU(value.reinterpreti64())) {
          trap("i64.truncUFloat overflow");
        }
      }
      return Literal(uint64_t(val));
    }
  }
  Flow visitAtomicFence(AtomicFence* curr) {
    // Wasm currently supports only sequentially consistent atomics, in which
    // case atomic_fence can be lowered to nothing.
    NOTE_ENTER("AtomicFence");
    return Flow();
  }
  Flow visitTupleMake(TupleMake* curr) {
    NOTE_ENTER("tuple.make");
    LiteralList arguments;
    Flow flow = generateArguments(curr->operands, arguments);
    if (flow.breaking()) {
      return flow;
    }
    for (auto arg : arguments) {
      assert(arg.type.isConcrete());
      flow.values.push_back(arg);
    }
    return flow;
  }
  Flow visitTupleExtract(TupleExtract* curr) {
    NOTE_ENTER("tuple.extract");
    Flow flow = visit(curr->tuple);
    if (flow.breaking()) {
      return flow;
    }
    assert(flow.values.size() > curr->index);
    return Flow(flow.values[curr->index]);
  }
  Flow visitLocalGet(LocalGet* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitLocalSet(LocalSet* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitGlobalGet(GlobalGet* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitGlobalSet(GlobalSet* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitCall(Call* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitCallIndirect(CallIndirect* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitLoad(Load* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitStore(Store* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitMemorySize(MemorySize* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitMemoryGrow(MemoryGrow* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitMemoryInit(MemoryInit* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitDataDrop(DataDrop* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitMemoryCopy(MemoryCopy* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitMemoryFill(MemoryFill* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableGet(TableGet* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableSet(TableSet* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableSize(TableSize* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableGrow(TableGrow* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableFill(TableFill* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableCopy(TableCopy* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitTableInit(TableInit* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitElemDrop(ElemDrop* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitAtomicRMW(AtomicRMW* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitAtomicCmpxchg(AtomicCmpxchg* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitAtomicWait(AtomicWait* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitAtomicNotify(AtomicNotify* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitSIMDLoad(SIMDLoad* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitSIMDLoadSplat(SIMDLoad* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitSIMDLoadExtend(SIMDLoad* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitSIMDLoadZero(SIMDLoad* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
    WASM_UNREACHABLE("unimp");
  }
  Flow visitPop(Pop* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitCallRef(CallRef* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitRefNull(RefNull* curr) {
    NOTE_ENTER("RefNull");
    return Literal::makeNull(curr->type);
  }
  Flow visitRefIs(RefIs* curr) {
    NOTE_ENTER("RefIs");
    Flow flow = visit(curr->value);
    if (flow.breaking()) {
      return flow;
    }
    const auto& value = flow.getSingleValue();
    NOTE_EVAL1(value);
    switch (curr->op) {
      case RefIsNull:
        return Literal(value.isNull());
      case RefIsFunc:
        return Literal(!value.isNull() && value.type.isFunction());
      case RefIsData:
        return Literal(!value.isNull() && value.isData());
      case RefIsI31:
        return Literal(!value.isNull() &&
                       value.type.getHeapType() == HeapType::i31);
      default:
        WASM_UNREACHABLE("unimplemented ref.is_*");
    }
  }
  Flow visitRefFunc(RefFunc* curr) {
    NOTE_ENTER("RefFunc");
    NOTE_NAME(curr->func);
    return Literal::makeFunc(curr->func, curr->type);
  }
  Flow visitRefEq(RefEq* curr) {
    NOTE_ENTER("RefEq");
    Flow flow = visit(curr->left);
    if (flow.breaking()) {
      return flow;
    }
    auto left = flow.getSingleValue();
    flow = visit(curr->right);
    if (flow.breaking()) {
      return flow;
    }
    auto right = flow.getSingleValue();
    NOTE_EVAL2(left, right);
    return Literal(int32_t(left == right));
  }
  Flow visitTry(Try* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitThrow(Throw* curr) {
    NOTE_ENTER("Throw");
    LiteralList arguments;
    Flow flow = generateArguments(curr->operands, arguments);
    if (flow.breaking()) {
      return flow;
    }
    NOTE_EVAL1(curr->event);
    WasmException exn;
    exn.event = curr->event;
    for (auto item : arguments) {
      exn.values.push_back(item);
    }
    throwException(exn);
    WASM_UNREACHABLE("throw");
  }
  Flow visitRethrow(Rethrow* curr) { WASM_UNREACHABLE("unimp"); }
  Flow visitI31New(I31New* curr) {
    NOTE_ENTER("I31New");
    Flow flow = visit(curr->value);
    if (flow.breaking()) {
      return flow;
    }
    const auto& value = flow.getSingleValue();
    NOTE_EVAL1(value);
    return Literal::makeI31(value.geti32());
  }
  Flow visitI31Get(I31Get* curr) {
    NOTE_ENTER("I31Get");
    Flow flow = visit(curr->i31);
    if (flow.breaking()) {
      return flow;
    }
    const auto& value = flow.getSingleValue();
    NOTE_EVAL1(value);
    return Literal(value.geti31(curr->signed_));
  }

  // Helper for ref.test, ref.cast, and br_on_cast, which share almost all their
  // logic except for what they return.
  struct Cast {
    enum Outcome {
      // We took a break before doing anything.
      Break,
      // The input was null.
      Null,
      // The cast succeeded.
      Success,
      // The cast failed.
      Failure
    } outcome;

    Flow breaking;
    Literal originalRef;
    Literal castRef;
  };

  template<typename T> Cast doCast(T* curr) {
    Cast cast;
    Flow ref = this->visit(curr->ref);
    if (ref.breaking()) {
      cast.outcome = cast.Break;
      cast.breaking = ref;
      return cast;
    }
    Flow rtt = this->visit(curr->rtt);
    if (rtt.breaking()) {
      cast.outcome = cast.Break;
      cast.breaking = rtt;
      return cast;
    }
    cast.originalRef = ref.getSingleValue();
    if (cast.originalRef.isNull()) {
      cast.outcome = cast.Null;
      return cast;
    }
    // The input may not be GC data or a function; for example it could be an
    // anyref of null (already handled above) or anything else (handled here,
    // but this is for future use as atm the binaryen interpreter cannot
    // represent external references).
    if (!cast.originalRef.isData() && !cast.originalRef.isFunction()) {
      cast.outcome = cast.Failure;
      return cast;
    }
    Literal seenRtt;
    Literal intendedRtt = rtt.getSingleValue();
    if (cast.originalRef.isFunction()) {
      // Function casts are simple in that they have no RTT hierarchies; instead
      // each reference has the canonical RTT for the signature.
      // We must have a module in order to perform the cast, to get the type. If
      // we do not have one, or if the function is not present (which may happen
      // if we are optimizing a function before the entire module is built),
      // then this is something we cannot precompute.
      auto* func = module
                     ? module->getFunctionOrNull(cast.originalRef.getFunc())
                     : nullptr;
      if (!func) {
        cast.outcome = cast.Break;
        cast.breaking = NONCONSTANT_FLOW;
        return cast;
      }
      seenRtt = Literal(Type(Rtt(0, func->sig)));
      if (!seenRtt.isSubRtt(intendedRtt)) {
        cast.outcome = cast.Failure;
        return cast;
      }
      cast.castRef =
        Literal(func->name, Type(intendedRtt.type.getHeapType(), NonNullable));
    } else {
      // GC data store an RTT in each instance.
      assert(cast.originalRef.isData());
      auto gcData = cast.originalRef.getGCData();
      seenRtt = gcData->rtt;
      if (!seenRtt.isSubRtt(intendedRtt)) {
        cast.outcome = cast.Failure;
        return cast;
      }
      cast.castRef =
        Literal(gcData, Type(intendedRtt.type.getHeapType(), NonNullable));
    }
    cast.outcome = cast.Success;
    return cast;
  }

  Flow visitRefTest(RefTest* curr) {
    NOTE_ENTER("RefTest");
    auto cast = doCast(curr);
    if (cast.outcome == cast.Break) {
      return cast.breaking;
    }
    return Literal(int32_t(cast.outcome == cast.Success));
  }
  Flow visitRefCast(RefCast* curr) {
    NOTE_ENTER("RefCast");
    auto cast = doCast(curr);
    if (cast.outcome == cast.Break) {
      return cast.breaking;
    }
    if (cast.outcome == cast.Null) {
      return Literal::makeNull(Type(curr->type.getHeapType(), Nullable));
    }
    if (cast.outcome == cast.Failure) {
      trap("cast error");
    }
    assert(cast.outcome == cast.Success);
    return cast.castRef;
  }
  Flow visitBrOn(BrOn* curr) {
    NOTE_ENTER("BrOn");
    // BrOnCast* uses the casting infrastructure, so handle it first.
    if (curr->op == BrOnCast || curr->op == BrOnCastFail) {
      auto cast = doCast(curr);
      if (cast.outcome == cast.Break) {
        return cast.breaking;
      }
      if (cast.outcome == cast.Null || cast.outcome == cast.Failure) {
        if (curr->op == BrOnCast) {
          return cast.originalRef;
        } else {
          return Flow(curr->name, cast.originalRef);
        }
      }
      assert(cast.outcome == cast.Success);
      if (curr->op == BrOnCast) {
        return Flow(curr->name, cast.castRef);
      } else {
        return cast.castRef;
      }
    }
    // The others do a simpler check for the type.
    Flow flow = visit(curr->ref);
    if (flow.breaking()) {
      return flow;
    }
    const auto& value = flow.getSingleValue();
    NOTE_EVAL1(value);
    if (curr->op == BrOnNull) {
      // Unlike the others, BrOnNull does not propagate the value if it takes
      // the branch.
      if (value.isNull()) {
        return Flow(curr->name);
      }
      // If the branch is not taken, we return the non-null value.
      return {value};
    }
    if (curr->op == BrOnNonNull) {
      // Unlike the others, BrOnNonNull does not return a value if it does not
      // take the branch.
      if (value.isNull()) {
        return Flow();
      }
      // If the branch is taken, we send the non-null value.
      return Flow(curr->name, value);
    }
    // See if the input is the right kind (ignoring the flipping behavior of
    // BrOn*).
    bool isRightKind;
    if (value.isNull()) {
      // A null is never the right kind.
      isRightKind = false;
    } else {
      switch (curr->op) {
        case BrOnNonFunc:
        case BrOnFunc:
          isRightKind = value.type.isFunction();
          break;
        case BrOnNonData:
        case BrOnData:
          isRightKind = value.isData();
          break;
        case BrOnNonI31:
        case BrOnI31:
          isRightKind = value.type.getHeapType() == HeapType::i31;
          break;
        default:
          WASM_UNREACHABLE("invalid br_on_*");
      }
    }
    // The Non* operations require us to flip the normal behavior.
    switch (curr->op) {
      case BrOnNonFunc:
      case BrOnNonData:
      case BrOnNonI31:
        isRightKind = !isRightKind;
        break;
      default: {
      }
    }
    if (isRightKind) {
      // Take the branch.
      return Flow(curr->name, value);
    }
    return {value};
  }
  Flow visitRttCanon(RttCanon* curr) { return Literal(curr->type); }
  Flow visitRttSub(RttSub* curr) {
    Flow parent = this->visit(curr->parent);
    if (parent.breaking()) {
      return parent;
    }
    auto parentValue = parent.getSingleValue();
    auto newSupers = std::make_unique<RttSupers>(parentValue.getRttSupers());
    newSupers->push_back(parentValue.type);
    if (curr->fresh) {
      newSupers->back().makeFresh();
    }
    return Literal(std::move(newSupers), curr->type);
  }
  Flow visitStructNew(StructNew* curr) {
    NOTE_ENTER("StructNew");
    auto rtt = this->visit(curr->rtt);
    if (rtt.breaking()) {
      return rtt;
    }
    const auto& fields = curr->rtt->type.getHeapType().getStruct().fields;
    Literals data(fields.size());
    for (Index i = 0; i < fields.size(); i++) {
      if (curr->isWithDefault()) {
        data[i] = Literal::makeZero(fields[i].type);
      } else {
        auto value = this->visit(curr->operands[i]);
        if (value.breaking()) {
          return value;
        }
        data[i] = value.getSingleValue();
      }
    }
    return Flow(Literal(std::make_shared<GCData>(rtt.getSingleValue(), data),
                        curr->type));
  }
  Flow visitStructGet(StructGet* curr) {
    NOTE_ENTER("StructGet");
    Flow ref = this->visit(curr->ref);
    if (ref.breaking()) {
      return ref;
    }
    auto data = ref.getSingleValue().getGCData();
    if (!data) {
      trap("null ref");
    }
    auto field = curr->ref->type.getHeapType().getStruct().fields[curr->index];
    return extendForPacking(data->values[curr->index], field, curr->signed_);
  }
  Flow visitStructSet(StructSet* curr) {
    NOTE_ENTER("StructSet");
    Flow ref = this->visit(curr->ref);
    if (ref.breaking()) {
      return ref;
    }
    Flow value = this->visit(curr->value);
    if (value.breaking()) {
      return value;
    }
    auto data = ref.getSingleValue().getGCData();
    if (!data) {
      trap("null ref");
    }
    auto field = curr->ref->type.getHeapType().getStruct().fields[curr->index];
    data->values[curr->index] =
      truncateForPacking(value.getSingleValue(), field);
    return Flow();
  }

  // Arbitrary deterministic limit on size. If we need to allocate a Literals
  // vector that takes around 1-2GB of memory then we are likely to hit memory
  // limits on 32-bit machines, and in particular on wasm32 VMs that do not
  // have 4GB support, so give up there.
  static const Index ArrayLimit = (1 << 30) / sizeof(Literal);

  Flow visitArrayNew(ArrayNew* curr) {
    NOTE_ENTER("ArrayNew");
    auto rtt = this->visit(curr->rtt);
    if (rtt.breaking()) {
      return rtt;
    }
    auto size = this->visit(curr->size);
    if (size.breaking()) {
      return size;
    }
    const auto& element = curr->rtt->type.getHeapType().getArray().element;
    Index num = size.getSingleValue().geti32();
    if (num >= ArrayLimit) {
      hostLimit("allocation failure");
    }
    Literals data(num);
    if (curr->isWithDefault()) {
      for (Index i = 0; i < num; i++) {
        data[i] = Literal::makeZero(element.type);
      }
    } else {
      auto init = this->visit(curr->init);
      if (init.breaking()) {
        return init;
      }
      auto field = curr->type.getHeapType().getArray().element;
      auto value = truncateForPacking(init.getSingleValue(), field);
      for (Index i = 0; i < num; i++) {
        data[i] = value;
      }
    }
    return Flow(Literal(std::make_shared<GCData>(rtt.getSingleValue(), data),
                        curr->type));
  }
  Flow visitArrayGet(ArrayGet* curr) {
    NOTE_ENTER("ArrayGet");
    Flow ref = this->visit(curr->ref);
    if (ref.breaking()) {
      return ref;
    }
    Flow index = this->visit(curr->index);
    if (index.breaking()) {
      return index;
    }
    auto data = ref.getSingleValue().getGCData();
    if (!data) {
      trap("null ref");
    }
    Index i = index.getSingleValue().geti32();
    if (i >= data->values.size()) {
      trap("array oob");
    }
    auto field = curr->ref->type.getHeapType().getArray().element;
    return extendForPacking(data->values[i], field, curr->signed_);
  }
  Flow visitArraySet(ArraySet* curr) {
    NOTE_ENTER("ArraySet");
    Flow ref = this->visit(curr->ref);
    if (ref.breaking()) {
      return ref;
    }
    Flow index = this->visit(curr->index);
    if (index.breaking()) {
      return index;
    }
    Flow value = this->visit(curr->value);
    if (value.breaking()) {
      return value;
    }
    auto data = ref.getSingleValue().getGCData();
    if (!data) {
      trap("null ref");
    }
    Index i = index.getSingleValue().geti32();
    if (i >= data->values.size()) {
      trap("array oob");
    }
    auto field = curr->ref->type.getHeapType().getArray().element;
    data->values[i] = truncateForPacking(value.getSingleValue(), field);
    return Flow();
  }
  Flow visitArrayLen(ArrayLen* curr) {
    NOTE_ENTER("ArrayLen");
    Flow ref = this->visit(curr->ref);
    if (ref.breaking()) {
      return ref;
    }
    auto data = ref.getSingleValue().getGCData();
    if (!data) {
      trap("null ref");
    }
    return Literal(int32_t(data->values.size()));
  }
  Flow visitArrayCopy(ArrayCopy* curr) {
    NOTE_ENTER("ArrayCopy");
    Flow destRef = this->visit(curr->destRef);
    if (destRef.breaking()) {
      return destRef;
    }
    Flow destIndex = this->visit(curr->destIndex);
    if (destIndex.breaking()) {
      return destIndex;
    }
    Flow srcRef = this->visit(curr->srcRef);
    if (srcRef.breaking()) {
      return srcRef;
    }
    Flow srcIndex = this->visit(curr->srcIndex);
    if (srcIndex.breaking()) {
      return srcIndex;
    }
    Flow length = this->visit(curr->length);
    if (length.breaking()) {
      return length;
    }
    auto destData = destRef.getSingleValue().getGCData();
    if (!destData) {
      trap("null ref");
    }
    auto srcData = srcRef.getSingleValue().getGCData();
    if (!srcData) {
      trap("null ref");
    }
    size_t destVal = destIndex.getSingleValue().getUnsigned();
    size_t srcVal = srcIndex.getSingleValue().getUnsigned();
    size_t lengthVal = length.getSingleValue().getUnsigned();
    if (lengthVal >= ArrayLimit) {
      hostLimit("allocation failure");
    }
    std::vector<Literal> copied;
    copied.resize(lengthVal);
    for (size_t i = 0; i < lengthVal; i++) {
      if (srcVal + i >= srcData->values.size()) {
        trap("oob");
      }
      copied[i] = srcData->values[srcVal + i];
    }
    for (size_t i = 0; i < lengthVal; i++) {
      if (destVal + i >= destData->values.size()) {
        trap("oob");
      }
      destData->values[destVal + i] = copied[i];
    }
    return Flow();
  }
  Flow visitRefAs(RefAs* curr) {
    NOTE_ENTER("RefAs");
    Flow flow = visit(curr->value);
    if (flow.breaking()) {
      return flow;
    }
    const auto& value = flow.getSingleValue();
    NOTE_EVAL1(value);
    if (value.isNull()) {
      trap("null ref");
    }
    switch (curr->op) {
      case RefAsNonNull:
        // We've already checked for a null.
        break;
      case RefAsFunc:
        if (value.type.isFunction()) {
          trap("not a func");
        }
        break;
      case RefAsData:
        if (value.isData()) {
          trap("not a data");
        }
        break;
      case RefAsI31:
        if (value.type.getHeapType() != HeapType::i31) {
          trap("not an i31");
        }
        break;
      default:
        WASM_UNREACHABLE("unimplemented ref.is_*");
    }
    return value;
  }

  virtual void trap(const char* why) { WASM_UNREACHABLE("unimp"); }

  virtual void hostLimit(const char* why) { WASM_UNREACHABLE("unimp"); }

  virtual void throwException(const WasmException& exn) {
    WASM_UNREACHABLE("unimp");
  }

private:
  // Truncate the value if we need to. The storage is just a list of Literals,
  // so we can't just write the value like we would to a C struct field and
  // expect it to truncate for us. Instead, we truncate so the stored value is
  // proper for the type.
  Literal truncateForPacking(Literal value, const Field& field) {
    if (field.type == Type::i32) {
      int32_t c = value.geti32();
      if (field.packedType == Field::i8) {
        value = Literal(c & 0xff);
      } else if (field.packedType == Field::i16) {
        value = Literal(c & 0xffff);
      }
    }
    return value;
  }

  Literal extendForPacking(Literal value, const Field& field, bool signed_) {
    if (field.type == Type::i32) {
      int32_t c = value.geti32();
      if (field.packedType == Field::i8) {
        // The stored value should already be truncated.
        assert(c == (c & 0xff));
        if (signed_) {
          value = Literal((c << 24) >> 24);
        }
      } else if (field.packedType == Field::i16) {
        assert(c == (c & 0xffff));
        if (signed_) {
          value = Literal((c << 16) >> 16);
        }
      }
    }
    return value;
  }
};

// Execute a suspected constant expression (precompute and C-API).
template<typename SubType>
class ConstantExpressionRunner : public ExpressionRunner<SubType> {
public:
  enum FlagValues {
    // By default, just evaluate the expression, i.e. all we want to know is
    // whether it computes down to a concrete value, where it is not necessary
    // to preserve side effects like those of a `local.tee`.
    DEFAULT = 0,
    // Be very careful to preserve any side effects. For example, if we are
    // intending to replace the expression with a constant afterwards, even if
    // we can technically evaluate down to a constant, we still cannot replace
    // the expression if it also sets a local, which must be preserved in this
    // scenario so subsequent code keeps functioning.
    PRESERVE_SIDEEFFECTS = 1 << 0,
    // Traverse through function calls, attempting to compute their concrete
    // value. Must not be used in function-parallel scenarios, where the called
    // function might be concurrently modified, leading to undefined behavior.
    TRAVERSE_CALLS = 1 << 1
  };

  // Flags indicating special requirements, for example whether we are just
  // evaluating (default), also going to replace the expression afterwards or
  // executing in a function-parallel scenario. See FlagValues.
  typedef uint32_t Flags;

  // Indicates no limit of maxDepth or maxLoopIterations.
  static const Index NO_LIMIT = 0;

protected:
  // Flags indicating special requirements. See FlagValues.
  Flags flags = FlagValues::DEFAULT;

  // Map remembering concrete local values set in the context of this flow.
  std::unordered_map<Index, Literals> localValues;
  // Map remembering concrete global values set in the context of this flow.
  std::unordered_map<Name, Literals> globalValues;

public:
  struct NonconstantException {
  }; // TODO: use a flow with a special name, as this is likely very slow

  ConstantExpressionRunner(Module* module,
                           Flags flags,
                           Index maxDepth,
                           Index maxLoopIterations)
    : ExpressionRunner<SubType>(module, maxDepth, maxLoopIterations),
      flags(flags) {}

  // Sets a known local value to use.
  void setLocalValue(Index index, Literals& values) {
    assert(values.isConcrete());
    localValues[index] = values;
  }

  // Sets a known global value to use.
  void setGlobalValue(Name name, Literals& values) {
    assert(values.isConcrete());
    globalValues[name] = values;
  }

  Flow visitLocalGet(LocalGet* curr) {
    NOTE_ENTER("LocalGet");
    NOTE_EVAL1(curr->index);
    // Check if a constant value has been set in the context of this runner.
    auto iter = localValues.find(curr->index);
    if (iter != localValues.end()) {
      return Flow(iter->second);
    }
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitLocalSet(LocalSet* curr) {
    NOTE_ENTER("LocalSet");
    NOTE_EVAL1(curr->index);
    if (!(flags & FlagValues::PRESERVE_SIDEEFFECTS)) {
      // If we are evaluating and not replacing the expression, remember the
      // constant value set, if any, and see if there is a value flowing through
      // a tee.
      auto setFlow = ExpressionRunner<SubType>::visit(curr->value);
      if (!setFlow.breaking()) {
        setLocalValue(curr->index, setFlow.values);
        if (curr->type.isConcrete()) {
          assert(curr->isTee());
          return setFlow;
        }
        return Flow();
      }
    }
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitGlobalGet(GlobalGet* curr) {
    NOTE_ENTER("GlobalGet");
    NOTE_NAME(curr->name);
    if (this->module != nullptr) {
      auto* global = this->module->getGlobal(curr->name);
      // Check if the global has an immutable value anyway
      if (!global->imported() && !global->mutable_) {
        return ExpressionRunner<SubType>::visit(global->init);
      }
    }
    // Check if a constant value has been set in the context of this runner.
    auto iter = globalValues.find(curr->name);
    if (iter != globalValues.end()) {
      return Flow(iter->second);
    }
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitGlobalSet(GlobalSet* curr) {
    NOTE_ENTER("GlobalSet");
    NOTE_NAME(curr->name);
    if (!(flags & FlagValues::PRESERVE_SIDEEFFECTS) &&
        this->module != nullptr) {
      // If we are evaluating and not replacing the expression, remember the
      // constant value set, if any, for subsequent gets.
      assert(this->module->getGlobal(curr->name)->mutable_);
      auto setFlow = ExpressionRunner<SubType>::visit(curr->value);
      if (!setFlow.breaking()) {
        setGlobalValue(curr->name, setFlow.values);
        return Flow();
      }
    }
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitCall(Call* curr) {
    NOTE_ENTER("Call");
    NOTE_NAME(curr->target);
    // Traverse into functions using the same mode, which we can also do
    // when replacing as long as the function does not have any side effects.
    // Might yield something useful for simple functions like `clamp`, sometimes
    // even if arguments are only partially constant or not constant at all.
    if ((flags & FlagValues::TRAVERSE_CALLS) != 0 && this->module != nullptr) {
      auto* func = this->module->getFunction(curr->target);
      if (!func->imported()) {
        if (func->sig.results.isConcrete()) {
          auto numOperands = curr->operands.size();
          assert(numOperands == func->getNumParams());
          auto prevLocalValues = localValues;
          localValues.clear();
          for (Index i = 0; i < numOperands; ++i) {
            auto argFlow = ExpressionRunner<SubType>::visit(curr->operands[i]);
            if (!argFlow.breaking()) {
              assert(argFlow.values.isConcrete());
              localValues[i] = argFlow.values;
            }
          }
          auto retFlow = ExpressionRunner<SubType>::visit(func->body);
          localValues = prevLocalValues;
          if (retFlow.breakTo == RETURN_FLOW) {
            return Flow(retFlow.values);
          } else if (!retFlow.breaking()) {
            return retFlow;
          }
        }
      }
    }
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitCallIndirect(CallIndirect* curr) {
    NOTE_ENTER("CallIndirect");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitCallRef(CallRef* curr) {
    NOTE_ENTER("CallRef");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitLoad(Load* curr) {
    NOTE_ENTER("Load");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitStore(Store* curr) {
    NOTE_ENTER("Store");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitMemorySize(MemorySize* curr) {
    NOTE_ENTER("MemorySize");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitMemoryGrow(MemoryGrow* curr) {
    NOTE_ENTER("MemoryGrow");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitMemoryInit(MemoryInit* curr) {
    NOTE_ENTER("MemoryInit");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitDataDrop(DataDrop* curr) {
    NOTE_ENTER("DataDrop");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitMemoryCopy(MemoryCopy* curr) {
    NOTE_ENTER("MemoryCopy");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitMemoryFill(MemoryFill* curr) {
    NOTE_ENTER("MemoryFill");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableGet(TableGet* curr) {
    NOTE_ENTER("TableGet");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableSet(TableSet* curr) {
    NOTE_ENTER("TableSet");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableSize(TableSize* curr) {
    NOTE_ENTER("TableSize");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableGrow(TableGrow* curr) {
    NOTE_ENTER("TableGrow");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableFill(TableFill* curr) {
    NOTE_ENTER("TableFill");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableCopy(TableCopy* curr) {
    NOTE_ENTER("TableCopy");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTableInit(TableInit* curr) {
    NOTE_ENTER("TableInit");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitElemDrop(ElemDrop* curr) {
    NOTE_ENTER("ElemDrop");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitAtomicRMW(AtomicRMW* curr) {
    NOTE_ENTER("AtomicRMW");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitAtomicCmpxchg(AtomicCmpxchg* curr) {
    NOTE_ENTER("AtomicCmpxchg");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitAtomicWait(AtomicWait* curr) {
    NOTE_ENTER("AtomicWait");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitAtomicNotify(AtomicNotify* curr) {
    NOTE_ENTER("AtomicNotify");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitSIMDLoad(SIMDLoad* curr) {
    NOTE_ENTER("SIMDLoad");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitSIMDLoadSplat(SIMDLoad* curr) {
    NOTE_ENTER("SIMDLoadSplat");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitSIMDLoadExtend(SIMDLoad* curr) {
    NOTE_ENTER("SIMDLoadExtend");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
    NOTE_ENTER("SIMDLoadStoreLane");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitPop(Pop* curr) {
    NOTE_ENTER("Pop");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitTry(Try* curr) {
    NOTE_ENTER("Try");
    return Flow(NONCONSTANT_FLOW);
  }
  Flow visitRethrow(Rethrow* curr) {
    NOTE_ENTER("Rethrow");
    return Flow(NONCONSTANT_FLOW);
  }

  void trap(const char* why) override { throw NonconstantException(); }

  void hostLimit(const char* why) override { throw NonconstantException(); }

  virtual void throwException(const WasmException& exn) override {
    throw NonconstantException();
  }
};

// Execute an initializer expression of a global, data or element segment.
// see: https://webassembly.org/docs/modules/#initializer-expression
template<typename GlobalManager>
class InitializerExpressionRunner
  : public ExpressionRunner<InitializerExpressionRunner<GlobalManager>> {
  GlobalManager& globals;

public:
  InitializerExpressionRunner(GlobalManager& globals, Index maxDepth)
    : ExpressionRunner<InitializerExpressionRunner<GlobalManager>>(nullptr,
                                                                   maxDepth),
      globals(globals) {}

  Flow visitGlobalGet(GlobalGet* curr) { return Flow(globals[curr->name]); }
};

//
// An instance of a WebAssembly module, which can execute it via AST
// interpretation.
//
// To embed this interpreter, you need to provide an ExternalInterface instance
// (see below) which provides the embedding-specific details, that is, how to
// connect to the embedding implementation.
//
// To call into the interpreter, use callExport.
//

template<typename GlobalManager, typename SubType> class ModuleInstanceBase {
public:
  //
  // You need to implement one of these to create a concrete interpreter. The
  // ExternalInterface provides embedding-specific functionality like calling
  // an imported function or accessing memory.
  //
  struct ExternalInterface {
    ExternalInterface(
      std::map<Name, std::shared_ptr<SubType>> linkedInstances = {}) {}
    virtual ~ExternalInterface() = default;
    virtual void init(Module& wasm, SubType& instance) {}
    virtual void importGlobals(GlobalManager& globals, Module& wasm) = 0;
    virtual Literals callImport(Function* import, LiteralList& arguments) = 0;
    virtual Literals callTable(Name tableName,
                               Index index,
                               Signature sig,
                               LiteralList& arguments,
                               Type result,
                               SubType& instance) = 0;
    virtual bool growMemory(Address oldSize, Address newSize) = 0;
    virtual void trap(const char* why) = 0;
    virtual void hostLimit(const char* why) = 0;
    virtual void throwException(const WasmException& exn) = 0;

    // the default impls for load and store switch on the sizes. you can either
    // customize load/store, or the sub-functions which they call
    virtual Literal load(Load* load, Address addr) {
      switch (load->type.getBasic()) {
        case Type::i32: {
          switch (load->bytes) {
            case 1:
              return load->signed_ ? Literal((int32_t)load8s(addr))
                                   : Literal((int32_t)load8u(addr));
            case 2:
              return load->signed_ ? Literal((int32_t)load16s(addr))
                                   : Literal((int32_t)load16u(addr));
            case 4:
              return Literal((int32_t)load32s(addr));
            default:
              WASM_UNREACHABLE("invalid size");
          }
          break;
        }
        case Type::i64: {
          switch (load->bytes) {
            case 1:
              return load->signed_ ? Literal((int64_t)load8s(addr))
                                   : Literal((int64_t)load8u(addr));
            case 2:
              return load->signed_ ? Literal((int64_t)load16s(addr))
                                   : Literal((int64_t)load16u(addr));
            case 4:
              return load->signed_ ? Literal((int64_t)load32s(addr))
                                   : Literal((int64_t)load32u(addr));
            case 8:
              return Literal((int64_t)load64s(addr));
            default:
              WASM_UNREACHABLE("invalid size");
          }
          break;
        }
        case Type::f32:
          return Literal(load32u(addr)).castToF32();
        case Type::f64:
          return Literal(load64u(addr)).castToF64();
        case Type::v128:
          return Literal(load128(addr).data());
        case Type::funcref:
        case Type::externref:
        case Type::anyref:
        case Type::eqref:
        case Type::i31ref:
        case Type::dataref:
        case Type::none:
        case Type::unreachable:
          WASM_UNREACHABLE("unexpected type");
      }
      WASM_UNREACHABLE("invalid type");
    }
    virtual void store(Store* store, Address addr, Literal value) {
      switch (store->valueType.getBasic()) {
        case Type::i32: {
          switch (store->bytes) {
            case 1:
              store8(addr, value.geti32());
              break;
            case 2:
              store16(addr, value.geti32());
              break;
            case 4:
              store32(addr, value.geti32());
              break;
            default:
              WASM_UNREACHABLE("invalid store size");
          }
          break;
        }
        case Type::i64: {
          switch (store->bytes) {
            case 1:
              store8(addr, value.geti64());
              break;
            case 2:
              store16(addr, value.geti64());
              break;
            case 4:
              store32(addr, value.geti64());
              break;
            case 8:
              store64(addr, value.geti64());
              break;
            default:
              WASM_UNREACHABLE("invalid store size");
          }
          break;
        }
        // write floats carefully, ensuring all bits reach memory
        case Type::f32:
          store32(addr, value.reinterpreti32());
          break;
        case Type::f64:
          store64(addr, value.reinterpreti64());
          break;
        case Type::v128:
          store128(addr, value.getv128());
          break;
        case Type::funcref:
        case Type::externref:
        case Type::anyref:
        case Type::eqref:
        case Type::i31ref:
        case Type::dataref:
        case Type::none:
        case Type::unreachable:
          WASM_UNREACHABLE("unexpected type");
      }
    }

    virtual int8_t load8s(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual uint8_t load8u(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual int16_t load16s(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual uint16_t load16u(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual int32_t load32s(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual uint32_t load32u(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual int64_t load64s(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual uint64_t load64u(Address addr) { WASM_UNREACHABLE("unimp"); }
    virtual std::array<uint8_t, 16> load128(Address addr) {
      WASM_UNREACHABLE("unimp");
    }

    virtual void store8(Address addr, int8_t value) {
      WASM_UNREACHABLE("unimp");
    }
    virtual void store16(Address addr, int16_t value) {
      WASM_UNREACHABLE("unimp");
    }
    virtual void store32(Address addr, int32_t value) {
      WASM_UNREACHABLE("unimp");
    }
    virtual void store64(Address addr, int64_t value) {
      WASM_UNREACHABLE("unimp");
    }
    virtual void store128(Address addr, const std::array<uint8_t, 16>&) {
      WASM_UNREACHABLE("unimp");
    }

    virtual void tableStore(Name tableName, Index addr, const Literal& entry) {
      WASM_UNREACHABLE("unimp");
    }
    virtual const Literal& tableLoad(Name tableName, Index addr) {
      WASM_UNREACHABLE("unimp");
    }
    virtual Index tableSize(Name tableName) { WASM_UNREACHABLE("unimp"); }
    virtual Index growTable(Name tableName, Index delta, Literal initialValue) {
      WASM_UNREACHABLE("unimp");
    }
  };

  SubType* self() { return static_cast<SubType*>(this); }

  Module& wasm;

  // Values of globals
  GlobalManager globals;

  // Multivalue ABI support (see push/pop).
  std::vector<Literals> multiValues;

  ModuleInstanceBase(
    Module& wasm,
    ExternalInterface* externalInterface,
    std::map<Name, std::shared_ptr<SubType>> linkedInstances_ = {})
    : wasm(wasm), externalInterface(externalInterface),
      linkedInstances(linkedInstances_) {
    // import globals from the outside
    externalInterface->importGlobals(globals, wasm);
    // prepare memory
    memorySize = wasm.memory.initial;
    // generate internal (non-imported) globals
    ModuleUtils::iterDefinedGlobals(wasm, [&](Global* global) {
      globals[global->name] =
        InitializerExpressionRunner<GlobalManager>(globals, maxDepth)
          .visit(global->init)
          .values;
    });

    // initialize the rest of the external interface
    externalInterface->init(wasm, *self());

    initializeTableContents();
    initializeMemoryContents();

    // run start, if present
    if (wasm.start.is()) {
      LiteralList arguments;
      callFunction(wasm.start, arguments);
    }
  }

  // call an exported function
  Literals callExport(Name name, const LiteralList& arguments) {
    Export* export_ = wasm.getExportOrNull(name);
    if (!export_) {
      externalInterface->trap("callExport not found");
    }
    return callFunction(export_->value, arguments);
  }

  Literals callExport(Name name) { return callExport(name, LiteralList()); }

  // get an exported global
  Literals getExport(Name name) {
    Export* export_ = wasm.getExportOrNull(name);
    if (!export_) {
      externalInterface->trap("getExport external not found");
    }
    Name internalName = export_->value;
    auto iter = globals.find(internalName);
    if (iter == globals.end()) {
      externalInterface->trap("getExport internal not found");
    }
    return iter->second;
  }

  std::string printFunctionStack() {
    std::string ret = "/== (binaryen interpreter stack trace)\n";
    for (int i = int(functionStack.size()) - 1; i >= 0; i--) {
      ret += std::string("|: ") + functionStack[i].str + "\n";
    }
    ret += std::string("\\==\n");
    return ret;
  }

private:
  // Keep a record of call depth, to guard against excessive recursion.
  size_t callDepth;

  // Function name stack. We maintain this explicitly to allow printing of
  // stack traces.
  std::vector<Name> functionStack;

  std::unordered_set<size_t> droppedDataSegments;
  std::unordered_set<Name> droppedElementSegments;

  void initializeTableContents() {
    Const offset;
    offset.value = Literal(uint32_t(0));
    offset.finalize();

    ModuleUtils::iterActiveElementSegments(wasm, [&](ElementSegment* segment) {
      Const size;
      size.value = Literal(uint32_t(segment->data.size()));
      size.finalize();

      TableInit init;
      init.segment = segment->name;
      init.table = segment->table;
      init.destOffset = segment->offset;
      init.srcOffset = &offset;
      init.size = &size;
      init.finalize();

      ElemDrop drop;
      drop.segment = segment->name;
      drop.finalize();

      Function dummyFunc;
      FunctionScope dummyScope(&dummyFunc, {});
      RuntimeExpressionRunner runner(*this, dummyScope, maxDepth);
      runner.visit(&init);
      runner.visit(&drop);
    });
  }

  void initializeMemoryContents() {
    Const offset;
    offset.value = Literal(uint32_t(0));
    offset.finalize();

    // apply active memory segments
    for (size_t i = 0, e = wasm.memory.segments.size(); i < e; ++i) {
      Memory::Segment& segment = wasm.memory.segments[i];
      if (segment.isPassive) {
        continue;
      }

      Const size;
      size.value = Literal(uint32_t(segment.data.size()));
      size.finalize();

      MemoryInit init;
      init.segment = i;
      init.dest = segment.offset;
      init.offset = &offset;
      init.size = &size;
      init.finalize();

      DataDrop drop;
      drop.segment = i;
      drop.finalize();

      // we don't actually have a function, but we need one in order to visit
      // the memory.init and data.drop instructions.
      Function dummyFunc;
      FunctionScope dummyScope(&dummyFunc, {});
      RuntimeExpressionRunner runner(*this, dummyScope, maxDepth);
      runner.visit(&init);
      runner.visit(&drop);
    }
  }

  class FunctionScope {
  public:
    std::vector<Literals> locals;
    Function* function;

    FunctionScope(Function* function, const LiteralList& arguments)
      : function(function) {
      if (function->sig.params.size() != arguments.size()) {
        std::cerr << "Function `" << function->name << "` expects "
                  << function->sig.params.size() << " parameters, got "
                  << arguments.size() << " arguments." << std::endl;
        WASM_UNREACHABLE("invalid param count");
      }
      locals.resize(function->getNumLocals());
      for (size_t i = 0; i < function->getNumLocals(); i++) {
        if (i < arguments.size()) {
          if (!Type::isSubType(arguments[i].type, function->sig.params[i])) {
            std::cerr << "Function `" << function->name << "` expects type "
                      << function->sig.params[i] << " for parameter " << i
                      << ", got " << arguments[i].type << "." << std::endl;
            WASM_UNREACHABLE("invalid param count");
          }
          locals[i] = {arguments[i]};
        } else {
          assert(function->isVar(i));
          locals[i] = Literal::makeZeros(function->getLocalType(i));
        }
      }
    }
  };

  // Executes expressions with concrete runtime info, the function and module at
  // runtime
  class RuntimeExpressionRunner
    : public ExpressionRunner<RuntimeExpressionRunner> {
    ModuleInstanceBase& instance;
    FunctionScope& scope;
    // Stack of <caught exception, caught catch's try label>
    SmallVector<std::pair<WasmException, Name>, 4> exceptionStack;

  protected:
    struct ImportInstanceInfo {
      ModuleInstanceBase* instance;
      Name name;
      ImportInstanceInfo(ModuleInstanceBase* instance, Name name)
        : instance(instance), name(name) {}
    };

    // Returns the instance that defines the memory used by this one.
    SubType* getMemoryInstance() {
      auto* inst = instance.self();
      while (inst->wasm.memory.imported()) {
        inst = inst->linkedInstances.at(inst->wasm.memory.module).get();
      }
      return inst;
    }

    ImportInstanceInfo getTableInstance(Name name) {
      Table* table = instance.wasm.getTable(name);
      if (table->imported()) {
        auto inst = instance.linkedInstances.at(table->module).get();
        Export* tableExport = inst->wasm.getExport(table->base);
        return ImportInstanceInfo(inst, tableExport->value);
      } else {
        return ImportInstanceInfo(&instance, name);
      }
    }

    // Returns a reference to the current value of a potentially imported global
    Literals& getGlobal(Name name) {
      auto* inst = instance.self();
      auto* global = inst->wasm.getGlobal(name);
      while (global->imported()) {
        inst = inst->linkedInstances.at(global->module).get();
        Export* globalExport = inst->wasm.getExport(global->base);
        global = inst->wasm.getGlobal(globalExport->value);
      }

      return inst->globals[global->name];
    }

  public:
    RuntimeExpressionRunner(ModuleInstanceBase& instance,
                            FunctionScope& scope,
                            Index maxDepth)
      : ExpressionRunner<RuntimeExpressionRunner>(&instance.wasm, maxDepth),
        instance(instance), scope(scope) {}

    Flow visitCall(Call* curr) {
      NOTE_ENTER("Call");
      NOTE_NAME(curr->target);
      LiteralList arguments;
      Flow flow = this->generateArguments(curr->operands, arguments);
      if (flow.breaking()) {
        return flow;
      }
      auto* func = instance.wasm.getFunction(curr->target);
      Flow ret;
      if (func->imported()) {
        ret.values = instance.externalInterface->callImport(func, arguments);
      } else {
        ret.values = instance.callFunctionInternal(curr->target, arguments);
      }
#ifdef WASM_INTERPRETER_DEBUG
      std::cout << "(returned to " << scope.function->name << ")\n";
#endif
      // TODO: make this a proper tail call (return first)
      if (curr->isReturn) {
        ret.breakTo = RETURN_FLOW;
      }
      return ret;
    }
    Flow visitCallIndirect(CallIndirect* curr) {
      NOTE_ENTER("CallIndirect");
      LiteralList arguments;
      Flow flow = this->generateArguments(curr->operands, arguments);
      if (flow.breaking()) {
        return flow;
      }
      Flow target = this->visit(curr->target);
      if (target.breaking()) {
        return target;
      }

      Index index = target.getSingleValue().geti32();
      Type type = curr->isReturn ? scope.function->sig.results : curr->type;

      Flow ret;
      auto* table = instance.wasm.getTable(curr->table);
      if (table->imported()) {
        auto inst = instance.linkedInstances.at(table->module);
        Export* tableExport = inst->wasm.getExport(table->base);
        ret = inst->externalInterface->callTable(tableExport->value,
                                                 index,
                                                 curr->sig,
                                                 arguments,
                                                 type,
                                                 *instance.self());
      } else {
        ret = instance.externalInterface->callTable(
          curr->table, index, curr->sig, arguments, type, *instance.self());
      }

      // TODO: make this a proper tail call (return first)
      if (curr->isReturn) {
        ret.breakTo = RETURN_FLOW;
      }
      return ret;
    }
    Flow visitCallRef(CallRef* curr) {
      NOTE_ENTER("CallRef");
      LiteralList arguments;
      Flow flow = this->generateArguments(curr->operands, arguments);
      if (flow.breaking()) {
        return flow;
      }
      Flow target = this->visit(curr->target);
      if (target.breaking()) {
        return target;
      }
      if (target.getSingleValue().isNull()) {
        trap("null target in call_ref");
      }
      Name funcName = target.getSingleValue().getFunc();
      auto* func = instance.wasm.getFunction(funcName);
      Flow ret;
      if (func->imported()) {
        ret.values = instance.externalInterface->callImport(func, arguments);
      } else {
        ret.values = instance.callFunctionInternal(funcName, arguments);
      }
#ifdef WASM_INTERPRETER_DEBUG
      std::cout << "(returned to " << scope.function->name << ")\n";
#endif
      // TODO: make this a proper tail call (return first)
      if (curr->isReturn) {
        ret.breakTo = RETURN_FLOW;
      }
      return ret;
    }

    Flow visitLocalGet(LocalGet* curr) {
      NOTE_ENTER("LocalGet");
      auto index = curr->index;
      NOTE_EVAL1(index);
      NOTE_EVAL1(scope.locals[index]);
      return scope.locals[index];
    }
    Flow visitLocalSet(LocalSet* curr) {
      NOTE_ENTER("LocalSet");
      auto index = curr->index;
      Flow flow = this->visit(curr->value);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(index);
      NOTE_EVAL1(flow.getSingleValue());
      assert(curr->isTee() ? Type::isSubType(flow.getType(), curr->type)
                           : true);
      scope.locals[index] = flow.values;
      return curr->isTee() ? flow : Flow();
    }

    Flow visitGlobalGet(GlobalGet* curr) {
      NOTE_ENTER("GlobalGet");
      auto name = curr->name;
      NOTE_EVAL1(name);
      return getGlobal(name);
    }
    Flow visitGlobalSet(GlobalSet* curr) {
      NOTE_ENTER("GlobalSet");
      auto name = curr->name;
      Flow flow = this->visit(curr->value);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(name);
      NOTE_EVAL1(flow.getSingleValue());

      getGlobal(name) = flow.values;
      return Flow();
    }

    Flow visitLoad(Load* curr) {
      NOTE_ENTER("Load");
      Flow flow = this->visit(curr->ptr);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(flow);
      auto* inst = getMemoryInstance();
      auto addr = inst->getFinalAddress(curr, flow.getSingleValue());
      if (curr->isAtomic) {
        inst->checkAtomicAddress(addr, curr->bytes);
      }
      auto ret = inst->externalInterface->load(curr, addr);
      NOTE_EVAL1(addr);
      NOTE_EVAL1(ret);
      return ret;
    }
    Flow visitStore(Store* curr) {
      NOTE_ENTER("Store");
      Flow ptr = this->visit(curr->ptr);
      if (ptr.breaking()) {
        return ptr;
      }
      Flow value = this->visit(curr->value);
      if (value.breaking()) {
        return value;
      }
      auto* inst = getMemoryInstance();
      auto addr = inst->getFinalAddress(curr, ptr.getSingleValue());
      if (curr->isAtomic) {
        inst->checkAtomicAddress(addr, curr->bytes);
      }
      NOTE_EVAL1(addr);
      NOTE_EVAL1(value);
      inst->externalInterface->store(curr, addr, value.getSingleValue());
      return Flow();
    }

    Flow visitAtomicRMW(AtomicRMW* curr) {
      NOTE_ENTER("AtomicRMW");
      Flow ptr = this->visit(curr->ptr);
      if (ptr.breaking()) {
        return ptr;
      }
      auto value = this->visit(curr->value);
      if (value.breaking()) {
        return value;
      }
      NOTE_EVAL1(ptr);
      auto* inst = getMemoryInstance();
      auto addr = inst->getFinalAddress(curr, ptr.getSingleValue());
      NOTE_EVAL1(addr);
      NOTE_EVAL1(value);
      auto loaded = inst->doAtomicLoad(addr, curr->bytes, curr->type);
      NOTE_EVAL1(loaded);
      auto computed = value.getSingleValue();
      switch (curr->op) {
        case RMWAdd:
          computed = loaded.add(computed);
          break;
        case RMWSub:
          computed = loaded.sub(computed);
          break;
        case RMWAnd:
          computed = loaded.and_(computed);
          break;
        case RMWOr:
          computed = loaded.or_(computed);
          break;
        case RMWXor:
          computed = loaded.xor_(computed);
          break;
        case RMWXchg:
          break;
      }
      inst->doAtomicStore(addr, curr->bytes, computed);
      return loaded;
    }
    Flow visitAtomicCmpxchg(AtomicCmpxchg* curr) {
      NOTE_ENTER("AtomicCmpxchg");
      Flow ptr = this->visit(curr->ptr);
      if (ptr.breaking()) {
        return ptr;
      }
      NOTE_EVAL1(ptr);
      auto expected = this->visit(curr->expected);
      if (expected.breaking()) {
        return expected;
      }
      auto replacement = this->visit(curr->replacement);
      if (replacement.breaking()) {
        return replacement;
      }
      auto* inst = getMemoryInstance();
      auto addr = inst->getFinalAddress(curr, ptr.getSingleValue());
      expected =
        Flow(wrapToSmallerSize(expected.getSingleValue(), curr->bytes));
      NOTE_EVAL1(addr);
      NOTE_EVAL1(expected);
      NOTE_EVAL1(replacement);
      auto loaded = inst->doAtomicLoad(addr, curr->bytes, curr->type);
      NOTE_EVAL1(loaded);
      if (loaded == expected.getSingleValue()) {
        inst->doAtomicStore(addr, curr->bytes, replacement.getSingleValue());
      }
      return loaded;
    }
    Flow visitAtomicWait(AtomicWait* curr) {
      NOTE_ENTER("AtomicWait");
      Flow ptr = this->visit(curr->ptr);
      if (ptr.breaking()) {
        return ptr;
      }
      NOTE_EVAL1(ptr);
      auto expected = this->visit(curr->expected);
      NOTE_EVAL1(expected);
      if (expected.breaking()) {
        return expected;
      }
      auto timeout = this->visit(curr->timeout);
      NOTE_EVAL1(timeout);
      if (timeout.breaking()) {
        return timeout;
      }
      auto* inst = getMemoryInstance();
      auto bytes = curr->expectedType.getByteSize();
      auto addr = inst->getFinalAddress(curr, ptr.getSingleValue(), bytes);
      auto loaded = inst->doAtomicLoad(addr, bytes, curr->expectedType);
      NOTE_EVAL1(loaded);
      if (loaded != expected.getSingleValue()) {
        return Literal(int32_t(1)); // not equal
      }
      // TODO: add threads support!
      //       for now, just assume we are woken up
      return Literal(int32_t(0)); // woken up
    }
    Flow visitAtomicNotify(AtomicNotify* curr) {
      NOTE_ENTER("AtomicNotify");
      Flow ptr = this->visit(curr->ptr);
      if (ptr.breaking()) {
        return ptr;
      }
      NOTE_EVAL1(ptr);
      auto count = this->visit(curr->notifyCount);
      NOTE_EVAL1(count);
      if (count.breaking()) {
        return count;
      }
      auto* inst = getMemoryInstance();
      auto addr = inst->getFinalAddress(curr, ptr.getSingleValue(), 4);
      // Just check TODO actual threads support
      inst->checkAtomicAddress(addr, 4);
      return Literal(int32_t(0)); // none woken up
    }
    Flow visitSIMDLoad(SIMDLoad* curr) {
      NOTE_ENTER("SIMDLoad");
      switch (curr->op) {
        case Load8SplatVec128:
        case Load16SplatVec128:
        case Load32SplatVec128:
        case Load64SplatVec128:
          return visitSIMDLoadSplat(curr);
        case Load8x8SVec128:
        case Load8x8UVec128:
        case Load16x4SVec128:
        case Load16x4UVec128:
        case Load32x2SVec128:
        case Load32x2UVec128:
          return visitSIMDLoadExtend(curr);
        case Load32ZeroVec128:
        case Load64ZeroVec128:
          return visitSIMDLoadZero(curr);
      }
      WASM_UNREACHABLE("invalid op");
    }
    Flow visitSIMDLoadSplat(SIMDLoad* curr) {
      Load load;
      load.type = Type::i32;
      load.bytes = curr->getMemBytes();
      load.signed_ = false;
      load.offset = curr->offset;
      load.align = curr->align;
      load.isAtomic = false;
      load.ptr = curr->ptr;
      Literal (Literal::*splat)() const = nullptr;
      switch (curr->op) {
        case Load8SplatVec128:
          splat = &Literal::splatI8x16;
          break;
        case Load16SplatVec128:
          splat = &Literal::splatI16x8;
          break;
        case Load32SplatVec128:
          splat = &Literal::splatI32x4;
          break;
        case Load64SplatVec128:
          load.type = Type::i64;
          splat = &Literal::splatI64x2;
          break;
        default:
          WASM_UNREACHABLE("invalid op");
      }
      load.finalize();
      Flow flow = this->visit(&load);
      if (flow.breaking()) {
        return flow;
      }
      return (flow.getSingleValue().*splat)();
    }
    Flow visitSIMDLoadExtend(SIMDLoad* curr) {
      Flow flow = this->visit(curr->ptr);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(flow);
      Address src(uint32_t(flow.getSingleValue().geti32()));
      auto* inst = getMemoryInstance();
      auto loadLane = [&](Address addr) {
        switch (curr->op) {
          case Load8x8SVec128:
            return Literal(int32_t(inst->externalInterface->load8s(addr)));
          case Load8x8UVec128:
            return Literal(int32_t(inst->externalInterface->load8u(addr)));
          case Load16x4SVec128:
            return Literal(int32_t(inst->externalInterface->load16s(addr)));
          case Load16x4UVec128:
            return Literal(int32_t(inst->externalInterface->load16u(addr)));
          case Load32x2SVec128:
            return Literal(int64_t(inst->externalInterface->load32s(addr)));
          case Load32x2UVec128:
            return Literal(int64_t(inst->externalInterface->load32u(addr)));
          default:
            WASM_UNREACHABLE("unexpected op");
        }
        WASM_UNREACHABLE("invalid op");
      };
      auto fillLanes = [&](auto lanes, size_t laneBytes) {
        for (auto& lane : lanes) {
          lane = loadLane(
            inst->getFinalAddress(curr, Literal(uint32_t(src)), laneBytes));
          src = Address(uint32_t(src) + laneBytes);
        }
        return Literal(lanes);
      };
      switch (curr->op) {
        case Load8x8SVec128:
        case Load8x8UVec128: {
          std::array<Literal, 8> lanes;
          return fillLanes(lanes, 1);
        }
        case Load16x4SVec128:
        case Load16x4UVec128: {
          std::array<Literal, 4> lanes;
          return fillLanes(lanes, 2);
        }
        case Load32x2SVec128:
        case Load32x2UVec128: {
          std::array<Literal, 2> lanes;
          return fillLanes(lanes, 4);
        }
        default:
          WASM_UNREACHABLE("unexpected op");
      }
      WASM_UNREACHABLE("invalid op");
    }
    Flow visitSIMDLoadZero(SIMDLoad* curr) {
      Flow flow = this->visit(curr->ptr);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(flow);
      auto* inst = getMemoryInstance();
      Address src =
        inst->getFinalAddress(curr, flow.getSingleValue(), curr->getMemBytes());
      auto zero =
        Literal::makeZero(curr->op == Load32ZeroVec128 ? Type::i32 : Type::i64);
      if (curr->op == Load32ZeroVec128) {
        auto val = Literal(inst->externalInterface->load32u(src));
        return Literal(std::array<Literal, 4>{{val, zero, zero, zero}});
      } else {
        auto val = Literal(inst->externalInterface->load64u(src));
        return Literal(std::array<Literal, 2>{{val, zero}});
      }
    }
    Flow visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) {
      NOTE_ENTER("SIMDLoadStoreLane");
      Flow flow = this->visit(curr->ptr);
      if (flow.breaking()) {
        return flow;
      }
      NOTE_EVAL1(flow);
      auto* inst = getMemoryInstance();
      Address addr =
        inst->getFinalAddress(curr, flow.getSingleValue(), curr->getMemBytes());
      flow = this->visit(curr->vec);
      if (flow.breaking()) {
        return flow;
      }
      Literal vec = flow.getSingleValue();
      switch (curr->op) {
        case Load8LaneVec128:
        case Store8LaneVec128: {
          std::array<Literal, 16> lanes = vec.getLanesUI8x16();
          if (curr->isLoad()) {
            lanes[curr->index] = Literal(inst->externalInterface->load8u(addr));
            return Literal(lanes);
          } else {
            inst->externalInterface->store8(addr, lanes[curr->index].geti32());
            return {};
          }
        }
        case Load16LaneVec128:
        case Store16LaneVec128: {
          std::array<Literal, 8> lanes = vec.getLanesUI16x8();
          if (curr->isLoad()) {
            lanes[curr->index] =
              Literal(inst->externalInterface->load16u(addr));
            return Literal(lanes);
          } else {
            inst->externalInterface->store16(addr, lanes[curr->index].geti32());
            return {};
          }
        }
        case Load32LaneVec128:
        case Store32LaneVec128: {
          std::array<Literal, 4> lanes = vec.getLanesI32x4();
          if (curr->isLoad()) {
            lanes[curr->index] =
              Literal(inst->externalInterface->load32u(addr));
            return Literal(lanes);
          } else {
            inst->externalInterface->store32(addr, lanes[curr->index].geti32());
            return {};
          }
        }
        case Store64LaneVec128:
        case Load64LaneVec128: {
          std::array<Literal, 2> lanes = vec.getLanesI64x2();
          if (curr->isLoad()) {
            lanes[curr->index] =
              Literal(inst->externalInterface->load64u(addr));
            return Literal(lanes);
          } else {
            inst->externalInterface->store64(addr, lanes[curr->index].geti64());
            return {};
          }
        }
      }
      WASM_UNREACHABLE("unexpected op");
    }
    Flow visitMemorySize(MemorySize* curr) {
      NOTE_ENTER("MemorySize");
      auto* inst = getMemoryInstance();
      return Literal::makeFromInt64(inst->memorySize,
                                    inst->wasm.memory.indexType);
    }
    Flow visitMemoryGrow(MemoryGrow* curr) {
      NOTE_ENTER("MemoryGrow");
      auto* inst = getMemoryInstance();
      auto indexType = inst->wasm.memory.indexType;
      auto fail = Literal::makeFromInt64(-1, indexType);
      Flow flow = this->visit(curr->delta);
      if (flow.breaking()) {
        return flow;
      }
      Flow ret = Literal::makeFromInt64(inst->memorySize, indexType);
      uint64_t delta = flow.getSingleValue().getUnsigned();
      if (delta > uint32_t(-1) / Memory::kPageSize && indexType == Type::i32) {
        return fail;
      }
      if (inst->memorySize >= uint32_t(-1) - delta && indexType == Type::i32) {
        return fail;
      }
      auto newSize = inst->memorySize + delta;
      if (newSize > inst->wasm.memory.max) {
        return fail;
      }
      if (!inst->externalInterface->growMemory(inst->memorySize *
                                                 Memory::kPageSize,
                                               newSize * Memory::kPageSize)) {
        // We failed to grow the memory in practice, even though it was valid
        // to try to do so.
        return fail;
      }
      inst->memorySize = newSize;
      return ret;
    }
    Flow visitMemoryInit(MemoryInit* curr) {
      NOTE_ENTER("MemoryInit");
      Flow dest = this->visit(curr->dest);
      if (dest.breaking()) {
        return dest;
      }
      Flow offset = this->visit(curr->offset);
      if (offset.breaking()) {
        return offset;
      }
      Flow size = this->visit(curr->size);
      if (size.breaking()) {
        return size;
      }
      NOTE_EVAL1(dest);
      NOTE_EVAL1(offset);
      NOTE_EVAL1(size);

      assert(curr->segment < instance.wasm.memory.segments.size());
      Memory::Segment& segment = instance.wasm.memory.segments[curr->segment];

      Address destVal(dest.getSingleValue().getUnsigned());
      Address offsetVal(uint32_t(offset.getSingleValue().geti32()));
      Address sizeVal(uint32_t(size.getSingleValue().geti32()));

      if (offsetVal + sizeVal > 0 &&
          instance.droppedDataSegments.count(curr->segment)) {
        trap("out of bounds segment access in memory.init");
      }
      if ((uint64_t)offsetVal + sizeVal > segment.data.size()) {
        trap("out of bounds segment access in memory.init");
      }
      auto* inst = getMemoryInstance();
      if (destVal + sizeVal > inst->memorySize * Memory::kPageSize) {
        trap("out of bounds memory access in memory.init");
      }
      for (size_t i = 0; i < sizeVal; ++i) {
        Literal addr(destVal + i);
        inst->externalInterface->store8(
          inst->getFinalAddressWithoutOffset(addr, 1),
          segment.data[offsetVal + i]);
      }
      return {};
    }
    Flow visitDataDrop(DataDrop* curr) {
      NOTE_ENTER("DataDrop");
      instance.droppedDataSegments.insert(curr->segment);
      return {};
    }
    Flow visitMemoryCopy(MemoryCopy* curr) {
      NOTE_ENTER("MemoryCopy");
      Flow dest = this->visit(curr->dest);
      if (dest.breaking()) {
        return dest;
      }
      Flow source = this->visit(curr->source);
      if (source.breaking()) {
        return source;
      }
      Flow size = this->visit(curr->size);
      if (size.breaking()) {
        return size;
      }
      NOTE_EVAL1(dest);
      NOTE_EVAL1(source);
      NOTE_EVAL1(size);
      Address destVal(dest.getSingleValue().getUnsigned());
      Address sourceVal(source.getSingleValue().getUnsigned());
      Address sizeVal(size.getSingleValue().getUnsigned());

      auto* inst = getMemoryInstance();
      if (sourceVal + sizeVal > inst->memorySize * Memory::kPageSize ||
          destVal + sizeVal > inst->memorySize * Memory::kPageSize ||
          // FIXME: better/cheaper way to detect wrapping?
          sourceVal + sizeVal < sourceVal || sourceVal + sizeVal < sizeVal ||
          destVal + sizeVal < destVal || destVal + sizeVal < sizeVal) {
        trap("out of bounds segment access in memory.copy");
      }

      int64_t start = 0;
      int64_t end = sizeVal;
      int step = 1;
      // Reverse direction if source is below dest
      if (sourceVal < destVal) {
        start = int64_t(sizeVal) - 1;
        end = -1;
        step = -1;
      }
      for (int64_t i = start; i != end; i += step) {
        inst->externalInterface->store8(
          inst->getFinalAddressWithoutOffset(Literal(destVal + i), 1),
          inst->externalInterface->load8s(
            inst->getFinalAddressWithoutOffset(Literal(sourceVal + i), 1)));
      }
      return {};
    }
    Flow visitMemoryFill(MemoryFill* curr) {
      NOTE_ENTER("MemoryFill");
      Flow dest = this->visit(curr->dest);
      if (dest.breaking()) {
        return dest;
      }
      Flow value = this->visit(curr->value);
      if (value.breaking()) {
        return value;
      }
      Flow size = this->visit(curr->size);
      if (size.breaking()) {
        return size;
      }
      NOTE_EVAL1(dest);
      NOTE_EVAL1(value);
      NOTE_EVAL1(size);
      Address destVal(dest.getSingleValue().getUnsigned());
      Address sizeVal(size.getSingleValue().getUnsigned());

      auto* inst = getMemoryInstance();
      // FIXME: cheaper wrapping detection?
      if (destVal > inst->memorySize * Memory::kPageSize ||
          sizeVal > inst->memorySize * Memory::kPageSize ||
          destVal + sizeVal > inst->memorySize * Memory::kPageSize) {
        trap("out of bounds memory access in memory.fill");
      }
      uint8_t val(value.getSingleValue().geti32());
      for (size_t i = 0; i < sizeVal; ++i) {
        inst->externalInterface->store8(
          inst->getFinalAddressWithoutOffset(Literal(destVal + i), 1), val);
      }
      return {};
    }
    Flow visitTableGet(TableGet* curr) {
      NOTE_ENTER("TableGet");
      Flow offset = this->visit(curr->offset);
      if (offset.breaking()) {
        return offset;
      }
      auto importInstanceInfo = getTableInstance(curr->table);
      return importInstanceInfo.instance->externalInterface->tableLoad(
        importInstanceInfo.name, offset.getSingleValue().geti32());
    }
    Flow visitTableSet(TableSet* curr) {
      NOTE_ENTER("TableSet");
      Flow offset = this->visit(curr->offset);
      if (offset.breaking()) {
        return offset;
      }
      Flow value = this->visit(curr->value);
      if (value.breaking()) {
        return value;
      }
      auto importInstanceInfo = getTableInstance(curr->table);
      importInstanceInfo.instance->externalInterface->tableStore(
        importInstanceInfo.name,
        offset.getSingleValue().geti32(),
        value.getSingleValue());
      return {};
    }
    Flow visitTableSize(TableSize* curr) {
      NOTE_ENTER("TableSize");
      auto importInstanceInfo = getTableInstance(curr->table);
      auto tableSize =
        importInstanceInfo.instance->externalInterface->tableSize(
          importInstanceInfo.name);
      return Literal::makeFromInt32(tableSize, Type::i32);
    }
    Flow visitTableGrow(TableGrow* curr) {
      NOTE_ENTER("TableGrow");
      Flow delta = this->visit(curr->delta);
      if (delta.breaking()) {
        return delta;
      }
      Flow initialValue = this->visit(curr->initialValue);
      if (initialValue.breaking()) {
        return initialValue;
      }
      auto deltaVal = delta.getSingleValue().geti32();

      auto importInstanceInfo = getTableInstance(curr->table);
      auto* table =
        importInstanceInfo.instance->wasm.getTable(importInstanceInfo.name);
      auto tableSize =
        importInstanceInfo.instance->externalInterface->tableSize(
          importInstanceInfo.name);
      if (tableSize + deltaVal > table->max) {
        return Flow(Literal::makeFromInt32(-1, Type::i32));
      }
      auto ret = importInstanceInfo.instance->externalInterface->growTable(
        importInstanceInfo.name, deltaVal, initialValue.getSingleValue());
      return Flow(Literal::makeFromInt32(ret, Type::i32));
    }
    Flow visitTableFill(TableFill* curr) {
      NOTE_ENTER("TableFill");
      Flow dest = this->visit(curr->dest);
      if (dest.breaking()) {
        return dest;
      }
      Flow value = this->visit(curr->value);
      if (value.breaking()) {
        return value;
      }
      Flow size = this->visit(curr->size);
      if (size.breaking()) {
        return size;
      }
      NOTE_EVAL1(dest);
      NOTE_EVAL1(value);
      NOTE_EVAL1(size);
      Index destVal(dest.getSingleValue().getUnsigned());
      Index sizeVal(size.getSingleValue().getUnsigned());

      auto importInstanceInfo = getTableInstance(curr->table);
      auto tableSize =
        importInstanceInfo.instance->externalInterface->tableSize(
          importInstanceInfo.name);
      if (destVal > tableSize * Table::kPageSize ||
          sizeVal > tableSize * Table::kPageSize ||
          destVal + sizeVal > tableSize * Table::kPageSize) {
        trap("out of bounds table access in table.fill");
      }
      for (size_t i = 0; i < sizeVal; ++i) {
        importInstanceInfo.instance->externalInterface->tableStore(
          importInstanceInfo.name, destVal + i, value.getSingleValue());
      }
      return {};
    }
    Flow visitTableCopy(TableCopy* curr) {
      NOTE_ENTER("TableCopy");
      Flow destOffset = this->visit(curr->destOffset);
      if (destOffset.breaking()) {
        return destOffset;
      }
      Flow srcOffset = this->visit(curr->srcOffset);
      if (srcOffset.breaking()) {
        return srcOffset;
      }
      Flow size = this->visit(curr->size);
      if (size.breaking()) {
        return size;
      }
      NOTE_EVAL1(destOffset);
      NOTE_EVAL1(srcOffset);
      NOTE_EVAL1(size);

      Index destOffsetVal(destOffset.getSingleValue().geti32());
      Index srcOffsetVal(uint32_t(srcOffset.getSingleValue().geti32()));
      Index sizeVal(uint32_t(size.getSingleValue().geti32()));

      auto srcInstanceInfo = getTableInstance(curr->srcTable);
      auto srcTableSize =
        srcInstanceInfo.instance->externalInterface->tableSize(
          srcInstanceInfo.name);
      if ((uint64_t)srcOffsetVal + sizeVal > srcTableSize * Table::kPageSize) {
        trap("out of bounds source table access in table.copy");
      }
      auto destInstanceInfo = getTableInstance(curr->destTable);
      auto destTableSize =
        destInstanceInfo.instance->externalInterface->tableSize(
          destInstanceInfo.name);
      if ((uint64_t)destOffsetVal + sizeVal >
          destTableSize * Table::kPageSize) {
        trap("out of bounds destination table access in table.copy");
      }

      size_t start = 0;
      size_t end = sizeVal;
      int step = 1;
      // Reverse direction if source is below dest
      if (srcOffsetVal < destOffsetVal) {
        start = size_t(sizeVal) - 1;
        end = -1;
        step = -1;
      }
      for (size_t i = start; i != end; i += step) {
        destInstanceInfo.instance->externalInterface->tableStore(
          destInstanceInfo.name,
          destOffsetVal + i,
          srcInstanceInfo.instance->externalInterface->tableLoad(
            srcInstanceInfo.name, srcOffsetVal + i));
      }

      return {};
    }
    Flow visitTableInit(TableInit* curr) {
      NOTE_ENTER("TableInit");
      Flow destOffset = this->visit(curr->destOffset);
      if (destOffset.breaking()) {
        return destOffset;
      }
      Flow srcOffset = this->visit(curr->srcOffset);
      if (srcOffset.breaking()) {
        return srcOffset;
      }
      Flow size = this->visit(curr->size);
      if (size.breaking()) {
        return size;
      }
      NOTE_EVAL1(destOffset);
      NOTE_EVAL1(srcOffset);
      NOTE_EVAL1(size);

      auto segment = instance.wasm.getElementSegment(curr->segment);

      Index destOffsetVal(destOffset.getSingleValue().geti32());
      Index srcOffsetVal(uint32_t(srcOffset.getSingleValue().geti32()));
      Index sizeVal(uint32_t(size.getSingleValue().geti32()));

      if (srcOffsetVal + sizeVal > 0 &&
          instance.droppedElementSegments.count(curr->segment)) {
        trap("out of bounds segment access in table.init");
      }
      if ((uint64_t)srcOffsetVal + sizeVal > segment->data.size()) {
        trap("out of bounds segment access in table.init");
      }
      auto importInstanceInfo = getTableInstance(curr->table);
      auto tableSize =
        importInstanceInfo.instance->externalInterface->tableSize(
          importInstanceInfo.name);
      if ((uint64_t)destOffsetVal + sizeVal > tableSize * Table::kPageSize) {
        trap("out of bounds table access in table.init");
      }
      for (size_t i = 0; i < sizeVal; ++i) {
        Flow item = this->visit(segment->data[srcOffsetVal + i]);
        importInstanceInfo.instance->externalInterface->tableStore(
          importInstanceInfo.name, destOffsetVal + i, item.getSingleValue());
      }
      return {};
    }
    Flow visitElemDrop(ElemDrop* curr) {
      NOTE_ENTER("ElemDrop");
      instance.droppedElementSegments.insert(curr->segment);
      return {};
    }
    Flow visitTry(Try* curr) {
      NOTE_ENTER("Try");
      try {
        return this->visit(curr->body);
      } catch (const WasmException& e) {
        auto processCatchBody = [&](Expression* catchBody) {
          // Push the current exception onto the exceptionStack in case
          // 'rethrow's use it
          exceptionStack.push_back(std::make_pair(e, curr->name));
          // We need to pop exceptionStack in either case: when the catch body
          // exits normally or when a new exception is thrown
          Flow ret;
          try {
            ret = this->visit(catchBody);
          } catch (const WasmException&) {
            exceptionStack.pop_back();
            throw;
          }
          exceptionStack.pop_back();
          return ret;
        };

        for (size_t i = 0; i < curr->catchEvents.size(); i++) {
          if (curr->catchEvents[i] == e.event) {
            instance.multiValues.push_back(e.values);
            return processCatchBody(curr->catchBodies[i]);
          }
        }
        if (curr->hasCatchAll()) {
          return processCatchBody(curr->catchBodies.back());
        }
        // This exception is not caught by this try-catch. Rethrow it.
        throw;
      }
    }
    Flow visitRethrow(Rethrow* curr) {
      for (int i = exceptionStack.size() - 1; i >= 0; i--) {
        if (exceptionStack[i].second == curr->target) {
          throwException(exceptionStack[i].first);
        }
      }
      WASM_UNREACHABLE("rethrow");
    }
    Flow visitPop(Pop* curr) {
      NOTE_ENTER("Pop");
      assert(!instance.multiValues.empty());
      auto ret = instance.multiValues.back();
      assert(curr->type == ret.getType());
      instance.multiValues.pop_back();
      return ret;
    }

    void trap(const char* why) override {
      instance.externalInterface->trap(why);
    }

    void hostLimit(const char* why) override {
      instance.externalInterface->hostLimit(why);
    }

    void throwException(const WasmException& exn) override {
      instance.externalInterface->throwException(exn);
    }

    // Given a value, wrap it to a smaller given number of bytes.
    Literal wrapToSmallerSize(Literal value, Index bytes) {
      if (value.type == Type::i32) {
        switch (bytes) {
          case 1: {
            return value.and_(Literal(uint32_t(0xff)));
          }
          case 2: {
            return value.and_(Literal(uint32_t(0xffff)));
          }
          case 4: {
            break;
          }
          default:
            WASM_UNREACHABLE("unexpected bytes");
        }
      } else {
        assert(value.type == Type::i64);
        switch (bytes) {
          case 1: {
            return value.and_(Literal(uint64_t(0xff)));
          }
          case 2: {
            return value.and_(Literal(uint64_t(0xffff)));
          }
          case 4: {
            return value.and_(Literal(uint64_t(0xffffffffUL)));
          }
          case 8: {
            break;
          }
          default:
            WASM_UNREACHABLE("unexpected bytes");
        }
      }
      return value;
    }
  };

public:
  // Call a function, starting an invocation.
  Literals callFunction(Name name, const LiteralList& arguments) {
    // if the last call ended in a jump up the stack, it might have left stuff
    // for us to clean up here
    callDepth = 0;
    functionStack.clear();
    return callFunctionInternal(name, arguments);
  }

  // Internal function call. Must be public so that callTable implementations
  // can use it (refactor?)
  Literals callFunctionInternal(Name name, const LiteralList& arguments) {
    if (callDepth > maxDepth) {
      externalInterface->trap("stack limit");
    }
    auto previousCallDepth = callDepth;
    callDepth++;
    auto previousFunctionStackSize = functionStack.size();
    functionStack.push_back(name);

    Function* function = wasm.getFunction(name);
    assert(function);
    FunctionScope scope(function, arguments);

#ifdef WASM_INTERPRETER_DEBUG
    std::cout << "entering " << function->name << "\n  with arguments:\n";
    for (unsigned i = 0; i < arguments.size(); ++i) {
      std::cout << "    $" << i << ": " << arguments[i] << '\n';
    }
#endif

    Flow flow =
      RuntimeExpressionRunner(*this, scope, maxDepth).visit(function->body);
    // cannot still be breaking, it means we missed our stop
    assert(!flow.breaking() || flow.breakTo == RETURN_FLOW);
    auto type = flow.getType();
    if (!Type::isSubType(type, function->sig.results)) {
      std::cerr << "calling " << function->name << " resulted in " << type
                << " but the function type is " << function->sig.results
                << '\n';
      WASM_UNREACHABLE("unexpected result type");
    }
    // may decrease more than one, if we jumped up the stack
    callDepth = previousCallDepth;
    // if we jumped up the stack, we also need to pop higher frames
    while (functionStack.size() > previousFunctionStackSize) {
      functionStack.pop_back();
    }
#ifdef WASM_INTERPRETER_DEBUG
    std::cout << "exiting " << function->name << " with " << flow.values
              << '\n';
#endif
    return flow.values;
  }

protected:
  Address memorySize; // in pages

  static const Index maxDepth = 250;

  void trapIfGt(uint64_t lhs, uint64_t rhs, const char* msg) {
    if (lhs > rhs) {
      std::stringstream ss;
      ss << msg << ": " << lhs << " > " << rhs;
      externalInterface->trap(ss.str().c_str());
    }
  }

  template<class LS>
  Address getFinalAddress(LS* curr, Literal ptr, Index bytes) {
    Address memorySizeBytes = memorySize * Memory::kPageSize;
    uint64_t addr = ptr.type == Type::i32 ? ptr.geti32() : ptr.geti64();
    trapIfGt(curr->offset, memorySizeBytes, "offset > memory");
    trapIfGt(addr, memorySizeBytes - curr->offset, "final > memory");
    addr += curr->offset;
    trapIfGt(bytes, memorySizeBytes, "bytes > memory");
    checkLoadAddress(addr, bytes);
    return addr;
  }

  template<class LS> Address getFinalAddress(LS* curr, Literal ptr) {
    return getFinalAddress(curr, ptr, curr->bytes);
  }

  Address getFinalAddressWithoutOffset(Literal ptr, Index bytes) {
    uint64_t addr = ptr.type == Type::i32 ? ptr.geti32() : ptr.geti64();
    checkLoadAddress(addr, bytes);
    return addr;
  }

  void checkLoadAddress(Address addr, Index bytes) {
    Address memorySizeBytes = memorySize * Memory::kPageSize;
    trapIfGt(addr, memorySizeBytes - bytes, "highest > memory");
  }

  void checkAtomicAddress(Address addr, Index bytes) {
    checkLoadAddress(addr, bytes);
    // Unaligned atomics trap.
    if (bytes > 1) {
      if (addr & (bytes - 1)) {
        externalInterface->trap("unaligned atomic operation");
      }
    }
  }

  Literal doAtomicLoad(Address addr, Index bytes, Type type) {
    checkAtomicAddress(addr, bytes);
    Const ptr;
    ptr.value = Literal(int32_t(addr));
    ptr.type = Type::i32;
    Load load;
    load.bytes = bytes;
    // When an atomic loads a partial number of bytes for the type, it is
    // always an unsigned extension.
    load.signed_ = false;
    load.align = bytes;
    load.isAtomic = true; // understatement
    load.ptr = &ptr;
    load.type = type;
    return externalInterface->load(&load, addr);
  }

  void doAtomicStore(Address addr, Index bytes, Literal toStore) {
    checkAtomicAddress(addr, bytes);
    Const ptr;
    ptr.value = Literal(int32_t(addr));
    ptr.type = Type::i32;
    Const value;
    value.value = toStore;
    value.type = toStore.type;
    Store store;
    store.bytes = bytes;
    store.align = bytes;
    store.isAtomic = true; // understatement
    store.ptr = &ptr;
    store.value = &value;
    store.valueType = value.type;
    return externalInterface->store(&store, addr, toStore);
  }

  ExternalInterface* externalInterface;
  std::map<Name, std::shared_ptr<SubType>> linkedInstances;
};

// The default ModuleInstance uses a trivial global manager
using TrivialGlobalManager = std::map<Name, Literals>;
class ModuleInstance
  : public ModuleInstanceBase<TrivialGlobalManager, ModuleInstance> {
public:
  ModuleInstance(
    Module& wasm,
    ExternalInterface* externalInterface,
    std::map<Name, std::shared_ptr<ModuleInstance>> linkedInstances = {})
    : ModuleInstanceBase(wasm, externalInterface, linkedInstances) {}
};

} // namespace wasm

#endif // wasm_wasm_interpreter_h
