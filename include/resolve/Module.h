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
            dt::Option<module_ptr> parent,
            opt_node_id nodeId,
            opt_def_id defId,
            opt_def_id nearestModDef
        ) : kind(kind),
            parent(parent),
            nodeId(nodeId),
            defId(defId),
            nearestModDef(nearestModDef) {}

        ModuleKind kind;
        dt::Option<module_ptr> parent{None};

        // Node id for `Block` module
        opt_node_id nodeId{None};

        // Definition id for `Def` module
        dt::Option<def_id> defId{None};

        // Nearest `mod` definition
        opt_def_id nearestModDef;

        mod_ns_map valueNS;
        mod_ns_map typeNS;
        mod_ns_map lifetimeNS;
        prim_type_set_t shadowedPrimTypes{0};

        // `Fictive` or `Root` module
        static inline module_ptr newWrapperModule(ModuleKind kind, dt::Option<module_ptr> parent) {
            return std::make_shared<Module>(kind, parent, None, None, None);
        }

        // `Block` module
        static inline module_ptr newBlockModule(node_id nodeId, module_ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, nodeId, None, nearestModDef);
        }

        // `Def` module
        static inline module_ptr newDefModule(def_id defId, module_ptr parent, opt_def_id nearestModDef) {
            return std::make_shared<Module>(ModuleKind::Block, parent, None, defId, nearestModDef);
        }

        mod_ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return valueNS;
                case Namespace::Type: return typeNS;
                case Namespace::Lifetime: return lifetimeNS;
            }
            common::Logger::notImplemented("Module::getNS");
        }

        constexpr inline const char * kindStr() const {
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
