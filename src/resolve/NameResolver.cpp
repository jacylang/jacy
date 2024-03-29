#include "resolve/NameResolver.h"

namespace jc::resolve {
    NameResolver::NameResolver() : StubVisitor("NameResolver"), config(config::Config::getInstance()) {}

    message::MessageResult<dt::none_t> NameResolver::resolve(
        const sess::Session::Ptr & sess,
        const ast::Party & party
    ) {
        this->sess = sess;
        pathResolver.init(sess);

        printRibsFlag = config::Config::getInstance().checkDevPrint(config::Config::DevPrint::Ribs);

        log::assertLogic(sess->modTreeRoot.unwrap()->parent.none(), "Root module must not have parent");

        try {
            enterRootRib();
            visitEach(party.items);
        } catch (std::exception & e) {
            log.debug("Module path on name resolution fail:\n", utils::arr::join(scopePath, " -> "));
            throw;
        }

        // Debug call to print generated rib stack output
        dumpRibs();

        sess->resolutions = std::move(_resolutions);

        // `PathResolver` has its own message collection, thus we need to extract all of them
        return {None, utils::arr::moveConcat(msg.extractMessages(), pathResolver.extractMessages())};
    }

    void NameResolver::visit(const ast::Func & func) {
        enterFuncModule(func.name.unwrap().sym, Module::getFuncSuffix(func.sig));

        if (func.sig.returnType.isSome()) {
            func.sig.returnType.asSome().autoAccept(*this);
        }

        enterRib(); // -> (params) rib

        for (const auto & param : func.sig.params) {
            /// Order matters, we resolve type first, then default value
            ///  to disallow referencing pattern identifier in default value

            param.type.autoAccept(*this);

            if (param.defaultValue.some()) {
                param.defaultValue.unwrap().accept(*this);
            }

            param.pat.autoAccept(*this);
        }

        if (func.body.some()) {
            func.body.unwrap().value.autoAccept(*this);
        }

        exitRib(); // <- (params) rib

        exitRib(); // <- `func` mod rib
    }

    void NameResolver::visit(const ast::Impl & impl) {
        impl.trait.then([&](const auto & traitRef) {
            traitRef.autoAccept(*this);
        });

        impl.forType.autoAccept(*this);

        enterModule(Module::getImplName(impl), Namespace::Type); // -> `impl` mod
        visitEach(impl.members);
        exitRib(); // <- `impl` mod
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterModule(mod.getName().sym); // -> 'mod` mod
        visitEach(mod.items);
        exitRib(); // <- `mod` mod, mod mod mod mod mod
    }

    void NameResolver::visit(const ast::Struct & st) {
        enterModule(st.getName().sym); // -> `struct` mod
        for (const auto & field : st.fields) {
            field.node.autoAccept(*this);
        }
        exitRib(); // <- `struct` mod
    }

    void NameResolver::visit(const ast::Trait & trait) {
        enterModule(trait.getName().sym);
        visitEach(trait.superTraits);
        visitEach(trait.members);
        exitRib();
    }

    void NameResolver::visit(const ast::Init & init) {
        enterFuncModule(span::Symbol::fromKw(span::Kw::Init), Module::getFuncSuffix(init.sig));

        if (init.sig.returnType.isSome()) {
            init.sig.returnType.asSome().autoAccept(*this);
        }

        enterRib(); // -> (params) rib

        for (const auto & param : init.sig.params) {
            param.pat.autoAccept(*this);
            if (param.defaultValue.some()) {
                param.defaultValue.unwrap().accept(*this);
            }
        }

        if (init.body.some()) {
            init.body.unwrap().value.autoAccept(*this);
        }

        exitRib(); // <- (params) rib

        exitRib(); // <- `func` mod rib
    }

    // Statements //
    void NameResolver::visit(const ast::LetStmt & letStmt) {
        enterRib();

        if (letStmt.type.some()) {
            letStmt.type.unwrap().autoAccept(*this);
        }

        if (letStmt.assignExpr.some()) {
            letStmt.assignExpr.unwrap().autoAccept(*this);
        }

        // Note: `let a = a;` must fail because assignment is right-associative.
        //  Keep this visitor after `assignExpr` visitor!.
        letStmt.pat.autoAccept(*this);
    }

