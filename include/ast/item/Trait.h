#ifndef JACY_AST_ITEM_TRAIT_H
#define JACY_AST_ITEM_TRAIT_H

#include "ast/item/Item.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Trait : Item {
        Trait(
            ident_pr && name,
            opt_gen_params && generics,
            type_path_list && superTraits,
            item_list && members,
            const Span & span
        ) : Item(span, ItemKind::Trait),
            name(std::move(name)),
            generics(std::move(generics)),
            superTraits(std::move(superTraits)),
            members(std::move(members)) {}

        ident_pr name;
        opt_gen_params generics;
        type_path_list superTraits;
        item_list members;

        Ident getName() const override {
            return name.unwrap();
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_TRAIT_H
