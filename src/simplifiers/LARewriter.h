//
// Created by Martin Blicha on 03.02.21.
//

#ifndef OPENSMT_LAREWRITER_H
#define OPENSMT_LAREWRITER_H

#include "PTRef.h"

#include <unordered_map>
#include <string>

class LIALogic;

// Simple single-use version
PTRef rewriteDivMod(LIALogic & logic, PTRef root);

/// Class for removing div/mod expressions from a formula suitable for re-use
class DivModRewriter {
    static std::size_t counter;
    static const char * divPrefix;
    static const char * modPrefix;
    struct DivModPair {
        PTRef div;
        PTRef mod;
    };
    std::unordered_map<std::pair<PTRef, PTRef>, DivModPair, PTRefPairHash> divModeCache;
    static DivModPair freshDivModPair(LIALogic & logic);
public:
    PTRef rewriteDivMod(LIALogic & logic, PTRef root);
};

#endif //OPENSMT_LAREWRITER_H
