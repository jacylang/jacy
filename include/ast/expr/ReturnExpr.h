#ifndef JACY_RETURNEXPR_H
#define JACY_RETURNEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct ReturnExpr : Expr {
        ReturnExpr(const Location & loc) : Expr(loc, ExprType::Return) {}

        expr_ptr expr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_RETURNEXPR_H
