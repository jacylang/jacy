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
            GenericArg::OptList generics,
            Span span
        ) : Node {span},
            ident {std::move(ident)},
            generics {std::move(generics)} {}

        Ident::PR ident;
        GenericArg::OptList generics = None;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct Path : Node, PathInterface {
        using Opt = Option<Path>;

        Path(bool global, PathSeg::List && segments, Span span)
            : Node {span}, global {global}, segments {std::move(segments)} {}

        bool global;
        PathSeg::List segments;

        NodeId getNodeId() const override {
            return id;
        }

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

        bool segHasGenerics(size_t index) const override {
            return segments.at(index).unwrap().generics.some();
        }

        const GenericArg::List & getSegGenerics(size_t index) const override {
            return segments.at(index).unwrap().generics.unwrap();
        }

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_PATH_H
