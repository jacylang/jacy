#ifndef JACY_AST_EXPR_LAMBDA_H
#define JACY_AST_EXPR_LAMBDA_H

#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct LambdaParam;
    using lambda_param_list = std::vector<std::shared_ptr<LambdaParam>>;

    struct LambdaParam : Node {
        LambdaParam(id_ptr name, opt_type_ptr type, const Span & span)
            : Node(span),
              name(std::move(name)),
              type(std::move(type)) {}

        id_ptr name;
        opt_type_ptr type;
    };

    struct Lambda : Expr {
        Lambda(
            lambda_param_list params,
            opt_type_ptr returnType,
            expr_ptr body,
            const Span & span
        ) : params(std::move(params)),
            returnType(std::move(returnType)),
            body(std::move(body)),
            Expr(span, ExprKind::Lambda) {}

        lambda_param_list params;
        opt_type_ptr returnType;
        expr_ptr body;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LAMBDA_H
