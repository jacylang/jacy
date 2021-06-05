#ifndef JACY_AST_ITEM_STRUCT_H
#define JACY_AST_ITEM_STRUCT_H

#include "ast/item/Item.h"
#include "ast/fragments/TypeParams.h"
#include "ast/fragments/Field.h"

namespace jc::ast {
    struct Struct : Item {
        Struct(
            id_ptr name,
            opt_type_params typeParams,
            field_list_ptr fields,
            const Span & span
        ) : name(std::move(name)),
            typeParams(std::move(typeParams)),
            fields(std::move(fields)),
            Item(span, ItemKind::Struct) {}

        id_ptr name;
        opt_type_params typeParams;
        field_list_ptr fields;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_STRUCT_H
