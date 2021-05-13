#ifndef JACY_INVOKE_H
#define JACY_INVOKE_H

#include "ast/fragments/ArgList.h"

namespace jc::ast {
    struct Invoke : Expr {
        Invoke(expr_ptr lhs, arg_list_ptr args)
            : lhs(lhs), args(args), Expr(lhs->loc, ExprType::Invoke) {}

        expr_ptr lhs;
        arg_list_ptr args;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_INVOKE_H
