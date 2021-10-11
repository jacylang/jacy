#ifndef JACY_AST_EXPR_LITEXPR_H
#define JACY_AST_EXPR_LITEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct LitExpr : Expr {
        explicit LitExpr(const parser::Token & token, const Span & span)
            : Expr{span, ExprKind::LiteralConstant}, token{token} {}

        parser::Token token;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LITEXPR_H
