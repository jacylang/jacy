#ifndef JACY_AST_EXPR_PREFIX_H
#define JACY_AST_EXPR_PREFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Prefix : Expr {
        Prefix(const parser::Token & op, ExprPtr && rhs, const Span & span)
            : Expr(span, ExprKind::Prefix), op(op), rhs(std::move(rhs)) {}

        parser::Token op;
        ExprPtr rhs;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PREFIX_H
