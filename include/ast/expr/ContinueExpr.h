#ifndef JACY_CONTINUEEXPR_H
#define JACY_CONTINUEEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ContinueExpr : Expr {
        explicit ContinueExpr(const Span & span) : Expr(span, ExprKind::Continue) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_CONTINUEEXPR_H
