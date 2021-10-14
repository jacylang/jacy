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

    // It is impossible now to make Expr ADT instead of using boxing :(

//    struct ArrayExpr;
//    struct AssignExpr;
//    struct BlockExpr;
//    struct BorrowExpr;
//    struct BreakExpr;
//    struct ContinueExpr;
//    struct DerefExpr;
//    struct FieldExpr;
//    struct IfExpr;
//    struct InfixExpr;
//    struct InvokeExpr;
//    struct LitExpr;
//    struct LoopExpr;
//    struct MatchExpr;
//    struct PathExpr;
//    struct PostfixExpr;
//    struct PrefixExpr;
//    struct ReturnExpr;
//    struct TupleExpr;

    struct Expr : HirNode {
        using Ptr = std::unique_ptr<Expr>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;
//        using ValueT = std::variant<
//                ArrayExpr,
//                AssignExpr,
//                BlockExpr,
//                BorrowExpr,
//                BreakExpr,
//                ContinueExpr,
//                DerefExpr,
//                FieldExpr,
//                IfExpr,
//                InfixExpr,
//                InvokeExpr,
//                LitExpr,
//                LoopExpr,
//                MatchExpr,
//                PathExpr,
//                PostfixExpr,
//                PrefixExpr,
//                ReturnExpr,
//                TupleExpr
//            >;

        Expr(ExprKind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        ExprKind kind;
    };
}

#endif // JACY_HIR_NODES_EXPR_H
