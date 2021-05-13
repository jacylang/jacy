#ifndef JACY_TUPLEEXPR_H
#define JACY_TUPLEEXPR_H

#include "ast/fragments/ArgList.h"

namespace jc::ast {
    struct TupleExpr : Expr {
        TupleExpr(arg_list_ptr elements, const Location & loc)
            : elements(elements), Expr(loc, ExprType::Tuple) {}

        arg_list_ptr elements;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TUPLEEXPR_H
