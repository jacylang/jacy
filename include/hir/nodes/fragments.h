#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "span/Ident.h"
#include "hir/nodes/Stmt.h"
#include "hir/nodes/Expr.h"

namespace jc::hir {
    struct Arg : HirNode {
        using List = std::vector<Arg>;

        Arg(const span::Ident & ident, Expr::Ptr && value, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), ident(ident), value{std::move(value)} {}

        span::Ident ident;
        Expr::Ptr value;
    };

    struct Block : HirNode {
        Block(Stmt::List && stmts, const HirId & hirId, const Span & span)
            : HirNode(hirId, span), stmts{std::move(stmts)} {}

        Stmt::List stmts;
    };

    /// General path fragment used for type and expression paths
    // TODO: Add generic args
    struct Path {
        Path(resolve::Res res) : res(res) {}

        resolve::Res res;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
