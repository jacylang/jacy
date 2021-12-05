#ifndef JACY_AST_EXPR_POSTFIX_H
#define JACY_AST_EXPR_POSTFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Postfix : Expr {
        Postfix(
            Expr::Ptr && lhs,
            const parser::Token & op,
            Span span
        ) : Expr{span, ExprKind::Postfix},
            lhs{std::move(lhs)},
            op{op} {}

        Expr::Ptr lhs;
        parser::Token op;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_POSTFIX_H