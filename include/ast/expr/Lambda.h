#ifndef JACY_AST_EXPR_LAMBDA_H
#define JACY_AST_EXPR_LAMBDA_H

#include "ast/fragments/Pattern.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct LambdaParam;
    using lambda_param_list = std::vector<LambdaParam>;

    struct LambdaParam : Node {
        LambdaParam(pat_ptr pat, opt_type_ptr type, const Span & span)
            : Node(span),
              pat(std::move(pat)),
              type(std::move(type)) {}

        pat_ptr pat;
        opt_type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

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
