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
        using Owners = std::map<DefId, OwnerInfo>;

        Party(Owners && owners) : owners {std::move(owners)} {}

        Owners owners;

        const Mod * partyMod() const {
            return owners.at(DefId::ROOT_DEF_ID).ownerNode().asOwnerNode().asParty();
        }
    };
}

#endif // JACY_HIR_NODES_PARTY_H
