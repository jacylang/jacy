#ifndef JACY_TYPEPARAMS_H
#define JACY_TYPEPARAMS_H

#include "tree/expr/Identifier.h"
#include "tree/fragments/Type.h"

namespace jc::tree {
    struct TypeParam;
    struct TypeParams;
    using type_param_list = std::vector<std::shared_ptr<TypeParam>>;
    using type_params_ptr = std::shared_ptr<TypeParams>;

    struct TypeParam : Node {
        TypeParam(id_ptr id, type_ptr type) : id(id), type(type), Node(id->loc) {}

        id_ptr id;
        type_ptr type;
    };

    struct TypeParams : Node {
        TypeParams(const type_param_list & typeParams, const Location & loc)
            : typeParams(typeParams), Node(loc) {}

        type_param_list typeParams;
    };
}

#endif //JACY_TYPEPARAMS_H
