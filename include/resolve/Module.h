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

        /// {node id of any node which behaves as module -> submodule}
        /// List of possible node types which child node_id can point to:
        /// - `mod`
        /// - `struct`
        /// - `trait`
        ///
        /// Also, these nodes are anonymous modules, which are not presented in `childrenNames` but exist in `children`
        /// - Block expression (`{}`), func body, lambda body
        ///
        /// Note: It is important note that `impl` is not presented anywhere
        ///  as far as it does not name anything -- it uses some name
        std::map<node_id, module_ptr> children{};

        /// name -> offset in `children` for named modules
        std::map<std::string, size_t> childrenNames;

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
