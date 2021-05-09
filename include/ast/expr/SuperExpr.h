#ifndef JACY_SUPEREXPR_H
#define JACY_SUPEREXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct SuperExpr : Expr {
        SuperExpr(const Location & loc) : Expr(loc, ExprType::Super) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_SUPEREXPR_H
