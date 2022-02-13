#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Pat  {
        using Ptr = std::unique_ptr<Pat>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

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

        Pat(Kind kind, HirId hirId, Span span)  {hirId, span}, kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & pat) {
            return static_cast<T*>(pat.get());
        }
    };
}

#endif // JACY_HIR_NODES_PAT_H
