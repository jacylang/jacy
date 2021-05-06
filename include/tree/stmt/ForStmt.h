#ifndef JACY_FORSTMT_H
#define JACY_FORSTMT_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Expr.h"
#include "tree/fragments/Block.h"
#include "tree/expr/Identifier.h"

namespace jc::tree {
    struct ForStmt : Stmt {
        // TODO: Add destructuring

        ForStmt(const Location & loc) : Stmt(loc, StmtType::For) {}

        id_ptr forEntity;
        expr_ptr inExpr;
        block_ptr body;
    };
}

#endif // JACY_FORSTMT_H
