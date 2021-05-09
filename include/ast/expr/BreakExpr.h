#ifndef JACY_BREAKEXPR_H
#define JACY_BREAKEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct BreakExpr : Expr {
        explicit BreakExpr(const Location & loc) : Expr(loc, ExprType::Break) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_BREAKEXPR_H
