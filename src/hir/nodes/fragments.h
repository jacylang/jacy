#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "span/Ident.h"
#include "hir/nodes/Stmt.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/Type.h"
#include "hir/nodes/Pat.h"
#include "resolve/Resolutions.h"

namespace jc::hir {
    struct Arg {
        using List = std::vector<Arg>;

        Arg(const span::Ident::Opt & ident, Expr && value)
            : ident {ident}, value {std::move(value)} {}

        span::Ident::Opt ident;
        Expr value;
    };

    struct Block {
        using Opt = Option<Block>;

        Block(Stmt::List && stmts, HirId hirId, Span span)
            : hirId {hirId}, span {span}, stmts {std::move(stmts)} {}

        HirId hirId;
        Span span;
        Stmt::List stmts;
    };

    /// Identifier of the `Body` declared below
    /// For now, just a wrapper over `HirId`
    struct BodyId {
        HirId hirId;

        bool operator<(const BodyId & other) const {
            return hirId < other.hirId;
        }
    };

    struct Param {
        using List = std::vector<Param>;

        Param(Pat::Ptr && pat, HirId hirId, Span span) : hirId {hirId}, span {span}, pat {std::move(pat)} {}

        HirId hirId;
        Span span;
        Pat::Ptr pat;
        // TODO: Default value as `AnonConst`, should it be here or in `FuncSig`?
    };

    /// Function body
    /// Separated from `Func` as it is type checked apart
    struct Body {
        Body(bool exprBody, Expr && value, Param::List && params)
            : exprBody {exprBody}, value {std::move(value)}, params {std::move(params)} {}

        /// Denotes that `func`'s body was defined with `=`
        bool exprBody;

        /// Function body value, BlockExpr if `func` was declared with `{}` and any expr if with `=`
        Expr value;

        Param::List params;

        BodyId getId() const {
            // Note: The BodyId is actually an HirId of expression inside Body
            return BodyId {value.hirId};
        }
    };

    /// Anonymous constant, used in `const` parameters and arguments, etc.
    struct AnonConst {
        using Opt = Option<AnonConst>;

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

        const auto & getConstArg() const {
            return std::get<Const>(value);
        }
    };

    /// General path fragment used for type and expression paths
    struct PathSeg {
        using List = std::vector<PathSeg>;

        PathSeg(const span::Ident & name, GenericArg::List && generics)
            : name {std::move(name)}, generics {std::move(generics)} {}

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

    struct GenericBound {
        using List = std::vector<GenericBound>;

        /// Trait bound
        struct Trait {
            Path path;
        };

        struct Lifetime {
            Lifetime(Ident name, HirId hirId, Span span) : hirId {hirId}, span {span}, name {name} {}

            HirId hirId;
            Span span;
            Ident name;
        };

        using ValueT = std::variant<Trait, Lifetime>;

        enum class Kind {
            Trait,
            Lifetime,
        };

        GenericBound(Trait && trait) : kind {Kind::Trait}, value {std::move(trait)} {}

        GenericBound(Lifetime && lifetime) : kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        Kind kind;
        ValueT value;
    };

    struct GenericParam {
        /// Lifetime parameter
        struct Lifetime {
            Ident name;
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

        GenericParam(Type type, GenericBound::List && bounds, HirId hirId, Span span)
            : hirId {hirId},
              span {span},
              kind {Kind::Type},
              value {std::move(type)},
              bounds {std::move(bounds)} {}

        GenericParam(Lifetime lifetime, GenericBound::List && bounds, HirId hirId, Span span)
            : hirId {hirId},
              span {span},
              kind {Kind::Lifetime},
              value {std::move(lifetime)},
              bounds {std::move(bounds)} {}

        GenericParam(Const anonConst, GenericBound::List && bounds, HirId hirId, Span span)
            : hirId {hirId},
              span {span},
              kind {Kind::Const},
              value {std::move(anonConst)},
              bounds {std::move(bounds)} {}

        HirId hirId;
        Span span;
        Kind kind;
        ValueT value;
        GenericBound::List bounds;

        const auto & getType() const {
            return std::get<Type>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstParam() const {
            return std::get<Const>(value);
        }
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
