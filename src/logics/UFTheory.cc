#include "Theory.h"
//
// Simplify with unit propagation, add the diamond equalities if
// present.  If partitions cannot mix, do no simplifications but just
// update the root.
//
bool UFTheory::simplify(const vec<PFRef>& formulas, int curr)
{
    if (this->keepPartitions()) {
        pfstore[formulas[curr]].root = getLogic().mkAnd(pfstore[formulas[curr]].formulas);
        return true;
    }
    else {
        PTRef coll_f = getCollateFunction(formulas, curr);

        PTRef trans = getLogic().learnEqTransitivity(coll_f);
//      pfstore[formulas[curr]].push(trans);
        coll_f = getLogic().isTrue(trans) ? coll_f : getLogic().mkAnd(coll_f, trans);

        bool res = computeSubstitutions(coll_f, formulas, curr);
        vec<Map<PTRef, lbool, PTRefHash>::Pair> units;
        pfstore[formulas[curr]].units.getKeysAndVals(units);
        vec<PTRef> substs_vec;
        for (int i = 0; i < units.size(); i++) {
            if (units[i].data == l_True) {
                substs_vec.push(units[i].key);
            }
        }
        PTRef substs_formula = uflogic.mkAnd(substs_vec);
        pfstore[formulas[curr]].substs = substs_formula;
        return res;
    }
}

