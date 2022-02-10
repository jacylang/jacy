#ifndef JACY_HIR_NODES_HIRNODE_H
#define JACY_HIR_NODES_HIRNODE_H

#include "span/Span.h"
#include "resolve/Definition.h"

namespace jc::hir {
    using span::Span;
    using span::Ident;
    using ast::NodeId;
    using resolve::DefId;

    template<class T>
    using N = std::unique_ptr<T>;

    /// The structure used for saving the closest owner definition
    struct OwnerDef {
        using IdT = uint32_t;

        OwnerDef(NodeId nodeId, DefId defId, IdT initialId) : nodeId {nodeId}, defId {defId}, nextId {initialId} {}

        NodeId nodeId;
        DefId defId;
        IdT nextId;
    };

    struct HirId {
        HirId(const resolve::DefId & owner, const OwnerDef::IdT & id) : owner {owner}, id {id} {}

        static const HirId DUMMY;

        /// The owner of the HIR node (e.g. `struct`)
        resolve::DefId owner;

        /// An identifier unique per each owner, i.e. in each item the first node has id of 0
        OwnerDef::IdT id;

        DefId::Opt asOwner() const {
            if (id == 0) {
                return owner;
            }
            return None;
        }

        static HirId makeOwner(DefId defId) {
            return HirId(defId, 0);
        }

        friend std::ostream & operator<<(std::ostream & os, const HirId & hirId) {
            return os << hirId.owner << log::Color::DarkBlue << "@" << hirId.id
                      << log::Color::Reset;
        }

        bool operator<(const HirId & other) const {
            return std::tie(owner, id) < std::tie(other.owner, other.id);
        }
    };

    struct HirNode {
        HirNode(HirId hirId, Span span) : hirId {hirId}, span {span} {}

        HirId hirId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_HIRNODE_H
