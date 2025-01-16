/*
 * Copyright 2022 WebAssembly Community Group participants
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

#include <optional>
#include <variant>

#include "analysis/cfg.h"
#include "ir/bits.h"
#include "ir/branch-utils.h"
#include "ir/eh-utils.h"
#include "ir/gc-type-utils.h"
#include "ir/linear-execution.h"
#include "ir/local-graph.h"
#include "ir/module-utils.h"
#include "ir/possible-contents.h"
#include "support/insert_ordered.h"
#include "wasm.h"

namespace std {

std::ostream& operator<<(std::ostream& stream,
                         const wasm::PossibleContents& contents) {
  contents.dump(stream);
  return stream;
}

} // namespace std

namespace wasm {

PossibleContents PossibleContents::combine(const PossibleContents& a,
                                           const PossibleContents& b) {
  auto aType = a.getType();
  auto bType = b.getType();
  // First handle the trivial cases of them being equal, or one of them is
  // None or Many.
  if (a == b) {
    return a;
  }
  if (b.isNone()) {
    return a;
  }
  if (a.isNone()) {
    return b;
  }
  if (a.isMany()) {
    return a;
  }
  if (b.isMany()) {
    return b;
  }

  if (!aType.isRef() || !bType.isRef()) {
    // At least one is not a reference. The only possibility left for a useful
    // combination here is if they have the same type (since we've already ruled
    // out the case of them being equal). If they have the same type then
    // neither is a reference and we can emit an exact type (since subtyping is
    // not relevant for non-references).
    if (aType == bType) {
      return ExactType(aType);
    } else {
      return Many();
    }
  }

  // Special handling for references from here.

  if (a.isNull() && b.isNull()) {
    // These must be nulls in different hierarchies, otherwise a would have
    // been handled by the `a == b` case above.
    assert(aType != bType);
    return Many();
  }

  auto lub = Type::getLeastUpperBound(aType, bType);
  if (lub == Type::none) {
    // The types are not in the same hierarchy.
    return Many();
  }

  // From here we can assume there is a useful LUB.

  // Nulls can be combined in by just adding nullability to a type.
  if (a.isNull() || b.isNull()) {
    // Only one of them can be null here, since we already handled the case
    // where they were both null.
    assert(!a.isNull() || !b.isNull());
    // If only one is a null then we can use the type info from the b, and
    // just add in nullability. For example, a literal of type T and a null
    // becomes an exact type of T that allows nulls, and so forth.
    auto mixInNull = [](ConeType cone) {
      cone.type = Type(cone.type.getHeapType(), Nullable);
      return cone;
    };
    if (!a.isNull()) {
      return mixInNull(a.getCone());
    } else if (!b.isNull()) {
      return mixInNull(b.getCone());
    }
  }

  // Find a ConeType that describes both inputs, using the shared ancestor which
  // is the LUB. We need to find how big a cone we need: the cone must be big
  // enough to contain both the inputs.
  auto aDepth = a.getCone().depth;
  auto bDepth = b.getCone().depth;
  Index newDepth;
  if (aDepth == FullDepth || bDepth == FullDepth) {
    // At least one has full (infinite) depth, so we know the new depth must
    // be the same.
    newDepth = FullDepth;
  } else {
    // The depth we need under the lub is how far from the lub we are, plus
    // the depth of our cone.
    // TODO: we could make a single loop that also does the LUB, at the same
    // time, and also avoids calling getDepth() which loops once more?
    auto aDepthFromRoot = aType.getHeapType().getDepth();
    auto bDepthFromRoot = bType.getHeapType().getDepth();
    auto lubDepthFromRoot = lub.getHeapType().getDepth();
    assert(lubDepthFromRoot <= aDepthFromRoot);
    assert(lubDepthFromRoot <= bDepthFromRoot);
    Index aDepthUnderLub = aDepthFromRoot - lubDepthFromRoot + aDepth;
    Index bDepthUnderLub = bDepthFromRoot - lubDepthFromRoot + bDepth;

    // The total cone must be big enough to contain all the above.
    newDepth = std::max(aDepthUnderLub, bDepthUnderLub);
  }

  return ConeType{lub, newDepth};
}

void PossibleContents::intersect(const PossibleContents& other) {
  // This does not yet handle all possible content.
  assert(other.isFullConeType() || other.isLiteral() || other.isNone());

  if (*this == other) {
    // Nothing changes.
    return;
  }

  if (!haveIntersection(*this, other)) {
    // There is no intersection at all.
    // Note that this code path handles |this| or |other| being None.
    value = None();
    return;
  }

  if (isSubContents(other, *this)) {
    // The intersection is just |other|.
    // Note that this code path handles |this| being Many.
    value = other.value;
    return;
  }

  if (isSubContents(*this, other)) {
    // The intersection is just |this|.
    return;
  }

  if (isLiteral() || other.isLiteral()) {
    // We've ruled out either being a subcontents of the other. A literal has
    // no other intersection possibility.
    value = None();
    return;
  }

  auto type = getType();
  auto otherType = other.getType();
  auto heapType = type.getHeapType();
  auto otherHeapType = otherType.getHeapType();

  // If both inputs are nullable then the intersection is nullable as well.
  auto nullability =
    type.isNullable() && otherType.isNullable() ? Nullable : NonNullable;

  auto setNoneOrNull = [&]() {
    if (nullability == Nullable) {
      value = Literal::makeNull(heapType);
    } else {
      value = None();
    }
  };

  // If the heap types are not compatible then they are in separate hierarchies
  // and there is no intersection, aside from possibly a null of the bottom
  // type.
  auto isSubType = HeapType::isSubType(heapType, otherHeapType);
  auto otherIsSubType = HeapType::isSubType(otherHeapType, heapType);
  if (!isSubType && !otherIsSubType) {
    if (heapType.getBottom() == otherHeapType.getBottom()) {
      setNoneOrNull();
    } else {
      value = None();
    }
    return;
  }

  // The heap types are compatible, so intersect the cones.
  auto depthFromRoot = heapType.getDepth();
  auto otherDepthFromRoot = otherHeapType.getDepth();

  // To compute the new cone, find the new heap type for it, and to compute its
  // depth, consider the adjustments to the existing depths that stem from the
  // choice of new heap type.
  HeapType newHeapType;

  if (depthFromRoot < otherDepthFromRoot) {
    newHeapType = otherHeapType;
  } else {
    newHeapType = heapType;
  }

  // Note the global's information, if we started as a global. In that case, the
  // code below will refine our type but we can remain a global, which we will
  // accomplish by restoring our global status at the end.
  std::optional<Name> globalName;
  if (isGlobal()) {
    globalName = getGlobal();
  }

  auto newType = Type(newHeapType, nullability);

  // By assumption |other| has full depth. Consider the other cone in |this|.
  if (hasFullCone()) {
    // Both are full cones, so the result is as well.
    value = FullConeType(newType);
  } else {
    // The result is a partial cone. If the cone starts in |otherHeapType| then
    // we need to adjust the depth down, since it will be smaller than the
    // original cone:
    /*
    //                             ..
    //                            /
    //              otherHeapType
    //            /               \
    //   heapType                  ..
    //            \
    */
    // E.g. if |this| is a cone of depth 10, and |otherHeapType| is an immediate
    // subtype of |this|, then the new cone must be of depth 9.
    auto newDepth = getCone().depth;
    if (newHeapType == otherHeapType) {
      assert(depthFromRoot <= otherDepthFromRoot);
      auto reduction = otherDepthFromRoot - depthFromRoot;
      if (reduction > newDepth) {
        // The cone on heapType does not even reach the cone on otherHeapType,
        // so the result is not a cone.
        setNoneOrNull();
        return;
      }
      newDepth -= reduction;
    }

    value = ConeType{newType, newDepth};
  }

  if (globalName) {
    // Restore the global but keep the new and refined type.
    value = GlobalInfo{*globalName, getType()};
  }
}

bool PossibleContents::haveIntersection(const PossibleContents& a,
                                        const PossibleContents& b) {
  if (a.isNone() || b.isNone()) {
    // One is the empty set, so nothing can intersect here.
    return false;
  }

  if (a.isMany() || b.isMany()) {
    // One is the set of all things, so definitely something can intersect since
    // we've ruled out an empty set for both.
    return true;
  }

  if (a == b) {
    // The intersection is equal to them.
    return true;
  }

  auto aType = a.getType();
  auto bType = b.getType();

  if (!aType.isRef() || !bType.isRef()) {
    // At least one is not a reference. The only way they can intersect is if
    // the type is identical, and they are not both literals (we've already
    // ruled out them being identical earlier).
    return aType == bType && (!a.isLiteral() || !b.isLiteral());
  }

  // From here on we focus on references.

  auto aHeapType = aType.getHeapType();
  auto bHeapType = bType.getHeapType();

  if (aType.isNullable() && bType.isNullable() &&
      aHeapType.getBottom() == bHeapType.getBottom()) {
    // A compatible null is possible on both sides.
    return true;
  }

  // We ruled out having a compatible null on both sides. If one is simply a
  // null then no chance for an intersection remains.
  if (a.isNull() || b.isNull()) {
    return false;
  }

  auto aSubB = HeapType::isSubType(aHeapType, bHeapType);
  auto bSubA = HeapType::isSubType(bHeapType, aHeapType);
  if (!aSubB && !bSubA) {
    // No type can appear in both a and b, so the types differ, so the values
    // do not overlap.
    return false;
  }

  // From here on we focus on references and can ignore the case of null - any
  // intersection must be of a non-null value, so we can focus on the heap
  // types.

  auto aDepthFromRoot = aHeapType.getDepth();
  auto bDepthFromRoot = bHeapType.getDepth();

  if (aSubB) {
    // A is a subtype of B. For there to be an intersection we need their cones
    // to intersect, that is, to rule out the case where the cone from B is not
    // deep enough to reach A.
    assert(aDepthFromRoot >= bDepthFromRoot);
    return aDepthFromRoot - bDepthFromRoot <= b.getCone().depth;
  } else if (bSubA) {
    assert(bDepthFromRoot >= aDepthFromRoot);
    return bDepthFromRoot - aDepthFromRoot <= a.getCone().depth;
  } else {
    WASM_UNREACHABLE("we ruled out no subtyping before");
  }

  // TODO: we can also optimize things like different Literals, but existing
  //       passes do such things already so it is low priority.
}

bool PossibleContents::isSubContents(const PossibleContents& a,
                                     const PossibleContents& b) {
  if (a == b) {
    return true;
  }

  if (a.isNone()) {
    return true;
  }

  if (b.isNone()) {
    return false;
  }

  if (a.isMany()) {
    return false;
  }

  if (b.isMany()) {
    return true;
  }

  if (a.isLiteral()) {
    // Note we already checked for |a == b| above. We need b to be a set that
    // contains the literal a.
    return !b.isLiteral() && Type::isSubType(a.getType(), b.getType());
  }

  if (b.isLiteral()) {
    return false;
  }

  if (b.isFullConeType()) {
    if (a.isNull()) {
      return b.getType().isNullable();
    }
    return Type::isSubType(a.getType(), b.getType());
  }

  if (a.isFullConeType()) {
    // We've already ruled out b being a full cone type before.
    return false;
  }

  WASM_UNREACHABLE("unhandled case of isSubContents");
}

namespace {

// We are going to do a very large flow operation, potentially, as we create
// a Location for every interesting part in the entire wasm, and some of those
// places will have lots of links (like a struct field may link out to every
// single struct.get of that type), so we must make the data structures here
// as efficient as possible. Towards that goal, we work with location
// *indexes* where possible, which are small (32 bits) and do not require any
// complex hashing when we use them in sets or maps.
//
// Note that we do not use indexes everywhere, since the initial analysis is
// done in parallel, and we do not have a fixed indexing of locations yet. When
// we merge the parallel data we create that indexing, and use indexes from then
// on.
using LocationIndex = uint32_t;

#ifndef NDEBUG
// Assert on not having duplicates in a vector.
template<typename T> void disallowDuplicates(const T& targets) {
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::unordered_set<LocationIndex> uniqueTargets;
  for (const auto& target : targets) {
    uniqueTargets.insert(target);
  }
  assert(uniqueTargets.size() == targets.size());
#endif
}
#endif

// A link indicates a flow of content from one location to another. For
// example, if we do a local.get and return that value from a function, then
// we have a link from the ExpressionLocation of that local.get to a
// ResultLocation.
template<typename T> struct Link {
  T from;
  T to;

  bool operator==(const Link<T>& other) const {
    return from == other.from && to == other.to;
  }
};

using LocationLink = Link<Location>;
using IndexLink = Link<LocationIndex>;

} // anonymous namespace

} // namespace wasm

