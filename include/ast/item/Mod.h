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
    };
}


#endif // JACY_AST_ITEM_MOD_H
