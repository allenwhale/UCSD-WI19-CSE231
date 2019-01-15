#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/IR/InstIterator.h"
#include <iterator>
#include <unordered_map>
using namespace std;
using namespace llvm;

namespace {
struct CountStaticInstructions : public FunctionPass {
    static char ID;
    CountStaticInstructions() : FunctionPass(ID) {
    }

    bool runOnFunction(Function &F) override {
        unordered_map<int, int> count;
        for (auto inst = inst_begin(F); inst != inst_end(F); inst++) {
            count[inst->getOpcode()]++;
        }
        for (auto kv:count) {
            fprintf(stderr, "%s\t%d\n", Instruction::getOpcodeName(kv.first), kv.second);
        }
        return false;
    }
};     // end of struct TestPass
}  // end of anonymous namespace

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi",
                                               "CountStaticInstructions",
                                               false /* Only looks at CFG */,
                                               false /* Analysis Pass */);
