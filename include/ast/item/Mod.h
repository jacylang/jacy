#ifndef JACY_AST_ITEM_MOD_H
#define JACY_AST_ITEM_MOD_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct Mod : Item {
        Mod(
            attr_list attributes,
            id_ptr name,
            item_list items,
            const Span & span
        ) : name(std::move(name)),
            items(std::move(items)),
            Item(span, std::move(attributes), ItemKind::Mod) {}

        id_ptr name;
        item_list items;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}


#endif // JACY_AST_ITEM_MOD_H
