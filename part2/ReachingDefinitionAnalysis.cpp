#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include <set>
#include <unordered_set>
#include <unordered_map>
using namespace std;
using namespace llvm;
const unordered_set<string> opHasRet = {
    "alloca", "load",	"select",
    "icmp",	  "fcmp",	"getelementptr",
    "br",	  "switch", "store",		"phi", };
class ReachingInfo : public Info {
public:

set<unsigned>info;
ReachingInfo()
{
}

ReachingInfo(const set<unsigned>& a)
{
    info = a;
}

void print()
{
    for (auto it : info) fprintf(stderr, "%d|", it);
    fprintf(stderr, "\n");
}

static bool equals(ReachingInfo *a, ReachingInfo *b)
{
    return a->info == b->info;
}

static ReachingInfo *join(ReachingInfo *info1, ReachingInfo *info2, ReachingInfo *result)
{
    result->info.insert(info1->info.begin(), info1->info.end());
    result->info.insert(info2->info.begin(), info2->info.end());
    return result;
}
static void join(ReachingInfo *info1, ReachingInfo *result)
{
    result->info.insert(info1->info.begin(), info1->info.end());
}
};
class ReachingDefinitionAnalysis : public DataFlowAnalysis<ReachingInfo, true> {
public:
ReachingDefinitionAnalysis(ReachingInfo &bottom, ReachingInfo &initialState)
    : DataFlowAnalysis(bottom, initialState)
{
}
void flowfunction(Instruction *					I,
                  std::vector<unsigned>&		IncomingEdges,
                  std::vector<unsigned>&		OutgoingEdges,
                  std::vector<ReachingInfo *> & Infos)
{
    string opname = I->getOpcodeName();
    int cat = opHasRet.find(opname) != opHasRet.end() || I->isBinaryOp() ? 1 : 2;

    if (opname == "phi") cat = 3;
    unsigned instidx = InstrToIndex[I];
    ReachingInfo *outinfo = new ReachingInfo();
    for (auto e: IncomingEdges)
        ReachingInfo::join(EdgeToInfo[Edge(e, instidx)], outinfo);
    if (cat == 1) {
        outinfo->info.insert(instidx);
    } else if (cat == 3) {
        BasicBlock *blk = I->getParent();
        for (auto it = blk->begin(); it != blk->end(); ++it)
            if (isa<PHINode>(cast<Instruction>(it)))
                outinfo->info.insert(InstrToIndex[cast<Instruction>(it)]);
    }
    Infos.clear();
    for (int i = 0; i < (int)OutgoingEdges.size(); i++)
        Infos.push_back(new ReachingInfo(outinfo->info));
    delete outinfo;
}
};
namespace {
struct ReachingDefinitionAnalysisPass : public FunctionPass {
    static char ID;
    ReachingDefinitionAnalysisPass() : FunctionPass(ID)
    {
    }

    bool runOnFunction(Function &F) override
    {
        ReachingInfo bottom, initialState;
        ReachingDefinitionAnalysis rda(bottom, initialState);


        rda.runWorklistAlgorithm(&F);
        rda.print();

        return false;
    }
};     // end of struct ReachingDefinitionAnalysisPass
}  // end of anonymous namespace

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching",
                                                      "reaching definition analysis",
                                                      false /* Only looks at CFG */,
                                                      false /* Analysis Pass */);
