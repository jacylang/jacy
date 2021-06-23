#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;
        party.getRootModule()->accept(*this);

        return {dt::None, extractSuggestions()};
    }

    void ModuleTreeBuilder::visit(const ast::RootModule & rootModule) {
        mod = std::make_shared<Module>(ModuleKind::Root, dt::None);
        sess->modTreeRoot = mod;
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
        // Note: Here, we only need function body to visit and do not enter module because body is a Block expression
        func.body->accept(*this);
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        enterMod(dt::None, impl.id, dt::None);
        visitEach(impl.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        define(Namespace::Item, mod.name, mod.id);
        enterMod(mod.name.unwrap()->getValue(), mod.id, mod.name.unwrap()->span);
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        define(Namespace::Item, _struct.name, _struct.id);
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        define(Namespace::Item, trait.name, trait.id);
        enterMod(trait.name.unwrap()->getValue(), trait.id, trait.name.unwrap()->span);
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        define(Namespace::Type, typeAlias.name, typeAlias.id);
        typeAlias.type.accept(*this);
    }

    void ModuleTreeBuilder::visit(const ast::Block & block) {
        if (block.blockKind == ast::BlockKind::OneLine) {
            // Note: One line block does not open module,
            //  anyway it can contain another Block which could be a multi-line
            block.oneLine.unwrap().accept(*this);
        } else {
            enterMod(dt::None, block.id, dt::None);
            visitEach(block.stmts.unwrap());
            exitMod();
        }
    }

    // Modules //
    void ModuleTreeBuilder::define(Namespace ns, const ast::id_ptr & ident, node_id nodeId) {
        const auto & name = ident.unwrap()->getValue();
        auto & map = mod->getNS(ns);
        if (utils::map::has(map, name)) {
            suggestErrorMsg("'" + name + "' has been already declared", ident.unwrap()->span);
        }
        map[name] = nodeId;
    }



//    void ModuleTreeBuilder::exitMod() {
//        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit global scope");
//    }
}
