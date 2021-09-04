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

    struct Item {
        using Ptr = std::unique_ptr<Item>;

        Item(ItemKind kind) : kind{kind} {}

        ItemKind kind;
    };

    struct ItemNode {
        ItemNode(span::Ident && name, Item::Ptr && item, DefId defId, const Span & span)
            : name{std::move(name)}, item{std::move(item)}, defId{defId}, span{span} {}

        span::Ident name;
        Item::Ptr item;
        DefId defId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
