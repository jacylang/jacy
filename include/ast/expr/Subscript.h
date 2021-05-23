#ifndef JACY_SUBSCRIPT_H
#define JACY_SUBSCRIPT_H

#include "ast/expr/Expr.h"

namespace jc::ast {
    struct Subscript : Expr {
        Subscript(expr_ptr lhs, expr_list indices, const Span & span)
            : lhs(std::move(lhs)), indices(std::move(indices)), Expr(span, ExprKind::Subscript) {}

        expr_ptr lhs;
        expr_list indices;


        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_SUBSCRIPT_H
