#ifndef JACY_AST_EXPR_MATCHEXPR_H
#define JACY_AST_EXPR_MATCHEXPR_H

#include "ast/expr/Expr.h"
#include "Block.h"
#include "ast/fragments/Pattern.h"

namespace jc::ast {
    struct MatchArm : Node {
        using List = std::vector<MatchArm>;

        MatchArm(
            Pattern::List patterns,
            Block::Ptr body,
            const Span & span
        ) : Node{span},
            patterns(std::move(patterns)),
            body(std::move(body)) {}

        Pattern::List patterns;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct MatchExpr : Expr {
        MatchExpr(
            Expr::Ptr subject,
            MatchArm::List entries,
            const Span & span
        ) : Expr{span, ExprKind::Match},
            subject(std::move(subject)),
            entries(std::move(entries)) {}

        Expr::Ptr subject;
        MatchArm::List entries;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_MATCHEXPR_H
