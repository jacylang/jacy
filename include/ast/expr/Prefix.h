#ifndef JACY_PREFIX_H
#define JACY_PREFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Prefix : Expr {
        Prefix(const parser::Token & op, expr_ptr rhs, const Location & loc)
            : op(op), rhs(rhs), Expr(loc, ExprType::Prefix) {}

        parser::Token op;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_PREFIX_H
