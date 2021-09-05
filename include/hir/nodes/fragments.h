#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "span/Ident.h"
#include "hir/nodes/Stmt.h"
#include "hir/nodes/Expr.h"

namespace jc::hir {
    struct Arg : HirNode {
        using List = std::vector<Arg>;

        Arg(const span::Ident::Opt & ident, Expr::Ptr && value, const HirId & hirId, const Span & span)
            : HirNode{hirId, span}, ident{ident}, value{std::move(value)} {}

        span::Ident::Opt ident;
        Expr::Ptr value;
    };

    struct Block : HirNode {
        using Opt = Option<Block>;

        Block(Stmt::List && stmts, const HirId & hirId, const Span & span)
            : HirNode{hirId, span}, stmts{std::move(stmts)} {}

        Stmt::List stmts;
    };

    /// General path fragment used for type and expression paths
    struct PathSeg : HirNode {
        using List = std::vector<PathSeg>;

        PathSeg(span::Ident && name, const HirId & hirId, const Span & span)
            : HirNode{hirId, span}, name{std::move(name)} {}

        span::Ident name;
        // TODO: Generic args
    };

    struct Path {
        Path(const resolve::Res & res, PathSeg::List && segments, const Span & span)
            : res{res}, segments{std::move(segments)}, span{span} {}

        resolve::Res res;
        PathSeg::List segments;
        Span span;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
