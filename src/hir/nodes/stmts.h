#ifndef JACY_HIR_NODES_STMTS_H
#define JACY_HIR_NODES_STMTS_H

#include "hir/nodes/Stmt.h"
#include "hir/nodes/patterns.h"
#include "hir/nodes/fragments.h"
#include "hir/nodes/Item.h"

namespace jc::hir {
    struct ExprStmt : Stmt {
        ExprStmt(ExprWrapper && expr)
            : Stmt {Stmt::Kind::Expr}, expr {std::move(expr)} {}

        ExprWrapper expr;
    };

    struct LetStmt : Stmt {
        LetStmt(
            Pat::Ptr && pat,
            Type::OptPtr && type,
            ExprWrapper::Opt && value
        ) : Stmt {Stmt::Kind::Let},
            pat {std::move(pat)},
            type {std::move(type)},
            value {std::move(value)} {}

        Pat::Ptr pat;
        Type::OptPtr type;
        ExprWrapper::Opt value;
    };

    struct ItemStmt : Stmt {
        ItemStmt(ItemId && item)
            : Stmt {Stmt::Kind::Item}, item {std::move(item)} {}

        ItemId item;
    };
}

#endif // JACY_HIR_NODES_STMTS_H
