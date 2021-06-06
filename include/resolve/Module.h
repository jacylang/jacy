#ifndef JACY_RESOLVE_MODULESTACK_H
#define JACY_RESOLVE_MODULESTACK_H

#include "ast/Party.h"

namespace jc::resolve {
    struct ModNode;
    using ast::node_id;
    using ns_map = std::map<std::string, node_id>;
    using mod_node_ptr = std::shared_ptr<ModNode>;

    enum class Namespace {
        Value,
        Type,
    };

    struct ModNode {
        ModNode(dt::Option<mod_node_ptr> parent) : parent(parent) {}

        dt::Option<mod_node_ptr> parent;
        std::map<std::string, mod_node_ptr> children;

        ns_map valueNS;
        ns_map typeNS;

        ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return valueNS;
                case Namespace::Type: return typeNS;
                default:;
            }
        }
    };

    struct ModulePrinter {
        ModulePrinter();

        void print(mod_node_ptr module);

    private:
        common::Logger log{"ModulePrinter"};

        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_RESOLVE_MODULESTACK_H
