#ifndef JACY_AST_EXPR_BLOCK_H
#define JACY_AST_EXPR_BLOCK_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Block;
    using BlockPtr = PR<N<Block>>;
    using opt_block_ptr = Option<BlockPtr>;
    using block_list = std::vector<BlockPtr>;

    struct Block : Expr {
        Block(stmt_list && stmts, const Span & span)
            : Expr(span, ExprKind::Block),
              stmts(std::move(stmts)) {}

        stmt_list stmts;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_EXPR_BLOCK_H
