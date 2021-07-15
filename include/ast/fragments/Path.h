#ifndef JACY_AST_FRAGMENTS_PATH_H
#define JACY_AST_FRAGMENTS_PATH_H

#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct PathSeg;
    using path_seg_list = std::vector<PR<PathSeg>>;

    struct PathSeg : Node {
        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,

            Error,
        } kind;

        PathSeg(
            ident_pr ident,
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
            ident(None),
            generics(std::move(generics)) {}

        opt_ident ident{None};
        opt_gen_params generics{None};

        PathSeg(PathSeg&&) = default;

        static inline constexpr Kind getKind(const parser::Token & token) {
            switch (token.kind) {
                case parser::TokenKind::Super: return Kind::Super;
                case parser::TokenKind::Self: return Kind::Self;
                case parser::TokenKind::Party: return Kind::Party;
                case parser::TokenKind::Id: return Kind::Ident;
                default:;
            }
            return Kind::Error;
        }

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
