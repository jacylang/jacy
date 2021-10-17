#ifndef JACY_HIR_NODES_PARTY_H
#define JACY_HIR_NODES_PARTY_H

#include "hir/nodes/items.h"

namespace jc::hir {
    struct OwnerNode {
        // Note: `Mod` and `Item` are not separated, `Mod` is only for top-level `Party` module
        using ValueT = std::variant<Mod, Item>;

        enum class Kind {
            Party,
            Item,
        };

        OwnerNode(Mod && mod) : kind {Kind::Party}, node {std::move(mod)} {}
        OwnerNode(Item && item) : kind {Kind::Item}, node {std::move(item)} {}

        Kind kind;
        ValueT node;
    };

    struct Party {
        using ItemMap = std::map<ItemId, ItemNode>;

        Party(
            Mod && rootMod,
            ItemMap && items
        ) : rootMod{std::move(rootMod)},
            items{std::move(items)} {}

        Mod rootMod;
        ItemMap items;
    };
}

#endif // JACY_HIR_NODES_PARTY_H
