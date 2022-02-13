#ifndef JACY_HIR_NODES_STMT_H
#define JACY_HIR_NODES_STMT_H

#include "hir/nodes/HirId.h"

namespace jc::hir {
    struct StmtKind {
        using Ptr = N<StmtKind>;

        enum class Kind {
            Let,
            Item,
            Expr,
        };

        StmtKind(Kind kind) : kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & expr) {
            return static_cast<T*>(expr.get());
        }
    };

    struct Stmt {
        using List = std::vector<Stmt>;

        Stmt(StmtKind::Ptr && stmt, HirId hirId, Span span) : hirId {hirId}, span {span}, stmt {std::move(stmt)} {}

        HirId hirId;
        Span span;
        StmtKind::Ptr stmt;
    };
}

#endif // JACY_HIR_NODES_STMT_H
