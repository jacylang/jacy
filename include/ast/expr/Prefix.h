#ifndef JACY_PREFIX_H
#define JACY_PREFIX_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Prefix : Expr {
        Prefix(const parser::Token & op, expr_ptr rhs)
            : op(op), rhs(rhs), Expr(op.loc, ExprType::Prefix) {}

        parser::Token op;
        expr_ptr rhs;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    const auto makePrefix = [](const parser::Token & token, expr_ptr rhs) {
        return std::make_shared<Prefix>(token, rhs);
    };
}

#endif // JACY_PREFIX_H
