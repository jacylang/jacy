#ifndef JACY_ASSIGNMENT_H
#define JACY_ASSIGNMENT_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Assignment : Expr {
        Assignment(expr_ptr lhs, const parser::Token & op, expr_ptr rhs, const Location & loc)
            : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)), Expr(loc, ExprType::Assign) {}

        expr_ptr lhs;
        parser::Token op;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_ASSIGNMENT_H
