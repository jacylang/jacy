#ifndef JACY_HIR_NODES_ITEMS_H
#define JACY_HIR_NODES_ITEMS_H

#include "hir/nodes/Item.h"
#include "hir/nodes/Expr.h"
#include "span/Ident.h"
#include "hir/nodes/Type.h"
#include "ast/fragments/func_fragments.h"
#include "hir/nodes/fragments.h"
#include "ast/item/items.h"

namespace jc::hir {
    using CommonField = NamedNode<Type, Ident::Opt>;

    struct Variant {
        using Data = std::variant<CommonField::List, AnonConst::Opt>;

        // TODO: Requires unification for AST `Enum` field types
        enum class Kind {
            Struct, // Struct variant (`{field: Type, ...}`)
            Tuple, // Tuple variant (`(Types...)`)
            Unit, // Variant with optional discriminant (such as `Foo = 1` or just `Bar`)
        };

        Variant(Ident ident, Data && data, Kind kind, Span span)
            : span {span},
              ident {ident},
              data {std::move(data)},
              kind {kind} {}

        Span span;
        Ident ident;
        Data data;
        Kind kind;

        const auto & getCommonFields() const {
            return std::get<CommonField::List>(data);
        }

        const auto & getDiscriminant() const {
            return std::get<AnonConst::Opt>(data);
        }
    };

    struct Enum : ItemKind {
        Enum(std::vector<Variant> && variants) : ItemKind {ItemKind::Kind::Enum}, variants {std::move(variants)} {}

        std::vector<Variant> variants;
    };

    /// Function signature used for raw `func`
    /// and `func` signatures without implementations (in traits)
    struct FuncSig {
        using ReturnType = ast::FuncReturnType<Type>;

        FuncSig(Type::List && inputs, ReturnType && returnType)
            : inputs {std::move(inputs)}, returnType {std::move(returnType)} {}

        Type::List inputs;
        ReturnType returnType;
    };

    struct Func : ItemKind {
        Func(FuncSig && sig, GenericParam::List && generics, BodyId body)
            : ItemKind {ItemKind::Kind::Func},
              sig {std::move(sig)},
              generics {std::move(generics)},
              body {body} {}

        FuncSig sig;
        GenericParam::List generics;
        BodyId body;
    };

    struct Impl : ItemKind {};

    struct Mod : ItemKind {
        Mod(ItemId::List && items) : ItemKind {ItemKind::Kind::Mod}, items {std::move(items)} {}

        ItemId::List items;
    };

    struct Struct : ItemKind {
        Struct(CommonField::List && fields, GenericParam::List && generics)
            : ItemKind {ItemKind::Kind::Struct},
              fields {std::move(fields)},
              generics {std::move(generics)} {}

        /// Using `CommonField`s as `struct` field list
        CommonField::List fields;

        GenericParam::List generics;
    };

    struct TraitMember {
        struct Const {
            Type type;
            BodyId::Opt val;
        };

        struct Func {
            using ValueT = std::variant<BodyId, Ident::List>;

            FuncSig sig;

        private:
            ValueT body;

        public:
            auto isImplemented() const {
                return std::holds_alternative<BodyId>(body);
            }

            const auto & asImplemented() const {
                return std::get<BodyId>(body);
            }

            const auto & asNonImplemented() const {
                return std::get<Ident::List>(body);
            }
        };

        struct TypeAlias {
            Type::Opt type;
        };

        using ValueT = std::variant<Const, Func, TypeAlias>;

        enum class Kind {
            Const,
            Func,
            TypeAlias,
        };

        TraitMember(
            Ident name,
            DefId defId,
            GenericParam::List && generics,
            Const && constItem,
            Span span
        ) : kind {Kind::Const},
            name {name},
            defId {defId},
            generics {std::move(generics)},
            value {std::move(constItem)},
            span {span} {}

        TraitMember(
            Ident name,
            DefId defId,
            GenericParam::List && generics,
            Func && func,
            Span span
        ) : kind {Kind::Func},
            name {name},
            defId {defId},
            generics {std::move(generics)},
            value {std::move(func)},
            span {span} {}

        TraitMember(
            Ident name,
            DefId defId,
            GenericParam::List && generics,
            TypeAlias && typeAlias,
            Span span
        ) : kind {Kind::TypeAlias},
            name {name},
            defId {defId},
            generics {std::move(generics)},
            value {std::move(typeAlias)},
            span {span} {}

        Kind kind;
        Ident name;
        DefId defId;
        GenericParam::List generics;
        ValueT value;
        Span span;
    };

    struct Trait : ItemKind {};

    struct TypeAlias : ItemKind {
        TypeAlias(GenericParam::List && generics, Type && type)
            : ItemKind {ItemKind::Kind::TypeAlias},
              generics {std::move(generics)},
              type {std::move(type)} {}

        GenericParam::List generics;
        Type type;
    };

    struct UseDecl : ItemKind {
        using Kind = ast::UseTree::Kind;

        UseDecl(Kind kind, Path && path) : ItemKind {ItemKind::Kind::Use}, kind {kind}, path {std::move(path)} {}

        Kind kind;
        Path path;
    };
}

#endif // JACY_HIR_NODES_ITEMS_H
