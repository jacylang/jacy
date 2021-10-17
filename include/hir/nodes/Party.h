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
        using Owners = std::map<resolve::DefId, OwnerNode>;
        using Bodies = std::map<BodyId, Body>;

        Party(
            Owners && owners
        ) : owners {std::move(owners)} {}

        Owners owners;
        Bodies bodies;
    };
}

#endif // JACY_HIR_NODES_PARTY_H
