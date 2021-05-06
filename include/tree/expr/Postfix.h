#ifndef JACY_POSTFIX_H
#define JACY_POSTFIX_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct Postfix : Expr {
        Postfix(expr_ptr expr, const parser::Token & token)
            : expr(expr), token(token), Expr(expr->loc, ExprType::Postfix) {}

        expr_ptr expr;
        parser::Token token;
    };
}

#endif // JACY_POSTFIX_H
