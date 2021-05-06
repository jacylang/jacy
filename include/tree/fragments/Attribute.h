#ifndef JACY_ATTRIBUTE_H
#define JACY_ATTRIBUTE_H

#include "tree/Node.h"
#include "tree/expr/Identifier.h"
#include "tree/fragments/NamedList.h"

namespace jc::tree {
    struct Attribute;
    using attr_ptr = std::shared_ptr<Attribute>;
    using attr_list = std::vector<attr_ptr>;

    struct Attribute : Node {
        Attribute(id_ptr id, named_list_ptr params, const Location & loc) : id(id), params(params), Node(loc) {}

        id_ptr id;
        named_list_ptr params;
    };
}

#endif // JACY_ATTRIBUTE_H
