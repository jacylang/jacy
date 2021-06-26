#ifndef JACY_INCLUDE_RESOLVE_RESSTORAGE_H
#define JACY_INCLUDE_RESOLVE_RESSTORAGE_H

#include <map>

#include "ast/Node.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;

    enum class ResKind {
        Def,
        Local,
        Error,
    };

    struct Res {
        Res() : kind(ResKind::Error) {}
        Res(def_id def) : kind(ResKind::Def), def(def) {}
        Res(node_id nodeId) : kind(ResKind::Local), nodeId(nodeId) {}

        ResKind kind;
        dt::Option<def_id> def{dt::None};
        dt::Option<node_id> nodeId{dt::None};

        bool isErr() const {
            return kind == ResKind::Error;
        }

        def_id asDef() const {
            return def.unwrap();
        }

        node_id asLocal() const {
            return nodeId.unwrap();
        }
    };

    /// ResStorage
    /// @brief Collection of {path: node} names resolutions
    class ResStorage {
    public:
        ResStorage() = default;

        Res getRes(node_id name) const {
            // Note: It is actually a c-t error if resolution does not exists
            //  as far as unresolved names are stored as Error Res
            return resolutions.at(name);
        }

        void setRes(node_id name, Res res) {
            resolutions.emplace(name, res);
        }

        const std::map<node_id, Res> getResolutions() const {
            return resolutions;
        }

    private:
        /// Map of Identifier node id -> resolution
        std::map<node_id, Res> resolutions;
    };
}

#endif // JACY_INCLUDE_RESOLVE_RESSTORAGE_H
