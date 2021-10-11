#ifndef JACY_AST_EXPR_LITEXPR_H
#define JACY_AST_EXPR_LITEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct LitExpr : Expr {
        explicit LitExpr(Expr::Ptr && expr, const Span & span)
            : Expr {span, ExprKind::LiteralConstant}, expr {std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LITEXPR_H
