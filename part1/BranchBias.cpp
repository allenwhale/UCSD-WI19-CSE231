#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace {
struct BranchBias : public FunctionPass {
    static char ID;
    BranchBias() : FunctionPass(ID) {
    }

    bool runOnFunction(Function &F) override {
        Module *M = F.getParent();
        LLVMContext &C = M->getContext();
        Function *update = cast<Function>(
            M->getOrInsertFunction("updateBranchInfo",
                                   Type::getVoidTy(C),
                                   Type::getInt1Ty(C)));
        Function *print = cast<Function>(
            M->getOrInsertFunction("printOutBranchInfo",
                                   Type::getVoidTy(C)));

        for (auto blk = F.begin(); blk != F.end(); blk++) {
            IRBuilder<> builder(cast<BasicBlock>(blk));
            BranchInst *binst = dyn_cast<BranchInst>(blk->getTerminator());
            if (binst && binst->isConditional()) {
                builder.SetInsertPoint(blk->getTerminator());
                builder.CreateCall(update, { binst->getCondition() });
            }
            if (blk->getTerminator()->getOpcode() == 1) {
                builder.SetInsertPoint(blk->getTerminator());
                builder.CreateCall(print);
            }
        }
        return false;
    }
};     // end of struct TestPass
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb",
                                  "Compute Branch Bias",
                                  false /* Only looks at CFG */,
                                  false /* Analysis Pass */);
