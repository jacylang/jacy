#ifndef JACY_AST_EXPR_QUESTEXPR_H
#define JACY_AST_EXPR_QUESTEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Postfix : Expr {
        Postfix(
            Expr::Ptr && lhs,
            const parser::Token & op,
            const Span & span
        ) : Expr{span, ExprKind::Quest},
            lhs{std::move(lhs)},
            op{op} {}

        Expr::Ptr lhs;
        parser::Token op;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_QUESTEXPR_H
