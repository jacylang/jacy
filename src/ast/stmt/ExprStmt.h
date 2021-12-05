#ifndef JACY_AST_STMT_EXPRSTMT_H
#define JACY_AST_STMT_EXPRSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ExprStmt : Stmt {
        ExprStmt(Expr::Ptr && expr, Span span) : Stmt{span, StmtKind::Expr}, expr{std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_EXPRSTMT_H