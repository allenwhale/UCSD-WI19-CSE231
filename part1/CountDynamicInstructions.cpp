#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include <iterator>
#include <unordered_map>
using namespace std;
using namespace llvm;


namespace {
struct CountDynamicInstructions : public FunctionPass {
    static char ID;
    CountDynamicInstructions() : FunctionPass(ID) {
    }

    bool runOnFunction(Function &F) override {
		// Module *M =
        Module *M = F.getParent();
        LLVMContext &C = M->getContext();
        Function *update = cast<Function>(
            M->getOrInsertFunction("updateInstrInfo",
                                   Type::getVoidTy(C),
                                   Type::getInt32Ty(C),
                                   Type::getInt32PtrTy(C),
                                   Type::getInt32PtrTy(C)));

        Function *print = cast<Function>(
            M->getOrInsertFunction("printOutInstrInfo",
                                   Type::getVoidTy(C)));

        for (auto blk = F.begin(); blk != F.end(); blk++) {
            unordered_map<int, int> count;
            for (auto inst = blk->begin(); inst != blk->end(); inst++) {
                count[inst->getOpcode()]++;
            }
            IRBuilder<> builder(cast<BasicBlock>(blk));
            vector<Constant*> keys, values;
            for (auto kv:count) {
                keys.push_back(ConstantInt::get(Type::getInt32Ty(C), kv.first));
                values.push_back(ConstantInt::get(Type::getInt32Ty(C), kv.second));
            }
            int len = count.size();
            ArrayType *array_type = ArrayType::get(Type::getInt32Ty(C), len);
            GlobalVariable *global_keys = new GlobalVariable(
                *M,                                 //module
                array_type,                         //type
                true,                               //Constant
                GlobalVariable::InternalLinkage,    //linkage
                ConstantArray::get(array_type, keys));
            GlobalVariable *global_values = new GlobalVariable(
                *M,                                 //module
                array_type,                         //type
                true,                               //Constant
                GlobalVariable::InternalLinkage,    //linkage
                ConstantArray::get(array_type, values));
            Value* idx[2] = { ConstantInt::get(Type::getInt32Ty(C), 0),
                              ConstantInt::get(Type::getInt32Ty(C), 0) };
            vector<Value*> args;
            args.push_back(ConstantInt::get(Type::getInt32Ty(C), len));
            args.push_back(builder.CreateInBoundsGEP(global_keys, idx));
            args.push_back(builder.CreateInBoundsGEP(global_values, idx));
            builder.SetInsertPoint(blk->getTerminator());
            builder.CreateCall(update, args);
            if (blk->getTerminator()->getOpcode() == 1) {
                builder.SetInsertPoint(blk->getTerminator());
                builder.CreateCall(print);
            }
        }
        return false;
    }
};     // end of struct TestPass
}  // end of anonymous namespace

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X("cse231-cdi",
                                                "CountDynamicInstructions",
                                                false /* Only looks at CFG */,
                                                false /* Analysis Pass */);
