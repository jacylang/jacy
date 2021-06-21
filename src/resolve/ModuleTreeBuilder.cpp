#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        party.getRootModule()->accept(*this);
        sess->modTreeRoot = mod;

        return {dt::None, extractSuggestions()};
    }

    void ModuleTreeBuilder::visit(const ast::RootModule & rootModule) {
        mod = std::make_shared<Module>(dt::None);
        rootModule.getRootFile()->accept(*this);
        rootModule.getRootDir()->accept(*this);
    }

    void ModuleTreeBuilder::visit(const ast::FileModule & fileModule) {
        // This is actually impossible to redeclare file, filesystem does not allow it
        enterMod(fileModule.getName(), fileModule.id, dt::None);
        fileModule.getFile()->accept(*this);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::DirModule & dirModule) {
        enterMod(dirModule.getName(), dirModule.id, dt::None);
        for (const auto & module : dirModule.getModules()) {
            module->accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        declare(ModuleNamespace::Item, func.name, func.id);
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterMod(mod.name.unwrap()->getValue(), mod.id, mod.name.unwrap()->span);
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        declare(ModuleNamespace::Item, _struct.name, _struct.id);
        StubVisitor::visit(_struct);
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterMod(trait.name.unwrap()->getValue(), trait.id, trait.name.unwrap()->span);
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        declare(ModuleNamespace::Type, typeAlias.name, typeAlias.id);
        typeAlias.type.accept(*this);
    }

    // Modules //
    void ModuleTreeBuilder::declare(ModuleNamespace ns, const ast::id_ptr & ident, node_id nodeId) {
        const auto & name = ident.unwrap()->getValue();
        auto & map = mod->getNS(ns);
        if (utils::map::has(map, name)) {
            suggestErrorMsg("'" + name + "' `mod` has been already declared", ident.unwrap()->span);
        }
        map[name] = nodeId;
    }

    /// `nameSpan` is optional for filesystem modules (file/dir do not have span)
    void ModuleTreeBuilder::enterMod(const std::string & name, node_id nodeId, const dt::Option<span::Span> & nameSpan) {
        if (utils::map::has(mod->children, nodeId)) {
            log.devPanic("Tried to declare module with same node_id twice");
        }

        auto child = std::make_shared<Module>(mod);
        if (utils::map::has(mod->childrenNames, name)) {
            if (not nameSpan) {
                log.devPanic(
                    "This is impossible to enter module which is file/dir and which has been already declared"
                );
            } else {
                suggestErrorMsg("'" + name + "' has been already declared", nameSpan.unwrap());
            }
        } else {
            // Add node_id -> module binding only if it wasn't redeclared
            mod->children.emplace(nodeId, child);

            // Add module name as offset of child in `children`
            mod->childrenNames.emplace(name, mod->children.size() - 1);
        }
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit global scope");
    }
}
