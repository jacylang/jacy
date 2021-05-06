#ifndef JACY_NAMEDLIST_H
#define JACY_NAMEDLIST_H

#include "tree/expr/Identifier.h"

namespace jc::tree {
    struct NamedElement;
    struct NamedList;
    using named_el_ptr = std::shared_ptr<NamedElement>;
    using named_el_list = std::vector<named_el_ptr>;
    using named_list_ptr = std::shared_ptr<NamedList>;

    struct NamedElement {
        id_ptr id;
        expr_ptr value;
    };

    struct NamedList : Node {
        NamedList(const named_el_list & elements, const Location & loc) : elements(elements), Node(loc) {}

        named_el_list elements;
    };
}

#endif // JACY_NAMEDLIST_H
