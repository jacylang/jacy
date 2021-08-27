#ifndef JACY_AST_STMT_EXPRSTMT_H
#define JACY_AST_STMT_EXPRSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ExprStmt : Stmt {
        ExprStmt(ExprPtr && expr, const Span & span) : Stmt(span, StmtKind::Expr), expr(std::move(expr)) {}

        ExprPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_EXPRSTMT_H
