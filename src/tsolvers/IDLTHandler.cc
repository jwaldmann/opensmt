//
// Created by Martin Blicha on 10.05.20.
//

#include "IDLTHandler.h"
#include "LIALogic.h"
#include "stpsolver/STPSolver.h"
#include "TreeOps.h"

IDLTHandler::IDLTHandler(SMTConfig& c, LIALogic& l, TermMapper& tmap)
        : TSolverHandler(c, tmap)
        , logic(l)
{
    stpsolver = new STPSolver(config, logic);
    SolverId my_id = stpsolver->getId();
    tsolvers[my_id.id] = stpsolver;
    solverSchedule.push(my_id.id);
}

Logic &IDLTHandler::getLogic()
{
    return logic;
}

const Logic &IDLTHandler::getLogic() const
{
    return logic;
}