#ifndef JACY_AST_STMT_STRUCTDECL_H
#define JACY_AST_STMT_STRUCTDECL_H

#include "ast/stmt/Stmt.h"

namespace jc::ast {
    struct Struct : Stmt {
        Struct(
            id_ptr id,
            stmt_list members,
            const Location & loc
        ) : id(std::move(id)),
            members(std::move(members)),
            Stmt(loc, StmtType::Struct) {}

        id_ptr id;
        stmt_list members;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_STRUCTDECL_H
