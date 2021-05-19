#ifndef JACY_FUNCPARAM_H
#define JACY_FUNCPARAM_H

#include "ast/expr/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct FuncParam;
    using func_param_ptr = std::shared_ptr<FuncParam>;
    using func_param_list = std::vector<func_param_ptr>;

    struct FuncParam : Node {
        FuncParam(
            id_ptr id,
            type_ptr type,
            opt_expr_ptr defaultValue,
            const Location & loc
        ) : id(std::move(id)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)),
            Node(loc) {}

        id_ptr id;
        type_ptr type;
        opt_expr_ptr defaultValue;
    };
}

#endif // JACY_FUNCPARAM_H
