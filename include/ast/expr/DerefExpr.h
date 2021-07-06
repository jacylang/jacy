#ifndef JACY_AST_EXPR_DEREFEXPR_H
#define JACY_AST_EXPR_DEREFEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct DerefExpr : Expr {
        DerefExpr(
            expr_ptr expr,
            const Span & span
        ) : Expr(span, ExprKind::Deref),
            expr(std::move(expr)) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_DEREFEXPR_H
