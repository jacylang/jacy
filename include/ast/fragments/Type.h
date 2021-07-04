#ifndef JACY_AST_FRAGMENTS_TYPE_H
#define JACY_AST_FRAGMENTS_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/Generics.h"
#include "ast/BaseVisitor.h"

// TODO: Separate Types by files in folder `type`

namespace jc::ast {
    struct Type;
    struct TupleTypeEl;
    struct TypePathSeg;
    struct TypePath;
    using type_list = std::vector<type_ptr>;
    using tuple_t_el_ptr = P<TupleTypeEl>;
    using tuple_t_el_list = std::vector<tuple_t_el_ptr>;
    using id_t_list = std::vector<P<TypePathSeg>>;
    using type_path_ptr = P<TypePath>;
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
        virtual ~Type() = default;

        TypeKind kind;

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
        TupleTypeEl(opt_id_ptr name, opt_type_ptr type, const Span & span)
            : Node(span),
              name(std::move(name)),
              type(std::move(type)) {}

        opt_id_ptr name;
        opt_type_ptr type;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TupleType : Type {
        TupleType(tuple_t_el_list elements, const Span & span)
            : Type(span, TypeKind::Tuple), elements(std::move(elements)) {}

        tuple_t_el_list elements;

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
        ArrayType(type_ptr type, expr_ptr sizeExpr, const Span & span)
            : Type(span, TypeKind::Array),
              type(std::move(type)),
              sizeExpr(std::move(sizeExpr)) {}

        type_ptr type;
        expr_ptr sizeExpr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePathSeg : Node {
        TypePathSeg(id_ptr name, opt_gen_params generics, const Span & span)
            : Node(span), name(std::move(name)), generics(std::move(generics)) {}

        id_ptr name;
        opt_gen_params generics;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TypePath : Type {
        TypePath(bool global, id_t_list segments, const Span & span)
            : Type(span, TypeKind::Path), global(global), segments(std::move(segments)) {}

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
