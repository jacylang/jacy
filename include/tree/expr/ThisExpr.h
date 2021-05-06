#ifndef JACY_THISEXPR_H
#define JACY_THISEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ThisExpr : Expr {
        ThisExpr(const Location & loc) : Expr(loc, ExprType::This) {}
    };
}

#endif // JACY_THISEXPR_H