namespace std {

template<> struct hash<wasm::LocationLink> {
  size_t operator()(const wasm::LocationLink& loc) const {
    return std::hash<std::pair<wasm::Location, wasm::Location>>{}(
      {loc.from, loc.to});
  }
};

template<> struct hash<wasm::IndexLink> {
  size_t operator()(const wasm::IndexLink& loc) const {
    return std::hash<std::pair<wasm::LocationIndex, wasm::LocationIndex>>{}(
      {loc.from, loc.to});
  }
};

} // namespace std

namespace wasm {

namespace {

// The data we gather from each function, as we process them in parallel. Later
// this will be merged into a single big graph.
struct CollectedFuncInfo {
  // All the links we found in this function. Rarely are there duplicates
  // in this list (say when writing to the same global location from another
  // global location), and we do not try to deduplicate here, just store them in
  // a plain array for now, which is faster (later, when we merge all the info
  // from the functions, we need to deduplicate anyhow).
  std::vector<LocationLink> links;

  // All the roots of the graph, that is, places that begin by containing some
  // particular content. That includes i32.const, ref.func, struct.new, etc. All
  // possible contents in the rest of the graph flow from such places.
  //
  // The vector here is of the location of the root and then its contents.
  std::vector<std::pair<Location, PossibleContents>> roots;

  // In some cases we need to know the parent of the expression. Consider this:
  //
  //  (struct.set $A k
  //    (local.get $ref)
  //    (local.get $value)
  //  )
  //
  // Imagine that the first local.get, for $ref, receives a new value. That can
  // affect where the struct.set sends values: if previously that local.get had
  // no possible contents, and now it does, then we have DataLocations to
  // update. Likewise, when the second local.get is updated we must do the same,
  // but again which DataLocations we update depends on the ref passed to the
  // struct.set. To handle such things, we set add a childParent link, and then
  // when we update the child we can find the parent and handle any special
  // behavior we need there.
  std::unordered_map<Expression*, Expression*> childParents;

  // All functions that might be called from the outside. Any RefFunc suggests
  // that, in open world. (We could be more precise and use our flow analysis to
  // see which, in fact, flow outside, but it is unclear how useful that would
  // be. Anyhow, closed-world is more important to optimize, and avoids this.)
  std::unordered_set<Name> calledFromOutside;
};

// Does a walk while maintaining a map of names of branch targets to those
// expressions, so they can be found by their name.
// TODO: can this replace ControlFlowWalker in other places?
template<typename SubType, typename VisitorType = Visitor<SubType>>
struct BreakTargetWalker : public PostWalker<SubType, VisitorType> {
  std::unordered_map<Name, Expression*> breakTargets;

  Expression* findBreakTarget(Name name) { return breakTargets[name]; }

