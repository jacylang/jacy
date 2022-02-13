#ifndef JACY_HIR_NODES_STMT_H
#define JACY_HIR_NODES_STMT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Stmt {
        using Ptr = N<Stmt>;
        using List = std::vector<Stmt::Ptr>;

        enum class Kind {
            Let,
            Item,
            Expr,
        };

        Stmt(Kind kind) : kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & expr) {
            return static_cast<T*>(expr.get());
        }
    };

    struct StmtWrapper {
        StmtWrapper(Stmt::Ptr && stmt, HirId hirId, Span span) : hirId {hirId}, span {span}, stmt {std::move(stmt)} {}

        HirId hirId;
        Span span;
        Stmt::Ptr stmt;
    };
}

#endif // JACY_HIR_NODES_STMT_H
