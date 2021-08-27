#ifndef JACY_HIR_NODES_PARTY_H
#define JACY_HIR_NODES_PARTY_H

#include "hir/nodes/items.h"

namespace jc::hir {
    struct Party {
        using ItemMap = std::map<ItemId, ItemNode>;

        Party(
            Mod && rootMod,
            ItemMap && items
        ) : rootMod(std::move(rootMod)),
            items(std::move(items)) {}

        Mod rootMod;
        ItemMap items;
    };
}

#endif // JACY_HIR_NODES_PARTY_H
