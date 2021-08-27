#ifndef JACY_AST_EXPR_BREAKEXPR_H
#define JACY_AST_EXPR_BREAKEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct BreakExpr : Expr {
        BreakExpr(OptExprPtr && expr, const Span & span)
            : Expr(span, ExprKind::Break),
              expr(std::move(expr)) {}

        OptExprPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_BREAKEXPR_H
