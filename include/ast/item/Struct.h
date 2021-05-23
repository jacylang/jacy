#ifndef JACY_AST_STMT_STRUCTDECL_H
#define JACY_AST_STMT_STRUCTDECL_H

#include "ast/item/Item.h"
#include "ast/fragments/TypeParams.h"
#include "ast/fragments/Field.h"

namespace jc::ast {
    struct Struct : Item {
        Struct(
            attr_list attributes,
            id_ptr name,
            opt_type_params typeParams,
            field_list fields,
            const Span & span
        ) : name(std::move(name)),
            typeParams(std::move(typeParams)),
            fields(std::move(fields)),
            Item(span, std::move(attributes), ItemKind::Struct) {}

        id_ptr name;
        opt_type_params typeParams;
        field_list fields;


        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }

        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_STMT_STRUCTDECL_H
