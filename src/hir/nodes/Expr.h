#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include <memory>
#include "span/Span.h"

namespace jc::hir {
    using span::Span;

    // It would be nice to have Expr and all other nodes as value types,
    // anyway, it would require usage of std::variant which is really inconvenient to work with when there are
    // so many types. Thus, I'll just box Expr.

    // It is impossible now to make Expr ADT instead of using boxing :(

    struct ExprKind {
        using Ptr = std::unique_ptr<ExprKind>;

        enum class Kind {
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

        ExprKind(Kind kind) : kind {kind} {}

        Kind kind;
    };

    struct Expr {
        using Opt = Option<Expr>;
        using List = std::vector<Expr>;

        Expr(ExprKind::Ptr && expr, Span span) : expr {std::move(expr)}, span {span} {}

        ExprKind::Ptr expr;
        Span span;
    };
}

#endif // JACY_HIR_NODES_EXPR_H
