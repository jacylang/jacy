#ifndef JACY_AST_FRAGMENTS_TYPE_H
#define JACY_AST_FRAGMENTS_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/Generics.h"
#include "ast/BaseVisitor.h"
#include "ast/fragments/Path.h"

// TODO: Separate Types by files in folder `type`

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
        using Ptr = N<Type>;
        using List = std::vector<Ptr>;

        Type(const Span & span, TypeKind kind) : Node(span), kind(kind) {}
        virtual ~Type() = default;

        TypeKind kind;

        template<class T>
        static T * as(const N<Type> & item) {
            return static_cast<T*>(item.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct ParenType : Type {
        ParenType(type_ptr type, const Span & span) : Type(span, TypeKind::Paren), type(std::move(type)) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleTypeEl : Node {
        using List = std::vector<TupleTypeEl>;

        TupleTypeEl(Ident::OptPR name, opt_type_ptr type, const Span & span)
            : Node(span),
              name(std::move(name)),
              type(std::move(type)) {}

        Ident::OptPR name;
        opt_type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleType : Type {

        TupleType(TupleTypeEl::List elements, const Span & span)
            : Type(span, TypeKind::Tuple), elements(std::move(elements)) {}

        TupleTypeEl::List elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct FuncType : Type {
        FuncType(
            Type::List params,
            type_ptr returnType,
            const Span & span
        ) : Type(span, TypeKind::Func),
            params(std::move(params)),
            returnType(std::move(returnType)) {}

        Type::List params;
        type_ptr returnType;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SliceType : Type {
        SliceType(type_ptr type, const Span & span)
            : Type(span, TypeKind::Slice), type(std::move(type)) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ArrayType : Type {
        ArrayType(type_ptr type, Expr::Ptr sizeExpr, const Span & span)
            : Type(span, TypeKind::Array),
              type(std::move(type)),
              sizeExpr(std::move(sizeExpr)) {}

        type_ptr type;
        Expr::Ptr sizeExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePath : Type {
        using Ptr = N<TypePath>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        TypePath(Path && path) : Type(path.span, TypeKind::Path), path(std::move(path)) {}

        Path path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct UnitType : Type {
        explicit UnitType(const Span & span) : Type(span, TypeKind::Unit) {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_TYPE_H
