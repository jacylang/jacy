#ifndef JACY_TYPE_H
#define JACY_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/TypeParams.h"
#include "ast/BaseVisitor.h"

// TODO: Separate Types by files in folder `type`

namespace jc::ast {
    struct Type;
    struct TupleTypeElement;
    struct IdType;
    struct TypePath;
    using type_ptr = std::shared_ptr<Type>;
    using type_list = std::vector<type_ptr>;
    using tuple_t_el_ptr = std::shared_ptr<TupleTypeElement>;
    using tuple_t_el_list = std::vector<tuple_t_el_ptr>;
    using id_t_list = std::vector<std::shared_ptr<IdType>>;
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

        Error,
    };

    struct Type : Node {
        Type(const Span & span, TypeKind kind) : Node(span), kind(kind) {}

        TypeKind kind;

        virtual void accept(BaseVisitor & visitor) = 0;

        static inline type_ptr asBase(type_ptr type) {
            return std::static_pointer_cast<Type>(type);
        }
    };

    struct ParenType : Type {
        ParenType(type_ptr type, const Span & span)
            : type(std::move(type)), Type(span, TypeKind::Paren) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct TupleType : Type {
        TupleType(tuple_t_el_list elements, const Span & span)
            : elements(std::move(elements)), Type(span, TypeKind::Tuple) {}

        tuple_t_el_list elements;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct TupleTypeElement : Node {
        TupleTypeElement(opt_id_ptr id, opt_type_ptr type, const Span & span)
            : id(std::move(id)), type(std::move(type)), Node(span) {}

        opt_id_ptr id;
        opt_type_ptr type;
    };

    struct FuncType : Type {
        FuncType(type_list params, type_ptr returnType, const Span & span)
            : params(std::move(params)), returnType(std::move(returnType)), Type(span, TypeKind::Func) {}

        type_list params;
        type_ptr returnType;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct SliceType : Type {
        SliceType(type_ptr type, const Span & span)
            : type(std::move(type)), Type(span, TypeKind::Slice) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct ArrayType : Type {
        ArrayType(type_ptr type, expr_ptr sizeExpr, const Span & span)
            : type(std::move(type)), size(std::move(sizeExpr)), Type(span, TypeKind::Array) {}

        type_ptr type;
        expr_ptr size;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct IdType : Node {
        IdType(id_ptr id, opt_type_params typeParams, const Span & span)
            : id(std::move(id)), typeParams(std::move(typeParams)), Node(span) {}

        id_ptr id;
        opt_type_params typeParams;
    };

    struct TypePath : Type {
        TypePath(bool global, id_t_list ids, const Span & span)
            : global(global), ids(std::move(ids)), Type(span, TypeKind::Path) {}

        bool global;
        id_t_list ids;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct UnitType : Type {
        explicit UnitType(const Span & span) : Type(span, TypeKind::Unit) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct ErrorType : Type {
        explicit ErrorType(const Span & span) : Type(span, TypeKind::Error) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TYPE_H
