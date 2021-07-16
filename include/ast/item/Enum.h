#ifndef JACY_AST_ITEM_ENUM_H
#define JACY_AST_ITEM_ENUM_H

#include <vector>
#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/item/Struct.h"

namespace jc::ast {
    struct EnumEntry;
    using enum_entry_ptr = N<EnumEntry>;
    using enum_entry_list = std::vector<enum_entry_ptr>;

    enum class EnumEntryKind {
        Raw, // `A`
        Discriminant, // `A = const expr`
        Tuple, // `A(a, b, c...)`
        Struct, // `A {a, b, c...}`
    };

    struct EnumEntry : Node {
        EnumEntry(EnumEntryKind kind, ident_pr name, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::monostate{}) {}

        EnumEntry(EnumEntryKind kind, ident_pr name, expr_ptr discriminant, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::move(discriminant)) {}

        EnumEntry(EnumEntryKind kind, ident_pr name, tuple_field_list tupleFields, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::move(tupleFields)) {}

        EnumEntry(EnumEntryKind kind, ident_pr name, struct_field_list fields, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::move(fields)) {}

        EnumEntryKind kind;
        ident_pr name;
        std::variant<std::monostate, expr_ptr, tuple_field_list, struct_field_list> body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Enum : Item {
        Enum(ident_pr && name, enum_entry_list && entries, const Span & span)
            : Item(span, ItemKind::Enum), name(std::move(name)), entries(std::move(entries)) {}

        ident_pr name;
        enum_entry_list entries{};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_ENUM_H
