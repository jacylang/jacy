#ifndef JACY_DOWHILESTMT_H
#define JACY_DOWHILESTMT_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Expr.h"
#include "tree/fragments/Block.h"

namespace jc::tree {
    struct DoWhileStmt : Stmt {
        DoWhileStmt(const Location & loc) : Stmt(loc, StmtType::DoWhile) {}

        block_ptr body;
        expr_ptr condition;
    };
}

#endif // JACY_DOWHILESTMT_H
