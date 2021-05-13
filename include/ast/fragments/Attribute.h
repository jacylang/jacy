#ifndef JACY_ATTRIBUTE_H
#define JACY_ATTRIBUTE_H

#include "ast/Node.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/ArgList.h"

namespace jc::ast {
    struct Attribute;
    using attr_ptr = std::shared_ptr<Attribute>;
    using attr_list = std::vector<attr_ptr>;

    struct Attribute : Node {
        Attribute(id_ptr id, arg_list_ptr params, const Location & loc) : id(id), params(params), Node(loc) {}

        id_ptr id;
        arg_list_ptr params;
    };
}

#endif // JACY_ATTRIBUTE_H
