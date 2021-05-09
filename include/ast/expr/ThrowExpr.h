#ifndef JACY_THROWEXPR_H
#define JACY_THROWEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ThrowExpr : Expr {
        ThrowExpr(const Location & loc) : Expr(loc, ExprType::Throw) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_THROWEXPR_H
