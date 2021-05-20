#ifndef JACY_WHILESTMT_H
#define JACY_WHILESTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Expr.h"
#include "ast/expr/Block.h"

namespace jc::ast {
    struct WhileStmt : Stmt {
        WhileStmt(expr_ptr condition, block_ptr body, const Span & span)
            : condition(condition), body(body), Stmt(span, StmtType::While) {}

        expr_ptr condition;
        block_ptr body;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_WHILESTMT_H
