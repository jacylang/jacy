#ifndef JACY_AST_EXPR_INFIX_H
#define JACY_AST_EXPR_INFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Infix;
    using infix_ptr = N<Infix>;

    struct Infix : Expr {
        Infix(ExprPtr lhs, const parser::Token & op, ExprPtr rhs, const Span & span)
            : Expr(span, ExprKind::Infix), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

        ExprPtr lhs;
        parser::Token op;
        ExprPtr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INFIX_H
