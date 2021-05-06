#ifndef JACY_INVOKE_H
#define JACY_INVOKE_H

#include "tree/fragments/NamedList.h"

namespace jc::tree {
    struct Invoke : Expr {
        Invoke(expr_ptr lhs, named_list_ptr args)
            : lhs(lhs), args(args), Expr(lhs->loc, ExprType::Invoke) {}

        expr_ptr lhs;
        named_list_ptr args;
    };
}

#endif // JACY_INVOKE_H
