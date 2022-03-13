#include "resolve/ModuleTreeBuilder.h"

namespace jc::resolve {
    message::MessageResult<dt::none_t> ModuleTreeBuilder::build(
        sess::Session::Ptr sess,
        const ast::Party & party
    ) {
        this->sess = sess;

        // Enter root module
        // Note: Maybe define ROOT_NODE_ID?
        auto rootModuleDef = _defTable.define(
            Vis::Pub,
            ast::NodeId::ROOT_NODE_ID,
            DefKind::Mod,
            span::Ident {span::Symbol::fromKw(span::Kw::Root), span::Span {}}
        );

        assert(rootModuleDef == DefId::ROOT_DEF_ID);

        auto rootModule = std::make_shared<Module>(ModuleKind::Def, None, DefId::ROOT_DEF_ID, DefId::ROOT_DEF_ID);
        mod = _defTable.addModule(DefId::ROOT_DEF_ID, rootModule);

        visitEach(party.items);

        sess->defTable = std::move(_defTable);
        sess->modTreeRoot = std::move(mod);

        return {None, msg.extractMessages()};
    }

    void ModuleTreeBuilder::visit(const ast::Const & constItem) {
        addDef(getItemVis(constItem), constItem.id, DefKind::Const, constItem.name.unwrap());
    }

    void ModuleTreeBuilder::visit(const ast::Enum & _enum) {
        enterModule(getItemVis(_enum), _enum.id, DefKind::Enum, _enum.name.unwrap());
        visitEach(_enum.variants);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Variant & variant) {
        // Note: Enum variants are always public
        addDef(Vis::Pub, variant.id, DefKind::Variant, variant.name.unwrap());
    }

    void ModuleTreeBuilder::visit(const ast::Func & func) {
        // Note: Don't confuse Func module with its body,
        //  Func module stores type parameters but body is a nested block
        enterFuncModule(func, func.sig, DefKind::Func);

        defineGenerics(func.generics);

        if (func.body.some()) {
            func.body.unwrap().value.autoAccept(*this);
        }

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Impl & impl) {
        auto synthName = span::Ident {Module::getImplName(impl), impl.span.fromStartWithLen(4)};

        enterModule(getItemVis(impl), impl.id, DefKind::Impl, synthName);

        defineGenerics(impl.generics);

        visitEach(impl.members);

        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Mod & mod) {
        enterModule(getItemVis(mod), mod.id, DefKind::Mod, mod.name.unwrap());
        visitEach(mod.items);
        exitMod();
    }

    void ModuleTreeBuilder::visit(const ast::Struct & _struct) {
        // Add default initializer definition
        // Struct name identifier span used as `init` span
        addFuncDef(
            Vis::Pub,
            _struct.name.nodeId(),
            DefKind::DefaultInit,
            _struct.getName(),
            Module::getStructDefaultInitSuffix(_struct.fields)
        );

        enterModule(getItemVis(_struct), _struct.id, DefKind::Struct, _struct.name.unwrap());
        defineGenerics(_struct.generics);
        exitMod();
        // Note: We only need to declare a struct as far as it does not contain assoc items
    }

    void ModuleTreeBuilder::visit(const ast::Trait & trait) {
        enterModule(getItemVis(trait), trait.id, DefKind::Trait, trait.name.unwrap());
        defineGenerics(trait.generics);

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
        enterFuncModule(init, init.sig, DefKind::Init);

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
    Vis ModuleTreeBuilder::getItemVis(const ast::Item & item) {
        switch (item.vis.kind) {
            case ast::VisKind::Pub:
                return Vis::Pub;
            case ast::VisKind::Unset:
                return Vis::Unset;
        }
    }

    /// Adds definition by name to specific namespace determined by DefKind in current module
    DefId ModuleTreeBuilder::addDef(Vis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident) {
        auto defId = _defTable.define(vis, nodeId, defKind, ident);

        const auto & name = ident.sym;
        const auto & ns = Def::getDefKindNS(_defTable.getDef(defId).kind);

        log.dev(
            "Trying to add def '",
            name,
            "'",
            " in module '",
            (moduleNameStack.size() > 0 ? moduleNameStack.back() : "[ROOT]"),
            "' with defId [",
            defId,
            "] in ",
            nsToString(ns),
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
                ", previously defined as ",
                oldDef.unwrap()
            );
            reportCannotRedefine(ident, defKind, oldDef.unwrap());
        }

        return defId;
    }

    DefId ModuleTreeBuilder::addFuncDef(
        Vis vis,
        NodeId nodeId,
        DefKind defKind,
        const span::Ident & baseName,
        Symbol suffix
    ) {
        // Note: We only define functions as single overloading, never as a name (such as single defId for each `init`)
        // In overload definition, name contains both base name (like `foo`) and suffix (like `(label1:label2:...)`)
        auto defId = _defTable.define(vis, nodeId, defKind, Def::getFuncIdent(baseName, suffix));

        // Trying to find overloading by base name (for `func foo(...)` it would be `foo` without labels)
        auto nameBinding = mod->find(Namespace::Value, baseName.sym);

        // Here we check if name already exists in module and not a function overload base name.
        // It means that some non-function definition already uses this name.
        if (nameBinding.some() and not nameBinding.unwrap().isFOS()) {
            // TODO: Maybe add separate `suggestCannotRedefineFunc`?
            // Note!!!: If new `nameBinding::Kind`'s will be added, don't use `asDef`!
            reportCannotRedefine(baseName, defKind, nameBinding.unwrap(), suffix);
            return defId;
        }

        FOSId::Opt overloadId = None;
        if (nameBinding.some()) {
            // Note: It is a bug to have not a func overloading here, due to check above
            overloadId = nameBinding.unwrap().asFOS();
        }

        // Create new overload if no exists in current module.
        // Overload name in overloads mapping only contains suffix as base name is a FuncOverloadId.
        auto newFuncRes = _defTable.tryDefineFunc(defId, overloadId, suffix);

        if (newFuncRes.err()) {
            overloadId = newFuncRes.unwrapErr().first;
            const auto & oldDef = newFuncRes.unwrapErr().second;
            log.dev("Tried to redefine function '", baseName, suffix, "' previously defined as ", oldDef);
            reportCannotRedefine(baseName, defKind, oldDef, suffix);
        } else {
            overloadId = newFuncRes.unwrap();
        }

        // Define function overload in module
        // Note!: In module, function names do not have suffixes, only base name
        auto oldDef = mod->tryDefineFOS(baseName.sym, overloadId.unwrap());
        if (oldDef.some()) {
            log.dev("Tried to redefine function '", baseName, "' previously defined as ", oldDef.unwrap());
            reportCannotRedefine(baseName, defKind, oldDef.unwrap(), suffix);
        }

        return defId;
    }

