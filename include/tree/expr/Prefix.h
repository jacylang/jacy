#ifndef JACY_PREFIX_H
#define JACY_PREFIX_H

#include "tree/expr/Expr.h"

namespace jc::tree {
    struct Prefix : Expr {
        Prefix(const Location & loc) : Expr(loc, ExprType::Prefix) {}

        parser::Token token;
        expr_ptr rhs;
    };
}

#endif // JACY_PREFIX_H
