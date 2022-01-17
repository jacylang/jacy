#ifndef JACY_AST_ITEM_ENUM_H
#define JACY_AST_ITEM_ENUM_H

#include <vector>
#include <variant>

#include "ast/item/Item.h"
#include "ast/fragments/Ident.h"
#include "ast/item/Struct.h"

namespace jc::ast {
    struct Variant : Node {
        using List = std::vector<Variant>;

        enum class Kind {
            Unit, // `A = const expr` (optional discriminant)
            Tuple, // `A(a, b, c...)`
            Struct, // `A {a, b, c...}`
        };

        Variant(Kind kind, Ident::PR && name, Expr::OptPtr && disc, Span span)
            : Node {span}, kind {kind}, name {std::move(name)}, body {std::move(disc)} {
        }

        Variant(Kind kind, Ident::PR && name, TupleTypeEl::List && tupleFields, Span span)
            : Node {span}, kind {kind}, name {std::move(name)}, body {std::move(tupleFields)} {
        }

        Variant(Kind kind, Ident::PR && name, StructField::List && fields, Span span)
            : Node {span}, kind {kind}, name {std::move(name)}, body {std::move(fields)} {
        }

        Kind kind;
        Ident::PR name;
        std::variant<Expr::OptPtr, TupleTypeEl::List, StructField::List> body;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Enum : Item {
        Enum(Ident::PR && name, Variant::List && entries, Span span)
            : Item {span, ItemKind::Enum}, name {std::move(name)}, entries {std::move(entries)} {
        }

        Ident::PR name;
        Variant::List entries {};

        span::Ident getName() const override {
            return name.unwrap();
        }

        NodeId::Opt getNameNodeId() const override {
            return name.unwrap().id;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_ITEM_ENUM_H
