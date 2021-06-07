#ifndef JACY_INCLUDE_RESOLVE_RESSTORAGE_H
#define JACY_INCLUDE_RESOLVE_RESSTORAGE_H

#include <map>

#include "ast/Node.h"

namespace jc::resolve {
    using ast::node_id;

    class ResStorage {
    public:
        ResStorage() = default;

        node_id getRes(node_id name) const {
            return resolutions.at(name);
        }

    private:
        std::map<node_id, node_id> resolutions;
    };
}

#endif // JACY_INCLUDE_RESOLVE_RESSTORAGE_H
