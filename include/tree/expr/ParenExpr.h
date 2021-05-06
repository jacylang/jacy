#ifndef JACY_PARENEXPR_H
#define JACY_PARENEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ParenExpr : Expr {
        ParenExpr(expr_ptr expr) : expr(expr), Expr(expr->loc, ExprType::Paren) {}

        expr_ptr expr;
    };
}

#endif // JACY_PARENEXPR_H
