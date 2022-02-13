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

    struct ChildId {
        using ValueT = uint32_t;

        ValueT value;

        /// The identifier of the first child, as `0` is the `ChildId` of owner itself
        static ChildId firstChild() {
            return ChildId {1};
        }

        static ChildId ownerChild() {
            return ChildId {0};
        }

        ChildId operator++(int) {
            return ChildId {value++};
        }
    };

    struct HirId {
        HirId(const resolve::DefId & owner, const ChildId & id) : owner {owner}, id {id} {}

        static const HirId DUMMY;

        /// The owner of the HIR node (e.g. `struct`)
        resolve::DefId owner;

        /// An identifier unique per each owner, i.e. in each item the first node has id of 0
        ChildId id;

        DefId::Opt asOwner() const {
            if (id.value == 0) {
                return owner;
            }
            return None;
        }

        static HirId makeOwner(DefId defId) {
            return HirId(defId, ChildId {0});
        }

        friend std::ostream & operator<<(std::ostream & os, const HirId & hirId) {
            return os << hirId.owner << log::Color::DarkBlue << "@" << hirId.id.value
                      << log::Color::Reset;
        }

        bool operator<(const HirId & other) const {
            return std::tie(owner, id.value) < std::tie(other.owner, other.id.value);
        }
    };

    struct HirNode {
    };
}

#endif // JACY_HIR_NODES_HIRNODE_H
