#ifndef JACY_AST_EXPR_BREAKEXPR_H
#define JACY_AST_EXPR_BREAKEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct BreakExpr : Expr {
        BreakExpr(OptExpr::Ptr && expr, const Span & span)
            : Expr(span, ExprKind::Break),
              expr(std::move(expr)) {}

        OptExpr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_BREAKEXPR_H
