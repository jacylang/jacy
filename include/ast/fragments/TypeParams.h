#ifndef JACY_TYPEPARAMS_H
#define JACY_TYPEPARAMS_H

#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct Type;
    struct TypeParam;
    using type_param_list = std::vector<std::shared_ptr<TypeParam>>;
    using opt_type_params = dt::Option<type_param_list>;
    using type_ptr = std::shared_ptr<Type>;
    using opt_type_ptr = dt::Option<type_ptr>;

    struct TypeParam : Node {
        explicit TypeParam(const Span & span) : Node(span) {}
    };

    struct Lifetime : TypeParam {
        Lifetime(id_ptr id, const Span & span) : id(id), TypeParam(span) {}

        id_ptr id;
    };

    struct GenericType : TypeParam {
        GenericType(id_ptr id, opt_type_ptr type, const Span & span)
            : id(std::move(id)), type(std::move(type)), TypeParam(span) {}

        id_ptr id;
        opt_type_ptr type;
    };

    struct ConstParam : TypeParam {
        ConstParam(
            id_ptr id,
            type_ptr type,
            opt_expr_ptr defaultValue,
            const Span & span
        ) : id(std::move(id)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)),
            TypeParam(span) {}

        id_ptr id;
        type_ptr type;
        opt_expr_ptr defaultValue;
    };
}

#endif // JACY_TYPEPARAMS_H


