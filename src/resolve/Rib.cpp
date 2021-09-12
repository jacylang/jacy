#include "resolve/Rib.h"

namespace jc::resolve {
    NodeId::Opt Rib::defineLocal(NodeId nodeId, const Symbol & name) {
        const auto & found = locals.find(name);
        if (found == locals.end()) {
            locals.emplace(name, nodeId);
            return None;
        }
        return found->second;
    }

    NodeId::Opt Rib::findLocal(const Symbol & name) {
        const auto & local = locals.find(name);
        if (local != locals.end()) {
            return local->second;
        }
        return None;
    }

    void Rib::bindMod(Module::Ptr module) {
        boundModule = module;
    }
}
