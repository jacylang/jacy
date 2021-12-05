#ifndef JACY_AST_EXPR_BORROWEXPR_H
#define JACY_AST_EXPR_BORROWEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct BorrowExpr : Expr {
        BorrowExpr(
            bool mut,
            Expr::Ptr expr,
            Span span
        ) : Expr{span, ExprKind::Borrow},
            mut{mut},
            expr{std::move(expr)} {}

        bool mut;
        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BORROWEXPR_H