#ifndef JACY_RESOLVE_MODULETREEBUILDER_H
#define JACY_RESOLVE_MODULETREEBUILDER_H

#include "ast/StubVisitor.h"

namespace jc::resolve {
    struct ModNode;
    using ast::node_id;
    using mod_node_ptr = std::shared_ptr<ModNode>;

    struct ModNode {
        ModNode(const std::string & name, dt::Option<mod_node_ptr> parent) : name(name), parent(parent) {}

        std::string name;
        dt::Option<mod_node_ptr> parent;
        std::vector<mod_node_ptr> children;
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
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Trait & trait) override;

//        void visit(const ast::Struct & _struct) override;

    private:
        common::Logger log{"ScopeTreeBuilder"};

        // Modules //
    private:
        mod_node_ptr mod;

        void enterMod(const std::string & name);
        void exitMod();
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
