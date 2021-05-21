#ifndef JACY_LAMBDAEXPR_H
#define JACY_LAMBDAEXPR_H

#include "ast/fragments/NamedList.h"

namespace jc::ast {
    struct LambdaExpr : Expr {
        LambdaExpr(expr_ptr lhs, named_list_ptr args)
            : lhs(lhs), args(args), Expr(lhs->loc, ExprType::Invoke) {}

        expr_ptr lhs;
        named_list_ptr args;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_LAMBDAEXPR_H
