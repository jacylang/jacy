#ifndef JACY_AST_EXPR_INVOKE_H
#define JACY_AST_EXPR_INVOKE_H

#include "ast/fragments/Arg.h"

namespace jc::ast {
    struct Invoke : Expr {
        Invoke(ExprPtr lhs, arg_list args, const Span & span)
            : Expr(span, ExprKind::Invoke), lhs(std::move(lhs)), args(std::move(args)) {}

        ExprPtr lhs;
        arg_list args;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_INVOKE_H
