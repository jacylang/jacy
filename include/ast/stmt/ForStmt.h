#ifndef JACY_AST_STMT_FORSTMT_H
#define JACY_AST_STMT_FORSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"
#include "ast/fragments/Pattern.h"

namespace jc::ast {
    struct ForStmt : Stmt {
        ForStmt(
            Pattern::Ptr && pat,
            Expr::Ptr && inExpr,
            Block::Ptr && body,
            const Span & span
        ) : Stmt(span, StmtKind::For),
            pat(std::move(pat)),
            inExpr{std::move(inExpr)},
            body(std::move(body)) {}

        Pattern::Ptr pat;
        Expr::Ptr inExpr;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_FORSTMT_H
