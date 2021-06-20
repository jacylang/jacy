#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"

namespace jc::resolve {
    struct Module;
    using ast::node_id;
    using mod_ns_map = std::map<std::string, node_id>;
    using module_ptr = std::shared_ptr<Module>;

    enum class ModuleNamespace {
        Item,
        Type,
    };

    struct Module {
        Module(dt::Option<module_ptr> parent) : parent(parent) {}

        dt::Option<module_ptr> parent;
        std::map<std::string, module_ptr> children{};

        mod_ns_map valueNS;
        mod_ns_map typeNS;

        mod_ns_map & getNS(ModuleNamespace ns) {
            switch (ns) {
                case ModuleNamespace::Item: return valueNS;
                case ModuleNamespace::Type: return typeNS;
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
