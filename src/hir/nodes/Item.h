#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "resolve/Definition.h"

namespace jc::hir {
    using resolve::DefId;

    // Wrapper for type strictness
    struct ItemId {
        using List = std::vector<ItemId>;

        resolve::DefId defId;

        bool operator<(const ItemId & other) const {
            return defId < other.defId;
        }
    };

    /// The base class for all items
    struct ItemKind {
        using Ptr = std::unique_ptr<ItemKind>;

        enum class Kind {
            Enum,
            Func,
            Impl,
            Mod,
            Struct,
            Trait,
            TypeAlias,
            Use,
        };

        ItemKind(Kind kind) : kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & item) {
            return static_cast<T *>(item.get());
        }
    };

    /// The wrapper over `Item` and its additional info.
    /// It is useful because we can lower specific item independently and then construct the full `Item`.
    struct Item {
        using Vis = ast::Vis;

        Item(Vis vis, span::Ident && name, ItemKind::Ptr && kind, DefId defId, NodeId nodeId, Span span)
            : vis {vis},
              name {std::move(name)},
              kind {std::move(kind)},
              defId {defId},
              nodeId {nodeId},
              span {span} {}

        Vis vis;
        span::Ident name;
        ItemKind::Ptr kind;
        DefId defId;
        NodeId nodeId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
