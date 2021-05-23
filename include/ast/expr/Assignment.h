#ifndef JACY_ASSIGNMENT_H
#define JACY_ASSIGNMENT_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Assignment : Expr {
        Assignment(expr_ptr lhs, const parser::Token & op, expr_ptr rhs, const span::Span & span)
            : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)), Expr(span, ExprKind::Assign) {}

        expr_ptr lhs;
        parser::Token op;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_ASSIGNMENT_H
