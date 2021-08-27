#ifndef JACY_AST_ITEM_TRAIT_H
#define JACY_AST_ITEM_TRAIT_H

#include "ast/item/Item.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Trait : Item {
        Trait(
            Ident::PR && name,
            GenericParam::OptList && generics,
            TypePath::List && superTraits,
            Item::List && members,
            const Span & span
        ) : Item{span, ItemKind::Trait},
            name(std::move(name)),
            generics(std::move(generics)),
            superTraits(std::move(superTraits)),
            members(std::move(members)) {}

        Ident::PR name;
        GenericParam::OptList generics;
        TypePath::List superTraits;
        Item::List members;

        span::Ident getName() const override {
            return name.unwrap();
        }

        OptNodeId getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_TRAIT_H
