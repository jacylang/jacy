#ifndef JACY_TRYCATCHEXPR_H
#define JACY_TRYCATCHEXPR_H

#include "tree/fragments/Block.h"
#include "tree/expr/Expr.h"

namespace jc::tree {
    struct TryCatchExpr : Expr {
        TryCatchExpr(const Location & loc) : Expr(loc, ExprType::TryCatch) {}

        block_ptr tryBlock;
        block_list catchBlocks;
        block_ptr finallyBlock;
    };
}

#endif // JACY_TRYCATCHEXPR_H
