#ifndef JACY_AST_ITEM_STRUCT_H
#define JACY_AST_ITEM_STRUCT_H

#include "ast/item/Item.h"
#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct StructField;
    using struct_field_list = std::vector<StructField>;

    struct StructField : Node {
        StructField(
            Ident::PR name,
            Type::Ptr type,
            const Span & span
        ) : Node(span),
            name(std::move(name)),
            type(std::move(type)) {}

        Ident::PR name;
        Type::Ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Struct : Item {
        Struct(
            Ident::PR name,
            GenericParam::OptList generics,
            struct_field_list fields,
            const Span & span
        ) : Item(span, ItemKind::Struct),
            name(std::move(name)),
            generics(std::move(generics)),
            fields(std::move(fields)) {}

        Ident::PR name;
        GenericParam::OptList generics;
        struct_field_list fields;

        span::Ident getName() const override {
            return name.unwrap();
        }

        OptNodeId getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_STRUCT_H
