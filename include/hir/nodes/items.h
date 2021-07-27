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

    /// Function body
    /// Separated from `Func` as it is type checked apart
    struct Body {
        Body(bool exprBody, expr_ptr && value) : exprBody(exprBody), value(std::move(value)) {}

        /// Denotes that `func`'s body was defined with `=`
        bool exprBody;

        /// Function body value, BlockExpr if `func` was declared with `{}` and any expr if with `=`
        expr_ptr value;
    };

    /// Function signature used for raw `func`
    /// and `func` signatures without implementations (in traits)
    struct FuncSig {
        FuncSig(type_list && inputs, type_ptr && ret) : inputs(std::move(inputs)), ret(std::move(ret)) {}

        type_list inputs;
        type_ptr ret;
    };

    struct Func : Item {
        Func(FuncSig && sig, Body && body) : Item(ItemKind::Func), sig(std::move(sig)), body(std::move(body)) {}

        FuncSig sig;
        Body body;
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
