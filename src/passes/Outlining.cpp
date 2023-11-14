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
#define ASSERT_OK(val)                                                         \
  if (auto _val = (val); auto err = _val.getErr()) {                           \
    Fatal() << err->msg;                                                       \
  }

namespace wasm {

// Instances of this walker are intended to walk a function at a time, at the
// behest of the owner of the instance.
struct ReconstructStringifyWalker
  : public StringifyWalker<ReconstructStringifyWalker> {

  ReconstructStringifyWalker(Module* wasm)
    : existingBuilder(*wasm), outlinedBuilder(*wasm) {
    this->setModule(wasm);
    DBG(std::cout << "\n\nexistingBuilder: " << &existingBuilder
                  << " outlinedBuilder: " << &outlinedBuilder);
  }

  // As we reconstruct the IR during outlining, we need to know what
  // state we're in to determine which IRBuilder to send the instruction to.
  enum ReconstructState {
    NotInSeq = 0,  // Will not be outlined into a new function.
    InSeq = 1,     // Currently being outlined into a new function.
    InSkipSeq = 2, // A sequence that has already been outlined.
  };
  // We begin with the assumption that we are not currently in a sequence that
  // will be outlined.
  ReconstructState state = ReconstructState::NotInSeq;

  // The list of sequences that will be outlined, contained in the function
  // currently being walked.
  std::vector<OutliningSequence> sequences;
  // Tracks the OutliningSequence the walker is about to outline or is currently
  // outlining.
  uint32_t seqCounter = 0;
  // Counts the number of instructions visited since the function began,
  // corresponds to the indices in the sequences.
  uint32_t instrCounter = 0;
  // A reusable builder for reconstructing the function that will have sequences
  // of instructions removed to be placed into an outlined function. The removed
  // sequences will be replaced by a call to the outlined function.
  IRBuilder existingBuilder;
  // A reusable builder for constructing the outlined functions that will
  // contain repeat sequences found in the program.
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
      ASSERT_OK(existingBuilder.visitBlockStart(curr->block));
      DBG(desc = "Block Start for ");
    } else if (auto curr = reason.getIfStart()) {
      ASSERT_OK(existingBuilder.visitIfStart(curr->iff));
      DBG(desc = "If Start for ");
    } else if (reason.getEnd()) {
      ASSERT_OK(existingBuilder.visitEnd());
      DBG(desc = "End for ");
    } else {
      DBG(desc = "addUniqueSymbol for unimplemented control flow ");
      WASM_UNREACHABLE("unimplemented control flow");
    }
    DBG(printAddUniqueSymbol(desc));
  }

  void visitExpression(Expression* curr) {
    maybeBeginSeq();

    IRBuilder* builder = state == InSeq      ? &outlinedBuilder
                         : state == NotInSeq ? &existingBuilder
                                             : nullptr;
    if (builder) {
      ASSERT_OK(builder->visit(curr));
    }
    DBG(printVisitExpression(curr));

    if (state == InSeq || state == InSkipSeq) {
      maybeEndSeq();
    }
  }

  // Helpers
  void startExistingFunction(Function* func) {
    ASSERT_OK(existingBuilder.build());
    ASSERT_OK(existingBuilder.visitFunctionStart(func));
    instrCounter = 0;
    seqCounter = 0;
    state = NotInSeq;
    DBG(std::cout << "\n\n$" << func->name << " Func Start "
                  << &existingBuilder);
  }

  ReconstructState getCurrState() {
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
    instrCounter++;
    auto currState = getCurrState();
    if (currState != state) {
      switch (currState) {
        case NotInSeq:
          break;
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
    ASSERT_OK(outlinedBuilder.visitFunctionStart(outlinedFunc));

    // Add a local.get instruction for every parameter of the outlined function.
    Signature sig = outlinedFunc->type.getSignature();
    for (Index i = 0; i < sig.params.size(); i++) {
      ASSERT_OK(outlinedBuilder.makeLocalGet(i));
    }

    // Make a call from the existing function to the outlined function. This
    // call will replace the instructions moved to the outlined function.
    ASSERT_OK(existingBuilder.makeCall(outlinedFunc->name, false));
    DBG(std::cout << "\ncreated outlined fn: " << outlinedFunc->name);
  }

  void transitionToInSkipSeq() {
    Function* outlinedFunc =
      getModule()->getFunction(sequences[seqCounter].func);
    ASSERT_OK(existingBuilder.makeCall(outlinedFunc->name, false));
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
      ASSERT_OK(outlinedBuilder.visitEnd());
      DBG(std::cout << "\n\nEnd of sequence to " << &outlinedBuilder);
    }
    // Completed a sequence so increase the seqCounter and reset the state.
    seqCounter++;
  }

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
    // Walk the module and create a "string representation" of the program.
    stringify.walkModule(module);
    // Collect all of the substrings of the string representation that appear
    // more than once in the program.
    auto substrings =
      StringifyProcessor::repeatSubstrings(stringify.hashString);
    DBG(printHashString(stringify.hashString, stringify.exprs));
    // Remove substrings that are substrings of longer repeat substrings.
    substrings = StringifyProcessor::dedupe(substrings);
    // Remove substrings with branch and return instructions until an analysis
    // is performed to see if the intended destination of the branch is included
    // in the substring to be outlined.
    substrings =
      StringifyProcessor::filterBranches(substrings, stringify.exprs);
    // Remove substrings with local.set instructions until Outlining is extended
    // to support arranging for the written values to be returned from the
    // outlined function and written back to the original locals.
    substrings =
      StringifyProcessor::filterLocalSets(substrings, stringify.exprs);
    // Remove substrings with local.get instructions until Outlining is extended
    // to support passing the local values as additional arguments to the
    // outlined function.
    substrings =
      StringifyProcessor::filterLocalGets(substrings, stringify.exprs);
    // Convert substrings to sequences that are more easily outlineable as we
    // walk the functions in a module. Sequences contain indices that
    // are relative to the enclosing function while substrings have indices
    // relative to the entire program.
    auto sequences = makeSequences(module, substrings, stringify);
    outline(module, sequences);
  }

  Name addOutlinedFunction(Module* module,
                           const SuffixTree::RepeatedSubstring& substring,
                           const std::vector<Expression*>& exprs) {
    auto startIdx = substring.StartIndices[0];
    // The outlined functions can be named anything.
    Name outlinedFunc =
      Names::getValidFunctionName(*module, std::string("outline$"));
    // Calculate the function signature for the outlined sequence.
    StackSignature sig;
    for (uint32_t exprIdx = startIdx; exprIdx < startIdx + substring.Length;
         exprIdx++) {
      sig += StackSignature(exprs[exprIdx]);
    }
    // Purposefully inserting the outlined functions at the beginning of the
    // functions vector, so that outlined function assertions are positioned
    // within the test module. This greatly improves readability of the
    // outlining lit tests. Because of the direct manipulation of the functions
    // vector, instead of using the addFunction() helper method, a call to
    // updateFunctionsMap() is required.
    module->functions.insert(
      module->functions.begin(),
      Builder::makeFunction(
        outlinedFunc, Signature(sig.params, sig.results), {}));
    module->updateFunctionsMap();
    return outlinedFunc;
  }

  using Sequences =
    std::unordered_map<Name, std::vector<wasm::OutliningSequence>>;

  // Converts an array of SuffixTree::RepeatedSubstring to a mapping of original
  // functions to repeated sequences they contain. These sequences are ordered
  // by start index by construction because the substring's start indices are
  // ordered.
  Sequences makeSequences(Module* module,
                          const Substrings& substrings,
                          const HashStringifyWalker& stringify) {
    Sequences seqByFunc;
    for (auto& substring : substrings) {
      Name outlinedFunc =
        addOutlinedFunction(module, substring, stringify.exprs);
      for (auto seqIdx : substring.StartIndices) {
        // seqIdx is relative to the entire program; making the idx of the
        // sequence relative to its function is better for outlining because we
        // walk functions.
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
