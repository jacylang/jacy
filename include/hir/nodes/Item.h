#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Item;
    using item_ptr = std::unique_ptr<Item>;
    using item_list = std::vector<item_ptr>;

    // Wrapper for type strictness
    struct ItemId {
        resolve::DefId defId;
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
        ItemNode(span::Ident && name, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), name(std::move(name)) {}

        span::Ident name;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
