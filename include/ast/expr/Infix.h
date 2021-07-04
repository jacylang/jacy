#ifndef JACY_AST_EXPR_INFIX_H
#define JACY_AST_EXPR_INFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Infix;
    using infix_ptr = P<Infix>;

    struct Infix : Expr {
        Infix(expr_ptr lhs, const parser::Token & op, expr_ptr rhs, const Span & span)
            : Expr(span, ExprKind::Infix), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

        expr_ptr lhs;
        parser::Token op;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INFIX_H
