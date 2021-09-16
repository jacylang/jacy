#ifndef JACY_AST_FRAGMENTS_SIMPLEPATH_H
#define JACY_AST_FRAGMENTS_SIMPLEPATH_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "data_types/Option.h"
#include "ast/fragments/PathInterface.h"

namespace jc::ast {
    struct SimplePathSeg : Node {
        SimplePathSeg(Ident::PR && ident, const Span & span)
            : Node{span}, ident{std::move(ident)} {}

        Ident::PR ident;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SimplePath : Node, PathInterface {
        SimplePath(
            bool global,
            std::vector<SimplePathSeg> && segments,
            const Span & span
        ) : Node{span},
            global{global},
            segments{std::move(segments)} {}

        bool global;
        std::vector<SimplePathSeg> segments;

        bool isGlobal() const override {
            return global;
        }

        size_t size() const override {
            return segments.size();
        }

        Ident getSegIdent(size_t index) const override {
            return segments.at(index).ident.unwrap();
        }

        Ident lastSegIdent() const override {
            return segments.back().ident.unwrap();
        }

        bool segHasGenerics(size_t) const override {
            return false;
        }

        const GenericParam::List & getSegGenerics(size_t index) const override {
            log::devPanic("Called `ast::SimplePath::getSegGenerics`");
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_SIMPLEPATH_H
