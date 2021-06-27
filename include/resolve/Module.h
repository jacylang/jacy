#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"
#include "resolve/Definition.h"
#include "resolve/ResStorage.h"

namespace jc::resolve {
    struct Module;
    using ast::node_id;
    using mod_ns_map = std::map<std::string, def_id>;

    enum class ModuleKind {
        Block,
        Def,
    };

    struct Module {
        explicit Module(node_id nodeId, module_ptr parent)
            : kind(ModuleKind::Block), nodeId(nodeId), parent(parent) {}
        explicit Module(def_id defId, dt::Option<module_ptr> parent)
            : kind(ModuleKind::Def), defId(defId), parent(parent) {}

        ModuleKind kind;
        dt::Option<node_id> nodeId{dt::None};
        dt::Option<def_id> defId{dt::None};
        dt::Option<module_ptr> parent{dt::None};

        mod_ns_map valueNS;
        mod_ns_map typeNS;
        mod_ns_map lifetimeNS;
        prim_type_set_t shadowedPrimTypes;

        mod_ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return valueNS;
                case Namespace::Type: return typeNS;
                case Namespace::Lifetime: return lifetimeNS;
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
