#ifndef JACY_AST_EXPR_INFIX_H
#define JACY_AST_EXPR_INFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Infix : Expr {
        Infix(Expr::Ptr lhs, const parser::Token & op, Expr::Ptr rhs, const Span & span)
            : Expr{span, ExprKind::Infix), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)} {}

        Expr::Ptr lhs;
        parser::Token op;
        Expr::Ptr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INFIX_H
