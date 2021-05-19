#ifndef JACY_AST_STMT_TRAIT_H
#define JACY_AST_STMT_TRAIT_H

#include "ast/stmt/Stmt.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Trait : Stmt {
        Trait(
            id_ptr id,
            opt_type_params typeParams,
            type_path_list superTraits,
            stmt_list members,
            const Location & loc
        ) : id(std::move(id)),
            typeParams(std::move(typeParams)),
            superTraits(std::move(superTraits)),
            members(std::move(members)),
            Stmt(loc, StmtType::Trait) {}

        id_ptr id;
        opt_type_params typeParams;
        type_path_list superTraits;
        stmt_list members;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_TRAIT_H
