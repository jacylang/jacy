#ifndef JACY_HIR_NODES_EXPRS_H
#define JACY_HIR_NODES_EXPRS_H

#include "hir/nodes/Expr.h"

namespace jc::hir {
    struct Array : Expr {
        Array(Expr::List && elements, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Array, hirId, span}, elements{std::move(elements)} {}

        Expr::List elements;
    };

    struct Assign : Expr {
        Assign(Expr::Ptr && lhs, const parser::Token & op, Expr::Ptr && rhs, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Assign, hirId, span}, lhs{std::move(lhs)}, op{op}, rhs{std::move(rhs)} {}

        Expr::Ptr lhs;
        parser::Token op;
        Expr::Ptr rhs;
    };

    struct BlockExpr : Expr {
        BlockExpr(Block && block, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Block, hirId, span}, block{std::move(block)} {}

        Block block;
    };

    struct Invoke : Expr {
        Invoke(Expr::Ptr && lhs, Arg::List && args, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Invoke, hirId, span}, lhs{std::move(lhs)}, args{std::move(args)} {}

        Expr::Ptr lhs;
        Arg::List args;
    };

    struct Literal : Expr {
        // FIXME: Unify `Literal` usage for AST and HIR
        Literal(const parser::Token & token, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Literal, hirId, span}, token{token} {}

        parser::Token token;
    };

    struct Return : Expr {
        Return(Expr::Ptr && expr, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Return, hirId, span}, expr{std::move(expr)} {}

        Expr::Ptr expr;
    };

    struct Tuple : Expr {
        Tuple(Expr::List && values, const HirId & hirId, const Span & span)
            : Expr{ExprKind::Tuple, hirId, span}, values{std::move(values)} {}

        Expr::List values;
    };
}

#endif // JACY_HIR_NODES_EXPRS_H
