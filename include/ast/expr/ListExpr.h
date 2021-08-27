#ifndef JACY_AST_EXPR_LISTEXPR_H
#define JACY_AST_EXPR_LISTEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ListExpr : Expr {
        ListExpr(ExprList && elements, const Span & span)
            : Expr(span, ExprKind::List), elements(std::move(elements)) {}

        ExprList elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LISTEXPR_H
