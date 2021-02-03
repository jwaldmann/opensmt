//
// Created by Martin Blicha on 03.02.21.
//

#include "LARewriter.h"

#include "LIALogic.h"
#include "OsmtApiException.h"

std::size_t DivModRewriter::counter = 0;
const char * DivModRewriter::divPrefix = ".div";
const char * DivModRewriter::modPrefix = ".mod";

DivModRewriter::DivModPair DivModRewriter::freshDivModPair(LIALogic & logic) {
    std::string num = std::to_string(counter++);
    std::string divName = divPrefix + num;
    std::string modName = modPrefix + num;
    return {logic.mkNumVar(divName.c_str()), logic.mkNumVar(modName.c_str())};
}

PTRef rewriteDivMod(LIALogic & logic, PTRef root) {
    return DivModRewriter().rewriteDivMod(logic, root);
}

// TODO: figure out how to cache assigned variables to avoid duplication
PTRef DivModRewriter::rewriteDivMod(LIALogic &logic, PTRef root) {
    if (root == PTRef_Undef or not logic.hasSortBool(root)) {
        throw OsmtApiException("Div/Mod rewriting should only be called on formulas, not terms!");
    }
    vec<PTRef> definitions;
    struct DFSEntry {
        DFSEntry(PTRef term) : term(term) {}
        PTRef term;
        unsigned int nextChild = 0;
    };
    // MB: Relies on an invariant that id of a child is lower than id of a parent.
    auto size = Idx(logic.getPterm(root).getId()) + 1;
    std::vector<char> done;
    done.resize(size, 0);
    std::unordered_map<PTRef, PTRef, PTRefHash> substitutions;
    std::vector<DFSEntry> toProcess;
    toProcess.push_back(DFSEntry{root});
    while (not toProcess.empty()) {
        auto & currentEntry = toProcess.back();
        PTRef currentRef = currentEntry.term;
        Pterm const & term = logic.getPterm(currentRef);
        unsigned childrenCount = term.size();
        if (currentEntry.nextChild < childrenCount) {
            PTRef nextChild = term[currentEntry.nextChild];
            ++currentEntry.nextChild;
            if (done[Idx(logic.getPterm(nextChild).getId())] == 0) {
                toProcess.push_back(DFSEntry(nextChild));
            }
            continue;
        }
        // If we are here, we have already processed all children
        assert(done[Idx(term.getId())] == 0);
        vec<PTRef> newArgs(childrenCount);
        bool needsChange = false;
        for (unsigned i = 0; i < childrenCount; ++i) {
            auto it = substitutions.find(term[i]);
            bool childChanged = it != substitutions.end();
            needsChange |= childChanged;
            newArgs[i] = childChanged ? it->second : term[i];
        }
        PTRef newTerm = needsChange ? logic.insertTerm(term.symb(), newArgs) : currentRef;
        if (needsChange) {
            substitutions.insert({currentRef, newTerm});
        }
        // The reference "term" has now been possibly invalidated! Do not access it anymore!
        SymRef symRef = logic.getSymRef(newTerm);
        if (logic.isIntDiv(symRef) || logic.isIntMod(symRef)) {
            // check cache first
            PTRef dividend = logic.getPterm(newTerm)[0];
            PTRef divisor = logic.getPterm(newTerm)[1];
            auto it = divModeCache.find({dividend, divisor});
            bool inCache = (it != divModeCache.end());
            DivModPair divMod = inCache ? it->second : freshDivModPair(logic);
            if (not inCache) {
                divModeCache.insert({{dividend, divisor}, divMod});
            }
            PTRef divVar = divMod.div;
            PTRef modVar = divMod.mod;
            substitutions[currentRef] = logic.isIntDiv(symRef) ? divVar : modVar;
            // collect the definitions to add
            assert(logic.isConstant(divisor));
            auto divisorVal = logic.getNumConst(divisor);
            // general case
            auto upperBound = abs(divisorVal) - 1;
            // dividend = divVar * dividend + modVar
            // 0 <= modVar <= |dividend| - 1
            definitions.push(logic.mkAnd(
                    logic.mkEq(dividend, logic.mkNumPlus(logic.mkNumTimes(divisor, divVar), modVar)),
                    logic.mkAnd(
                            logic.mkNumLeq(logic.getTerm_NumZero(), modVar),
                            logic.mkNumLeq(modVar, logic.mkConst(upperBound))
                            )
                    ));
        }
        done[Idx(logic.getPterm(currentRef).getId())] = 1;
        toProcess.pop_back();
    }
    if (substitutions.empty()) {
        assert(definitions.size() == 0);
        return root;
    }
    auto it = substitutions.find(root);
    assert(it != substitutions.end());
    definitions.push(it->second);
    return logic.mkAnd(definitions);
}