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
            const Span & span
        ) : Item(span, std::move(attributes), ItemKind::Field) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_ITEM_FIELD_H
