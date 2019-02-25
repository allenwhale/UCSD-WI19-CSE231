#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include <set>
#include <unordered_set>
#include <unordered_map>
using namespace std;
using namespace llvm;
class LivenessInfo : public Info {
public:

set<unsigned>info;
LivenessInfo ()
{
}

LivenessInfo (LivenessInfo *&a)
{
    info = a->info;
}
LivenessInfo (const set<unsigned>&a)
{
    info = a;
}

void print()
{
    for (auto it : info) fprintf(stderr, "%d|", it);
    fprintf(stderr, "\n");
}

static bool equals(LivenessInfo *a, LivenessInfo *b)
{
    return a->info == b->info;
}

static LivenessInfo *join(LivenessInfo *info1, LivenessInfo *info2, LivenessInfo *result)
{
    result->info.insert(info1->info.begin(), info1->info.end());
    result->info.insert(info2->info.begin(), info2->info.end());
    return result;
}
static void join(LivenessInfo *info1, LivenessInfo *result)
{
    result->info.insert(info1->info.begin(), info1->info.end());
}
};
unordered_set<string> opHasRet = { "alloca", "load", "select", "icmp", "fcmp", "getelementptr" };
class LivenessAnalysis : public DataFlowAnalysis<LivenessInfo, false> {
public:
LivenessAnalysis(LivenessInfo & bottom, LivenessInfo & initialState)
    : DataFlowAnalysis(bottom, initialState)
{
}
void flowfunction(Instruction *					I,
                  std::vector<unsigned>&		IncomingEdges,
                  std::vector<unsigned>&		OutgoingEdges,
                  std::vector<LivenessInfo *> & Infos)
{
    set<unsigned> operands;
    unsigned insIdx = InstrToIndex[I];
    int opCat = 0;
    LivenessInfo *outInfo = new LivenessInfo();
    for (int i = 0; i < (int)IncomingEdges.size(); i++)
        LivenessInfo::join(EdgeToInfo[make_pair(IncomingEdges[i], insIdx)], outInfo);
    for (int i = 0; i < (int)I->getNumOperands(); i++) {
        Instruction *ins = (Instruction *)(I->getOperand(i));
		// if (ins == NULL)
		//     printf("null\n");
        if (InstrToIndex.find(ins) != InstrToIndex.end())
            operands.insert(InstrToIndex[ins]);
    }
	// fprintf(stderr, "I: %d\n", insIdx);
	// for (auto x:operands)
	//     fprintf(stderr, "%d ", x);
	// fprintf(stderr, "\n");
    Infos.clear();
    if (I->isBinaryOp() || opHasRet.find(I->getOpcodeName()) != opHasRet.end())
        opCat = 1;
    else if (isa<PHINode>(I))
        opCat = 3;
    else
        opCat = 2;
    if (opCat == 1) {
        LivenessInfo::join(new LivenessInfo(operands), outInfo);
        outInfo->info.erase(insIdx);
        for (int i = 0; i < (int)OutgoingEdges.size(); i++)
            Infos.push_back(outInfo);
    } else if (opCat == 2) {
        LivenessInfo::join(new LivenessInfo(operands), outInfo);
        for (int i = 0; i < (int)OutgoingEdges.size(); i++)
            Infos.push_back(outInfo);
    } else {
        BasicBlock *blk = I->getParent();
        set<unsigned> results;
        for (auto i = blk->begin(); i != blk->end(); i++) {
            Instruction *ins = dyn_cast<Instruction>(i);
            if (isa<PHINode>(ins) && InstrToIndex.find(ins) != InstrToIndex.end())
                outInfo->info.erase(InstrToIndex[ins]);
        }
        for (int i = 0; i < (int)OutgoingEdges.size(); i++)
            Infos.push_back(new LivenessInfo(outInfo));
        for (auto i = blk->begin(); i != blk->end(); i++) {
            Instruction *ins = dyn_cast<Instruction>(i);
            if (isa<PHINode>(ins)) {
                PHINode *phi = (PHINode *)(ins);
                for (int j = 0; j < (int)phi->getNumIncomingValues(); j++) {
                    Instruction *phiv = (Instruction *)(phi->getIncomingValue(j));
                    if (InstrToIndex.find(phiv) == InstrToIndex.end())
                        continue;
                    for (int k = 0; k < (int)OutgoingEdges.size(); k++)
                        if (phiv->getParent() == IndexToInstr[OutgoingEdges[k]]->getParent())
                            LivenessInfo::join(new LivenessInfo(set<unsigned>({ InstrToIndex[phiv] })), Infos[k]);
                }
            }
        }
    }
}
};
namespace {
struct LivenessAnalysisPass : public FunctionPass {
    static char ID;
    LivenessAnalysisPass() : FunctionPass(ID)
    {
    }

    bool runOnFunction(Function &F) override
    {
        LivenessInfo bottom, initialState;
        LivenessAnalysis rda(bottom, initialState);


        rda.runWorklistAlgorithm(&F);
        rda.print();

        return false;
    }
};     // end of struct LivenessAnalysisPass
}  // end of anonymous namespace

char LivenessAnalysisPass::ID = 0;
static RegisterPass<LivenessAnalysisPass> X("cse231-liveness",
                                            "LivenessAnalysis",
                                            false /* Only looks at CFG */,
                                            false /* Analysis Pass */);
