#ifndef JACY_BREAKEXPR_H
#define JACY_BREAKEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct BreakExpr : Expr {
        explicit BreakExpr(const Location & loc) : Expr(loc, ExprType::Break) {}
    };
}

#endif // JACY_BREAKEXPR_H
