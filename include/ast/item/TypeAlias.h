#ifndef JACY_AST_ITEM_TYPEALIAS_H
#define JACY_AST_ITEM_TYPEALIAS_H

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct TypeAlias : Item {
        TypeAlias(
            ident_pr name,
            opt_type_ptr type,
            const Span & span
        ) : Item(span, ItemKind::TypeAlias),
            name(std::move(name)),
            type(std::move(type)) {}

        ident_pr name;
        opt_type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_TYPEALIAS_H