    // Expressions //
    void NameResolver::visit(const ast::Block & block) {
        const auto prevDepth = getDepth();
        enterBlock(block.id); // -> block rib
        for (const auto & stmt : block.stmts) {
            stmt.autoAccept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Lambda & lambda) {
        enterRib(); // -> (lambda params)

        for (const auto & param : lambda.params) {
            param.pat.autoAccept(*this);
            if (param.type.some()) {
                param.type.unwrap().autoAccept(*this);
            }
        }

        if (lambda.returnType.some()) {
            lambda.returnType.unwrap().autoAccept(*this);
        }

        lambda.body.autoAccept(*this);

        exitRib(); // <- (lambda params)
    }

    void NameResolver::visit(const ast::PathExpr & pathExpr) {
        // Note!!!: PathExpr MUST BE visited only in case of it is a part of an expression.
        //  For example, `StructExpr` must call `resolvePath` itself, but not visit its path
        //  Every Node that uses `PathExpr` not as "always from value namespace" must resolve path itself!
        resolvePath(Namespace::Value, pathExpr.path);
    }

    void NameResolver::visit(const ast::MatchArm & arm) {
        arm.pat.autoAccept(*this);
        arm.body.autoAccept(*this);
    }

    void NameResolver::visit(const ast::Invoke & invoke) {
        /// Function invocation has a specific name resolution workflow
        /// When function is called by path, i.e. `path::to::func(...)`,
        ///  we need to resolve it relying on specified labels

        auto[gotLabels, suffix] = Module::getCallSuffix(invoke.args);

        if (invoke.lhs.unwrap()->kind == ast::Expr::Kind::Path) {
            const auto & pathExpr = ast::Expr::as<ast::PathExpr>(invoke.lhs.unwrap());
            resolvePath(Namespace::Value, pathExpr->path, suffix);
        } else if (gotLabels) {
            msg.error()
               .setText("Labeled invocation not allowed with non-path left-hand side")
               .setPrimaryLabel(invoke.span, "Cannot use labels here")
               .emit();
        } else {
            invoke.lhs.autoAccept(*this);
            visitNamedNodeList<ast::Expr::Ptr>(invoke.args);
        }
    }

    // Types //
    void NameResolver::visit(const ast::TypePath & typePath) {
        resolvePath(Namespace::Type, typePath.path);
    }

    // Patterns //
    /// All identifiers (not Path) appeared in patterns are bindings, thus we just define them in current rib

    void NameResolver::visit(const ast::IdentPat & pat) {
        defineLocal(pat.id, pat.name);
    }

    void NameResolver::visit(const ast::PathPat &) {
        // Complex case, as path in pattern can be const, variant or some associated item.
        // All these names are stored in different namespaces, so we need to find both in value and type namespaces
    }

    void NameResolver::visit(const ast::StructPat & pat) {
        // Note: Path in StructPat is always a type
        resolvePath(Namespace::Type, pat.path);

        // TODO!: Destructuring fields definitions

        for (const auto & field : pat.fields) {
            field.ident.autoAccept(*this);
            field.pat.autoAccept(*this);
        }
    }

    // Ribs //
    size_t NameResolver::getDepth() const {
        return ribStack.size();
    }

    const Rib::Ptr & NameResolver::curRib() const {
        const auto depth = getDepth();
        if (depth == 0) {
            log::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds: ", depth);
        }
        return ribStack.at(depth - 1);
    }

    void NameResolver::enterRootRib() {
        appendCustomPath("[ROOT]");

        log.dev("Enter root rib");
        ribStack.emplace_back(std::make_unique<Rib>(Rib::Kind::Root));
        currentModule = sess->modTreeRoot.unwrap();
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterRib(Rib::Kind kind) {
        if (getDepth() == UINT32_MAX) {
            log::devPanic("Maximum ribStack depth limit exceeded");
        }
        ribStack.emplace_back(std::make_unique<Rib>(kind));
    }

    void NameResolver::enterModule(Symbol name, Namespace ns, Rib::Kind kind) {
        using namespace utils::map;

        log.dev("Enter module '", name, "' from ", nsToString(ns), " namespace");

        const auto & moduleDef = expectAt(
            currentModule->getNS(ns),
            name,
            "`NameResolver::enterModule` -> namespace: '" + nsToString(ns) + "'"
        ).asDef();

        currentModule = sess->defTable.getModule(moduleDef);

        appendModulePath(name, currentModule->getDefId());

        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterFuncModule(Symbol baseName, Symbol suffix) {
        log.dev("Enter func module '", baseName, "'");
        using namespace utils::map;

        currentModule = sess->defTable
                            .getFuncModule(
                                expectAt(
                                    currentModule->getNS(Namespace::Value),
                                    baseName,
                                    "`NameResolver::enterFuncModule`"
                                ).asFOS(), suffix
                            );

        appendModulePath(baseName + suffix, currentModule->getDefId());

        enterRib(Rib::Kind::Raw);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterBlock(NodeId nodeId, Rib::Kind kind) {
        log.dev("Enter block ", nodeId);
        appendBlockPath(nodeId);

        currentModule = sess->defTable.getBlock(nodeId);
        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::exitRib() {
        if (getDepth() == 0) {
            log::devPanic("NameResolver: Tried to exit from empty rib stack");
        }
        if (curRib()->boundModule.some()) {
            log.dev("Exit module, current path: ", scopePath);
            currentModule = currentModule->parent.unwrap("Tried to exit top-level module");
            // Remove last segment if exit from module/block
            removePathSeg();
        }
        printRib();
        ribStack.pop_back();
    }

    void NameResolver::liftToDepth(size_t prevDepth) {
        if (prevDepth > getDepth()) {
            log::devPanic("Called `NameResolver::lifeToDepth` with `prevDepth` > `depth`");
        }

        // Note: Save depth when we started, because it will be changed in `exitRib`
        const auto depth = getDepth();
        for (size_t i = prevDepth; i < depth; i++) {
            exitRib();
        }
    }

    // Definitions //
    void NameResolver::defineLocal(NodeId identPatId, const ast::Ident::PR & ident) {
        const auto & name = ident.unwrap().sym;
        log.dev("Define '", name, "' local");

        const auto & redecl = curRib()->defineLocal(identPatId, name);

        if (redecl.some()) {
            msg.error()
               .setText("'", name, "' has been already declared")
               .setPrimaryLabel(ident.span(), "'", name, "' has been already declared")
               .emit();
        } else {
            // TODO: Update logic for `a | b` pat.
            //  This should only happen in case of fresh name in `a | b`
            _resolutions.setRes(identPatId, Res {identPatId});
        }
    }

    // Resolution //
    /// Resolves any kind of path
    /// Namespace used for last segment in path, e.g. in `a::b::c` `c` must be in specified namespace
    void NameResolver::resolvePath(Namespace targetNs, const ast::Path & path, const Symbol::Opt & suffix) {
        // TODO: Resolve segment generics

        // Resolve local //
        // If path is one segment long then it can be a local variable.
        // TODO: Don't try to resolve local, if it is single-element path but has generics though
        if (path.segments.size() == 1) {
            const auto & seg = path.segments.at(0).unwrap();
            const auto & ident = seg.ident.unwrap().sym;
            auto resolved = resolveLocal(ident, path);
            if (resolved) {
                return;
            }
        }

        auto res = pathResolver.resolve(
            currentModule,
            targetNs,
            path,
            suffix,
            ResMode::Specific
        );

        if (res.ok()) {
            _resolutions.setRes(path.id, Res {res.asSpecific()});
        } else {
            // Set error resolution
            _resolutions.setRes(path.id, Res {});
        }
    }

    /**
     * @brief Try to resolve local. Local variables have higher precedence than items,
     *  even if in code they are declared earlier independently on depth of appearance.
     * @param ns
     * @param name
     * @param path Path node referencing particular name
     * @return
     */
    bool NameResolver::resolveLocal(Symbol name, const ast::Path & path) {
        auto depth = getDepth();
        auto pathNodeId = path.id;
        while (true) {
            if (depth == 0) {
                break;
            }
            const auto & rib = ribStack.at(depth - 1);
            auto local = rib->findLocal(name);
            if (local.some()) {
                _resolutions.setRes(pathNodeId, Res {local.unwrap()});
                return true;
            }
            depth--;
        }
        log.dev("Local '", name, "' not found");
        return false;
    }

    // Debug //
    void NameResolver::printRib() {
        if (not printRibsFlag or ribStack.empty()) {
            return;
        }
        ribsDebugOutput += log::fmt("[", getDepth(), "] (locals): ", curRib()->locals, "\n");
    }

    void NameResolver::dumpRibs() {
        if (not printRibsFlag or ribStack.empty()) {
            return;
        }
        log.info("Printing rib (`-print=ribs`)");
        log.raw(ribsDebugOutput).nl();
    }

    void NameResolver::appendModulePath(Symbol modName, DefId defId) {
        // Check for dev mode here as getting definition might be an expensive operation
        if (not config.checkDevStage(config::Config::DevStage::NameRes)) {
            return;
        }
        appendCustomPath(sess->defTable.getDef(defId).kindStr() + " '" + modName.toString() + "'");
    }

    void NameResolver::appendBlockPath(NodeId nodeId) {
        appendCustomPath("block#" + nodeId.toString());
    }

    void NameResolver::appendCustomPath(const std::string & segment) {
        if (not config.checkDevStage(config::Config::DevStage::NameRes)) {
            return;
        }
        scopePath.emplace_back(segment);
    }

    /// Removes last path segment from `scopePath`
    void NameResolver::removePathSeg() {
        if (not config.checkDevStage(config::Config::DevStage::NameRes)) {
            return;
        }
        scopePath.pop_back();
    }
}
