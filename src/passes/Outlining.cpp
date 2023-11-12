#include "ir/names.h"
#include "ir/utils.h"
#include "pass.h"
#include "passes/stringify-walker.h"
#include "support/suffix_tree.h"
#include "wasm.h"

#define OUTLINING_DEBUG 0

#if OUTLINING_DEBUG
#define DBG(statement) statement
#else
#define DBG(statement)
#endif

// Check a Result or MaybeResult for error and call Fatal() if the error exists.
#define ASSERT_ERR(val)                                                        \
  if (auto _val = (val); auto err = _val.getErr()) {                           \
    Fatal() << err->msg;                                                       \
  }

namespace wasm {

struct ReconstructStringifyWalker
  : public StringifyWalker<ReconstructStringifyWalker> {

  ReconstructStringifyWalker(Module* wasm)
    : existingBuilder(*wasm), outlinedBuilder(*wasm) {
    this->setModule(wasm);
    DBG(std::cout << "\n\nexistingBuilder: " << &existingBuilder
                  << " outlinedBuilder: " << &outlinedBuilder);
  }

  enum ReconstructState {
    NotInSeq = 0,
    InSeq = 1,
    InSkipSeq = 2,
  };
  ReconstructState state = ReconstructState::NotInSeq;
  uint32_t seqCounter = 0;
  uint32_t instrCounter = 0;
  std::vector<OutliningSequence> sequences;
  std::unordered_map<Name, std::vector<OutliningSequence>> seqToFunc;
  IRBuilder existingBuilder;
  IRBuilder outlinedBuilder;

  void addUniqueSymbol(SeparatorReason reason) {
    if (auto curr = reason.getFuncStart()) {
      startExistingFunction(curr->func);
      return;
    }

    // instrCounter is managed manually and incremented at the beginning of
    // addUniqueSymbol() and visitExpression(), except for the case where we are
    // starting a new function, as that resets the counters back to 0.
    instrCounter++;

    DBG(std::string desc);
    if (auto curr = reason.getBlockStart()) {
      ASSERT_ERR(existingBuilder.visitBlockStart(curr->block));
      DBG(desc = "Block Start for ";);
    } else if (auto curr = reason.getIfStart()) {
      ASSERT_ERR(existingBuilder.visitIfStart(curr->iff));
      DBG(desc = "If Start for ";);
    } else if (reason.getEnd()) {
      ASSERT_ERR(existingBuilder.visitEnd());
      DBG(desc = "End for ";);
    } else {
      DBG(desc = "addUniqueSymbol for unhandled instruction ";);
      WASM_UNREACHABLE("unimplemented instruction");
    }
    DBG(printAddUniqueSymbol(desc););
  }
  void visitExpression(Expression* curr) {
    maybeBeginSeq();

    IRBuilder* builder = state == InSeq      ? &outlinedBuilder
                         : state == NotInSeq ? &existingBuilder
                                             : nullptr;
    if (builder) {
      ASSERT_ERR(builder->visit(curr));
    }
    DBG(printVisitExpression(curr));

    if (state == InSeq || state == InSkipSeq) {
      maybeEndSeq();
    }
  }
  // Helpers
  void startExistingFunction(Function* func) {
    ASSERT_ERR(existingBuilder.build());
    ASSERT_ERR(existingBuilder.visitFunctionStart(func));
    instrCounter = 0;
    seqCounter = 0;
    state = NotInSeq;
    DBG(std::cout << "\n\n$" << func->name << " Func Start "
                  << &existingBuilder);
  }
  ReconstructState getCurrState() {
    instrCounter++;
    // We are either in a sequence or not in a sequence. If we are in a sequence
    // and have already created the body of the outlined function that will be
    // called, then we will skip instructions, otherwise we add the instructions
    // to the outlined function. If we are not in a sequence, then the
    // instructions are sent to the existing function.
    if (seqCounter < sequences.size() &&
        instrCounter >= sequences[seqCounter].startIdx &&
        instrCounter < sequences[seqCounter].endIdx) {
      return getModule()->getFunction(sequences[seqCounter].func)->body
               ? InSkipSeq
               : InSeq;
    }
    return NotInSeq;
  }
  void maybeBeginSeq() {
    auto currState = getCurrState();
    if (currState != state) {
      switch (currState) {
        case NotInSeq:
          return;
        case InSeq:
          transitionToInSeq();
          break;
        case InSkipSeq:
          transitionToInSkipSeq();
          break;
      }
    }
    state = currState;
  }
  void transitionToInSeq() {
    Function* outlinedFunc =
      getModule()->getFunction(sequences[seqCounter].func);
    ASSERT_ERR(outlinedBuilder.visitFunctionStart(outlinedFunc));

    // Add a local.get instruction for every parameter of the outlined function.
    Signature sig = outlinedFunc->type.getSignature();
    for (Index i = 0; i < sig.params.size(); i++) {
      ASSERT_ERR(outlinedBuilder.makeLocalGet(i));
    }

    // Make a call from the existing function to the outlined function. This
    // call will replace the instructions moved to the outlined function
    ASSERT_ERR(existingBuilder.makeCall(outlinedFunc->name, false));
    DBG(std::cout << "\ncreated outlined fn: " << outlinedFunc->name);
  }

  void transitionToInSkipSeq() {
    Function* outlinedFunc =
      getModule()->getFunction(sequences[seqCounter].func);
    ASSERT_ERR(existingBuilder.makeCall(outlinedFunc->name, false));
    DBG(std::cout << "\n\nstarting to skip instructions "
                  << sequences[seqCounter].startIdx << " - "
                  << sequences[seqCounter].endIdx - 1 << " to "
                  << sequences[seqCounter].func
                  << " and adding call() instead");
  }
  void maybeEndSeq() {
    if (instrCounter + 1 == sequences[seqCounter].endIdx) {
      transitionToNotInSeq();
      state = NotInSeq;
    }
  }
  void transitionToNotInSeq() {
    if (state == InSeq) {
      ASSERT_ERR(outlinedBuilder.visitEnd());
      DBG(std::cout << "\n\nEnd of sequence to " << &outlinedBuilder);
    }
    // Completed a sequence so increase the seqCounter and reset the state
    seqCounter++;
  }

#define RECONSTRUCT_DEBUG 0

#if OUTLINING_DEBUG
  void printAddUniqueSymbol(std::string desc) {
    std::cout << "\n"
              << desc << std::to_string(instrCounter) << " to "
              << &existingBuilder;
  }
  void printVisitExpression(Expression* curr) {
    auto* builder = state == InSeq      ? &outlinedBuilder
                    : state == NotInSeq ? &existingBuilder
                                        : nullptr;
    auto verb = state == InSkipSeq ? "skipping " : "adding ";
    std::cout << "\n"
              << verb << std::to_string(instrCounter) << ": "
              << ShallowExpression{curr} << " to " << builder;
  }
#endif
};

struct Outlining : public Pass {
  void run(Module* module) {
    HashStringifyWalker stringify;
    stringify.walkModule(module);
    auto substrings =
      StringifyProcessor::repeatSubstrings(stringify.hashString);
    DBG(printHashString(stringify.hashString, stringify.exprs));
    substrings = StringifyProcessor::dedupe(substrings);
    substrings =
      StringifyProcessor::filterBranches(substrings, stringify.exprs);
    substrings =
      StringifyProcessor::filterLocalSets(substrings, stringify.exprs);
    substrings =
      StringifyProcessor::filterLocalGets(substrings, stringify.exprs);

    auto sequences = makeSequences(module, substrings, stringify);
    outline(module, sequences);
  }

