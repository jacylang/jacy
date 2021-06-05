#ifndef JACY_AST_FRAGMENTS_TYPE_H
#define JACY_AST_FRAGMENTS_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/TypeParams.h"
#include "ast/BaseVisitor.h"

// TODO: Separate Types by files in folder `type`

namespace jc::ast {
    struct Type;
    struct TupleTypeElement;
    struct TypePathSegment;
    struct TypePath;
    using type_list = std::vector<type_ptr>;
    using tuple_t_el_ptr = std::shared_ptr<TupleTypeElement>;
    using tuple_t_el_list = std::vector<tuple_t_el_ptr>;
    using id_t_list = std::vector<std::shared_ptr<TypePathSegment>>;
    using type_path_ptr = std::shared_ptr<TypePath>;
    using opt_type_path_ptr = dt::Option<type_path_ptr>;
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

        TypeKind kind;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct ParenType : Type {
        ParenType(type_ptr type, const Span & span)
            : type(std::move(type)), Type(span, TypeKind::Paren) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleTypeElement : Node {
        TupleTypeElement(opt_id_ptr name, opt_type_ptr type, const Span & span)
            : name(std::move(name)), type(std::move(type)), Node(span) {}

        opt_id_ptr name;
        opt_type_ptr type;
    };

    struct TupleType : Type {
        TupleType(tuple_t_el_list elements, const Span & span)
            : elements(std::move(elements)), Type(span, TypeKind::Tuple) {}

        tuple_t_el_list elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct FuncType : Type {
        FuncType(type_list params, type_ptr returnType, const Span & span)
            : params(std::move(params)), returnType(std::move(returnType)), Type(span, TypeKind::Func) {}

        type_list params;
        type_ptr returnType;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SliceType : Type {
        SliceType(type_ptr type, const Span & span)
            : type(std::move(type)), Type(span, TypeKind::Slice) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ArrayType : Type {
        ArrayType(type_ptr type, expr_ptr sizeExpr, const Span & span)
            : type(std::move(type)), sizeExpr(std::move(sizeExpr)), Type(span, TypeKind::Array) {}

        type_ptr type;
        expr_ptr sizeExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePathSegment : Node {
        TypePathSegment(id_ptr name, opt_type_params typeParams, const Span & span)
            : name(std::move(name)), typeParams(std::move(typeParams)), Node(span) {}

        id_ptr name;
        opt_type_params typeParams;
    };

    struct TypePath : Type {
        TypePath(bool global, id_t_list segments, const Span & span)
            : global(global), segments(std::move(segments)), Type(span, TypeKind::Path) {}

        bool global{};
        id_t_list segments{};

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
