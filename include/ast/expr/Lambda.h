#ifndef JACY_AST_EXPR_LAMBDA_H
#define JACY_AST_EXPR_LAMBDA_H

#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Lambda : Expr {
        Lambda(
            lambda_param_list params,
            opt_type_ptr returnType,
            expr_ptr body,
            const Span & span
        ) : Expr(span, ExprKind::Lambda),
            params(std::move(params)),
            returnType(std::move(returnType)),
            body(std::move(body)) {}

        lambda_param_list params;
        opt_type_ptr returnType;
        expr_ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LAMBDA_H
