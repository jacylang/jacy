#ifndef JACY_IFEXPR_H
#define JACY_IFEXPR_H

#include "ast/expr/Expr.h"

#include <utility>
#include "Block.h"

namespace jc::ast {
    struct IfExpr : Expr {
        IfExpr(
            expr_ptr condition,
            opt_block_ptr ifBranch,
            opt_block_ptr elseBranch,
            const Location & loc
        ) : condition(std::move(condition)),
            ifBranch(std::move(ifBranch)),
            elseBranch(std::move(elseBranch)),
            Expr(loc, ExprType::If) {}

        expr_ptr condition;
        opt_block_ptr ifBranch;
        opt_block_ptr elseBranch;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_IFEXPR_H
