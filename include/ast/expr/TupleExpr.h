#ifndef JACY_AST_EXPR_TUPLEEXPR_H
#define JACY_AST_EXPR_TUPLEEXPR_H

#include "ast/fragments/NamedList.h"

namespace jc::ast {
    struct TupleExpr : Expr {
        TupleExpr(named_list elements, const Span & span)
            : elements(std::move(elements)), Expr(span, ExprKind::Tuple) {}

        named_list elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_TUPLEEXPR_H
