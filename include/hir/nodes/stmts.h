#ifndef JACY_HIR_NODES_STMTS_H
#define JACY_HIR_NODES_STMTS_H

#include "hir/nodes/Stmt.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct ExprStmt : Stmt {
        ExprStmt(expr_ptr && expr, const HirId & hirId, const Span & span)
            : Stmt(StmtKind::Expr, hirId, span), expr(std::move(expr)) {}

        expr_ptr expr;
    };
}

#endif // JACY_HIR_NODES_STMTS_H
