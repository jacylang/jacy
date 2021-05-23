#ifndef JACY_SPREADEXPR_H
#define JACY_SPREADEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct SpreadExpr : Expr {
        SpreadExpr(const parser::Token & token, expr_ptr expr, const Span & span)
            : token(token), expr(std::move(expr)), Expr(span, ExprKind::Spread) {}

        parser::Token token;
        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SPREADEXPR_H
