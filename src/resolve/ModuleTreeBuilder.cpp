#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        const auto & rootModuleDef = _defStorage.define(_modDepth, DefVis::Pub, DefKind::Mod, None, None);
        assert(rootModuleDef == ROOT_DEF_ID);
        auto rootModule = std::make_shared<Module>(ModuleKind::Def, None, None, ROOT_DEF_ID, ROOT_DEF_ID);
        mod = _defStorage.addModule(ROOT_DEF_ID, rootModule);

        visitEach(party.items);

        sess->defStorage = std::move(_defStorage);
        sess->modTreeRoot = std::move(mod);

        return {None, extractSuggestions()};
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
        if (func.body.some()) {
            func.body.unwrap().value.autoAccept(*this);
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
            type.autoAccept(*this);
        });
    }

    void ModuleTreeBuilder::visit(const ast::UseDecl & useDecl) {
        _defStorage.setUseDeclModule(useDecl.id, mod);
    }

    void ModuleTreeBuilder::visit(const ast::Block & block) {
        enterBlock(block.id);
        visitEach(block.stmts);
        exitMod();
    }

    // Definitions //
    DefVis ModuleTreeBuilder::getItemVis(const ast::Item & item) {
        switch (item.vis.kind) {
            case ast::VisKind::Pub: return DefVis::Pub;
            case ast::VisKind::Unset: return DefVis::Unset;
        }
    }

    /// Adds definition by name to specific namespace determined by DefKind in current module
    DefId ModuleTreeBuilder::addDef(DefVis vis, const ast::ident_pr & ident, DefKind defKind) {
        const auto & name = ident.unwrap().name;
        const auto defId = _defStorage.define(_modDepth, vis, defKind, ident.span(), ident.unwrap().id);
        const auto ns = Def::getNS(_defStorage.getDef(defId).kind);

        log.dev(
            "Trying to add def '",
            name,
            "'",
            (curModuleName.some() ? " in module '" + curModuleName.unwrap() + "'" : ""),
            " with defId [",
            defId,
            "] in ",
            Module::nsToString(ns),
            " namespace");

        // Try to emplace definition in namespace, and if it is already defined suggest an error
        const auto & oldDefId = mod->tryDefine(ns, name, defId);
        if (oldDefId.some()) {
            log.dev(
                "Tried to redefine '",
                name,
                "' as ",
                Def::kindStr(defKind),
                ", previously defined with id #",
                oldDefId.unwrap());
            suggestCannotRedefine(ident, defKind, oldDefId.unwrap());
        }

        // If type is defined then check if its name shadows one of primitive types
        if (ns == Namespace::Type) {
            // TODO: Add warning suggestion
            const auto maybePrimType = getPrimTypeBitMask(name);
            if (maybePrimType.some()) {
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
                        addDef(DefVis::Pub, ast::Node::cast<ast::TypeParam>(gen.get())->name, DefKind::TypeParam);
                        break;
                    }
                    case ast::GenericParamKind::Const: {
                        addDef(DefVis::Pub, ast::Node::cast<ast::ConstParam>(gen.get())->name, DefKind::ConstParam);
                        break;
                    }
                    case ast::GenericParamKind::Lifetime: {
                        addDef(DefVis::Pub, ast::Node::cast<ast::Lifetime>(gen.get())->name, DefKind::Lifetime);
                        break;
                    }
                }
            }
        });
    }

    // Modules //

    /// Enter anonymous module (block) and adds it to DefStorage by nodeId
    void ModuleTreeBuilder::enterBlock(NodeId nodeId) {
        log.dev("Enter [BLOCK] module ", nodeId);
        enterChildModule(_defStorage.addBlock(nodeId, Module::newBlockModule(nodeId, mod, nearestModDef)));

        // For debug //
        curModuleName = None;
    }

    /// Enters named module, defines it in current module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(DefVis vis, const ast::ident_pr & ident, DefKind defKind) {
        const auto defId = addDef(vis, ident, defKind);
        const auto & name = ident.unwrap().name;
        log.dev("Enter [DEF] module '", name, "' defined with id #", defId);

        // We entered a new `mod`, so update `nearestModDef`
        if (defKind == DefKind::Mod) {
            nearestModDef = defId;
        }

        enterChildModule(_defStorage.addModule(defId, Module::newDefModule(defId, mod, nearestModDef)));

        // For debug //
        curModuleName = name;
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
        const ast::ident_pr & ident,
        DefKind as,
        const DefId & prevDefId
    ) {
        const auto & prevDef = _defStorage.getDef(prevDefId);
        const auto & prevDefSpan = sess->nodeStorage.getNodeSpan(prevDef.nameNodeId.unwrap());
        suggest(
            std::make_unique<sugg::MsgSpanLinkSugg>(
                "Cannot redeclare '" + ident.unwrap().name + "' as " + Def::kindStr(as),
                ident.span(),
                "Because it is already declared as " + prevDef.kindStr() + " here",
                prevDefSpan,
                sugg::SuggKind::Error));
    }
}
