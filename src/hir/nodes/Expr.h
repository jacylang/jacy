#ifndef JACY_HIR_NODES_EXPR_H
#define JACY_HIR_NODES_EXPR_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    // It would be nice to have Expr and all other nodes as value types,
    // anyway, it would require usage of std::variant which is really inconvenient to work with when there are
    // so many types. Thus, I'll just box Expr.

    // It is impossible now to make Expr ADT instead of using boxing :(

    struct Expr : HirNode {
        using Ptr = std::unique_ptr<Expr>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

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

        Expr(Kind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & expr) {
            return static_cast<T*>(expr.get());
        }
    };
}

#endif // JACY_HIR_NODES_EXPR_H
