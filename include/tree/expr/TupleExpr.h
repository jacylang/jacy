#ifndef JACY_TUPLEEXPR_H
#define JACY_TUPLEEXPR_H

#include "tree/fragments/NamedList.h"

namespace jc::tree {
    struct TupleExpr : Expr {
        TupleExpr(named_list_ptr elements, const Location & loc)
            : elements(elements), Expr(loc, ExprType::Tuple) {}

        named_list_ptr elements;
    };
}

#endif // JACY_TUPLEEXPR_H
