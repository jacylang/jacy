#ifndef JACY_INCLUDE_RESOLVE_RESSTORAGE_H
#define JACY_INCLUDE_RESOLVE_RESSTORAGE_H

#include <map>

#include "ast/Node.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;

    struct Res {
        enum class ResKind {
            Def,
            Local,
        };

        dt::Option<def_id> def;
        dt::Option<node_id> nodeId;
    };

    /// ResStorage
    /// @brief Collection of {path: node} names resolutions
    class ResStorage {
    public:
        ResStorage() = default;

        opt_node_id getRes(node_id name) const {
            const auto & found = resolutions.find(name);
            if (found != resolutions.end()) {
                return found->second;
            }
            return dt::None;
        }

        void setRes(node_id name, node_id res) {
            resolutions.emplace(name, res);
        }

        const std::map<node_id, node_id> getResolutions() const {
            return resolutions;
        }

    private:
        std::map<node_id, node_id> resolutions;
    };
}

#endif // JACY_INCLUDE_RESOLVE_RESSTORAGE_H
