#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct PatKind {
        using Ptr = std::unique_ptr<PatKind>;

        enum class Kind {
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

        PatKind(Kind kind) : kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & pat) {
            return static_cast<T*>(pat.get());
        }
    };

    struct Pat  {
        using Opt = Option<Pat>;
        using List = std::vector<Pat>;

        Pat(PatKind && kind, HirId hirId, Span span) : hirId {hirId}, span {span}, kind {std::move(kind)} {}

        HirId hirId;
        Span span;
        PatKind kind;
    };
}

#endif // JACY_HIR_NODES_PAT_H
