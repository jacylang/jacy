#include "resolve/Module.h"

namespace jc::resolve {
    void Module::assertKind(ModuleKind kind) const {
        if (this->kind != kind) {
            log::devPanic(
                "[Module::assertKind] Failed - expected '" + kindStr(kind) + "', got '" + kindStr() + "'");
        }
    }

    auto Module::getNodeId() const {
        assertKind(ModuleKind::Block);
        return std::get<NodeId>(id);
    }

    const DefId & Module::getDefId() const {
        assertKind(ModuleKind::Def);
        return std::get<DefId>(id);
    }

    const Module::NSMap & Module::getNS(Namespace ns) const {
        switch (ns) {
            case Namespace::Value: return perNS.value;
            case Namespace::Type: return perNS.type;
            case Namespace::Lifetime: return perNS.lifetime;
        }
        log::notImplemented("getNS");
    }

    Module::NSMap & Module::getNS(Namespace ns) {
        switch (ns) {
            case Namespace::Value: return perNS.value;
            case Namespace::Type: return perNS.type;
            case Namespace::Lifetime: return perNS.lifetime;
        }
        log::notImplemented("getNS");
    }

    IntraModuleDef::Opt Module::find(Namespace nsKind, const Symbol & name) const {
        const auto & ns = getNS(nsKind);
        const auto & def = ns.find(name);
        if (def == ns.end()) {
            return None;
        }
        return def->second;
    }

    DefId::Opt Module::findDefOnly(Namespace nsKind, const Symbol & name) const {
        const auto & ns = getNS(nsKind);
        const auto & def = ns.find(name);
        if (def == ns.end()) {
            return None;
        }
        if (def->second.kind == IntraModuleDef::Kind::Target) {
            return def->second.asDef();
        }
        return None;
    }

    /// Search for name in all namespaces
    /// Mostly used to find alternatives for failed resolutions
    PerNS<IntraModuleDef::Opt> Module::findAll(const Symbol & name) const {
        return {
            find(Namespace::Value, name),
            find(Namespace::Type, name),
            find(Namespace::Lifetime, name)
        };
    }

    std::string Module::toString() const {
        std::string repr = kindStr();
        repr += " ";
        if (kind == ModuleKind::Block) {
            repr += "block #" + std::get<NodeId>(id).toString();
        } else if (kind == ModuleKind::Def) {
            repr += "module #" + std::get<DefId>(id).getIndex().toString();
        }
        return repr;
    }
}
