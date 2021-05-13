#ifndef JACY_ARGLIST_H
#define JACY_ARGLIST_H

#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct Argument;
    struct ArgList;
    using arg_ptr = std::shared_ptr<Argument>;
    using arg_list = std::vector<arg_ptr>;
    using arg_list_ptr = std::shared_ptr<ArgList>;

    struct Argument : Node {
        Argument(dt::Option<id_ptr> id, dt::Option<expr_ptr> value, const Location & loc)
            : id(std::move(id)), value(std::move(value)), Node(loc) {}

        dt::Option<id_ptr> id;
        dt::Option<expr_ptr> value;
    };

    struct ArgList : Node {
        ArgList(arg_list elements, const Location & loc) : elements(elements), Node(loc) {}

        arg_list elements;
    };
}

#endif // JACY_ARGLIST_H
