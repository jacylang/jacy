#ifndef JACY_AST_STMT_ITEM_H
#define JACY_AST_STMT_ITEM_H

#include "ast/fragments/Attribute.h"

namespace jc::ast {
    struct Item;
    using item_ptr = std::shared_ptr<Item>;
    using item_list = std::vector<item_ptr>;

    enum class ItemKind {
        Enum,
        Func,
        Impl,
        Struct,
        Trait,
        TypeAlias,
    };

    struct Item : Node {
        Item(ItemKind kind, attr_list attributes, const Span & span)
            : attributes(std::move(attributes)), kind(kind), Node(span) {}

        attr_list attributes;
        ItemKind kind;

        virtual void accept(BaseVisitor & visitor) = 0;
    };
}

#endif // JACY_AST_STMT_ITEM_H
