#ifndef JACY_HIR_NODES_PAT_H
#define JACY_HIR_NODES_PAT_H

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

        Pat(PatKind::Ptr && kind, Span span) : span {span}, kind {std::move(kind)} {}

        Span span;
        PatKind::Ptr kind;
    };
}

#endif // JACY_HIR_NODES_PAT_H
