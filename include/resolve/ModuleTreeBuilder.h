#ifndef JACY_RESOLVE_MODULETREEBUILDER_H
#define JACY_RESOLVE_MODULETREEBUILDER_H

#include "ast/StubVisitor.h"

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

    class ModuleTreeBuilder : public ast::StubVisitor {
    public:
        ModuleTreeBuilder() : StubVisitor("ScopeTreeBuilder") {}

        mod_node_ptr build(const ast::Party & party);

        void visit(const ast::RootModule & rootModule) override;
        void visit(const ast::FileModule & fileModule) override;
        void visit(const ast::DirModule & dirModule) override;

        void visit(const ast::Func & func) override;
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Struct & _struct) override;
        void visit(const ast::Trait & trait) override;
        void visit(const ast::TypeAlias & typeAlias) override;

//        void visit(const ast::Struct & _struct) override;

    private:
        common::Logger log{"ScopeTreeBuilder"};

        // Modules //
    private:
        mod_node_ptr mod;
        void declare(Namespace ns, const std::string & name, node_id nodeId);

        void enterMod(const std::string & name);
        void exitMod();
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
