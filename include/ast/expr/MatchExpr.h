#ifndef JACY_AST_EXPR_MATCHEXPR_H
#define JACY_AST_EXPR_MATCHEXPR_H

#include "ast/expr/Expr.h"
#include "Block.h"
#include "ast/fragments/Pattern.h"

namespace jc::ast {
    struct MatchArm;
    using match_arm_ptr = P<MatchArm>;
    using match_arm_list = std::vector<match_arm_ptr>;

    struct MatchArm : Node {
        MatchArm(
            pat_list patterns,
            block_ptr body,
            const Span & span
        ) : Node(span),
            patterns(std::move(patterns)),
            body(std::move(body)) {}

        pat_list patterns;
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
