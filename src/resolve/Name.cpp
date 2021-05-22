#include "resolve/Name.h"

namespace jc::resolve {
    decl_result Rib::declare(const std::string & name, Name::Kind kind, ast::node_id nodeId) {
        const auto & found = names.find(name);
        if (found == names.end()) {
            names.emplace(name, std::make_shared<Name>(kind, nodeId));
            return dt::None;
        }
        return std::tuple<Name::Kind, ast::node_id>{found->second->kind, found->second->nodeId};
    }
}
