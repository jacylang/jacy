#ifndef JACY_AST_EXPR_QUESTEXPR_H
#define JACY_AST_EXPR_QUESTEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct QuestExpr : Expr {
        QuestExpr(
            ExprPtr && expr,
            const Span & span
        ) : Expr(span, ExprKind::Quest),
            expr(std::move(expr)) {}

        ExprPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_QUESTEXPR_H
