#ifndef JACY_HIR_NODES_STMT_H
#define JACY_HIR_NODES_STMT_H

#include "hir/nodes/HirNode.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    enum class StmtKind {
        Let,
        Item,
        Expr,
    };

    struct Stmt : HirNode {
        Stmt(StmtKind kind, const HirId & hirId, const Span & span) : HirNode(hirId, span), kind(kind) {}

        StmtKind kind;
    };

    struct ExprStmt : Stmt {
        ExprStmt(expr_ptr && expr, const HirId & hirId, const Span & span)
            : Stmt(StmtKind::Expr, hirId, span), expr(std::move(expr)) {}

        expr_ptr expr;
    };
}

#endif // JACY_HIR_NODES_STMT_H
