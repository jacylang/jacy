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

    struct OwnerNode {
        using ValueT = std::variant<Mod, ItemWrapper>;

        enum class Kind {
            Party,
            Item,
        };

        OwnerNode(Mod && rootMod) : kind {Kind::Party}, value {std::move(rootMod)} {}
        OwnerNode(ItemWrapper && item) : kind {Kind::Item}, value {std::move(item)} {}

        Kind kind;
        ValueT value;
    };

    struct OwnerInfo {
        /// The `ChildId` identifier is used instead of `BodyId` as `BodyId` is actually just a `HirId`
        ///  but we know that this bodies belong to current owner, thus no need to store owner `DefId`
        using Bodies = std::map<ChildId, Body>;
        using Nodes = std::map<ChildId, HirNode::Ptr>;

        OwnerInfo(Bodies && bodies, Nodes && nodes) : bodies {std::move(bodies)}, nodes {std::move(nodes)} {}

        Bodies bodies;
        Nodes nodes;
    };

    /// The root node of the party (package)
    struct Party {
        using Owners = std::map<DefId, OwnerInfo>;

        Party(Owners && owners) : owners {std::move(owners)} {}

        Owners owners;
    };
}

#endif // JACY_HIR_NODES_PARTY_H
