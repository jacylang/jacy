#ifndef JACY_AST_EXPR_BLOCK_H
#define JACY_AST_EXPR_BLOCK_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Block;
    using BlockPtr = PR<N<Block>>;
    using OptBlockPtr = Option<BlockPtr>;
    using BlockList = std::vector<BlockPtr>;

    struct Block : Expr {
        Block(StmtList && stmts, const Span & span)
            : Expr(span, ExprKind::Block),
              stmts(std::move(stmts)) {}

        StmtList stmts;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BLOCK_H
