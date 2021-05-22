#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include <stack>

#include "ast/Node.h"

namespace jc::resolve {
    struct Name;
    struct Type;
    struct Item;
    struct Local;
    struct Lifetime;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using rib_stack = std::stack<rib_ptr>;
    using opt_node_id = dt::Option<ast::node_id>;
    using type_ptr = std::shared_ptr<Type>;
    using item_ptr = std::shared_ptr<Item>;
    using local_ptr = std::shared_ptr<Local>;
    using lifetime_ptr = std::shared_ptr<Lifetime>;

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

        std::string kindStr() const {
            return kindStr(kind);
        }

        static std::string kindStr(Kind kind) {
            switch (kind) {
                case Kind::Generic: return "generic type";
            }
        }
    };

    struct Item : Name {
        enum class Kind {
            Func,
            Trait,
        } kind;

        Item(Kind kind, ast::node_id nodeId) : kind(kind), Name(nodeId) {}

        std::string kindStr() const {
            return kindStr(kind);
        }

        static std::string kindStr(Kind kind) {
            switch (kind) {
                case Kind::Func: return "function";
            }
        }
    };

    struct Local : Name {

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
