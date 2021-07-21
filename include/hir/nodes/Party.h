#ifndef JACY_HIR_NODES_PARTY_H
#define JACY_HIR_NODES_PARTY_H

#include "hir/nodes/items.h"

namespace jc::hir {
    struct Party {
        Party(Mod && rootMod) : rootMod(std::move(rootMod)) {}

        Mod rootMod;
    };
}

#endif // JACY_HIR_NODES_PARTY_H