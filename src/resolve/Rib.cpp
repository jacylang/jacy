#include "resolve/Rib.h"

namespace jc::resolve {
    opt_node_id Rib::define(const ast::id_ptr & ident) {
        const auto & name = ident.unwrap()->getValue();
        const auto & found = locals.find(name);
        if (found == locals.end()) {
            locals.emplace(name, ident.unwrap()->id);
            return dt::None;
        }
        return found->second;
    }

    bool Rib::resolve(Namespace ns, const ast::id_ptr & ident, ResStorage & resStorage) {
        const auto & nodeId = ident.unwrap()->id;
        const auto & name = ident.unwrap()->getValue();

        // Try to resolve local var first as it has higher precedence than items
        if (ns == Namespace::Value) {
            const auto & local = locals.find(name);
            if (local != locals.end()) {
                common::Logger::devDebug();
                resStorage.setRes(nodeId, Res{local->second});
                return true;
            }
        }

        // If no module bound we unable to resolve name
        if (boundModule) {
            const auto & modNS = boundModule.unwrap()->getNS(ns);
            const auto & def = modNS.find(name);
            if (def != modNS.end()) {
                resStorage.setRes(nodeId, Res{def->second});
                return true;
            }
        }

        common::Logger::devDebug("Set error resolution for node #", nodeId);
        // Set error resolution
        resStorage.setRes(nodeId, Res{});
        return false;
    }

    void Rib::bindMod(module_ptr module) {
        boundModule = std::move(module);
    }
}
