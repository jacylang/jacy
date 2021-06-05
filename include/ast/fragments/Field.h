#ifndef JACY_AST_FRAGMENTS_FIELD_H
#define JACY_AST_FRAGMENTS_FIELD_H

#include "ast/Node.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct Field;
    using field_ptr = std::shared_ptr<Field>;
    using field_list = std::vector<field_ptr>;

    struct Field : Node {
        Field(
            id_ptr name,
            type_ptr type,
            const Span & span
        ) : name(std::move(name)),
            type(std::move(type)),
            Node(span) {}

        id_ptr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_FIELD_H
