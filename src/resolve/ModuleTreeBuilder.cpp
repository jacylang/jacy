#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(Module * module) {
    }

    // ModuleTreeBuilder //
    void ModuleTreeBuilder::visit(const ast::FileModule & fileModule) {
        enterMod(fileModule.getName());
        fileModule.getFile()->accept(*this);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::DirModule & dirModule) {
        enterMod(dirModule.getName());
        for (const auto & module : dirModule.getModules()) {
            module->accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterMod(mod.name.unwrap()->getValue());
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterMod(trait.name.unwrap()->getValue());
        visitEach(trait.members);
        exitMod();
    }

//    void ScopeTreeBuilder::visit(const ast::Struct & _struct) {
//        enterModule(_struct.name.unwrap()->getValue());
//        // TODO: Update for associated items in the future
//        exitMod();
//    }

    // Modules //
    void ModuleTreeBuilder::declare(Namespace ns, const std::string & name, node_id nodeId) {
        auto & map = mod->getNS(ns);
        if (utils::map::has(map, name)) {
            // TODO!!!: Suggestions
            log.error(name + "has been already declared in this scope");
            return;
        }
        map[name] = nodeId;
    }

    void ModuleTreeBuilder::enterMod(const dt::Option<std::string> & name) {
        auto child = new Module(mod);
        if (name) {
            // TODO: Check for redeclaration
            mod->children.emplace(name.unwrap(), child);
        }
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = std::move(mod->parent.unwrap("[ScopeTreeBuilder]: Tried to exit global scope"));
    }
}
