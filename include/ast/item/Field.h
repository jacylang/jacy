#ifndef JACY_AST_ITEM_FIELD_H
#define JACY_AST_ITEM_FIELD_H

#include "ast/item/Item.h"

namespace jc::ast {
    struct Field;
    using field_ptr = std::shared_ptr<Field>;
    using field_list = std::vector<field_ptr>;

    struct Field : Item {
        Field(
            attr_list attributes,
            id_ptr name,
            type_ptr type,
            const Span & span
        ) : name(std::move(name)),
            type(std::move(type)),
            Item(span, std::move(attributes), ItemKind::Field) {}

        id_ptr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_ITEM_FIELD_H
