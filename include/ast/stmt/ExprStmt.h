#ifndef JACY_EXPRSTMT_H
#define JACY_EXPRSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ExprStmt : Stmt {
        ExprStmt(expr_ptr expr) : expr(expr), Stmt(expr->loc, StmtType::Expr) {}

        expr_ptr expr;

        bool isAssignable() const override {
            return expr->isAssignable();
        }

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_EXPRSTMT_H
