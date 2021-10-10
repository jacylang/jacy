#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct WildcardPat : Pat {
        WildcardPat(HirId hirId, Span span) : Pat {PatKind::Wildcard, hirId, span} {
        }
    };

    struct LitPat {
        LitPat(Expr::Ptr && value, HirId hirId, Span span)
            : Pat {PatKind::Lit, hirId, span}, value {std::move(value)} {
        }

        Expr value;
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

    struct RefPat {
        RefPat(Mutability mut, Pat::Ptr && pat, HirId hirId, Span span)
            : Pat {PatKind::Ref, hirId, span}, mut {mut}, pat {std::move(pat)} {
        }

        Mutability mut;
        Pat::Ptr pat;
    };

    struct PathPat {
        RefPat(Path && path, HirId hirId, Span span)
            : Pat {PatKind::Path, hirId, span}, path {std::move(path)} {}

        Path path;
    };

    struct StructPatField : HirNode {
        using List = std::vector<StructPatField>;

        StructPatField(span::Ident && ident, Pat::Ptr && pat, HirId hirId, Span span)
            : HirNode {hirId, span}, ident {std::move(ident)}, pat {std::move(pat)} {
        }

        span::Ident ident;
        Pat::Ptr pat;
    };

    struct StructPat : Pat {
        StructPat(Path && path, StructPatField::List && fields, HirId hirId, Span span)
            : Pat {PatKind::Struct, hirId, span},
              path {std::move(path)},
              fields {std::move(fields)} {}

        Path path;
        StructPatField::List fields;
    };
}

#endif // JACY_HIR_NODES_PATTERNS_H
