#ifndef JACY_THISEXPR_H
#define JACY_THISEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ThisExpr : Expr {
        ThisExpr(const Span & span) : Expr(span, ExprKind::This) {}


        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_THISEXPR_H
