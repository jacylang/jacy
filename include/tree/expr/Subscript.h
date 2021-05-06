#ifndef JACY_SUBSCRIPT_H
#define JACY_SUBSCRIPT_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct Subscript : Expr {
        Subscript(expr_ptr lhs, expr_list indices)
            : lhs(lhs), indices(indices), Expr(lhs->loc, ExprType::Subscript) {}

        expr_ptr lhs;
        expr_list indices;
    };
}

#endif // JACY_SUBSCRIPT_H
