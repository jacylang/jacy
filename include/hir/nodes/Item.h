#ifndef JACY_HIR_NODES_ITEM_H
#define JACY_HIR_NODES_ITEM_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    enum class ItemKind {

    };

    struct Item : HirNode {
        Item(ItemKind kind, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), kind(kind) {}

        ItemKind kind;
    };
}

#endif // JACY_HIR_NODES_ITEM_H
