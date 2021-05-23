#ifndef JACY_TYPEALIAS_H
#define JACY_TYPEALIAS_H

#include "ast/item/Item.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct TypeAlias : Item {
        TypeAlias(
            attr_list attributes,
            id_ptr name,
            type_ptr type,
            const Span & span
        ) : name(std::move(name)),
            type(std::move(type)),
            Item(span, std::move(attributes), ItemKind::TypeAlias) {}

        id_ptr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(*this);
        }
        void accept(ConstVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_TYPEALIAS_H
