#ifndef JACY_LISTEXPR_H
#define JACY_LISTEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ListExpr : Expr {
        ListExpr(expr_list elements, const Location & loc) : elements(elements), Expr(loc, ExprType::List) {}

        expr_list elements;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_LISTEXPR_H
