#ifndef JACY_HIR_NODES_STMT_H
#define JACY_HIR_NODES_STMT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Stmt : HirNode {
        enum class Kind {
            Let,
            Item,
            Expr,
        };

        using Ptr = N<Stmt>;
        using List = std::vector<Stmt::Ptr>;

        Stmt(Kind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & expr) {
            return static_cast<T*>(expr.get());
        }
    };
}

#endif // JACY_HIR_NODES_STMT_H
