#ifndef JACY_AST_EXPR_MATCHEXPR_H
#define JACY_AST_EXPR_MATCHEXPR_H

#include "ast/expr/Expr.h"
#include "Block.h"
#include "ast/fragments/Pat.h"

namespace jc::ast {
    struct MatchArm : Node {
        using List = std::vector<MatchArm>;

        MatchArm(
            Pat::Ptr pat,
            Expr::Ptr body,
            const Span & span
        ) : Node {span},
            pat {std::move(pat)},
            body {std::move(body)} {
        }

        Pat::Ptr pat;
        Expr::Ptr body;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct MatchExpr : Expr {
        MatchExpr(
            Expr::Ptr subject,
            MatchArm::List entries,
            const Span & span
        ) : Expr {span, ExprKind::Match},
            subject {std::move(subject)},
            arms {std::move(entries)} {
        }

        Expr::Ptr subject;
        MatchArm::List arms;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_MATCHEXPR_H
