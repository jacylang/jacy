#ifndef JACY_AST_STMT_FORSTMT_H
#define JACY_AST_STMT_FORSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    struct ForStmt : Stmt {
        // TODO: Add destructuring

        ForStmt(
            id_ptr forEntity,
            expr_ptr inExpr,
            block_ptr body,
            const Span & span
        ) : Stmt(span, StmtKind::For),
            forEntity(std::move(forEntity)),
            inExpr(std::move(inExpr)),
            body(std::move(body)) {}

        id_ptr forEntity;
        expr_ptr inExpr;
        block_ptr body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_FORSTMT_H
