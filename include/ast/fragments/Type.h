#ifndef JACY_AST_FRAGMENTS_TYPE_H
#define JACY_AST_FRAGMENTS_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/Generics.h"
#include "ast/BaseVisitor.h"
#include "ast/fragments/Path.h"

// TODO: Separate Types by files in folder `type`

namespace jc::ast {
    struct Type;
    struct TupleTypeEl;
    using type_list = std::vector<type_ptr>;
    using type_path_ptr = N<TypePath>;
    using tuple_field_list = std::vector<TupleTypeEl>;
    using opt_type_path_ptr = Option<type_path_ptr>;
    using type_path_list = std::vector<type_path_ptr>;

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
        TupleTypeEl(opt_ident name, opt_type_ptr type, const Span & span)
            : Node(span),
              name(std::move(name)),
              type(std::move(type)) {}

        opt_ident name;
        opt_type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleType : Type {
        TupleType(tuple_field_list elements, const Span & span)
            : Type(span, TypeKind::Tuple), elements(std::move(elements)) {}

        tuple_field_list elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct FuncType : Type {
        FuncType(
            type_list params,
            type_ptr returnType,
            const Span & span
        ) : Type(span, TypeKind::Func),
            params(std::move(params)),
            returnType(std::move(returnType)) {}

        type_list params;
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
        ArrayType(type_ptr type, ExprPtr sizeExpr, const Span & span)
            : Type(span, TypeKind::Array),
              type(std::move(type)),
              sizeExpr(std::move(sizeExpr)) {}

        type_ptr type;
        ExprPtr sizeExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePath : Type {
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
