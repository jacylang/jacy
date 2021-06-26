#include "resolve/Name.h"

namespace jc::resolve {
    opt_node_id Rib::define(const std::string & name, ast::node_id nodeId) {
        const auto & found = locals.find(name);
        if (found == locals.end()) {
            locals.emplace(name, nodeId);
            return dt::None;
        }
        return found->second;
    }

    opt_node_id Rib::resolve(const std::string & name) {
        const auto & found = locals.find(name);
        if (found == locals.end()) {
            return dt::None;
        }
        return found->second;
    }

    void Rib::bindMod(module_ptr module) {
        boundModule = std::move(module);
    }
}
