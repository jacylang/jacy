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
        TypeParam(id_ptr id, type_ptr type) : id(id), type(type), Node(id->loc) {}

        id_ptr id;
        type_ptr type;
    };
}

#endif //JACY_TYPEPARAMS_H
