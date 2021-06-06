#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    // ModulePrinter //
    ModulePrinter::ModulePrinter() {
        log.getConfig().printOwner = false;
    }

    void ModulePrinter::print(Module * module) {
        printIndent();
        log.raw(module->name, "{");
        log.nl();
        indent++;
        for (const auto & child : module->children) {
            print(child);
        }
        indent--;
        log.nl();
        printIndent();
        log.raw("}");
    }

    void ModulePrinter::printIndent() {
        log.raw(utils::str::repeat("  ", indent));
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
    void ModuleTreeBuilder::enterMod(const std::string & name) {
        auto child = new Module(name, mod);
        // TODO: Check for redeclaration
        mod->children.emplace_back(child);
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = std::move(mod->parent.unwrap("[ScopeTreeBuilder]: Tried to exit global scope"));
    }
}
