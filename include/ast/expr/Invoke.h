#ifndef JACY_AST_EXPR_INVOKE_H
#define JACY_AST_EXPR_INVOKE_H

#include "ast/fragments/NamedList.h"

namespace jc::ast {
    struct Invoke : Expr {
        Invoke(expr_ptr lhs, named_list args, const Span & span)
            : lhs(std::move(lhs)), args(std::move(args)), Expr(span, ExprKind::Invoke) {}

        expr_ptr lhs;
        named_list args;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INVOKE_H
