#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"
#include "resolve/Definition.h"

namespace jc::resolve {
    struct Module;
    using ast::node_id;
    using mod_ns_map = std::map<std::string, def_id>;

    enum class ModuleKind {
        Root,
        Block,
        Def,
    };

    struct Module {
        Module(ModuleKind kind, dt::Option<module_ptr> parent) : kind(kind), parent(parent) {}
        Module(def_id defId, dt::Option<module_ptr> parent) : kind(ModuleKind::Def), defId(defId), parent(parent) {}

        ModuleKind kind;
        dt::Option<def_id> defId{dt::None};
        dt::Option<module_ptr> parent{dt::None};

        mod_ns_map valueNS;
        mod_ns_map typeNS;
        mod_ns_map lifetimeNS;
        std::map<std::string, module_ptr> children;
        std::map<node_id, module_ptr> anonBlocks;

        mod_ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return valueNS;
                case Namespace::Type: return typeNS;
                case Namespace::Lifetime: return lifetimeNS;
                default: {
                    common::Logger::devPanic("Invalid `Module` namespace specified");
                }
            }
        }

        constexpr static inline const char * nsToString(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return "item namespace";
                case Namespace::Type: return "type namespace";
                case Namespace::Lifetime: return "lifetime namespace";
            }
        }
    };

    struct ModulePrinter {
        ModulePrinter();

        void print(module_ptr module);

    private:
        common::Logger log{"ModulePrinter"};

        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_RESOLVE_MODULESTACK_H
