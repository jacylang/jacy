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

    bool Rib::find(Namespace ns, const Symbol & name, NodeId refNodeId, Resolutions & resStorage) {
        // Try to find local var first as it has higher precedence than items
        if (ns == Namespace::Value) {
        }

        // Try to find name in bound module

        return resolved;
    }

    void Rib::bindMod(Module::Ptr module) {
        boundModule = module;
    }
}
