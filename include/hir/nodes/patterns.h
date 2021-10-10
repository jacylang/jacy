#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct WildcardPat : Pat {
        WildcardPat(HirId hirId, Span span) : Pat {PatKind::Wildcard, hirId, span} {}
    };

    struct LitPat {
        LitPat(Expr::Ptr && value, HirId hirId, Span span)
            : Pat {PatKind::Lit, hirId, span} value {std::move(value)} {}

        Expr value;
    };

    struct IdentPat : Pat {
        IdentPat(IdentPatAnno anno, HirId nameHirId, span::Ident ident, Pat::Opt && pat, HirId hirId, Span span)
            : Pat {PatKind::Ident, hirId, span},
              anno {anno},
              nameHirId {nameHirId},
              ident {ident},
              pat {std::move(pat)} {}

        IdentPatAnno anno;
        HirId nameHirId;
        span::Ident ident;
        Pat::Opt pat;
    };

    struct RefPat {
        Mutability mut;
        Pat pat;
    };

    struct PathPat {
        Path path;
    };

    struct StructPatField : HirNode {
        using List = std::vector<StructPatField>;

        StructPatField(span::Ident && ident, Pat && pat, HirId hirId, Span span)
            : HirNode {hirId, span}, ident {std::move(ident)}, pat {std::move(pat)} {
        }

        span::Ident ident;
        Pat pat;
    };

    struct StructPat {
        StructPat(Path && path, StructPatField::List && fields) : path {std::move(path)}, fields {std::move(fields)} {
        }

        Path path;
        StructPatField::List fields;
    };
}

#endif // JACY_HIR_NODES_PATTERNS_H
