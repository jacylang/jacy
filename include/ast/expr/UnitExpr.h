#ifndef JACY_UNITEXPR_H
#define JACY_UNITEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct UnitExpr : Expr {
        UnitExpr(const Location & loc) : Expr(loc, ExprType::Unit) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_UNITEXPR_H
