#ifndef JACY_AST_EXPR_RETURNEXPR_H
#define JACY_AST_EXPR_RETURNEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ReturnExpr : Expr {
        ReturnExpr(Expr::OptPtr && expr, const Span & span)
            : Expr(span, ExprKind::Return),
              expr(std::move(expr)) {}

        Expr::OptPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_RETURNEXPR_H
