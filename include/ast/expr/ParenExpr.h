#ifndef JACY_PARENEXPR_H
#define JACY_PARENEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ParenExpr : Expr {
        ParenExpr(expr_ptr expr, const Span & span)
            : expr(std::move(expr)), Expr(span, ExprKind::Paren) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_PARENEXPR_H
