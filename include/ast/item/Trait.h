#ifndef JACY_AST_STMT_TRAIT_H
#define JACY_AST_STMT_TRAIT_H

#include "ast/item/Item.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Trait : Item {
        Trait(
            id_ptr name,
            opt_type_params typeParams,
            type_path_list superTraits,
            item_list members,
            const Span & span
        ) : name(std::move(name)),
            typeParams(std::move(typeParams)),
            superTraits(std::move(superTraits)),
            members(std::move(members)),
            Item() {}

        id_ptr name;
        opt_type_params typeParams;
        type_path_list superTraits;
        item_list members;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_TRAIT_H
