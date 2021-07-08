#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"
#include "resolve/Definition.h"
#include "resolve/ResStorage.h"

namespace jc::resolve {
    struct Module;
    using ast::node_id;
    using mod_ns_map = std::map<std::string, def_id>;
    using dt::None;

    enum class ModuleKind {
        Root,
        Block,
        Def,
        Fictive,
    };

    struct Module {
        Module(
            ModuleKind kind,
            opt_module_ptr parent,
            opt_node_id nodeId,
            opt_def_id defId,
            opt_def_id nearestModDef
        ) : kind(kind),
            parent(parent),
            nodeId(nodeId),
            defId(defId),
            nearestModDef(nearestModDef) {}

        ModuleKind kind;
        opt_module_ptr parent{None};

        // Node id for `Block` module
        opt_node_id nodeId{None};

        // Definition id for `Def` module
        opt_def_id defId{None};

        // Nearest `mod` definition
        opt_def_id nearestModDef;

        PerNS<mod_ns_map> perNS;
        prim_type_set_t shadowedPrimTypes{0};

        // `Fictive` or `Root` module
        static inline module_ptr newWrapperModule(ModuleKind kind, opt_module_ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(kind, parent, None, None, nearestModDef);
        }

        // `Block` module
        static inline module_ptr newBlockModule(node_id nodeId, module_ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, nodeId, None, nearestModDef);
        }

        // `Def` module
        static inline module_ptr newDefModule(def_id defId, module_ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Def, parent, None, defId, nearestModDef);
        }

        opt_def_id find(Namespace nsKind, const std::string & name) const {
            const auto & ns = getNS(nsKind);
            const auto & def = ns.find(name);
            if (def == ns.end()) {
                return dt::None;
            }
            return def->second;
        }

        // Find alternatives in other namespaces excluding specified one
        PerNS<opt_def_id> findAlt(Namespace excludeNs, const std::string & name) const {
            return {
                find(Namespace::Value, name),
                find(Namespace::Type, name),
                find(Namespace::Lifetime, name)
            };
        }

        const mod_ns_map & getNS(Namespace ns) const {
            switch (ns) {
                case Namespace::Value: return perNS.valueNS;
                case Namespace::Type: return perNS.typeNS;
                case Namespace::Lifetime: return perNS.lifetimeNS;
            }
            common::Logger::notImplemented("Module::getNS");
        }

        mod_ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return perNS.valueNS;
                case Namespace::Type: return perNS.typeNS;
                case Namespace::Lifetime: return perNS.lifetimeNS;
            }
            common::Logger::notImplemented("Module::getNS");
        }

        inline const char * kindStr() const {
            switch (kind) {
                case ModuleKind::Root: return "[ROOT]";
                case ModuleKind::Block: return "[BLOCK]";
                case ModuleKind::Def: return "[DEF]";
                case ModuleKind::Fictive: return "[FICTIVE]";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }

        constexpr static inline const char * nsToString(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return "value namespace";
                case Namespace::Type: return "type namespace";
                case Namespace::Lifetime: return "lifetime namespace";
                default: return "[NO REPRESENTATION (bug)]";
            }
        }
    };

    struct ModulePrinter {
        ModulePrinter();

        void print(sess::sess_ptr sess);

    private:
        sess::sess_ptr sess;
        common::Logger log{"ModulePrinter"};

        void printMod(module_ptr module);
        void printNS(const mod_ns_map & ns);
        void printDef(def_id defId);
        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_RESOLVE_MODULESTACK_H
