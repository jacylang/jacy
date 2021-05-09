#ifndef JACY_TRYCATCHEXPR_H
#define JACY_TRYCATCHEXPR_H

#include "ast/fragments/Block.h"
#include "ast/expr/Expr.h"

namespace jc::ast {
    struct TryCatchExpr : Expr {
        TryCatchExpr(const Location & loc) : Expr(loc, ExprType::TryCatch) {}

        block_ptr tryBlock;
        block_list catchBlocks;
        block_ptr finallyBlock;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TRYCATCHEXPR_H
