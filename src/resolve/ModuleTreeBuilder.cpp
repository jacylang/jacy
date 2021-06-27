#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        mod = std::make_shared<Module>(ModuleKind::Root);

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        sess->defStorage = std::move(defStorage);
        sess->modTreeRoot = std::move(mod);

        return {dt::None, extractSuggestions()};
    }

    void ModuleTreeBuilder::visit(const ast::File & file) {
        // That is actually impossible to redeclare file, filesystem does not allow it

        // Note: Here we define file through `DefStorage`, but not through `define` method
        //  as files are not presented in any namespace
        enterModule(
            sess->sourceMap.getSourceFile(file.fileId).filename(),
            dt::None,
            defStorage.define(DefKind::File, file.span, dt::None));

        visitEach(file.items);

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Dir & dir) {
        enterModule(dir.name, dt::None, defStorage.define(DefKind::Dir, dir.span, dt::None));
        for (const auto & module : dir.modules) {
            module->accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Enum & _enum) {
        enterModule(
            _enum.name.unwrap()->getValue(), _enum.name.span(), define(_enum.name, DefKind::Enum));
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        define(enumEntry.name, DefKind::Variant);
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        enterModule(
            func.name.unwrap()->getValue(), func.name.span(), define(func.name, DefKind::Func));
        if (func.body) {
            func.body.unwrap().accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        // Note: Impl is defined as unnamed definition
        const auto defId = defStorage.define(DefKind::Impl, impl.span, dt::None);
        enterBlock(impl.id);
        visitEach(impl.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterModule(
            mod.name.unwrap()->getValue(), mod.name.span(), define(mod.name, DefKind::Mod));
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        define(_struct.name, DefKind::Struct);
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterModule(
            trait.name.unwrap()->getValue(), trait.name.span(), define(trait.name, DefKind::Trait));
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        define(typeAlias.name, DefKind::TypeAlias);
        typeAlias.type.accept(*this);
    }

    void ModuleTreeBuilder::visit(const ast::Block & block) {
        if (block.blockKind == ast::BlockKind::OneLine) {
            // Note: One line block does not open module,
            //  anyway it can contain another Block which could be a multi-line
            block.oneLine.unwrap().accept(*this);
        } else {
            // Note: Block is not a definition, it is a pure anonymous module
            enterBlock(block.id);
            visitEach(block.stmts.unwrap());
            exitMod();
        }
    }

    // Definitions //

    /// Adds definition by name to specific namespace determined by DefKind
    def_id ModuleTreeBuilder::define(const ast::id_ptr & ident, DefKind defKind) {
        const auto & name = ident.unwrap()->getValue();
        const auto defId = defStorage.define(defKind, ident.span(), ident.unwrap()->id);
        const auto ns = Def::getNS(defStorage.getDef(defId).kind);

        log.dev(
            "Define '",
            name,
            "' in module with defId [",
            defId,
            "] in ",
            Module::nsToString(ns));

        auto & map = mod->getNS(ns);
        if (utils::map::has(map, name)) {
            suggestErrorMsg("'" + name + "' has been already declared", ident.unwrap()->span);
        }

        // If type is defined then check if its name shadows one of primitive types
        if (ns == Namespace::Type) {
            // TODO: Add warning suggestion
            const auto maybePrimType = getPrimTypeBitMask(name);
            if (maybePrimType) {
                // Set primitive type shadow flag
                mod->shadowedPrimTypes |= maybePrimType.unwrap();
            }
        }

        map.emplace(name, defId);
        return defId;
    }

    void ModuleTreeBuilder::defineGenerics(const ast::opt_type_params & maybeGenerics) {
        maybeGenerics.then([&](const ast::type_param_list & generics) {
            for (const auto & gen : generics) {
                switch (gen->kind) {
                    case ast::TypeParamKind::Type: {
                        define(std::static_pointer_cast<ast::GenericType>(gen)->name, DefKind::TypeParam);
                        break;
                    }
                    case ast::TypeParamKind::Const: {
                        define(std::static_pointer_cast<ast::ConstParam>(gen)->name, DefKind::ConstParam);
                        break;
                    }
                    case ast::TypeParamKind::Lifetime: {
                        define(std::static_pointer_cast<ast::Lifetime>(gen)->name, DefKind::Lifetime);
                        break;
                    }
                }
            }
        });
    }

    // Modules //
    /// Enter anonymous module (block) and adds it to DefStorage by nodeId
    void ModuleTreeBuilder::enterBlock(node_id nodeId) {
        log.dev("Enter block module #", nodeId);
        auto child = defStorage.addBlock(nodeId, std::make_shared<Module>(ModuleKind::Block));
        enterMod(child);
    }

    /// Enters named module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(const ast::id_ptr & ident, def_id defId) {
        const auto & name = ident.unwrap()->getValue();
        log.dev("Enter module '", name, "' defined with id #", defId);
        auto child = defStorage.addModule(defId, std::make_shared<Module>(ModuleKind::Def));
        if (utils::map::has(mod->typeNS, name)) {
            suggestErrorMsg("'" + name + "' has been already declared in this scope", ident.span());
        } else {
            mod->typeNS.emplace(name, defId);
        }

        enterMod(child);
    }

    void ModuleTreeBuilder::enterMod(module_ptr child) {
        child->shadowedPrimTypes = mod->shadowedPrimTypes;
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit global scope");
    }
}
