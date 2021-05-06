#ifndef JACY_SPREADEXPR_H
#define JACY_SPREADEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct SpreadExpr : Expr {
        SpreadExpr(expr_ptr expr, const Location & loc) : expr(expr), Expr(loc, ExprType::Spread) {}

        expr_ptr expr;
    };
}

#endif // JACY_SPREADEXPR_H
