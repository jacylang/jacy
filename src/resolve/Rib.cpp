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

    bool Rib::find(Namespace ns, const std::string & name, node_id refNodeId, ResStorage & resStorage) {
        // Try to resolve local var first as it has higher precedence than items
        if (ns == Namespace::Value) {
            const auto & local = locals.find(name);
            if (local != locals.end()) {
                common::Logger::devDebug("Set resolution for node #", refNodeId, " as local #", local->second);
                resStorage.setRes(refNodeId, Res{local->second});
                return true;
            }
        }

        // If no module bound we unable to resolve name
        if (boundModule) {
            const auto & modNS = boundModule.unwrap()->getNS(ns);
            const auto & def = modNS.find(name);
            if (def != modNS.end()) {
                common::Logger::devDebug("Set resolution for node #", refNodeId, " as def #", def->second);
                resStorage.setRes(refNodeId, Res{def->second});
                return true;
            }
        }

        return false;
    }

    void Rib::bindMod(module_ptr module) {
        boundModule = module;
    }
}
