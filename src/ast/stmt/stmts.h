#ifndef JACY_SRC_AST_STMT_STMTS_H
#define JACY_SRC_AST_STMT_STMTS_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"
#include "ast/item/Item.h"
#include "ast/fragments/Pat.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct ExprStmt : Stmt {
        ExprStmt(Expr::Ptr && expr, Span span) : Stmt {span, Stmt::Kind::Expr}, expr {std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ItemStmt : Stmt {
        explicit ItemStmt(Item::Ptr && item, Span span)
            : Stmt {span, Stmt::Kind::Item}, item {std::move(item)} {}

        Item::Ptr item;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct LetStmt : Stmt {
        LetStmt(
            Pat::Ptr && pat,
            Type::OptPtr && type,
            Expr::OptPtr && assignExpr,
            Span span
        ) : Stmt {span, Stmt::Kind::Let},
            pat {std::move(pat)},
            type {std::move(type)},
            assignExpr {std::move(assignExpr)} {}

        Pat::Ptr pat;
        Type::OptPtr type;
        Expr::OptPtr assignExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SRC_AST_STMT_STMTS_H
