#ifndef JACY_AST_EXPR_BREAKEXPR_H
#define JACY_AST_EXPR_BREAKEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    /// `break EXPR?;`
    struct BreakExpr : Expr {
        BreakExpr(Expr::OptPtr && expr, Span span)
            : Expr {span, Expr::Kind::Break},
              expr {std::move(expr)} {}

        Expr::OptPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_BREAKEXPR_H
