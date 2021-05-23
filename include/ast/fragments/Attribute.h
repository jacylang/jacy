#ifndef JACY_AST_FRAGMENTS_ATTRIBUTE_H
#define JACY_AST_FRAGMENTS_ATTRIBUTE_H

#include "ast/Node.h"
#include "Identifier.h"
#include "ast/fragments/NamedList.h"

namespace jc::ast {
    struct Attribute;
    using attr_ptr = std::shared_ptr<Attribute>;
    using attr_list = std::vector<attr_ptr>;

    struct Attribute : Node {
        Attribute(id_ptr name, named_list_ptr params, const Span & span)
            : name(std::move(name)), params(std::move(params)), Node(span) {}

        id_ptr name;
        named_list_ptr params;
    };
}

#endif // JACY_AST_FRAGMENTS_ATTRIBUTE_H
