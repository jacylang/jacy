#ifndef JACY_VARDECL_H
#define JACY_VARDECL_H

#include "ast/stmt/Stmt.h"
#include "common/common.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct VarDecl : Stmt {
        VarDecl(parser::Token kind, id_ptr id, type_ptr type, opt_expr_ptr assignExpr)
            : kind(kind), id(id), type(type), assignExpr(assignExpr), Stmt(kind.loc, StmtType::VarDecl) {}

        parser::Token kind;
        id_ptr id;
        type_ptr type;
        opt_expr_ptr assignExpr;

        bool isAssignable() const override {
            return true;
        }

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_VARDECL_H
