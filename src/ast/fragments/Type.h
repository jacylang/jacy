#ifndef JACY_AST_FRAGMENTS_TYPE_H
#define JACY_AST_FRAGMENTS_TYPE_H

#include "ast/Node.h"
#include "ast/BaseVisitor.h"
#include "ast/fragments/Path.h"
#include "ast/fragments/AnonConst.h"

namespace jc::ast {
    struct Type : Node {
        using Ptr = APR<N<Type>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        enum class Kind {
            Paren,
            Tuple,
            Func,
            Slice,
            Array,
            Path,
            Unit,
        };

        Type(Span span, Kind kind) : Node {span}, kind {kind} {}

        virtual ~Type() = default;

        Kind kind;

        template<class T>
        static T * as(const N<Type> & item) {
            return static_cast<T*>(item.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    using NamedType = NamedNode<Type::Ptr, Ident::OptPR>;

    struct ParenType : Type {
        ParenType(Type::Ptr type, Span span) : Type {span, Type::Kind::Paren}, type {std::move(type)} {}

        Type::Ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleType : Type {
        TupleType(NamedType::List elements, Span span)
            : Type {span, Type::Kind::Tuple}, elements {std::move(elements)} {}

        NamedType::List elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct FuncType : Type {
        FuncType(
            NamedType::List params,
            Type::Ptr returnType,
            Span span
        ) : Type {span, Type::Kind::Func},
            params {std::move(params)},
            returnType {std::move(returnType)} {}

        NamedType::List params;
        Type::Ptr returnType;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SliceType : Type {
        SliceType(Type::Ptr type, Span span)
            : Type {span, Type::Kind::Slice}, type {std::move(type)} {}

        Type::Ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ArrayType : Type {
        ArrayType(Type::Ptr type, AnonConst sizeExpr, Span span)
            : Type {span, Type::Kind::Array},
              type {std::move(type)},
              sizeExpr {std::move(sizeExpr)} {}

        Type::Ptr type;
        AnonConst sizeExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePath : Type {
        using Ptr = N<TypePath>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        TypePath(Path && path) : Type {path.span, Type::Kind::Path}, path {std::move(path)} {}

        Path path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UnitType : Type {
        explicit UnitType(Span span) : Type {span, Type::Kind::Unit} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_TYPE_H
