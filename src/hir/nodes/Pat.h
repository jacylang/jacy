#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    enum class PatKind {
        Multi,
        Wildcard,
        Lit,
        Ident,
        Path,
        Ref,
        Struct,
        Tuple,
        Slice,
    };

    struct Pat : HirNode {
        using Ptr = std::unique_ptr<Pat>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Pat(PatKind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        PatKind kind;

        template<class T>
        static T * as(const Ptr & pat) {
            return static_cast<T*>(pat.get());
        }
    };
}

#endif // JACY_HIR_NODES_PAT_H
