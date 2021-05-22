#ifndef JACY_HIR_NAME_H
#define JACY_HIR_NAME_H

#include <stack>

#include "ast/Node.h"

namespace jc::hir {
    struct Name;
    struct Rib;
    using name_ptr = std::shared_ptr<Name>;
    using name_map = std::map<std::string, name_ptr>;
    using rib_ptr = std::shared_ptr<Rib>;
    using rib_stack = std::stack<rib_ptr>;

    struct Name {
        ast::node_id nodeId;
    };

    struct Rib {
        name_map types;
        name_map items;
        name_map locals;
    };
}

#endif // JACY_HIR_NAME_H
