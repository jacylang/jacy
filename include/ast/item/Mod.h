#ifndef JACY_AST_ITEM_MOD_H
#define JACY_AST_ITEM_MOD_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct Mod : Item {
        Mod(
            Ident::PR && name,
            Item::List && items,
            const Span & span
        ) : Item(span, ItemKind::Mod),
            name(std::move(name)),
            items(std::move(items)) {}

        Ident::PR name;
        Item::List items;

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


#endif // JACY_AST_ITEM_MOD_H
