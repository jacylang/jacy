#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        mod = Module::newWrapperModule(ModuleKind::Root, dt::None);

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        sess->defStorage = std::move(_defStorage);
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
        enterModule(_enum.name, DefKind::Enum);
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        addDef(enumEntry.name, DefKind::Variant);
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        // Note: Don't confuse Func module with its body,
        //  Func module stores type parameters but body is a nested block
        enterModule(func.name, DefKind::Func);
        if (func.body) {
            func.body.unwrap().accept(*this);
        }
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        // Note: Impl is a block and it will be bound to some type in NameResolver
        enterBlock(impl.id);
        visitEach(impl.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterModule(mod.name, DefKind::Mod);
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        addDef(_struct.name, DefKind::Struct);
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterModule(trait.name, DefKind::Trait);
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        addDef(typeAlias.name, DefKind::TypeAlias);
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
    def_id ModuleTreeBuilder::addDef(const ast::id_ptr & ident, DefKind defKind) {
        const auto & name = ident.unwrap()->getValue();
        const auto defId = _defStorage.define(defKind, ident.span(), ident.unwrap()->id);
        const auto ns = Def::getNS(_defStorage.getDef(defId).kind);

        log.dev(
            "Trying to add def '",
            name,
            "'",
            (curModuleName ? " in module '" + curModuleName.unwrap() + "'" : ""),
            " with defId [",
            defId,
            "] in ",
            Module::nsToString(ns));

        auto & nsMap = mod->getNS(ns);

        // Try to emplace definition in namespace, and if it is already defined suggest an error
        const auto & defined = nsMap.emplace(name, defId);
        if (not defined.second) {
            log.dev(
                "Tried to redefine '",
                name,
                "' as ",
                Def::kindStr(defKind),
                ", previously defined with id #",
                defined.first->second);
            suggestCannotRedefine(ident, defKind, defined.first->second);
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

    void ModuleTreeBuilder::defineGenerics(const ast::opt_gen_params & maybeGenerics) {
        maybeGenerics.then([&](const ast::gen_param_list & generics) {
            for (const auto & gen : generics) {
                switch (gen->kind) {
                    case ast::GenericParamKind::Type: {
                        addDef(std::static_pointer_cast<ast::TypeParam>(gen)->name, DefKind::TypeParam);
                        break;
                    }
                    case ast::GenericParamKind::Const: {
                        addDef(std::static_pointer_cast<ast::ConstParam>(gen)->name, DefKind::ConstParam);
                        break;
                    }
                    case ast::GenericParamKind::Lifetime: {
                        addDef(std::static_pointer_cast<ast::Lifetime>(gen)->name, DefKind::Lifetime);
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
        enterChildModule(_defStorage.addBlock(nodeId, Module::newBlockModule(nodeId, mod)));

        // For debug //
        curModuleName = dt::None;
    }

    /// Enters named module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(const ast::id_ptr & ident, DefKind defKind) {
        const auto defId = addDef(ident, defKind);
        const auto & name = ident.unwrap()->getValue();
        log.dev("Enter module '", name, "' defined with id #", defId);
        enterChildModule(_defStorage.addModule(defId, std::make_shared<Module>(defId, mod)));

        // For debug //
        curModuleName = name;
    }

    void ModuleTreeBuilder::enterFictiveModule(const std::string & name, DefKind defKind) {
        curModuleName = name;

        log.dev("Enter fictive module '", name, "' ", Def::kindStr(defKind));
        const auto moduleDefId = _defStorage.define(defKind, dt::None, dt::None);
        auto child = _defStorage.addModule(moduleDefId, std::make_shared<Module>(ModuleKind::Fictive, mod));
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
        log.dev("Exit ", mod->kindStr(), " module");
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit root module");
    }

    // Suggestions //
    void ModuleTreeBuilder::suggestCannotRedefine(
        const ast::id_ptr & ident,
        DefKind as,
        def_id prevDefId
    ) {
        const auto & prevDef = _defStorage.getDef(prevDefId);
        const auto & prevDefSpan = sess->nodeMap.getNodeSpan(prevDef.nameNodeId.unwrap());
        suggest(
            std::make_unique<sugg::MsgSpanLinkSugg>(
                "Cannot redeclare '" + ident.unwrap()->getValue() + "' as " + Def::kindStr(as),
                ident.span(),
                "Because it is already declared as " + prevDef.kindStr() + " here",
                prevDefSpan,
                sugg::SuggKind::Error));
    }
}
