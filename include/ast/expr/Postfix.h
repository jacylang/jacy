#ifndef JACY_POSTFIX_H
#define JACY_POSTFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Postfix : Expr {
        Postfix(expr_ptr lhs, const parser::Token & token)
            : lhs(lhs), token(token), Expr(lhs->loc, ExprType::Postfix) {}

        expr_ptr lhs;
        parser::Token token;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_POSTFIX_H
