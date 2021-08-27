#ifndef JACY_AST_EXPR_PARENEXPR_H
#define JACY_AST_EXPR_PARENEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ParenExpr : Expr {
        ParenExpr(ExprPtr && expr, const Span & span)
            : Expr(span, ExprKind::Paren), expr(std::move(expr)) {}

        ExprPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PARENEXPR_H
