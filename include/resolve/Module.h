#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"
#include "resolve/Definition.h"

namespace jc::resolve {
    struct Module;
    using ast::node_id;
    using mod_ns_map = std::map<std::string, node_id>;

    enum class Namespace {
        Item,
        Type,
    };

    enum class ModuleKind {
        Root,
        Block,
        Def,
    };

    struct Module {
        Module(ModuleKind kind, dt::Option<module_ptr> parent) : kind(kind), parent(parent) {}
        Module(def_id defId, dt::Option<module_ptr> parent) : kind(ModuleKind::Def), defId(defId), parent(parent) {}

        ModuleKind kind;
        dt::Option<def_id> defId;
        dt::Option<module_ptr> parent;

        mod_ns_map valueNS;
        mod_ns_map typeNS;

        mod_ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Item: return valueNS;
                case Namespace::Type: return typeNS;
                default: {
                    common::Logger::devPanic("Invalid `ModNode` namespace specified");
                }
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
