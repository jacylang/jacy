#ifndef JACY_AST_ITEM_TYPEALIAS_H
#define JACY_AST_ITEM_TYPEALIAS_H

#include "ast/item/Item.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct TypeAlias : Item {
        TypeAlias(
            id_ptr name,
            type_ptr type,
            const Span & span
        ) : name(std::move(name)),
            type(std::move(type)),
            Item(span, ItemKind::TypeAlias) {}

        id_ptr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_TYPEALIAS_H
