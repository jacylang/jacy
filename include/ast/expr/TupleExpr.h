#ifndef JACY_TUPLEEXPR_H
#define JACY_TUPLEEXPR_H

#include "ast/fragments/NamedList.h"

namespace jc::ast {
    struct TupleExpr : Expr {
        TupleExpr(named_list_ptr elements, const Span & span)
            : elements(std::move(elements)), Expr(span, ExprKind::Tuple) {}

        named_list_ptr elements;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_TUPLEEXPR_H
