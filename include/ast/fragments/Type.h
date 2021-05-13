#ifndef JACY_TYPE_H
#define JACY_TYPE_H

#include "ast/Node.h"
#include "ast/fragments/TypeParams.h"

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
    };

    struct ParenType : Type {
        ParenType(type_ptr type, const Location & loc) : type(type), Type(loc, TypeKind::Paren) {}

        type_ptr type;
    };

    struct ListType : Type {
        ListType(type_ptr type, const Location & loc) : type(type), Type(loc, TypeKind::List) {}

        type_ptr type;
    };

    struct IdType : Node {
        IdType(id_ptr id, type_param_list typeParams) : id(id), typeParams(typeParams), Node(id->loc) {}

        id_ptr id;
        type_param_list typeParams;
    };

    struct RefType : Type {
        RefType(id_t_list ids, const Location & loc) : ids(ids), Type(loc, TypeKind::Ref) {}

        id_t_list ids;
    };

    struct TupleType : Type {
        TupleType(tuple_t_el_list elements, const Location & loc) : elements(elements), Type(loc, TypeKind::Tuple) {}

        tuple_t_el_list elements;
    };

    struct UnitType : Type {
        explicit UnitType(const Location & loc) : Type(loc, TypeKind::Unit) {}
    };
}

#endif // JACY_TYPE_H
