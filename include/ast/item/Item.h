#ifndef JACY_AST_ITEM_ITEM_H
#define JACY_AST_ITEM_ITEM_H

#include "ast/fragments/Attribute.h"

namespace jc::ast {
    struct Item;
    using pure_item_ptr = std::shared_ptr<Item>;
    using item_ptr = PR<pure_item_ptr>;
    using item_list = std::vector<item_ptr>;

    enum class ItemKind {
        Enum,
        Func,
        Impl,
        Mod,
        Struct,
        Trait,
        TypeAlias,
        Use,
    };

    struct Item : Node {
        Item(const Span & span, ItemKind kind)
            : kind(kind), Node(span) {}

        attr_list attributes;
        ItemKind kind;

        void setAttributes(attr_list && attributes) {
            this->attributes = std::move(attributes);
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_ITEM_ITEM_H
