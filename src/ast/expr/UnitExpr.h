#ifndef JACY_AST_EXPR_UNITEXPR_H
#define JACY_AST_EXPR_UNITEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct UnitExpr : Expr {
        UnitExpr(Span span) : Expr {span, Expr::Kind::Unit} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_UNITEXPR_H
