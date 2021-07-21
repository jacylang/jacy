#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Item;
    using item_ptr = std::unique_ptr<Item>;
    using item_list = std::vector<item_ptr>;

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

    struct Item : HirNode {
        Item(ItemKind kind, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), kind(kind) {}

        ItemKind kind;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
