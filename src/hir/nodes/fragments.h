#ifndef JACY_HIR_NODES_FRAGMENTS_H
#define JACY_HIR_NODES_FRAGMENTS_H

#include "span/Ident.h"
#include "hir/nodes/Stmt.h"
#include "hir/nodes/Expr.h"
#include "hir/nodes/Type.h"
#include "hir/nodes/Pat.h"
#include "resolve/Resolutions.h"

namespace jc::hir {
    template<class T>
    using N = std::unique_ptr<T>;

    using ast::NodeId;
    using span::Ident;

    template<class T, class Name = Ident>
    struct NamedNode {
        using List = std::vector<NamedNode<T, Name>>;

        NamedNode(Name && name, T && node, Span span)
            : name {std::move(name)}, node {std::move(node)}, span {span} {}

        Name name;
        T node;

        /// Span for the whole node
        Span span;
    };

    using Arg = NamedNode<Expr, Ident::Opt>;

    struct Block {
        using Opt = Option<Block>;

        Block(Stmt::List && stmts, Span span)
            : span {span}, stmts {std::move(stmts)} {}

        Span span;
        Stmt::List stmts;
    };

    /// Identifier of the `Body` declared below
    /// For now, just a wrapper over `HirId`
    struct BodyId {
        using Opt = Option<BodyId>;

        NodeId nodeId;

        bool operator<(const BodyId & other) const {
            return nodeId < other.nodeId;
        }
    };

    struct Param {
        using List = std::vector<Param>;

        Param(Pat && pat, Span span) : span {span}, pat {std::move(pat)} {}

        Span span;
        Pat pat;
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
            return BodyId {value.nodeId};
        }
    };

    /// Anonymous constant, used in `const` parameters and arguments, etc.
    struct AnonConst {
        using Opt = Option<AnonConst>;

        BodyId bodyId;
    };

    struct GenericArg {
        struct Lifetime {
            Span span; // Span including `'`
            Ident name;
        };

        struct ConstArg {
            AnonConst value;
            /// Span of the constant expression which is not stored directly inside `AnonConst`
            Span span;
        };

        using List = std::vector<GenericArg>;
        using ValueT = std::variant<Type, Lifetime, ConstArg>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericArg(Type && type) : value {std::move(type)}, kind {Kind::Type} {}

        GenericArg(Lifetime && lifetime) : value {std::move(lifetime)}, kind {Kind::Lifetime} {}

        GenericArg(ConstArg && value) : value {std::move(value)}, kind {Kind::Const} {}

        ValueT value;
        Kind kind;

        const auto & getType() const {
            return std::get<Type>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstArg() const {
            return std::get<ConstArg>(value);
        }
    };

    /// General path fragment used for type and expression paths
    struct PathSeg {
        using List = std::vector<PathSeg>;

        PathSeg(Ident name, GenericArg::List && generics)
            : name {name}, generics {std::move(generics)} {}

        Ident name;
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
            Lifetime(Ident name, Span span) : span {span}, name {name} {}

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
        struct TypeParam {
            Ident name;
            // TODO: Default type (`func<T = i32> foo()`)
        };

        struct ConstParam {
            Ident name;
            Type type;
            // TODO: Default
        };

        using ValueT = std::variant<Lifetime, TypeParam, ConstParam>;
        using List = std::vector<GenericParam>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericParam(TypeParam type, GenericBound::List && bounds, Span span)
            : span {span},
              kind {Kind::Type},
              value {std::move(type)},
              bounds {std::move(bounds)} {}

        GenericParam(Lifetime lifetime, GenericBound::List && bounds, Span span)
            : span {span},
              kind {Kind::Lifetime},
              value {std::move(lifetime)},
              bounds {std::move(bounds)} {}

        GenericParam(ConstParam anonConst, GenericBound::List && bounds, Span span)
            : span {span},
              kind {Kind::Const},
              value {std::move(anonConst)},
              bounds {std::move(bounds)} {}

        Span span;
        Kind kind;
        ValueT value;
        GenericBound::List bounds;

        const auto & getType() const {
            return std::get<TypeParam>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstParam() const {
            return std::get<ConstParam>(value);
        }
    };
}

#endif // JACY_HIR_NODES_FRAGMENTS_H
