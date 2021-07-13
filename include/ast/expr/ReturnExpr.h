#ifndef JACY_AST_EXPR_RETURNEXPR_H
#define JACY_AST_EXPR_RETURNEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ReturnExpr : Expr {
        ReturnExpr(opt_expr_ptr && expr, const Span & span)
            : Expr(span, ExprKind::Return),
              expr(std::move(expr)) {}

        opt_expr_ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_RETURNEXPR_H
