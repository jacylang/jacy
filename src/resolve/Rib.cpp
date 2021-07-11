#include "resolve/Rib.h"

namespace jc::resolve {
    opt_node_id Rib::define(const ast::id_ptr & ident) {
        const auto & name = ident.unwrap()->getValue();
        const auto & found = locals.find(name);
        if (found == locals.end()) {
            locals.emplace(name, ident.unwrap()->id);
            return None;
        }
        return found->second;
    }

    bool Rib::find(Namespace ns, const std::string & name, node_id refNodeId, ResStorage & resStorage) {
        // Try to find local var first as it has higher precedence than items
        if (ns == Namespace::Value) {
            const auto & local = locals.find(name);
            if (local != locals.end()) {
                common::Logger::devDebug("Set resolution for node #", refNodeId, " as local #", local->second);
                resStorage.setRes(refNodeId, Res{local->second});
                return true;
            }
        }

        // Try to find name in bound module
        bool resolved = false;
        boundModule.then([&](const auto & mod) {
            mod->find(ns, name).then([&](auto defId) {
                common::Logger::devDebug("Set resolution for node #", refNodeId, " as def #", defId);
                resStorage.setRes(refNodeId, Res{defId});
                resolved = true;
            });
        });

        return resolved;
    }

    void Rib::bindMod(module_ptr module) {
        boundModule = module;
    }
}
