#ifndef JACY_AST_EXPR_LAMBDA_H
#define JACY_AST_EXPR_LAMBDA_H

#include "ast/fragments/Pattern.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct LambdaParam;
    using lambda_param_list = std::vector<LambdaParam>;

    struct LambdaParam : Node {
        LambdaParam(Pattern::Ptr pat, Type::OptPtr type, const Span & span)
            : Node(span),
              pat(std::move(pat)),
              type(std::move(type)) {}

        Pattern::Ptr pat;
        Type::OptPtr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Lambda : Expr {
        Lambda(
            lambda_param_list params,
            Type::OptPtr returnType,
            Expr::Ptr body,
            const Span & span
        ) : Expr(span, ExprKind::Lambda),
            params(std::move(params)),
            returnType(std::move(returnType)),
            body(std::move(body)) {}

        lambda_param_list params;
        Type::OptPtr returnType;
        Expr::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LAMBDA_H
