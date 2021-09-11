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

    bool Rib::find(Namespace ns, const Symbol & name, NodeId refNodeId, Resolutions & resStorage) {
        // Try to find local var first as it has higher precedence than items
        if (ns == Namespace::Value) {
            const auto & local = locals.find(name);
            if (local != locals.end()) {
                log::Logger::devDebug("Set resolution for node ", refNodeId, " as local ", local->second);
                resStorage.setRes(refNodeId, Res{local->second});
                return true;
            }
        }

        // Try to find name in bound module
        bool resolved = false;
        boundModule.then([&](const auto & mod) {
            mod->find(ns, name).then([&](const auto & defId) {
                log::Logger::devDebug("Set resolution for node ", refNodeId, " as def ", defId);
                resStorage.setRes(refNodeId, Res {defId});
                resolved = true;
            });
        });

        return resolved;
    }

    void Rib::bindMod(Module::Ptr module) {
        boundModule = module;
    }
}
