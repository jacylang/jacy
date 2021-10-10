#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    // It would be nice to have Expr and all other nodes as value types,
    // anyway, it would require usage of std::variant which is really inconvenient to work with when there are
    // so many types. Thus, I'll just box Expr.

    enum class ExprKind {
        Array,
        Assign,
        Block,
        Borrow,
        Break,
        Continue,
        Deref,
        Field,
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

    struct ArrayExpr;
    struct AssignExpr;
    struct BlockExpr;
    struct BorrowExpr;
    struct BreakExpr;
    struct ContinueExpr;
    struct DerefExpr;
    struct FieldExpr;
    struct IfExpr;
    struct InfixExpr;
    struct InvokeExpr;

    struct Expr : HirNode {
        using ValueT = std::variant<>;
        using Opt = Option<Expr>;
        using List = std::vector<Expr>;

        Expr(ExprKind kind, const HirId & hirId, const Span & span) : HirNode {hirId, span}, kind {kind} {
        }

        ExprKind kind;
    };
}

#endif // JACY_HIR_NODES_EXPR_H
