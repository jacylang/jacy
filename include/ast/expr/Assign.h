#ifndef JACY_AST_EXPR_ASSIGN_H
#define JACY_AST_EXPR_ASSIGN_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Assign : Expr {
        Assign(ExprPtr && lhs, const parser::Token & op, ExprPtr && rhs, const span::Span & span)
            : Expr(span, ExprKind::Assign), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

        ExprPtr lhs;
        parser::Token op;
        ExprPtr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_ASSIGN_H
