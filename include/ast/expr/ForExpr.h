#ifndef JACY_AST_STMT_FOREXPR_H
#define JACY_AST_STMT_FOREXPR_H

#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"
#include "ast/fragments/Pattern.h"

namespace jc::ast {
    struct ForExpr : Expr {
        ForExpr(
            Pattern::Ptr && pat,
            Expr::Ptr && inExpr,
            Block::Ptr && body,
            const Span & span
        ) : Expr{span, ExprKind::For},
            pat{std::move(pat)},
            inExpr{std::move(inExpr)},
            body{std::move(body)} {}

        Pattern::Ptr pat;
        Expr::Ptr inExpr;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_FOREXPR_H
