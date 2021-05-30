#ifndef JACY_AST_STMT_EXPRSTMT_H
#define JACY_AST_STMT_EXPRSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ExprStmt : Stmt {
        ExprStmt(expr_ptr expr, const Span & span) : expr(std::move(expr)), Stmt(span, StmtKind::Expr) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_EXPRSTMT_H
