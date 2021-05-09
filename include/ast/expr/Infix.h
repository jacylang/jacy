#ifndef JACY_INFIX_H
#define JACY_INFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Infix;
    using infix_ptr = std::shared_ptr<Infix>;

    struct Infix : Expr {
        Infix(expr_ptr lhs, const parser::Token & token, expr_ptr rhs)
            : lhs(lhs), token(token), rhs(rhs), Expr(lhs->loc, ExprType::Infix) {}

        expr_ptr lhs;
        parser::Token token;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    const auto makeInfix = [](expr_ptr lhs, const parser::Token & token, expr_ptr rhs) {
        return std::make_shared<Infix>(lhs, token, rhs);
    };
}

#endif // JACY_INFIX_H
