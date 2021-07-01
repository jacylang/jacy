#ifndef JACY_AST_EXPR_MATCHEXPR_H
#define JACY_AST_EXPR_MATCHEXPR_H

#include "ast/expr/Expr.h"
#include "Block.h"

namespace jc::ast {
    struct MatchArm;
    using match_arm_ptr = std::shared_ptr<MatchArm>;
    using match_arm_list = std::vector<match_arm_ptr>;

    struct MatchArm : Node {
        MatchArm(
            expr_list conditions,
            block_ptr body,
            const Span & span
        ) : Node(span),
            conditions(std::move(conditions)),
            body(std::move(body)) {}

        expr_list conditions;
        block_ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct MatchExpr : Expr {
        MatchExpr(
            expr_ptr subject,
            match_arm_list entries,
            const Span & span
        ) : Expr(span, ExprKind::When),
            subject(std::move(subject)),
            entries(std::move(entries)) {}

        expr_ptr subject;
        match_arm_list entries;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_MATCHEXPR_H
