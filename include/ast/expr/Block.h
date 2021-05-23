#ifndef JACY_BLOCK_H
#define JACY_BLOCK_H

#include "ast/expr/Expr.h"
#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Block;
    using block_ptr = std::shared_ptr<Block>;
    using opt_block_ptr = dt::Option<block_ptr>;
    using block_list = std::vector<block_ptr>;

    struct Block : Expr {
        Block(stmt_list stmts, const Span & span)
            : stmts(std::move(stmts)), Expr(span, ExprKind::Block) {}

        stmt_list stmts;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_BLOCK_H
