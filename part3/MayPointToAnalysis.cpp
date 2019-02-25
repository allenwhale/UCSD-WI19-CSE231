#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include <set>
#include <unordered_set>
#include <unordered_map>
using namespace std;
using namespace llvm;
class MayPointToInfo : public Info {
public:

map<unsigned, set<unsigned> > pointsTo;
MayPointToInfo ()
{
}

MayPointToInfo (MayPointToInfo *&a)
{
    pointsTo = a->pointsTo;
}
MayPointToInfo (const map<unsigned, set<unsigned> >&a)
{
    pointsTo = a;
}

void print()
{
    for (auto &it : pointsTo) {
        if (it.second.size() == 0)
            continue;
        fprintf(stderr, "R%u->(", it.first);
        for (auto i:it.second)
            fprintf(stderr, "M%u/", i);
        fprintf(stderr, ")|");
    }
    fprintf(stderr, "\n");
}

static bool equals(MayPointToInfo *a, MayPointToInfo *b)
{
    return a->pointsTo == b->pointsTo;
}

static MayPointToInfo *join(MayPointToInfo *info1, MayPointToInfo *info2, MayPointToInfo *result)
{
    MayPointToInfo::join(info1, result);
    MayPointToInfo::join(info2, result);
    return result;
}
static void join(MayPointToInfo *info1, MayPointToInfo *result)
{
    for (auto &p : info1->pointsTo)
        result->pointsTo[p.first].insert(p.second.begin(), p.second.end());
}
};
class MayPointToAnalysis : public DataFlowAnalysis<MayPointToInfo, true> {
public:
MayPointToAnalysis(MayPointToInfo & bottom, MayPointToInfo & initialState)
    : DataFlowAnalysis(bottom, initialState)
{
}
void flowfunction(Instruction *						I,
                  std::vector<unsigned>&			IncomingEdges,
                  std::vector<unsigned>&			OutgoingEdges,
                  std::vector<MayPointToInfo *> &	Infos)
{
    unsigned insIdx = InstrToIndex[I];
    MayPointToInfo *outInfo = new MayPointToInfo();

    for (int i = 0; i < (int)IncomingEdges.size(); i++)
        MayPointToInfo::join(EdgeToInfo[make_pair(IncomingEdges[i], insIdx)], outInfo);
    Infos.clear();
    if (isa<AllocaInst>(I)) {
        outInfo->pointsTo[insIdx].insert(insIdx);
    } else if (isa<BitCastInst>(I)) {
        unsigned rv = InstrToIndex[dyn_cast<Instruction>(I->getOperand(0))];
        outInfo->pointsTo[insIdx].insert(outInfo->pointsTo[rv].begin(), outInfo->pointsTo[rv].end());
    } else if (isa<GetElementPtrInst>(I)) {
        unsigned rp = InstrToIndex[dyn_cast<Instruction>(dyn_cast<GetElementPtrInst>(I)->getPointerOperand())];
        outInfo->pointsTo[insIdx].insert(outInfo->pointsTo[rp].begin(), outInfo->pointsTo[rp].end());
    } else if (isa<LoadInst>(I)) {
        if (I->getType()->isPointerTy()) {
            unsigned rp = InstrToIndex[dyn_cast<Instruction>(dyn_cast<LoadInst>(I)->getPointerOperand())];
            for (auto it : outInfo->pointsTo[rp])
                outInfo->pointsTo[insIdx].insert(outInfo->pointsTo[it].begin(), outInfo->pointsTo[it].end());
        }
    } else if (isa<StoreInst>(I)) {
        unsigned rv = InstrToIndex[dyn_cast<Instruction>(dyn_cast<StoreInst>(I)->getValueOperand())];
        unsigned rp = InstrToIndex[dyn_cast<Instruction>(dyn_cast<StoreInst>(I)->getPointerOperand())];
        auto rpSet = outInfo->pointsTo[rp];
        auto rvSet = outInfo->pointsTo[rv];
        for (auto y : rpSet)
            outInfo->pointsTo[y].insert(rvSet.begin(), rvSet.end());
    } else if (isa<SelectInst>(I)) {
        unsigned rt = InstrToIndex[dyn_cast<Instruction>(dyn_cast<SelectInst>(I)->getTrueValue())];
        unsigned rf = InstrToIndex[dyn_cast<Instruction>(dyn_cast<SelectInst>(I)->getFalseValue())];
        outInfo->pointsTo[insIdx].insert(outInfo->pointsTo[rt].begin(), outInfo->pointsTo[rt].end());
        outInfo->pointsTo[insIdx].insert(outInfo->pointsTo[rf].begin(), outInfo->pointsTo[rf].end());
    } else if (isa<PHINode>(I)) {
        BasicBlock *blk = I->getParent();
        for (auto ins = blk->begin(); ins != blk->end(); ins++) {
            int idx = InstrToIndex[dyn_cast<Instruction>(ins)];
            for (int j = 0; j < (int)ins->getNumOperands(); j++) {
                auto &pset = outInfo->pointsTo[InstrToIndex[dyn_cast<Instruction>(ins->getOperand(j))]];
                outInfo->pointsTo[idx].insert(pset.begin(), pset.end());
            }
        }
    }
    for (int i = 0; i < (int)OutgoingEdges.size(); i++)
        Infos.push_back(outInfo);
}
};
namespace {
struct MayPointToAnalysisPass : public FunctionPass {
    static char ID;
    MayPointToAnalysisPass() : FunctionPass(ID)
    {
    }

    bool runOnFunction(Function &F) override
    {
        MayPointToInfo bottom, initialState;
        MayPointToAnalysis rda(bottom, initialState);


        rda.runWorklistAlgorithm(&F);
        rda.print();

        return false;
    }
};     // end of struct MayPointToAnalysisPass
}  // end of anonymous namespace

char MayPointToAnalysisPass::ID = 0;
static RegisterPass<MayPointToAnalysisPass> X("cse231-maypointto",
                                              "MayPointToAnalysis",
                                              false /* Only looks at CFG */,
                                              false /* Analysis Pass */);
