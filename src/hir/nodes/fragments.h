#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "span/Ident.h"
#include "hir/nodes/Stmt.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/Type.h"

namespace jc::hir {
    struct Arg : HirNode {
        using List = std::vector<Arg>;

        Arg(const span::Ident::Opt & ident, Expr::Ptr && value, HirId hirId, Span span)
            : HirNode {hirId, span}, ident {ident}, value {std::move(value)} {}

        span::Ident::Opt ident;
        Expr::Ptr value;
    };

    struct Block : HirNode {
        using Opt = Option<Block>;

        Block(Stmt::List && stmts, HirId hirId, Span span)
            : HirNode {hirId, span}, stmts {std::move(stmts)} {}

        Stmt::List stmts;
    };

    struct GenericArg {
        // TODO: Lifetime
        using ValueT = std::variant<Type::Ptr>;

        enum class Kind {
            Type,
            Lifetime,
        };

        GenericArg(Type::Ptr && type) : value {std::move(type)}, kind {Kind::Type} {}

        ValueT value;
        Kind kind;
    };

    /// General path fragment used for type and expression paths
    struct PathSeg : HirNode {
        using List = std::vector<PathSeg>;

        PathSeg(const span::Ident & name, HirId hirId, Span span)
            : HirNode {hirId, span}, name {std::move(name)} {}

        span::Ident name;
        // TODO: Generic args
    };

    struct Path {
        Path(const resolve::Res & res, PathSeg::List && segments, Span span)
            : res {res}, segments {std::move(segments)}, span {span} {}

        resolve::Res res;
        PathSeg::List segments;
        Span span;
    };

    struct GenericParam : HirNode {
        using List = std::vector<GenericParam>;

        enum class Kind {
            Type,
            Lifetime,
        };

        GenericParam(Kind kind, Ident name, HirId hirId, Span span)
            : HirNode {hirId, span}, kind {kind}, name {name} {}

        Kind kind;
        Ident name;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
