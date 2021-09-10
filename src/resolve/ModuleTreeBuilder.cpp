#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> ModuleTreeBuilder::build(sess::Session::Ptr sess, const ast::Party & party) {
        this->sess = sess;

        // Enter root module
        // Note: Maybe define ROOT_NODE_ID?
        auto rootModuleDef = _defTable.define(
            DefVis::Pub,
            ast::NodeId::DUMMY,
            DefKind::Mod,
            span::Ident::empty()
        );

        assert(rootModuleDef == DefId::ROOT_DEF_ID);

        auto rootModule = std::make_shared<Module>(ModuleKind::Def, None, DefId::ROOT_DEF_ID, DefId::ROOT_DEF_ID);
        mod = _defTable.addModule(DefId::ROOT_DEF_ID, rootModule);

        visitEach(party.items);

        sess->defTable = std::move(_defTable);
        sess->modTreeRoot = std::move(mod);

        return {None, extractSuggestions()};
    }

    void ModuleTreeBuilder::visit(const ast::Enum & _enum) {
        enterModule(getItemVis(_enum), _enum.id, DefKind::Enum, _enum.name.unwrap());
        visitEach(_enum.entries);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::EnumEntry & enumEntry) {
        // Note: Enum variants are always public
        addDef(DefVis::Pub, enumEntry.id, DefKind::Variant, enumEntry.name.unwrap());
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        // Note: Don't confuse Func module with its body,
        //  Func module stores type parameters but body is a nested block
        enterModule(
            getItemVis(func),
            func.id,
            DefKind::Func,
            Module::getFuncName(func.name.unwrap().sym.toString(), func.sig, func.name.unwrap().span)
        );

        if (func.body.some()) {
            func.body.unwrap().value.autoAccept(*this);
        }

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        auto synthName = span::Ident {Module::getImplName(impl), impl.span.fromStartWithLen(4)};

        enterModule(getItemVis(impl), impl.id, DefKind::Impl, synthName);
        visitEach(impl.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterModule(getItemVis(mod), mod.id, DefKind::Mod, mod.name.unwrap());
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        addDef(getItemVis(_struct), _struct.id, DefKind::Struct, _struct.name.unwrap());
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterModule(getItemVis(trait), trait.id, DefKind::Trait, trait.name.unwrap());
        visitEach(trait.members);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::TypeAlias & typeAlias) {
        addDef(getItemVis(typeAlias), typeAlias.id, DefKind::TypeAlias, typeAlias.name.unwrap());

        typeAlias.type.then([&](const auto & type) {
            type.autoAccept(*this);
        });
    }

    void ModuleTreeBuilder::visit(const ast::UseDecl & useDecl) {
        _defTable.setUseDeclModule(useDecl.id, mod);
    }

    void ModuleTreeBuilder::visit(const ast::Init & init) {
        // `Init` has pretty same logic as `Func`, for help look at `Func` visitor
        enterModule(getItemVis(init), init.id, DefKind::Init, Module::getInitName(init));

        if (init.body.some()) {
            init.body.unwrap().value.autoAccept(*this);
        }

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Block & block) {
        enterBlock(block.id);
        visitEach(block.stmts);
        exitMod();
    }

    // Definitions //
    DefVis ModuleTreeBuilder::getItemVis(const ast::Item & item) {
        switch (item.vis.kind) {
            case ast::VisKind::Pub:
                return DefVis::Pub;
            case ast::VisKind::Unset:
                return DefVis::Unset;
        }
    }

    /// Adds definition by name to specific namespace determined by DefKind in current module
    DefId ModuleTreeBuilder::addDefCommon(
        DefId defId,
        DefKind defKind,
        const span::Ident & ident
    ) {
        const auto & name = ident.sym;
        const auto ns = Def::getNS(_defTable.getDef(defId).kind);

        log.dev(
            "Trying to add def '",
            name,
            "'",
            " in module '",
            (moduleNameStack.size() > 0 ? moduleNameStack.back() : "[ROOT]"),
            "' with defId [",
            defId,
            "] in ",
            Module::nsToString(ns),
            " namespace"
        );

        // Try to emplace definition in namespace, and if it is already defined suggest an error
        const auto & oldDef = mod->tryDefine(ns, name, defId);
        if (oldDef.some()) {
            log.dev(
                "Tried to redefine '",
                name,
                "' as ",
                Def::kindStr(defKind),
                ", previously defined with id ",
                oldDef.unwrap()
            );
            suggestCannotRedefine(ident, defKind, oldDef.unwrap());
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

    DefId ModuleTreeBuilder::addDef(DefVis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident) {
        auto defId = _defTable.define(vis, nodeId, defKind, ident);
        return addDefCommon(defId, vis, nodeId, defKind, ident);
    }

    DefId ModuleTreeBuilder::addFuncDef(DefVis vis, NodeId nodeId, const span::Ident & ident) {
        auto defId = _defTable.defineFunc(vis, nodeId, )
    }

    void ModuleTreeBuilder::defineGenerics(const ast::GenericParam::OptList & maybeGenerics) {
        maybeGenerics.then([&](const ast::GenericParam::List & generics) {
            for (const auto & gen: generics) {
                switch (gen->kind) {
                    case ast::GenericParamKind::Type: {
                        const auto & typeParam = ast::Node::cast<ast::TypeParam>(gen.get());
                        addDef(DefVis::Pub, typeParam->id, DefKind::TypeParam, typeParam->name.unwrap());
                        break;
                    }
                    case ast::GenericParamKind::Const: {
                        const auto & constParam = ast::Node::cast<ast::ConstParam>(gen.get());
                        addDef(DefVis::Pub, constParam->id, DefKind::ConstParam, constParam->name.unwrap());
                        break;
                    }
                    case ast::GenericParamKind::Lifetime: {
                        const auto & lifetimeParam = ast::Node::cast<ast::Lifetime>(gen.get());
                        addDef(DefVis::Pub, lifetimeParam->id, DefKind::Lifetime, lifetimeParam->name.unwrap());
                        break;
                    }
                }
            }
        });
    }

    // Modules //

    /// Important!: For API consistency -- keep two ends of module methods for entering and exiting.
    ///  Much work is done on enter/exit, thus to avoid mistakes use `enterChildModule` and `exitMod`
    ///  in custom aliases

    /// Enter anonymous module (block) and adds it to DefStorage by nodeId
    void ModuleTreeBuilder::enterBlock(NodeId nodeId) {
        log.dev("Enter [BLOCK] module ", nodeId);
        enterChildModule(_defTable.addBlock(nodeId, Module::newBlockModule(nodeId, mod, nearestModDef)));

        // For debug //
        moduleNameStack.push_back("[BLOCK]");
    }

    /// Enters named module, defines it in current module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(DefVis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident) {
        const auto defId = addDef(vis, nodeId, defKind, ident);
        const auto & name = ident.sym;
        log.dev("Enter [DEF] module '", name, "' defined with id ", defId);

        // We entered a new `mod`, so update `nearestModDef`
        if (defKind == DefKind::Mod) {
            nearestModDef = defId;
        }

        enterChildModule(_defTable.addModule(defId, Module::newDefModule(defId, mod, nearestModDef)));

        // For debug //
        moduleNameStack.push_back(name.toString());
    }

    void ModuleTreeBuilder::enterChildModule(Module::Ptr child) {
        child->shadowedPrimTypes = mod->shadowedPrimTypes;
        mod = child;
    }

    void ModuleTreeBuilder::exitMod() {
        log.dev("Exit ", mod->kindStr(), " module");
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit root module");

        // Set nearest `mod` from parent we lift to
        nearestModDef = mod->nearestModDef;

        moduleNameStack.pop_back();
    }

    // Suggestions //
    void ModuleTreeBuilder::suggestCannotRedefine(
        const span::Ident & ident,
        DefKind as,
        const IntraModuleDef & prevDefId
    ) {
        const auto & prevDef = _defTable.getDef(prevDefId);

        // Note: The only things we can redefine are obviously "named" things,
        //  thus if name span found -- it is a bug
        const auto & prevDefSpan = _defTable.getDefNameSpan(prevDefId);
        suggest(std::make_unique<sugg::MsgSpanLinkSugg>(
            log::fmt("Cannot redeclare '", ident.sym, "' as ", Def::kindStr(as)),
            ident.span,
            "Because it is already declared as " + prevDef.kindStr() + " here",
            prevDefSpan,
            sugg::SuggKind::Error
        ));
    }
}
