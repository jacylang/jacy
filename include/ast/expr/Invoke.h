#ifndef JACY_INVOKE_H
#define JACY_INVOKE_H

#include "ast/fragments/NamedList.h"

namespace jc::ast {
    struct Invoke : Expr {
        Invoke(expr_ptr lhs, named_list_ptr args, const Span & span)
            : lhs(std::move(lhs)), args(std::move(args)), Expr(span, ExprKind::Invoke) {}

        expr_ptr lhs;
        named_list_ptr args;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }
        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INVOKE_H
