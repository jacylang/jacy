#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    // It would be nice to have Expr and all other nodes as value types,
    // anyway, it would require usage of std::variant which is really inconvenient to work with when there're
    // so many types. Thus I'll just box Expr.
    struct Expr;
    using expr_ptr = N<Expr>;
    using expr_list = std::vector<expr_ptr>;

    enum class ExprKind {
        Array,
    };

    struct Expr : HirNode {
        Expr(ExprKind kind, const HirId & hirId, const Span & span) : HirNode(hirId, span), kind(kind) {}

        ExprKind kind;
    };

    struct Array : Expr {
        Array()
    };
}

#endif // JACY_HIR_NODES_EXPR_H
