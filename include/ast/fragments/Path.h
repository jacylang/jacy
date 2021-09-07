#ifndef JACY_AST_FRAGMENTS_PATH_H
#define JACY_AST_FRAGMENTS_PATH_H

#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct PathSeg : Node {
        using PR = PR<PathSeg>;
        using List = std::vector<PR>;

        PathSeg(
            Ident::PR ident,
            GenericParam::OptList generics,
            const Span & span
        ) : Node{span},
            ident{std::move(ident)},
            generics{std::move(generics)} {}

        Ident::PR ident;
        GenericParam::OptList generics{None};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Path : Node {
        Path(bool global, PathSeg::List && segments, const Span & span)
            : Node{span}, global{global}, segments{std::move(segments)} {}

        bool global;
        PathSeg::List segments;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_PATH_H
