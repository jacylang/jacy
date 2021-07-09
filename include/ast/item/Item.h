#ifndef JACY_AST_ITEM_ITEM_H
#define JACY_AST_ITEM_ITEM_H

#include "ast/fragments/Attribute.h"

namespace jc::ast {
    struct Item;
    using item_ptr = PR<N<Item>>;
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

    enum class VisKind {
        Pub,
    };

    struct Vis {
        Vis(VisKind kind, const Span & span) : kind(kind), span(span) {}

        VisKind kind;
        Span span;
    };

    struct Item : Node {
        Item(const Span & span, ItemKind kind) : Node(span), kind(kind) {}

        attr_list attributes;
        ItemKind kind;

        void setAttributes(attr_list && attributes) {
            this->attributes = std::move(attributes);
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_ITEM_ITEM_H
