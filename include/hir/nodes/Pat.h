#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

#include "hir/nodes/HirNode.h"
#include "hir/nodes/Expr.h"

namespace jc::hir {
    struct IdentPat;

    /// `ref` and `mut` combinations for simplicity and expressiveness
    enum class IdentPatAnno {
        None,
        Ref,
        Mut,
        RefMut,
    };

    enum class Mutability {
        Immut,
        Mut,
    };

    struct Pat : HirNode {
        using Opt = Option<Pat>;

        enum class Kind {
            Wildcard,
            Lit,
            Ident,
            Path,
            Ref,
            Struct,
        };

        using ValueT = std::variant<
            std::monostate
        >;
    };
}

#endif // JACY_HIR_NODES_PAT_H
