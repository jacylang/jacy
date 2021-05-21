#ifndef JACY_EXPRSTMT_H
#define JACY_EXPRSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ExprStmt : Stmt {
        ExprStmt(expr_ptr expr)
            : expr(std::move(expr)), Stmt(expr->span, StmtKind::Expr) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_EXPRSTMT_H
