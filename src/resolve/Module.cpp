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
            case Namespace::Any: {
                log::devPanic("Called `Module::getDefKindNS` on `Namespace::Any`");
            }
        }
        log::notImplemented("getDefKindNS");
    }

    Module::NSMap & Module::getNS(Namespace ns) {
        switch (ns) {
            case Namespace::Value: return perNS.value;
            case Namespace::Type: return perNS.type;
            case Namespace::Lifetime: return perNS.lifetime;
            case Namespace::Any: {
                log::devPanic("Called `Module::getDefKindNS` on `Namespace::Any`");
            }
        }
        log::notImplemented("getDefKindNS");
    }

    bool Module::has(Namespace nsKind, Symbol name) const {
        if (nsKind == Namespace::Any) {
            return has(Namespace::Value, name) or has(Namespace::Type, name) or has(Namespace::Lifetime, name);
        }

        const auto & ns = getNS(nsKind);
        return ns.find(name) != ns.end();
    }

    NameBinding::Opt Module::find(Namespace nsKind, Symbol name) const {
        const auto & ns = getNS(nsKind);
        const auto & def = ns.find(name);
        if (def == ns.end()) {
            return None;
        }
        return def->second;
    }

    /// Search for name in all namespaces
    /// Mostly used to find alternatives for failed resolutions
    PerNS<NameBinding::Opt> Module::findAll(Symbol name) const {
        return {
            find(Namespace::Value, name),
            find(Namespace::Type, name),
            find(Namespace::Lifetime, name)
        };
    }

    /**
     * @brief Try to find FOS by name, returns Some(FOSId) if found, None if nothing found
     *  and Err(DefId) in case if name is used for non-FOS definition
     * @param name
     * @return
     */
    dt::Result<FOSId::Opt, DefId> Module::tryFindFOS(Symbol name) const {
        const auto & ns = getNS(Namespace::Value);
        const auto & found = ns.find(name);
        if (found == ns.end()) {
            return Ok<FOSId::Opt>(None);
        }
        if (found->second.kind == NameBinding::Kind::FOS) {
            return Ok<FOSId::Opt>(found->second.asFOS());
        }
        return Err(found->second.asDef());
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
