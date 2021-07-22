#ifndef JACY_HIR_NODES_PARTY_H
#define JACY_HIR_NODES_PARTY_H

#include "hir/nodes/items.h"

namespace jc::hir {
    using item_map = std::map<ItemId, item_ptr>;

    struct Party {
        Party(
            Mod && rootMod,
            item_map && items
        ) : rootMod(std::move(rootMod)),
            items(std::move(items)) {}

        Mod rootMod;
        item_map items;
    };
}

#endif // JACY_HIR_NODES_PARTY_H
