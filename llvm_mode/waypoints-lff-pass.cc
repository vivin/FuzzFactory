#include "basefeedback.hpp"

using namespace fuzzfactory;

class HeapOpFeedback : public BaseLibFuncFeedback<HeapOpFeedback> {
public:
    HeapOpFeedback(Module& M) : BaseLibFuncFeedback<HeapOpFeedback>(M, "heap", "__afl_heap_dsf") { }

    void visitBasicBlock(BasicBlock& basicBlock) {
        auto irb = insert_before(basicBlock);
        createCreateTraceFileIfNotExistsCall(irb);

        for (Instruction &instruction : basicBlock) {
            if (isa<CallInst>(instruction)) {
                auto &call = cast<CallInst>(instruction);
                Function *function = call.getCalledFunction();
                if (!function) {
                    continue;
                } // todo: only increment if we see a free called after at least 1 alloc. this exercises alloc-free paths.
                  // todo: incorporate asan in some way. basically we want to prioritize paths with buff overflows. so we
                  // todo: want to check if something reads/writes outside a buff area. how to do that? use implementation
                  // todo: like asan? poisoned bytes?? then if we write to loc that has poisoned bytes we increment counter
                  // todo: by 2 maybe? or maybe a separate counter for overflows? then we can combine both heap and this
                  // todo: domain-specific fuzzer. guides coverage towards paths that maximize heap ops and buff overflows.

                if (shouldInterceptFunction(function)) {
                    auto fIrb = insert_before(call);
                    fIrb.CreateCall(DsfIncrementFunction, {DsfMapVariable, getConst(0), getConst(1)});

                    createAppendTraceCall(fIrb, function->getName());
                }
            }
        }
    }
};

FUZZFACTORY_REGISTER_DOMAIN(HeapOpFeedback);