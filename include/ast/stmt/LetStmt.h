#ifndef JACY_AST_STMT_LETSTMT_H
#define JACY_AST_STMT_LETSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"
#include "ast/fragments/Pattern.h"

namespace jc::ast {
    struct LetStmt : Stmt {
        LetStmt(
            id_ptr name,
            opt_type_ptr type,
            opt_expr_ptr assignExpr,
            const Span & span
        ) : Stmt(span, StmtKind::Var),
            name(std::move(name)),
            type(std::move(type)),
            assignExpr(std::move(assignExpr)) {}

        parser::Token kind;
        id_ptr name;
        opt_type_ptr type;
        opt_expr_ptr assignExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_LETSTMT_H
