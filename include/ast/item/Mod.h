#ifndef JACY_AST_ITEM_MOD_H
#define JACY_AST_ITEM_MOD_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct Mod : Item {
        Mod(
            id_ptr name,
            item_list items,
            const Span & span
        ) : name(std::move(name)),
            items(std::move(items)),
            Item(span, ItemKind::Mod) {}

        id_ptr name;
        item_list items;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}


#endif // JACY_AST_ITEM_MOD_H
