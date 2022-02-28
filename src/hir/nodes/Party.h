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
        using Items = ItemId::Map<Item>;
        using TraitMembers = TraitMemberId::Map<TraitMember>;
        using ImplMembers = ImplMemberId::Map<ImplMember>;
        using Bodies = std::map<BodyId, Body>;

        Party(
            Mod && rootMod,
            Items && items,
            TraitMembers && traitMembers,
            ImplMembers && implMembers,
            Bodies && bodies
        ) : rootMod {std::move(rootMod)},
            items {std::move(items)},
            traitMembers {std::move(traitMembers)},
            implMembers {std::move(implMembers)},
            bodies {std::move(bodies)} {}

        Mod rootMod;
        Items items;
        TraitMembers traitMembers;
        ImplMembers implMembers;
        Bodies bodies;

        const Item & item(ItemId itemId) const {
            return items.at(itemId);
        }

        const TraitMember & traitMember(TraitMemberId traitMemberId) const {
            return traitMembers.at(traitMemberId);
        }

        const ImplMember & implMember(ImplMemberId implMemberId) const {
            return implMembers.at(implMemberId);
        }

        const Body & body(BodyId bodyId) const {
            return bodies.at(bodyId);
        }
    };
}

#endif // JACY_HIR_NODES_PARTY_H
