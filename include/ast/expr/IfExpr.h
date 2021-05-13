#ifndef JACY_IFEXPR_H
#define JACY_IFEXPR_H

#include "ast/expr/Expr.h"

#include <utility>
#include "ast/fragments/Block.h"

namespace jc::ast {
    struct IfExpr : Expr {
        IfExpr(
            dt::Option<expr_ptr> condition,
            dt::Option<block_ptr> ifBranch,
            dt::Option<block_ptr> elseBranch,
            const Location & loc
        ) : condition(std::move(condition)),
            ifBranch(std::move(ifBranch)),
            elseBranch(std::move(elseBranch)),
            Expr(loc, ExprType::If) {}

        dt::Option<expr_ptr> condition;
        dt::Option<block_ptr> ifBranch;
        dt::Option<block_ptr> elseBranch;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_IFEXPR_H
