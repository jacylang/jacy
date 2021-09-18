#ifndef JACY_AST_FRAGMENTS_TYPE_H
#define JACY_AST_FRAGMENTS_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/Generics.h"
#include "ast/BaseVisitor.h"
#include "ast/fragments/Path.h"

namespace jc::ast {
    enum class TypeKind {
        Paren,
        Tuple,
        Func,
        Slice,
        Array,
        Path,
        Unit,
    };

    struct Type : Node {
        using Ptr = PR<N<Type>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Type(const Span & span, TypeKind kind) : Node{span}, kind{kind} {}
        virtual ~Type() = default;

        TypeKind kind;

        template<class T>
        static T * as(const N<Type> & item) {
            return static_cast<T*>(item.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct ParenType : Type {
        ParenType(Type::Ptr type, const Span & span) : Type{span, TypeKind::Paren}, type{std::move(type)} {}

        Type::Ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleTypeEl : Node {
        using List = std::vector<TupleTypeEl>;

        TupleTypeEl(Ident::OptPR name, Type::OptPtr type, const Span & span)
            : Node{span},
              name{std::move(name)},
              type{std::move(type)} {}

        Ident::OptPR name;
        Type::OptPtr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleType : Type {

        TupleType(TupleTypeEl::List elements, const Span & span)
            : Type{span, TypeKind::Tuple}, elements{std::move(elements)} {}

        TupleTypeEl::List elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct FuncType : Type {
        FuncType(
            Type::List params,
            Type::Ptr returnType,
            const Span & span
        ) : Type{span, TypeKind::Func},
            params{std::move(params)},
            returnType{std::move(returnType)} {}

        Type::List params;
        Type::Ptr returnType;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SliceType : Type {
        SliceType(Type::Ptr type, const Span & span)
            : Type{span, TypeKind::Slice}, type{std::move(type)} {}

        Type::Ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ArrayType : Type {
        ArrayType(Type::Ptr type, Expr::Ptr sizeExpr, const Span & span)
            : Type{span, TypeKind::Array},
              type{std::move(type)},
              sizeExpr{std::move(sizeExpr)} {}

        Type::Ptr type;
        Expr::Ptr sizeExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePath : Type {
        using Ptr = N<TypePath>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        TypePath(Path && path) : Type{path.span, TypeKind::Path}, path{std::move(path)} {}

        Path path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UnitType : Type {
        explicit UnitType(const Span & span) : Type{span, TypeKind::Unit} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_TYPE_H
