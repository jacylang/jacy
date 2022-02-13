#ifndef JACY_HIR_NODES_STMTS_H
#define JACY_HIR_NODES_STMTS_H

#include "hir/nodes/Stmt.h"
#include "hir/nodes/patterns.h"
#include "hir/nodes/fragments.h"
#include "hir/nodes/Item.h"

namespace jc::hir {
    struct ExprStmt : StmtKind {
        ExprStmt(Expr && expr)
            : StmtKind {StmtKind::Kind::Expr}, expr {std::move(expr)} {}

        Expr expr;
    };

    struct LetStmt : StmtKind {
        LetStmt(
            Pat && pat,
            Type::Opt && type,
            Expr::Opt && value
        ) : StmtKind {StmtKind::Kind::Let},
            pat {std::move(pat)},
            type {std::move(type)},
            value {std::move(value)} {}

        Pat pat;
        Type::Opt type;
        Expr::Opt value;
    };

    struct ItemStmt : StmtKind {
        ItemStmt(ItemId && item)
            : StmtKind {StmtKind::Kind::Item}, item {std::move(item)} {}

        ItemId item;
    };
}

#endif // JACY_HIR_NODES_STMTS_H
