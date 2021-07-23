#ifndef JACY_HIR_NODES_ITEMS_H
#define JACY_HIR_NODES_ITEMS_H

#include "hir/nodes/Item.h"
#include "hir/nodes/Expr.h"
#include "span/Ident.h"
#include "hir/nodes/Type.h"

namespace jc::hir {
    struct Variant : HirNode {
        enum class Kind {

        };

        span::Ident ident;
    };

    struct Enum : Item {
        Enum(std::vector<Variant> && variants) : Item(ItemKind::Enum), variants(std::move(variants)) {}

        std::vector<Variant> variants;
    };

    /// Function signature used for raw `func`
    /// and `func` signatures without implementations (in traits)
    struct FuncSig {
        FuncSig(type_list && inputs, type_ptr && ret) : inputs(std::move(inputs)), ret(std::move(ret)) {}

        type_list inputs;
        type_ptr ret;
    };

    struct Func : Item {
        Func(FuncSig && sig) : Item(ItemKind::Func), sig(std::move(sig)) {}

        FuncSig sig;
    };

    struct Impl : Item {};

    struct Mod : Item {
        Mod(item_id_list && items) : Item(ItemKind::Mod), items(std::move(items)) {}

        item_id_list items;
    };

    struct Struct : Item {};

    struct Trait : Item {};

    struct TypeAlias : Item {};

    struct UseDecl : Item {};
}

#endif // JACY_HIR_NODES_ITEMS_H
