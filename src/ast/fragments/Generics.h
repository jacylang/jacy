#ifndef JACY_AST_FRAGMENTS_GENERICS_H
#define JACY_AST_FRAGMENTS_GENERICS_H

#include "ast/fragments/Ident.h"
#include "ast/fragments/AnonConst.h"

namespace jc::ast {
    struct Type;
    using GenericsTypePtr = PR<N<Type>>;

    // Generic Parameters //
    struct GenericParam {
        struct Lifetime {
            Ident::PR name;
            Span span;
        };

        struct Const {
            Const(
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

        struct Type {
            Type(
                Ident::PR name,
                Option<GenericsTypePtr> type
            ) : name {std::move(name)},
                boundType {std::move(type)} {}

            Ident::PR name;
            Option<GenericsTypePtr> boundType;
        };

        using ValueT = std::variant<Type, Const, Lifetime>;
        using List = std::vector<GenericParam>;
        using OptList = Option<List>;

        enum class Kind {
            Type,
            Lifetime,
            Const,
        };

        GenericParam(Type && type) : kind {Kind::Type}, value {std::move(type)} {}

        GenericParam(Lifetime && lifetime) : kind {Kind::Lifetime}, value {std::move(lifetime)} {}

        GenericParam(Const && constParam) : kind {Kind::Const}, value {std::move(constParam)} {}

        NodeId id;
        Kind kind;

    private:
        ValueT value;

    public:
        void accept(BaseVisitor & visitor) const {
            return visitor.visit(*this);
        }

        const auto & getTypeParam() const {
            return std::get<Type>(value);
        }

        const auto & getLifetime() const {
            return std::get<Lifetime>(value);
        }

        const auto & getConstParam() const {
            return std::get<Const>(value);
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
        struct Lifetime : Node {
            Lifetime(Ident::PR && name, Span span) : Node {span}, name {std::move(name)} {}

            Ident::PR name;
        };

        using PR = PR<GenericArg>;
        using List = std::vector<GenericArg::PR>;
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


