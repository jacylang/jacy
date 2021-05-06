#ifndef JACY_LISTEXPR_H
#define JACY_LISTEXPR_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct ListExpr : Expr {
        ListExpr(expr_list elements, const Location & loc) : elements(elements), Expr(loc, ExprType::List) {}

        expr_list elements;
    };
}

#endif // JACY_LISTEXPR_H
