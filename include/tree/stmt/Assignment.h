#ifndef JACY_ASSIGNMENT_H
#define JACY_ASSIGNMENT_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Expr.h"

namespace jc::tree {
    struct Assignment : Stmt {
        Assignment(stmt_ptr lhs, const parser::Token & token, expr_ptr rhs)
            : lhs(lhs), token(token), rhs(rhs), Stmt(lhs->loc, StmtType::Assignment) {}

        stmt_ptr lhs;
        parser::Token token;
        expr_ptr rhs;
    };
}

#endif // JACY_ASSIGNMENT_H
