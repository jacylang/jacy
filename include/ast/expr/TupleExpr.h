#ifndef JACY_AST_EXPR_TUPLEEXPR_H
#define JACY_AST_EXPR_TUPLEEXPR_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct TupleExpr : Expr {
        TupleExpr(expr_list elements, bool allowLambda, const Span & span)
            : Expr(span, ExprKind::Tuple), elements(std::move(elements)), allowLambda(allowLambda) {}

        expr_list elements;
        bool allowLambda{false};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_TUPLEEXPR_H
