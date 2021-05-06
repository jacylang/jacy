#ifndef JACY_CONTINUEEXPR_H
#define JACY_CONTINUEEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ContinueExpr : Expr {
        explicit ContinueExpr(const Location & loc) : Expr(loc, ExprType::Continue) {}
    };
}

#endif // JACY_CONTINUEEXPR_H
