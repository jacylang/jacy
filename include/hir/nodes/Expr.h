#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include "hir/nodes/Node.h"

namespace jc::hir {
    enum class ExprKind {

    };

    struct Expr : Node {
        Expr(ExprKind kind, const Span & span) : Node(span), kind(kind) {}

        ExprKind kind;
    };
}

#endif // JACY_HIR_NODES_EXPR_H
