#ifndef JACY_AST_STMT_LETSTMT_H
#define JACY_AST_STMT_LETSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Type.h"
#include "ast/fragments/Pat.h"

namespace jc::ast {
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

#endif // JACY_AST_STMT_LETSTMT_H
