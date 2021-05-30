#ifndef JACY_AST_STMT_VARSTMT_H
#define JACY_AST_STMT_VARSTMT_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct VarStmt : Stmt {
        VarStmt(
            const parser::Token & kind,
            id_ptr name,
            type_ptr type,
            opt_expr_ptr assignExpr,
            const Span & span
        ) : kind(kind),
            name(std::move(name)),
            type(std::move(type)),
            assignExpr(std::move(assignExpr)),
            Stmt(span, StmtKind::Var) {}

        parser::Token kind;
        id_ptr name;
        type_ptr type;
        opt_expr_ptr assignExpr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_VARSTMT_H
