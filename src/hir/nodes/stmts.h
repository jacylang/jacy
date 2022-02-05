#ifndef JACY_HIR_NODES_STMTS_H
#define JACY_HIR_NODES_STMTS_H

#include "hir/nodes/Stmt.h"
#include "hir/nodes/patterns.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct ExprStmt : Stmt {
        ExprStmt(Expr::Ptr && expr, HirId hirId, Span span)
            : Stmt {StmtKind::Expr, hirId, span}, expr {std::move(expr)} {}

        Expr::Ptr expr;
    };

    struct LetStmt : Stmt {
        LetStmt(
            Pat::Ptr && pat,
            Type::Ptr && type,
            Expr::Ptr && value,
            HirId hirId,
            Span span
        ) : Stmt {StmtKind::Let, hirId, span},
            pat {std::move(pat)},
            type {std::move(type)},
            value {std::move(value)} {}

        Pat::Ptr pat;
        Type::Ptr type;
        Expr::Ptr value;
    };
}

#endif // JACY_HIR_NODES_STMTS_H
