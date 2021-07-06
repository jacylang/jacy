#ifndef JACY_AST_EXPR_SUBSCRIPT_H
#define JACY_AST_EXPR_SUBSCRIPT_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Subscript : Expr {
        Subscript(expr_ptr lhs, expr_list indices, const Span & span)
            : Expr(span, ExprKind::Subscript), lhs(std::move(lhs)), indices(std::move(indices)) {}

        expr_ptr lhs;
        expr_list indices;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_SUBSCRIPT_H
