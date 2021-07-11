#ifndef JACY_AST_FRAGMENTS_SIMPLEPATH_H
#define JACY_AST_FRAGMENTS_SIMPLEPATH_H

#include "ast/Node.h"
#include "ast/fragments/Identifier.h"
#include "data_types/Option.h"

namespace jc::ast {
    struct SimplePathSeg;
    struct SimplePath;
    using simple_path_seg_ptr = N<SimplePathSeg>;
    using simple_path_ptr = N<SimplePath>;

    struct SimplePathSeg : Node {
        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,
        } kind;

        SimplePathSeg(id_ptr && ident, const Span & span) : Node(span), kind(Kind::Ident), ident(ident) {}
        SimplePathSeg(Kind kind, const Span & span) : Node(span), kind(kind) {}

        opt_id_ptr ident{None};

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SimplePath : Node {
        SimplePath(
            bool global,
            std::vector<simple_path_seg_ptr> && segments,
            const Span & span
        ) : Node(span),
            global(global),
            segments(std::move(segments)) {}

        bool global;
        std::vector<simple_path_seg_ptr> segments;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_SIMPLEPATH_H
