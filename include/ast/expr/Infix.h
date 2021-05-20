#ifndef JACY_INFIX_H
#define JACY_INFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Infix;
    using infix_ptr = std::shared_ptr<Infix>;

    struct Infix : Expr {
        Infix(expr_ptr lhs, const parser::Token & op, expr_ptr rhs)
            : lhs(lhs), op(op), rhs(rhs), Expr(lhs->loc, ExprType::Infix) {}

        expr_ptr lhs;
        parser::Token op;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_INFIX_H
