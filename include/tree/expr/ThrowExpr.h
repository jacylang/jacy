#ifndef JACY_THROWEXPR_H
#define JACY_THROWEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ThrowExpr : Expr {
        ThrowExpr(const Location & loc) : Expr(loc, ExprType::Throw) {}

        expr_ptr expr;
    };
}

#endif // JACY_THROWEXPR_H
