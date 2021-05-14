#ifndef JACY_NAMEDLIST_H
#define JACY_NAMEDLIST_H

#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct NamedElement;
    struct NamedList;
    using named_el_ptr = std::shared_ptr<NamedElement>;
    using named_list = std::vector<named_el_ptr>;
    using named_list_ptr = std::shared_ptr<NamedList>;

    struct NamedElement : Node {
        NamedElement(dt::Option<id_ptr> id, dt::Option<expr_ptr> value, const Location & loc)
            : id(std::move(id)), value(std::move(value)), Node(loc) {}

        dt::Option<id_ptr> id;
        dt::Option<expr_ptr> value;
    };

    struct NamedList : Node {
        NamedList(named_list elements, const Location & loc) : elements(elements), Node(loc) {}

        named_list elements;
    };
}

#endif // JACY_NAMEDLIST_H
