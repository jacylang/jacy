#ifndef JACY_AST_STMT_WHILESTMT_H
#define JACY_AST_STMT_WHILESTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"

namespace jc::ast {
    struct WhileStmt : Stmt {
        WhileStmt(
            Expr::Ptr && condition,
            Block::Ptr && body,
            const Span & span
        ) : Stmt(span, StmtKind::While),
            condition(std::move(condition)),
            body(std::move(body)) {}

        Expr::Ptr condition;
        Block::Ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_WHILESTMT_H
