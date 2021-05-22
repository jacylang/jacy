#ifndef JACY_HIR_NAME_H
#define JACY_HIR_NAME_H

#include <stack>

#include "ast/Node.h"

namespace jc::hir {
    struct Name;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using rib_stack = std::stack<rib_ptr>;

    template<class T>
    using name_map = std::map<std::string, std::shared_ptr<T>>;

    struct Name {
        ast::node_id nodeId;
    };

    struct Type : Name {
        enum class Kind {
            Generic,
        } kind;
    };

    struct Item : Name {

    };

    struct Local : Name {

    };

    struct Rib {
        name_map<Type> types;
        name_map<Item> items;
        name_map<Local> locals;
    };
}

#endif // JACY_HIR_NAME_H
