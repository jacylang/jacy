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

    struct Item : HirNode {
        Item(ItemKind kind, span::Ident && name, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), kind(kind), name(std::move(name)) {}

        ItemKind kind;
        span::Ident name;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
