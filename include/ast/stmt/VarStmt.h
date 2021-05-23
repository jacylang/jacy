#ifndef JACY_VARDECL_H
#define JACY_VARDECL_H

#include "ast/stmt/Stmt.h"
#include "common/common.h"
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
            Stmt(span, StmtKind::VarDecl) {}

        parser::Token kind;
        id_ptr name;
        type_ptr type;
        opt_expr_ptr assignExpr;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(const ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_VARDECL_H
