#ifndef JACY_EXPRSTMT_H
#define JACY_EXPRSTMT_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ExprStmt : Stmt {
        ExprStmt(expr_ptr expr) : expr(expr), Stmt(expr->loc, StmtType::Expr) {}

        expr_ptr expr;

        bool isAssignable() const override {
            return expr->isAssignable();
        }
    };
}

#endif // JACY_EXPRSTMT_H
