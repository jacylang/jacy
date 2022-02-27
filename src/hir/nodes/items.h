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

    struct Const : ItemKind {
        Const(Type && type, BodyId body) : ItemKind {ItemKind::Kind::Const}, type {std::move(type)}, body {body} {}

        Type type;

        // Note: Constants outside of traits must have bodies
        BodyId body;
    };

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

    struct ImplMember {
        struct Const {
            Type type;
            BodyId val;
        };

        struct Func {
            FuncSig sig;
            GenericParam::List generics;
            BodyId body;
        };

        struct Init {
            FuncSig sig;
            GenericParam::List generics;
            BodyId body;
        };

        struct TypeAlias {
            GenericParam::List generics;
            Type type;
        };

        using ValueT = std::variant<Const, Func, Init, TypeAlias>;

        enum class Kind {
            Const,
            Func,
            Init,
            TypeAlias,
        };

        ImplMember(
            Ident name,
            DefId defId,
            Const && constItem,
            Span span
        ) : kind {Kind::Const},
            name {name},
            defId {defId},
            value {std::move(constItem)},
            span {span} {}

        ImplMember(
            Ident name,
            DefId defId,
            Func && func,
            Span span
        ) : kind {Kind::Func},
            name {name},
            defId {defId},
            value {std::move(func)},
            span {span} {}

        ImplMember(
            Ident name,
            DefId defId,
            Init && init,
            Span span
        ) : kind {Kind::Init},
            name {name},
            defId {defId},
            value {std::move(init)},
            span {span} {}

        ImplMember(
            Ident name,
            DefId defId,
            TypeAlias && typeAlias,
            Span span
        ) : kind {Kind::TypeAlias},
            name {name},
            defId {defId},
            value {std::move(typeAlias)},
            span {span} {}

        const auto & asConst() const {
            return std::get<Const>(value);
        }

        const auto & asFunc() const {
            return std::get<Func>(value);
        }

        const auto & asInit() const {
            return std::get<Init>(value);
        }

        const auto & asTypeAlias() const {
            return std::get<TypeAlias>(value);
        }

        Kind kind;
        Ident name;
        DefId defId;
        ValueT value;
        Span span;
    };

    struct Impl : ItemKind {
        struct TraitRef {
            using Opt = Option<TraitRef>;

            Path path;
            ItemId traitItemId;
        };

        Impl(
            GenericParam::List && generics,
            TraitRef::Opt trait,
            Type forType,
            ImplMemberId::List && members
        ) : ItemKind {ItemKind::Kind::Impl},
            generics {std::move(generics)},
            trait {trait},
            forType {std::move(forType)},
            members {std::move(members)} {}

        GenericParam::List generics;
        TraitRef::Opt trait;
        Type forType;
        ImplMemberId::List members;
    };

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
        using List = std::vector<TraitMember>;

        struct Const {
            Type type;
            BodyId::Opt val;
        };

        struct Func {
            using ValueT = std::variant<BodyId, Ident::List>;

            GenericParam::List generics;
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
            GenericParam::List generics;
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
            Const && constItem,
            Span span
        ) : kind {Kind::Const},
            name {name},
            defId {defId},
            value {std::move(constItem)},
            span {span} {}

        TraitMember(
            Ident name,
            DefId defId,
            Func && func,
            Span span
        ) : kind {Kind::Func},
            name {name},
            defId {defId},
            value {std::move(func)},
            span {span} {}

        TraitMember(
            Ident name,
            DefId defId,
            TypeAlias && typeAlias,
            Span span
        ) : kind {Kind::TypeAlias},
            name {name},
            defId {defId},
            value {std::move(typeAlias)},
            span {span} {}

        const auto & asConst() const {
            return std::get<Const>(value);
        }

        const auto & asFunc() const {
            return std::get<Func>(value);
        }

        const auto & asTypeAlias() const {
            return std::get<TypeAlias>(value);
        }

        Kind kind;
        Ident name;
        DefId defId;
        ValueT value;
        Span span;
    };

    struct Trait : ItemKind {
        Trait(GenericParam::List && generics, TraitMemberId::List && members)
            : ItemKind {ItemKind::Kind::Trait},
              generics {std::move(generics)},
              members {std::move(members)} {}

        GenericParam::List generics;
        TraitMemberId::List members;
    };

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