  Name addOutlinedFunction(Module* module,
                           const SuffixTree::RepeatedSubstring& substring,
                           const std::vector<Expression*>& exprs) {
    auto startIdx = substring.StartIndices[0];
    // The outlined functions can be named anything. Use the start index of
    // the first time the outlined sequence was seen in the module to help
    // with debugging later.
    Name outlinedFunc = Names::getValidFunctionName(
      *module, std::string("outline$") + std::to_string(startIdx));
    // Calculate the function signature for the outlined sequence.
    StackSignature sig;
    for (uint32_t exprIdx = startIdx; exprIdx < startIdx + substring.Length;
         exprIdx++) {
      sig += StackSignature(exprs[exprIdx]);
    }
    module->addFunction(Builder::makeFunction(
      outlinedFunc, Signature(sig.params, sig.results), {}));
    return outlinedFunc;
  }

  using Sequences =
    std::unordered_map<Name, std::vector<wasm::OutliningSequence>>;

  // Converts an array of SuffixTree::RepeatedSubstring to a mapping of original
  // functions to repeated sequences they contain.
  Sequences makeSequences(Module* module,
                          const Substrings& substrings,
                          const HashStringifyWalker& stringify) {
    Sequences seqByFunc;
    for (auto& substring : substrings) {
      Name outlinedFunc =
        addOutlinedFunction(module, substring, stringify.exprs);
      for (auto seqIdx : substring.StartIndices) {
        // seqIdx is relative to the entire program; making the idx of the
        // sequence relative to its function makes outlining easier because we
        // walk functions
        auto [relativeIdx, existingFunc] = stringify.makeRelative(seqIdx);
        auto seq = OutliningSequence(
          relativeIdx, relativeIdx + substring.Length, outlinedFunc);
        seqByFunc[existingFunc].push_back(seq);
      }
    }
    return seqByFunc;
  }

  void outline(Module* module, Sequences seqByFunc) {
    // TODO: Make this a function-parallel sub-pass.
    ReconstructStringifyWalker reconstruct(module);
    std::vector<Name> keys(seqByFunc.size());
    std::transform(seqByFunc.begin(),
                   seqByFunc.end(),
                   keys.begin(),
                   [](auto pair) { return pair.first; });
    for (auto func : keys) {
      reconstruct.sequences = std::move(seqByFunc[func]);
      reconstruct.doWalkFunction(module->getFunction(func));
    }
  }

#if OUTLINING_DEBUG
  void printHashString(const std::vector<uint32_t>& hashString,
                       const std::vector<Expression*>& exprs) {
    std::cout << "\n\n";
    for (Index idx = 0; idx < hashString.size(); idx++) {
      Expression* expr = exprs[idx];
      if (expr) {
        std::cout << idx << " - " << hashString[idx] << ": "
                  << ShallowExpression{expr} << "\n";
      } else {
        std::cout << idx << ": unique symbol\n";
      }
    }
  }
#endif
};

Pass* createOutliningPass() { return new Outlining(); }

} // namespace wasm
