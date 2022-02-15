#ifndef JACY_HIR_NODES_STMT_H
#define JACY_HIR_NODES_STMT_H

#include "span/Span.h"

namespace jc::hir {
    struct StmtKind {
        using Ptr = std::unique_ptr<StmtKind>;

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

        Stmt(StmtKind::Ptr && kind, NodeId nodeId, Span span)
            : kind {std::move(kind)}, nodeId {nodeId}, span {span} {}

        StmtKind::Ptr kind;
        NodeId nodeId;
        Span span;
    };
}

#endif // JACY_HIR_NODES_STMT_H
