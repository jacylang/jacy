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

    /// Identifier of the `Body` declared below
    /// For now, just a wrapper over `HirId`
    struct BodyId {
        HirId hirId;
    };

    /// Function body
    /// Separated from `Func` as it is type checked apart
    struct Body {
        Body(bool exprBody, Expr::Ptr && value) : exprBody {exprBody}, value {std::move(value)} {}

        /// Denotes that `func`'s body was defined with `=`
        bool exprBody;

        /// Function body value, BlockExpr if `func` was declared with `{}` and any expr if with `=`
        Expr::Ptr value;
    };

    struct GenericArg {
        struct Lifetime {
            HirId hirId;
            Span span; // Span including `'`
            Ident name;
        };

        using List = std::vector<GenericArg>;
        // TODO: ConstArg as AnonConst
        using ValueT = std::variant<Type::Ptr, Lifetime>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericArg(Lifetime && lifetime) : value {std::move(lifetime)}, kind {Kind::Type} {}

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
        GenericArg::List generics;
    };

    struct Path {
        Path(const resolve::Res & res, PathSeg::List && segments, Span span)
            : res {res}, segments {std::move(segments)}, span {span} {}

        resolve::Res res;
        PathSeg::List segments;
        Span span;
    };

    struct GenericParam : HirNode {
        /// Lifetime parameter
        struct Lifetime {
            Ident name;
            // TODO: Bounds
        };

        /// Generic type parameter
        struct Type {
            // TODO: Default type (`func<T = i32> foo()`)
        };

        using ValueT = std::variant<Lifetime, Type>;
        using List = std::vector<GenericParam>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericParam(Lifetime lifetime, HirId hirId, Span span)
            : HirNode {hirId, span}, kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        GenericParam(Type type, HirId hirId, Span span)
            : HirNode {hirId, span}, kind {Kind::Lifetime}, value {std::move(type)} {}

        Kind kind;
        ValueT value;
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
