#ifndef JACY_AST_FRAGMENTS_GENERICS_H
#define JACY_AST_FRAGMENTS_GENERICS_H

#include "ast/fragments/Ident.h"

namespace jc::ast {
    struct Type;
    using GenericsTypePtr = N<Type>;

    // Generic Parameters //
    enum class GenericParamKind {
        Type,
        Lifetime,
        Const,
    };

    struct GenericParam : Node {
        using List = std::vector<N<GenericParam>>;
        using OptList = Option<List>;

        explicit GenericParam(GenericParamKind kind, Span span) : Node{span}, kind{kind} {}
        virtual ~GenericParam() = default;

        GenericParamKind kind;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct TypeParam : GenericParam {
        TypeParam(
            Ident::PR name,
            Option<PR<GenericsTypePtr>> type,
            Span span
        ) : GenericParam{GenericParamKind::Type, span},
            name{std::move(name)},
            boundType{std::move(type)} {}

        Ident::PR name;
        Option<PR<N<Type>>> boundType;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// Common structure for generic parameters and arguments as being just an identifier
    /// Span of the Lifetime (from `GenericParam <- Node`) is a span of lifetime with quote `'`
    /// whereas `name`'s span does not include it.
    struct Lifetime : GenericParam {
        Lifetime(Ident::PR name, Span span)
            : GenericParam{GenericParamKind::Lifetime, span},
              name{std::move(name)} {}

        Ident::PR name;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ConstParam : GenericParam {
        ConstParam(
            Ident::PR name,
            PR<N<Type>> type,
            Expr::OptPtr defaultValue,
            Span span
        ) : GenericParam{GenericParamKind::Const, span},
            name{std::move(name)},
            type{std::move(type)},
            defaultValue{std::move(defaultValue)} {}

        Ident::PR name;
        PR<N<Type>> type;
        Expr::OptPtr defaultValue;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
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


