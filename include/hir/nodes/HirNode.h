#ifndef JACY_HIR_NODES_HIRNODE_H
#define JACY_HIR_NODES_HIRNODE_H

#include "span/Span.h"
#include "resolve/Definition.h"

namespace jc::hir {
    using span::Span;
    using resolve::DefId;

    template<class T>
    using N = std::unique_ptr<T>;

    /// The structure used for saving the closest owner definition
    struct OwnerDef {
        using IdT = uint32_t;

        OwnerDef(DefId defId, IdT initialId) : defId {defId}, nextId {initialId} {
        }

        DefId defId;
        IdT nextId;
    };

    struct HirId {
        HirId(const resolve::DefId & owner, const OwnerDef::IdT & id) : owner {owner}, id {id} {
        }

        static const HirId DUMMY;

        /// The owner of the HIR node (e.g. `struct`)
        resolve::DefId owner;

        /// An identifier unique per each owner, i.e. in each item the first node has id of 0
        OwnerDef::IdT id;
    };

    struct HirNode {
        HirNode(HirId hirId, Span span) : hirId {hirId}, span {span} {
        }

        HirId hirId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_HIRNODE_H
