#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct WildcardPat {
        WildcardPat()
    };

    struct LitPat {
        Expr value;
    };

    struct IdentPat {
        IdentPatAnno anno;
        HirId hirId;
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
