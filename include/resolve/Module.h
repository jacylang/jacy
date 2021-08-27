#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"
#include "resolve/Definition.h"
#include "resolve/ResStorage.h"

namespace jc::resolve {
    using ast::NodeId;

    enum class ModuleKind {
        Block,
        Def,
    };

    struct Module {
        using Ptr = std::shared_ptr<Module>;
        using OptPtr = Option<Ptr>;

        using NSMap = std::map<std::string, DefId>;

        Module(
            ModuleKind kind,
            Ptr parent,
            OptNodeId nodeId,
            opt_def_id defId,
            opt_def_id nearestModDef
        ) : kind(kind),
            parent(parent),
            nodeId(nodeId),
            defId(defId),
            nearestModDef(nearestModDef) {}

        ModuleKind kind;
        OptPtr parent{None};

        // Node id for `Block` module
        OptNodeId nodeId{None};

        // Definition id for `Def` module
        opt_def_id defId{None};

        // Nearest `mod` definition
        opt_def_id nearestModDef;

        PerNS<NSMap> perNS;
        PrimTypeSet shadowedPrimTypes{0};

        // `Block` module
        static inline Module::Ptr newBlockModule(NodeId nodeId, Module::Ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, nodeId, None, nearestModDef);
        }

        // `Def` module
        static inline Module::Ptr newDefModule(const DefId & defId, Module::Ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Def, parent, None, defId, nearestModDef);
        }

        opt_def_id find(Namespace nsKind, const std::string & name) const {
            const auto & ns = getNS(nsKind);
            const auto & def = ns.find(name);
            if (def == ns.end()) {
                return None;
            }
            return def->second;
        }

        // Search for name in all namespaces
        // Also used to find alternatives for failed resolutions
        PerNS<opt_def_id> findAll(const std::string & name) const {
            return {
                find(Namespace::Value, name),
                find(Namespace::Type, name),
                find(Namespace::Lifetime, name)
            };
        }

        const Module::NSMap & getNS(Namespace ns) const {
            switch (ns) {
                case Namespace::Value: return perNS.value;
                case Namespace::Type: return perNS.type;
                case Namespace::Lifetime: return perNS.lifetime;
            }
            log::Logger::notImplemented("Module::getNS");
        }

        Module::NSMap & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return perNS.value;
                case Namespace::Type: return perNS.type;
                case Namespace::Lifetime: return perNS.lifetime;
            }
            log::Logger::notImplemented("Module::getNS");
        }

        opt_def_id tryDefine(Namespace ns, const std::string & name, const DefId & defId) {
            const auto & defined = getNS(ns).emplace(name, defId);
            // Note: emplace returns `pair<new element iterator, true>` if emplaced new element
            //  and `pair<old element iterator, false>` if tried to re-emplace
            if (not defined.second) {
                // Return old def id
                return defined.first->second;
            }
            return None;
        }

        std::string toString() const {
            std::string repr = kindStr();
            repr += " ";
            if (kind == ModuleKind::Block) {
                repr += "block #" + nodeId.unwrap().toString();
            } else if (kind == ModuleKind::Def) {
                repr += "module #" + std::to_string(defId.unwrap().getIndex());
            }
            return repr;
        }

        inline const char * kindStr() const {
            switch (kind) {
                case ModuleKind::Block: return "[BLOCK]";
                case ModuleKind::Def: return "[DEF]";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        constexpr static inline const char * nsToString(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return "value";
                case Namespace::Type: return "type";
                case Namespace::Lifetime: return "lifetime";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }
    };
}

#endif // JACY_RESOLVE_MODULESTACK_H
