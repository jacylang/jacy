#ifndef JACY_AST_ITEM_STRUCT_H
#define JACY_AST_ITEM_STRUCT_H

#include "ast/item/Item.h"
#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct StructField;
    using struct_field_list = std::vector<StructField>;

    struct StructField : Node {
        StructField(
            ident_pr name,
            type_ptr type,
            const Span & span
        ) : Node(span),
            name(std::move(name)),
            type(std::move(type)) {}

        ident_pr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Struct : Item {
        Struct(
            ident_pr name,
            opt_gen_params generics,
            struct_field_list fields,
            const Span & span
        ) : Item(span, ItemKind::Struct),
            name(std::move(name)),
            generics(std::move(generics)),
            fields(std::move(fields)) {}

        ident_pr name;
        opt_gen_params generics;
        struct_field_list fields;

        span::Ident getName() const override {
            return name.unwrap();
        }

        opt_node_id getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_STRUCT_H
