#ifndef JACY_AST_STMT_STRUCTDECL_H
#define JACY_AST_STMT_STRUCTDECL_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct Struct : Stmt {
        Struct(
            id_ptr id,
            opt_type_params typeParams,
            stmt_list members,
            const Span & span
        ) : id(std::move(id)),
            typeParams(std::move(typeParams)),
            members(std::move(members)),
            Stmt(span, StmtType::Struct) {}

        id_ptr id;
        opt_type_params typeParams;
        stmt_list members;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_STRUCTDECL_H
