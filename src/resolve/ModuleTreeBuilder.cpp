#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        mod = Module::newRootModule();

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
        enterModule(getItemVis(_enum), _enum.name, DefKind::Enum);
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        // Note: Enum variants are always public
        addDef(DefVis::Pub, enumEntry.name, DefKind::Variant);
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        // Note: Don't confuse Func module with its body,
        //  Func module stores type parameters but body is a nested block
        enterModule(getItemVis(func), func.name, DefKind::Func);
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
        enterModule(getItemVis(mod), mod.name, DefKind::Mod);
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        addDef(getItemVis(_struct), _struct.name, DefKind::Struct);
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterModule(getItemVis(trait), trait.name, DefKind::Trait);
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        addDef(getItemVis(typeAlias), typeAlias.name, DefKind::TypeAlias);

        typeAlias.type.then([&](const auto & type) {
            type.accept(*this);
        });
    }

    void ModuleTreeBuilder::visit(const ast::UseDecl & useDecl) {
        _defStorage.setUseDeclModule(useDecl.id, mod);
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
    DefVis ModuleTreeBuilder::getItemVis(const ast::Item & item) {
        switch (item.vis.kind) {
            case ast::VisKind::Pub: return DefVis::Pub;
            case ast::VisKind::Unset: return DefVis::Unset;
        }
    }

    /// Adds definition by name to specific namespace determined by DefKind in current module
    def_id ModuleTreeBuilder::addDef(DefVis vis, const ast::id_ptr & ident, DefKind defKind) {
        const auto & name = ident.unwrap()->getValue();
        const auto defId = _defStorage.define(_modDepth, vis, defKind, ident.span(), ident.unwrap()->id);
        const auto ns = Def::getNS(_defStorage.getDef(defId).kind);

        log.dev(
            "Trying to add def '",
            name,
            "'",
            (curModuleName ? " in module '" + curModuleName.unwrap() + "'" : ""),
            " with defId [",
            defId,
            "] in ",
            Module::nsToString(ns),
            " namespace");

        // Try to emplace definition in namespace, and if it is already defined suggest an error
        const auto & oldDefId = mod->tryDefine(ns, name, defId);
        if (not oldDefId) {
            log.dev(
                "Tried to redefine '",
                name,
                "' as ",
                Def::kindStr(defKind),
                ", previously defined with id #",
                oldDefId);
            suggestCannotRedefine(ident, defKind, oldDefId);
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
                        addDef(DefVis::Pub, std::static_pointer_cast<ast::TypeParam>(gen)->name, DefKind::TypeParam);
                        break;
                    }
                    case ast::GenericParamKind::Const: {
                        addDef(DefVis::Pub, std::static_pointer_cast<ast::ConstParam>(gen)->name, DefKind::ConstParam);
                        break;
                    }
                    case ast::GenericParamKind::Lifetime: {
                        addDef(DefVis::Pub, std::static_pointer_cast<ast::Lifetime>(gen)->name, DefKind::Lifetime);
                        break;
                    }
                }
            }
        });
    }

    // Modules //

    /// Enter anonymous module (block) and adds it to DefStorage by nodeId
    void ModuleTreeBuilder::enterBlock(node_id nodeId) {
        log.dev("Enter [BLOCK] module #", nodeId);
        enterChildModule(_defStorage.addBlock(nodeId, Module::newBlockModule(nodeId, mod, nearestModDef)));

        // For debug //
        curModuleName = dt::None;
    }

    /// Enters named module, defines it in current module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(DefVis vis, const ast::id_ptr & ident, DefKind defKind) {
        const auto defId = addDef(vis, ident, defKind);
        const auto & name = ident.unwrap()->getValue();
        log.dev("Enter [DEF] module '", name, "' defined with id #", defId);

        // We entered a new `mod`, so update `nearestModDef`
        if (defKind == DefKind::Mod) {
            nearestModDef = defId;
        }

        enterChildModule(_defStorage.addModule(defId, Module::newDefModule(defId, mod, nearestModDef)));

        // For debug //
        curModuleName = name;
    }

    void ModuleTreeBuilder::enterFictiveModule(const std::string & name, DefKind defKind) {
        curModuleName = name;

        log.dev("Enter [FICTIVE] module '", name, "' ", Def::kindStr(defKind));

        // Note: Fictive modules are always public
        const auto defId = _defStorage.define(_modDepth, DefVis::Pub, defKind, dt::None, dt::None);
        auto child = _defStorage.addModule(
            defId, Module::newFictiveModule(ModuleKind::Fictive, mod, defId)
        );
        if (utils::map::has(mod->perNS.type, name)) {
            log.devPanic("Tried to redefine fictive module '", name, "'");
        }
        mod->perNS.type.emplace(name, defId);

        enterChildModule(child);
    }

    void ModuleTreeBuilder::enterChildModule(module_ptr child) {
        child->shadowedPrimTypes = mod->shadowedPrimTypes;
        mod = child;
        if (_modDepth == UINT32_MAX) {
            log.devPanic("Exceeded definitions depth limit");
        }
        _modDepth++;
    }

    void ModuleTreeBuilder::exitMod() {
        log.dev("Exit ", mod->kindStr(), " module");
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit root module");

        // Set nearest `mod` from parent we lift to
        nearestModDef = mod->nearestModDef;
        _modDepth--;
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
