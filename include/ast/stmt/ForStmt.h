#ifndef JACY_FORSTMT_H
#define JACY_FORSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"
#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct ForStmt : Stmt {
        // TODO: Add destructuring

        ForStmt(id_ptr forEntity, expr_ptr inExpr, block_ptr body, const Span & span)
            : forEntity(forEntity), inExpr(inExpr), body(body), Stmt(span, StmtType::For) {}

        id_ptr forEntity;
        expr_ptr inExpr;
        block_ptr body;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_FORSTMT_H
