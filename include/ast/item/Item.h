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
        Unset,
        Pub,
    };

    struct Vis {
        Vis() : kind(VisKind::Unset), span(None) {}
        Vis(VisKind kind, const span::opt_span & span) : kind(kind), span(span) {}

        VisKind kind;
        span::opt_span span;
    };

    struct Item : Node {
        Item(const Span & span, ItemKind kind) : Node(span), kind(kind) {}

        attr_list attributes;
        ItemKind kind;
        Vis vis;

        void setAttributes(attr_list && attributes) {
            this->attributes = std::move(attributes);
        }

        void setVis(Vis && vis) {
            this->vis = std::move(vis);
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_ITEM_ITEM_H
