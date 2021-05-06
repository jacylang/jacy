#ifndef JACY_TYPE_H
#define JACY_TYPE_H

#include "tree/Node.h"

namespace jc::tree {
    struct Type;
    struct TupleTypeElement;
    using type_ptr = std::shared_ptr<Type>;
    using type_list = std::vector<type_ptr>;
    using tuple_t_el_ptr = std::shared_ptr<TupleTypeElement>;
    using tuple_t_el_list = std::vector<tuple_t_el_ptr>;

    enum class TypeKind {
        Paren,
        Id,
        Tuple,
        Func,
        List,
    };

    struct Type : Node {
        Type(const Location & loc, TypeKind kind) : Node(loc), kind(kind) {}

        TypeKind kind;
    };

    struct TupleTypeElement : Node {
        TupleTypeElement(id_ptr id, type_ptr type) : id(id), type(type), Node(id->loc) {}

        id_ptr id;
        type_ptr type;
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

    struct TupleType : Type {
        TupleType(tuple_t_el_list elements, const Location & loc) : elements(elements), Type(loc, TypeKind::Tuple) {}

        tuple_t_el_list elements;
    };
}

#endif // JACY_TYPE_H
