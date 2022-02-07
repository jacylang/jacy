#ifndef JACY_HIR_NODES_ITEMS_H
#define JACY_HIR_NODES_ITEMS_H

#include "hir/nodes/Item.h"
#include "hir/nodes/Expr.h"
#include "span/Ident.h"
#include "hir/nodes/Type.h"
#include "ast/fragments/func_fragments.h"
#include "hir/nodes/fragments.h"
#include "ast/item/UseDecl.h"

namespace jc::hir {
    struct CommonField : HirNode {
        using List = std::vector<CommonField>;

        CommonField(Ident ident, Type::Ptr && type, HirId hirId, Span span)
            : HirNode {hirId, span},
              ident {ident},
              type {std::move(type)} {}

        Ident ident;
        Type::Ptr type;
    };

    struct Variant : HirNode {
        using Data = std::variant<CommonField::List, AnonConst::Opt>;

        // TODO: Requires unification for AST `Enum` field types
        enum class Kind {
            Struct, // Struct variant (`{field: Type, ...}`)
            Tuple, // Tuple variant (`(Types...)`)
            Unit, // Variant with optional discriminant (such as `Foo = 1` or just `Bar`)
        };

        Variant(Ident ident, Data && data, Kind kind, HirId hirId, Span span)
            : HirNode {hirId, span},
              ident {ident},
              data {std::move(data)},
              kind {kind} {}

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

    struct Enum : Item {
        Enum(std::vector<Variant> && variants) : Item {Item::Kind::Enum}, variants {std::move(variants)} {}

        std::vector<Variant> variants;
    };

    /// Function signature used for raw `func`
    /// and `func` signatures without implementations (in traits)
    struct FuncSig {
        using ReturnType = ast::FuncReturnType<Type::Ptr>;

        FuncSig(Type::List && inputs, ReturnType && returnType)
            : inputs {std::move(inputs)}, returnType {std::move(returnType)} {}

        Type::List inputs;
        ReturnType returnType;
    };

    struct Func : Item {
        Func(FuncSig && sig, GenericParam::List && generics, BodyId body)
            : Item {Item::Kind::Func},
              sig {std::move(sig)},
              generics {std::move(generics)},
              body {body} {}

        FuncSig sig;
        GenericParam::List generics;
        BodyId body;
    };

    struct Impl : Item {};

    struct Mod : Item {
        Mod(ItemId::List && items) : Item {Item::Kind::Mod}, items {std::move(items)} {}

        ItemId::List items;
    };

    struct Struct : Item {
        Struct(CommonField::List && fields, GenericParam::List && generics)
            : Item {Item::Kind::Struct},
              fields {std::move(fields)},
              generics {std::move(generics)} {}

        /// Using `CommonField`s as `struct` field list
        CommonField::List fields;

        GenericParam::List generics;
    };

    struct Trait : Item {};

    struct TypeAlias : Item {
        TypeAlias(GenericParam::List && generics, Type::Ptr && type)
            : Item {Item::Kind::TypeAlias},
              generics {std::move(generics)},
              type {std::move(type)} {}

        GenericParam::List generics;
        Type::Ptr type;
    };

    struct UseDecl : Item {
        using Kind = ast::UseTree::Kind;

        UseDecl(Kind kind, Path && path) : Item {Item::Kind::Use}, kind {kind}, path {std::move(path)} {}

        Kind kind;
        Path path;
    };
}

#endif // JACY_HIR_NODES_ITEMS_H
