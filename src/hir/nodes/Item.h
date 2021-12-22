#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    // Wrapper for type strictness
    struct ItemId {
        using List = std::vector<ItemId>;

        resolve::DefId defId;

        bool operator<(const ItemId & other) const {
            return defId < other.defId;
        }
    };

    enum class ItemKind {
        Enum,
        Func,
        Impl,
        Mod,
        Struct,
        Trait,
        TypeAlias,
        Use,
    };

    /// The base class for all items
    struct Item {
        using Ptr = std::unique_ptr<Item>;

        Item(ItemKind kind) : kind {kind} {
        }

        ItemKind kind;

        template<class T>
        static T * as(const Ptr & item) {
            return static_cast<T*>(item.get());
        }
    };

    /// The wrapper over `ItemInner` and its additional info.
    /// It is useful because we can lower specific item independently and then construct the full `Item`.
    struct ItemWrapper {
        using Vis = ast::Vis;

        ItemWrapper(Vis vis, span::Ident && name, Item::Ptr && item, DefId defId, Span span)
            : vis {vis},
              name {std::move(name)},
              item {std::move(item)},
              defId {defId},
              span {span} {}

        Vis vis;
        span::Ident name;
        Item::Ptr item;
        DefId defId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
