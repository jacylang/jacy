#ifndef JACY_AST_FRAGMENTS_PATH_H
#define JACY_AST_FRAGMENTS_PATH_H

#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct PathSeg;
    using path_seg_ptr = PR<N<PathSeg>>;
    using path_seg_list = std::vector<path_seg_ptr>;

    struct PathSeg : Node {
        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,

            Error,
        } kind;

        PathSeg(
            id_ptr ident,
            opt_gen_params generics,
            const Span & span
        ) : Node(span),
            kind(Kind::Ident),
            ident(std::move(ident)),
            generics(std::move(generics)) {}

        PathSeg(
            Kind kind,
            opt_gen_params generics,
            const Span & span
        ) : Node(span),
            kind(kind),
            ident(dt::None),
            generics(std::move(generics)) {}

        opt_id_ptr ident;
        opt_gen_params generics;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Path : Node {
        Path(bool global, path_seg_list && segments, const Span & span)
            : Node(span), global(global), segments(std::move(segments)) {}

        bool global;
        path_seg_list segments;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_PATH_H
