#ifndef JACY_AST_FRAGMENTS_GENERICS_H
#define JACY_AST_FRAGMENTS_GENERICS_H

#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct Type;
    using GenericsTypePtr = PR <N<Type>>;

    // Generic Parameters //
    enum class GenericParamKind {
        Type,
        Lifetime,
        Const,
    };

    /// Common structure for generic parameters and arguments as being just an identifier
    /// Span of the Lifetime (from `GenericParam <- Node`) is a span of lifetime with quote `'`
    /// whereas `name`'s span does not include it.
    struct Lifetime : Node {
        Lifetime(Ident::PR name, Span span)
            : Node(span),
              name {std::move(name)} {}

        Ident::PR name;
        Span span;
    };

    struct ConstParam : Node {
        ConstParam(
            Ident::PR name,
            GenericsTypePtr type,
            Expr::OptPtr defaultValue,
            Span span
        ) : Node {span},
            name {std::move(name)},
            type {std::move(type)},
            defaultValue {std::move(defaultValue)} {}

        Ident::PR name;
        GenericsTypePtr type;
        Expr::OptPtr defaultValue;
    };

    struct TypeParam : Node {
        TypeParam(
            Ident::PR name,
            Option <GenericsTypePtr> type,
            Span span
        ) : Node {span},
            name {std::move(name)},
            boundType {std::move(type)} {}

        Ident::PR name;
        Option <GenericsTypePtr> boundType;
    };

    struct GenericParam {
        using ValueT = std::variant<TypeParam, ConstParam, Lifetime>;
        using List = std::vector<N < GenericParam>>;
        using OptList = Option<List>;

        explicit GenericParam(GenericParamKind kind) : kind {kind} {}

        GenericParamKind kind;
    };

    // Generic arguments //
    struct GenericArg {
        using List = std::vector<GenericArg>;
        using OptList = Option<List>;
        using ValueT = std::variant<GenericsTypePtr, Lifetime, Expr::Ptr>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericArg(GenericsTypePtr && type) : kind {Kind::Type}, value {std::move(type)} {}

        GenericArg(Lifetime && lifetime) : kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        GenericArg(Expr::Ptr && expr) : kind {Kind::Const}, value {std::move(expr)} {}

        Kind kind;
        ValueT value;
    };
}

#endif // JACY_AST_FRAGMENTS_GENERICS_H


