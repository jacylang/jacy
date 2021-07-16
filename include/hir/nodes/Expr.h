#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    enum class ExprKind {

    };

    struct Expr : HirNode {
        Expr(ExprKind kind, const Span & span) : HirNode(span), kind(kind) {}

        ExprKind kind;
    };
}

#endif // JACY_HIR_NODES_EXPR_H
