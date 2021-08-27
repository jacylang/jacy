#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct ItemNode;
    struct Item;
    struct ItemId;
    using Item::Ptr = std::unique_ptr<Item>;
    using item_id_list = std::vector<ItemId>;

    // Wrapper for type strictness
    struct ItemId {
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
        Item(ItemKind kind) : kind(kind) {}

        ItemKind kind;
    };

    struct ItemNode : HirNode {
        ItemNode(span::Ident && name, Item::Ptr && item, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), name(std::move(name)), item(std::move(item)) {}

        span::Ident name;
        Item::Ptr item;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
