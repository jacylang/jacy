#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    using ast::Mutability;
    using ast::IdentPatAnno;

    struct MultiPat : Pat {
        MultiPat(Pat::List && pats, HirId hirId, Span span)
            : Pat {PatKind::Multi, hirId, span}, pats {std::move(pats)} {}

        Pat::List pats;
    };

    struct WildcardPat : Pat {
        WildcardPat(HirId hirId, Span span) : Pat {PatKind::Wildcard, hirId, span} {
        }
    };

    struct LitPat : Pat {
        LitPat(Expr::Ptr && value, HirId hirId, Span span)
            : Pat {PatKind::Lit, hirId, span}, value {std::move(value)} {
        }

        Expr::Ptr value;
    };

    struct IdentPat : Pat {
        IdentPat(IdentPatAnno anno, HirId nameHirId, span::Ident ident, Pat::OptPtr && pat, HirId hirId, Span span)
            : Pat {PatKind::Ident, hirId, span},
              anno {anno},
              nameHirId {nameHirId},
              ident {ident},
              pat {std::move(pat)} {
        }

        IdentPatAnno anno;
        HirId nameHirId;
        span::Ident ident;
        Pat::OptPtr pat;
    };

    struct RefPat : Pat {
        RefPat(Mutability mut, Pat::Ptr && pat, HirId hirId, Span span)
            : Pat {PatKind::Ref, hirId, span}, mut {mut}, pat {std::move(pat)} {
        }

        Mutability mut;
        Pat::Ptr pat;
    };

    struct PathPat : Pat {
        PathPat(Path && path, HirId hirId, Span span)
            : Pat {PatKind::Path, hirId, span}, path {std::move(path)} {
        }

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
            : Pat {PatKind::Struct, hirId, span},
              path {std::move(path)},
              fields {std::move(fields)},
              rest {rest} {
        }

        Path path;
        StructPatField::List fields;
        parser::Token::Opt rest;
    };

    struct TuplePat : Pat {
        using RestPatIndexT = ast::TuplePat::RestPatIndexT;

        TuplePat(Pat::List && els, RestPatIndexT restPatIndex, HirId hirId, Span span)
            : Pat {PatKind::Tuple, hirId, span}, els {std::move(els)}, restPatIndex {restPatIndex} {}

        Pat::List els;
        RestPatIndexT restPatIndex;
    };
}

#endif // JACY_HIR_NODES_PATTERNS_H
