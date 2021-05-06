#ifndef JACY_FUNCPARAM_H
#define JACY_FUNCPARAM_H

#include "tree/expr/Identifier.h"
#include "tree/fragments/Type.h"

namespace jc::tree {
    struct FuncParam;
    using func_param_ptr = std::shared_ptr<FuncParam>;
    using func_param_list = std::vector<func_param_ptr>;

    struct FuncParam {
        FuncParam(id_ptr id, type_ptr type, expr_ptr defaultValue)
            : id(id), type(type), defaultValue(defaultValue) {}

        id_ptr id;
        type_ptr type;
        expr_ptr defaultValue;
    };
}

#endif // JACY_FUNCPARAM_H
