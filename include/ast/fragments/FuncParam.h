#ifndef JACY_FUNCPARAM_H
#define JACY_FUNCPARAM_H

#include "Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct FuncParam;
    using func_param_ptr = std::shared_ptr<FuncParam>;
    using func_param_list = std::vector<func_param_ptr>;

    struct FuncParam : Node {
        FuncParam(
            id_ptr name,
            type_ptr type,
            opt_expr_ptr defaultValue,
            const Span & span
        ) : name(std::move(name)),
            type(std::move(type)),
            defaultValue(std::move(defaultValue)),
            Node(span) {}

        id_ptr name;
        type_ptr type;
        opt_expr_ptr defaultValue;
    };
}

#endif // JACY_FUNCPARAM_H
