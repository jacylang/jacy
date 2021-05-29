#ifndef JACY_AST_ITEM_ITEM_H
#define JACY_AST_ITEM_ITEM_H

#include "ast/fragments/Attribute.h"
#include "ast/ConstVisitor.h"

namespace jc::ast {
    struct Item;
    using item_ptr = std::shared_ptr<Item>;
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
        Item(const Span & span, attr_list attributes, ItemKind kind)
            : kind(kind), attributes(std::move(attributes)), Node(span) {}

        attr_list attributes;
        ItemKind kind;

        virtual void accept(BaseVisitor & visitor) = 0;
        virtual void accept(ConstVisitor & visitor) const = 0;
    };
}

#endif // JACY_AST_ITEM_ITEM_H
