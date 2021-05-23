#ifndef JACY_AST_EXPR_QUESTEXPR_H
#define JACY_AST_EXPR_QUESTEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct QuestExpr : Expr {
        QuestExpr(
            expr_ptr expr,
            const Span & span
        ) : Expr(span, ExprKind::Quest),
            expr(std::move(expr)) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_QUESTEXPR_H
