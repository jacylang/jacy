#ifndef JACY_AST_ITEM_STRUCT_H
#define JACY_AST_ITEM_STRUCT_H

#include "ast/item/Item.h"
#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct StructField;
    using struct_field_ptr = std::shared_ptr<StructField>;
    using struct_field_list = std::vector<struct_field_ptr>;

    struct StructField : Node {
        StructField(
            id_ptr name,
            type_ptr type,
            const Span & span
        ) : Node(span),
            name(std::move(name)),
            type(std::move(type)) {}

        id_ptr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Struct : Item {
        Struct(
            id_ptr name,
            opt_gen_params typeParams,
            struct_field_list fields,
            const Span & span
        ) : Item(span, ItemKind::Struct),
            name(std::move(name)),
            typeParams(std::move(typeParams)),
            fields(std::move(fields)) {}

        id_ptr name;
        opt_gen_params typeParams;
        struct_field_list fields;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_STRUCT_H
