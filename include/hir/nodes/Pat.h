#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct IdentPat;

    enum class PatKind {
        Multi,
        Wildcard,
        Lit,
        Ident,
        Path,
        Ref,
        Struct,
    };

    struct Pat : HirNode {
        using Ptr = std::unique_ptr<Pat>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Pat(PatKind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        PatKind kind;
    };
}

#endif // JACY_HIR_NODES_PAT_H
