#ifndef JACY_INFIX_H
#define JACY_INFIX_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct Infix;
    using infix_ptr = std::shared_ptr<Infix>;
    const auto makeInfix = std::make_shared<Infix>;

    struct Infix : Expr {
        Infix(expr_ptr lhs, const parser::Token & token)
            : lhs(lhs), token(token), rhs(rhs), Expr(lhs->loc, ExprType::Infix) {}

        expr_ptr lhs;
        parser::Token token;
        expr_ptr rhs;
    };
}

#endif // JACY_INFIX_H
