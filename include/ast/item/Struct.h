#ifndef JACY_AST_STMT_STRUCTDECL_H
#define JACY_AST_STMT_STRUCTDECL_H

#include "ast/item/Item.h"
#include "ast/fragments/TypeParams.h"

namespace jc::ast {
    struct Struct : Item {
        Struct(
            attr_list attributes,
            id_ptr name,
            opt_type_params typeParams,
            item_list members,
            const Span & span
        ) : name(std::move(name)),
            typeParams(std::move(typeParams)),
            members(std::move(members)),
            Item(ItemKind::Struct, std::move(attributes), span) {}

        id_ptr name;
        opt_type_params typeParams;
        item_list members;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_AST_STMT_STRUCTDECL_H
