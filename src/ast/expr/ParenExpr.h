#ifndef JACY_AST_EXPR_PARENEXPR_H
#define JACY_AST_EXPR_PARENEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ParenExpr : Expr {
        ParenExpr(Expr::Ptr && expr, Span span)
            : Expr {span, Expr::Kind::Paren}, expr {std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_PARENEXPR_H
