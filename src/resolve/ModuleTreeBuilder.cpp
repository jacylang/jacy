#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        mod = std::make_unique<Module>(ModuleKind::Root, dt::None);
        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        if (mod->kind != ModuleKind::Root) {
            log.devPanic("ModuleTreeBuilder top module is not of kind Root");
        }

        sess->defStorage = std::move(defStorage);
        sess->modTreeRoot = std::move(mod);

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
        enterMod(
            _enum.name.unwrap()->getValue(),
            _enum.name.span(),
            define(DefKind::Enum, _enum.name, DefKind::Enum, _enum.span)
        );
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        define(Namespace::Type, enumEntry.name, defStorage.define(DefKind::Variant, enumEntry.span));
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        const auto defId = defStorage.define(DefKind::Func, func.span);
        define(Namespace::Item, func.name, defId);
        enterMod(
            func.name.unwrap()->getValue(),
            func.name.span(),
            defId
        );
        // Note: Here, we only need function body to visit and do not enter module because body is a Block expression
        if (func.body) {
            func.body.unwrap().accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        const auto defId = defStorage.define(DefKind::Impl, impl.span);
        enterAnonMod(impl.id, defId);
        visitEach(impl.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        const auto defId = defStorage.define(DefKind::Mod, mod.span);
        define(Namespace::Type, mod.name, defId);
        enterMod(
            mod.name.unwrap()->getValue(),
            mod.name.span(),
            defId
        );
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        define(Namespace::Type, _struct.name, defStorage.define(DefKind::Struct, _struct.span));
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        const auto defId = defStorage.define(DefKind::Trait, trait.span);
        define(Namespace::Type, trait.name, defId);
        enterMod(
            trait.name.unwrap()->getValue(),
            trait.name.span(),
            defId
        );
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        define(Namespace::Type, typeAlias.name, defStorage.define(DefKind::TypeAlias, typeAlias.span));
        typeAlias.type.accept(*this);
    }

    void ModuleTreeBuilder::visit(const ast::Block & block) {
        if (block.blockKind == ast::BlockKind::OneLine) {
            // Note: One line block does not open module,
            //  anyway it can contain another Block which could be a multi-line
            block.oneLine.unwrap().accept(*this);
        } else {
            // Note: Block is not a definition, it is a pure anonymous module
            enterAnonMod(block.id, dt::None);
            visitEach(block.stmts.unwrap());
            exitMod();
        }
    }

    // Definitions //
    void ModuleTreeBuilder::defineGenerics(const ast::opt_type_params & maybeGenerics) {
        if (!maybeGenerics) {
            return;
        }
        // FIXME: USE ONE LOOP!!!
        const auto & typeParams = maybeGenerics.unwrap();
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Type) {
                define(
                    std::static_pointer_cast<ast::GenericType>(typeParam)->name.unwrap()->getValue(),
                    DefKind::TypeParam,
                    typeParam->id);
            }
        }
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Lifetime) {
                define(
                    std::static_pointer_cast<ast::Lifetime>(typeParam)->name.unwrap()->getValue(),
                    DefKind::Lifetime,
                    typeParam->id);
            }
        }
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Const) {
                define(
                    std::static_pointer_cast<ast::ConstParam>(typeParam)->name.unwrap()->getValue(),
                    DefKind::ConstParam,
                    typeParam->id);
            }
        }
    }

    // Modules //
    void ModuleTreeBuilder::enterAnonMod(node_id nodeId, dt::Option<def_id> defId) {
        if (utils::map::has(mod->anonBlocks, nodeId)) {
            log.devPanic("Tried to redeclare anonymous block");
        }
        auto child = std::make_shared<Module>(ModuleKind::Block, mod);
        child->defId = defId;
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
