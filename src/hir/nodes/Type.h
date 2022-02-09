#ifndef JACY_HIR_NODES_TYPE_H
#define JACY_HIR_NODES_TYPE_H

#include "hir/nodes/HirNode.h"

namespace jc::hir {
    struct Type : HirNode {
        using Ptr = N<Type>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Type::Ptr>;

        enum class Kind {
            Infer,
            Tuple,
            Func,
            Slice,
            Array,
            Path,
            Unit,
        };

        Type(Kind kind, HirId hirId, Span span) : HirNode {hirId, span}, kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const Ptr & item) {
            return static_cast<T*>(item.get());
        }
    };
}

#endif // JACY_HIR_NODES_TYPE_H
