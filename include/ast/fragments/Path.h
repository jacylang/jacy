#ifndef JACY_AST_FRAGMENTS_PATH_H
#define JACY_AST_FRAGMENTS_PATH_H

#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct PathSeg : Node {
        using List = std::vector<PR<PathSeg>>;

        const enum class Kind {
            Super,
            Self,
            Party,
            Ident,

            Error,
        } kind;

        PathSeg(
            Ident::PR ident,
            GenericParam::OptList generics,
            const Span & span
        ) : Node{span},
            kind(Kind::Ident),
            ident(std::move(ident)),
            generics(std::move(generics)) {}

        PathSeg(
            Kind kind,
            GenericParam::OptList generics,
            const Span & span
        ) : Node{span},
            kind(kind),
            ident(None),
            generics(std::move(generics)) {}

        Ident::OptPR ident{None};
        GenericParam::OptList generics{None};

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
        Path(bool global, PathSeg::List && segments, const Span & span)
            : Node{span), global(global}, segments(std::move(segments)) {}

        bool global;
        PathSeg::List segments;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_PATH_H
