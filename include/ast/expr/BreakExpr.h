#ifndef JACY_BREAKEXPR_H
#define JACY_BREAKEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct BreakExpr : Expr {
        BreakExpr(opt_expr_ptr expr, Span span)
            : Expr(span, ExprKind::Break),
              expr(std::move(expr)) {}

        opt_expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_BREAKEXPR_H
