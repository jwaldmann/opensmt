#include "Theory.h"

//
// Unit propagate with simplifications and split equalities into
// inequalities.  If partitions cannot mix, only do the splitting to
// inequalities.
//
bool LIATheory::simplify(const vec<PFRef>& formulas, int curr)
{
    if (this->keepPartitions()) {
        vec<PTRef> & flas = pfstore[formulas[curr]].formulas;
        for (int i = 0; i < flas.size(); ++i) {
            PTRef & fla = flas[i];
            PTRef old = flas[i];
            lialogic.simplifyAndSplitEq(old, fla);
            lialogic.transferPartitionMembership(old, fla);
        }
        pfstore[formulas[curr]].root = getLogic().mkAnd(flas);
    } else {
        PTRef coll_f = getCollateFunction(formulas, curr);
        computeSubstitutions(coll_f, formulas, curr);
        // MB: In LIA we need to add the substitutions back to the formula, since not all equalities have solution
        // given integer model of vars on left-hand side.
        auto const & subst = getTSolverHandler().getSubstitutions();
        auto const & entries = subst.getKeysAndValsPtrs();
        vec<PTRef> args;
        for (auto * entry : entries) {
            if (lialogic.isNumVar(entry->key) && entry->data.sgn == l_True) {
                args.push(lialogic.mkEq(entry->key, entry->data.tr));
            }
        }
        args.push(pfstore[formulas[curr]].root);
        PTRef newRoot = lialogic.mkAnd(args);
        lialogic.simplifyAndSplitEq(newRoot, pfstore[formulas[curr]].root);
    }
    return true;
}
