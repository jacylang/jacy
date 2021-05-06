#ifndef JACY_IFEXPR_H
#define JACY_IFEXPR_H

#include "tree/expr/Expr.h"
#include "tree/fragments/Block.h"

namespace jc::tree {
    struct IfExpr : Expr {
        IfExpr(expr_ptr condition, block_ptr ifBranch, block_ptr elseBranch, const Location & loc)
            : condition(condition), ifBranch(ifBranch), elseBranch(elseBranch), Expr(loc, ExprType::If) {}

        expr_ptr condition;
        block_ptr ifBranch;
        block_ptr elseBranch;
    };
}

#endif // JACY_IFEXPR_H
