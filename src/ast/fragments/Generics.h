#ifndef JACY_AST_FRAGMENTS_GENERICS_H
#define JACY_AST_FRAGMENTS_GENERICS_H

#include "ast/fragments/Ident.h"
#include "ast/fragments/AnonConst.h"

namespace jc::ast {
    struct Type;
    using GenericsTypePtr = PR<N<Type>>;

    // Generic Parameters //

    /// Common structure for generic parameters and arguments as being just an identifier
    /// Span of the Lifetime (from `GenericParam <- Node`) is a span of lifetime with quote `'`
    /// whereas `name`'s span does not include it.
    struct Lifetime {
        Lifetime(Ident::PR name) : name {std::move(name)} {}

        // TODO: Add Span for name with `'`?
        Ident::PR name;

        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }
    };

    struct ConstParam {
        ConstParam(
            Ident::PR name,
            GenericsTypePtr type,
            AnonConst::Opt defaultValue
        ) : name {std::move(name)},
            type {std::move(type)},
            defaultValue {std::move(defaultValue)} {}

        Ident::PR name;
        GenericsTypePtr type;
        AnonConst::Opt defaultValue;
    };

    struct TypeParam {
        TypeParam(
            Ident::PR name,
            Option<GenericsTypePtr> type
        ) : name {std::move(name)},
            boundType {std::move(type)} {}

        Ident::PR name;
        Option<GenericsTypePtr> boundType;
    };

    struct GenericParam {
        using ValueT = std::variant<TypeParam, ConstParam, Lifetime>;
        using List = std::vector<GenericParam>;
        using OptList = Option<List>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericParam(TypeParam && type) : kind {Kind::Type}, value {std::move(type)} {}

        GenericParam(Lifetime && lifetime) : kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        GenericParam(ConstParam && constParam) : kind {Kind::Const}, value {std::move(constParam)} {}

        NodeId id;
        Kind kind;

    private:
        ValueT value;

    public:
        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }

        const auto & getTypeParam() const {
            return std::get<TypeParam>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstParam() const {
            return std::get<ConstParam>(value);
        }

        const Ident & name() const {
            switch (kind) {
                case Kind::Type: {
                    return getTypeParam().name.unwrap();
                }
                case Kind::Lifetime: {
                    return getLifetime().name.unwrap();
                }
                case Kind::Const: {
                    return getConstParam().name.unwrap();
                }
            }
        }
    };

    // Generic arguments //
    struct GenericArg {
        struct Lifetime {
            Lifetime(Ident::PR name) : name {name} {}

            Ident::PR name;
            NodeId id;
        };

        using List = std::vector<GenericArg>;
        using OptList = Option<List>;
        using ValueT = std::variant<GenericsTypePtr, Lifetime, AnonConst>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericArg(GenericsTypePtr && type) : kind {Kind::Type}, value {std::move(type)} {}

        GenericArg(Lifetime && lifetime) : kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        GenericArg(AnonConst && expr) : kind {Kind::Const}, value {std::move(expr)} {}

        Kind kind;

    private:
        ValueT value;

    public:
        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }

        const auto & getTypeArg() const {
            return std::get<GenericsTypePtr>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstArg() const {
            return std::get<AnonConst>(value);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_GENERICS_H


