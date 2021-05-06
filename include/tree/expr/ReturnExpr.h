#ifndef JACY_RETURNEXPR_H
#define JACY_RETURNEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ReturnExpr : Expr {
        ReturnExpr(const Location & loc) : Expr(loc, ExprType::Return) {}

        expr_ptr expr;
    };
}

#endif // JACY_RETURNEXPR_H
