#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"
#include "ast/fragments/Pat.h"

namespace jc::hir {
    using ast::Mutability;
    using ast::IdentPatAnno;

    struct MultiPat : Pat {
        MultiPat(Pat::List && pats, HirId hirId, Span span)
            : Pat {Pat::Kind::Multi, hirId, span}, pats {std::move(pats)} {}

        Pat::List pats;
    };

    struct WildcardPat : Pat {
        WildcardPat(HirId hirId, Span span) : Pat {Pat::Kind::Wildcard, hirId, span} {}
    };

    struct LitPat : Pat {
        LitPat(Expr && value, HirId hirId, Span span)
            : Pat {Pat::Kind::Lit, hirId, span}, value {std::move(value)} {}

        Expr value;
    };

    /// `ref mut IDENT @ pattern`
    struct IdentPat : Pat {
        IdentPat(IdentPatAnno anno, HirId nameHirId, span::Ident ident, Pat::OptPtr && pat, HirId hirId, Span span)
            : Pat {Pat::Kind::Ident, hirId, span},
              anno {anno},
              nameHirId {nameHirId},
              ident {ident},
              pat {std::move(pat)} {}

        IdentPatAnno anno;
        HirId nameHirId;
        span::Ident ident;
        Pat::OptPtr pat;
    };

    /// `&mut pattern`
    struct RefPat : Pat {
        RefPat(Mutability mut, Pat::Ptr && pat, HirId hirId, Span span)
            : Pat {Pat::Kind::Ref, hirId, span}, mut {mut}, pat {std::move(pat)} {}

        Mutability mut;
        Pat::Ptr pat;
    };

    struct PathPat : Pat {
        PathPat(Path && path, HirId hirId, Span span)
            : Pat {Pat::Kind::Path, hirId, span}, path {std::move(path)} {}

        Path path;
    };

    struct StructPatField : HirNode {
        using List = std::vector<StructPatField>;

        StructPatField(bool shortcut, span::Ident ident, Pat::Ptr && pat, HirId hirId, Span span)
            : HirNode {hirId, span}, shortcut {shortcut}, ident {std::move(ident)}, pat {std::move(pat)} {}

        // Note: Read about shortcut in `ast::StructPatField`
        bool shortcut;
        span::Ident ident;
        Pat::Ptr pat;
    };

    struct StructPat : Pat {
        StructPat(Path && path, StructPatField::List && fields, const parser::Token::Opt & rest, HirId hirId, Span span)
            : Pat {Pat::Kind::Struct, hirId, span},
              path {std::move(path)},
              fields {std::move(fields)},
              rest {rest} {}

        Path path;
        StructPatField::List fields;
        parser::Token::Opt rest;
    };

    struct TuplePat : Pat {
        using RestPatIndexT = ast::TuplePat::RestPatIndexT;

        TuplePat(Pat::List && els, RestPatIndexT restPatIndex, HirId hirId, Span span)
            : Pat {Pat::Kind::Tuple, hirId, span}, els {std::move(els)}, restPatIndex {restPatIndex} {}

        Pat::List els;
        RestPatIndexT restPatIndex;
    };

    struct SlicePat : Pat {
        SlicePat(Pat::List && before, Span::Opt restPatSpan, Pat::List && after, HirId hirId, Span span)
            : Pat {Pat::Kind::Slice, hirId, span},
              before {std::move(before)},
              restPatSpan {restPatSpan},
              after {std::move(after)} {}

        Pat::List before;
        Span::Opt restPatSpan;
        Pat::List after;
    };
}

#endif // JACY_HIR_NODES_PATTERNS_H
