#ifndef JACY_AST_ITEM_ENUM_H
#define JACY_AST_ITEM_ENUM_H

#include <vector>
#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/item/Struct.h"

namespace jc::ast {
    enum class EnumEntryKind {
        Raw, // `A`
        Discriminant, // `A = const expr`
        Tuple, // `A(a, b, c...)`
        Struct, // `A {a, b, c...}`
    };

    struct EnumEntry : Node {
        using List = std::vector<EnumEntry>;

        EnumEntry(EnumEntryKind kind, Ident::PR && name, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::monostate{}) {}

        EnumEntry(EnumEntryKind kind, Ident::PR && name, Expr::Ptr && discriminant, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::move(discriminant)) {}

        EnumEntry(EnumEntryKind kind, Ident::PR && name, TupleTypeEl::List && tupleFields, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::move(tupleFields)) {}

        EnumEntry(EnumEntryKind kind, Ident::PR && name, struct_field_list && fields, const Span & span)
            : Node(span), kind(kind), name(std::move(name)), body(std::move(fields)) {}

        EnumEntryKind kind;
        Ident::PR name;
        std::variant<std::monostate, Expr::Ptr, TupleTypeEl::List, struct_field_list> body;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Enum : Item {
        Enum(Ident::PR && name, EnumEntry::List && entries, const Span & span)
            : Item(span, ItemKind::Enum), name(std::move(name)), entries(std::move(entries)) {}

        Ident::PR name;
        EnumEntry::List entries{};

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

#endif // JACY_AST_ITEM_ENUM_H
