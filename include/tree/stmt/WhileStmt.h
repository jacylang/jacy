#ifndef JACY_WHILESTMT_H
#define JACY_WHILESTMT_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Expr.h"
#include "tree/fragments/Block.h"

namespace jc::tree {
    struct WhileStmt : Stmt {
        WhileStmt(const Location & loc) : Stmt(loc, StmtType::While) {}

        expr_ptr condition;
        block_ptr body;
    };
}

#endif // JACY_WHILESTMT_H
