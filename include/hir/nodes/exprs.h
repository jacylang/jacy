#ifndef JACY_HIR_NODES_EXPRS_H
#define JACY_HIR_NODES_EXPRS_H

#include "hir/nodes/Expr.h"

namespace jc::hir {
    struct Array : Expr {
        Array(expr_list && elements, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Array, hirId, span), elements(std::move(elements)) {}

        expr_list elements;
    };

    struct Assign : Expr {
        Assign(expr_ptr && lhs, const parser::Token & op, expr_ptr && rhs, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Assign, hirId, span), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

        expr_ptr lhs;
        parser::Token op;
        expr_ptr rhs;
    };

    struct BlockExpr : Expr {
        BlockExpr(Block && block, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Block, hirId, span), block(std::move(block)) {}

        Block block;
    };

    struct Invoke : Expr {
        Invoke(expr_ptr && lhs, Arg::List && args, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Invoke, hirId, span), lhs(std::move(lhs)), args(std::move(args)) {}

        expr_ptr lhs;
        Arg::List args;
    };

    struct Literal : Expr {
        // FIXME: Unify `Literal` usage for AST and HIR
        Literal(const parser::Token & token, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Literal, hirId, span), token(token) {}

        parser::Token token;
    };

    struct Return : Expr {
        Return(expr_ptr && expr, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Return, hirId, span), expr(std::move(expr)) {}

        expr_ptr expr;
    };

    struct Tuple : Expr {
        Tuple(expr_list && values, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Tuple, hirId, span), values(std::move(values)) {}

        expr_list values;
    };
}

#endif // JACY_HIR_NODES_EXPRS_H
