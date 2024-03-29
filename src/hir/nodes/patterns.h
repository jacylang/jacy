#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"
#include "ast/fragments/Pat.h"

namespace jc::hir {
    using ast::Mutability;
    using ast::IdentPatAnno;

    struct MultiPat : PatKind {
        MultiPat(Pat::List && pats) : PatKind {PatKind::Kind::Multi}, pats {std::move(pats)} {}

        Pat::List pats;
    };

    struct WildcardPat : PatKind {
        WildcardPat() : PatKind {PatKind::Kind::Wildcard} {}
    };

    struct LitPat : PatKind {
        LitPat(Expr && value) : PatKind {PatKind::Kind::Lit}, value {std::move(value)} {}

        Expr value;
    };

    /// `ref mut IDENT @ pattern`
    struct IdentPat : PatKind {
        IdentPat(NodeId resNodeId, IdentPatAnno anno, span::Ident ident, Pat::Opt && pat)
            : PatKind {PatKind::Kind::Ident},
              resNodeId {resNodeId},
              anno {anno},
              ident {ident},
              pat {std::move(pat)} {}

        NodeId resNodeId;
        IdentPatAnno anno;
        span::Ident ident;
        Pat::Opt pat;
    };

    /// `&mut pattern`
    struct RefPat : PatKind {
        RefPat(Mutability mut, Pat && pat)
            : PatKind {PatKind::Kind::Ref}, mut {mut}, pat {std::move(pat)} {}

        Mutability mut;
        Pat pat;
    };

    struct PathPat : PatKind {
        PathPat(Path && path)
            : PatKind {PatKind::Kind::Path}, path {std::move(path)} {}

        Path path;
    };

    struct StructPatField {
        using List = std::vector<StructPatField>;

        StructPatField(bool shortcut, span::Ident ident, Pat && pat, Span span)
            : span {span},
              shortcut {shortcut},
              ident {std::move(ident)},
              pat {std::move(pat)} {}

        Span span;
        // Note: Read about shortcut in `ast::StructPatField`
        bool shortcut;
        span::Ident ident;
        Pat pat;
    };

    struct StructPat : PatKind {
        StructPat(Path && path, StructPatField::List && fields, const parser::Token::Opt & rest)
            : PatKind {PatKind::Kind::Struct},
              path {std::move(path)},
              fields {std::move(fields)},
              rest {rest} {}

        Path path;
        StructPatField::List fields;
        parser::Token::Opt rest;
    };

    struct TuplePat : PatKind {
        using Element = NamedNode<Pat, Ident::Opt>;
        using RestPatIndexT = ast::TuplePat::RestPatIndexT;

        TuplePat(Element::List && els, RestPatIndexT restPatIndex)
            : PatKind {PatKind::Kind::Tuple}, els {std::move(els)}, restPatIndex {restPatIndex} {}

        Element::List els;
        RestPatIndexT restPatIndex;
    };

    struct SlicePat : PatKind {
        SlicePat(Pat::List && before, Span::Opt restPatSpan, Pat::List && after)
            : PatKind {PatKind::Kind::Slice},
              before {std::move(before)},
              restPatSpan {restPatSpan},
              after {std::move(after)} {}

        Pat::List before;
        Span::Opt restPatSpan;
        Pat::List after;
    };
}

#endif // JACY_HIR_NODES_PATTERNS_H
