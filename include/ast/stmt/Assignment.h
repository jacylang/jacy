#ifndef JACY_ASSIGNMENT_H
#define JACY_ASSIGNMENT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Assignment : Stmt {
        Assignment(stmt_ptr lhs, const parser::Token & token, expr_ptr rhs)
            : lhs(lhs), token(token), rhs(rhs), Stmt(lhs->loc, StmtType::Assignment) {}

        stmt_ptr lhs;
        parser::Token token;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_ASSIGNMENT_H
