#ifndef JACY_RETURNEXPR_H
#define JACY_RETURNEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ReturnExpr : Expr {
        ReturnExpr(opt_expr_ptr expr, const Span & span)
            : Expr(span, ExprKind::Return),
              expr(std::move(expr)) {}

        opt_expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }
        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_RETURNEXPR_H
