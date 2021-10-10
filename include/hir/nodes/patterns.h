#ifndef JACY_HIR_NODES_PATTERNS_H
#define JACY_HIR_NODES_PATTERNS_H

#include "hir/nodes/Pat.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/fragments.h"

namespace jc::hir {
    struct WildcardPat {};

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
}

#endif // JACY_HIR_NODES_PATTERNS_H
