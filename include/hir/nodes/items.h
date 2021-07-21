#ifndef JACY_HIR_NODES_ITEMS_H
#define JACY_HIR_NODES_ITEMS_H

#include "hir/nodes/Item.h"
#include "hir/nodes/Expr.h"
#include "span/Ident.h"

namespace jc::hir {
    struct Variant : HirNode {
        enum class Kind {

        };

        span::Ident ident;
    };

    struct Enum : Item {
        Enum(std::vector<Variant> && variants) : Item(ItemKind::Enum), variants(std::move(variants)) {}

        std::vector<Variant> variants;
    };

    struct Func : Item {};

    struct Impl : Item {};

    struct Mod : Item {
        Mod(item_node_list && items) : Item(ItemKind::Mod), items(std::move(items)) {}

        item_node_list items;
    };

    struct Struct : Item {};

    struct Trait : Item {};

    struct TypeAlias : Item {};

    struct UseDecl : Item {};
}

#endif // JACY_HIR_NODES_ITEMS_H