  static void scan(SubType* self, Expression** currp) {
    auto* curr = *currp;
    BranchUtils::operateOnScopeNameDefs(
      curr, [&](Name name) { self->breakTargets[name] = curr; });

    PostWalker<SubType, VisitorType>::scan(self, currp);
  }
};

// Walk the wasm and find all the links we need to care about, and the locations
// and roots related to them. This builds up a CollectedFuncInfo data structure.
// After all InfoCollectors run, those data structures will be merged and the
// main flow will begin.
struct InfoCollector
  : public BreakTargetWalker<InfoCollector, OverriddenVisitor<InfoCollector>> {
  CollectedFuncInfo& info;
  const PassOptions& options;

  InfoCollector(CollectedFuncInfo& info, const PassOptions& options)
    : info(info), options(options) {}

  // Check if a type is relevant for us. If not, we can ignore it entirely.
  bool isRelevant(Type type) {
    if (type == Type::unreachable || type == Type::none) {
      return false;
    }
    if (type.isTuple()) {
      for (auto t : type) {
        if (isRelevant(t)) {
          return true;
        }
      }
    }
    return true;
  }

  bool isRelevant(Signature sig) {
    return isRelevant(sig.params) || isRelevant(sig.results);
  }

  bool isRelevant(Expression* curr) { return curr && isRelevant(curr->type); }

  template<typename T> bool isRelevant(const T& vec) {
    for (auto* expr : vec) {
      if (isRelevant(expr->type)) {
        return true;
      }
    }
    return false;
  }

  // Each visit*() call is responsible for connecting the children of a node to
  // that node. Responsibility for connecting the node's output to anywhere
  // else (another expression or the function itself, if we are at the top
  // level) is the responsibility of the outside.

  void visitBlock(Block* curr) {
    if (curr->list.empty()) {
      return;
    }

    // The final item in the block can flow a value to here as well.
    receiveChildValue(curr->list.back(), curr);
  }
  void visitIf(If* curr) {
    // Each arm may flow out a value.
    receiveChildValue(curr->ifTrue, curr);
    receiveChildValue(curr->ifFalse, curr);
  }
  void visitLoop(Loop* curr) { receiveChildValue(curr->body, curr); }
  void visitBreak(Break* curr) {
    // Connect the value (if present) to the break target.
    handleBreakValue(curr);

    // The value may also flow through in a br_if (the type will indicate that,
    // which receiveChildValue will notice).
    receiveChildValue(curr->value, curr);
  }
  void visitSwitch(Switch* curr) { handleBreakValue(curr); }
  void visitLoad(Load* curr) {
    // We could infer the exact type here, but as no subtyping is possible, it
    // would have no benefit, so just add a generic root (which will be "Many").
    // See the comment on the ContentOracle class.
    addRoot(curr);
  }
  void visitStore(Store* curr) {}
  void visitAtomicRMW(AtomicRMW* curr) { addRoot(curr); }
  void visitAtomicCmpxchg(AtomicCmpxchg* curr) { addRoot(curr); }
  void visitAtomicWait(AtomicWait* curr) { addRoot(curr); }
  void visitAtomicNotify(AtomicNotify* curr) { addRoot(curr); }
  void visitAtomicFence(AtomicFence* curr) {}
  void visitSIMDExtract(SIMDExtract* curr) { addRoot(curr); }
  void visitSIMDReplace(SIMDReplace* curr) { addRoot(curr); }
  void visitSIMDShuffle(SIMDShuffle* curr) { addRoot(curr); }
  void visitSIMDTernary(SIMDTernary* curr) { addRoot(curr); }
  void visitSIMDShift(SIMDShift* curr) { addRoot(curr); }
  void visitSIMDLoad(SIMDLoad* curr) { addRoot(curr); }
  void visitSIMDLoadStoreLane(SIMDLoadStoreLane* curr) { addRoot(curr); }
  void visitMemoryInit(MemoryInit* curr) {}
  void visitDataDrop(DataDrop* curr) {}
  void visitMemoryCopy(MemoryCopy* curr) {}
  void visitMemoryFill(MemoryFill* curr) {}
  void visitConst(Const* curr) {
    addRoot(curr, PossibleContents::literal(curr->value));
  }
  void visitUnary(Unary* curr) {
    // We could optimize cases like this using interpreter integration: if the
    // input is a Literal, we could interpret the Literal result. However, if
    // the input is a literal then the GUFA pass will emit a Const there, and
    // the Precompute pass can use that later to interpret a result. That is,
    // the input we need here, a constant, is already something GUFA can emit as
    // an output. As a result, integrating the interpreter here would perhaps
    // make compilation require fewer steps, but it wouldn't let us optimize
    // more than we could before.
    addRoot(curr);
  }
  void visitBinary(Binary* curr) { addRoot(curr); }
  void visitSelect(Select* curr) {
    receiveChildValue(curr->ifTrue, curr);
    receiveChildValue(curr->ifFalse, curr);
  }
  void visitDrop(Drop* curr) {}
  void visitMemorySize(MemorySize* curr) { addRoot(curr); }
  void visitMemoryGrow(MemoryGrow* curr) { addRoot(curr); }
  void visitRefNull(RefNull* curr) {
    addRoot(
      curr,
      PossibleContents::literal(Literal::makeNull(curr->type.getHeapType())));
  }
  void visitRefIsNull(RefIsNull* curr) {
    // TODO: Optimize when possible. For example, if we can infer an exact type
    //       here which allows us to know the result then we should do so. This
    //       is unlike the case in visitUnary, above: the information that lets
    //       us optimize *cannot* be written into Binaryen IR (unlike a Literal)
    //       so using it during this pass allows us to optimize new things.
    addRoot(curr);
  }
  void visitRefFunc(RefFunc* curr) {
    addRoot(
      curr,
      PossibleContents::literal(Literal(curr->func, curr->type.getHeapType())));

    // The presence of a RefFunc indicates the function may be called
    // indirectly, so add the relevant connections for this particular function.
    // We do so here in the RefFunc so that we only do it for functions that
    // actually have a RefFunc.
    auto* func = getModule()->getFunction(curr->func);
    for (Index i = 0; i < func->getParams().size(); i++) {
      info.links.push_back(
        {SignatureParamLocation{func->type, i}, ParamLocation{func, i}});
    }
    for (Index i = 0; i < func->getResults().size(); i++) {
      info.links.push_back(
        {ResultLocation{func, i}, SignatureResultLocation{func->type, i}});
    }

    if (!options.closedWorld) {
      info.calledFromOutside.insert(curr->func);
    }
  }
  void visitRefEq(RefEq* curr) {
    addRoot(curr);
  }
  void visitTableGet(TableGet* curr) {
    // TODO: be more precise
    addRoot(curr);
  }
  void visitTableSet(TableSet* curr) {}
  void visitTableSize(TableSize* curr) { addRoot(curr); }
  void visitTableGrow(TableGrow* curr) { addRoot(curr); }
  void visitTableFill(TableFill* curr) { addRoot(curr); }
  void visitTableCopy(TableCopy* curr) { addRoot(curr); }
  void visitTableInit(TableInit* curr) {}

  void visitNop(Nop* curr) {}
  void visitUnreachable(Unreachable* curr) {}

#ifndef NDEBUG
  // For now we only handle pops in a catch body, see visitTry(). To check for
  // errors, use counter of the pops we handled and all the pops; those sums
  // must agree at the end, or else we've seen something we can't handle.
  Index totalPops = 0;
  Index handledPops = 0;
#endif

  void visitPop(Pop* curr) {
#ifndef NDEBUG
    totalPops++;
#endif
  }
  void visitRefI31(RefI31* curr) {
    // TODO: optimize like struct references
    addRoot(curr);
  }
  void visitI31Get(I31Get* curr) {
    // TODO: optimize like struct references
    addRoot(curr);
  }

  void visitRefCast(RefCast* curr) { receiveChildValue(curr->ref, curr); }
  void visitRefTest(RefTest* curr) { addRoot(curr); }
  void visitBrOn(BrOn* curr) {
    // TODO: optimize when possible
    handleBreakValue(curr);
    receiveChildValue(curr->ref, curr);
  }
  void visitRefAs(RefAs* curr) {
    if (curr->op == ExternConvertAny || curr->op == AnyConvertExtern) {
      // The external conversion ops emit something of a completely different
      // type, which we must mark as a root.
      addRoot(curr);
      return;
    }

    // All other RefAs operations flow values through while refining them (the
    // filterExpressionContents method will handle the refinement
    // automatically).
    receiveChildValue(curr->value, curr);
  }

  void visitLocalSet(LocalSet* curr) {
    if (!isRelevant(curr->value->type)) {
      return;
    }

    // Tees flow out the value (receiveChildValue will see if this is a tee
    // based on the type, automatically).
    receiveChildValue(curr->value, curr);

    // We handle connecting local.gets to local.sets below, in visitFunction.
  }
  void visitLocalGet(LocalGet* curr) {
    // We handle connecting local.gets to local.sets below, in visitFunction.
  }

  // Globals read and write from their location.
  void visitGlobalGet(GlobalGet* curr) {
    if (isRelevant(curr->type)) {
      // FIXME: we allow tuples in globals, so GlobalLocation needs a tupleIndex
      //        and we should loop here.
      assert(!curr->type.isTuple());
      info.links.push_back(
        {GlobalLocation{curr->name}, ExpressionLocation{curr, 0}});
    }
  }
  void visitGlobalSet(GlobalSet* curr) {
    if (isRelevant(curr->value->type)) {
      info.links.push_back(
        {ExpressionLocation{curr->value, 0}, GlobalLocation{curr->name}});
    }
  }

  // Iterates over a list of children and adds links to parameters and results
  // as needed. The param/result functions receive the index and create the
  // proper location for it.
  template<typename T>
  void handleCall(T* curr,
                  std::function<Location(Index)> makeParamLocation,
                  std::function<Location(Index)> makeResultLocation) {
    Index i = 0;
    for (auto* operand : curr->operands) {
      if (isRelevant(operand->type)) {
        info.links.push_back(
          {ExpressionLocation{operand, 0}, makeParamLocation(i)});
      }
      i++;
    }

    // Add results, if anything flows out.
    for (Index i = 0; i < curr->type.size(); i++) {
      if (isRelevant(curr->type[i])) {
        info.links.push_back(
          {makeResultLocation(i), ExpressionLocation{curr, i}});
      }
    }

    // If this is a return call then send the result to the function return as
    // well.
    if (curr->isReturn) {
      auto results = getFunction()->getResults();
      for (Index i = 0; i < results.size(); i++) {
        auto result = results[i];
        if (isRelevant(result)) {
          info.links.push_back(
            {makeResultLocation(i), ResultLocation{getFunction(), i}});
        }
      }
    }
  }

  // Calls send values to params in their possible targets, and receive
  // results.

  template<typename T> void handleDirectCall(T* curr, Name targetName) {
    auto* target = getModule()->getFunction(targetName);
    handleCall(
      curr,
      [&](Index i) {
        assert(i <= target->getParams().size());
        return ParamLocation{target, i};
      },
      [&](Index i) {
        assert(i <= target->getResults().size());
        return ResultLocation{target, i};
      });
  }
  template<typename T> void handleIndirectCall(T* curr, HeapType targetType) {
    // If the heap type is not a signature, which is the case for a bottom type
    // (null) then nothing can be called.
    if (!targetType.isSignature()) {
      assert(targetType.isBottom());
      return;
    }
    handleCall(
      curr,
      [&](Index i) {
        assert(i <= targetType.getSignature().params.size());
        return SignatureParamLocation{targetType, i};
      },
      [&](Index i) {
        assert(i <= targetType.getSignature().results.size());
        return SignatureResultLocation{targetType, i};
      });
  }
  template<typename T> void handleIndirectCall(T* curr, Type targetType) {
    // If the type is unreachable, nothing can be called (and there is no heap
    // type to get).
    if (targetType != Type::unreachable) {
      handleIndirectCall(curr, targetType.getHeapType());
    }
  }

  void visitCall(Call* curr) {
    Name targetName;
    if (!Intrinsics(*getModule()).isCallWithoutEffects(curr)) {
      // This is just a normal call.
      handleDirectCall(curr, curr->target);
      return;
    }
    // A call-without-effects receives a function reference and calls it, the
    // same as a CallRef. When we have a flag for non-closed-world, we should
    // handle this automatically by the reference flowing out to an import,
    // which is what binaryen intrinsics look like. For now, to support use
    // cases of a closed world but that also use this intrinsic, handle the
    // intrinsic specifically here. (Without that, the closed world assumption
    // makes us ignore the function ref that flows to an import, so we are not
    // aware that it is actually called.)
    auto* target = curr->operands.back();

    // We must ignore the last element when handling the call - the target is
    // used to perform the call, and not sent during the call.
    curr->operands.pop_back();

    if (auto* refFunc = target->dynCast<RefFunc>()) {
      // We can see exactly where this goes.
      handleDirectCall(curr, refFunc->func);
    } else {
      // We can't see where this goes. We must be pessimistic and assume it
      // can call anything of the proper type, the same as a CallRef. (We could
      // look at the possible contents of |target| during the flow, but that
      // would require special logic like we have for StructGet etc., and the
      // intrinsics will be lowered away anyhow, so just running after that is
      // a workaround.)
      handleIndirectCall(curr, target->type);
    }

    // Restore the target.
    curr->operands.push_back(target);
  }
  void visitCallIndirect(CallIndirect* curr) {
    // TODO: the table identity could also be used here
    // TODO: optimize the call target like CallRef
    handleIndirectCall(curr, curr->heapType);
  }
  void visitCallRef(CallRef* curr) {
    handleIndirectCall(curr, curr->target->type);
  }

  // Creates a location for a null of a particular type and adds a root for it.
  // Such roots are where the default value of an i32 local comes from, or the
  // value in a ref.null.
  Location getNullLocation(Type type) {
    auto location = NullLocation{type};
    addRoot(location, PossibleContents::literal(Literal::makeZero(type)));
    return location;
  }

  // Iterates over a list of children and adds links from them. The target of
  // those link is created using a function that is passed in, which receives
  // the index of the child.
  void linkChildList(ExpressionList& operands,
                     std::function<Location(Index)> makeTarget) {
    Index i = 0;
    for (auto* operand : operands) {
      // This helper is not used from places that allow a tuple (hence we can
      // hardcode the index 0 a few lines down).
      assert(!operand->type.isTuple());

      if (isRelevant(operand->type)) {
        info.links.push_back({ExpressionLocation{operand, 0}, makeTarget(i)});
      }
      i++;
    }
  }

  void visitStructNew(StructNew* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    auto type = curr->type.getHeapType();
    if (curr->isWithDefault()) {
      // Link the default values to the struct's fields.
      auto& fields = type.getStruct().fields;
      for (Index i = 0; i < fields.size(); i++) {
        info.links.push_back(
          {getNullLocation(fields[i].type), DataLocation{type, i}});
      }
    } else {
      // Link the operands to the struct's fields.
      linkChildList(curr->operands, [&](Index i) {
        return DataLocation{type, i};
      });
    }
    addRoot(curr, PossibleContents::exactType(curr->type));
  }
  void visitArrayNew(ArrayNew* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    auto type = curr->type.getHeapType();
    if (curr->init) {
      info.links.push_back(
        {ExpressionLocation{curr->init, 0}, DataLocation{type, 0}});
    } else {
      info.links.push_back(
        {getNullLocation(type.getArray().element.type), DataLocation{type, 0}});
    }
    addRoot(curr, PossibleContents::exactType(curr->type));
  }
  void visitArrayNewData(ArrayNewData* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    addRoot(curr, PossibleContents::exactType(curr->type));
    auto heapType = curr->type.getHeapType();
    Type elemType = heapType.getArray().element.type;
    addRoot(DataLocation{heapType, 0}, PossibleContents::fromType(elemType));
  }
  void visitArrayNewElem(ArrayNewElem* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    addRoot(curr, PossibleContents::exactType(curr->type));
    auto heapType = curr->type.getHeapType();
    Type segType = getModule()->getElementSegment(curr->segment)->type;
    addRoot(DataLocation{heapType, 0}, PossibleContents::fromType(segType));
    return;
  }
  void visitArrayNewFixed(ArrayNewFixed* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    if (!curr->values.empty()) {
      auto type = curr->type.getHeapType();
      linkChildList(curr->values, [&](Index i) {
        // The index i is ignored, as we do not track indexes in Arrays -
        // everything is modeled as if at index 0.
        return DataLocation{type, 0};
      });
    }
    addRoot(curr, PossibleContents::exactType(curr->type));
  }

  // Struct operations access the struct fields' locations.
  void visitStructGet(StructGet* curr) {
    if (!isRelevant(curr->ref)) {
      // If references are irrelevant then we will ignore them, and we won't
      // have information about this struct.get's reference, which means we
      // won't have information to compute relevant values for this struct.get.
      // Instead, just mark this as an unknown value (root).
      addRoot(curr);
      return;
    }
    // The struct.get will receive different values depending on the contents
    // in the reference, so mark us as the parent of the ref, and we will
    // handle all of this in a special way during the flow. Note that we do
    // not even create a DataLocation here; anything that we need will be
    // added during the flow.
    addChildParentLink(curr->ref, curr);
  }
  void visitStructSet(StructSet* curr) {
    if (curr->ref->type == Type::unreachable) {
      return;
    }
    // See comment on visitStructGet. Here we also connect the value.
    addChildParentLink(curr->ref, curr);
    addChildParentLink(curr->value, curr);
  }
  void visitStructRMW(StructRMW* curr) {
    if (curr->ref->type == Type::unreachable) {
      return;
    }
    // TODO: Model the modification part of the RMW in addition to the read and
    // the write.
    addRoot(curr);
  }
  void visitStructCmpxchg(StructCmpxchg* curr) {
    if (curr->ref->type == Type::unreachable) {
      return;
    }
    // TODO: Model the modification part of the RMW in addition to the read and
    // the write.
    addRoot(curr);
  }
  // Array operations access the array's location, parallel to how structs work.
  void visitArrayGet(ArrayGet* curr) {
    if (!isRelevant(curr->ref)) {
      addRoot(curr);
      return;
    }
    addChildParentLink(curr->ref, curr);
  }
  void visitArraySet(ArraySet* curr) {
    if (curr->ref->type == Type::unreachable) {
      return;
    }
    addChildParentLink(curr->ref, curr);
    addChildParentLink(curr->value, curr);
  }

  void visitArrayLen(ArrayLen* curr) {
    // TODO: optimize when possible (perhaps we can infer a Literal for the
    //       length)
    addRoot(curr);
  }
  void visitArrayCopy(ArrayCopy* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    // Our flow handling of GC data is not simple: we have special code for each
    // read and write instruction. Therefore, to avoid adding special code for
    // ArrayCopy, model it as a combination of an ArrayRead and ArrayWrite, by
    // just emitting fake expressions for those. The fake expressions are not
    // part of the main IR, which is potentially confusing during debugging,
    // however, which is a downside.
    Builder builder(*getModule());
    auto* get =
      builder.makeArrayGet(curr->srcRef, curr->srcIndex, curr->srcRef->type);
    visitArrayGet(get);
    auto* set = builder.makeArraySet(curr->destRef, curr->destIndex, get);
    visitArraySet(set);
  }
  void visitArrayFill(ArrayFill* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    // See ArrayCopy, above.
    Builder builder(*getModule());
    auto* set = builder.makeArraySet(curr->ref, curr->index, curr->value);
    visitArraySet(set);
  }
  template<typename ArrayInit> void visitArrayInit(ArrayInit* curr) {
    // Check for both unreachability and a bottom type. In either case we have
    // no work to do, and would error on an assertion below in finding the array
    // type.
    auto field = GCTypeUtils::getField(curr->ref->type);
    if (!field) {
      return;
    }
    // See ArrayCopy, above. Here an additional complexity is that we need to
    // model the read from the segment. As in TableGet, for now we just assume
    // any value is possible there (a root in the graph), which we set up
    // manually here as a fake unknown value, using a fake local.get that we
    // root.
    // TODO: be more precise about what is in the table
    auto valueType = field->type;
    Builder builder(*getModule());
    auto* get = builder.makeLocalGet(-1, valueType);
    addRoot(get);
    auto* set = builder.makeArraySet(curr->ref, curr->index, get);
    visitArraySet(set);
  }
  void visitArrayInitData(ArrayInitData* curr) { visitArrayInit(curr); }
  void visitArrayInitElem(ArrayInitElem* curr) { visitArrayInit(curr); }
  void visitStringNew(StringNew* curr) {
    if (curr->type == Type::unreachable) {
      return;
    }
    addRoot(curr, PossibleContents::exactType(curr->type));
  }
  void visitStringConst(StringConst* curr) {
    addRoot(curr,
            PossibleContents::literal(Literal(std::string(curr->string.str))));
  }
  void visitStringMeasure(StringMeasure* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitStringEncode(StringEncode* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitStringConcat(StringConcat* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitStringEq(StringEq* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitStringWTF16Get(StringWTF16Get* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitStringSliceWTF(StringSliceWTF* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }

  // TODO: Model which throws can go to which catches. For now, anything thrown
  //       is sent to the location of that tag, and any catch of that tag can
  //       read them.
  void visitTry(Try* curr) {
    receiveChildValue(curr->body, curr);
    for (auto* catchBody : curr->catchBodies) {
      receiveChildValue(catchBody, curr);
    }

    auto numTags = curr->catchTags.size();
    for (Index tagIndex = 0; tagIndex < numTags; tagIndex++) {
      auto tag = curr->catchTags[tagIndex];
      auto* body = curr->catchBodies[tagIndex];

      auto params = getModule()->getTag(tag)->params();
      if (params.size() == 0) {
        continue;
      }

      // Find the pop of the tag's contents. The body must start with such a
      // pop, which might be of a tuple.
      auto* pop = EHUtils::findPop(body);
      // There must be a pop since we checked earlier if it was an empty tag,
      // and would not reach here.
      assert(pop);
      assert(pop->type.size() == params.size());
      for (Index i = 0; i < params.size(); i++) {
        if (isRelevant(params[i])) {
          info.links.push_back(
            {TagLocation{tag, i}, ExpressionLocation{pop, i}});
        }
      }

#ifndef NDEBUG
      // This pop was in the position we can handle, note that (see visitPop
      // for details).
      handledPops++;
#endif
    }
  }
  void visitTryTable(TryTable* curr) {
    receiveChildValue(curr->body, curr);

    // Connect caught tags with their branch targets, and materialize non-null
    // exnref values.
    auto numTags = curr->catchTags.size();
    for (Index tagIndex = 0; tagIndex < numTags; tagIndex++) {
      auto tag = curr->catchTags[tagIndex];
      auto target = curr->catchDests[tagIndex];

      Index exnrefIndex = 0;
      if (tag.is()) {
        auto params = getModule()->getTag(tag)->params();

        for (Index i = 0; i < params.size(); i++) {
          if (isRelevant(params[i])) {
            info.links.push_back(
              {TagLocation{tag, i}, getBreakTargetLocation(target, i)});
          }
        }

        exnrefIndex = params.size();
      }

      if (curr->catchRefs[tagIndex]) {
        auto location = CaughtExnRefLocation{};
        addRoot(location,
                PossibleContents::fromType(Type(HeapType::exn, NonNullable)));
        info.links.push_back(
          {location, getBreakTargetLocation(target, exnrefIndex)});
      }
    }
  }
  void visitThrow(Throw* curr) {
    auto& operands = curr->operands;
    if (!isRelevant(operands)) {
      return;
    }

    auto tag = curr->tag;
    for (Index i = 0; i < curr->operands.size(); i++) {
      info.links.push_back(
        {ExpressionLocation{operands[i], 0}, TagLocation{tag, i}});
    }
  }
  void visitRethrow(Rethrow* curr) {}
  void visitThrowRef(ThrowRef* curr) {}

  void visitTupleMake(TupleMake* curr) {
    if (isRelevant(curr->type)) {
      for (Index i = 0; i < curr->operands.size(); i++) {
        info.links.push_back({ExpressionLocation{curr->operands[i], 0},
                              ExpressionLocation{curr, i}});
      }
    }
  }
  void visitTupleExtract(TupleExtract* curr) {
    if (isRelevant(curr->type)) {
      info.links.push_back({ExpressionLocation{curr->tuple, curr->index},
                            ExpressionLocation{curr, 0}});
    }
  }

  // Adds a result to the current function, such as from a return or the value
  // that flows out.
  void addResult(Expression* value) {
    if (value && isRelevant(value->type)) {
      for (Index i = 0; i < value->type.size(); i++) {
        info.links.push_back(
          {ExpressionLocation{value, i}, ResultLocation{getFunction(), i}});
      }
    }
  }

  void visitReturn(Return* curr) { addResult(curr->value); }

  void visitContBind(ContBind* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitContNew(ContNew* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitResume(Resume* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }
  void visitSuspend(Suspend* curr) {
    // TODO: optimize when possible
    addRoot(curr);
  }

  void visitFunction(Function* func) {
    // Functions with a result can flow a value out from their body.
    addResult(func->body);

    // See visitPop().
    assert(handledPops == totalPops);

    // Handle local.get/sets: each set must write to the proper gets.
    //
    // Note that we do not use LocalLocation because LocalGraph gives us more
    // precise information: we generate direct links from sets to relevant gets
    // rather than consider each local index a single location, which
    // LocalLocation does. (LocalLocation is useful in cases where we do need a
    // single location, such as when we consider what type to give the local;
    // the type must be the same for all gets of that local.)
    LocalGraph localGraph(func, getModule());

    for (auto& [curr, _] : localGraph.locations) {
      auto* get = curr->dynCast<LocalGet>();
      if (!get) {
        continue;
      }

      auto index = get->index;
      auto type = func->getLocalType(index);
      if (!isRelevant(type)) {
        continue;
      }

      // Each get reads from its relevant sets.
      for (auto* set : localGraph.getSets(get)) {
        for (Index i = 0; i < type.size(); i++) {
          Location source;
          if (set) {
            // This is a normal local.set.
            source = ExpressionLocation{set->value, i};
          } else if (getFunction()->isParam(index)) {
            // This is a parameter.
            source = ParamLocation{getFunction(), index};
          } else {
            // This is the default value from the function entry, a null.
            source = getNullLocation(type[i]);
          }
          info.links.push_back({source, ExpressionLocation{get, i}});
        }
      }
    }
  }

  // Helpers

  // Returns the location of a break target by the name (e.g. returns the
  // location of a block, if the name is the name of a block). Also receives the
  // index in a tuple, if this is part of a tuple value.
  Location getBreakTargetLocation(Name target, Index i) {
    return ExpressionLocation{findBreakTarget(target), i};
  }

  // Handles the value sent in a break instruction. Does not handle anything
  // else like the condition etc.
  void handleBreakValue(Expression* curr) {
    BranchUtils::operateOnScopeNameUsesAndSentValues(
      curr, [&](Name target, Expression* value) {
        if (value && isRelevant(value->type)) {
          for (Index i = 0; i < value->type.size(); i++) {
            // Breaks send the contents of the break value to the branch target
            // that the break goes to.
            info.links.push_back({ExpressionLocation{value, i},
                                  getBreakTargetLocation(target, i)});
          }
        }
      });
  }

  // Connect a child's value to the parent, that is, all content in the child is
  // now considered possible in the parent as well.
  void receiveChildValue(Expression* child, Expression* parent) {
    if (isRelevant(parent) && isRelevant(child)) {
      // The tuple sizes must match (or, if not a tuple, the size should be 1 in
      // both cases).
      assert(child->type.size() == parent->type.size());
      for (Index i = 0; i < child->type.size(); i++) {
        info.links.push_back(
          {ExpressionLocation{child, i}, ExpressionLocation{parent, i}});
      }
    }
  }

  // See the comment on CollectedFuncInfo::childParents.
  void addChildParentLink(Expression* child, Expression* parent) {
    if (isRelevant(child->type)) {
      info.childParents[child] = parent;
    }
  }

  // Adds a root, if the expression is relevant. If the value is not specified,
  // mark the root as containing Many (which is the common case, so avoid
  // verbose code).
  void addRoot(Expression* curr,
               PossibleContents contents = PossibleContents::many()) {
    // TODO Use a cone type here when relevant
    if (isRelevant(curr)) {
      if (contents.isMany()) {
        contents = PossibleContents::fromType(curr->type);
      }
      addRoot(ExpressionLocation{curr, 0}, contents);
    }
  }

  // As above, but given an arbitrary location and not just an expression.
  void addRoot(Location loc,
               PossibleContents contents = PossibleContents::many()) {
    info.roots.emplace_back(loc, contents);
  }
};

// TrapsNeverHappen Oracle. This makes inferences *backwards* from traps that we
// know will not happen due to the TNH assumption. For example,
//
//  (local.get $a)
//  (ref.cast $B (local.get $a))
//
// The cast happens right after the first local.get, and we assume it does not
// fail, so the local must contain a B, even though the IR only has A.
//
// This analysis complements ContentOracle, which uses this analysis internally.
// ContentOracle does a forward flow analysis (as content moves from place to
// place) which increases from "nothing", while this does a backwards analysis
// that decreases from "everything" (or rather, from the type declared in the
// IR), so the two cannot be done at once.
//
// TODO: We could cycle between this and ContentOracle for repeated
//       improvements.
// TODO: This pass itself could benefit from internal cycles.
//
// This analysis mainly focuses on information across calls, as simple backwards
// inference is done in OptimizeCasts. Note that it is not needed if a call is
// inlined, obviously, and so it mostly helps cases like functions too large to
// inline, or when optimizing for size, or with indirect calls.
//
// We track cast parameters by mapping an index to the type it is definitely
// cast to if the function is entered. From that information we can infer things
// about the values being sent to the function (which we can assume must have
// the right type so that the casts do not trap).
using CastParams = std::unordered_map<Index, Type>;

// The information we collect and utilize as we operate in parallel in each
// function.
struct TNHInfo {
  CastParams castParams;

  // TODO: Returns as well: when we see (ref.cast (call $foo)) in all callers
  //       then we can refine inside $foo (in closed world).

  // We gather calls in parallel in order to process them later.
  std::vector<Call*> calls;
  std::vector<CallRef*> callRefs;

  // Note if a function body definitely traps.
  bool traps = false;

  // We gather inferences in parallel and combine them at the end.
  std::unordered_map<Expression*, PossibleContents> inferences;
};

class TNHOracleAnalysis
  : public ModuleUtils::ParallelFunctionAnalysis<TNHInfo> {
  const PassOptions& options;
  std::shared_ptr<const Oracle> prevOracle;

public:
  using Parent = ModuleUtils::ParallelFunctionAnalysis<TNHInfo>;
  TNHOracleAnalysis(Module& wasm,
                    const PassOptions& options,
                    std::shared_ptr<const Oracle> prevOracle)
    : Parent(wasm,
             [this, &options](Function* func, TNHInfo& info) {
               scan(func, info, options);
             }),
      options(options), prevOracle(prevOracle) {

    // After the scanning phase that we run in the constructor, continue to the
    // second phase of analysis: inference.
    infer();
  }

  // Get the type we inferred was possible at a location.
  PossibleContents getContents(Expression* curr) const {
    if (auto iter = inferences.find(curr); iter != inferences.end()) {
      // We only store useful contents that improve on the naive estimate that
      // uses the type in the IR.
      [[maybe_unused]] auto naiveContents =
        PossibleContents::fullConeType(curr->type);
      auto contents = iter->second;
      assert(contents != naiveContents);
      return contents;
    }

    // We inferred nothing ourselves, so defer to the previous oracle.
    return prevOracle->getExprContents(curr);
  }

private:
  // Maps expressions to the content we inferred there. If an expression is not
  // here then expression->type (the type in Binaryen IR) is all we have.
  std::unordered_map<Expression*, PossibleContents> inferences;

  // Phase 1: Scan to find cast parameters and calls. This operates on a single
  // function, and is called in parallel.
  void scan(Function* func, TNHInfo& info, const PassOptions& options);

  // Phase 2: Infer contents based on what we scanned.
  void infer();

  // Optimize one specific call (or call_ref).
  void optimizeCallCasts(Expression* call,
                         const ExpressionList& operands,
                         const CastParams& targetCastParams,
                         const analysis::CFGBlockIndexes& blockIndexes,
                         TNHInfo& info);

  // Get the information known by the previous oracle.
  PossibleContents getPrevContents(Expression* expr) {
    return prevOracle->getExprContents(expr);
  }
};

// Wraps around a TNHOracleAnalysis in order to provide a standard Oracle
// interface (this avoids multiple inheritance).
class TNHOracle : public Oracle {
  TNHOracleAnalysis analysis;

public:
  TNHOracle(Module& wasm,
            const PassOptions& options,
            std::shared_ptr<const Oracle> prevOracle)
    : Oracle(wasm, options), analysis(wasm, options, prevOracle) {}

  // Get the type we inferred was possible at a location.
  PossibleContents getContents(Location location) const override {
    if (auto* exprLoc = std::get_if<ExpressionLocation>(&location)) {
      return analysis.getContents(exprLoc->expr);
    }

    // We do not infer about non-expression locations yet.
    return PossibleContents::many();
  }
};

void TNHOracleAnalysis::scan(Function* func,
                             TNHInfo& info,
                             const PassOptions& options) {
  if (func->imported()) {
    return;
  }

  // Gather parameters that are definitely cast in the function entry.
  struct EntryScanner : public LinearExecutionWalker<EntryScanner> {
    Module& wasm;
    const PassOptions& options;
    TNHInfo& info;

    EntryScanner(Module& wasm, const PassOptions& options, TNHInfo& info)
      : wasm(wasm), options(options), info(info) {}

    // Note while we are still in the entry (first) block.
    bool inEntryBlock = true;

    static void doNoteNonLinear(EntryScanner* self, Expression** currp) {
      // This is the end of the first basic block.
      self->inEntryBlock = false;
    }

    void visitCall(Call* curr) { info.calls.push_back(curr); }

    void visitCallRef(CallRef* curr) {
      // We can only optimize call_ref in closed world, as otherwise the
      // call can go somewhere we can't see.
      if (options.closedWorld) {
        info.callRefs.push_back(curr);
      }
    }

    void visitRefAs(RefAs* curr) {
      if (curr->op == RefAsNonNull) {
        noteCast(curr->value, curr->type);
      }
    }
    void visitRefCast(RefCast* curr) { noteCast(curr->ref, curr->type); }

    // Note a cast of an expression to a particular type.
    void noteCast(Expression* expr, Type type) {
      if (!inEntryBlock) {
        return;
      }

      auto* fallthrough = Properties::getFallthrough(expr, options, wasm);
      if (auto* get = fallthrough->dynCast<LocalGet>()) {
        // To optimize, this needs to be a param, and of a useful type.
        //
        // Note that if we see more than one cast we keep the first one. This is
        // not important in optimized code, as the most refined cast would be
        // the only one to exist there, so it's ok to keep things simple here.
        if (getFunction()->isParam(get->index) && type != get->type &&
            info.castParams.count(get->index) == 0) {
          info.castParams[get->index] = type;
        }
      }
    }

    // Operations that trap on null are equivalent to casts to non-null, in that
    // they imply that their input is non-null if traps never happen.
    //
    // We only look at them if the input is actually nullable, since if they
    // are non-nullable then we can add no information. (This is equivalent
    // to the handling of RefAsNonNull above, in the sense that in optimized
    // code the RefAs will not appear if the input is already non-nullable).
    // This function is called with the reference that will be trapped on,
    // if it is null.
    void notePossibleTrap(Expression* expr) {
      if (!expr->type.isRef() || expr->type.isNonNullable()) {
        return;
      }
      noteCast(expr, Type(expr->type.getHeapType(), NonNullable));
    }

    void visitStructGet(StructGet* curr) { notePossibleTrap(curr->ref); }
    void visitStructSet(StructSet* curr) { notePossibleTrap(curr->ref); }
    void visitStructRMW(StructRMW* curr) { notePossibleTrap(curr->ref); }
    void visitStructCmpxchg(StructCmpxchg* curr) {
      notePossibleTrap(curr->ref);
    }
    void visitArrayGet(ArrayGet* curr) { notePossibleTrap(curr->ref); }
    void visitArraySet(ArraySet* curr) { notePossibleTrap(curr->ref); }
    void visitArrayLen(ArrayLen* curr) { notePossibleTrap(curr->ref); }
    void visitArrayCopy(ArrayCopy* curr) {
      notePossibleTrap(curr->srcRef);
      notePossibleTrap(curr->destRef);
    }
    void visitArrayFill(ArrayFill* curr) { notePossibleTrap(curr->ref); }
    void visitArrayInitData(ArrayInitData* curr) {
      notePossibleTrap(curr->ref);
    }
    void visitArrayInitElem(ArrayInitElem* curr) {
      notePossibleTrap(curr->ref);
    }

    void visitFunction(Function* curr) {
      // In optimized TNH code, a function that always traps will be turned
      // into a singleton unreachable instruction, so it is enough to check
      // for that.
      if (curr->body->is<Unreachable>()) {
        info.traps = true;
      }
    }
  } scanner(wasm, options, info);
  scanner.walkFunction(func);
}

void TNHOracleAnalysis::infer() {
  // Phase 2: Inside each function, optimize calls based on the cast params of
  // the called function (which we noted during phase 1).
  //
  // Specifically, each time we call a target that will cast a param, we can
  // infer that the param must have that type (or else we'd trap, but we are
  // assuming traps never happen).
  //
  // While doing so we must be careful of control flow transfers right before
  // the call:
  //
  //  (call $target
  //    (A)
  //    (br_if ..)
  //    (B)
  //  )
  //
  // If we branch in the br_if then we might execute A and then something else
  // entirely, and not reach B or the call. In that case we can't infer anything
  // about A (perhaps, for example, we branch away exactly when A would fail the
  // cast). Therefore in the optimization below we only optimize code that, if
  // reached, will definitely reach the call, like B.
  //
  // TODO: Some control flow transfers are ok, so long as we must reach the
  //       call, like if we replace the br_if with an if with two arms (and no
  //       branches in either).
  // TODO: We can also infer backwards past basic blocks from casts, even
  //       without calls. Any cast tells us something about the uses of that
  //       value that must reach the cast.
  // TODO: We can do a whole-program flow of this information.

  // For call_ref, we need to know which functions belong to each type. Gather
  // that first. This map will map each heap type to each function that is of
  // that type or a subtype, i.e., might be called when that type is seen in a
  // call_ref target.
  std::unordered_map<HeapType, std::vector<Function*>> typeFunctions;
  if (options.closedWorld) {
    for (auto& func : wasm.functions) {
      auto type = func->type;
      auto& info = map[wasm.getFunction(func->name)];
      if (info.traps) {
        // This function definitely traps, so we can assume it is never called,
        // and don't need to even bother putting it in |typeFunctions|.
        continue;
      }
      while (1) {
        typeFunctions[type].push_back(func.get());
        if (auto super = type.getDeclaredSuperType()) {
          type = *super;
        } else {
          break;
        }
      }
    }
  }

  doAnalysis([&](Function* func, TNHInfo& info) {
    // We will need some CFG information below. Computing this is expensive, so
    // only do it if we find optimization opportunities.
    std::optional<analysis::CFGBlockIndexes> blockIndexes;

    auto ensureCFG = [&]() {
      if (!blockIndexes) {
        auto cfg = analysis::CFG::fromFunction(func);
        blockIndexes = analysis::CFGBlockIndexes(cfg);
      }
    };

    for (auto* call : info.calls) {
      auto& targetInfo = map[wasm.getFunction(call->target)];

      auto& targetCastParams = targetInfo.castParams;
      if (targetCastParams.empty()) {
        continue;
      }

      // This looks promising, create the CFG if we haven't already, and
      // optimize.
      ensureCFG();
      optimizeCallCasts(
        call, call->operands, targetCastParams, *blockIndexes, info);

      // Note that we don't need to do anything for targetInfo.traps for a
      // direct call: the inliner will inline the singleton unreachable in the
      // target function anyhow.
    }

    for (auto* call : info.callRefs) {
      auto targetType = getPrevContents(call->target).getType();
      if (!targetType.isRef()) {
        // This is unreachable or null, and other passes will optimize that.
        continue;
      }

      // We should only get here in a closed world, in which we know which
      // functions might be called (the scan phase only notes callRefs if we are
      // in fact in a closed world).
      assert(options.closedWorld);

      auto iter = typeFunctions.find(targetType.getHeapType());
      if (iter == typeFunctions.end()) {
        // No function exists of this type, so the call_ref will trap. We can
        // mark the target as empty, which has the identical effect.
        info.inferences[call->target] = PossibleContents::none();
        continue;
      }

      // Go through the targets and ignore any that will trap. That will leave
      // us with the actually possible targets.
      //
      // Note that we did not even add functions that certainly trap to
      // |typeFunctions| at all, so those are already excluded.
      const auto& targets = iter->second;
      std::vector<Function*> possibleTargets;
      for (Function* target : targets) {
        auto& targetInfo = map[target];

        // If any of our operands will fail a cast, then we will trap.
        bool traps = false;
        for (auto& [castIndex, castType] : targetInfo.castParams) {
          auto operandType =
            getPrevContents(call->operands[castIndex]).getType();
          auto result = GCTypeUtils::evaluateCastCheck(operandType, castType);
          if (result == GCTypeUtils::Failure) {
            traps = true;
            break;
          }
        }
        if (!traps) {
          possibleTargets.push_back(target);
        }
      }

      if (possibleTargets.empty()) {
        // No target is possible.
        info.inferences[call->target] = PossibleContents::none();
        continue;
      }

      if (possibleTargets.size() == 1) {
        // There is exactly one possible call target, which means we can
        // actually infer what the call_ref is calling. Add that as an
        // inference.
        // TODO: We could also optimizeCallCasts() here, but it is low priority
        //       as other opts will make this call direct later, after which a
        //       lot of other optimizations become possible anyhow.
        auto target = possibleTargets[0]->name;
        info.inferences[call->target] = PossibleContents::literal(
          Literal(target, wasm.getFunction(target)->type));
        continue;
      }

      // More than one target exists: apply the intersection of their
      // constraints. That is, if they all cast the k-th parameter to type T (or
      // more) than we can apply that here.
      auto numParams = call->operands.size();
      std::vector<Type> sharedCastParamsVec(numParams, Type::unreachable);
      for (auto* target : possibleTargets) {
        auto& targetInfo = map[target];
        auto& targetCastParams = targetInfo.castParams;
        for (Index i = 0; i < numParams; i++) {
          auto iter = targetCastParams.find(i);
          if (iter == targetCastParams.end()) {
            // If the target does not cast, we cannot do anything with this
            // parameter; mark it as unoptimizable with an impossible type.
            sharedCastParamsVec[i] = Type::none;
            continue;
          }

          // This function casts this param. Combine this with existing info.
          auto castType = iter->second;
          sharedCastParamsVec[i] =
            Type::getLeastUpperBound(sharedCastParamsVec[i], castType);
        }
      }

      // Build a map of the interesting cast params we found, and if there are
      // any, optimize using them.
      CastParams sharedCastParams;
      for (Index i = 0; i < numParams; i++) {
        auto type = sharedCastParamsVec[i];
        if (type != Type::none) {
          sharedCastParams[i] = type;
        }
      }
      if (!sharedCastParams.empty()) {
        ensureCFG();
        optimizeCallCasts(
          call, call->operands, sharedCastParams, *blockIndexes, info);
      }
    }
  });

  // Combine all of our inferences from the parallel phase above us into the
  // final list of inferences.
  for (auto& [_, info] : map) {
    for (auto& [expr, contents] : info.inferences) {
      inferences[expr] = contents;
    }
  }
}

void TNHOracleAnalysis::optimizeCallCasts(
  Expression* call,
  const ExpressionList& operands,
  const CastParams& targetCastParams,
  const analysis::CFGBlockIndexes& blockIndexes,
  TNHInfo& info) {
  // Optimize in the same basic block as the call: all instructions still in
  // that block will definitely execute if the call is reached. We will do that
  // by going backwards through the call's operands and fallthrough values, and
  // optimizing while we are still in the same basic block.
  auto callBlockIndex = blockIndexes.get(call);

  // Operands must exist since there is a cast param, so a param exists.
  assert(operands.size() > 0);
  for (int i = int(operands.size() - 1); i >= 0; i--) {
    auto* operand = operands[i];

    if (blockIndexes.get(operand) != callBlockIndex) {
      // Control flow might transfer; stop.
      break;
    }

    auto iter = targetCastParams.find(i);
    if (iter == targetCastParams.end()) {
      // This param is not cast, so skip it.
      continue;
    }

    // If the call executes then this parameter is definitely reached (since it
    // is in the same basic block), and we know that it will be cast to a more
    // refined type.
    auto castType = iter->second;

    // Apply what we found to the operand and also to its fallthrough
    // values.
    //
    // At the loop entry |curr| has been checked for a possible control flow
    // transfer (and that problem ruled out).
    auto* curr = operand;
    while (1) {
      // Note the type if it is useful.
      //
      // There are two constraints on this location: what we know about it
      // already (from the previous oracle), and also the cast type, so we know
      // only their intersection can appear here.
      auto known = getPrevContents(curr);
      auto intersection = PossibleContents::fullConeType(castType);
      intersection.intersect(known);
      if (intersection != known) {
        info.inferences[curr] = intersection;
      }

      auto* next = Properties::getImmediateFallthrough(curr, options, wasm);
      if (next == curr) {
        // No fallthrough, we're done with this param.
        break;
      }

      // There is a fallthrough. Check for a control flow transfer.
      if (blockIndexes.get(next) != callBlockIndex) {
        // Control flow might transfer; stop. We also cannot look at any further
        // operands (if a child of this operand is in another basic block from
        // the call, so are previous operands), so return from the entire
        // function.
        return;
      }

      // Continue to the fallthrough.
      curr = next;
    }
  }
}

// Main logic for building data for the flow analysis and then performing that
// analysis.
struct Flower {
  Module& wasm;
  const PassOptions& options;

  Flower(Module& wasm, const PassOptions& options);

  // Each LocationIndex will have one LocationInfo that contains the relevant
  // information we need for each location.
  struct LocationInfo {
    // The location at this index.
    Location location;

    // The possible contents in that location.
    PossibleContents contents;

    // A list of the target locations to which this location sends content.
    // TODO: benchmark SmallVector<1> here, as commonly there may be a single
    //       target (an expression has one parent)
    std::vector<LocationIndex> targets;

    LocationInfo(Location location) : location(location) {}
  };

  // Maps location indexes to the info stored there, as just described above.
  std::vector<LocationInfo> locations;

  // Reverse mapping of locations to their indexes.
  std::unordered_map<Location, LocationIndex> locationIndexes;

  const Location& getLocation(LocationIndex index) {
    assert(index < locations.size());
    return locations[index].location;
  }

  PossibleContents& getContents(LocationIndex index) {
    assert(index < locations.size());
    return locations[index].contents;
  }

  // Check what we know about the type of an expression, using static
  // information from a TrapsNeverHappen oracle (see TNHOracle), if we have one.
  PossibleContents getTNHContents(Expression* curr) {
    if (!tnhOracle) {
      // No oracle; just use the type in the IR.
      return PossibleContents::fullConeType(curr->type);
    }
    return tnhOracle->getExprContents(curr);
  }

private:
  std::unique_ptr<TNHOracle> tnhOracle;

  std::vector<LocationIndex>& getTargets(LocationIndex index) {
    assert(index < locations.size());
    return locations[index].targets;
  }

  // Convert the data into the efficient LocationIndex form we will use during
  // the flow analysis. This method returns the index of a location, allocating
  // one if this is the first time we see it.
  LocationIndex getIndex(const Location& location) {
    auto iter = locationIndexes.find(location);
    if (iter != locationIndexes.end()) {
      return iter->second;
    }

    // Allocate a new index here.
    size_t index = locations.size();
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
    std::cout << "  new index " << index << " for ";
    dump(location);
#endif
    if (index >= std::numeric_limits<LocationIndex>::max()) {
      // 32 bits should be enough since each location takes at least one byte
      // in the binary, and we don't have 4GB wasm binaries yet... do we?
      Fatal() << "Too many locations for 32 bits";
    }
    locations.emplace_back(location);
    locationIndexes[location] = index;

    return index;
  }

  bool hasIndex(const Location& location) {
    return locationIndexes.find(location) != locationIndexes.end();
  }

  IndexLink getIndexes(const LocationLink& link) {
    return {getIndex(link.from), getIndex(link.to)};
  }

  // See the comment on CollectedFuncInfo::childParents. This is the merged info
  // from all the functions and the global scope.
  std::unordered_map<LocationIndex, LocationIndex> childParents;

  // The work remaining to do during the flow: locations that we need to flow
  // content from, after new content reached them.
  //
  // Using a set here is efficient as multiple updates may arrive to a location
  // before we get to processing it.
  //
  // The items here could be {location, newContents}, but it is more efficient
  // to have already written the new contents to the main data structure. That
  // avoids larger data here, and also, updating the contents as early as
  // possible is helpful as anything reading them meanwhile (before we get to
  // their work item in the queue) will see the newer value, possibly avoiding
  // flowing an old value that would later be overwritten.
  //
  // This must be ordered to avoid nondeterminism. The problem is that our
  // operations are imprecise and so the transitive property does not hold:
  // (AvB)vC may differ from Av(BvC). Likewise (AvB)^C may differ from
  // (A^C)v(B^C). An example of the latter is if a location is sent a null func
  // and an i31, and the location can only contain funcref. If the null func
  // arrives first, then later we'd merge null func + i31 which ends up as Many,
  // and then we filter that to funcref and get funcref. But if the i31 arrived
  // first, we'd filter it into nothing, and then the null func that arrives
  // later would be the final result. This would not happen if our operations
  // were precise, but we only make approximations here to avoid unacceptable
  // overhead, such as cone types but not arbitrary unions, etc.
  InsertOrderedSet<LocationIndex> workQueue;

  // All existing links in the graph. We keep this to know when a link we want
  // to add is new or not.
  std::unordered_set<IndexLink> links;

  // Update a location with new contents that are added to everything already
  // present there. If the update changes the contents at that location (if
  // there was anything new) then we also need to flow from there, which we will
  // do by adding the location to the work queue, and eventually flowAfterUpdate
  // will be called on this location.
  //
  // Returns whether it is worth sending new contents to this location in the
  // future. If we return false, the sending location never needs to do that
  // ever again.
  bool updateContents(LocationIndex locationIndex,
                      PossibleContents newContents);

  // Slow helper that converts a Location to a LocationIndex. This should be
  // avoided. TODO: remove the remaining uses of this.
  bool updateContents(const Location& location,
                      const PossibleContents& newContents) {
    return updateContents(getIndex(location), newContents);
  }

  // Flow contents from a location where a change occurred. This sends the new
  // contents to all the normal targets of this location (using
  // flowToTargetsAfterUpdate), and also handles special cases of flow after.
  void flowAfterUpdate(LocationIndex locationIndex);

  // Internal part of flowAfterUpdate that handles sending new values to the
  // given location index's normal targets (that is, the ones listed in the
  // |targets| vector).
  void flowToTargetsAfterUpdate(LocationIndex locationIndex,
                                const PossibleContents& contents);

  // Add a new connection while the flow is happening. If the link already
  // exists it is not added.
  void connectDuringFlow(Location from, Location to);

  // Contents sent to certain locations can be filtered in a special way during
  // the flow, which is handled in these helpers. These may update
  // |worthSendingMore| which is whether it is worth sending any more content to
  // this location in the future.
  void filterExpressionContents(PossibleContents& contents,
                                const ExpressionLocation& exprLoc,
                                bool& worthSendingMore);
  void filterGlobalContents(PossibleContents& contents,
                            const GlobalLocation& globalLoc);
  void filterDataContents(PossibleContents& contents,
                          const DataLocation& dataLoc);
  void filterPackedDataReads(PossibleContents& contents,
                             const ExpressionLocation& exprLoc);

  // Reads from GC data: a struct.get or array.get. This is given the type of
  // the read operation, the field that is read on that type, the known contents
  // in the reference the read receives, and the read instruction itself. We
  // compute where we need to read from based on the type and the ref contents
  // and get that data, adding new links in the graph as needed.
  void readFromData(Type declaredType,
                    Index fieldIndex,
                    const PossibleContents& refContents,
                    Expression* read);

  // Similar to readFromData, but does a write for a struct.set or array.set.
  void writeToData(Expression* ref, Expression* value, Index fieldIndex);

  // We will need subtypes during the flow, so compute them once ahead of time.
  std::unique_ptr<SubTypes> subTypes;

  // The depth of children for each type. This is 0 if the type has no
  // subtypes, 1 if it has subtypes but none of those have subtypes themselves,
  // and so forth.
  std::unordered_map<HeapType, Index> maxDepths;

  // Given a ConeType, return the normalized depth, that is, the canonical depth
  // given the actual children it has. If this is a full cone, then we can
  // always pick the actual maximal depth and use that instead of FullDepth==-1.
  // For a non-full cone, we also reduce the depth as much as possible, so it is
  // equal to the maximum depth of an existing subtype.
  Index getNormalizedConeDepth(Type type, Index depth) {
    return std::min(depth, maxDepths[type.getHeapType()]);
  }

  void normalizeConeType(PossibleContents& cone) {
    assert(cone.isConeType());
    auto type = cone.getType();
    auto before = cone.getCone().depth;
    auto normalized = getNormalizedConeDepth(type, before);
    if (normalized != before) {
      cone = PossibleContents::coneType(type, normalized);
    }
  }

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  // Dump out a location for debug purposes.
  void dump(Location location);
#endif
};

Flower::Flower(Module& wasm, const PassOptions& options)
  : wasm(wasm), options(options) {

  // If traps never happen, create a TNH oracle.
  //
  // Atm this oracle only helps on GC content, so disable it without GC.
  if (options.trapsNeverHappen && wasm.features.hasGC()) {
#ifdef POSSIBLE_CONTENTS_DEBUG
    std::cout << "tnh phase\n";
#endif

    // Start with an empty base oracle, infer TNH once in the middle, and a
    // second time.
    auto base = std::make_shared<Oracle>(wasm, options);
    auto mid = std::make_shared<TNHOracle>(wasm, options, base);
    tnhOracle = std::make_unique<TNHOracle>(wasm, options, mid);
  }

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "parallel phase\n";
#endif

  // First, collect information from each function.
  ModuleUtils::ParallelFunctionAnalysis<CollectedFuncInfo> analysis(
    wasm, [&](Function* func, CollectedFuncInfo& info) {
      InfoCollector finder(info, options);

      if (func->imported()) {
        // Imports return unknown values.
        auto results = func->getResults();
        for (Index i = 0; i < results.size(); i++) {
          finder.addRoot(ResultLocation{func, i},
                         PossibleContents::fromType(results[i]));
        }
        return;
      }

      finder.walkFunctionInModule(func, &wasm);
    });

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "single phase\n";
#endif

  // Also walk the global module code (for simplicity, also add it to the
  // function map, using a "function" key of nullptr).
  auto& globalInfo = analysis.map[nullptr];
  InfoCollector finder(globalInfo, options);
  finder.walkModuleCode(&wasm);

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "global init phase\n";
#endif

  // Connect global init values (which we've just processed, as part of the
  // module code) to the globals they initialize.
  for (auto& global : wasm.globals) {
    if (global->imported()) {
      // Imports are unknown values.
      finder.addRoot(GlobalLocation{global->name},
                     PossibleContents::fromType(global->type));
      continue;
    }
    auto* init = global->init;
    if (finder.isRelevant(init->type)) {
      globalInfo.links.push_back(
        {ExpressionLocation{init, 0}, GlobalLocation{global->name}});
    }
  }

  // Merge the function information into a single large graph that represents
  // the entire program all at once, indexing and deduplicating everything as we
  // go.

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "merging+indexing phase\n";
#endif

  // The merged roots. (Note that all other forms of merged data are declared at
  // the class level, since we need them during the flow, but the roots are only
  // needed to start the flow, so we can declare them here.)
  //
  // This must be insert-ordered for the same reason as |workQueue| is, see
  // above.
  InsertOrderedMap<Location, PossibleContents> roots;

  // Any function that may be called from the outside, like an export, is a
  // root, since they can be called with unknown parameters.
  auto calledFromOutside = [&](Name funcName) {
    auto* func = wasm.getFunction(funcName);
    auto params = func->getParams();
    for (Index i = 0; i < func->getParams().size(); i++) {
      roots[ParamLocation{func, i}] = PossibleContents::fromType(params[i]);
    }
  };

  for (auto& [func, info] : analysis.map) {
    for (auto& link : info.links) {
      links.insert(getIndexes(link));
    }
    for (auto& [root, value] : info.roots) {
      roots[root] = value;

      // Ensure an index even for a root with no links to it - everything needs
      // an index.
      getIndex(root);
    }
    for (auto [child, parent] : info.childParents) {
      // In practice we do not have any childParent connections with a tuple;
      // assert on that just to be safe.
      assert(!child->type.isTuple());
      childParents[getIndex(ExpressionLocation{child, 0})] =
        getIndex(ExpressionLocation{parent, 0});
    }

    for (auto func : info.calledFromOutside) {
      calledFromOutside(func);
    }
  }

  // We no longer need the function-level info.
  analysis.map.clear();

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "external phase\n";
#endif

  // Exports can be modified from the outside.
  for (auto& ex : wasm.exports) {
    if (ex->kind == ExternalKind::Function) {
      calledFromOutside(ex->value);
    } else if (ex->kind == ExternalKind::Table) {
      // If any table is exported, assume any function in any table (including
      // other tables) can be called from the outside.
      // TODO: This could be more precise about which tables are exported and
      //       which are not: perhaps one table is exported but we can optimize
      //       the functions in another table, which is not exported. However,
      //       it is simpler to treat them all the same, and this handles the
      //       common case of no tables being exported at all.
      // TODO: This does not handle table.get/table.set, or call_ref, for which
      //       we'd need to see which references are used and which escape etc.
      //       For now, assume a closed world for such such advanced use cases /
      //       assume this pass won't be run in them anyhow.
      // TODO: do this only once if multiple tables are exported
      for (auto& elementSegment : wasm.elementSegments) {
        for (auto* curr : elementSegment->data) {
          if (auto* refFunc = curr->dynCast<RefFunc>()) {
            calledFromOutside(refFunc->func);
          }
        }
      }
    } else if (ex->kind == ExternalKind::Global) {
      // Exported mutable globals are roots, since the outside may write any
      // value to them.
      auto name = ex->value;
      auto* global = wasm.getGlobal(name);
      if (global->mutable_) {
        roots[GlobalLocation{name}] = PossibleContents::fromType(global->type);
      }
    }
  }

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "struct phase\n";
#endif

  subTypes = std::make_unique<SubTypes>(wasm);
  maxDepths = subTypes->getMaxDepths();

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "Link-targets phase\n";
#endif

  // Add all links to the targets vectors of the source locations, which we will
  // use during the flow.
  for (auto& link : links) {
    getTargets(link.from).push_back(link.to);
  }

#ifndef NDEBUG
  // Each vector of targets (which is a vector for efficiency) must have no
  // duplicates.
  for (auto& info : locations) {
    disallowDuplicates(info.targets);
  }
#endif

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "roots phase\n";
#endif

  // Set up the roots, which are the starting state for the flow analysis: send
  // their initial content to them to start the flow.
  for (const auto& [location, value] : roots) {
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
    std::cout << "  init root\n";
    dump(location);
    value.dump(std::cout, &wasm);
    std::cout << '\n';
#endif

    updateContents(location, value);
  }

#ifdef POSSIBLE_CONTENTS_DEBUG
  std::cout << "flow phase\n";
  size_t iters = 0;
#endif

  // Flow the data while there is still stuff flowing.
  while (!workQueue.empty()) {
#ifdef POSSIBLE_CONTENTS_DEBUG
    iters++;
    if ((iters & 255) == 0) {
      std::cout << iters++ << " iters, work left: " << workQueue.size() << '\n';
    }
#endif

    auto iter = workQueue.begin();
    auto locationIndex = *iter;
    workQueue.erase(iter);

    flowAfterUpdate(locationIndex);
  }

  // TODO: Add analysis and retrieval logic for fields of immutable globals,
  //       including multiple levels of depth (necessary for itables in j2wasm).
}

bool Flower::updateContents(LocationIndex locationIndex,
                            PossibleContents newContents) {
  auto& contents = getContents(locationIndex);
  auto oldContents = contents;

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::cout << "\nupdateContents\n";
  dump(getLocation(locationIndex));
  contents.dump(std::cout, &wasm);
  std::cout << "\n with new contents \n";
  newContents.dump(std::cout, &wasm);
  std::cout << '\n';
#endif

  auto location = getLocation(locationIndex);

  // Handle special cases: Some locations can only contain certain contents, so
  // filter accordingly. In principle we need to filter both before and after
  // combining with existing content; filtering afterwards is obviously
  // necessary as combining two things will create something larger than both,
  // and our representation has limitations (e.g. two different ref types will
  // result in a cone, potentially a very large one). Filtering beforehand is
  // necessary for the a more subtle reason: consider a location that contains
  // an i8 which is sent a 0 and then 0x100. If we filter only after, then we'd
  // combine 0 and 0x100 first and get "unknown integer"; only by filtering
  // 0x100 to 0 beforehand (since 0x100 & 0xff => 0) will we combine 0 and 0 and
  // not change anything, which is correct.
  //
  // For efficiency reasons we aim to only filter once, depending on the type of
  // filtering. Most can be filtered a single time afterwards, while for data
  // locations, where the issue is packed integer fields, it's necessary to do
  // it before as we've mentioned, and also sufficient (see details in
  // filterDataContents).
  if (auto* dataLoc = std::get_if<DataLocation>(&location)) {
    filterDataContents(newContents, *dataLoc);
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
    std::cout << "  pre-filtered data contents:\n";
    newContents.dump(std::cout, &wasm);
    std::cout << '\n';
#endif
  } else if (auto* exprLoc = std::get_if<ExpressionLocation>(&location)) {
    if (exprLoc->expr->is<StructGet>() || exprLoc->expr->is<ArrayGet>()) {
      // Packed data reads must be filtered before the combine() operation, as
      // we must only combine the filtered contents (e.g. if 0xff arrives which
      // as a signed read is truly 0xffffffff then we cannot first combine the
      // existing 0xffffffff with the new 0xff, as they are different, and the
      // result will no longer be a constant). There is no need to filter atomic
      // RMW operations here because they always do unsigned reads.
      filterPackedDataReads(newContents, *exprLoc);
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
      std::cout << "  pre-filtered packed read contents:\n";
      newContents.dump(std::cout, &wasm);
      std::cout << '\n';
#endif
    }
  }

  contents.combine(newContents);

  if (contents.isNone()) {
    // There is still nothing here. There is nothing more to do here but to
    // return that it is worth sending more.
    return true;
  }

  // It is not worth sending any more to this location if we are now in the
  // worst possible case, as no future value could cause any change.
  bool worthSendingMore = true;
  if (contents.isConeType()) {
    if (!contents.getType().isRef()) {
      // A cone type of a non-reference is the worst case, since subtyping is
      // not relevant there, and so if we only know something about the type
      // then we already know nothing beyond what the type in the wasm tells us
      // (and from there we can only go to Many).
      worthSendingMore = false;
    } else {
      // Normalize all reference cones. There is never a point to flow around
      // anything non-normalized, which might lead to extra work. For example,
      // if A has no subtypes, then a full cone for A is really the same as one
      // with depth 0 (an exact type). And we don't want to see the full cone
      // arrive and think it was an improvement over the one with depth 0 and do
      // more flowing based on that.
      normalizeConeType(contents);
    }
  }

  // Check if anything changed.
  if (contents == oldContents) {
    // Nothing actually changed, so just return.
    return worthSendingMore;
  }

  // Handle filtering (see comment earlier, this is the later filtering stage).
  bool filtered = false;
  if (auto* exprLoc = std::get_if<ExpressionLocation>(&location)) {
    // TODO: Replace this with specific filterFoo or flowBar methods like we
    //       have for filterGlobalContents. That could save a little wasted work
    //       here. Might be best to do that after the spec is fully stable.
    filterExpressionContents(contents, *exprLoc, worthSendingMore);
    filtered = true;
  } else if (auto* globalLoc = std::get_if<GlobalLocation>(&location)) {
    filterGlobalContents(contents, *globalLoc);
    filtered = true;
  }

  // Check if anything changed after filtering, if we did so.
  if (filtered) {
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
    std::cout << "  filtered contents:\n";
    contents.dump(std::cout, &wasm);
    std::cout << '\n';
#endif

    if (contents == oldContents) {
      return worthSendingMore;
    }
  }

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::cout << "  updateContents has something new\n";
  contents.dump(std::cout, &wasm);
  std::cout << '\n';
#endif

  // After filtering we should always have more precise information than "many"
  // - in the worst case, we can have the type declared in the wasm.
  assert(!contents.isMany());

  // Add a work item if there isn't already.
  workQueue.insert(locationIndex);

  return worthSendingMore;
}

void Flower::flowAfterUpdate(LocationIndex locationIndex) {
  const auto location = getLocation(locationIndex);
  auto& contents = getContents(locationIndex);

  // We are called after a change at a location. A change means that some
  // content has arrived, since we never send empty values around. Assert on
  // that.
  assert(!contents.isNone());

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::cout << "\nflowAfterUpdate to:\n";
  dump(location);
  std::cout << "  arriving:\n";
  contents.dump(std::cout, &wasm);
  std::cout << '\n';
#endif

  // Flow the contents to the normal targets of this location.
  flowToTargetsAfterUpdate(locationIndex, contents);

  // We are mostly done, except for handling interesting/special cases in the
  // flow, additional operations that we need to do aside from sending the new
  // contents to the normal (statically linked) targets.

  if (auto* exprLoc = std::get_if<ExpressionLocation>(&location)) {
    auto iter = childParents.find(locationIndex);
    if (iter == childParents.end()) {
      return;
    }

    // This is indeed one of the special cases where it is the child of a
    // parent, and we need to do some special handling because of that child-
    // parent connection.
    [[maybe_unused]] auto* child = exprLoc->expr;
    auto parentIndex = iter->second;
    auto* parent = std::get<ExpressionLocation>(getLocation(parentIndex)).expr;

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
    std::cout << "  special, parent:\n" << *parent << '\n';
#endif

    if (auto* get = parent->dynCast<StructGet>()) {
      // |child| is the reference child of a struct.get.
      assert(get->ref == child);
      readFromData(get->ref->type, get->index, contents, get);
    } else if (auto* set = parent->dynCast<StructSet>()) {
      // |child| is either the reference or the value child of a struct.set.
      assert(set->ref == child || set->value == child);
      writeToData(set->ref, set->value, set->index);
    } else if (auto* get = parent->dynCast<ArrayGet>()) {
      assert(get->ref == child);
      readFromData(get->ref->type, 0, contents, get);
    } else if (auto* set = parent->dynCast<ArraySet>()) {
      assert(set->ref == child || set->value == child);
      writeToData(set->ref, set->value, 0);
    } else {
      // TODO: ref.test and all other casts can be optimized (see the cast
      //       helper code used in OptimizeInstructions and RemoveUnusedBrs)
      WASM_UNREACHABLE("bad childParents content");
    }
  }
}

void Flower::flowToTargetsAfterUpdate(LocationIndex locationIndex,
                                      const PossibleContents& contents) {
  // Send the new contents to all the targets of this location. As we do so,
  // prune any targets that we do not need to bother sending content to in the
  // future, to save space and work later.
  auto& targets = getTargets(locationIndex);
  targets.erase(std::remove_if(targets.begin(),
                               targets.end(),
                               [&](LocationIndex targetIndex) {
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
                                 std::cout << "  send to target\n";
                                 dump(getLocation(targetIndex));
#endif
                                 return !updateContents(targetIndex, contents);
                               }),
                targets.end());

  if (contents.isMany()) {
    // We contain Many, and just called updateContents on our targets to send
    // that value to them. We'll never need to send anything from here ever
    // again, since we sent the worst case possible already, so we can just
    // clear our targets vector. But we should have already removed all the
    // targets in the above remove_if operation, since they should have all
    // notified us that we do not need to send them any more updates.
    assert(targets.empty());
  }
}

void Flower::connectDuringFlow(Location from, Location to) {
  auto newLink = LocationLink{from, to};
  auto newIndexLink = getIndexes(newLink);
  if (links.count(newIndexLink) == 0) {
    // This is a new link. Add it to the known links.
    links.insert(newIndexLink);

    // Add it to the |targets| vector.
    auto& targets = getTargets(newIndexLink.from);
    targets.push_back(newIndexLink.to);
#ifndef NDEBUG
    disallowDuplicates(targets);
#endif

    // In addition to adding the link, which will ensure new contents appearing
    // later will be sent along, we also update with the current contents.
    updateContents(to, getContents(getIndex(from)));
  }
}

void Flower::filterExpressionContents(PossibleContents& contents,
                                      const ExpressionLocation& exprLoc,
                                      bool& worthSendingMore) {
  auto type = exprLoc.expr->type;

  if (type.isTuple()) {
    // TODO: Optimize tuples here as well. We would need to take into account
    //       exprLoc.tupleIndex for that in all the below.
    return;
  }

  // The caller cannot know of a situation where it might not be worth sending
  // more to a reference - all that logic is in here. That is, the rest of this
  // function is the only place we can mark |worthSendingMore| as false for a
  // reference.
  bool isRef = type.isRef();
  assert(!isRef || worthSendingMore);

  // The TNH oracle informs us of the maximal contents possible here.
  auto maximalContents = getTNHContents(exprLoc.expr);
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::cout << "TNHOracle informs us that " << *exprLoc.expr << " contains "
            << maximalContents << "\n";
#endif
  contents.intersect(maximalContents);
  if (contents.isNone()) {
    // Nothing was left here at all.
    return;
  }

  // For references we need to normalize the intersection, see below. For non-
  // references, we are done (we did all the relevant work in the intersect()
  // call).
  if (!isRef) {
    return;
  }

  // Normalize the intersection. We want to check later if any more content can
  // arrive here, and also we want to avoid flowing around anything non-
  // normalized, as explained earlier.
  //
  // Note that this normalization is necessary even though |contents| was
  // normalized before the intersection, e.g.:
  /*
  //      A
  //     / \
  //    B   C
  //        |
  //        D
  */
  // Consider the case where |maximalContents| is Cone(B, Infinity) and the
  // original |contents| was Cone(A, 2) (which is normalized). The naive
  // intersection is Cone(B, 1), since the core intersection logic makes no
  // assumptions about the rest of the types. That is then normalized to
  // Cone(B, 0) since there happens to be no subtypes for B.
  //
  // Note that the intersection may also not be a cone type, if it is a global
  // or literal. In that case we don't have anything more to do here.
  if (!contents.isConeType()) {
    return;
  }

  normalizeConeType(contents);

  // There is a chance that the intersection is equal to the maximal contents,
  // which would mean nothing more can arrive here.
  normalizeConeType(maximalContents);

  if (contents == maximalContents) {
    // We already contain everything possible, so this is the worst case.
    worthSendingMore = false;
  }
}

void Flower::filterGlobalContents(PossibleContents& contents,
                                  const GlobalLocation& globalLoc) {
  auto* global = wasm.getGlobal(globalLoc.name);
  if (global->mutable_ == Immutable) {
    // This is an immutable global. We never need to consider this value as
    // "Many", since in the worst case we can just use the immutable value. That
    // is, we can always replace this value with (global.get $name) which will
    // get the right value. Likewise, using the immutable global value is often
    // better than a cone type (even an exact one), but TODO: we could note both
    // a cone/exact type *and* that something is equal to a global, in some
    // cases. See https://github.com/WebAssembly/binaryen/pull/5083
    if (contents.isMany() || contents.isConeType()) {
      contents = PossibleContents::global(global->name, global->type);

      // TODO: We could do better here, to set global->init->type instead of
      //       global->type, or even the contents.getType() - either of those
      //       may be more refined. But other passes will handle that in
      //       general (by refining the global's type).

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
      std::cout << "  setting immglobal to ImmutableGlobal\n";
      contents.dump(std::cout, &wasm);
      std::cout << '\n';
#endif
    }
  }
}

void Flower::filterDataContents(PossibleContents& contents,
                                const DataLocation& dataLoc) {
  auto field = GCTypeUtils::getField(dataLoc.type, dataLoc.index);
  if (!field) {
    // This is a bottom type; nothing will be written here.
    assert(dataLoc.type.isBottom());
    contents = PossibleContents::none();
    return;
  }

  if (field->isPacked()) {
    // We must handle packed fields carefully.
    if (contents.isLiteral()) {
      // This is a constant. We can truncate it and use that value.
      auto mask = Literal(int32_t(Bits::lowBitMask(field->getByteSize() * 8)));
      contents = PossibleContents::literal(contents.getLiteral().and_(mask));
    } else {
      // This is not a constant. We can't even handle a global here, as we'd
      // need to track that this global's value must be truncated before it is
      // used, and we don't do that atm. Leave only the type.
      // TODO Consider tracking packing on GlobalInfo alongside the type.
      //      Another option is to make GUFA.cpp apply packing on the read,
      //      like CFP does - but that can only be done when replacing a
      //      StructGet of a packed field, and not anywhere else we saw that
      //      value reach.
      contents = PossibleContents::fromType(contents.getType());
    }
    // Given that the above only (1) turns an i32 into a masked i32 or (2) turns
    // anything else into an unknown i32, this is safe to run as pre-filtering,
    // that is, before we combine contents, since
    //
    //  (a) two constants are ok as masking is distributive,
    //        (x & M) U (y & M)  ==  (x U y) & M
    //  (b) if one is a constant and the other is not then
    //        (x & M) U ?  ==  ?  ==  (x U ?)  ==  (x U ?) & M
    //      (where ? is an unknown i32)
    //  (c) and if both are not constants then likewise we always end up as an
    //      unknown i32
    //
  }
}

void Flower::filterPackedDataReads(PossibleContents& contents,
                                   const ExpressionLocation& exprLoc) {
  auto* expr = exprLoc.expr;

  // Packed fields are stored as the truncated bits (see comment on
  // DataLocation; the actual truncation is done in filterDataContents), which
  // means that unsigned gets just work but signed ones need fixing (and we only
  // know how to do that here, when we reach the get and see if it is signed).
  auto signed_ = false;
  Expression* ref;
  Index index;
  if (auto* get = expr->dynCast<StructGet>()) {
    signed_ = get->signed_;
    ref = get->ref;
    index = get->index;
  } else if (auto* get = expr->dynCast<ArrayGet>()) {
    signed_ = get->signed_;
    ref = get->ref;
    // Arrays are treated as having a single field.
    index = 0;
  } else {
    WASM_UNREACHABLE("bad packed read");
  }
  if (!signed_) {
    return;
  }

  // We are reading data here, so the reference must be a valid struct or
  // array, otherwise we would never have gotten here.
  assert(ref->type.isRef());
  auto field = GCTypeUtils::getField(ref->type.getHeapType(), index);
  assert(field);
  if (!field->isPacked()) {
    return;
  }

  if (contents.isLiteral()) {
    // This is a constant. We can sign-extend it and use that value.
    auto shifts = Literal(int32_t(32 - field->getByteSize() * 8));
    auto lit = contents.getLiteral();
    lit = lit.shl(shifts);
    lit = lit.shrS(shifts);
    contents = PossibleContents::literal(lit);
  } else {
    // This is not a constant. As in filterDataContents, give up and leave
    // only the type, since we have no way to track the sign-extension on
    // top of whatever this is.
    contents = PossibleContents::fromType(contents.getType());
  }
}

void Flower::readFromData(Type declaredType,
                          Index fieldIndex,
                          const PossibleContents& refContents,
                          Expression* read) {
#ifndef NDEBUG
  // We must not have anything in the reference that is invalid for the wasm
  // type there.
  auto maximalContents = PossibleContents::fullConeType(declaredType);
  assert(PossibleContents::isSubContents(refContents, maximalContents));
#endif

  // The data that a struct.get reads depends on two things: the reference that
  // we read from, and the relevant DataLocations. The reference determines
  // which DataLocations are relevant: if it is an exact type then we have a
  // single DataLocation to read from, the one type that can be read from there.
  // Otherwise, we might read from any subtype, and so all their DataLocations
  // are relevant.
  //
  // What can be confusing is that the information about the reference is also
  // inferred during the flow. That is, we use our current information about the
  // reference to decide what to do here. But the flow is not finished yet!
  // To keep things valid, we must therefore react to changes in either the
  // reference - when we see that more types might be read from here - or the
  // DataLocations - when new things are written to the data we can read from.
  // Specifically, at every point in time we want to preserve the property that
  // we've read from all relevant types based on the current reference, and
  // we've read the very latest possible contents from those types. And then
  // since we preserve that property til the end of the flow, it is also valid
  // then. At the end of the flow, the current reference's contents are the
  // final and correct contents for that location, which means we've ended up
  // with the proper result: the struct.get reads everything it should.
  //
  // To implement what was just described, we call this function when the
  // reference is updated. This function will then set up connections in the
  // graph so that updates to the relevant DataLocations will reach us in the
  // future.

  if (refContents.isNull() || refContents.isNone()) {
    // Nothing is read here as this is either a null or unreachable code. (Note
    // that the contents must be a subtype of the wasm type, which rules out
    // other possibilities like a non-null literal such as an integer or a
    // function reference.)
    return;
  }

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::cout << "    add special reads\n";
#endif

  // The only possibilities left are a cone type (the worst case is where the
  // cone matches the wasm type), or a global.
  //
  // TODO: The Global case may have a different cone type than the heapType,
  //       which we could use here.
  // TODO: A Global may refer to an immutable global, which we can read the
  //       field from potentially (reading it from the struct.new/array.new
  //       in the definition of it, if it is not imported; or, we could track
  //       the contents of immutable fields of allocated objects, and not just
  //       represent them as an exact type).
  //       See the test TODO with text "We optimize some of this, but stop at
  //       reading from the immutable global"
  assert(refContents.isGlobal() || refContents.isConeType());

  // Just look at the cone here, discarding information about this being a
  // global, if it was one. All that matters from now is the cone. We also
  // normalize the cone to avoid wasted work later.
  auto cone = refContents.getCone();
  auto normalizedDepth = getNormalizedConeDepth(cone.type, cone.depth);

  // We create a ConeReadLocation for the canonical cone of this type, to
  // avoid bloating the graph, see comment on ConeReadLocation().
  auto coneReadLocation =
    ConeReadLocation{cone.type.getHeapType(), normalizedDepth, fieldIndex};
  if (!hasIndex(coneReadLocation)) {
    // This is the first time we use this location, so create the links for it
    // in the graph.
    subTypes->iterSubTypes(
      cone.type.getHeapType(),
      normalizedDepth,
      [&](HeapType type, Index depth) {
        connectDuringFlow(DataLocation{type, fieldIndex}, coneReadLocation);
      });

    // TODO: we can end up with redundant links here if we see one cone first
    //       and then a larger one later. But removing links is not efficient,
    //       so for now just leave that.
  }

  // Link to the canonical location.
  connectDuringFlow(coneReadLocation, ExpressionLocation{read, 0});
}

void Flower::writeToData(Expression* ref, Expression* value, Index fieldIndex) {
#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
  std::cout << "    add special writes\n";
#endif

  auto refContents = getContents(getIndex(ExpressionLocation{ref, 0}));

#ifndef NDEBUG
  // We must not have anything in the reference that is invalid for the wasm
  // type there.
  auto maximalContents = PossibleContents::fullConeType(ref->type);
  assert(PossibleContents::isSubContents(refContents, maximalContents));
#endif

  // We could set up links here as we do for reads, but as we get to this code
  // in any case, we can just flow the values forward directly. This avoids
  // adding any links (edges) to the graph (and edges are what we want to avoid
  // adding, as there can be a quadratic number of them). In other words, we'll
  // loop over the places we need to send info to, which we can figure out in a
  // simple way, and by doing so we avoid materializing edges into the graph.
  //
  // Note that this is different from readFromData, above, which does add edges
  // to the graph (and works hard to add as few as possible, see the "canonical
  // cone reads" logic). The difference is because readFromData must "subscribe"
  // to get notifications from the relevant DataLocations. But when writing that
  // is not a problem: whenever a change happens in the reference or the value
  // of a struct.set then this function will get called, and those are the only
  // things we care about. And we can then just compute the values we are
  // sending (based on the current contents of the reference and the value), and
  // where we should send them to, and do that right here. (And as commented in
  // readFromData, that is guaranteed to give us the right result in the end: at
  // every point in time we send the right data, so when the flow is finished
  // we've sent information based on the final and correct information about our
  // reference and value.)

  auto valueContents = getContents(getIndex(ExpressionLocation{value, 0}));

  // See the related comment in readFromData() as to why these are the only
  // things we need to check, and why the assertion afterwards contains the only
  // things possible.
  if (refContents.isNone() || refContents.isNull()) {
    return;
  }
  assert(refContents.isGlobal() || refContents.isConeType());

  // As in readFromData, normalize to the proper cone.
  auto cone = refContents.getCone();
  auto normalizedDepth = getNormalizedConeDepth(cone.type, cone.depth);

  subTypes->iterSubTypes(
    cone.type.getHeapType(), normalizedDepth, [&](HeapType type, Index depth) {
      auto heapLoc = DataLocation{type, fieldIndex};
      updateContents(heapLoc, valueContents);
    });
}

#if defined(POSSIBLE_CONTENTS_DEBUG) && POSSIBLE_CONTENTS_DEBUG >= 2
void Flower::dump(Location location) {
  if (auto* loc = std::get_if<ExpressionLocation>(&location)) {
    std::cout << "  exprloc \n"
              << *loc->expr << " : " << loc->tupleIndex << '\n';
  } else if (auto* loc = std::get_if<DataLocation>(&location)) {
    std::cout << "  dataloc ";
    if (wasm.typeNames.count(loc->type)) {
      std::cout << '$' << wasm.typeNames[loc->type].name;
    } else {
      std::cout << loc->type << '\n';
    }
    std::cout << " : " << loc->index << '\n';
  } else if (auto* loc = std::get_if<TagLocation>(&location)) {
    std::cout << "  tagloc " << loc->tag << " : " << loc->tupleIndex << '\n';
  } else if (auto* loc = std::get_if<ParamLocation>(&location)) {
    std::cout << "  paramloc " << loc->func->name << " : " << loc->index
              << '\n';
  } else if (auto* loc = std::get_if<LocalLocation>(&location)) {
    std::cout << "  localloc " << loc->func->name << " : " << loc->index
              << '\n';
  } else if (auto* loc = std::get_if<ResultLocation>(&location)) {
    std::cout << "  resultloc $" << loc->func->name << " : " << loc->index
              << '\n';
  } else if (auto* loc = std::get_if<GlobalLocation>(&location)) {
    std::cout << "  globalloc " << loc->name << '\n';
  } else if (std::get_if<SignatureParamLocation>(&location)) {
    std::cout << "  sigparamloc " << '\n';
  } else if (std::get_if<SignatureResultLocation>(&location)) {
    std::cout << "  sigresultloc " << '\n';
  } else if (auto* loc = std::get_if<NullLocation>(&location)) {
    std::cout << "  Nullloc " << loc->type << '\n';
  } else {
    std::cout << "  (other)\n";
  }
}
#endif

} // anonymous namespace

void ContentOracle::analyze() {
  Flower flower(wasm, options);
  for (LocationIndex i = 0; i < flower.locations.size(); i++) {
    locationContents[flower.getLocation(i)] = flower.getContents(i);
  }
}

} // namespace wasm