    void ModuleTreeBuilder::defineGenerics(const ast::GenericParam::OptList & maybeGenerics) {
        maybeGenerics.then([&](const ast::GenericParam::List & generics) {
            for (const auto & p : generics) {
                const auto & param = p.unwrap();
                switch (param.kind) {
                    case ast::GenericParam::Kind::Type: {
                        const auto & typeParam = param.getTypeParam();
                        addDef(Vis::Pub, param.id, DefKind::TypeParam, typeParam.name.unwrap());
                        break;
                    }
                    case ast::GenericParam::Kind::Lifetime: {
                        const auto & lifetimeParam = param.getLifetime();
                        addDef(Vis::Pub, param.id, DefKind::Lifetime, lifetimeParam.name.unwrap());
                        break;
                    }
                    case ast::GenericParam::Kind::Const: {
                        const auto & constParam = param.getConstParam();
                        addDef(Vis::Pub, param.id, DefKind::ConstParam, constParam.name.unwrap());
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
        enterChildModule("[BLOCK]", _defTable.addBlock(nodeId, Module::newBlockModule(nodeId, mod, nearestModDef)));
    }

    /// Enters named module, defines it in current module and adds module to DefStorage by defId
    void ModuleTreeBuilder::enterModule(Vis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident) {
        const auto defId = addDef(vis, nodeId, defKind, ident);
        const auto & name = ident.sym;
        log.dev("Enter [DEF] module '", name, "' defined with id ", defId);

        // We entered a new `mod`, so update `nearestModDef`
        if (defKind == DefKind::Mod) {
            nearestModDef = defId;
        }

        enterChildModule(name.toString(), _defTable.addModule(defId, Module::newDefModule(defId, mod, nearestModDef)));
    }

    void ModuleTreeBuilder::enterFuncModule(const ast::Item & funcItem, const ast::FuncSig & sig, DefKind kind) {
        auto funcDefId = addFuncDef(
            getItemVis(funcItem),
            funcItem.id,
            kind,
            funcItem.getName(),
            Module::getFuncSuffix(sig)
        );

        enterChildModule(
            funcItem.getName().sym.toString(),
            _defTable.addModule(funcDefId, Module::newDefModule(funcDefId, mod, nearestModDef))
        );
    }

    void ModuleTreeBuilder::enterChildModule(const std::string & name, Module::Ptr child) {
        child->shadowedPrimTypes = mod->shadowedPrimTypes;
        mod = child;

        // For debug //
        moduleNameStack.push_back(name);
    }

    void ModuleTreeBuilder::exitMod() {
        log.dev("Exit ", mod->kindStr(), " module");
        mod = mod->parent.unwrap("[ModuleTreeBuilder]: Tried to exit root module");

        // Set nearest `mod` from parent we lift to
        nearestModDef = mod->nearestModDef;

        moduleNameStack.pop_back();
    }

    // Suggestions //
    void ModuleTreeBuilder::reportCannotRedefine(
        const span::Ident & ident,
        DefKind as,
        const NameBinding & prevModDef,
        Symbol::Opt suffix
    ) {
        // Note: The only things we can redefine are obviously "named" things,
        //  thus if name span found -- it is a bug

        auto redefinedName = ident.sym;
        if (suffix.some()) {
            redefinedName = redefinedName + suffix.unwrap();
        }

        DefId::Opt prevDefId = None;
        if (prevModDef.isFOS()) {
            prevDefId = _defTable.getFOSFirstDef(prevModDef.asFOS());
        } else {
            prevDefId = prevModDef.asDef();
        }

        const auto & prevDefSpan = _defTable.getDefNameSpan(prevDefId.unwrap());
        const auto & prevDef = _defTable.getDef(prevDefId.unwrap());

        // TODO: Header when suggestion headers will be implemented:
        //  log::fmt("Name '", redefinedName, "' for ", Def::kindStr(as), " is conflicting")

        // TODO: Link labels
        msg.error()
           .setText("Cannot redeclare '", redefinedName, "' as ", Def::kindStr(as))
           .setPrimaryLabel(ident.span, "Cannot redeclare '", redefinedName, "' as ", Def::kindStr(as))
           .emit();

        //        report(
        //            std::make_unique<message::MsgSpanLinkSugg>(
        //                log::fmt("Cannot redeclare '", redefinedName, "' as ", Def::kindStr(as)),
        //                ident.span,
        //                "Because it is already declared as " + prevDef.kindStr() + " here",
        //                prevDefSpan,
        //                message::Level::Error
        //            )
        //        );
    }
}
