#ifndef JACY_TYPE_H
#define JACY_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/TypeParams.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Type;
    struct TupleTypeElement;
    struct IdType;
    using type_ptr = std::shared_ptr<Type>;
    using type_list = std::vector<type_ptr>;
    using tuple_t_el_ptr = std::shared_ptr<TupleTypeElement>;
    using tuple_t_el_list = std::vector<tuple_t_el_ptr>;
    using id_t_list = std::vector<std::shared_ptr<IdType>>;

    enum class TypeKind {
        Paren,
        Tuple,
        Func,
        List,
        Ref,
        Unit,
    };

    struct Type : Node {
        Type(const Location & loc, TypeKind kind) : Node(loc), kind(kind) {}

        TypeKind kind;

        virtual void accept(BaseVisitor & visitor) = 0;

        static inline type_ptr asBase(type_ptr type) {
            return std::static_pointer_cast<Type>(type);
        }
    };

    struct ParenType : Type {
        ParenType(type_ptr type, const Location & loc) : type(type), Type(loc, TypeKind::Paren) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct TupleType : Type {
        TupleType(tuple_t_el_list elements, const Location & loc) : elements(elements), Type(loc, TypeKind::Tuple) {}

        tuple_t_el_list elements;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct TupleTypeElement : Node {
        TupleTypeElement(opt_id_ptr id, opt_type_ptr type, const Location & loc)
            : id(id), type(type), Node(loc) {}

        opt_id_ptr id;
        opt_type_ptr type;
    };

    struct FuncType : Type {
        FuncType(type_list params, type_ptr returnType, const Location & loc)
            : params(params), returnType(returnType), Type(loc, TypeKind::Func) {}

        type_list params;
        type_ptr returnType;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct ArrayType : Type {
        ArrayType(type_ptr type, const Location & loc) : type(type), Type(loc, TypeKind::List) {}

        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct IdType : Node {
        IdType(id_ptr id, type_param_list typeParams) : id(id), typeParams(typeParams), Node(id->loc) {}

        id_ptr id;
        type_param_list typeParams;
    };

    struct RefType : Type {
        RefType(id_t_list ids, const Location & loc) : ids(ids), Type(loc, TypeKind::Ref) {}

        id_t_list ids;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };

    struct UnitType : Type {
        explicit UnitType(const Location & loc) : Type(loc, TypeKind::Unit) {}

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TYPE_H
