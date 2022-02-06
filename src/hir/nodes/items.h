#ifndef JACY_HIR_NODES_ITEMS_H
#define JACY_HIR_NODES_ITEMS_H

#include "hir/nodes/Item.h"
#include "hir/nodes/Expr.h"
#include "span/Ident.h"
#include "hir/nodes/Type.h"
#include "ast/fragments/func_fragments.h"
#include "hir/nodes/fragments.h"

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
        using Data = std::variant<CommonField::List, Option < AnonConst>>;

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
    };

    struct Enum : Item {
        Enum(std::vector<Variant> && variants) : Item {ItemKind::Enum}, variants {std::move(variants)} {}

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
        Func(FuncSig && sig, GenericParam::List && generics, Body && body)
            : Item {ItemKind::Func},
              sig {std::move(sig)},
              generics {std::move(generics)},
              body {std::move(body)} {}

        FuncSig sig;
        GenericParam::List generics;
        Body body;
    };

    struct Impl : Item {};

    struct Mod : Item {
        Mod(ItemId::List && items) : Item {ItemKind::Mod}, items {std::move(items)} {}

        ItemId::List items;
    };

    struct Struct : Item {};

    struct Trait : Item {};

    struct TypeAlias : Item {};

    struct UseDecl : Item {};
}

#endif // JACY_HIR_NODES_ITEMS_H
