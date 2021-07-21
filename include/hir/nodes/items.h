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
        Enum(std::vector<Variant> && variants, const HirId & hirId, const Span & span)
            : Item(ItemKind::Enum, hirId, span), variants(std::move(variants)) {}

        std::vector<Variant> variants;
    };
}

#endif // JACY_HIR_NODES_ITEMS_H
