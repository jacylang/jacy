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
    struct ItemInner {
        using Ptr = std::unique_ptr<ItemInner>;

        ItemInner(ItemKind kind) : kind{kind} {}

        ItemKind kind;
    };

    /// The wrapper over `ItemInner` and its additional info.
    /// It is useful because we can lower specific item independently and then construct the full `Item`.
    struct Item {
        Item(span::Ident && name, ItemInner::Ptr && item, DefId defId, Span span)
            : name{std::move(name)}, item{std::move(item)}, defId{defId}, span{span} {}

        span::Ident name;
        ItemInner::Ptr item;
        DefId defId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
