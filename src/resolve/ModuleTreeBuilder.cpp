#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        mod = std::make_unique<Module>(ModuleKind::Root, dt::None);
        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        sess->defStorage = std::move(defStorage);

        return {dt::None, extractSuggestions()};
    }

    void ModuleTreeBuilder::visit(const ast::File & file) {
        // This is actually impossible to redeclare file, filesystem does not allow it
        enterMod(
            sess->sourceMap.getSourceFile(file.fileId).filename(),
            dt::None,
            defStorage.define(DefKind::File, file.span)
        );

        visitEach(file.items);

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Dir & dir) {
        enterMod(dir.name, dt::None, dir.id);
        for (const auto & module : dir.modules) {
            module->accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Enum & _enum) {
        define(Namespace::Type, _enum.name, defStorage.define(DefKind::Enum, _enum.span));
        enterMod(
            _enum.name.unwrap()->getValue(),
            _enum.name.span(),
            defStorage.define(DefKind::Enum, _enum.span)
        );
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        define(Namespace::Type, enumEntry.name, defStorage.define(DefKind::Variant, enumEntry.span));
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        define(Namespace::Item, func.name, defStorage.define(DefKind::Func, func.span));
        // Note: Here, we only need function body to visit and do not enter module because body is a Block expression
        func.body->accept(*this);
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        enterAnonMod(impl.id);
        // Set definition of `impl` block
        mod->defId = defStorage.define(DefKind::Impl, impl.span);
        visitEach(impl.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        define(Namespace::Type, mod.name, defStorage.define(DefKind::Mod, mod.span));
        enterMod(
            mod.name.unwrap()->getValue(),
            mod.name.span(),
            defStorage.define(DefKind::Mod, mod.span)
        );
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        define(Namespace::Type, _struct.name, defStorage.define(DefKind::Struct, _struct.span));
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        define(Namespace::Type, trait.name, defStorage.define(DefKind::Trait, trait.span));
        enterMod(
            trait.name.unwrap()->getValue(),
            trait.name.span(),
            defStorage.define(DefKind::Trait, trait.span)
        );
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
            enterAnonMod(block.id);
            visitEach(block.stmts.unwrap());
            exitMod();
        }
    }

    // Modules //
    void ModuleTreeBuilder::define(Namespace ns, const ast::id_ptr & ident, def_id defId) {
        log.dev(
            "Define '",
            ident.unwrap()->getValue(),
            "' in module with defId [",
            defId,
            "] in ",
            Module::nsToString(ns));
        const auto & name = ident.unwrap()->getValue();
        auto & map = mod->getNS(ns);
        if (utils::map::has(map, name)) {
            suggestErrorMsg("'" + name + "' has been already declared", ident.unwrap()->span);
        }
        map.emplace(name, defId);
    }

    void ModuleTreeBuilder::enterAnonMod(node_id nodeId) {
        if (utils::map::has(mod->anonBlocks, nodeId)) {
            log.devPanic("Tried to redeclare anonymous block");
        }
        auto child = std::make_shared<Module>(ModuleKind::Block, mod);
        mod->anonBlocks.emplace(nodeId, child);
        mod = child;
    }

    void ModuleTreeBuilder::enterMod(const std::string & name, const dt::Option<ast::Span> & nameSpan, def_id defId) {
        log.dev("Enter mod '", name, "'");
        auto child = std::make_shared<Module>(defId, mod);
        if (utils::map::has(mod->children, name)) {
            if (not nameSpan) {
                log.devPanic("Module without name span redeclaration");
            }
            suggestErrorMsg("'" + name + "' has been already declared in this scope", nameSpan.unwrap());
        } else {
            mod->children.emplace(name, child);
        }

        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit global scope");
    }
}
