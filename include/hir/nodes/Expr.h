#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include "hir/nodes/HirNode.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    // It would be nice to have Expr and all other nodes as value types,
    // anyway, it would require usage of std::variant which is really inconvenient to work with when there're
    // so many types. Thus I'll just box Expr.
    struct Expr;
    using expr_ptr = N<Expr>;
    using expr_list = std::vector<expr_ptr>;

    enum class ExprKind {
        Array,
        Assign,
        Block,
        If,
        Infix,
        Invoke,
        Literal,
        Loop,
        Match,
        Path,
        Prefix,
        Return,
        Tuple,
    };

    struct Expr : HirNode {
        Expr(ExprKind kind, const HirId & hirId, const Span & span) : HirNode(hirId, span), kind(kind) {}

        ExprKind kind;
    };

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
        Invoke(expr_ptr && lhs, arg_list && args, const HirId & hirId, const Span & span)
            : Expr(ExprKind::Invoke, hirId, span), lhs(std::move(lhs)), args(std::move(args)) {}

        expr_ptr lhs;
        arg_list args;
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

#endif // JACY_HIR_NODES_EXPR_H
