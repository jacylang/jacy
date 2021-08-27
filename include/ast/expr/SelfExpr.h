#ifndef JACY_AST_EXPR_SELFEXPR_H
#define JACY_AST_EXPR_SELFEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct SelfExpr : Expr {
        SelfExpr{const Span & span) : Expr(span, ExprKind::This} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_SELFEXPR_H
