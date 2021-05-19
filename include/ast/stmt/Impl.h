#ifndef JACY_AST_STMT_IMPL_H
#define JACY_AST_STMT_IMPL_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Impl : Stmt {
        Impl(
            opt_type_params typeParams,
            type_path_ptr traitTypePath,
            type_ptr forType,
            stmt_list members,
            const Location & loc
        ) : typeParams(std::move(typeParams)),
            traitTypePath(std::move(traitTypePath)),
            forType(std::move(forType)),
            members(std::move(members)),
            Stmt(loc, StmtType::Impl) {}

        opt_type_params typeParams;
        type_path_ptr traitTypePath;
        type_ptr forType;
        stmt_list members;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_IMPL_H
