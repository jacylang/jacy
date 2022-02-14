#ifndef JACY_HIR_NODES_PARTY_H
#define JACY_HIR_NODES_PARTY_H

#include "utils/map.h"
#include "hir/nodes/exprs.h"
#include "hir/nodes/stmts.h"
#include "hir/nodes/items.h"
#include "hir/nodes/types.h"
#include "hir/nodes/patterns.h"

namespace jc::hir {
    using resolve::DefId;

    /// The root node of the party (package)
    struct Party {
        using Items = std::map<ItemId, Item>;
        using Bodies = std::map<BodyId, Body>;

        Party(Mod && rootMod, Items && items, Bodies && bodies)
            : rootMod {std::move(rootMod)}, items {std::move(items)}, bodies {std::move(bodies)} {}

        Mod rootMod;
        Items items;
        Bodies bodies;
    };
}

#endif // JACY_HIR_NODES_PARTY_H
