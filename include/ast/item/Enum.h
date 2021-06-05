#ifndef JACY_AST_ITEM_ENUM_H
#define JACY_AST_ITEM_ENUM_H

#include <vector>
#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/Identifier.h"
#include "ast/fragments/Field.h"

namespace jc::ast {
    struct EnumEntry;
    using enum_entry_ptr = std::shared_ptr<EnumEntry>;
    using enum_entry_list = std::vector<enum_entry_ptr>;

    enum class EnumEntryKind {
        Raw, // `A`
        Discriminant, // `A = const expr`
        Tuple, // `A(a, b, c...)`
        Struct, // `A {a, b, c...}`
    };

    struct EnumEntry : Node {
        EnumEntry(EnumEntryKind kind, id_ptr name, const Span & span)
            : kind(kind), name(std::move(name)), body(std::monostate{}), Node(span) {}

        EnumEntry(EnumEntryKind kind, id_ptr name, expr_ptr discriminant, const Span & span)
            : kind(kind), name(std::move(name)), body(std::move(discriminant)), Node(span) {}

        EnumEntry(EnumEntryKind kind, id_ptr name, named_list_ptr tupleFields, const Span & span)
            : kind(kind), name(std::move(name)), body(std::move(tupleFields)), Node(span) {}

        EnumEntry(EnumEntryKind kind, id_ptr name, field_list_ptr fields, const Span & span)
            : kind(kind), name(std::move(name)), body(std::move(fields)), Node(span) {}

        EnumEntryKind kind;
        id_ptr name;
        std::variant<std::monostate, expr_ptr, named_list_ptr, field_list_ptr> body;
    };

    struct Enum : Item {
        Enum(enum_entry_list entries, const Span & span)
            : entries(std::move(entries)), Item(span, ItemKind::Enum) {}

        id_ptr name;
        enum_entry_list entries;


        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_ENUM_H
