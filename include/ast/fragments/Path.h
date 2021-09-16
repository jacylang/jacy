#ifndef JACY_AST_FRAGMENTS_PATH_H
#define JACY_AST_FRAGMENTS_PATH_H

#include "ast/fragments/Generics.h"
#include "ast/fragments/PathInterface.h"

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
        GenericParam::OptList generics = None;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct Path : Node, PathInterface {
        Path(bool global, PathSeg::List && segments, const Span & span)
            : Node{span}, global{global}, segments{std::move(segments)} {}

        bool global;
        PathSeg::List segments;

        bool isGlobal() const override {
            return global;
        }

        size_t size() const override {
            return segments.size();
        }

        Ident getSegIdent(size_t index) const override {
            return segments.at(index).unwrap().ident.unwrap();
        }

        Ident lastSegIdent() const override {
            return segments.back().unwrap().ident.unwrap();
        }

        GenericParam::OptList getSegGenerics(size_t index) const override {
            return segments.at(index).unwrap().generics;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_PATH_H
