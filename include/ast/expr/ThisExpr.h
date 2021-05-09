#ifndef JACY_THISEXPR_H
#define JACY_THISEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ThisExpr : Expr {
        ThisExpr(const Location & loc) : Expr(loc, ExprType::This) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_THISEXPR_H
