#ifndef JACY_AST_STMT_FOREXPR_H
#define JACY_AST_STMT_FOREXPR_H

#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"
#include "ast/fragments/Pat.h"

namespace jc::ast {
    /// For-Loop
    struct ForExpr : Expr {
        ForExpr(
            Pat::Ptr && pat,
            Expr::Ptr && inExpr,
            Block::Ptr && body,
            Span span
        ) : Expr {span, Expr::Kind::For},
            pat {std::move(pat)},
            inExpr {std::move(inExpr)},
            body {std::move(body)} {}

        Pat::Ptr pat;
        Expr::Ptr inExpr;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_FOREXPR_H
