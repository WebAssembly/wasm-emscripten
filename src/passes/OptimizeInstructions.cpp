/*
 * Copyright 2016 WebAssembly Community Group participants
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
// Optimize combinations of instructions
//

#include <algorithm>
#include <cmath>
#include <type_traits>

#include <ir/abstract.h>
#include <ir/bits.h>
#include <ir/boolean.h>
#include <ir/cost.h>
#include <ir/drop.h>
#include <ir/effects.h>
#include <ir/eh-utils.h>
#include <ir/find_all.h>
#include <ir/gc-type-utils.h>
#include <ir/iteration.h>
#include <ir/literal-utils.h>
#include <ir/load-utils.h>
#include <ir/localize.h>
#include <ir/manipulation.h>
#include <ir/match.h>
#include <ir/ordering.h>
#include <ir/properties.h>
#include <ir/type-updating.h>
#include <ir/utils.h>
#include <pass.h>
#include <support/stdckdint.h>
#include <support/threads.h>
#include <wasm.h>

#include "call-utils.h"

// TODO: Use the new sign-extension opcodes where appropriate. This needs to be
// conditionalized on the availability of atomics.

namespace wasm {

static Index getBitsForType(Type type) {
  if (!type.isNumber()) {
    return -1;
  }
  return type.getByteSize() * 8;
}

static bool isSignedOp(BinaryOp op) {
  switch (op) {
    case LtSInt32:
    case LeSInt32:
    case GtSInt32:
    case GeSInt32:
    case LtSInt64:
    case LeSInt64:
    case GtSInt64:
    case GeSInt64:
      return true;
    default:
      return false;
  }
}

// Useful information about locals
struct LocalInfo {
  static const Index kUnknown = Index(-1);

  Index maxBits = -1;
  Index signExtBits = 0;
};

struct LocalScanner : PostWalker<LocalScanner> {
  std::vector<LocalInfo>& localInfo;
  const PassOptions& passOptions;

  LocalScanner(std::vector<LocalInfo>& localInfo,
               const PassOptions& passOptions)
    : localInfo(localInfo), passOptions(passOptions) {}

  void doWalkFunction(Function* func) {
    // prepare
    localInfo.resize(func->getNumLocals());
    for (Index i = 0; i < func->getNumLocals(); i++) {
      auto& info = localInfo[i];
      if (func->isParam(i)) {
        info.maxBits = getBitsForType(func->getLocalType(i)); // worst-case
        info.signExtBits = LocalInfo::kUnknown; // we will never know anything
      } else {
        info.maxBits = info.signExtBits = 0; // we are open to learning
      }
    }
    // walk
    PostWalker<LocalScanner>::doWalkFunction(func);
    // finalize
    for (Index i = 0; i < func->getNumLocals(); i++) {
      auto& info = localInfo[i];
      if (info.signExtBits == LocalInfo::kUnknown) {
        info.signExtBits = 0;
      }
    }
  }

  void visitLocalSet(LocalSet* curr) {
    auto* func = getFunction();
    if (func->isParam(curr->index)) {
      return;
    }
    auto type = getFunction()->getLocalType(curr->index);
    if (type != Type::i32 && type != Type::i64) {
      return;
    }
    // an integer var, worth processing
    auto* value =
      Properties::getFallthrough(curr->value, passOptions, *getModule());
    auto& info = localInfo[curr->index];
    info.maxBits = std::max(info.maxBits, Bits::getMaxBits(value, this));
    auto signExtBits = LocalInfo::kUnknown;
    if (Properties::getSignExtValue(value)) {
      signExtBits = Properties::getSignExtBits(value);
    } else if (auto* load = value->dynCast<Load>()) {
      if (LoadUtils::isSignRelevant(load) && load->signed_) {
        signExtBits = load->bytes * 8;
      }
    }
    if (info.signExtBits == 0) {
      info.signExtBits = signExtBits; // first info we see
    } else if (info.signExtBits != signExtBits) {
      // contradictory information, give up
      info.signExtBits = LocalInfo::kUnknown;
    }
  }

  // define this for the templated getMaxBits method. we know nothing here yet
  // about locals, so return the maxes
  Index getMaxBitsForLocal(LocalGet* get) { return getBitsForType(get->type); }
};

namespace {
// perform some final optimizations
struct FinalOptimizer : public PostWalker<FinalOptimizer> {
  const PassOptions& passOptions;

  FinalOptimizer(const PassOptions& passOptions) : passOptions(passOptions) {}

  void visitBinary(Binary* curr) {
    if (auto* replacement = optimize(curr)) {
      replaceCurrent(replacement);
    }
  }

  Binary* optimize(Binary* curr) {
    using namespace Abstract;
    using namespace Match;
    {
      Const* c;
      if (matches(curr, binary(Add, any(), ival(&c)))) {
        // normalize x + (-C)  ==>   x - C
        if (c->value.isNegative()) {
          c->value = c->value.neg();
          curr->op = Abstract::getBinary(c->type, Sub);
        }
        // Wasm binary encoding uses signed LEBs, which slightly favor negative
        // numbers: -64 is more efficient than +64 etc., as well as other powers
        // of two 7 bits etc. higher. we therefore prefer x - -64 over x + 64.
        // in theory we could just prefer negative numbers over positive, but
        // that can have bad effects on gzip compression (as it would mean more
        // subtractions than the more common additions).
        int64_t value = c->value.getInteger();
        if (value == 0x40LL || value == 0x2000LL || value == 0x100000LL ||
            value == 0x8000000LL || value == 0x400000000LL ||
            value == 0x20000000000LL || value == 0x1000000000000LL ||
            value == 0x80000000000000LL || value == 0x4000000000000000LL) {
          c->value = c->value.neg();
          if (curr->op == Abstract::getBinary(c->type, Add)) {
            curr->op = Abstract::getBinary(c->type, Sub);
          } else {
            curr->op = Abstract::getBinary(c->type, Add);
          }
        }
        return curr;
      }
    }
    return nullptr;
  }
};

} // anonymous namespace

// Create a custom matcher for checking side effects
template<class Opt> struct PureMatcherKind {};
template<class Opt>
struct Match::Internal::KindTypeRegistry<PureMatcherKind<Opt>> {
  using matched_t = Expression*;
  using data_t = Opt*;
};
template<class Opt> struct Match::Internal::MatchSelf<PureMatcherKind<Opt>> {
  bool operator()(Expression* curr, Opt* opt) {
    return !opt->effects(curr).hasSideEffects();
  }
};

// Main pass class
struct OptimizeInstructions
  : public WalkerPass<PostWalker<OptimizeInstructions>> {
  bool isFunctionParallel() override { return true; }

  std::unique_ptr<Pass> create() override {
    return std::make_unique<OptimizeInstructions>();
  }

  bool fastMath;

  // In rare cases we make a change to a type, and will do a refinalize.
  bool refinalize = false;

  void doWalkFunction(Function* func) {
    fastMath = getPassOptions().fastMath;

    // First, scan locals.
    {
      LocalScanner scanner(localInfo, getPassOptions());
      scanner.setModule(getModule());
      scanner.walkFunction(func);
    }

    // Main walk.
    Super::doWalkFunction(func);

    if (refinalize) {
      ReFinalize().walkFunctionInModule(func, getModule());
    }

    // Final optimizations.
    {
      FinalOptimizer optimizer(getPassOptions());
      optimizer.walkFunction(func);
    }

    // Some patterns create blocks that can interfere 'catch' and 'pop', nesting
    // the 'pop' into a block making it invalid.
    EHUtils::handleBlockNestedPops(func, *getModule());
  }

  // Set to true when one of the visitors makes a change (either replacing the
  // node or modifying it).
  bool changed;

  // Used to avoid recursion in replaceCurrent, see below.
  bool inReplaceCurrent = false;

  void replaceCurrent(Expression* rep) {
    if (rep->type != getCurrent()->type) {
      // This operation will change the type, so refinalize.
      refinalize = true;
    }

    WalkerPass<PostWalker<OptimizeInstructions>>::replaceCurrent(rep);
    // We may be able to apply multiple patterns as one may open opportunities
    // for others. NB: patterns must not have cycles

    // To avoid recursion, this uses the following pattern: the initial call to
    // this method comes from one of the visit*() methods. We then loop in here,
    // and if we are called again we set |changed| instead of recursing, so that
    // we can loop on that value.
    if (inReplaceCurrent) {
      // We are in the loop below so just note a change and return to there.
      changed = true;
      return;
    }
    // Loop on further changes.
    inReplaceCurrent = true;
    do {
      changed = false;
      visit(getCurrent());
    } while (changed);
    inReplaceCurrent = false;
  }

  EffectAnalyzer effects(Expression* expr) {
    return EffectAnalyzer(getPassOptions(), *getModule(), expr);
  }

  decltype(auto) pure(Expression** binder) {
    using namespace Match::Internal;
    return Matcher<PureMatcherKind<OptimizeInstructions>>(binder, this);
  }

  bool canReorder(Expression* a, Expression* b) {
    return EffectAnalyzer::canReorder(getPassOptions(), *getModule(), a, b);
  }

  void visitBinary(Binary* curr) {
    // If this contains dead code, don't bother trying to optimize it, the type
    // might change (if might not be unreachable if just one arm is, for
    // example). This optimization pass focuses on actually executing code.
    if (curr->type == Type::unreachable) {
      return;
    }

    if (shouldCanonicalize(curr)) {
      canonicalize(curr);
    }

    {
      // TODO: It is an ongoing project to port more transformations to the
      // match API. Once most of the transformations have been ported, the
      // `using namespace Match` can be hoisted to function scope and this extra
      // block scope can be removed.
      using namespace Match;
      using namespace Abstract;
      Builder builder(*getModule());
      {
        // try to get rid of (0 - ..), that is, a zero only used to negate an
        // int. an add of a subtract can be flipped in order to remove it:
        //   (ival.add
        //     (ival.sub
        //       (ival.const 0)
        //       X
        //     )
        //     Y
        //   )
        // =>
        //   (ival.sub
        //     Y
        //     X
        //   )
        // Note that this reorders X and Y, so we need to be careful about that.
        Expression *x, *y;
        Binary* sub;
        if (matches(
              curr,
              binary(Add, binary(&sub, Sub, ival(0), any(&x)), any(&y))) &&
            canReorder(x, y)) {
          sub->left = y;
          sub->right = x;
          return replaceCurrent(sub);
        }
      }
      {
        // The flip case is even easier, as no reordering occurs:
        //   (ival.add
        //     Y
        //     (ival.sub
        //       (ival.const 0)
        //       X
        //     )
        //   )
        // =>
        //   (ival.sub
        //     Y
        //     X
        //   )
        Expression* y;
        Binary* sub;
        if (matches(curr,
                    binary(Add, any(&y), binary(&sub, Sub, ival(0), any())))) {
          sub->left = y;
          return replaceCurrent(sub);
        }
      }
      {
        // try de-morgan's AND law,
        //  (eqz X) and (eqz Y) === eqz (X or Y)
        // Note that the OR and XOR laws do not work here, as these
        // are not booleans (we could check if they are, but a boolean
        // would already optimize with the eqz anyhow, unless propagating).
        // But for AND, the left is true iff X and Y are each all zero bits,
        // and the right is true if the union of their bits is zero; same.
        Unary* un;
        Binary* bin;
        Expression *x, *y;
        if (matches(curr,
                    binary(&bin,
                           AndInt32,
                           unary(&un, EqZInt32, any(&x)),
                           unary(EqZInt32, any(&y))))) {
          bin->op = OrInt32;
          bin->left = x;
          bin->right = y;
          un->value = bin;
          return replaceCurrent(un);
        }
      }
      {
        // x <<>> (C & (31 | 63))   ==>   x <<>> C'
        // x <<>> (y & (31 | 63))   ==>   x <<>> y
        // x <<>> (y & (32 | 64))   ==>   x
        // where '<<>>':
        //   '<<', '>>', '>>>'. 'rotl' or 'rotr'
        BinaryOp op;
        Const* c;
        Expression *x, *y;

        // x <<>> C
        if (matches(curr, binary(&op, any(&x), ival(&c))) &&
            Abstract::hasAnyShift(op)) {
          // truncate RHS constant to effective size as:
          // i32(x) <<>> const(C & 31))
          // i64(x) <<>> const(C & 63))
          c->value = c->value.and_(
            Literal::makeFromInt32(c->type.getByteSize() * 8 - 1, c->type));
          // x <<>> 0   ==>   x
          if (c->value.isZero()) {
            return replaceCurrent(x);
          }
        }
        if (matches(curr,
                    binary(&op, any(&x), binary(And, any(&y), ival(&c)))) &&
            Abstract::hasAnyShift(op)) {
          // i32(x) <<>> (y & 31)   ==>   x <<>> y
          // i64(x) <<>> (y & 63)   ==>   x <<>> y
          if ((c->type == Type::i32 && (c->value.geti32() & 31) == 31) ||
              (c->type == Type::i64 && (c->value.geti64() & 63LL) == 63LL)) {
            curr->cast<Binary>()->right = y;
            return replaceCurrent(curr);
          }
          // i32(x) <<>> (y & C)   ==>   x,  where (C & 31) == 0
          // i64(x) <<>> (y & C)   ==>   x,  where (C & 63) == 0
          if (((c->type == Type::i32 && (c->value.geti32() & 31) == 0) ||
               (c->type == Type::i64 && (c->value.geti64() & 63LL) == 0LL)) &&
              !effects(y).hasSideEffects()) {
            return replaceCurrent(x);
          }
        }
      }
      {
        // -x + y   ==>   y - x
        //   where  x, y  are floating points
        Expression *x, *y;
        if (matches(curr, binary(Add, unary(Neg, any(&x)), any(&y))) &&
            canReorder(x, y)) {
          curr->op = Abstract::getBinary(curr->type, Sub);
          curr->left = x;
          std::swap(curr->left, curr->right);
          return replaceCurrent(curr);
        }
      }
      {
        // x + (-y)   ==>   x - y
        // x - (-y)   ==>   x + y
        //   where  x, y  are floating points
        Expression* y;
        if (matches(curr, binary(Add, any(), unary(Neg, any(&y)))) ||
            matches(curr, binary(Sub, any(), unary(Neg, any(&y))))) {
          curr->op = Abstract::getBinary(
            curr->type,
            curr->op == Abstract::getBinary(curr->type, Add) ? Sub : Add);
          curr->right = y;
          return replaceCurrent(curr);
        }
      }
      {
        // -x * -y   ==>   x * y
        //   where  x, y  are integers
        Binary* bin;
        Expression *x, *y;
        if (matches(curr,
                    binary(&bin,
                           Mul,
                           binary(Sub, ival(0), any(&x)),
                           binary(Sub, ival(0), any(&y))))) {
          bin->left = x;
          bin->right = y;
          return replaceCurrent(curr);
        }
      }
      {
        // -x * y   ==>   -(x * y)
        // x * -y   ==>   -(x * y)
        //   where  x, y  are integers
        Expression *x, *y;
        if ((matches(curr,
                     binary(Mul, binary(Sub, ival(0), any(&x)), any(&y))) ||
             matches(curr,
                     binary(Mul, any(&x), binary(Sub, ival(0), any(&y))))) &&
            !x->is<Const>() && !y->is<Const>()) {
          Builder builder(*getModule());
          return replaceCurrent(
            builder.makeBinary(Abstract::getBinary(curr->type, Sub),
                               builder.makeConst(Literal::makeZero(curr->type)),
                               builder.makeBinary(curr->op, x, y)));
        }
      }
      {
        if (getModule()->features.hasSignExt()) {
          Const *c1, *c2;
          Expression* x;
          // i64(x) << 56 >> 56   ==>   i64.extend8_s(x)
          // i64(x) << 48 >> 48   ==>   i64.extend16_s(x)
          // i64(x) << 32 >> 32   ==>   i64.extend32_s(x)
          if (matches(curr,
                      binary(ShrSInt64,
                             binary(ShlInt64, any(&x), i64(&c1)),
                             i64(&c2))) &&
              Bits::getEffectiveShifts(c1) == Bits::getEffectiveShifts(c2)) {
            switch (64 - Bits::getEffectiveShifts(c1)) {
              case 8:
                return replaceCurrent(builder.makeUnary(ExtendS8Int64, x));
              case 16:
                return replaceCurrent(builder.makeUnary(ExtendS16Int64, x));
              case 32:
                return replaceCurrent(builder.makeUnary(ExtendS32Int64, x));
              default:
                break;
            }
          }
          // i32(x) << 24 >> 24   ==>   i32.extend8_s(x)
          // i32(x) << 16 >> 16   ==>   i32.extend16_s(x)
          if (matches(curr,
                      binary(ShrSInt32,
                             binary(ShlInt32, any(&x), i32(&c1)),
                             i32(&c2))) &&
              Bits::getEffectiveShifts(c1) == Bits::getEffectiveShifts(c2)) {
            switch (32 - Bits::getEffectiveShifts(c1)) {
              case 8:
                return replaceCurrent(builder.makeUnary(ExtendS8Int32, x));
              case 16:
                return replaceCurrent(builder.makeUnary(ExtendS16Int32, x));
              default:
                break;
            }
          }
        }
      }
      {
        // unsigned(x) >= 0   =>   i32(1)
        // TODO: Use getDroppedChildrenAndAppend() here, so we can optimize even
        //       if pure.
        Const* c;
        Expression* x;
        if (matches(curr, binary(GeU, pure(&x), ival(&c))) &&
            c->value.isZero()) {
          c->value = Literal::makeOne(Type::i32);
          c->type = Type::i32;
          return replaceCurrent(c);
        }
        // unsigned(x) < 0   =>   i32(0)
        if (matches(curr, binary(LtU, pure(&x), ival(&c))) &&
            c->value.isZero()) {
          c->value = Literal::makeZero(Type::i32);
          c->type = Type::i32;
          return replaceCurrent(c);
        }
      }
    }
    if (auto* ext = Properties::getAlmostSignExt(curr)) {
      Index extraLeftShifts;
      auto bits = Properties::getAlmostSignExtBits(curr, extraLeftShifts);
      if (extraLeftShifts == 0) {
        if (auto* load =
              Properties::getFallthrough(ext, getPassOptions(), *getModule())
                ->dynCast<Load>()) {
          // pattern match a load of 8 bits and a sign extend using a shl of
          // 24 then shr_s of 24 as well, etc.
          if (LoadUtils::canBeSigned(load) &&
              ((load->bytes == 1 && bits == 8) ||
               (load->bytes == 2 && bits == 16))) {
            // if the value falls through, we can't alter the load, as it
            // might be captured in a tee
            if (load->signed_ == true || load == ext) {
              load->signed_ = true;
              return replaceCurrent(ext);
            }
          }
        }
      }
      // We can in some cases remove part of a sign extend, that is,
      //   (x << A) >> B   =>   x << (A - B)
      // If the sign-extend input cannot have a sign bit, we don't need it.
      if (Bits::getMaxBits(ext, this) + extraLeftShifts < bits) {
        return replaceCurrent(removeAlmostSignExt(curr));
      }
      // We also don't need it if it already has an identical-sized sign
      // extend applied to it. That is, if it is already a sign-extended
      // value, then another sign extend will do nothing. We do need to be
      // careful of the extra shifts, though.
      if (isSignExted(ext, bits) && extraLeftShifts == 0) {
        return replaceCurrent(removeAlmostSignExt(curr));
      }
    } else if (curr->op == EqInt32 || curr->op == NeInt32) {
      if (auto* c = curr->right->dynCast<Const>()) {
        if (auto* ext = Properties::getSignExtValue(curr->left)) {
          // We are comparing a sign extend to a constant, which means we can
          // use a cheaper zero-extend in some cases. That is,
          //  (x << S) >> S ==/!= C    =>    x & T ==/!= C
          // where S and T are the matching values for sign/zero extend of the
          // same size. For example, for an effective 8-bit value:
          //  (x << 24) >> 24 ==/!= C    =>    x & 255 ==/!= C
          //
          // The key thing to track here are the upper bits plus the sign bit;
          // call those the "relevant bits". This is crucial because x is
          // sign-extended, that is, its effective sign bit is spread to all
          // the upper bits, which means that the relevant bits on the left
          // side are either all 0, or all 1.
          auto bits = Properties::getSignExtBits(curr->left);
          uint32_t right = c->value.geti32();
          uint32_t numRelevantBits = 32 - bits + 1;
          uint32_t setRelevantBits =
            Bits::popCount(right >> uint32_t(bits - 1));
          // If all the relevant bits on C are zero
          // then we can mask off the high bits instead of sign-extending x.
          // This is valid because if x is negative, then the comparison was
          // false before (negative vs positive), and will still be false
          // as the sign bit will remain to cause a difference. And if x is
          // positive then the upper bits would be zero anyhow.
          if (setRelevantBits == 0) {
            curr->left = makeZeroExt(ext, bits);
            return replaceCurrent(curr);
          } else if (setRelevantBits == numRelevantBits) {
            // If all those bits are one, then we can do something similar if
            // we also zero-extend on the right as well. This is valid
            // because, as in the previous case, the sign bit differentiates
            // the two sides when they are different, and if the sign bit is
            // identical, then the upper bits don't matter, so masking them
            // off both sides is fine.
            curr->left = makeZeroExt(ext, bits);
            c->value = c->value.and_(Literal(Bits::lowBitMask(bits)));
            return replaceCurrent(curr);
          } else {
            // Otherwise, C's relevant bits are mixed, and then the two sides
            // can never be equal, as the left side's bits cannot be mixed.
            Builder builder(*getModule());
            // The result is either always true, or always false.
            c->value = Literal::makeFromInt32(curr->op == NeInt32, c->type);
            return replaceCurrent(
              builder.makeSequence(builder.makeDrop(ext), c));
          }
        }
      } else if (auto* left = Properties::getSignExtValue(curr->left)) {
        if (auto* right = Properties::getSignExtValue(curr->right)) {
          auto bits = Properties::getSignExtBits(curr->left);
          if (Properties::getSignExtBits(curr->right) == bits) {
            // we are comparing two sign-exts with the same bits, so we may as
            // well replace both with cheaper zexts
            curr->left = makeZeroExt(left, bits);
            curr->right = makeZeroExt(right, bits);
            return replaceCurrent(curr);
          }
        } else if (auto* load = curr->right->dynCast<Load>()) {
          // we are comparing a load to a sign-ext, we may be able to switch
          // to zext
          auto leftBits = Properties::getSignExtBits(curr->left);
          if (load->signed_ && leftBits == load->bytes * 8) {
            load->signed_ = false;
            curr->left = makeZeroExt(left, leftBits);
            return replaceCurrent(curr);
          }
        }
      } else if (auto* load = curr->left->dynCast<Load>()) {
        if (auto* right = Properties::getSignExtValue(curr->right)) {
          // we are comparing a load to a sign-ext, we may be able to switch
          // to zext
          auto rightBits = Properties::getSignExtBits(curr->right);
          if (load->signed_ && rightBits == load->bytes * 8) {
            load->signed_ = false;
            curr->right = makeZeroExt(right, rightBits);
            return replaceCurrent(curr);
          }
        }
      }
      // note that both left and right may be consts, but then we let
      // precompute compute the constant result
    } else if (curr->op == AddInt32 || curr->op == AddInt64 ||
               curr->op == SubInt32 || curr->op == SubInt64 ||
               curr->op == AddVecI8x16 || curr->op == AddVecI16x8 ||
               curr->op == SubVecI8x16 || curr->op == SubVecI16x8 ||
               curr->op == AddVecI32x4 || curr->op == AddVecI64x2 ||
               curr->op == SubVecI32x4 || curr->op == SubVecI64x2) {
      if (auto* ret = optimizeAddedConstants(curr)) {
        return replaceCurrent(ret);
      }
    } else if (curr->op == MulFloat32 || curr->op == MulFloat64 ||
               curr->op == DivFloat32 || curr->op == DivFloat64) {
      if (curr->left->type == curr->right->type) {
        if (auto* leftUnary = curr->left->dynCast<Unary>()) {
          if (leftUnary->op == Abstract::getUnary(curr->type, Abstract::Abs)) {
            if (auto* rightUnary = curr->right->dynCast<Unary>()) {
              if (leftUnary->op == rightUnary->op) { // both are abs ops
                // abs(x) * abs(y)   ==>   abs(x * y)
                // abs(x) / abs(y)   ==>   abs(x / y)
                curr->left = leftUnary->value;
                curr->right = rightUnary->value;
                leftUnary->value = curr;
                return replaceCurrent(leftUnary);
              }
            }
          }
        }
      }
    }
    // a bunch of operations on a constant right side can be simplified
    if (auto* right = curr->right->dynCast<Const>()) {
      if (curr->op == AndInt32) {
        auto mask = right->value.geti32();
        // and with -1 does nothing (common in asm.js output)
        if (mask == -1) {
          return replaceCurrent(curr->left);
        }
        // small loads do not need to be masked, the load itself masks
        if (auto* load = curr->left->dynCast<Load>()) {
          if ((load->bytes == 1 && mask == 0xff) ||
              (load->bytes == 2 && mask == 0xffff)) {
            load->signed_ = false;
            return replaceCurrent(curr->left);
          }
        } else if (auto maskedBits = Bits::getMaskedBits(mask)) {
          if (Bits::getMaxBits(curr->left, this) <= maskedBits) {
            // a mask of lower bits is not needed if we are already smaller
            return replaceCurrent(curr->left);
          }
        }
      }
      // some math operations have trivial results
      if (auto* ret = optimizeWithConstantOnRight(curr)) {
        return replaceCurrent(ret);
      }
      if (auto* ret = optimizeDoubletonWithConstantOnRight(curr)) {
        return replaceCurrent(ret);
      }
      if (right->type == Type::i32) {
        BinaryOp op;
        int32_t c = right->value.geti32();
        // First, try to lower signed operations to unsigned if that is
        // possible. Some unsigned operations like div_u or rem_u are usually
        // faster on VMs. Also this opens more possibilities for further
        // simplifications afterwards.
        if (c >= 0 && (op = makeUnsignedBinaryOp(curr->op)) != InvalidBinary &&
            Bits::getMaxBits(curr->left, this) <= 31) {
          curr->op = op;
        }
        if (c < 0 && c > std::numeric_limits<int32_t>::min() &&
            curr->op == DivUInt32) {
          // u32(x) / C   ==>   u32(x) >= C  iff C > 2^31
          // We avoid applying this for C == 2^31 due to conflict
          // with other rule which transform to more prefereble
          // right shift operation.
          curr->op = c == -1 ? EqInt32 : GeUInt32;
          return replaceCurrent(curr);
        }
        if (Bits::isPowerOf2((uint32_t)c)) {
          switch (curr->op) {
            case MulInt32:
              return replaceCurrent(optimizePowerOf2Mul(curr, (uint32_t)c));
            case RemUInt32:
              return replaceCurrent(optimizePowerOf2URem(curr, (uint32_t)c));
            case DivUInt32:
              return replaceCurrent(optimizePowerOf2UDiv(curr, (uint32_t)c));
            default:
              break;
          }
        }
      }
      if (right->type == Type::i64) {
        BinaryOp op;
        int64_t c = right->value.geti64();
        // See description above for Type::i32
        if (c >= 0 && (op = makeUnsignedBinaryOp(curr->op)) != InvalidBinary &&
            Bits::getMaxBits(curr->left, this) <= 63) {
          curr->op = op;
        }
        if (getPassOptions().shrinkLevel == 0 && c < 0 &&
            c > std::numeric_limits<int64_t>::min() && curr->op == DivUInt64) {
          // u64(x) / C   ==>   u64(u64(x) >= C)  iff C > 2^63
          // We avoid applying this for C == 2^31 due to conflict
          // with other rule which transform to more prefereble
          // right shift operation.
          // And apply this only for shrinkLevel == 0 due to it
          // increasing size by one byte.
          curr->op = c == -1LL ? EqInt64 : GeUInt64;
          curr->type = Type::i32;
          return replaceCurrent(
            Builder(*getModule()).makeUnary(ExtendUInt32, curr));
        }
        if (Bits::isPowerOf2((uint64_t)c)) {
          switch (curr->op) {
            case MulInt64:
              return replaceCurrent(optimizePowerOf2Mul(curr, (uint64_t)c));
            case RemUInt64:
              return replaceCurrent(optimizePowerOf2URem(curr, (uint64_t)c));
            case DivUInt64:
              return replaceCurrent(optimizePowerOf2UDiv(curr, (uint64_t)c));
            default:
              break;
          }
        }
      }
      if (curr->op == DivFloat32) {
        float c = right->value.getf32();
        if (Bits::isPowerOf2InvertibleFloat(c)) {
          return replaceCurrent(optimizePowerOf2FDiv(curr, c));
        }
      }
      if (curr->op == DivFloat64) {
        double c = right->value.getf64();
        if (Bits::isPowerOf2InvertibleFloat(c)) {
          return replaceCurrent(optimizePowerOf2FDiv(curr, c));
        }
      }
    }
    // a bunch of operations on a constant left side can be simplified
    if (curr->left->is<Const>()) {
      if (auto* ret = optimizeWithConstantOnLeft(curr)) {
        return replaceCurrent(ret);
      }
    }
    if (curr->op == AndInt32 || curr->op == OrInt32) {
      if (curr->op == AndInt32) {
        if (auto* ret = combineAnd(curr)) {
          return replaceCurrent(ret);
        }
      }
      // for or, we can potentially combine
      if (curr->op == OrInt32) {
        if (auto* ret = combineOr(curr)) {
          return replaceCurrent(ret);
        }
      }
      // bitwise operations
      // for and and or, we can potentially conditionalize
      if (auto* ret = conditionalizeExpensiveOnBitwise(curr)) {
        return replaceCurrent(ret);
      }
    }
    // relation/comparisons allow for math optimizations
    if (curr->isRelational()) {
      if (auto* ret = optimizeRelational(curr)) {
        return replaceCurrent(ret);
      }
    }
    // finally, try more expensive operations on the curr in
    // the case that they have no side effects
    if (!effects(curr->left).hasSideEffects()) {
      if (ExpressionAnalyzer::equal(curr->left, curr->right)) {
        if (auto* ret = optimizeBinaryWithEqualEffectlessChildren(curr)) {
          return replaceCurrent(ret);
        }
      }
    }

    if (auto* ret = deduplicateBinary(curr)) {
      return replaceCurrent(ret);
    }
  }

  void visitUnary(Unary* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }

    {
      using namespace Match;
      using namespace Abstract;
      Builder builder(*getModule());
      {
        // eqz(x - y)  =>  x == y
        Binary* inner;
        if (matches(curr, unary(EqZ, binary(&inner, Sub, any(), any())))) {
          inner->op = Abstract::getBinary(inner->left->type, Eq);
          inner->type = Type::i32;
          return replaceCurrent(inner);
        }
      }
      {
        // eqz(x + C)  =>  x == -C
        Const* c;
        Binary* inner;
        if (matches(curr, unary(EqZ, binary(&inner, Add, any(), ival(&c))))) {
          c->value = c->value.neg();
          inner->op = Abstract::getBinary(c->type, Eq);
          inner->type = Type::i32;
          return replaceCurrent(inner);
        }
      }
      {
        // eqz((signed)x % C_pot)  =>  eqz(x & (abs(C_pot) - 1))
        Const* c;
        Binary* inner;
        if (matches(curr, unary(EqZ, binary(&inner, RemS, any(), ival(&c)))) &&
            (c->value.isSignedMin() ||
             Bits::isPowerOf2(c->value.abs().getInteger()))) {
          inner->op = Abstract::getBinary(c->type, And);
          if (c->value.isSignedMin()) {
            c->value = Literal::makeSignedMax(c->type);
          } else {
            c->value = c->value.abs().sub(Literal::makeOne(c->type));
          }
          return replaceCurrent(curr);
        }
      }
      {
        // i32.wrap_i64 can be removed if the operations inside it do not
        // actually require 64 bits, e.g.:
        //
        // i32.wrap_i64(i64.extend_i32_u(x))  =>  x
        if (matches(curr, unary(WrapInt64, any()))) {
          if (auto* ret = optimizeWrappedResult(curr)) {
            return replaceCurrent(ret);
          }
        }
      }
      {
        // i32.eqz(i32.wrap_i64(x))  =>  i64.eqz(x)
        //   where maxBits(x) <= 32
        Unary* inner;
        Expression* x;
        if (matches(curr, unary(EqZInt32, unary(&inner, WrapInt64, any(&x)))) &&
            Bits::getMaxBits(x, this) <= 32) {
          inner->op = EqZInt64;
          return replaceCurrent(inner);
        }
      }
      {
        // i32.eqz(i32.eqz(x))  =>  i32(x) != 0
        // i32.eqz(i64.eqz(x))  =>  i64(x) != 0
        //   iff shinkLevel == 0
        // (1 instruction instead of 2, but 1 more byte)
        if (getPassRunner()->options.shrinkLevel == 0) {
          Expression* x;
          if (matches(curr, unary(EqZInt32, unary(EqZ, any(&x))))) {
            Builder builder(*getModule());
            return replaceCurrent(builder.makeBinary(
              getBinary(x->type, Ne),
              x,
              builder.makeConst(Literal::makeZero(x->type))));
          }
        }
      }
      {
        // i64.extend_i32_s(i32.wrap_i64(x))  =>  x
        //   where maxBits(x) <= 31
        //
        // i64.extend_i32_u(i32.wrap_i64(x))  =>  x
        //   where maxBits(x) <= 32
        Expression* x;
        UnaryOp unaryOp;
        if (matches(curr, unary(&unaryOp, unary(WrapInt64, any(&x))))) {
          if (unaryOp == ExtendSInt32 || unaryOp == ExtendUInt32) {
            auto maxBits = Bits::getMaxBits(x, this);
            if ((unaryOp == ExtendSInt32 && maxBits <= 31) ||
                (unaryOp == ExtendUInt32 && maxBits <= 32)) {
              return replaceCurrent(x);
            }
          }
        }
      }
      if (getModule()->features.hasSignExt()) {
        // i64.extend_i32_s(i32.wrap_i64(x))  =>  i64.extend32_s(x)
        Unary* inner;
        if (matches(curr,
                    unary(ExtendSInt32, unary(&inner, WrapInt64, any())))) {
          inner->op = ExtendS32Int64;
          inner->type = Type::i64;
          return replaceCurrent(inner);
        }
      }
    }

    if (curr->op == ExtendUInt32 || curr->op == ExtendSInt32) {
      if (auto* load = curr->value->dynCast<Load>()) {
        // i64.extend_i32_s(i32.load(_8|_16)(_u|_s)(x))  =>
        //    i64.load(_8|_16|_32)(_u|_s)(x)
        //
        // i64.extend_i32_u(i32.load(_8|_16)(_u|_s)(x))  =>
        //    i64.load(_8|_16|_32)(_u|_s)(x)
        //
        // but we can't do this in following cases:
        //
        //    i64.extend_i32_u(i32.load8_s(x))
        //    i64.extend_i32_u(i32.load16_s(x))
        //
        // this mixed sign/zero extensions can't represent in single
        // signed or unsigned 64-bit load operation. For example if `load8_s(x)`
        // return i8(-1) (0xFF) than sign extended result will be
        // i32(-1) (0xFFFFFFFF) and with zero extension to i64 we got
        // finally 0x00000000FFFFFFFF. However with `i64.load8_s` in this
        // situation we got `i64(-1)` (all ones) and with `i64.load8_u` it
        // will be 0x00000000000000FF.
        //
        // Another limitation is atomics which only have unsigned loads.
        // So we also avoid this only case:
        //
        //   i64.extend_i32_s(i32.atomic.load(x))

        // Special case for i32.load. In this case signedness depends on
        // extend operation.
        bool willBeSigned = curr->op == ExtendSInt32 && load->bytes == 4;
        if (!(curr->op == ExtendUInt32 && load->bytes <= 2 && load->signed_) &&
            !(willBeSigned && load->isAtomic)) {
          if (willBeSigned) {
            load->signed_ = true;
          }
          load->type = Type::i64;
          return replaceCurrent(load);
        }
      }
    }

    // Simple sign extends can be removed if the value is already sign-extended.
    auto signExtBits = getSignExtBits(curr->value);
    if (signExtBits > 0) {
      // Note that we can handle the case of |curr| having a larger sign-extend:
      // if we have an 8-bit value in 32-bit, then there are 24 sign bits, and
      // doing a sign-extend to 16 will only affect 16 of those 24, and the
      // effect is to leave them as they are.
      if ((curr->op == ExtendS8Int32 && signExtBits <= 8) ||
          (curr->op == ExtendS16Int32 && signExtBits <= 16) ||
          (curr->op == ExtendS8Int64 && signExtBits <= 8) ||
          (curr->op == ExtendS16Int64 && signExtBits <= 16) ||
          (curr->op == ExtendS32Int64 && signExtBits <= 32)) {
        return replaceCurrent(curr->value);
      }
    }

    if (Abstract::hasAnyReinterpret(curr->op)) {
      // i32.reinterpret_f32(f32.reinterpret_i32(x))  =>  x
      // i64.reinterpret_f64(f64.reinterpret_i64(x))  =>  x
      // f32.reinterpret_i32(i32.reinterpret_f32(x))  =>  x
      // f64.reinterpret_i64(i64.reinterpret_f64(x))  =>  x
      if (auto* inner = curr->value->dynCast<Unary>()) {
        if (Abstract::hasAnyReinterpret(inner->op)) {
          if (inner->value->type == curr->type) {
            return replaceCurrent(inner->value);
          }
        }
      }
      // f32.reinterpret_i32(i32.load(x))  =>  f32.load(x)
      // f64.reinterpret_i64(i64.load(x))  =>  f64.load(x)
      // i32.reinterpret_f32(f32.load(x))  =>  i32.load(x)
      // i64.reinterpret_f64(f64.load(x))  =>  i64.load(x)
      if (auto* load = curr->value->dynCast<Load>()) {
        if (!load->isAtomic && load->bytes == curr->type.getByteSize()) {
          load->type = curr->type;
          return replaceCurrent(load);
        }
      }
    }

    if (curr->op == EqZInt32) {
      if (auto* inner = curr->value->dynCast<Binary>()) {
        // Try to invert a relational operation using De Morgan's law
        auto op = invertBinaryOp(inner->op);
        if (op != InvalidBinary) {
          inner->op = op;
          return replaceCurrent(inner);
        }
      }
      // eqz of a sign extension can be of zero-extension
      if (auto* ext = Properties::getSignExtValue(curr->value)) {
        // we are comparing a sign extend to a constant, which means we can
        // use a cheaper zext
        auto bits = Properties::getSignExtBits(curr->value);
        curr->value = makeZeroExt(ext, bits);
        return replaceCurrent(curr);
      }
    } else if (curr->op == AbsFloat32 || curr->op == AbsFloat64) {
      // abs(-x)   ==>   abs(x)
      if (auto* unaryInner = curr->value->dynCast<Unary>()) {
        if (unaryInner->op ==
            Abstract::getUnary(unaryInner->type, Abstract::Neg)) {
          curr->value = unaryInner->value;
          return replaceCurrent(curr);
        }
      }
      // abs(x * x)   ==>   x * x
      // abs(x / x)   ==>   x / x
      if (auto* binary = curr->value->dynCast<Binary>()) {
        if ((binary->op == Abstract::getBinary(binary->type, Abstract::Mul) ||
             binary->op == Abstract::getBinary(binary->type, Abstract::DivS)) &&
            areConsecutiveInputsEqual(binary->left, binary->right)) {
          return replaceCurrent(binary);
        }
        // abs(0 - x)   ==>   abs(x),
        // only for fast math
        if (fastMath &&
            binary->op == Abstract::getBinary(binary->type, Abstract::Sub)) {
          if (auto* c = binary->left->dynCast<Const>()) {
            if (c->value.isZero()) {
              curr->value = binary->right;
              return replaceCurrent(curr);
            }
          }
        }
      }
    }

    if (auto* ret = deduplicateUnary(curr)) {
      return replaceCurrent(ret);
    }

    if (auto* ret = simplifyRoundingsAndConversions(curr)) {
      return replaceCurrent(ret);
    }
  }

  void visitSelect(Select* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    if (auto* ret = optimizeSelect(curr)) {
      return replaceCurrent(ret);
    }
    optimizeTernary(curr);
  }

  void visitGlobalSet(GlobalSet* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    // optimize out a set of a get
    auto* get = curr->value->dynCast<GlobalGet>();
    if (get && get->name == curr->name) {
      ExpressionManipulator::nop(curr);
      return replaceCurrent(curr);
    }
  }

  void visitIf(If* curr) {
    curr->condition = optimizeBoolean(curr->condition);
    if (curr->ifFalse) {
      if (auto* unary = curr->condition->dynCast<Unary>()) {
        if (unary->op == EqZInt32) {
          // flip if-else arms to get rid of an eqz
          curr->condition = unary->value;
          std::swap(curr->ifTrue, curr->ifFalse);
        }
      }
      if (curr->condition->type != Type::unreachable &&
          ExpressionAnalyzer::equal(curr->ifTrue, curr->ifFalse)) {
        // The sides are identical, so fold. If we can replace the If with one
        // arm and there are no side effects in the condition, replace it. But
        // make sure not to change a concrete expression to an unreachable
        // expression because we want to avoid having to refinalize.
        bool needCondition = effects(curr->condition).hasSideEffects();
        bool wouldBecomeUnreachable =
          curr->type.isConcrete() && curr->ifTrue->type == Type::unreachable;
        Builder builder(*getModule());
        if (!wouldBecomeUnreachable && !needCondition) {
          return replaceCurrent(curr->ifTrue);
        } else if (!wouldBecomeUnreachable) {
          return replaceCurrent(builder.makeSequence(
            builder.makeDrop(curr->condition), curr->ifTrue));
        } else {
          // Emit a block with the original concrete type.
          auto* ret = builder.makeBlock();
          if (needCondition) {
            ret->list.push_back(builder.makeDrop(curr->condition));
          }
          ret->list.push_back(curr->ifTrue);
          ret->finalize(curr->type);
          return replaceCurrent(ret);
        }
      }
      optimizeTernary(curr);
    }
  }

  void visitLocalSet(LocalSet* curr) {
    // Interactions between local.set/tee and ref.as_non_null can be optimized
    // in some cases, by removing or moving the ref.as_non_null operation.
    if (auto* as = curr->value->dynCast<RefAs>()) {
      if (as->op == RefAsNonNull &&
          getFunction()->getLocalType(curr->index).isNullable()) {
        //   (local.tee (ref.as_non_null ..))
        // =>
        //   (ref.as_non_null (local.tee ..))
        //
        // The reordering allows the ref.as to be potentially optimized further
        // based on where the value flows to.
        if (curr->isTee()) {
          curr->value = as->value;
          curr->finalize();
          as->value = curr;
          as->finalize();
          replaceCurrent(as);
          return;
        }

        // Otherwise, if this is not a tee, then no value falls through. The
        // ref.as_non_null acts as a null check here, basically. If we are
        // ignoring such traps, we can remove it.
        auto& passOptions = getPassOptions();
        if (passOptions.ignoreImplicitTraps || passOptions.trapsNeverHappen) {
          curr->value = as->value;
        }
      }
    }
  }

  void visitBreak(Break* curr) {
    if (curr->condition) {
      curr->condition = optimizeBoolean(curr->condition);
    }
  }

  void visitLoad(Load* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    optimizeMemoryAccess(curr->ptr, curr->offset, curr->memory);
  }

  void visitStore(Store* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    optimizeMemoryAccess(curr->ptr, curr->offset, curr->memory);
    optimizeStoredValue(curr->value, curr->bytes);
    if (auto* unary = curr->value->dynCast<Unary>()) {
      if (unary->op == WrapInt64) {
        // instead of wrapping to 32, just store some of the bits in the i64
        curr->valueType = Type::i64;
        curr->value = unary->value;
      } else if (!curr->isAtomic && Abstract::hasAnyReinterpret(unary->op) &&
                 curr->bytes == curr->valueType.getByteSize()) {
        // f32.store(y, f32.reinterpret_i32(x))  =>  i32.store(y, x)
        // f64.store(y, f64.reinterpret_i64(x))  =>  i64.store(y, x)
        // i32.store(y, i32.reinterpret_f32(x))  =>  f32.store(y, x)
        // i64.store(y, i64.reinterpret_f64(x))  =>  f64.store(y, x)
        curr->valueType = unary->value->type;
        curr->value = unary->value;
      }
    }
  }

  void optimizeStoredValue(Expression*& value, Index bytes) {
    if (!value->type.isInteger()) {
      return;
    }
    // truncates constant values during stores
    // (i32|i64).store(8|16|32)(p, C)   ==>
    //    (i32|i64).store(8|16|32)(p, C & mask)
    if (auto* c = value->dynCast<Const>()) {
      if (value->type == Type::i64 && bytes == 4) {
        c->value = c->value.and_(Literal(uint64_t(0xffffffff)));
      } else {
        c->value = c->value.and_(
          Literal::makeFromInt32(Bits::lowBitMask(bytes * 8), value->type));
      }
    }
    // stores of fewer bits truncates anyhow
    if (auto* binary = value->dynCast<Binary>()) {
      if (binary->op == AndInt32) {
        if (auto* right = binary->right->dynCast<Const>()) {
          if (right->type == Type::i32) {
            auto mask = right->value.geti32();
            if ((bytes == 1 && mask == 0xff) ||
                (bytes == 2 && mask == 0xffff)) {
              value = binary->left;
            }
          }
        }
      } else if (auto* ext = Properties::getSignExtValue(binary)) {
        // if sign extending the exact bit size we store, we can skip the
        // extension if extending something bigger, then we just alter bits we
        // don't save anyhow
        if (Properties::getSignExtBits(binary) >= Index(bytes) * 8) {
          value = ext;
        }
      }
    }
  }

  void visitMemoryCopy(MemoryCopy* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    assert(getModule()->features.hasBulkMemoryOpt());
    if (auto* ret = optimizeMemoryCopy(curr)) {
      return replaceCurrent(ret);
    }
  }

  void visitMemoryFill(MemoryFill* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    assert(getModule()->features.hasBulkMemoryOpt());
    if (auto* ret = optimizeMemoryFill(curr)) {
      return replaceCurrent(ret);
    }
  }

  void visitCallRef(CallRef* curr) {
    skipNonNullCast(curr->target, curr);
    if (trapOnNull(curr, curr->target)) {
      return;
    }
    if (curr->target->type == Type::unreachable) {
      // The call_ref is not reached; leave this for DCE.
      return;
    }

    if (auto* ref = curr->target->dynCast<RefFunc>()) {
      // We know the target!
      replaceCurrent(
        Builder(*getModule())
          .makeCall(ref->func, curr->operands, curr->type, curr->isReturn));
      return;
    }

    if (auto* get = curr->target->dynCast<TableGet>()) {
      // (call_ref ..args.. (table.get $table (index))
      //   =>
      // (call_indirect $table ..args.. (index))
      replaceCurrent(Builder(*getModule())
                       .makeCallIndirect(get->table,
                                         get->index,
                                         curr->operands,
                                         get->type.getHeapType(),
                                         curr->isReturn));
      return;
    }

    auto features = getModule()->features;

    // It is possible the target is not a function reference, but we can infer
    // the fallthrough value there. It takes more work to optimize this case,
    // but it is pretty important to allow a call_ref to become a fast direct
    // call, so make the effort.
    if (auto* ref = Properties::getFallthrough(
                      curr->target, getPassOptions(), *getModule())
                      ->dynCast<RefFunc>()) {
      // Check if the fallthrough make sense. We may have cast it to a different
      // type, which would be a problem - we'd be replacing a call_ref to one
      // type with a direct call to a function of another type. That would trap
      // at runtime; be careful not to emit invalid IR here.
      if (curr->target->type.getHeapType() != ref->type.getHeapType()) {
        return;
      }
      Builder builder(*getModule());
      if (curr->operands.empty()) {
        // No operands, so this is simple and there is nothing to reorder: just
        // emit:
        //
        // (block
        //  (drop curr->target)
        //  (call ref.func-from-curr->target)
        // )
        replaceCurrent(builder.makeSequence(
          builder.makeDrop(curr->target),
          builder.makeCall(ref->func, {}, curr->type, curr->isReturn)));
        return;
      }

      // In the presence of operands, we must execute the code in curr->target
      // after the last operand and before the call happens. Interpose at the
      // last operand:
      //
      // (call ref.func-from-curr->target)
      //  (operand1)
      //  (..)
      //  (operandN-1)
      //  (block
      //   (local.set $temp (operandN))
      //   (drop curr->target)
      //   (local.get $temp)
      //  )
      // )
      auto* lastOperand = curr->operands.back();
      auto lastOperandType = lastOperand->type;
      if (lastOperandType == Type::unreachable) {
        // The call_ref is not reached; leave this for DCE.
        return;
      }
      if (!TypeUpdating::canHandleAsLocal(lastOperandType)) {
        // We cannot create a local, so we must give up.
        return;
      }
      Index tempLocal = builder.addVar(
        getFunction(),
        TypeUpdating::getValidLocalType(lastOperandType, features));
      auto* set = builder.makeLocalSet(tempLocal, lastOperand);
      auto* drop = builder.makeDrop(curr->target);
      auto* get = TypeUpdating::fixLocalGet(
        builder.makeLocalGet(tempLocal, lastOperandType), *getModule());
      curr->operands.back() = builder.makeBlock({set, drop, get});
      replaceCurrent(builder.makeCall(
        ref->func, curr->operands, curr->type, curr->isReturn));
      return;
    }

    // If the target is a select of two different constants, we can emit an if
    // over two direct calls.
    if (auto* calls = CallUtils::convertToDirectCalls(
          curr,
          [](Expression* target) -> CallUtils::IndirectCallInfo {
            if (auto* refFunc = target->dynCast<RefFunc>()) {
              return CallUtils::Known{refFunc->func};
            }
            return CallUtils::Unknown{};
          },
          *getFunction(),
          *getModule())) {
      replaceCurrent(calls);
    }
  }

  // Note on removing casts (which the following utilities, skipNonNullCast and
  // skipCast do): removing a cast is potentially dangerous, as it removes
  // information from the IR. For example:
  //
  //  (ref.test (ref i31)
  //    (ref.cast (ref i31)
  //      (local.get $anyref)))
  //
  // The local has no useful type info here (it is anyref). The cast forces it
  // to be an i31, so we know that if we do not trap then the ref.test will
  // definitely be 1. But if we removed the ref.cast first (which we can do in
  // traps-never-happen mode) then we'd not have the type info we need to
  // optimize that way.
  //
  // To avoid such risks we should keep in mind the following:
  //
  //  * Before removing a cast we should use its type information in the best
  //    way we can. Only after doing so should a cast be removed. In the exmaple
  //    above, that means first seeing that the ref.test must return 1, and only
  //    then possibly removing the ref.cast.
  //  * Do not remove a cast if removing it might remove useful information for
  //    others. For example,
  //
  //      (ref.cast (ref null $A)
  //        (ref.as_non_null ..))
  //
  //    If we remove the inner cast then the outer cast becomes nullable. That
  //    means we'd be throwing away useful information, which we should not do,
  //    even in traps-never-happen mode and even if the wasm would validate
  //    without the cast. Only if we saw that the parents of the outer cast
  //    cannot benefit from non-nullability should we remove it.
  //    Another example:
  //
  //      (struct.get $A 0
  //        (ref.cast $B ..))
  //
  //    The cast only changes the type of the reference, which is consumed in
  //    this expression and so we don't have more parents to consider. But it is
  //    risky to remove this cast, since e.g. GUFA benefits from such info:
  //    it tells GUFA that we are reading from a $B here, and not the supertype
  //    $A. If $B may contain fewer values in field 0 than $A, then GUFA might
  //    be able to optimize better with this cast. Now, in traps-never-happen
  //    mode we can assume that only $B can arrive here, which means GUFA might
  //    be able to infer that even without the cast - but it might not, if we
  //    hit a limitation of GUFA. Some code patterns simply cannot be expected
  //    to be always inferred, say if a data structure has a tagged variant:
  //
  //      {
  //        tag: i32,
  //        ref: anyref
  //      }
  //
  //    Imagine that if tag == 0 then the reference always contains struct $A,
  //    and if tag == 1 then it always contains a struct $B, and so forth. We
  //    can't expect GUFA to figure out such invariants in general. But by
  //    having casts in the right places we can help GUFA optimize:
  //
  //      (if
  //        (tag == 1)
  //        (struct.get $A 0
  //          (ref.cast $B ..))
  //
  //    We know it must be a $B due to the tag. By keeping the cast there we can
  //    make sure that optimizations can benefit from that.
  //
  //    Given the large amount of potential benefit we can get from a successful
  //    optimization in GUFA, any reduction there may be a bad idea, so we
  //    should be very careful and probably *not* remove such casts.

  // If an instruction traps on a null input, there is no need for a
  // ref.as_non_null on that input: we will trap either way (and the binaryen
  // optimizer does not differentiate traps).
  //
  // See "notes on removing casts", above. However, in most cases removing a
  // non-null cast is obviously safe to do, since we only remove one if another
  // check will happen later.
  //
  // We also pass in the parent, because we need to be careful about ordering:
  // if the parent has other children than |input| then we may not be able to
  // remove the trap. For example,
  //
  //  (struct.set
  //   (ref.as_non_null X)
  //   (call $foo)
  //  )
  //
  // If X is null we'd trap before the call to $foo. If we remove the
  // ref.as_non_null then the struct.set will still trap, of course, but that
  // will only happen *after* the call, which is wrong.
  void skipNonNullCast(Expression*& input, Expression* parent) {
    // Check the other children for the ordering problem only if we find a
    // possible optimization, to avoid wasted work.
    bool checkedSiblings = false;
    auto& options = getPassOptions();
    while (1) {
      if (auto* as = input->dynCast<RefAs>()) {
        if (as->op == RefAsNonNull) {
          // The problem with effect ordering that is described above is not an
          // issue if traps are assumed to never happen anyhow.
          if (!checkedSiblings && !options.trapsNeverHappen) {
            // We need to see if a child with side effects exists after |input|.
            // If there is such a child, it is a problem as mentioned above (it
            // is fine for such a child to appear *before* |input|, as then we
            // wouldn't be reordering effects). Thus, all we need to do is
            // accumulate the effects in children after |input|, as we want to
            // move the trap across those.
            bool seenInput = false;
            EffectAnalyzer crossedEffects(options, *getModule());
            for (auto* child : ChildIterator(parent)) {
              if (child == input) {
                seenInput = true;
              } else if (seenInput) {
                crossedEffects.walk(child);
              }
            }

            // Check if the effects we cross interfere with the effects of the
            // trap we want to move. (We use a shallow effect analyzer since we
            // will only move the ref.as_non_null itself.)
            ShallowEffectAnalyzer movingEffects(options, *getModule(), input);
            if (crossedEffects.invalidates(movingEffects)) {
              return;
            }

            // If we got here, we've checked the siblings and found no problem.
            checkedSiblings = true;
          }
          input = as->value;
          continue;
        }
      }
      break;
    }
  }

  // As skipNonNullCast, but skips all casts if we can do so. This is useful in
  // cases where we don't actually care about the type but just the value, that
  // is, if casts of the type do not affect our behavior (which is the case in
  // ref.eq for example).
  //
  // |requiredType| is the required supertype of the final output. We will not
  // remove a cast that would leave something that would break that. If
  // |requiredType| is not provided we will accept any type there.
  //
  // See "notes on removing casts", above, for when this is safe to do.
  void skipCast(Expression*& input, Type requiredType = Type::none) {
    // Traps-never-happen mode is a requirement for us to optimize here.
    if (!getPassOptions().trapsNeverHappen) {
      return;
    }
    while (1) {
      if (auto* as = input->dynCast<RefAs>()) {
        if (requiredType == Type::none ||
            Type::isSubType(as->value->type, requiredType)) {
          input = as->value;
          continue;
        }
      } else if (auto* cast = input->dynCast<RefCast>()) {
        if (requiredType == Type::none ||
            Type::isSubType(cast->ref->type, requiredType)) {
          input = cast->ref;
          continue;
        }
      }
      break;
    }
  }

  // Appends a result after the dropped children, if we need them.
  Expression* getDroppedChildrenAndAppend(Expression* curr,
                                          Expression* result) {
    return wasm::getDroppedChildrenAndAppend(
      curr, *getModule(), getPassOptions(), result);
  }

  Expression* getDroppedChildrenAndAppend(Expression* curr, Literal value) {
    auto* result = Builder(*getModule()).makeConst(value);
    return getDroppedChildrenAndAppend(curr, result);
  }

  Expression* getResultOfFirst(Expression* first, Expression* second) {
    return wasm::getResultOfFirst(
      first, second, getFunction(), getModule(), getPassOptions());
  }

  // Optimize an instruction and the reference it operates on, under the
  // assumption that if the reference is a null then we will trap. Returns true
  // if we replaced the expression with something simpler. Returns false if we
  // found nothing to optimize, or if we just modified or replaced the ref (but
  // not the expression itself).
  bool trapOnNull(Expression* curr, Expression*& ref) {
    Builder builder(*getModule());

    if (getPassOptions().trapsNeverHappen) {
      // We can ignore the possibility of the reference being an input, so
      //
      //    (if
      //      (condition)
      //      (null)
      //      (other))
      // =>
      //    (drop
      //      (condition))
      //    (other)
      //
      // That is, we will by assumption not read from the null, so remove that
      // arm.
      //
      // TODO We could recurse here.
      // TODO We could do similar things for casts (rule out an impossible arm).
      // TODO Worth thinking about an 'assume' instrinsic of some form that
      //      annotates knowledge about a value, or another mechanism to allow
      //      that information to be passed around.

      // Note that we must check that the null is actually flowed out, that is,
      // that control flow is not transferred before:
      //
      //    (if
      //      (1)
      //      (block (result null)
      //        (return)
      //      )
      //      (other))
      //
      // The true arm has a bottom type, but in fact it just returns out of the
      // function and the null does not actually flow out. We can only optimize
      // here if a null definitely flows out (as only that would cause a trap).
      auto flowsOutNull = [&](Expression* child) {
        return child->type.isNull() && !effects(child).transfersControlFlow();
      };

      if (auto* iff = ref->dynCast<If>()) {
        if (iff->ifFalse) {
          if (flowsOutNull(iff->ifTrue)) {
            if (ref->type != iff->ifFalse->type) {
              refinalize = true;
            }
            ref = builder.makeSequence(builder.makeDrop(iff->condition),
                                       iff->ifFalse);
            return false;
          }
          if (flowsOutNull(iff->ifFalse)) {
            if (ref->type != iff->ifTrue->type) {
              refinalize = true;
            }
            ref = builder.makeSequence(builder.makeDrop(iff->condition),
                                       iff->ifTrue);
            return false;
          }
        }
      }

      if (auto* select = ref->dynCast<Select>()) {
        // We must check for unreachability explicitly here because a full
        // refinalize only happens at the end. That is, the select may stil be
        // reachable after we turned one child into an unreachable, and we are
        // calling getResultOfFirst which will error on unreachability.
        if (flowsOutNull(select->ifTrue) &&
            select->ifFalse->type != Type::unreachable) {
          ref = builder.makeSequence(
            builder.makeDrop(select->ifTrue),
            getResultOfFirst(select->ifFalse,
                             builder.makeDrop(select->condition)));
          return false;
        }
        if (flowsOutNull(select->ifFalse) &&
            select->ifTrue->type != Type::unreachable) {
          ref = getResultOfFirst(
            select->ifTrue,
            builder.makeSequence(builder.makeDrop(select->ifFalse),
                                 builder.makeDrop(select->condition)));
          return false;
        }
      }
    }

    // A nullable cast can be turned into a non-nullable one:
    //
    //    (struct.get ;; or something else that traps on a null ref
    //      (ref.cast null
    // =>
    //    (struct.get
    //      (ref.cast      ;; now non-nullable
    //
    // Either way we trap here, but refining the type may have benefits later.
    if (ref->type.isNullable()) {
      if (auto* cast = ref->dynCast<RefCast>()) {
        // Note that we must be the last child of the parent, otherwise effects
        // in the middle may need to remain:
        //
        //    (struct.set
        //      (ref.cast null
        //      (call ..
        //
        // The call here must execute before the trap in the struct.set. To
        // avoid that problem, inspect all children after us. If there are no
        // such children, then there is no problem; if there are, see below.
        auto canOptimize = true;
        auto seenRef = false;
        for (auto* child : ChildIterator(curr)) {
          if (child == ref) {
            seenRef = true;
          } else if (seenRef) {
            // This is a child after the reference. Check it for effects. For
            // simplicity, focus on the case of traps-never-happens: if we can
            // assume no trap occurs in the parent, then there must not be a
            // trap in the child either, unless control flow transfers and we
            // might not reach the parent.
            // TODO: handle more cases.
            if (!getPassOptions().trapsNeverHappen ||
                effects(child).transfersControlFlow()) {
              canOptimize = false;
              break;
            }
          }
        }
        if (canOptimize) {
          cast->type = Type(cast->type.getHeapType(), NonNullable);
        }
      }
    }

    auto fallthrough =
      Properties::getFallthrough(ref, getPassOptions(), *getModule());

    if (fallthrough->type.isNull()) {
      replaceCurrent(
        getDroppedChildrenAndAppend(curr, builder.makeUnreachable()));
      return true;
    }
    return false;
  }

  void visitRefEq(RefEq* curr) {
    // The types may prove that the same reference cannot appear on both sides.
    auto leftType = curr->left->type;
    auto rightType = curr->right->type;
    if (leftType == Type::unreachable || rightType == Type::unreachable) {
      // Leave this for DCE.
      return;
    }
    auto leftHeapType = leftType.getHeapType();
    auto rightHeapType = rightType.getHeapType();
    auto leftIsHeapSubtype = HeapType::isSubType(leftHeapType, rightHeapType);
    auto rightIsHeapSubtype = HeapType::isSubType(rightHeapType, leftHeapType);
    if (!leftIsHeapSubtype && !rightIsHeapSubtype &&
        (leftType.isNonNullable() || rightType.isNonNullable())) {
      // The heap types have no intersection, so the only thing that can
      // possibly appear on both sides is null, but one of the two is non-
      // nullable, which rules that out. So there is no way that the same
      // reference can appear on both sides.
      replaceCurrent(
        getDroppedChildrenAndAppend(curr, Literal::makeZero(Type::i32)));
      return;
    }

    // Equality does not depend on the type, so casts may be removable.
    //
    // This is safe to do first because nothing farther down cares about the
    // type, and we consume the two input references, so removing a cast could
    // not help our parents (see "notes on removing casts").
    Type nullableEq = Type(HeapType::eq, Nullable);
    skipCast(curr->left, nullableEq);
    skipCast(curr->right, nullableEq);

    // Identical references compare equal.
    // (Technically we do not need to check if the inputs are also foldable into
    // a single one, but we do not have utility code to handle non-foldable
    // cases yet; the foldable case we do handle is the common one of the first
    // child being a tee and the second a get of that tee. TODO)
    if (areConsecutiveInputsEqualAndFoldable(curr->left, curr->right)) {
      replaceCurrent(
        getDroppedChildrenAndAppend(curr, Literal::makeOne(Type::i32)));
      return;
    }

    // Canonicalize to the pattern of a null on the right-hand side, if there is
    // one. This makes pattern matching simpler.
    if (curr->left->is<RefNull>()) {
      std::swap(curr->left, curr->right);
    }

    // RefEq of a value to Null can be replaced with RefIsNull.
    if (curr->right->is<RefNull>()) {
      replaceCurrent(Builder(*getModule()).makeRefIsNull(curr->left));
    }
  }

  void visitStructNew(StructNew* curr) {
    // If values are provided, but they are all the default, then we can remove
    // them (in reachable code).
    if (curr->type == Type::unreachable || curr->isWithDefault()) {
      return;
    }

    auto& passOptions = getPassOptions();
    const auto& fields = curr->type.getHeapType().getStruct().fields;
    assert(fields.size() == curr->operands.size());

    for (Index i = 0; i < fields.size(); i++) {
      // The field must be defaultable.
      auto type = fields[i].type;
      if (!type.isDefaultable()) {
        return;
      }

      // The field must be written the default value.
      auto* value = Properties::getFallthrough(
        curr->operands[i], passOptions, *getModule());
      if (!Properties::isSingleConstantExpression(value) ||
          Properties::getLiteral(value) != Literal::makeZero(type)) {
        return;
      }
    }

    // Success! Drop the children and return a struct.new_with_default.
    auto* rep = getDroppedChildrenAndAppend(curr, curr);
    curr->operands.clear();
    assert(curr->isWithDefault());
    replaceCurrent(rep);
  }

  void visitStructGet(StructGet* curr) {
    skipNonNullCast(curr->ref, curr);
    trapOnNull(curr, curr->ref);
    // Relax acquire loads of unshared fields to unordered because they cannot
    // synchronize with other threads.
    if (curr->order == MemoryOrder::AcqRel && curr->ref->type.isRef() &&
        !curr->ref->type.getHeapType().isShared()) {
      curr->order = MemoryOrder::Unordered;
    }
  }

  void visitStructSet(StructSet* curr) {
    skipNonNullCast(curr->ref, curr);
    if (trapOnNull(curr, curr->ref)) {
      return;
    }

    if (curr->ref->type != Type::unreachable && curr->value->type.isInteger()) {
      // We must avoid the case of a null type.
      auto heapType = curr->ref->type.getHeapType();
      if (heapType.isStruct()) {
        const auto& fields = heapType.getStruct().fields;
        optimizeStoredValue(curr->value, fields[curr->index].getByteSize());
      }
    }

    // Relax release stores of unshared fields to unordered because they cannot
    // synchronize with other threads.
    if (curr->order == MemoryOrder::AcqRel && curr->ref->type.isRef() &&
        !curr->ref->type.getHeapType().isShared()) {
      curr->order = MemoryOrder::Unordered;
    }
  }

  void visitArrayNew(ArrayNew* curr) {
    // If a value is provided, we can optimize in some cases.
    if (curr->type == Type::unreachable || curr->isWithDefault()) {
      return;
    }

    Builder builder(*getModule());

    // ArrayNew of size 1 is less efficient than ArrayNewFixed with one value
    // (the latter avoids a Const, which ends up saving one byte).
    // TODO: also look at the case with a fallthrough or effects on the size
    if (auto* c = curr->size->dynCast<Const>()) {
      if (c->value.geti32() == 1) {
        // Optimize to ArrayNewFixed. Note that if the value is the default
        // then we may end up optimizing further in visitArrayNewFixed.
        replaceCurrent(
          builder.makeArrayNewFixed(curr->type.getHeapType(), {curr->init}));
        return;
      }
    }

    // If the type is defaultable then perhaps the value here is the default.
    auto type = curr->type.getHeapType().getArray().element.type;
    if (!type.isDefaultable()) {
      return;
    }

    // The value must be the default/zero.
    auto& passOptions = getPassOptions();
    auto zero = Literal::makeZero(type);
    auto* value =
      Properties::getFallthrough(curr->init, passOptions, *getModule());
    if (!Properties::isSingleConstantExpression(value) ||
        Properties::getLiteral(value) != zero) {
      return;
    }

    // Success! Drop the init and return an array.new_with_default.
    auto* init = curr->init;
    curr->init = nullptr;
    assert(curr->isWithDefault());
    replaceCurrent(builder.makeSequence(builder.makeDrop(init), curr));
  }

  void visitArrayNewFixed(ArrayNewFixed* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }

    auto size = curr->values.size();
    if (size == 0) {
      // TODO: Consider what to do in the trivial case of an empty array: we can
      //       can use ArrayNew or ArrayNewFixed there. Measure which is best.
      return;
    }

    auto& passOptions = getPassOptions();

    // If all the values are equal then we can optimize, either to
    // array.new_default (if they are all equal to the default) or array.new (if
    // they are all equal to some other value). First, see if they are all
    // equal, which we do by comparing in pairs: [0,1], then [1,2], etc.
    for (Index i = 0; i < size - 1; i++) {
      if (!areConsecutiveInputsEqual(curr->values[i], curr->values[i + 1])) {
        return;
      }
    }

    // Great, they are all equal!

    Builder builder(*getModule());

    // See if they are equal to a constant, and if that constant is the default.
    auto type = curr->type.getHeapType().getArray().element.type;
    if (type.isDefaultable()) {
      auto* value =
        Properties::getFallthrough(curr->values[0], passOptions, *getModule());

      if (Properties::isSingleConstantExpression(value) &&
          Properties::getLiteral(value) == Literal::makeZero(type)) {
        // They are all equal to the default. Drop the children and return an
        // array.new_with_default.
        auto* withDefault = builder.makeArrayNew(
          curr->type.getHeapType(), builder.makeConst(int32_t(size)));
        replaceCurrent(getDroppedChildrenAndAppend(curr, withDefault));
        return;
      }
    }

    // They are all equal to each other, but not to the default value. If there
    // are 2 or more elements here then we can save by using array.new. For
    // example, with 2 elements we are doing this:
    //
    //  (array.new_fixed
    //    (A)
    //    (A)
    //  )
    // =>
    //  (array.new
    //    (A)
    //    (i32.const 2) ;; get two copies of (A)
    //  )
    //
    // However, with 1, ArrayNewFixed is actually more compact, and we optimize
    // ArrayNew to it, above.
    if (size == 1) {
      return;
    }

    // Move children to locals, if we need to keep them around. We are removing
    // them all, except from the first, when we remove the array.new_fixed's
    // list of children and replace it with a single child + a constant for the
    // number of children.
    ChildLocalizer localizer(
      curr, getFunction(), *getModule(), getPassOptions());
    auto* block = localizer.getChildrenReplacement();
    auto* arrayNew = builder.makeArrayNew(curr->type.getHeapType(),
                                          builder.makeConst(int32_t(size)),
                                          curr->values[0]);
    block->list.push_back(arrayNew);
    block->finalize();
    replaceCurrent(block);
  }

  void visitArrayGet(ArrayGet* curr) {
    skipNonNullCast(curr->ref, curr);
    trapOnNull(curr, curr->ref);
  }

  void visitArraySet(ArraySet* curr) {
    skipNonNullCast(curr->ref, curr);
    if (trapOnNull(curr, curr->ref)) {
      return;
    }

    if (curr->value->type.isInteger()) {
      if (auto field = GCTypeUtils::getField(curr->ref->type)) {
        optimizeStoredValue(curr->value, field->getByteSize());
      }
    }
  }

  void visitArrayLen(ArrayLen* curr) {
    skipNonNullCast(curr->ref, curr);
    trapOnNull(curr, curr->ref);
  }

  void visitArrayCopy(ArrayCopy* curr) {
    skipNonNullCast(curr->destRef, curr);
    skipNonNullCast(curr->srcRef, curr);
    trapOnNull(curr, curr->destRef) || trapOnNull(curr, curr->srcRef);
  }

  void visitRefCast(RefCast* curr) {
    // Note we must check the ref's type here and not our own, since we only
    // refinalize at the end, which means our type may not have been updated yet
    // after a change in the child.
    // TODO: we could update unreachability up the stack perhaps, or just move
    //       all patterns that can add unreachability to a pass that does so
    //       already like vacuum or dce.
    if (curr->ref->type == Type::unreachable) {
      return;
    }

    if (curr->type.isNonNullable() && trapOnNull(curr, curr->ref)) {
      return;
    }

    Builder builder(*getModule());

    // Look at all the fallthrough values to get the most precise possible type
    // of the value we are casting. local.tee, br_if, and blocks can all "lose"
    // type information, so looking at all the fallthrough values can give us a
    // more precise type than is stored in the IR.
    Type refType =
      Properties::getFallthroughType(curr->ref, getPassOptions(), *getModule());

    // As a first step, we can tighten up the cast type to be the greatest lower
    // bound of the original cast type and the type we know the cast value to
    // have. We know any less specific type either cannot appear or will fail
    // the cast anyways.
    auto glb = Type::getGreatestLowerBound(curr->type, refType);
    if (glb != Type::unreachable && glb != curr->type) {
      curr->type = glb;
      refinalize = true;
      // Call replaceCurrent() to make us re-optimize this node, as we may have
      // just unlocked further opportunities. (We could just continue down to
      // the rest, but we'd need to do more work to make sure all the local
      // state in this function is in sync which this change; it's easier to
      // just do another clean pass on this node.)
      replaceCurrent(curr);
      return;
    }

    // Given what we know about the type of the value, determine what we know
    // about the results of the cast and optimize accordingly.
    switch (GCTypeUtils::evaluateCastCheck(refType, curr->type)) {
      case GCTypeUtils::Unknown:
        // The cast may or may not succeed, so we cannot optimize.
        break;
      case GCTypeUtils::Success:
      case GCTypeUtils::SuccessOnlyIfNonNull: {
        // We know the cast will succeed, or at most requires a null check, so
        // we can try to optimize it out. Find the best-typed fallthrough value
        // to propagate.
        auto** refp = Properties::getMostRefinedFallthrough(
          &curr->ref, getPassOptions(), *getModule());
        auto* ref = *refp;
        assert(ref->type.isRef());
        if (HeapType::isSubType(ref->type.getHeapType(),
                                curr->type.getHeapType())) {
          // We know ref's heap type matches, but the knowledge that the
          // nullabillity matches might come from somewhere else or we might not
          // know at all whether the nullability matches, so we might need to
          // emit a null check.
          bool needsNullCheck = ref->type.getNullability() == Nullable &&
                                curr->type.getNullability() == NonNullable;
          // If the best value to propagate is the argument to the cast, we can
          // simply remove the cast (or downgrade it to a null check if
          // necessary).
          if (ref == curr->ref) {
            if (needsNullCheck) {
              replaceCurrent(builder.makeRefAs(RefAsNonNull, curr->ref));
            } else {
              replaceCurrent(ref);
            }
            return;
          }
          // Otherwise we can't just remove the cast and replace it with `ref`
          // because the intermediate expressions might have had side effects.
          // We can replace the cast with a drop followed by a direct return of
          // the value, though.
          if (ref->type.isNull()) {
            // We can materialize the resulting null value directly.
            //
            // The type must be nullable for us to do that, which it normally
            // would be, aside from the interesting corner case of
            // uninhabitable types:
            //
            //  (ref.cast func
            //    (block (result (ref nofunc))
            //      (unreachable)
            //    )
            //  )
            //
            // (ref nofunc) is a subtype of (ref func), so the cast might seem
            // to be successful, but since the input is uninhabitable we won't
            // even reach the cast. Such casts will be evaluated as
            // Unreachable, so we'll not hit this assertion.
            assert(curr->type.isNullable());
            auto nullType = curr->type.getHeapType().getBottom();
            replaceCurrent(builder.makeSequence(builder.makeDrop(curr->ref),
                                                builder.makeRefNull(nullType)));
            return;
          }
          // We need to use a tee to return the value since we can't materialize
          // it directly.
          auto scratch = builder.addVar(getFunction(), ref->type);
          *refp = builder.makeLocalTee(scratch, ref, ref->type);
          Expression* get = builder.makeLocalGet(scratch, ref->type);
          if (needsNullCheck) {
            get = builder.makeRefAs(RefAsNonNull, get);
          }
          replaceCurrent(
            builder.makeSequence(builder.makeDrop(curr->ref), get));
          return;
        }
        // If we get here, then we know that the heap type of the cast input is
        // more refined than the heap type of the best available fallthrough
        // expression. The only way this can happen is if we were able to infer
        // that the input has bottom heap type because it was typed with
        // multiple, incompatible heap types in different fallthrough
        // expressions. For example:
        //
        // (ref.cast eqref
        //   (br_on_cast_fail $l anyref i31ref
        //     (br_on_cast_fail $l anyref structref
        //       ...)))
        //
        // In this case, the cast succeeds because the value must be null, so we
        // can fall through to handle that case.
        assert(Type::isSubType(refType, ref->type));
        assert(refType.getHeapType().isBottom());
      }
        [[fallthrough]];
      case GCTypeUtils::SuccessOnlyIfNull: {
        auto nullType = Type(curr->type.getHeapType().getBottom(), Nullable);
        // The cast either returns null or traps. In trapsNeverHappen mode
        // we know the result, since by assumption it will not trap.
        if (getPassOptions().trapsNeverHappen) {
          replaceCurrent(builder.makeBlock(
            {builder.makeDrop(curr->ref), builder.makeRefNull(nullType)},
            curr->type));
          return;
        }
        // Otherwise, we should have already refined the cast type to cast
        // directly to null.
        assert(curr->type == nullType);
        break;
      }
      case GCTypeUtils::Unreachable:
      case GCTypeUtils::Failure:
        // This cast cannot succeed, or it cannot even be reached, so we can
        // trap. Make sure to emit a block with the same type as us; leave
        // updating types for other passes.
        replaceCurrent(builder.makeBlock(
          {builder.makeDrop(curr->ref), builder.makeUnreachable()},
          curr->type));
        return;
    }

    // If we got past the optimizations above, it must be the case that we
    // cannot tell from the static types whether the cast will succeed or not,
    // which means we must have a proper down cast.
    assert(Type::isSubType(curr->type, curr->ref->type));

    if (auto* child = curr->ref->dynCast<RefCast>()) {
      // Repeated casts can be removed, leaving just the most demanding of them.
      // Since we know the current cast is a downcast, it must be strictly
      // stronger than its child cast and we can remove the child cast entirely.
      curr->ref = child->ref;
      return;
    }

    // Similarly, ref.cast can be combined with ref.as_non_null.
    //
    //   (ref.cast null (ref.as_non_null ..))
    // =>
    //   (ref.cast ..)
    //
    if (auto* as = curr->ref->dynCast<RefAs>(); as && as->op == RefAsNonNull) {
      curr->ref = as->value;
      curr->type = Type(curr->type.getHeapType(), NonNullable);
    }
  }

  void visitRefTest(RefTest* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }

    Builder builder(*getModule());

    // Parallel to the code in visitRefCast: we look not just at the final type
    // we are given, but at fallthrough values as well.
    Type refType =
      Properties::getFallthroughType(curr->ref, getPassOptions(), *getModule());

    // Improve the cast type as much as we can without changing the results.
    auto glb = Type::getGreatestLowerBound(curr->castType, refType);
    if (glb != Type::unreachable && glb != curr->castType) {
      curr->castType = glb;
    }

    switch (GCTypeUtils::evaluateCastCheck(refType, curr->castType)) {
      case GCTypeUtils::Unknown:
        break;
      case GCTypeUtils::Success:
        replaceCurrent(builder.makeBlock(
          {builder.makeDrop(curr->ref), builder.makeConst(int32_t(1))}));
        return;
      case GCTypeUtils::Unreachable:
        // Make sure to emit a block with the same type as us, to avoid other
        // code in this pass needing to handle unexpected unreachable code
        // (which is only properly propagated at the end of this pass when we
        // refinalize).
        replaceCurrent(builder.makeBlock(
          {builder.makeDrop(curr->ref), builder.makeUnreachable()}, Type::i32));
        return;
      case GCTypeUtils::Failure:
        replaceCurrent(builder.makeSequence(builder.makeDrop(curr->ref),
                                            builder.makeConst(int32_t(0))));
        return;
      case GCTypeUtils::SuccessOnlyIfNull:
        replaceCurrent(builder.makeRefIsNull(curr->ref));
        return;
      case GCTypeUtils::SuccessOnlyIfNonNull:
        // This adds an EqZ, but code size does not regress since ref.test
        // also encodes a type, and ref.is_null does not. The EqZ may also add
        // some work, but a cast is likely more expensive than a null check +
        // a fast int operation.
        replaceCurrent(
          builder.makeUnary(EqZInt32, builder.makeRefIsNull(curr->ref)));
        return;
    }
  }

  void visitRefIsNull(RefIsNull* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }

    // Optimizing RefIsNull is not that obvious, since even if we know the
    // result evaluates to 0 or 1 then the replacement may not actually save
    // code size, since RefIsNull is a single byte while adding a Const of 0
    // would be two bytes. Other factors are that we can remove the input and
    // the added drop on it if it has no side effects, and that replacing with a
    // constant may allow further optimizations later. For now, replace with a
    // constant, but this warrants more investigation. TODO

    Builder builder(*getModule());
    if (curr->value->type.isNonNullable()) {
      replaceCurrent(
        builder.makeSequence(builder.makeDrop(curr->value),
                             builder.makeConst(Literal::makeZero(Type::i32))));
    } else {
      skipCast(curr->value);
    }
  }

  void visitRefAs(RefAs* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }

    if (curr->op == ExternConvertAny || curr->op == AnyConvertExtern) {
      // These pass nulls through, and we can reorder them with null traps:
      //
      //  (any.convert_extern/extern.convert_any (ref.as_non_null.. ))
      // =>
      //  (ref.as_non_null (any.convert_extern/extern.convert_any ..))
      //
      // By moving the RefAsNonNull outside, it may reach a position where it
      // can be optimized (e.g. if the parent traps anyhow). And,
      // ExternConvertAny/AnyConvertExtern cannot be folded with anything, so
      // there is no harm to moving them inside.
      if (auto* refAsChild = curr->value->dynCast<RefAs>()) {
        if (refAsChild->op == RefAsNonNull) {
          // Reorder and fix up the types.
          curr->value = refAsChild->value;
          curr->finalize();
          refAsChild->value = curr;
          refAsChild->finalize();
          replaceCurrent(refAsChild);
          return;
        }

        // We can optimize away externalizations of internalizations and vice
        // versa.
        if ((curr->op == ExternConvertAny &&
             refAsChild->op == AnyConvertExtern) ||
            (curr->op == AnyConvertExtern &&
             refAsChild->op == ExternConvertAny)) {
          replaceCurrent(refAsChild->value);
          return;
        }
      }
      return;
    }

    assert(curr->op == RefAsNonNull);
    if (trapOnNull(curr, curr->value)) {
      return;
    }
    skipNonNullCast(curr->value, curr);
    if (!curr->value->type.isNullable()) {
      replaceCurrent(curr->value);
      return;
    }

    // As we do in visitRefCast, ref.cast can be combined with ref.as_non_null.
    // This code handles the case where the ref.as is on the outside:
    //
    //   (ref.as_non_null (ref.cast null ..))
    // =>
    //   (ref.cast ..)
    //
    if (auto* cast = curr->value->dynCast<RefCast>()) {
      // The cast cannot be non-nullable, or we would have handled this right
      // above by just removing the ref.as, since it would not be needed.
      assert(!cast->type.isNonNullable());
      cast->type = Type(cast->type.getHeapType(), NonNullable);
      replaceCurrent(cast);
    }
  }

  void visitTupleExtract(TupleExtract* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }

    if (auto* make = curr->tuple->dynCast<TupleMake>()) {
      Builder builder(*getModule());

      // Store the value of the lane we want in a tee, and return that after a
      // drop of the tuple (which might have side effects).
      auto valueType = make->type[curr->index];
      Index tempLocal = builder.addVar(getFunction(), valueType);
      make->operands[curr->index] =
        builder.makeLocalTee(tempLocal, make->operands[curr->index], valueType);
      auto* get = builder.makeLocalGet(tempLocal, valueType);
      replaceCurrent(getDroppedChildrenAndAppend(make, get));
    }
  }

  Index getMaxBitsForLocal(LocalGet* get) {
    // check what we know about the local
    return localInfo[get->index].maxBits;
  }

private:
  // Information about our locals
  std::vector<LocalInfo> localInfo;

  // Checks if the first is a local.tee and the second a local.get of that same
  // index. This is useful in the methods right below us, as it is a common
  // pattern where two consecutive inputs are equal despite being syntactically
  // different.
  bool areMatchingTeeAndGet(Expression* left, Expression* right) {
    if (auto* set = left->dynCast<LocalSet>()) {
      if (auto* get = right->dynCast<LocalGet>()) {
        if (set->isTee() && get->index == set->index) {
          return true;
        }
      }
    }
    return false;
  }

  // Check if two consecutive inputs to an instruction are equal. As they are
  // consecutive, no code can execeute in between them, which simplies the
  // problem here (and which is the case we care about in this pass, which does
  // simple peephole optimizations - all we care about is a single instruction
  // at a time, and its inputs).
  bool areConsecutiveInputsEqual(Expression* left, Expression* right) {
    // When we look for a tee/get pair, we can consider the fallthrough values
    // for the first, as the fallthrough happens last (however, we must use
    // NoTeeBrIf as we do not want to look through the tee). We cannot do this
    // on the second, however, as there could be effects in the middle.
    // TODO: Use effects here perhaps.
    auto& passOptions = getPassOptions();
    left =
      Properties::getFallthrough(left,
                                 passOptions,
                                 *getModule(),
                                 Properties::FallthroughBehavior::NoTeeBrIf);
    if (areMatchingTeeAndGet(left, right)) {
      return true;
    }

    // Ignore extraneous things and compare them syntactically. We can also
    // look at the full fallthrough for both sides now.
    left = Properties::getFallthrough(left, passOptions, *getModule());
    auto* originalRight = right;
    right = Properties::getFallthrough(right, passOptions, *getModule());
    if (!ExpressionAnalyzer::equal(left, right)) {
      return false;
    }

    // We must also not have non-fallthrough effects that invalidate us, such as
    // this situation:
    //
    //  (local.get $x)
    //  (block
    //    (local.set $x ..)
    //    (local.get $x)
    //  )
    //
    // The fallthroughs are identical, but the set may cause us to read a
    // different value.
    if (originalRight != right) {
      // TODO: We could be more precise here and ignore right itself in
      //       originalRightEffects.
      auto originalRightEffects = effects(originalRight);
      auto rightEffects = effects(right);
      if (originalRightEffects.invalidates(rightEffects)) {
        return false;
      }
    }

    // To be equal, they must also be known to return the same result
    // deterministically.
    return !Properties::isGenerative(left);
  }

  // Similar to areConsecutiveInputsEqual() but also checks if we can remove
  // them (but we do not assume the caller will always remove them).
  bool areConsecutiveInputsEqualAndRemovable(Expression* left,
                                             Expression* right) {
    // First, check for side effects. If there are any, then we can't even
    // assume things like local.get's of the same index being identical. (It is
    // also ok to have removable side effects here, see the function
    // description.)
    auto& passOptions = getPassOptions();
    if (EffectAnalyzer(passOptions, *getModule(), left)
          .hasUnremovableSideEffects() ||
        EffectAnalyzer(passOptions, *getModule(), right)
          .hasUnremovableSideEffects()) {
      return false;
    }

    return areConsecutiveInputsEqual(left, right);
  }

  // Check if two consecutive inputs to an instruction are equal and can also be
  // folded into the first of the two (but we do not assume the caller will
  // always fold them). This is similar to areConsecutiveInputsEqualAndRemovable
  // but also identifies reads from the same local variable when the first of
  // them is a "tee" operation and the second is a get (in which case, it is
  // fine to remove the get, but not the tee).
  //
  // The inputs here must be consecutive, but it is also ok to have code with no
  // side effects at all in the middle. For example, a Const in between is ok.
  bool areConsecutiveInputsEqualAndFoldable(Expression* left,
                                            Expression* right) {
    // TODO: We could probably consider fallthrough values for left, at least
    //       (since we fold into it).
    if (areMatchingTeeAndGet(left, right)) {
      return true;
    }

    // stronger property than we need - we can not only fold
    // them but remove them entirely.
    return areConsecutiveInputsEqualAndRemovable(left, right);
  }

  // Canonicalizing the order of a symmetric binary helps us
  // write more concise pattern matching code elsewhere.
  void canonicalize(Binary* binary) {
    assert(shouldCanonicalize(binary));
    auto swap = [&]() {
      assert(canReorder(binary->left, binary->right));
      if (binary->isRelational()) {
        binary->op = reverseRelationalOp(binary->op);
      }
      std::swap(binary->left, binary->right);
    };
    auto maybeSwap = [&]() {
      if (canReorder(binary->left, binary->right)) {
        swap();
      }
    };
    // Prefer a const on the right.
    if (binary->left->is<Const>() && !binary->right->is<Const>()) {
      swap();
    }
    if (auto* c = binary->right->dynCast<Const>()) {
      // x - C   ==>   x + (-C)
      // Prefer use addition if there is a constant on the right.
      if (binary->op == Abstract::getBinary(c->type, Abstract::Sub)) {
        c->value = c->value.neg();
        binary->op = Abstract::getBinary(c->type, Abstract::Add);
        return;
      }
      // Prefer to compare to 0 instead of to -1 or 1.
      // (signed)x > -1   ==>   x >= 0
      if (binary->op == Abstract::getBinary(c->type, Abstract::GtS) &&
          c->value.getInteger() == -1LL) {
        binary->op = Abstract::getBinary(c->type, Abstract::GeS);
        c->value = Literal::makeZero(c->type);
        return;
      }
      // (signed)x <= -1   ==>   x < 0
      if (binary->op == Abstract::getBinary(c->type, Abstract::LeS) &&
          c->value.getInteger() == -1LL) {
        binary->op = Abstract::getBinary(c->type, Abstract::LtS);
        c->value = Literal::makeZero(c->type);
        return;
      }
      // (signed)x < 1   ==>   x <= 0
      if (binary->op == Abstract::getBinary(c->type, Abstract::LtS) &&
          c->value.getInteger() == 1LL) {
        binary->op = Abstract::getBinary(c->type, Abstract::LeS);
        c->value = Literal::makeZero(c->type);
        return;
      }
      // (signed)x >= 1   ==>   x > 0
      if (binary->op == Abstract::getBinary(c->type, Abstract::GeS) &&
          c->value.getInteger() == 1LL) {
        binary->op = Abstract::getBinary(c->type, Abstract::GtS);
        c->value = Literal::makeZero(c->type);
        return;
      }
      // (unsigned)x < 1   ==>   x == 0
      if (binary->op == Abstract::getBinary(c->type, Abstract::LtU) &&
          c->value.getInteger() == 1LL) {
        binary->op = Abstract::getBinary(c->type, Abstract::Eq);
        c->value = Literal::makeZero(c->type);
        return;
      }
      // (unsigned)x >= 1   ==>   x != 0
      if (binary->op == Abstract::getBinary(c->type, Abstract::GeU) &&
          c->value.getInteger() == 1LL) {
        binary->op = Abstract::getBinary(c->type, Abstract::Ne);
        c->value = Literal::makeZero(c->type);
        return;
      }
      // Prefer compare to signed min (s_min) instead of s_min + 1.
      // (signed)x < s_min + 1   ==>   x == s_min
      if (binary->op == LtSInt32 && c->value.geti32() == INT32_MIN + 1) {
        binary->op = EqInt32;
        c->value = Literal::makeSignedMin(Type::i32);
        return;
      }
      if (binary->op == LtSInt64 && c->value.geti64() == INT64_MIN + 1) {
        binary->op = EqInt64;
        c->value = Literal::makeSignedMin(Type::i64);
        return;
      }
      // (signed)x >= s_min + 1   ==>   x != s_min
      if (binary->op == GeSInt32 && c->value.geti32() == INT32_MIN + 1) {
        binary->op = NeInt32;
        c->value = Literal::makeSignedMin(Type::i32);
        return;
      }
      if (binary->op == GeSInt64 && c->value.geti64() == INT64_MIN + 1) {
        binary->op = NeInt64;
        c->value = Literal::makeSignedMin(Type::i64);
        return;
      }
      // Prefer compare to signed max (s_max) instead of s_max - 1.
      // (signed)x > s_max - 1   ==>   x == s_max
      if (binary->op == GtSInt32 && c->value.geti32() == INT32_MAX - 1) {
        binary->op = EqInt32;
        c->value = Literal::makeSignedMax(Type::i32);
        return;
      }
      if (binary->op == GtSInt64 && c->value.geti64() == INT64_MAX - 1) {
        binary->op = EqInt64;
        c->value = Literal::makeSignedMax(Type::i64);
        return;
      }
      // (signed)x <= s_max - 1   ==>   x != s_max
      if (binary->op == LeSInt32 && c->value.geti32() == INT32_MAX - 1) {
        binary->op = NeInt32;
        c->value = Literal::makeSignedMax(Type::i32);
        return;
      }
      if (binary->op == LeSInt64 && c->value.geti64() == INT64_MAX - 1) {
        binary->op = NeInt64;
        c->value = Literal::makeSignedMax(Type::i64);
        return;
      }
      // Prefer compare to unsigned max (u_max) instead of u_max - 1.
      // (unsigned)x <= u_max - 1   ==>   x != u_max
      if (binary->op == Abstract::getBinary(c->type, Abstract::LeU) &&
          c->value.getInteger() == (int64_t)(UINT64_MAX - 1)) {
        binary->op = Abstract::getBinary(c->type, Abstract::Ne);
        c->value = Literal::makeUnsignedMax(c->type);
        return;
      }
      // (unsigned)x > u_max - 1   ==>   x == u_max
      if (binary->op == Abstract::getBinary(c->type, Abstract::GtU) &&
          c->value.getInteger() == (int64_t)(UINT64_MAX - 1)) {
        binary->op = Abstract::getBinary(c->type, Abstract::Eq);
        c->value = Literal::makeUnsignedMax(c->type);
        return;
      }
      return;
    }
    // Prefer a get on the right.
    if (binary->left->is<LocalGet>() && !binary->right->is<LocalGet>()) {
      return maybeSwap();
    }
    // Sort by the node id type, if different.
    if (binary->left->_id != binary->right->_id) {
      if (binary->left->_id > binary->right->_id) {
        return maybeSwap();
      }
      return;
    }
    // If the children have the same node id, we have to go deeper.
    if (auto* left = binary->left->dynCast<Unary>()) {
      auto* right = binary->right->cast<Unary>();
      if (left->op > right->op) {
        return maybeSwap();
      }
    }
    if (auto* left = binary->left->dynCast<Binary>()) {
      auto* right = binary->right->cast<Binary>();
      if (left->op > right->op) {
        return maybeSwap();
      }
    }
    if (auto* left = binary->left->dynCast<LocalGet>()) {
      auto* right = binary->right->cast<LocalGet>();
      if (left->index > right->index) {
        return maybeSwap();
      }
    }
  }

  // Optimize given that the expression is flowing into a boolean context
  Expression* optimizeBoolean(Expression* boolean) {
    // TODO use a general getFallthroughs
    if (auto* unary = boolean->dynCast<Unary>()) {
      if (unary) {
        if (unary->op == EqZInt32) {
          auto* unary2 = unary->value->dynCast<Unary>();
          if (unary2 && unary2->op == EqZInt32) {
            // double eqz
            return unary2->value;
          }
          if (auto* binary = unary->value->dynCast<Binary>()) {
            // !(x <=> y)   ==>   x <!=> y
            auto op = invertBinaryOp(binary->op);
            if (op != InvalidBinary) {
              binary->op = op;
              return binary;
            }
          }
        }
      }
    } else if (auto* binary = boolean->dynCast<Binary>()) {
      if (binary->op == SubInt32) {
        if (auto* c = binary->left->dynCast<Const>()) {
          if (c->value.geti32() == 0) {
            // bool(0 - x)   ==>   bool(x)
            return binary->right;
          }
        }
      } else if (binary->op == OrInt32) {
        // an or flowing into a boolean context can consider each input as
        // boolean
        binary->left = optimizeBoolean(binary->left);
        binary->right = optimizeBoolean(binary->right);
      } else if (binary->op == NeInt32) {
        if (auto* c = binary->right->dynCast<Const>()) {
          // x != 0 is just x if it's used as a bool
          if (c->value.geti32() == 0) {
            return binary->left;
          }
          // TODO: Perhaps use it for separate final pass???
          // x != -1   ==>    x ^ -1
          // if (num->value.geti32() == -1) {
          //   binary->op = XorInt32;
          //   return binary;
          // }
        }
      } else if (binary->op == RemSInt32) {
        // bool(i32(x) % C_pot)  ==>  bool(x & (C_pot - 1))
        // bool(i32(x) % min_s)  ==>  bool(x & max_s)
        if (auto* c = binary->right->dynCast<Const>()) {
          if (c->value.isSignedMin() ||
              Bits::isPowerOf2(c->value.abs().geti32())) {
            binary->op = AndInt32;
            if (c->value.isSignedMin()) {
              c->value = Literal::makeSignedMax(Type::i32);
            } else {
              c->value = c->value.abs().sub(Literal::makeOne(Type::i32));
            }
            return binary;
          }
        }
      }
      if (auto* ext = Properties::getSignExtValue(binary)) {
        // use a cheaper zero-extent, we just care about the boolean value
        // anyhow
        return makeZeroExt(ext, Properties::getSignExtBits(binary));
      }
    } else if (auto* block = boolean->dynCast<Block>()) {
      if (block->type == Type::i32 && block->list.size() > 0) {
        block->list.back() = optimizeBoolean(block->list.back());
      }
    } else if (auto* iff = boolean->dynCast<If>()) {
      if (iff->type == Type::i32) {
        iff->ifTrue = optimizeBoolean(iff->ifTrue);
        iff->ifFalse = optimizeBoolean(iff->ifFalse);
      }
    } else if (auto* select = boolean->dynCast<Select>()) {
      select->ifTrue = optimizeBoolean(select->ifTrue);
      select->ifFalse = optimizeBoolean(select->ifFalse);
    } else if (auto* tryy = boolean->dynCast<Try>()) {
      if (tryy->type == Type::i32) {
        tryy->body = optimizeBoolean(tryy->body);
        for (Index i = 0; i < tryy->catchBodies.size(); i++) {
          tryy->catchBodies[i] = optimizeBoolean(tryy->catchBodies[i]);
        }
      }
    }
    // TODO: recurse into br values?
    return boolean;
  }

  Expression* optimizeSelect(Select* curr) {
    using namespace Match;
    using namespace Abstract;
    Builder builder(*getModule());
    curr->condition = optimizeBoolean(curr->condition);
    {
      // Constant condition, we can just pick the correct side (barring side
      // effects)
      Expression *ifTrue, *ifFalse;
      if (matches(curr, select(pure(&ifTrue), any(&ifFalse), i32(0)))) {
        return ifFalse;
      }
      if (matches(curr, select(any(&ifTrue), any(&ifFalse), i32(0)))) {
        return builder.makeSequence(builder.makeDrop(ifTrue), ifFalse);
      }
      int32_t cond;
      if (matches(curr, select(any(&ifTrue), pure(&ifFalse), i32(&cond)))) {
        // The condition must be non-zero because a zero would have matched one
        // of the previous patterns.
        assert(cond != 0);
        return ifTrue;
      }
      // Don't bother when `ifFalse` isn't pure - we would need to reverse the
      // order using a temp local, which would be bad
    }
    {
      // TODO: Remove this after landing SCCP pass. See: #4161

      // i32(x) ? i32(x) : 0  ==>  x
      Expression *x, *y;
      if (matches(curr, select(any(&x), i32(0), any(&y))) &&
          areConsecutiveInputsEqualAndFoldable(x, y)) {
        return curr->ifTrue;
      }
      // i32(x) ? 0 : i32(x)  ==>  { x, 0 }
      if (matches(curr, select(i32(0), any(&x), any(&y))) &&
          areConsecutiveInputsEqualAndFoldable(x, y)) {
        return builder.makeSequence(builder.makeDrop(x), curr->ifTrue);
      }

      // i64(x) == 0 ? 0 : i64(x)  ==>  x
      // i64(x) != 0 ? i64(x) : 0  ==>  x
      if ((matches(curr, select(i64(0), any(&x), unary(EqZInt64, any(&y)))) ||
           matches(
             curr,
             select(any(&x), i64(0), binary(NeInt64, any(&y), i64(0))))) &&
          areConsecutiveInputsEqualAndFoldable(x, y)) {
        return curr->condition->is<Unary>() ? curr->ifFalse : curr->ifTrue;
      }

      // i64(x) == 0 ? i64(x) : 0  ==>  { x, 0 }
      // i64(x) != 0 ? 0 : i64(x)  ==>  { x, 0 }
      if ((matches(curr, select(any(&x), i64(0), unary(EqZInt64, any(&y)))) ||
           matches(
             curr,
             select(i64(0), any(&x), binary(NeInt64, any(&y), i64(0))))) &&
          areConsecutiveInputsEqualAndFoldable(x, y)) {
        return builder.makeSequence(
          builder.makeDrop(x),
          curr->condition->is<Unary>() ? curr->ifFalse : curr->ifTrue);
      }
    }
    {
      // Simplify selects between 0 and 1
      Expression* c;
      bool reversed = matches(curr, select(ival(0), ival(1), any(&c)));
      if (reversed || matches(curr, select(ival(1), ival(0), any(&c)))) {
        if (reversed) {
          c = optimizeBoolean(builder.makeUnary(EqZInt32, c));
        }
        if (!Properties::emitsBoolean(c)) {
          // cond ? 1 : 0 ==> !!cond
          c = builder.makeUnary(EqZInt32, builder.makeUnary(EqZInt32, c));
        }
        return curr->type == Type::i64 ? builder.makeUnary(ExtendUInt32, c) : c;
      }
    }
    // Flip the arms if doing so might help later optimizations here.
    if (auto* binary = curr->condition->dynCast<Binary>()) {
      auto inv = invertBinaryOp(binary->op);
      if (inv != InvalidBinary) {
        // For invertible binary operations, we prefer to have non-zero values
        // in the ifTrue, and zero values in the ifFalse, due to the
        // optimization right after us. Even if this does not help there, it is
        // a nice canonicalization. (To ensure convergence - that we don't keep
        // doing work each time we get here - do nothing if both are zero, or
        // if both are nonzero.)
        Const* c;
        if ((matches(curr->ifTrue, ival(0)) &&
             !matches(curr->ifFalse, ival(0))) ||
            (!matches(curr->ifTrue, ival()) &&
             matches(curr->ifFalse, ival(&c)) && !c->value.isZero())) {
          binary->op = inv;
          std::swap(curr->ifTrue, curr->ifFalse);
        }
      }
    }
    if (curr->type == Type::i32 &&
        Bits::getMaxBits(curr->condition, this) <= 1 &&
        Bits::getMaxBits(curr->ifTrue, this) <= 1 &&
        Bits::getMaxBits(curr->ifFalse, this) <= 1) {
      // The condition and both arms are i32 booleans, which allows us to do
      // boolean optimizations.
      Expression* x;
      Expression* y;

      // x ? y : 0   ==>   x & y
      if (matches(curr, select(any(&y), ival(0), any(&x)))) {
        return builder.makeBinary(AndInt32, y, x);
      }

      // x ? 1 : y   ==>   x | y
      if (matches(curr, select(ival(1), any(&y), any(&x)))) {
        return builder.makeBinary(OrInt32, y, x);
      }
    }
    {
      // Simplify x < 0 ? -1 : 1 or x >= 0 ? 1 : -1 to
      // i32(x) >> 31 | 1
      // i64(x) >> 63 | 1
      Binary* bin;
      if (matches(
            curr,
            select(ival(-1), ival(1), binary(&bin, LtS, any(), ival(0)))) ||
          matches(
            curr,
            select(ival(1), ival(-1), binary(&bin, GeS, any(), ival(0))))) {
        auto c = bin->right->cast<Const>();
        auto type = curr->ifTrue->type;
        if (type == c->type) {
          bin->type = type;
          bin->op = Abstract::getBinary(type, ShrS);
          c->value = Literal::makeFromInt32(type.getByteSize() * 8 - 1, type);
          curr->ifTrue->cast<Const>()->value = Literal::makeOne(type);
          return builder.makeBinary(
            Abstract::getBinary(type, Or), bin, curr->ifTrue);
        }
      }
    }
    {
      // Flip select to remove eqz if we can reorder
      Select* s;
      Expression *ifTrue, *ifFalse, *c;
      if (matches(
            curr,
            select(
              &s, any(&ifTrue), any(&ifFalse), unary(EqZInt32, any(&c)))) &&
          canReorder(ifTrue, ifFalse)) {
        s->ifTrue = ifFalse;
        s->ifFalse = ifTrue;
        s->condition = c;
        return s;
      }
    }
    {
      // Sides are identical, fold
      Expression *ifTrue, *ifFalse, *c;
      if (matches(curr, select(any(&ifTrue), any(&ifFalse), any(&c))) &&
          ExpressionAnalyzer::equal(ifTrue, ifFalse)) {
        auto value = effects(ifTrue);
        if (value.hasSideEffects()) {
          // At best we don't need the condition, but need to execute the
          // value twice. a block is larger than a select by 2 bytes, and we
          // must drop one value, so 3, while we save the condition, so it's
          // not clear this is worth it, TODO
        } else {
          // The value has no side effects, so we can replace ourselves with one
          // of the two identical values in the arms.
          auto condition = effects(c);
          if (!condition.hasSideEffects()) {
            return ifTrue;
          } else {
            // The condition is last, so we need a new local, and it may be a
            // bad idea to use a block like we do for an if. Do it only if we
            // can reorder
            if (!condition.invalidates(value)) {
              return builder.makeSequence(builder.makeDrop(c), ifTrue);
            }
          }
        }
      }
    }
    return nullptr;
  }

  // find added constants in an expression tree, including multiplied/shifted,
  // and combine them note that we ignore division/shift-right, as rounding
  // makes this nonlinear, so not a valid opt
  Expression* optimizeAddedConstants(Binary* binary) {
    assert(binary->type.isInteger() || binary->type.isVector());

    uint64_t constant = 0;
    std::vector<Const*> constants;

    struct SeekState {
      Expression* curr;
      uint64_t mul;
      SeekState(Expression* curr, uint64_t mul) : curr(curr), mul(mul) {}
    };
    std::vector<SeekState> seekStack;
    seekStack.emplace_back(binary, 1);
    while (!seekStack.empty()) {
      auto state = seekStack.back();
      seekStack.pop_back();
      auto curr = state.curr;
      auto mul = state.mul;
      if (auto* c = curr->dynCast<Const>()) {
        uint64_t value = c->value.getInteger();
        if (value != 0ULL) {
          constant += value * mul;
          constants.push_back(c);
        }
        continue;
      } else if (auto* binary = curr->dynCast<Binary>()) {
        if (binary->op == Abstract::getBinary(binary->type, Abstract::Add)) {
          seekStack.emplace_back(binary->right, mul);
          seekStack.emplace_back(binary->left, mul);
          continue;
        } else if (binary->op ==
                   Abstract::getBinary(binary->type, Abstract::Sub)) {
          // if the left is a zero, ignore it, it's how we negate ints
          auto* left = binary->left->dynCast<Const>();
          seekStack.emplace_back(binary->right, -mul);
          if (!left || !left->value.isZero()) {
            seekStack.emplace_back(binary->left, mul);
          }
          continue;
        } else if (binary->op ==
                   Abstract::getBinary(binary->type, Abstract::Shl)) {
          if (auto* c = binary->right->dynCast<Const>()) {
            seekStack.emplace_back(binary->left,
                                   mul << Bits::getEffectiveShifts(c));
            continue;
          }
        } else if (binary->op ==
                   Abstract::getBinary(binary->type, Abstract::Mul)) {
          if (auto* c = binary->left->dynCast<Const>()) {
            seekStack.emplace_back(binary->right,
                                   mul * (uint64_t)c->value.getInteger());
            continue;
          } else if (auto* c = binary->right->dynCast<Const>()) {
            seekStack.emplace_back(binary->left,
                                   mul * (uint64_t)c->value.getInteger());
            continue;
          }
        }
      }
    };
    // find all factors
    if (constants.size() <= 1) {
      // nothing much to do, except for the trivial case of adding/subbing a
      // zero
      if (auto* c = binary->right->dynCast<Const>()) {
        if (c->value.isZero()) {
          return binary->left;
        }
      }
      return nullptr;
    }
    // wipe out all constants, we'll replace with a single added one
    for (auto* c : constants) {
      c->value = Literal::makeZero(c->type);
    }
    // remove added/subbed zeros
    struct ZeroRemover : public PostWalker<ZeroRemover> {
      // TODO: we could save the binarys and costs we drop, and reuse them later

      PassOptions& passOptions;

      ZeroRemover(PassOptions& passOptions) : passOptions(passOptions) {}

      void visitBinary(Binary* curr) {
        if (!curr->type.isInteger()) {
          return;
        }
        auto type = curr->type;
        auto* left = curr->left->dynCast<Const>();
        auto* right = curr->right->dynCast<Const>();
        // Canonicalization prefers an add instead of a subtract wherever
        // possible. That prevents a subtracted constant on the right,
        // as it would be added. And for a zero on the left, it can't be
        // removed (it is how we negate ints).
        if (curr->op == Abstract::getBinary(type, Abstract::Add)) {
          if (left && left->value.isZero()) {
            replaceCurrent(curr->right);
            return;
          }
          if (right && right->value.isZero()) {
            replaceCurrent(curr->left);
            return;
          }
        } else if (curr->op == Abstract::getBinary(type, Abstract::Shl)) {
          // shifting a 0 is a 0, or anything by 0 has no effect, all unless the
          // shift has side effects
          if (((left && left->value.isZero()) ||
               (right && Bits::getEffectiveShifts(right) == 0)) &&
              !EffectAnalyzer(passOptions, *getModule(), curr->right)
                 .hasSideEffects()) {
            replaceCurrent(curr->left);
            return;
          }
        } else if (curr->op == Abstract::getBinary(type, Abstract::Mul)) {
          // multiplying by zero is a zero, unless the other side has side
          // effects
          if (left && left->value.isZero() &&
              !EffectAnalyzer(passOptions, *getModule(), curr->right)
                 .hasSideEffects()) {
            replaceCurrent(left);
            return;
          }
          if (right && right->value.isZero() &&
              !EffectAnalyzer(passOptions, *getModule(), curr->left)
                 .hasSideEffects()) {
            replaceCurrent(right);
            return;
          }
        }
      }
    };
    // Noting the type here not only simplifies the code below, but is also
    // necessary to avoid an error: if we look at walked->type then it may
    // actually differ from the original type, say if the walk ended up turning
    // |binary| into a simpler unreachable expression.
    auto type = binary->type;
    Expression* walked = binary;
    ZeroRemover remover(getPassOptions());
    remover.setModule(getModule());
    remover.walk(walked);
    if (constant == 0ULL) {
      return walked; // nothing more to do
    }
    // Add the total constant value we computed to the value remaining here.
    // Note that if the value is 32 bits then |makeFromInt64| will wrap to 32
    // bits for us; as all the operations before us and the add below us are
    // adds and subtracts, any overflow is not a problem.
    auto toAdd = Literal::makeFromInt64(constant, type);
    if (auto* c = walked->dynCast<Const>()) {
      // This is a constant, so just add it immediately (we could also leave
      // this for Precompute, in principle).
      c->value = c->value.add(toAdd);
      return c;
    }
    Builder builder(*getModule());
    return builder.makeBinary(Abstract::getBinary(type, Abstract::Add),
                              walked,
                              builder.makeConst(toAdd));
  }

  // Given an i64.wrap operation, see if we can remove it. If all the things
  // being operated on behave the same with or without wrapping, then we don't
  // need to go to 64 bits at all, e.g.:
  //
  //  int32_t(int64_t(x))               => x                 (extend, then wrap)
  //  int32_t(int64_t(x) + int64_t(10)) => x + int32_t(10)            (also add)
  //
  Expression* optimizeWrappedResult(Unary* wrap) {
    assert(wrap->op == WrapInt64);

    // Core processing logic. This goes through the children, in one of two
    // modes:
    //  * Scan: Find if there is anything we can't handle. Sets |canOptimize|
    //    with what it finds.
    //  * Optimize: Given we can handle everything, update things.
    enum Mode { Scan, Optimize };
    bool canOptimize = true;
    auto processChildren = [&](Mode mode) {
      // Use a simple stack as we go through the children. We use ** as we need
      // to replace children for some optimizations.
      SmallVector<Expression**, 2> stack;
      stack.emplace_back(&wrap->value);

      while (!stack.empty() && canOptimize) {
        auto* currp = stack.back();
        stack.pop_back();
        auto* curr = *currp;
        if (curr->type == Type::unreachable) {
          // Leave unreachability for other passes.
          canOptimize = false;
          return;
        } else if (auto* c = curr->dynCast<Const>()) {
          // A i64 const can be handled by just turning it into an i32.
          if (mode == Optimize) {
            c->value = Literal(int32_t(c->value.getInteger()));
            c->type = Type::i32;
          }
        } else if (auto* unary = curr->dynCast<Unary>()) {
          switch (unary->op) {
            case ExtendSInt32:
            case ExtendUInt32: {
              // Note that there is nothing to push to the stack here: the child
              // is 32-bit already, so we can stop looking. We just need to skip
              // the extend operation.
              if (mode == Optimize) {
                *currp = unary->value;
              }
              break;
            }
            default: {
              // TODO: handle more cases here and below,
              //       https://github.com/WebAssembly/binaryen/issues/5004
              canOptimize = false;
              return;
            }
          }
        } else if (auto* binary = curr->dynCast<Binary>()) {
          // Turn the binary into a 32-bit one, if we can.
          switch (binary->op) {
            case AddInt64:
            case SubInt64:
            case MulInt64: {
              // We can optimize these.
              break;
            }
            default: {
              canOptimize = false;
              return;
            }
          }
          if (mode == Optimize) {
            switch (binary->op) {
              case AddInt64: {
                binary->op = AddInt32;
                break;
              }
              case SubInt64: {
                binary->op = SubInt32;
                break;
              }
              case MulInt64: {
                binary->op = MulInt32;
                break;
              }
              default: {
                WASM_UNREACHABLE("bad op");
              }
            }
            // All things we can optimize change the type to i32.
            binary->type = Type::i32;
          }
          stack.push_back(&binary->left);
          stack.push_back(&binary->right);
        } else {
          // Anything else makes us give up.
          canOptimize = false;
          return;
        }
      }
    };

    processChildren(Scan);
    if (!canOptimize) {
      return nullptr;
    }

    // Optimize, and return the optimized results (in which we no longer need
    // the wrap operation itself).
    processChildren(Optimize);
    return wrap->value;
  }

  //   expensive1 | expensive2 can be turned into expensive1 ? 1 : expensive2,
  //   and expensive | cheap     can be turned into cheap     ? 1 : expensive,
  // so that we can avoid one expensive computation, if it has no side effects.
  Expression* conditionalizeExpensiveOnBitwise(Binary* binary) {
    // this operation can increase code size, so don't always do it
    auto& options = getPassRunner()->options;
    if (options.optimizeLevel < 2 || options.shrinkLevel > 0) {
      return nullptr;
    }
    const auto MIN_COST = 7;
    assert(binary->op == AndInt32 || binary->op == OrInt32);
    if (binary->right->is<Const>()) {
      return nullptr; // trivial
    }
    // bitwise logical operator on two non-numerical values, check if they are
    // boolean
    auto* left = binary->left;
    auto* right = binary->right;
    if (!Properties::emitsBoolean(left) || !Properties::emitsBoolean(right)) {
      return nullptr;
    }
    auto leftEffects = effects(left);
    auto rightEffects = effects(right);
    auto leftHasSideEffects = leftEffects.hasSideEffects();
    auto rightHasSideEffects = rightEffects.hasSideEffects();
    if (leftHasSideEffects && rightHasSideEffects) {
      return nullptr; // both must execute
    }
    // canonicalize with side effects, if any, happening on the left
    if (rightHasSideEffects) {
      if (CostAnalyzer(left).cost < MIN_COST) {
        return nullptr; // avoidable code is too cheap
      }
      if (leftEffects.invalidates(rightEffects)) {
        return nullptr; // cannot reorder
      }
      std::swap(left, right);
    } else if (leftHasSideEffects) {
      if (CostAnalyzer(right).cost < MIN_COST) {
        return nullptr; // avoidable code is too cheap
      }
    } else {
      // no side effects, reorder based on cost estimation
      auto leftCost = CostAnalyzer(left).cost;
      auto rightCost = CostAnalyzer(right).cost;
      if (std::max(leftCost, rightCost) < MIN_COST) {
        return nullptr; // avoidable code is too cheap
      }
      // canonicalize with expensive code on the right
      if (leftCost > rightCost) {
        std::swap(left, right);
      }
    }
    // worth it! perform conditionalization
    Builder builder(*getModule());
    if (binary->op == OrInt32) {
      return builder.makeIf(
        left, builder.makeConst(Literal(int32_t(1))), right);
    } else { // &
      return builder.makeIf(
        left, right, builder.makeConst(Literal(int32_t(0))));
    }
  }

  // We can combine `and` operations, e.g.
  //   (x == 0) & (y == 0)   ==>    (x | y) == 0
  Expression* combineAnd(Binary* curr) {
    assert(curr->op == AndInt32);

    using namespace Abstract;
    using namespace Match;

    {
      // (i32(x) == 0) & (i32(y) == 0)   ==>   i32(x | y) == 0
      // (i64(x) == 0) & (i64(y) == 0)   ==>   i64(x | y) == 0
      Expression *x, *y;
      if (matches(curr->left, unary(EqZ, any(&x))) &&
          matches(curr->right, unary(EqZ, any(&y))) && x->type == y->type) {
        auto* inner = curr->left->cast<Unary>();
        inner->value =
          Builder(*getModule()).makeBinary(getBinary(x->type, Or), x, y);
        return inner;
      }
    }
    {
      // Binary operations that inverse a bitwise AND can be
      // reordered. If F(x) = binary(x, c), and F(x) preserves AND,
      // that is,
      //
      //   F(x) & F(y) == F(x | y)
      //
      // Then also
      //
      //   binary(x, c) & binary(y, c)  =>  binary(x | y, c)
      Binary *bx, *by;
      Expression *x, *y;
      Const *cx, *cy;
      if (matches(curr->left, binary(&bx, any(&x), ival(&cx))) &&
          matches(curr->right, binary(&by, any(&y), ival(&cy))) &&
          bx->op == by->op && x->type == y->type && cx->value == cy->value &&
          inversesAnd(bx)) {
        by->op = getBinary(x->type, Or);
        by->type = x->type;
        by->left = x;
        by->right = y;
        bx->left = by;
        return bx;
      }
    }
    {
      // Binary operations that preserve a bitwise AND can be
      // reordered. If F(x) = binary(x, c), and F(x) preserves AND,
      // that is,
      //
      //   F(x) & F(y) == F(x & y)
      //
      // Then also
      //
      //   binary(x, c) & binary(y, c)  =>  binary(x & y, c)
      Binary *bx, *by;
      Expression *x, *y;
      Const *cx, *cy;
      if (matches(curr->left, binary(&bx, any(&x), ival(&cx))) &&
          matches(curr->right, binary(&by, any(&y), ival(&cy))) &&
          bx->op == by->op && x->type == y->type && cx->value == cy->value &&
          preserveAnd(bx)) {
        by->op = getBinary(x->type, And);
        by->type = x->type;
        by->left = x;
        by->right = y;
        bx->left = by;
        return bx;
      }
    }
    return nullptr;
  }

  // We can combine `or` operations, e.g.
  //   (x > y)  | (x == y)    ==>    x >= y
  //   (x != 0) | (y != 0)    ==>    (x | y) != 0
  Expression* combineOr(Binary* curr) {
    assert(curr->op == OrInt32);

    using namespace Abstract;
    using namespace Match;

    if (auto* left = curr->left->dynCast<Binary>()) {
      if (auto* right = curr->right->dynCast<Binary>()) {
        if (left->op != right->op &&
            ExpressionAnalyzer::equal(left->left, right->left) &&
            ExpressionAnalyzer::equal(left->right, right->right) &&
            !effects(left->left).hasSideEffects() &&
            !effects(left->right).hasSideEffects()) {
          switch (left->op) {
            //   (x > y) | (x == y)    ==>    x >= y
            case EqInt32: {
              if (right->op == GtSInt32) {
                left->op = GeSInt32;
                return left;
              }
              break;
            }
            default: {
            }
          }
        }
      }
    }
    {
      // Binary operations that inverses a bitwise OR to AND.
      // If F(x) = binary(x, c), and F(x) inverses OR,
      // that is,
      //
      //   F(x) | F(y) == F(x & y)
      //
      // Then also
      //
      //   binary(x, c) | binary(y, c)  =>  binary(x & y, c)
      Binary *bx, *by;
      Expression *x, *y;
      Const *cx, *cy;
      if (matches(curr->left, binary(&bx, any(&x), ival(&cx))) &&
          matches(curr->right, binary(&by, any(&y), ival(&cy))) &&
          bx->op == by->op && x->type == y->type && cx->value == cy->value &&
          inversesOr(bx)) {
        by->op = getBinary(x->type, And);
        by->type = x->type;
        by->left = x;
        by->right = y;
        bx->left = by;
        return bx;
      }
    }
    {
      // Binary operations that preserve a bitwise OR can be
      // reordered. If F(x) = binary(x, c), and F(x) preserves OR,
      // that is,
      //
      //   F(x) | F(y) == F(x | y)
      //
      // Then also
      //
      //   binary(x, c) | binary(y, c)  =>  binary(x | y, c)
      Binary *bx, *by;
      Expression *x, *y;
      Const *cx, *cy;
      if (matches(curr->left, binary(&bx, any(&x), ival(&cx))) &&
          matches(curr->right, binary(&by, any(&y), ival(&cy))) &&
          bx->op == by->op && x->type == y->type && cx->value == cy->value &&
          preserveOr(bx)) {
        by->op = getBinary(x->type, Or);
        by->type = x->type;
        by->left = x;
        by->right = y;
        bx->left = by;
        return bx;
      }
    }
    return nullptr;
  }

  // Check whether an operation preserves the Or operation through it, that is,
  //
  //   F(x | y) = F(x) | F(y)
  //
  // Mathematically that means F is homomorphic with respect to the | operation.
  //
  // F(x) is seen as taking a single parameter of its first child. That is, the
  // first child is |x|, and the rest is constant. For example, if we are given
  // a binary with operation != and the right child is a constant 0, then
  // F(x) = (x != 0).
  bool preserveOr(Binary* curr) {
    using namespace Abstract;
    using namespace Match;

    // (x != 0) | (y != 0)    ==>    (x | y) != 0
    // This effectively checks if any bits are set in x or y.
    if (matches(curr, binary(Ne, any(), ival(0)))) {
      return true;
    }
    // (x < 0) | (y < 0)    ==>    (x | y) < 0
    // This effectively checks if x or y have the sign bit set.
    if (matches(curr, binary(LtS, any(), ival(0)))) {
      return true;
    }
    return false;
  }

  // Check whether an operation inverses the Or operation to And, that is,
  //
  //   F(x | y) = F(x) & F(y)
  //
  // Mathematically that means F is homomorphic with respect to the | operation.
  //
  // F(x) is seen as taking a single parameter of its first child. That is, the
  // first child is |x|, and the rest is constant. For example, if we are given
  // a binary with operation != and the right child is a constant 0, then
  // F(x) = (x != 0).
  bool inversesOr(Binary* curr) {
    using namespace Abstract;
    using namespace Match;

    // (x >= 0) | (y >= 0)   ==>   (x & y) >= 0
    if (matches(curr, binary(GeS, any(), ival(0)))) {
      return true;
    }

    // (x !=-1) | (y !=-1)   ==>   (x & y) !=-1
    if (matches(curr, binary(Ne, any(), ival(-1)))) {
      return true;
    }

    return false;
  }

  // Check whether an operation preserves the And operation through it, that is,
  //
  //   F(x & y) = F(x) & F(y)
  //
  // Mathematically that means F is homomorphic with respect to the & operation.
  //
  // F(x) is seen as taking a single parameter of its first child. That is, the
  // first child is |x|, and the rest is constant. For example, if we are given
  // a binary with operation != and the right child is a constant 0, then
  // F(x) = (x != 0).
  bool preserveAnd(Binary* curr) {
    using namespace Abstract;
    using namespace Match;

    // (x < 0) & (y < 0)   ==>   (x & y) < 0
    if (matches(curr, binary(LtS, any(), ival(0)))) {
      return true;
    }

    // (x == -1) & (y == -1)   ==>   (x & y) == -1
    if (matches(curr, binary(Eq, any(), ival(-1)))) {
      return true;
    }

    return false;
  }

  // Check whether an operation inverses the And operation to Or, that is,
  //
  //   F(x & y) = F(x) | F(y)
  //
  // Mathematically that means F is homomorphic with respect to the & operation.
  //
  // F(x) is seen as taking a single parameter of its first child. That is, the
  // first child is |x|, and the rest is constant. For example, if we are given
  // a binary with operation != and the right child is a constant 0, then
  // F(x) = (x != 0).
  bool inversesAnd(Binary* curr) {
    using namespace Abstract;
    using namespace Match;

    // (x >= 0) & (y >= 0)   ==>   (x | y) >= 0
    if (matches(curr, binary(GeS, any(), ival(0)))) {
      return true;
    }

    return false;
  }

  // fold constant factors into the offset
  void optimizeMemoryAccess(Expression*& ptr, Address& offset, Name memory) {
    // ptr may be a const, but it isn't worth folding that in (we still have a
    // const); in fact, it's better to do the opposite for gzip purposes as well
    // as for readability.
    auto* last = ptr->dynCast<Const>();
    if (last) {
      uint64_t value64 = last->value.getInteger();
      uint64_t offset64 = offset;
      auto mem = getModule()->getMemory(memory);
      if (mem->is64()) {
        // Check for a 64-bit overflow.
        uint64_t sum;
        if (!std::ckd_add(&sum, value64, offset64)) {
          last->value = Literal(int64_t(sum));
          offset = 0;
        }
      } else {
        // don't do this if it would wrap the pointer
        if (value64 <= uint64_t(std::numeric_limits<int32_t>::max()) &&
            offset64 <= uint64_t(std::numeric_limits<int32_t>::max()) &&
            value64 + offset64 <=
              uint64_t(std::numeric_limits<int32_t>::max())) {
          last->value = Literal(int32_t(value64 + offset64));
          offset = 0;
        }
      }
    }
  }

  // Optimize a multiply by a power of two on the right, which
  // can be a shift.
  // This doesn't shrink code size, and VMs likely optimize it anyhow,
  // but it's still worth doing since
  //  * Often shifts are more common than muls.
  //  * The constant is smaller.
  template<typename T> Expression* optimizePowerOf2Mul(Binary* binary, T c) {
    static_assert(std::is_same<T, uint32_t>::value ||
                    std::is_same<T, uint64_t>::value,
                  "type mismatch");
    auto shifts = Bits::countTrailingZeroes(c);
    binary->op = std::is_same<T, uint32_t>::value ? ShlInt32 : ShlInt64;
    binary->right->cast<Const>()->value = Literal(static_cast<T>(shifts));
    return binary;
  }

  // Optimize an unsigned divide / remainder by a power of two on the right
  // This doesn't shrink code size, and VMs likely optimize it anyhow,
  // but it's still worth doing since
  //  * Usually ands are more common than urems.
  //  * The constant is slightly smaller.
  template<typename T> Expression* optimizePowerOf2URem(Binary* binary, T c) {
    static_assert(std::is_same<T, uint32_t>::value ||
                    std::is_same<T, uint64_t>::value,
                  "type mismatch");
    binary->op = std::is_same<T, uint32_t>::value ? AndInt32 : AndInt64;
    binary->right->cast<Const>()->value = Literal(c - 1);
    return binary;
  }

  template<typename T> Expression* optimizePowerOf2UDiv(Binary* binary, T c) {
    static_assert(std::is_same<T, uint32_t>::value ||
                    std::is_same<T, uint64_t>::value,
                  "type mismatch");
    auto shifts = Bits::countTrailingZeroes(c);
    binary->op = std::is_same<T, uint32_t>::value ? ShrUInt32 : ShrUInt64;
    binary->right->cast<Const>()->value = Literal(static_cast<T>(shifts));
    return binary;
  }

  template<typename T> Expression* optimizePowerOf2FDiv(Binary* binary, T c) {
    //
    // x / C_pot    =>   x * (C_pot ^ -1)
    //
    // Explanation:
    // Floating point numbers are represented as:
    //    ((-1) ^ sign) * (2 ^ (exp - bias)) * (1 + significand)
    //
    // If we have power of two numbers, then the mantissa (significand)
    // is all zeros. Let's focus on the exponent, ignoring the sign part:
    //    (2 ^ (exp - bias))
    //
    // and for inverted power of two floating point:
    //     1.0 / (2 ^ (exp - bias))   ->   2 ^ -(exp - bias)
    //
    // So inversion of C_pot is valid because it changes only the sign
    // of the exponent part and doesn't touch the significand part,
    // which remains the same (zeros).
    static_assert(std::is_same<T, float>::value ||
                    std::is_same<T, double>::value,
                  "type mismatch");
    double invDivisor = 1.0 / (double)c;
    binary->op = std::is_same<T, float>::value ? MulFloat32 : MulFloat64;
    binary->right->cast<Const>()->value = Literal(static_cast<T>(invDivisor));
    return binary;
  }

  Expression* makeZeroExt(Expression* curr, int32_t bits) {
    Builder builder(*getModule());
    return builder.makeBinary(
      AndInt32, curr, builder.makeConst(Literal(Bits::lowBitMask(bits))));
  }

  // given an "almost" sign extend - either a proper one, or it
  // has too many shifts left - we remove the sign extend. If there are
  // too many shifts, we split the shifts first, so this removes the
  // two sign extend shifts and adds one (smaller one)
  Expression* removeAlmostSignExt(Binary* outer) {
    auto* inner = outer->left->cast<Binary>();
    auto* outerConst = outer->right->cast<Const>();
    auto* innerConst = inner->right->cast<Const>();
    auto* value = inner->left;
    if (outerConst->value == innerConst->value) {
      return value;
    }
    // add a shift, by reusing the existing node
    innerConst->value = innerConst->value.sub(outerConst->value);
    return inner;
  }

  // Check if an expression is already sign-extended to an exact number of bits.
  bool isSignExted(Expression* curr, Index bits) {
    return getSignExtBits(curr) == bits;
  }

  // Returns the number of bits an expression is sign-extended (or 0 if it is
  // not).
  Index getSignExtBits(Expression* curr) {
    if (Properties::getSignExtValue(curr)) {
      return Properties::getSignExtBits(curr);
    }
    if (auto* get = curr->dynCast<LocalGet>()) {
      // Check what we know about the local.
      return localInfo[get->index].signExtBits;
    }
    return 0;
  }

  // optimize trivial math operations, given that the right side of a binary
  // is a constant
  Expression* optimizeWithConstantOnRight(Binary* curr) {
    using namespace Match;
    using namespace Abstract;
    Builder builder(*getModule());
    Expression* left;
    auto* right = curr->right->cast<Const>();
    auto type = curr->right->type;

    // Operations on zero
    if (matches(curr, binary(Shl, any(&left), ival(0))) ||
        matches(curr, binary(ShrU, any(&left), ival(0))) ||
        matches(curr, binary(ShrS, any(&left), ival(0))) ||
        matches(curr, binary(Or, any(&left), ival(0))) ||
        matches(curr, binary(Xor, any(&left), ival(0)))) {
      return left;
    }
    if (matches(curr, binary(Mul, pure(&left), ival(0))) ||
        matches(curr, binary(And, pure(&left), ival(0)))) {
      return right;
    }
    // -x * C   ==>    x * -C,   if  shrinkLevel != 0  or  C != C_pot
    // -x * C   ==>   -(x * C),  otherwise
    //    where  x, C  are integers
    Binary* inner;
    if (matches(
          curr,
          binary(Mul, binary(&inner, Sub, ival(0), any(&left)), ival()))) {
      if (getPassOptions().shrinkLevel != 0 ||
          !Bits::isPowerOf2(right->value.getInteger())) {
        right->value = right->value.neg();
        curr->left = left;
        return curr;
      } else {
        curr->left = left;
        Const* zero = inner->left->cast<Const>();
        return builder.makeBinary(inner->op, zero, curr);
      }
    }
    // x == 0   ==>   eqz x
    if (matches(curr, binary(Eq, any(&left), ival(0)))) {
      return builder.makeUnary(Abstract::getUnary(type, EqZ), left);
    }
    // Operations on one
    // (signed)x % 1   ==>   0
    if (matches(curr, binary(RemS, pure(&left), ival(1)))) {
      right->value = Literal::makeZero(type);
      return right;
    }
    // (signed)x % C_pot != 0   ==>  (x & (abs(C_pot) - 1)) != 0
    {
      Const* c;
      Binary* inner;
      if (matches(curr,
                  binary(Ne, binary(&inner, RemS, any(), ival(&c)), ival(0))) &&
          (c->value.isSignedMin() ||
           Bits::isPowerOf2(c->value.abs().getInteger()))) {
        inner->op = Abstract::getBinary(c->type, And);
        if (c->value.isSignedMin()) {
          c->value = Literal::makeSignedMax(c->type);
        } else {
          c->value = c->value.abs().sub(Literal::makeOne(c->type));
        }
        return curr;
      }
    }
    // i32(bool(x)) == 1  ==>  i32(bool(x))
    // i32(bool(x)) != 0  ==>  i32(bool(x))
    // i32(bool(x)) & 1   ==>  i32(bool(x))
    // i64(bool(x)) & 1   ==>  i64(bool(x))
    if ((matches(curr, binary(EqInt32, any(&left), i32(1))) ||
         matches(curr, binary(NeInt32, any(&left), i32(0))) ||
         matches(curr, binary(And, any(&left), ival(1)))) &&
        Bits::getMaxBits(left, this) == 1) {
      return left;
    }
    // i64(bool(x)) == 1  ==>  i32(bool(x))
    // i64(bool(x)) != 0  ==>  i32(bool(x))
    if ((matches(curr, binary(EqInt64, any(&left), i64(1))) ||
         matches(curr, binary(NeInt64, any(&left), i64(0)))) &&
        Bits::getMaxBits(left, this) == 1) {
      return builder.makeUnary(WrapInt64, left);
    }
    // bool(x) != 1  ==>  !bool(x)
    if (matches(curr, binary(Ne, any(&left), ival(1))) &&
        Bits::getMaxBits(left, this) == 1) {
      return builder.makeUnary(Abstract::getUnary(type, EqZ), left);
    }
    // bool(x)  ^ 1  ==>  !bool(x)
    if (matches(curr, binary(Xor, any(&left), ival(1))) &&
        Bits::getMaxBits(left, this) == 1) {
      auto* result = builder.makeUnary(Abstract::getUnary(type, EqZ), left);
      if (left->type == Type::i64) {
        // Xor's result is also an i64 in this case, but EqZ returns i32, so we
        // must expand it so that we keep returning the same value as before.
        // This means we replace a xor and a const with a xor and an extend,
        // which is still smaller (the const is 2 bytes, the extend just 1), and
        // also the extend may be removed by further work.
        result = builder.makeUnary(ExtendUInt32, result);
      }
      return result;
    }
    // bool(x) | 1  ==>  1
    if (matches(curr, binary(Or, pure(&left), ival(1))) &&
        Bits::getMaxBits(left, this) == 1) {
      return right;
    }

    // Operations on all 1s
    // x & -1   ==>   x
    if (matches(curr, binary(And, any(&left), ival(-1)))) {
      return left;
    }
    // x | -1   ==>   -1
    if (matches(curr, binary(Or, pure(&left), ival(-1)))) {
      return right;
    }
    // (signed)x % -1   ==>   0
    if (matches(curr, binary(RemS, pure(&left), ival(-1)))) {
      right->value = Literal::makeZero(type);
      return right;
    }
    // i32(x) / i32.min_s   ==>   x == i32.min_s
    if (matches(
          curr,
          binary(DivSInt32, any(), i32(std::numeric_limits<int32_t>::min())))) {
      curr->op = EqInt32;
      return curr;
    }
    // i64(x) / i64.min_s   ==>   i64(x == i64.min_s)
    // only for zero shrink level
    if (getPassOptions().shrinkLevel == 0 &&
        matches(
          curr,
          binary(DivSInt64, any(), i64(std::numeric_limits<int64_t>::min())))) {
      curr->op = EqInt64;
      curr->type = Type::i32;
      return builder.makeUnary(ExtendUInt32, curr);
    }
    // (unsigned)x < 0   ==>   i32(0)
    if (matches(curr, binary(LtU, pure(&left), ival(0)))) {
      right->value = Literal::makeZero(Type::i32);
      right->type = Type::i32;
      return right;
    }
    // (unsigned)x <= -1  ==>   i32(1)
    if (matches(curr, binary(LeU, pure(&left), ival(-1)))) {
      right->value = Literal::makeOne(Type::i32);
      right->type = Type::i32;
      return right;
    }
    // (unsigned)x > -1   ==>   i32(0)
    if (matches(curr, binary(GtU, pure(&left), ival(-1)))) {
      right->value = Literal::makeZero(Type::i32);
      right->type = Type::i32;
      return right;
    }
    // (unsigned)x >= 0   ==>   i32(1)
    if (matches(curr, binary(GeU, pure(&left), ival(0)))) {
      right->value = Literal::makeOne(Type::i32);
      right->type = Type::i32;
      return right;
    }
    // (unsigned)x < -1   ==>   x != -1
    // Friendlier to JS emitting as we don't need to write an unsigned -1 value
    // which is large.
    if (matches(curr, binary(LtU, any(), ival(-1)))) {
      curr->op = Abstract::getBinary(type, Ne);
      return curr;
    }
    // (unsigned)x <= 0   ==>   x == 0
    if (matches(curr, binary(LeU, any(), ival(0)))) {
      curr->op = Abstract::getBinary(type, Eq);
      return curr;
    }
    // (unsigned)x > 0   ==>   x != 0
    if (matches(curr, binary(GtU, any(), ival(0)))) {
      curr->op = Abstract::getBinary(type, Ne);
      return curr;
    }
    // (unsigned)x >= -1  ==>   x == -1
    if (matches(curr, binary(GeU, any(), ival(-1)))) {
      curr->op = Abstract::getBinary(type, Eq);
      return curr;
    }
    {
      Const* c;
      // (signed)x < (i32|i64).min_s   ==>   i32(0)
      if (matches(curr, binary(LtS, pure(&left), ival(&c))) &&
          c->value.isSignedMin()) {
        right->value = Literal::makeZero(Type::i32);
        right->type = Type::i32;
        return right;
      }
      // (signed)x <= (i32|i64).max_s   ==>   i32(1)
      if (matches(curr, binary(LeS, pure(&left), ival(&c))) &&
          c->value.isSignedMax()) {
        right->value = Literal::makeOne(Type::i32);
        right->type = Type::i32;
        return right;
      }
      // (signed)x > (i32|i64).max_s   ==>   i32(0)
      if (matches(curr, binary(GtS, pure(&left), ival(&c))) &&
          c->value.isSignedMax()) {
        right->value = Literal::makeZero(Type::i32);
        right->type = Type::i32;
        return right;
      }
      // (signed)x >= (i32|i64).min_s   ==>   i32(1)
      if (matches(curr, binary(GeS, pure(&left), ival(&c))) &&
          c->value.isSignedMin()) {
        right->value = Literal::makeOne(Type::i32);
        right->type = Type::i32;
        return right;
      }
      // (signed)x < (i32|i64).max_s   ==>   x != (i32|i64).max_s
      if (matches(curr, binary(LtS, any(), ival(&c))) &&
          c->value.isSignedMax()) {
        curr->op = Abstract::getBinary(type, Ne);
        return curr;
      }
      // (signed)x <= (i32|i64).min_s   ==>   x == (i32|i64).min_s
      if (matches(curr, binary(LeS, any(), ival(&c))) &&
          c->value.isSignedMin()) {
        curr->op = Abstract::getBinary(type, Eq);
        return curr;
      }
      // (signed)x > (i32|i64).min_s   ==>   x != (i32|i64).min_s
      if (matches(curr, binary(GtS, any(), ival(&c))) &&
          c->value.isSignedMin()) {
        curr->op = Abstract::getBinary(type, Ne);
        return curr;
      }
      // (signed)x >= (i32|i64).max_s   ==>   x == (i32|i64).max_s
      if (matches(curr, binary(GeS, any(), ival(&c))) &&
          c->value.isSignedMax()) {
        curr->op = Abstract::getBinary(type, Eq);
        return curr;
      }
    }
    // x * -1   ==>   0 - x
    if (matches(curr, binary(Mul, any(&left), ival(-1)))) {
      right->value = Literal::makeZero(type);
      curr->op = Abstract::getBinary(type, Sub);
      curr->left = right;
      curr->right = left;
      return curr;
    }
    {
      // ~(1 << x) aka (1 << x) ^ -1  ==>  rotl(-2, x)
      Expression* x;
      // Note that we avoid this in JS mode, as emitting a rotation would
      // require lowering that rotation for JS in another cycle of work.
      if (matches(curr, binary(Xor, binary(Shl, ival(1), any(&x)), ival(-1))) &&
          !getPassOptions().targetJS) {
        curr->op = Abstract::getBinary(type, RotL);
        right->value = Literal::makeFromInt32(-2, type);
        curr->left = right;
        curr->right = x;
        return curr;
      }
    }
    {
      // x * 2.0  ==>  x + x
      // but we apply this only for simple expressions like
      // local.get and global.get for avoid using extra local
      // variable.
      Expression* x;
      if (matches(curr, binary(Mul, any(&x), fval(2.0))) &&
          (x->is<LocalGet>() || x->is<GlobalGet>())) {
        curr->op = Abstract::getBinary(type, Abstract::Add);
        curr->right = ExpressionManipulator::copy(x, *getModule());
        return curr;
      }
    }
    {
      // x + (-0.0)   ==>   x
      double value;
      if (fastMath && matches(curr, binary(Add, any(), fval(&value))) &&
          value == 0.0 && std::signbit(value)) {
        return curr->left;
      }
    }
    // -x * fval(C)   ==>   x * -C
    // -x / fval(C)   ==>   x / -C
    if (matches(curr, binary(Mul, unary(Neg, any(&left)), fval())) ||
        matches(curr, binary(DivS, unary(Neg, any(&left)), fval()))) {
      right->value = right->value.neg();
      curr->left = left;
      return curr;
    }
    // x * -1.0   ==>
    //       -x,  if fastMath == true
    // -0.0 - x,  if fastMath == false
    if (matches(curr, binary(Mul, any(), fval(-1.0)))) {
      if (fastMath) {
        return builder.makeUnary(Abstract::getUnary(type, Neg), left);
      }
      // x * -1.0   ==>  -0.0 - x
      curr->op = Abstract::getBinary(type, Sub);
      right->value = Literal::makeZero(type).neg();
      std::swap(curr->left, curr->right);
      return curr;
    }
    if (matches(curr, binary(Mul, any(&left), constant(1))) ||
        matches(curr, binary(DivS, any(&left), constant(1))) ||
        matches(curr, binary(DivU, any(&left), constant(1)))) {
      if (curr->type.isInteger() || fastMath) {
        return left;
      }
    }
    {
      //   x !=  NaN   ==>   1
      //   x <=> NaN   ==>   0
      //   x op  NaN'  ==>   NaN',  iff `op` != `copysign` and `x` != C
      Const* c;
      Binary* bin;
      Expression* x;
      if (matches(curr, binary(&bin, pure(&x), fval(&c))) &&
          std::isnan(c->value.getFloat()) &&
          bin->op != getBinary(x->type, CopySign)) {
        if (bin->isRelational()) {
          // reuse "c" (nan) constant
          c->type = Type::i32;
          if (bin->op == getBinary(x->type, Ne)) {
            // x != NaN  ==>  1
            c->value = Literal::makeOne(Type::i32);
          } else {
            // x == NaN,
            // x >  NaN,
            // x <= NaN
            // x .. NaN  ==>  0
            c->value = Literal::makeZero(Type::i32);
          }
          return c;
        }
        // propagate NaN of RHS but canonicalize it
        c->value = Literal::standardizeNaN(c->value);
        return c;
      }
    }
    return nullptr;
  }

  // Returns true if the given binary operation can overflow. If we can't be
  // sure either way, we return true, assuming the worst.
  //
  // We can check for an unsigned overflow (more than the max number of bits) or
  // a signed one (where even reaching the sign bit is an overflow, as that
  // would turn us from positive to negative).
  bool canOverflow(Binary* binary, bool signed_) {
    using namespace Abstract;

    // If we know nothing about a limit on the amount of bits on either side,
    // give up.
    auto typeMaxBits = getBitsForType(binary->type);
    auto leftMaxBits = Bits::getMaxBits(binary->left, this);
    auto rightMaxBits = Bits::getMaxBits(binary->right, this);
    if (std::max(leftMaxBits, rightMaxBits) == typeMaxBits) {
      return true;
    }

    if (binary->op == getBinary(binary->type, Add)) {
      if (!signed_) {
        // Proof this cannot overflow:
        //
        // left + right <  2^leftMaxBits + 2^rightMaxBits          (1)
        //              <= 2^(typeMaxBits-1) + 2^(typeMaxBits-1)   (2)
        //              =  2^typeMaxBits                           (3)
        //
        // (1) By the definition of the max bits (e.g. an int32 has 32 max bits,
        //     and its max value is 2^32 - 1, which is < 2^32).
        // (2) By the above checks and early returns.
        // (3) 2^x + 2^x === 2*2^x === 2^(x+1)
        return false;
      }

      // For a signed comparison, check that the total cannot reach the sign
      // bit.
      return leftMaxBits + rightMaxBits >= typeMaxBits;
    }

    // TODO subtraction etc.
    return true;
  }

  // Folding two expressions into one with similar operations and
  // constants on RHSs
  Expression* optimizeDoubletonWithConstantOnRight(Binary* curr) {
    using namespace Match;
    using namespace Abstract;
    {
      Binary* inner;
      Const *c1, *c2 = curr->right->cast<Const>();
      if (matches(curr->left, binary(&inner, any(), ival(&c1))) &&
          inner->op == curr->op) {
        Type type = inner->type;
        BinaryOp op = inner->op;
        // (x & C1) & C2   =>   x & (C1 & C2)
        if (op == getBinary(type, And)) {
          c1->value = c1->value.and_(c2->value);
          return inner;
        }
        // (x | C1) | C2   =>   x | (C1 | C2)
        if (op == getBinary(type, Or)) {
          c1->value = c1->value.or_(c2->value);
          return inner;
        }
        // (x ^ C1) ^ C2   =>   x ^ (C1 ^ C2)
        if (op == getBinary(type, Xor)) {
          c1->value = c1->value.xor_(c2->value);
          return inner;
        }
        // (x * C1) * C2   =>   x * (C1 * C2)
        if (op == getBinary(type, Mul)) {
          c1->value = c1->value.mul(c2->value);
          return inner;
        }
        // TODO:
        // handle signed / unsigned divisions. They are more complex

        // (x <<>> C1) <<>> C2   =>   x <<>> (C1 + C2)
        if (hasAnyShift(op)) {
          // shifts only use an effective amount from the constant, so
          // adding must be done carefully
          auto total =
            Bits::getEffectiveShifts(c1) + Bits::getEffectiveShifts(c2);
          auto effectiveTotal = Bits::getEffectiveShifts(total, c1->type);
          if (total == effectiveTotal) {
            // no overflow, we can do this
            c1->value = Literal::makeFromInt32(total, c1->type);
            return inner;
          } else {
            // overflow. Handle different scenarious
            if (hasAnyRotateShift(op)) {
              // overflow always accepted in rotation shifts
              c1->value = Literal::makeFromInt32(effectiveTotal, c1->type);
              return inner;
            }
            // handle overflows for general shifts
            //   x << C1 << C2    =>   0 or { drop(x), 0 }
            //   x >>> C1 >>> C2  =>   0 or { drop(x), 0 }
            // iff `C1 + C2` -> overflows
            if ((op == getBinary(type, Shl) || op == getBinary(type, ShrU))) {
              auto* x = inner->left;
              c1->value = Literal::makeZero(c1->type);
              if (!effects(x).hasSideEffects()) {
                //  =>  0
                return c1;
              } else {
                //  =>  { drop(x), 0 }
                Builder builder(*getModule());
                return builder.makeBlock({builder.makeDrop(x), c1});
              }
            }
            //   i32(x) >> C1 >> C2   =>   x >> 31
            //   i64(x) >> C1 >> C2   =>   x >> 63
            // iff `C1 + C2` -> overflows
            if (op == getBinary(type, ShrS)) {
              c1->value = Literal::makeFromInt32(c1->type.getByteSize() * 8 - 1,
                                                 c1->type);
              return inner;
            }
          }
        }
      }
    }
    {
      // (x << C1) * C2   =>   x * (C2 << C1)
      Binary* inner;
      Const *c1, *c2;
      if (matches(
            curr,
            binary(Mul, binary(&inner, Shl, any(), ival(&c1)), ival(&c2)))) {
        inner->op = getBinary(inner->type, Mul);
        c1->value = c2->value.shl(c1->value);
        return inner;
      }
    }
    {
      // (x * C1) << C2   =>   x * (C1 << C2)
      Binary* inner;
      Const *c1, *c2;
      if (matches(
            curr,
            binary(Shl, binary(&inner, Mul, any(), ival(&c1)), ival(&c2)))) {
        c1->value = c1->value.shl(c2->value);
        return inner;
      }
    }
    {
      // TODO: Add cancelation for some large constants when shrinkLevel > 0
      // in FinalOptimizer.

      // (x >> C)  << C   =>   x & -(1 << C)
      // (x >>> C) << C   =>   x & -(1 << C)
      Binary* inner;
      Const *c1, *c2;
      if (matches(curr,
                  binary(Shl, binary(&inner, any(), ival(&c1)), ival(&c2))) &&
          (inner->op == getBinary(inner->type, ShrS) ||
           inner->op == getBinary(inner->type, ShrU)) &&
          Bits::getEffectiveShifts(c1) == Bits::getEffectiveShifts(c2)) {
        auto type = c1->type;
        if (type == Type::i32) {
          c1->value = Literal::makeFromInt32(
            -(1U << Bits::getEffectiveShifts(c1)), Type::i32);
        } else {
          c1->value = Literal::makeFromInt64(
            -(1ULL << Bits::getEffectiveShifts(c1)), Type::i64);
        }
        inner->op = getBinary(type, And);
        return inner;
      }
    }
    {
      // TODO: Add cancelation for some large constants when shrinkLevel > 0
      // in FinalOptimizer.

      // (x << C) >>> C   =>   x & (-1 >>> C)
      // (x << C) >> C    =>   skip
      Binary* inner;
      Const *c1, *c2;
      if (matches(
            curr,
            binary(ShrU, binary(&inner, Shl, any(), ival(&c1)), ival(&c2))) &&
          Bits::getEffectiveShifts(c1) == Bits::getEffectiveShifts(c2)) {
        auto type = c1->type;
        if (type == Type::i32) {
          c1->value = Literal::makeFromInt32(
            -1U >> Bits::getEffectiveShifts(c1), Type::i32);
        } else {
          c1->value = Literal::makeFromInt64(
            -1ULL >> Bits::getEffectiveShifts(c1), Type::i64);
        }
        inner->op = getBinary(type, And);
        return inner;
      }
    }
    {
      // TODO: Add canonicalization rotr to rotl and remove these rules.
      // rotl(rotr(x, C1), C2)   =>   rotr(x, C1 - C2)
      // rotr(rotl(x, C1), C2)   =>   rotl(x, C1 - C2)
      Binary* inner;
      Const *c1, *c2;
      if (matches(
            curr,
            binary(RotL, binary(&inner, RotR, any(), ival(&c1)), ival(&c2))) ||
          matches(
            curr,
            binary(RotR, binary(&inner, RotL, any(), ival(&c1)), ival(&c2)))) {
        auto diff = Bits::getEffectiveShifts(c1) - Bits::getEffectiveShifts(c2);
        c1->value = Literal::makeFromInt32(
          Bits::getEffectiveShifts(diff, c2->type), c2->type);
        return inner;
      }
    }
    return nullptr;
  }

  // optimize trivial math operations, given that the left side of a binary
  // is a constant. since we canonicalize constants to the right for symmetrical
  // operations, we only need to handle asymmetrical ones here
  // TODO: templatize on type?
  Expression* optimizeWithConstantOnLeft(Binary* curr) {
    using namespace Match;
    using namespace Abstract;

    auto type = curr->left->type;
    auto* left = curr->left->cast<Const>();
    // 0 <<>> x   ==>   0
    if (Abstract::hasAnyShift(curr->op) && left->value.isZero() &&
        !effects(curr->right).hasSideEffects()) {
      return curr->left;
    }
    // (signed)-1 >> x   ==>   -1
    // rotl(-1, x)       ==>   -1
    // rotr(-1, x)       ==>   -1
    if ((curr->op == Abstract::getBinary(type, ShrS) ||
         curr->op == Abstract::getBinary(type, RotL) ||
         curr->op == Abstract::getBinary(type, RotR)) &&
        left->value.getInteger() == -1LL &&
        !effects(curr->right).hasSideEffects()) {
      return curr->left;
    }
    {
      // C1 - (x + C2)  ==>  (C1 - C2) - x
      Const *c1, *c2;
      Expression* x;
      if (matches(curr,
                  binary(Sub, ival(&c1), binary(Add, any(&x), ival(&c2))))) {
        left->value = c1->value.sub(c2->value);
        curr->right = x;
        return curr;
      }
      // C1 - (C2 - x)  ==>   x + (C1 - C2)
      if (matches(curr,
                  binary(Sub, ival(&c1), binary(Sub, ival(&c2), any(&x))))) {
        left->value = c1->value.sub(c2->value);
        curr->op = Abstract::getBinary(type, Add);
        curr->right = x;
        std::swap(curr->left, curr->right);
        return curr;
      }
    }
    {
      // fval(C) / -x   ==>  -C / x
      Expression* right;
      if (matches(curr, binary(DivS, fval(), unary(Neg, any(&right))))) {
        left->value = left->value.neg();
        curr->right = right;
        return curr;
      }
    }
    return nullptr;
  }

  // TODO: templatize on type?
  Expression* optimizeRelational(Binary* curr) {
    using namespace Abstract;
    using namespace Match;

    auto type = curr->right->type;
    if (curr->left->type.isInteger()) {
      if (curr->op == Abstract::getBinary(type, Abstract::Eq) ||
          curr->op == Abstract::getBinary(type, Abstract::Ne)) {
        if (auto* left = curr->left->dynCast<Binary>()) {
          // TODO: inequalities can also work, if the constants do not overflow
          // integer math, even on 2s complement, allows stuff like
          // x + 5 == 7
          //   =>
          //     x == 2
          if (left->op == Abstract::getBinary(type, Abstract::Add)) {
            if (auto* leftConst = left->right->dynCast<Const>()) {
              if (auto* rightConst = curr->right->dynCast<Const>()) {
                return combineRelationalConstants(
                  curr, left, leftConst, nullptr, rightConst);
              } else if (auto* rightBinary = curr->right->dynCast<Binary>()) {
                if (rightBinary->op ==
                    Abstract::getBinary(type, Abstract::Add)) {
                  if (auto* rightConst = rightBinary->right->dynCast<Const>()) {
                    return combineRelationalConstants(
                      curr, left, leftConst, rightBinary, rightConst);
                  }
                }
              }
            }
          }
        }
      }
      // x - y == 0  =>  x == y
      // x - y != 0  =>  x != y
      // unsigned(x - y) > 0    =>   x != y
      // unsigned(x - y) <= 0   =>   x == y
      {
        Binary* inner;
        // unsigned(x - y) > 0    =>   x != y
        if (matches(curr,
                    binary(GtU, binary(&inner, Sub, any(), any()), ival(0)))) {
          curr->op = Abstract::getBinary(type, Ne);
          curr->right = inner->right;
          curr->left = inner->left;
          return curr;
        }
        // unsigned(x - y) <= 0   =>   x == y
        if (matches(curr,
                    binary(LeU, binary(&inner, Sub, any(), any()), ival(0)))) {
          curr->op = Abstract::getBinary(type, Eq);
          curr->right = inner->right;
          curr->left = inner->left;
          return curr;
        }
        // x - y == 0  =>  x == y
        // x - y != 0  =>  x != y
        // This is not true for signed comparisons like x -y < 0 due to overflow
        // effects (e.g. 8 - 0x80000000 < 0 is not the same as 8 < 0x80000000).
        if (matches(curr,
                    binary(Eq, binary(&inner, Sub, any(), any()), ival(0))) ||
            matches(curr,
                    binary(Ne, binary(&inner, Sub, any(), any()), ival(0)))) {
          curr->right = inner->right;
          curr->left = inner->left;
          return curr;
        }
      }

      // x + C1 > C2   ==>  x > (C2-C1)      if no overflowing, C2 >= C1
      // x + C1 > C2   ==>  x + (C1-C2) > 0  if no overflowing, C2 <  C1
      // And similarly for other relational operations on integers with a "+"
      // on the left.
      // TODO: support - and not just +
      {
        Binary* add;
        Const* c1;
        Const* c2;
        if (matches(curr,
                    binary(binary(&add, Add, any(), ival(&c1)), ival(&c2))) &&
            !canOverflow(add, isSignedOp(curr->op))) {
          // We want to subtract C2-C1 or C1-C2. When doing so, we must avoid an
          // overflow in that subtraction (so that we keep all the math here
          // properly linear in the mathematical sense). Overflows that concern
          // us include an underflow with unsigned values (e.g. 10 - 20, which
          // flips the result to a large positive number), and a sign bit
          // overflow for signed values (e.g. 0x80000000 - 1 = 0x7fffffff flips
          // from a negative number, -1, to a positive one). We also need to be
          // careful of signed handling of 0x80000000, for whom 0 - 0x80000000
          // is equal to 0x80000000, leading to
          //   x + 0x80000000 > 0                ;; always false
          // (apply the rule)
          //   x > 0 - 0x80000000 = 0x80000000   ;; depends on x
          // The general principle in all of this is that when we go from
          //   (a)    x + C1 > C2
          // to
          //   (b)    x      > (C2-C1)
          // then we want to adjust both sides in the same (linear) manner. That
          // is, we can write the latter as
          //   (b')   x + 0  > (C2-C1)
          // Comparing (a) and (b'), we want the constants to change in a
          // consistent way: C1 changes to 0, and C2 changes to C2-C1. Both
          // transformations should decrease the value, which is violated in all
          // the overflows described above:
          //   * Unsigned overflow: C1=20, C2=10, then C1 decreases but C2-C1
          //     is larger than C2.
          //   * Sign flip: C1=1, C2=0x80000000, then C1 decreases but C2-C1 is
          //     is larger than C2.
          //   * C1=0x80000000, C2=0, then C1 increases while C2-C1 stays the
          //     same.
          // In the first and second case we can apply the other rule using
          // C1-C2 rather than C2-C1. The third case, however, doesn't even work
          // that way.
          auto C1 = c1->value;
          auto C2 = c2->value;
          auto C1SubC2 = C1.sub(C2);
          auto C2SubC1 = C2.sub(C1);
          auto zero = Literal::makeZero(add->type);
          auto doC1SubC2 = false;
          auto doC2SubC1 = false;
          // Ignore the case of C1 or C2 being zero, as then C2-C1 or C1-C2
          // does not change anything (and we don't want the optimizer to think
          // we improved anything, or we could infinite loop on the mirage of
          // progress).
          if (C1 != zero && C2 != zero) {
            if (isSignedOp(curr->op)) {
              if (C2SubC1.leS(C2).getInteger() && zero.leS(C1).getInteger()) {
                // C2=>C2-C1 and C1=>0 both decrease, which means we can do the
                // rule
                //   (a)    x + C1   > C2
                //   (b')   x (+ 0)  > (C2-C1)
                // That is, subtracting C1 from both sides is ok; the constants
                // on both sides change in the same manner.
                doC2SubC1 = true;
              } else if (C1SubC2.leS(C1).getInteger() &&
                         zero.leS(C2).getInteger()) {
                // N.B. this code path is not tested atm as other optimizations
                // will canonicalize x + C into x - C, and so we would need to
                // implement the TODO above on subtraction and not only support
                // addition here.
                doC1SubC2 = true;
              }
            } else {
              // Unsigned.
              if (C2SubC1.leU(C2).getInteger() && zero.leU(C1).getInteger()) {
                doC2SubC1 = true;
              } else if (C1SubC2.leU(C1).getInteger() &&
                         zero.leU(C2).getInteger()) {
                doC1SubC2 = true;
              }
              // For unsigned, one of the cases must work out, as there are no
              // corner cases with the sign bit.
              assert(doC2SubC1 || doC1SubC2);
            }
          }
          if (doC2SubC1) {
            // This is the first line above, we turn into x > (C2-C1).
            c2->value = C2SubC1;
            curr->left = add->left;
            return curr;
          }
          // This is the second line above, we turn into x + (C1-C2) > 0.
          if (doC1SubC2) {
            c1->value = C1SubC2;
            c2->value = zero;
            return curr;
          }
        }
      }

      // Comparisons can sometimes be simplified depending on the number of
      // bits, e.g.  (unsigned)x > y  must be true if x has strictly more bits.
      // A common case is a constant on the right, e.g. (x & 255) < 256 must be
      // true.
      // TODO: use getMinBits in more places, see ideas in
      //       https://github.com/WebAssembly/binaryen/issues/2898
      {
        // Check if there is a nontrivial amount of bits on the left, which may
        // provide enough to optimize.
        auto leftMaxBits = Bits::getMaxBits(curr->left, this);
        auto type = curr->left->type;
        if (leftMaxBits < getBitsForType(type)) {
          using namespace Abstract;
          auto rightMinBits = Bits::getMinBits(curr->right);
          auto rightIsNegative = rightMinBits == getBitsForType(type);
          if (leftMaxBits < rightMinBits) {
            // There are not enough bits on the left for it to be equal to the
            // right, making various comparisons obviously false:
            //             x == y
            //   (unsigned)x >  y
            //   (unsigned)x >= y
            // and the same for signed, if y does not have the sign bit set
            // (in that case, the comparison is effectively unsigned).
            //
            // TODO: In addition to leftMaxBits < rightMinBits, we could
            //       handle the reverse, and also special cases like all bits
            //       being 1 on the right, things like (x & 255) <= 255  ->  1
            if (curr->op == Abstract::getBinary(type, Eq) ||
                curr->op == Abstract::getBinary(type, GtU) ||
                curr->op == Abstract::getBinary(type, GeU) ||
                (!rightIsNegative &&
                 (curr->op == Abstract::getBinary(type, GtS) ||
                  curr->op == Abstract::getBinary(type, GeS)))) {
              return getDroppedChildrenAndAppend(curr,
                                                 Literal::makeZero(Type::i32));
            }

            // And some are obviously true:
            //             x != y
            //   (unsigned)x <  y
            //   (unsigned)x <= y
            // and likewise for signed, as above.
            if (curr->op == Abstract::getBinary(type, Ne) ||
                curr->op == Abstract::getBinary(type, LtU) ||
                curr->op == Abstract::getBinary(type, LeU) ||
                (!rightIsNegative &&
                 (curr->op == Abstract::getBinary(type, LtS) ||
                  curr->op == Abstract::getBinary(type, LeS)))) {
              return getDroppedChildrenAndAppend(curr,
                                                 Literal::makeOne(Type::i32));
            }

            // For truly signed comparisons, where y's sign bit is set, we can
            // also infer some things, since we know y is signed but x is not
            // (since x does not have enough bits for the sign bit to be set).
            if (rightIsNegative) {
              //   (signed, non-negative)x >  (negative)y   =>   1
              //   (signed, non-negative)x >= (negative)y   =>   1
              if (curr->op == Abstract::getBinary(type, GtS) ||
                  curr->op == Abstract::getBinary(type, GeS)) {
                return getDroppedChildrenAndAppend(curr,
                                                   Literal::makeOne(Type::i32));
              }
              //   (signed, non-negative)x <  (negative)y   =>   0
              //   (signed, non-negative)x <= (negative)y   =>   0
              if (curr->op == Abstract::getBinary(type, LtS) ||
                  curr->op == Abstract::getBinary(type, LeS)) {
                return getDroppedChildrenAndAppend(
                  curr, Literal::makeZero(Type::i32));
              }
            }
          }
        }
      }
    }
    return nullptr;
  }

  Expression* simplifyRoundingsAndConversions(Unary* curr) {
    using namespace Abstract;
    using namespace Match;

    switch (curr->op) {
      case TruncSFloat64ToInt32:
      case TruncSatSFloat64ToInt32: {
        // i32 -> f64 -> i32 rountripping optimization:
        //   i32.trunc(_sat)_f64_s(f64.convert_i32_s(x))  ==>  x
        Expression* x;
        if (matches(curr->value, unary(ConvertSInt32ToFloat64, any(&x)))) {
          return x;
        }
        break;
      }
      case TruncUFloat64ToInt32:
      case TruncSatUFloat64ToInt32: {
        // u32 -> f64 -> u32 rountripping optimization:
        //   i32.trunc(_sat)_f64_u(f64.convert_i32_u(x))  ==>  x
        Expression* x;
        if (matches(curr->value, unary(ConvertUInt32ToFloat64, any(&x)))) {
          return x;
        }
        break;
      }
      case CeilFloat32:
      case CeilFloat64:
      case FloorFloat32:
      case FloorFloat64:
      case TruncFloat32:
      case TruncFloat64:
      case NearestFloat32:
      case NearestFloat64: {
        // Rounding after integer to float conversion may be skipped
        //   ceil(float(int(x)))     ==>  float(int(x))
        //   floor(float(int(x)))    ==>  float(int(x))
        //   trunc(float(int(x)))    ==>  float(int(x))
        //   nearest(float(int(x)))  ==>  float(int(x))
        Unary* inner;
        if (matches(curr->value, unary(&inner, any()))) {
          switch (inner->op) {
            case ConvertSInt32ToFloat32:
            case ConvertSInt32ToFloat64:
            case ConvertUInt32ToFloat32:
            case ConvertUInt32ToFloat64:
            case ConvertSInt64ToFloat32:
            case ConvertSInt64ToFloat64:
            case ConvertUInt64ToFloat32:
            case ConvertUInt64ToFloat64: {
              return inner;
            }
            default: {
            }
          }
        }
        break;
      }
      default: {
      }
    }
    return nullptr;
  }

  Expression* deduplicateUnary(Unary* unaryOuter) {
    if (auto* unaryInner = unaryOuter->value->dynCast<Unary>()) {
      if (unaryInner->op == unaryOuter->op) {
        switch (unaryInner->op) {
          case NegFloat32:
          case NegFloat64: {
            // neg(neg(x))  ==>   x
            return unaryInner->value;
          }
          case AbsFloat32:
          case CeilFloat32:
          case FloorFloat32:
          case TruncFloat32:
          case NearestFloat32:
          case AbsFloat64:
          case CeilFloat64:
          case FloorFloat64:
          case TruncFloat64:
          case NearestFloat64: {
            // unaryOp(unaryOp(x))  ==>   unaryOp(x)
            return unaryInner;
          }
          case ExtendS8Int32:
          case ExtendS16Int32: {
            assert(getModule()->features.hasSignExt());
            return unaryInner;
          }
          case EqZInt32: {
            // eqz(eqz(bool(x)))  ==>   bool(x)
            if (Bits::getMaxBits(unaryInner->value, this) == 1) {
              return unaryInner->value;
            }
            break;
          }
          default: {
          }
        }
      }
    }
    return nullptr;
  }

  Expression* deduplicateBinary(Binary* outer) {
    Type type = outer->type;
    if (type.isInteger()) {
      if (auto* inner = outer->right->dynCast<Binary>()) {
        if (outer->op == inner->op) {
          if (!EffectAnalyzer(getPassOptions(), *getModule(), outer->left)
                 .hasSideEffects()) {
            if (ExpressionAnalyzer::equal(inner->left, outer->left)) {
              // x - (x - y)  ==>   y
              // x ^ (x ^ y)  ==>   y
              if (outer->op == Abstract::getBinary(type, Abstract::Sub) ||
                  outer->op == Abstract::getBinary(type, Abstract::Xor)) {
                return inner->right;
              }
              // x & (x & y)  ==>   x & y
              // x | (x | y)  ==>   x | y
              if (outer->op == Abstract::getBinary(type, Abstract::And) ||
                  outer->op == Abstract::getBinary(type, Abstract::Or)) {
                return inner;
              }
            }
            if (ExpressionAnalyzer::equal(inner->right, outer->left) &&
                canReorder(outer->left, inner->left)) {
              // x ^ (y ^ x)  ==>   y
              // (note that we need the check for reordering here because if
              // e.g. y writes to a local that x reads, the second appearance
              // of x would be different from the first)
              if (outer->op == Abstract::getBinary(type, Abstract::Xor)) {
                return inner->left;
              }

              // x & (y & x)  ==>   y & x
              // x | (y | x)  ==>   y | x
              // (here we need the check for reordering for the more obvious
              // reason that previously x appeared before y, and now y appears
              // first; or, if we tried to emit x [&|] y here, reversing the
              // order, we'd be in the same situation as the previous comment)
              if (outer->op == Abstract::getBinary(type, Abstract::And) ||
                  outer->op == Abstract::getBinary(type, Abstract::Or)) {
                return inner;
              }
            }
          }
        }
      }
      if (auto* inner = outer->left->dynCast<Binary>()) {
        if (outer->op == inner->op) {
          if (!EffectAnalyzer(getPassOptions(), *getModule(), outer->right)
                 .hasSideEffects()) {
            if (ExpressionAnalyzer::equal(inner->right, outer->right)) {
              // (x ^ y) ^ y  ==>   x
              if (outer->op == Abstract::getBinary(type, Abstract::Xor)) {
                return inner->left;
              }
              // (x % y) % y  ==>   x % y
              // (x & y) & y  ==>   x & y
              // (x | y) | y  ==>   x | y
              if (outer->op == Abstract::getBinary(type, Abstract::RemS) ||
                  outer->op == Abstract::getBinary(type, Abstract::RemU) ||
                  outer->op == Abstract::getBinary(type, Abstract::And) ||
                  outer->op == Abstract::getBinary(type, Abstract::Or)) {
                return inner;
              }
            }
            // See comments in the parallel code earlier about ordering here.
            if (ExpressionAnalyzer::equal(inner->left, outer->right) &&
                canReorder(inner->left, inner->right)) {
              // (x ^ y) ^ x  ==>   y
              if (outer->op == Abstract::getBinary(type, Abstract::Xor)) {
                return inner->right;
              }
              // (x & y) & x  ==>   x & y
              // (x | y) | x  ==>   x | y
              if (outer->op == Abstract::getBinary(type, Abstract::And) ||
                  outer->op == Abstract::getBinary(type, Abstract::Or)) {
                return inner;
              }
            }
          }
        }
      }
    }
    return nullptr;
  }

  // given a relational binary with a const on both sides, combine the constants
  // left is also a binary, and has a constant; right may be just a constant, in
  // which case right is nullptr
  Expression* combineRelationalConstants(Binary* binary,
                                         Binary* left,
                                         Const* leftConst,
                                         Binary* right,
                                         Const* rightConst) {
    auto type = binary->right->type;
    // we fold constants to the right
    Literal extra = leftConst->value;
    if (left->op == Abstract::getBinary(type, Abstract::Sub)) {
      extra = extra.neg();
    }
    if (right && right->op == Abstract::getBinary(type, Abstract::Sub)) {
      extra = extra.neg();
    }
    rightConst->value = rightConst->value.sub(extra);
    binary->left = left->left;
    return binary;
  }

  Expression* optimizeMemoryCopy(MemoryCopy* memCopy) {
    auto& options = getPassOptions();

    if (options.ignoreImplicitTraps || options.trapsNeverHappen) {
      if (areConsecutiveInputsEqual(memCopy->dest, memCopy->source)) {
        // memory.copy(x, x, sz)  ==>  {drop(x), drop(x), drop(sz)}
        Builder builder(*getModule());
        return builder.makeBlock({builder.makeDrop(memCopy->dest),
                                  builder.makeDrop(memCopy->source),
                                  builder.makeDrop(memCopy->size)});
      }
    }

    // memory.copy(dst, src, C)  ==>  store(dst, load(src))
    if (auto* csize = memCopy->size->dynCast<Const>()) {
      auto bytes = csize->value.getInteger();
      Builder builder(*getModule());

      switch (bytes) {
        case 0: {
          if (options.ignoreImplicitTraps || options.trapsNeverHappen) {
            // memory.copy(dst, src, 0)  ==>  {drop(dst), drop(src)}
            return builder.makeBlock({builder.makeDrop(memCopy->dest),
                                      builder.makeDrop(memCopy->source)});
          }
          break;
        }
        case 1:
        case 2:
        case 4: {
          return builder.makeStore(bytes, // bytes
                                   0,     // offset
                                   1,     // align
                                   memCopy->dest,
                                   builder.makeLoad(bytes,
                                                    false,
                                                    0,
                                                    1,
                                                    memCopy->source,
                                                    Type::i32,
                                                    memCopy->sourceMemory),
                                   Type::i32,
                                   memCopy->destMemory);
        }
        case 8: {
          return builder.makeStore(bytes, // bytes
                                   0,     // offset
                                   1,     // align
                                   memCopy->dest,
                                   builder.makeLoad(bytes,
                                                    false,
                                                    0,
                                                    1,
                                                    memCopy->source,
                                                    Type::i64,
                                                    memCopy->sourceMemory),
                                   Type::i64,
                                   memCopy->destMemory);
        }
        case 16: {
          if (options.shrinkLevel == 0) {
            // This adds an extra 2 bytes so apply it only for
            // minimal shrink level
            if (getModule()->features.hasSIMD()) {
              return builder.makeStore(bytes, // bytes
                                       0,     // offset
                                       1,     // align
                                       memCopy->dest,
                                       builder.makeLoad(bytes,
                                                        false,
                                                        0,
                                                        1,
                                                        memCopy->source,
                                                        Type::v128,
                                                        memCopy->sourceMemory),
                                       Type::v128,
                                       memCopy->destMemory);
            }
          }
          break;
        }
        default: {
        }
      }
    }
    return nullptr;
  }

  Expression* optimizeMemoryFill(MemoryFill* memFill) {
    if (memFill->type == Type::unreachable) {
      return nullptr;
    }

    if (!memFill->size->is<Const>()) {
      return nullptr;
    }

    auto& options = getPassOptions();
    Builder builder(*getModule());

    auto* csize = memFill->size->cast<Const>();
    auto bytes = csize->value.getInteger();

    if (bytes == 0LL &&
        (options.ignoreImplicitTraps || options.trapsNeverHappen)) {
      // memory.fill(d, v, 0)  ==>  { drop(d), drop(v) }
      return builder.makeBlock(
        {builder.makeDrop(memFill->dest), builder.makeDrop(memFill->value)});
    }

    const uint32_t offset = 0, align = 1;

    if (auto* cvalue = memFill->value->dynCast<Const>()) {
      uint32_t value = cvalue->value.geti32() & 0xFF;
      // memory.fill(d, C1, C2)  ==>
      //   store(d, (C1 & 0xFF) * (-1U / max(bytes)))
      switch (bytes) {
        case 1: {
          return builder.makeStore(1, // bytes
                                   offset,
                                   align,
                                   memFill->dest,
                                   builder.makeConst<uint32_t>(value),
                                   Type::i32,
                                   memFill->memory);
        }
        case 2: {
          return builder.makeStore(2,
                                   offset,
                                   align,
                                   memFill->dest,
                                   builder.makeConst<uint32_t>(value * 0x0101U),
                                   Type::i32,
                                   memFill->memory);
        }
        case 4: {
          // transform only when "value" or shrinkLevel equal to zero due to
          // it could increase size by several bytes
          if (value == 0 || options.shrinkLevel == 0) {
            return builder.makeStore(
              4,
              offset,
              align,
              memFill->dest,
              builder.makeConst<uint32_t>(value * 0x01010101U),
              Type::i32,
              memFill->memory);
          }
          break;
        }
        case 8: {
          // transform only when "value" or shrinkLevel equal to zero due to
          // it could increase size by several bytes
          if (value == 0 || options.shrinkLevel == 0) {
            return builder.makeStore(
              8,
              offset,
              align,
              memFill->dest,
              builder.makeConst<uint64_t>(value * 0x0101010101010101ULL),
              Type::i64,
              memFill->memory);
          }
          break;
        }
        case 16: {
          if (options.shrinkLevel == 0) {
            if (getModule()->features.hasSIMD()) {
              uint8_t values[16];
              std::fill_n(values, 16, (uint8_t)value);
              return builder.makeStore(16,
                                       offset,
                                       align,
                                       memFill->dest,
                                       builder.makeConst<uint8_t[16]>(values),
                                       Type::v128,
                                       memFill->memory);
            } else {
              // { i64.store(d, C', 0), i64.store(d, C', 8) }
              auto destType = memFill->dest->type;
              Index tempLocal = builder.addVar(getFunction(), destType);
              return builder.makeBlock({
                builder.makeStore(
                  8,
                  offset,
                  align,
                  builder.makeLocalTee(tempLocal, memFill->dest, destType),
                  builder.makeConst<uint64_t>(value * 0x0101010101010101ULL),
                  Type::i64,
                  memFill->memory),
                builder.makeStore(
                  8,
                  offset + 8,
                  align,
                  builder.makeLocalGet(tempLocal, destType),
                  builder.makeConst<uint64_t>(value * 0x0101010101010101ULL),
                  Type::i64,
                  memFill->memory),
              });
            }
          }
          break;
        }
        default: {
        }
      }
    }
    // memory.fill(d, v, 1)  ==>  store8(d, v)
    if (bytes == 1LL) {
      return builder.makeStore(1,
                               offset,
                               align,
                               memFill->dest,
                               memFill->value,
                               Type::i32,
                               memFill->memory);
    }

    return nullptr;
  }

  // given a binary expression with equal children and no side effects in
  // either, we can fold various things
  Expression* optimizeBinaryWithEqualEffectlessChildren(Binary* binary) {
    // TODO add: perhaps worth doing 2*x if x is quite large?
    switch (binary->op) {
      case SubInt32:
      case XorInt32:
      case SubInt64:
      case XorInt64:
        return LiteralUtils::makeZero(binary->left->type, *getModule());
      case NeInt32:
      case LtSInt32:
      case LtUInt32:
      case GtSInt32:
      case GtUInt32:
      case NeInt64:
      case LtSInt64:
      case LtUInt64:
      case GtSInt64:
      case GtUInt64:
        return LiteralUtils::makeZero(Type::i32, *getModule());
      case AndInt32:
      case OrInt32:
      case AndInt64:
      case OrInt64:
        return binary->left;
      case EqInt32:
      case LeSInt32:
      case LeUInt32:
      case GeSInt32:
      case GeUInt32:
      case EqInt64:
      case LeSInt64:
      case LeUInt64:
      case GeSInt64:
      case GeUInt64:
        return LiteralUtils::makeFromInt32(1, Type::i32, *getModule());
      default:
        return nullptr;
    }
  }

  // Invert (negate) the opcode, so that it has the exact negative meaning as it
  // had before.
  BinaryOp invertBinaryOp(BinaryOp op) {
    switch (op) {
      case EqInt32:
        return NeInt32;
      case NeInt32:
        return EqInt32;
      case LtSInt32:
        return GeSInt32;
      case LtUInt32:
        return GeUInt32;
      case LeSInt32:
        return GtSInt32;
      case LeUInt32:
        return GtUInt32;
      case GtSInt32:
        return LeSInt32;
      case GtUInt32:
        return LeUInt32;
      case GeSInt32:
        return LtSInt32;
      case GeUInt32:
        return LtUInt32;

      case EqInt64:
        return NeInt64;
      case NeInt64:
        return EqInt64;
      case LtSInt64:
        return GeSInt64;
      case LtUInt64:
        return GeUInt64;
      case LeSInt64:
        return GtSInt64;
      case LeUInt64:
        return GtUInt64;
      case GtSInt64:
        return LeSInt64;
      case GtUInt64:
        return LeUInt64;
      case GeSInt64:
        return LtSInt64;
      case GeUInt64:
        return LtUInt64;

      case EqFloat32:
        return NeFloat32;
      case NeFloat32:
        return EqFloat32;

      case EqFloat64:
        return NeFloat64;
      case NeFloat64:
        return EqFloat64;

      default:
        return InvalidBinary;
    }
  }

  // Change the opcode so it is correct after reversing the operands. That is,
  // we had  X OP  Y  and we need OP' so that this is equivalent to that:
  //         Y OP' X
  BinaryOp reverseRelationalOp(BinaryOp op) {
    switch (op) {
      case EqInt32:
        return EqInt32;
      case NeInt32:
        return NeInt32;
      case LtSInt32:
        return GtSInt32;
      case LtUInt32:
        return GtUInt32;
      case LeSInt32:
        return GeSInt32;
      case LeUInt32:
        return GeUInt32;
      case GtSInt32:
        return LtSInt32;
      case GtUInt32:
        return LtUInt32;
      case GeSInt32:
        return LeSInt32;
      case GeUInt32:
        return LeUInt32;

      case EqInt64:
        return EqInt64;
      case NeInt64:
        return NeInt64;
      case LtSInt64:
        return GtSInt64;
      case LtUInt64:
        return GtUInt64;
      case LeSInt64:
        return GeSInt64;
      case LeUInt64:
        return GeUInt64;
      case GtSInt64:
        return LtSInt64;
      case GtUInt64:
        return LtUInt64;
      case GeSInt64:
        return LeSInt64;
      case GeUInt64:
        return LeUInt64;

      case EqFloat32:
        return EqFloat32;
      case NeFloat32:
        return NeFloat32;
      case LtFloat32:
        return GtFloat32;
      case LeFloat32:
        return GeFloat32;
      case GtFloat32:
        return LtFloat32;
      case GeFloat32:
        return LeFloat32;

      case EqFloat64:
        return EqFloat64;
      case NeFloat64:
        return NeFloat64;
      case LtFloat64:
        return GtFloat64;
      case LeFloat64:
        return GeFloat64;
      case GtFloat64:
        return LtFloat64;
      case GeFloat64:
        return LeFloat64;

      default:
        return InvalidBinary;
    }
  }

  BinaryOp makeUnsignedBinaryOp(BinaryOp op) {
    switch (op) {
      case DivSInt32:
        return DivUInt32;
      case RemSInt32:
        return RemUInt32;
      case ShrSInt32:
        return ShrUInt32;
      case LtSInt32:
        return LtUInt32;
      case LeSInt32:
        return LeUInt32;
      case GtSInt32:
        return GtUInt32;
      case GeSInt32:
        return GeUInt32;

      case DivSInt64:
        return DivUInt64;
      case RemSInt64:
        return RemUInt64;
      case ShrSInt64:
        return ShrUInt64;
      case LtSInt64:
        return LtUInt64;
      case LeSInt64:
        return LeUInt64;
      case GtSInt64:
        return GtUInt64;
      case GeSInt64:
        return GeUInt64;

      default:
        return InvalidBinary;
    }
  }

  bool shouldCanonicalize(Binary* binary) {
    if ((binary->op == SubInt32 || binary->op == SubInt64) &&
        binary->right->is<Const>() && !binary->left->is<Const>()) {
      return true;
    }
    if (Properties::isSymmetric(binary) || binary->isRelational()) {
      return true;
    }
    switch (binary->op) {
      case SubFloat32:
      case SubFloat64: {
        // Should apply  x - C  ->  x + (-C)
        return binary->right->is<Const>();
      }
      case AddFloat32:
      case MulFloat32:
      case AddFloat64:
      case MulFloat64: {
        // If the LHS is known to be non-NaN, the operands can commute.
        // We don't care about the RHS because right now we only know if
        // an expression is non-NaN if it is constant, but if the RHS is
        // constant, then this expression is already canonicalized.
        if (auto* c = binary->left->dynCast<Const>()) {
          return !c->value.isNaN();
        }
        return false;
      }
      default:
        return false;
    }
  }

  // Optimize an if-else or a select, something with a condition and two
  // arms with outputs.
  template<typename T> void optimizeTernary(T* curr) {
    using namespace Abstract;
    using namespace Match;
    Builder builder(*getModule());

    // If one arm is an operation and the other is an appropriate constant, we
    // can move the operation outside (where it may be further optimized), e.g.
    //
    //  (select
    //    (i32.eqz (X))
    //    (i32.const 0|1)
    //    (Y)
    //  )
    // =>
    //  (i32.eqz
    //    (select
    //      (X)
    //      (i32.const 1|0)
    //      (Y)
    //    )
    //  )
    //
    // Ignore unreachable code here; leave that for DCE.
    if (curr->type != Type::unreachable &&
        curr->ifTrue->type != Type::unreachable &&
        curr->ifFalse->type != Type::unreachable) {
      Unary* un;
      Const* c;
      auto check = [&](Expression* a, Expression* b) {
        return matches(b, bval(&c)) && matches(a, unary(&un, EqZ, any()));
      };
      if (check(curr->ifTrue, curr->ifFalse) ||
          check(curr->ifFalse, curr->ifTrue)) {
        // The new type of curr will be that of the value of the unary, as after
        // we move the unary out, its value is curr's direct child.
        auto newType = un->value->type;
        auto updateArm = [&](Expression* arm) -> Expression* {
          if (arm == un) {
            // This is the arm that had the eqz, which we need to remove.
            return un->value;
          } else {
            // This is the arm with the constant, which we need to flip.
            // Note that we also need to set the type to match the other arm.
            c->value =
              Literal::makeFromInt32(1 - c->value.getInteger(), newType);
            c->type = newType;
            return c;
          }
        };
        curr->ifTrue = updateArm(curr->ifTrue);
        curr->ifFalse = updateArm(curr->ifFalse);
        un->value = curr;
        curr->finalize();
        return replaceCurrent(un);
      }
    }

    {
      // Identical code on both arms can be folded out, e.g.
      //
      //  (select
      //    (i32.eqz (X))
      //    (i32.eqz (Y))
      //    (Z)
      //  )
      // =>
      //  (i32.eqz
      //    (select
      //      (X)
      //      (Y)
      //      (Z)
      //    )
      //  )
      //
      // Continue doing this while we can, noting the chain of moved expressions
      // as we go, then do a single replaceCurrent() at the end.
      SmallVector<Expression*, 1> chain;
      while (1) {
        // Ignore control flow structures (which are handled in MergeBlocks).
        if (!Properties::isControlFlowStructure(curr->ifTrue) &&
            ExpressionAnalyzer::shallowEqual(curr->ifTrue, curr->ifFalse)) {
          // TODO: consider the case with more than one child.
          ChildIterator ifTrueChildren(curr->ifTrue);
          if (ifTrueChildren.children.size() == 1) {
            // ifTrue and ifFalse's children will become the direct children of
            // curr, and so they must be compatible to allow for a proper new
            // type after the transformation.
            //
            // At minimum an LUB is required, as shown here:
            //
            //  (if
            //    (condition)
            //    (drop (i32.const 1))
            //    (drop (f64.const 2.0))
            //  )
            //
            // However, that may not be enough, as with nominal types we can
            // have things like this:
            //
            //  (if
            //    (condition)
            //    (struct.get $A 1 (..))
            //    (struct.get $B 1 (..))
            //  )
            //
            // It is possible that the LUB of $A and $B does not contain field
            // "1". With structural types this specific problem is not possible,
            // and it appears to be the case that with the GC MVP there is no
            // instruction that poses a problem, but in principle it can happen
            // there as well, if we add an instruction that returns the number
            // of fields in a type, for example. For that reason, and to avoid
            // a difference between structural and nominal typing here, disallow
            // subtyping in both. (Note: In that example, the problem only
            // happens because the type is not part of the struct.get - we infer
            // it from the reference. That is why after hoisting the struct.get
            // out, and computing a new type for the if that is now the child of
            // the single struct.get, we get a struct.get of a supertype. So in
            // principle we could fix this by modifying the IR as well, but the
            // problem is more general, so avoid that.)
            ChildIterator ifFalseChildren(curr->ifFalse);
            auto* ifTrueChild = *ifTrueChildren.begin();
            auto* ifFalseChild = *ifFalseChildren.begin();
            bool validTypes = ifTrueChild->type == ifFalseChild->type;

            // In addition, after we move code outside of curr then we need to
            // not change unreachability - if we did, we'd need to propagate
            // that further, and we leave such work to DCE and Vacuum anyhow.
            // This can happen in something like this for example, where the
            // outer type changes from i32 to unreachable if we move the
            // returns outside:
            //
            //  (if (result i32)
            //    (local.get $x)
            //    (return
            //      (local.get $y)
            //    )
            //    (return
            //      (local.get $z)
            //    )
            //  )
            assert(curr->ifTrue->type == curr->ifFalse->type);
            auto newOuterType = curr->ifTrue->type;
            if ((newOuterType == Type::unreachable) !=
                (curr->type == Type::unreachable)) {
              validTypes = false;
            }

            // If the expression we are about to move outside has side effects,
            // then we cannot do so in general with a select: we'd be reducing
            // the amount of the effects as well as moving them. For an if,
            // the side effects execute once, so there is no problem.
            // TODO: handle certain side effects when possible in select
            bool validEffects = std::is_same<T, If>::value ||
                                !ShallowEffectAnalyzer(
                                   getPassOptions(), *getModule(), curr->ifTrue)
                                   .hasSideEffects();

            // In addition, check for specific limitations of select.
            bool validChildren =
              !std::is_same<T, Select>::value ||
              Properties::canEmitSelectWithArms(ifTrueChild, ifFalseChild);

            if (validTypes && validEffects && validChildren) {
              // Replace ifTrue with its child.
              curr->ifTrue = ifTrueChild;
              // Relace ifFalse with its child, and reuse that node outside.
              auto* reuse = curr->ifFalse;
              curr->ifFalse = ifFalseChild;
              // curr's type may have changed, if the instructions we moved out
              // had different input types than output types.
              curr->finalize();
              // Point to curr from the code that is now outside of it.
              *ChildIterator(reuse).begin() = curr;
              if (!chain.empty()) {
                // We've already moved things out, so chain them to there. That
                // is, the end of the chain should now point to reuse (which
                // in turn already points to curr).
                *ChildIterator(chain.back()).begin() = reuse;
              }
              chain.push_back(reuse);
              continue;
            }
          }
        }
        break;
      }
      if (!chain.empty()) {
        // The beginning of the chain is the new top parent.
        return replaceCurrent(chain[0]);
      }
    }
  }
};

Pass* createOptimizeInstructionsPass() { return new OptimizeInstructions; }

} // namespace wasm
