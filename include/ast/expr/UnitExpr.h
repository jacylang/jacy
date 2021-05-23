#ifndef JACY_AST_EXPR_UNITEXPR_H
#define JACY_AST_EXPR_UNITEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct UnitExpr : Expr {
        UnitExpr(const Span & span) : Expr(span, ExprKind::Unit) {}


        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_UNITEXPR_H
