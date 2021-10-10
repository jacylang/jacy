#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct IdentPat;

    enum class PatKind {
        Wildcard,
        Lit,
        Ident,
        Path,
        Ref,
        Struct,
    };

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
        using Ptr = std::unique_ptr<Pat>;
        using OptPtr = Option<Ptr>;

        Pat(PatKind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        PatKind kind;
    };
}

#endif // JACY_HIR_NODES_PAT_H
