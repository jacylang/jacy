#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        mod = std::make_shared<Module>(ModuleKind::Root, dt::None);

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
        enterFictiveModule(sess->sourceMap.getSourceFile(file.fileId).filename(), DefKind::File);

        visitEach(file.items);

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Dir & dir) {
        enterFictiveModule(dir.name, DefKind::Dir);
        for (const auto & module : dir.modules) {
            module->accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Enum & _enum) {
        enterModule(_enum.name, define(_enum.name, DefKind::Enum));
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        define(enumEntry.name, DefKind::Variant);
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        // Note: Don't confuse Func module with its body,
        //  Func module stores type parameters but body is a nested block
        enterModule(func.name, define(func.name, DefKind::Func));
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
        enterModule(mod.name, define(mod.name, DefKind::Mod));
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        define(_struct.name, DefKind::Struct);
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterModule(trait.name, define(trait.name, DefKind::Trait));
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
            "'",
            (curModuleName ? " in module '" + curModuleName.unwrap() + "'" : ""),
            " with defId [",
            defId,
            "] in ",
            Module::nsToString(ns));

        auto & nsMap = mod->getNS(ns);
        const auto & defined = nsMap.emplace(name, defId);
        if (not defined.second) {
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
        curModuleName = dt::None;
        log.dev("Enter block module #", nodeId);
        auto child = defStorage.addBlock(nodeId, std::make_shared<Module>(ModuleKind::Block, mod));
        enterChildModule(child);
    }

    /// Enters named module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(const ast::id_ptr & ident, def_id defId) {
        const auto & name = ident.unwrap()->getValue();

        curModuleName = name;
        log.dev("Enter module '", name, "' defined with id #", defId);

        auto child = defStorage.addModule(defId, std::make_shared<Module>(ModuleKind::Def, mod));
        if (utils::map::has(mod->typeNS, name)) {
            suggestErrorMsg("'" + name + "' has been already declared in this scope", ident.span());
        } else {
            mod->typeNS.emplace(name, defId);
        }

        enterChildModule(child);
    }

    void ModuleTreeBuilder::enterFictiveModule(const std::string & name, DefKind defKind) {
        curModuleName = name;

        log.dev("Enter fictive module '", name, "' ", Def::kindStr(defKind));
        const auto moduleDefId = defStorage.define(defKind, dt::None, dt::None);
        auto child = defStorage.addModule(moduleDefId, std::make_shared<Module>(ModuleKind::Fictive, mod));
        if (utils::map::has(mod->typeNS, name)) {
            log.devPanic("Tried to redefine fictive module '", name, "'");
        }
        mod->typeNS.emplace(name, moduleDefId);

        enterChildModule(child);
    }

    void ModuleTreeBuilder::enterChildModule(module_ptr child) {
        child->shadowedPrimTypes = mod->shadowedPrimTypes;
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit root module");
    }

    // Suggestions //
    void ModuleTreeBuilder::suggestCannotRedefine(
        const ast::id_ptr & name,
        const std::string & as,
        const std::string & declaredAs,
        def_id defId
    ) {
        const auto prevDefSpan = sess->nodeMap.getNodeSpan(sess->defStorage.getDef(defId).nameNodeId.unwrap());
        suggest(
            std::make_unique<sugg::MsgSpanLinkSugg>(
                "Cannot redeclare '" + name.unwrap()->getValue() + "' as " + as,
                name.span(),
                "Because it is already declared as " + declaredAs + " here",
                prevDefSpan,
                sugg::SuggKind::Error));
    }
}
