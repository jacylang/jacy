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
            const auto & found = locals.find(name);
            if (found != locals.end()) {
                resStorage.setRes(nodeId, Res{found->second});
                return true;
            }
        }

        // If no module bound we unable to resolve name
        if (boundModule) {
            const auto & modNS = boundModule.unwrap()->getNS(ns);
            const auto & found = modNS.find(name);
            if (found != modNS.end()) {
                resStorage.setRes(nodeId, Res{found->second});
                return true;
            }
        }

        resStorage.setRes(nodeId, Res{});
        return false;
    }

    void Rib::bindMod(module_ptr module) {
        boundModule = std::move(module);
    }
}
