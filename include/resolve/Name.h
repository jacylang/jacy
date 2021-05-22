#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include <stack>

#include "ast/Node.h"

namespace jc::resolve {
    struct Name;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using rib_stack = std::stack<rib_ptr>;

    template<class T>
    using name_map = std::map<std::string, std::shared_ptr<T>>;

    struct Name {
        Name(ast::node_id nodeId) : nodeId(nodeId) {}

        ast::node_id nodeId;
    };

    struct Type : Name {
        enum class Kind {
            Generic,
        } kind;

        Type(Kind kind, ast::node_id nodeId) : kind(kind), Name(nodeId) {}
    };

    struct Item : Name {
        enum class Kind {

        } kind;
    };

    struct Local : Name {
        enum class Kind {

        } kind;
    };

    struct Lifetime : Name {
        explicit Lifetime(ast::node_id nodeId) : Name(nodeId) {}
    };

    struct Rib {
        name_map<Type> types;
        name_map<Item> items;
        name_map<Local> locals;
        name_map<Lifetime> lifetimes;
    };
}

#endif // JACY_RESOLVE_NAME_H
