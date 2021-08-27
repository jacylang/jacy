#ifndef JACY_AST_FRAGMENTS_SIMPLEPATH_H
#define JACY_AST_FRAGMENTS_SIMPLEPATH_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "data_types/Option.h"

namespace jc::ast {
    struct SimplePathSeg : Node {
        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,
        } kind;

        SimplePathSeg(Ident::PR && ident, const Span & span)
            : Node{span}, kind{Kind::Ident}, ident{std::move(ident)} {}
        SimplePathSeg(Kind kind, const Span & span) : Node{span}, kind{kind} {}

        Ident::OptPR ident{None};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SimplePath : Node {
        SimplePath(
            bool global,
            std::vector<SimplePathSeg> && segments,
            const Span & span
        ) : Node{span},
            global(global),
            segments{std::move(segments)} {}

        bool global;
        std::vector<SimplePathSeg> segments;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_SIMPLEPATH_H
