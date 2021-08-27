#ifndef JACY_AST_EXPR_SPREADEXPR_H
#define JACY_AST_EXPR_SPREADEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct SpreadExpr : Expr {
        SpreadExpr(const parser::Token & token, ExprPtr && expr, const Span & span)
            : Expr(span, ExprKind::Spread), token(token), expr(std::move(expr)) {}

        parser::Token token;
        ExprPtr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_SPREADEXPR_H
