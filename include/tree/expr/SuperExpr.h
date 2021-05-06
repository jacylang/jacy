#ifndef JACY_SUPEREXPR_H
#define JACY_SUPEREXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct SuperExpr : Expr {
        SuperExpr(const Location & loc) : Expr(loc, ExprType::Super) {}
    };
}

#endif // JACY_SUPEREXPR_H
