#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(mod_node_ptr module) {
        printIndent();
        log.raw("{");
        log.nl();
        indent++;
        for (auto it = module->children.begin(); it != module->children.end(); it++) {
            print(it->second);
            if (it != std::prev(module->children.end())) {
                log.nl();
            }
        }
        log.raw("values:", module->valueNS).nl();
        log.raw("types:", module->typeNS).nl();
        indent--;
        log.nl();
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
    }

    // ModuleTreeBuilder //
    mod_node_ptr ModuleTreeBuilder::build(const ast::Party & party) {
        party.getRootModule()->accept(*this);

        return mod;
    }

    void ModuleTreeBuilder::visit(const ast::RootModule & rootModule) {
        mod = std::make_shared<ModNode>(dt::None);
        rootModule.getRootFile()->accept(*this);
        rootModule.getRootDir()->accept(*this);
    }

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

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        declare(Namespace::Value, func.name.unwrap()->getValue(), func.id);
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterMod(mod.name.unwrap()->getValue());
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        declare(Namespace::Value, _struct.name.unwrap()->getValue(), _struct.id);
        StubVisitor::visit(_struct);
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterMod(trait.name.unwrap()->getValue());
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        declare(Namespace::Type, typeAlias.name.unwrap()->getValue(), typeAlias.id);
        typeAlias.type.accept(*this);
    }

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

    void ModuleTreeBuilder::enterMod(const std::string & name) {
        auto child = std::make_shared<ModNode>(mod);
        // TODO: Check for redeclaration
        mod->children.emplace(name, child);
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = mod->parent.unwrap("[ScopeTreeBuilder]: Tried to exit global scope");
    }
}
