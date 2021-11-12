#ifndef JACY_AST_EXPR_LAMBDA_H
#define JACY_AST_EXPR_LAMBDA_H

#include "ast/fragments/Pat.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct LambdaParam : Node {
        using List = std::vector<LambdaParam>;

        LambdaParam(Pat::Ptr pat, Type::OptPtr type, Span span)
            : Node{span},
              pat{std::move(pat)},
              type{std::move(type)} {}

        Pat::Ptr pat;
        Type::OptPtr type;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Lambda : Expr {
        Lambda(
            LambdaParam::List params,
            Type::OptPtr returnType,
            Expr::Ptr body,
            Span span
        ) : Expr{span, ExprKind::Lambda},
            params{std::move(params)},
            returnType{std::move(returnType)},
            body{std::move(body)} {}

        LambdaParam::List params;
        Type::OptPtr returnType;
        Expr::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_LAMBDA_H
