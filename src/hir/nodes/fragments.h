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

    /// Anonymous constant, used in `const` parameters and arguments, etc.
    struct AnonConst {
        HirId hirId;
        BodyId bodyId;
    };

    struct GenericArg {
        struct Lifetime {
            HirId hirId;
            Span span; // Span including `'`
            Ident name;
        };

        struct Const {
            AnonConst value;
            /// Span of the constant expression which is not stored directly inside `AnonConst`
            Span span;
        };

        using List = std::vector<GenericArg>;
        // TODO: ConstArg as AnonConst
        using ValueT = std::variant<Type::Ptr, Lifetime, Const>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericArg(Type::Ptr && type) : value {std::move(type)}, kind {Kind::Type} {}

        GenericArg(Lifetime && lifetime) : value {std::move(lifetime)}, kind {Kind::Lifetime} {}

        GenericArg(Const && value) : value {std::move(value)}, kind {Kind::Const} {}

        ValueT value;
        Kind kind;

        const auto & getType() const {
            return std::get<Type::Ptr>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstParam() const {
            return std::get<Const>(value);
        }
    };

    struct GenericParam : HirNode {
        /// Lifetime parameter
        struct Lifetime {
            Ident name;
            // TODO: Bounds
        };

        /// Generic type parameter
        struct Type {
            Ident name;
            // TODO: Default type (`func<T = i32> foo()`)
        };

        struct Const {
            Ident name;
            hir::Type::Ptr type;
            // TODO: Default
        };

        using ValueT = std::variant<Lifetime, Type, Const>;
        using List = std::vector<GenericParam>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericParam(Type type, HirId hirId, Span span)
            : HirNode {hirId, span}, kind {Kind::Type}, value {std::move(type)} {}

        GenericParam(Lifetime lifetime, HirId hirId, Span span)
            : HirNode {hirId, span}, kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        GenericParam(Const anonConst, HirId hirId, Span span)
            : HirNode {hirId, span}, kind {Kind::Const}, value {std::move(anonConst)} {}

        Kind kind;
        ValueT value;

        const auto & getType() const {
            return std::get<Type>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstArg() const {
            return std::get<Const>(value);
        }
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
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
