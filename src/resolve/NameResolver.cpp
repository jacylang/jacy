#include "resolve/NameResolver.h"

namespace jc::resolve {
    NameResolver::NameResolver() : StubVisitor("NameResolver"), config(config::Config::getInstance()) {}

    dt::SuggResult<dt::none_t> NameResolver::resolve(const sess::Session::Ptr & sess, const ast::Party & party) {
        this->sess = sess;
        printRibsFlag = config::Config::getInstance().checkPrint(config::Config::PrintKind::Ribs);

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
        return {None, extractSuggestions()};
    }

    void NameResolver::visit(const ast::Func & func) {
        enterFuncModule(func.name.unwrap().sym, Module::getFuncSuffix(func.sig));

        if (func.sig.returnType.some()) {
            func.sig.returnType.unwrap().autoAccept(*this);
        }

        enterRib(); // -> (params) rib

        for (const auto & param : func.sig.params) {
            /// Order matters, we resolve type first, then default value
            ///  to disallow referencing pattern identifier in default value

            param.type.autoAccept(*this);

            if (param.defaultValue.some()) {
                param.defaultValue.unwrap().autoAccept(*this);
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
        impl.traitTypePath.autoAccept(*this);

        if (impl.forType.some()) {
            impl.forType.unwrap().autoAccept(*this);
        }

        enterModule(Module::getImplName(impl), Namespace::Type); // -> `impl` mod
        visitEach(impl.members);
        exitRib(); // <- `impl` mod
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterModule(mod.getName().sym); // -> 'mod` mod
        visitEach(mod.items);
        exitRib(); // <- `mod` mod, mod mod mod mod mod
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        for (const auto & field : _struct.fields) {
            field.type.autoAccept(*this);
        }
    }

    void NameResolver::visit(const ast::Trait & trait) {
        enterModule(trait.getName().sym);
        visitEach(trait.superTraits);
        visitEach(trait.members);
        exitRib();
    }

    void NameResolver::visit(const ast::Init & init) {
        enterFuncModule(span::Symbol::fromKw(span::Kw::Init), Module::getFuncSuffix(init.sig));

        if (init.sig.returnType.some()) {
            init.sig.returnType.unwrap().autoAccept(*this);
        }

        enterRib(); // -> (params) rib

        for (const auto & param : init.sig.params) {
            param.pat.autoAccept(*this);
            if (param.defaultValue.some()) {
                param.defaultValue.unwrap().autoAccept(*this);
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
        letStmt.pat.autoAccept(*this);

        if (letStmt.type.some()) {
            letStmt.type.unwrap().autoAccept(*this);
        }

        if (letStmt.assignExpr.some()) {
            letStmt.assignExpr.unwrap().autoAccept(*this);
        }
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
        // Note: Each pattern in arm is separate rib, thus we need separate handler for it here

        const auto prevDepth = getDepth();

        for (const auto & pat : arm.patterns) {
            enterRib();
            pat.autoAccept(*this);
        }

        arm.body.autoAccept(*this);

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Invoke & invoke) {
        /// Function invocation has a specific name resolution workflow
        /// When function is called by path, i.e. `path::to::func(...)`,
        ///  we need to resolve it relying on specified labels

        auto[gotLabels, suffix] = Module::getCallSuffix(invoke.args);

        if (invoke.lhs.unwrap()->kind == ast::ExprKind::Path) {
            const auto & pathExpr = ast::Expr::as<ast::PathExpr>(invoke.lhs.unwrap());
            resolvePath(Namespace::Value, pathExpr->path, suffix);
        } else if (gotLabels) {
            // Labeled calls are not allowed in non-path invocations as we cannot resolve them.
            // This case must be handled in `Validator`, thus it is a bug
            log::devPanic("[NameResolver] Got invocation with labels with non-path lhs expression");
        } else {
            invoke.lhs.autoAccept(*this);
            visitEach(invoke.args);
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
        resolvePath(Namespace::Type, pat.path.unwrap()->path);

        // TODO!: Destructuring fields definitions

        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case ast::StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<ast::StructPatternDestructEl>(el.el);
                    //                    defineLocal(dp.name);
                    dp.pat.autoAccept(*this);
                    break;
                }
                case ast::StructPatEl::Kind::Borrow: {
                    const auto & bp = std::get<ast::StructPatBorrowEl>(el.el);
                    //                    defineLocal(bp.name);
                    break;
                }
                case ast::StructPatEl::Kind::Spread:;
            }
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

        log.dev("Enter module '", name, "' from ", Module::nsToString(ns), " namespace");

        currentModule = sess->defTable
                            .getModule(
                                expectAt(
                                    currentModule->getNS(ns),
                                    name,
                                    "`NameResolver::enterModule` -> namespace: '" + Module::nsToString(ns) + "'"
                                ).asDef()
                            );

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
                                ).asFuncOverload(), suffix
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
    void NameResolver::defineLocal(NodeId localNodeId, const ast::Ident::PR & ident) {
        const auto & name = ident.unwrap().sym;
        log.dev("Define '", name, "' local");

        const auto & redecl = curRib()->defineLocal(localNodeId, name);

        if (redecl.some()) {
            suggestErrorMsg(log::fmt("'", name, "' has been already declared"), ident.span());
        }
    }

    // Resolution //
    void NameResolver::resolveSimplePath(const ast::SimplePath &) {
        // TODO
        // Simple-dimple LOL
    }

    /// Resolves any kind of path
    /// Namespace used for last segment in path, e.g. in `a::b::c` `c` must be in specified namespace
    void NameResolver::resolvePath(Namespace targetNS, const ast::Path & path, const Symbol::Opt & suffix) {
        // TODO: global

        // TODO: Generic args not allowed in local variables, check for single-seg path with generics

        // Resolve local //
        // If path is one segment long then it can be a local variable
        if (path.segments.size() == 1) {
            const auto & seg = path.segments.at(0).unwrap();
            const auto & ident = seg.ident.unwrap().sym;
            auto resolved = resolveLocal(ident, path);
            if (resolved) {
                return;
            }
        }

        // TODO!!!: Keyword segments: self, super, etc.

        // Resolve complex path //

        Module::Ptr searchMod = sess->defTable.getModule(currentModule->nearestModDef);
        // Path as string, built iterating through path segments
        // When resolution fails, it contains all segments we dived into
        std::string pathStr;
        bool inaccessible = false;
        Option<UnresSeg> unresSeg  = None;
        PerNS<IntraModuleDef::Opt> altDefs = {None, None, None};

        for (size_t i = 0; i < path.segments.size(); i++) {
            // For path prefix `a::b::` we find segments in type namespace,
            // but last segment is resolved in target namespace
            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;
            Namespace ns = isPrefixSeg ? Namespace::Type : targetNS;

            const auto & seg = path.segments.at(i).unwrap();
            auto segName = seg.ident.unwrap().sym;

            // Add suffix for last segment if present
            if (not isPrefixSeg and suffix.some()) {
                segName += suffix.unwrap();
            }

            // TODO: Resolve segment generics

            // If this is a prefix segment, we need to lookup for a name,
            //  as it can be from parent (or grandparent+) scope
            if (isPrefixSeg) {
                // TODO: Optimize - merge `find` with `has` to avoid additional searching if found
                while (true) {
                    log.dev("Trying to find '", segName, "' first segment");
                    if (searchMod->has(Namespace::Type, segName) or searchMod->parent.none()) {
                        break;
                    }
                    searchMod = searchMod->parent.unwrap();
                }
            }

            searchMod->find(ns, segName).then([&](const IntraModuleDef & def) {
                DefId::Opt maybeDefId = None;
                if (def.isFuncOverload()) {
                    // Function overload cannot be a prefix of the path thus just don't even try to find something inside
                    if (not isPrefixSeg) {
                        auto overloads = sess->defTable.getFuncOverload(def.asFuncOverload());

                        if (suffix.none()) {
                            // If no suffix provided then only one overload can be referenced non-ambiguous
                            if (overloads.size() == 1) {
                                maybeDefId = overloads.begin()->second;
                                log.dev("Found single function for '", pathStr, "' - ", maybeDefId.unwrap());
                            } else {
                                log.dev("Ambiguous use of function '", segName, "'");
                                suggestErrorMsg(
                                    log::fmt(
                                        "Use of function '",
                                        segName,
                                        "' is ambiguous, use labels to disambiguate"
                                    ),
                                    path.span
                                );
                                return;
                            }
                        } else {
                            const auto & foundOverload = overloads.find(suffix.unwrap());
                            if (foundOverload == overloads.end()) {
                                const auto & fullName = segName + suffix.unwrap();
                                log.dev("Failed to find function '", fullName, "'");
                                suggestErrorMsg(
                                    log::fmt("Failed to find function '", fullName, "'"),
                                    path.span
                                );
                                return;
                            }
                            maybeDefId = foundOverload->second;
                        }
                    }
                } else {
                    maybeDefId = def.asDef();
                }

                auto defId = maybeDefId.unwrap();
                // Check visibility
                // Note: We check if current segment is not the first one,
                //  because items in a module are visible for other items in it, and we already found the name
                // Note!: Order matters, we need to check visibility before descend to next module
                if (not isFirstSeg and sess->defTable.getDefVis(defId) != DefVis::Pub) {
                    inaccessible = true;
                    unresSeg = {i, defId};
                    log.dev("Failed to resolve '", segName, "' as it is a private def ", defId);
                    return;
                }

                if (isPrefixSeg) {
                    // Resolve prefix path, `a::b::` (before target)
                    searchMod = sess->defTable.getModule(defId);
                    log.dev("Search in module by path segment '", pathStr, "' with def id ", defId);
                } else {
                    // Resolve last segment
                    _resolutions.setRes(path.id, Res {defId});
                    log.dev("Resolved path '", pathStr, "::", segName, "' as def id ", defId);
                }
            }).otherwise([&]() {
                // Resolution failed
                log.dev("Failed to resolve '", segName, "' by path '", pathStr, "'");
                unresSeg = {i, None};
                altDefs = searchMod->findAll(segName);
            });

            if (unresSeg.some()) {
                break;
            }

            if (not isFirstSeg) {
                pathStr += "::";
            }
            pathStr += segName.toString();
        }

        if (unresSeg.some()) {
            using namespace utils::arr;
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = expectAt(
                path.segments,
                unresSeg.unwrap().segIndex,
                "`unresolvedSegIdent`"
            ).unwrap().ident.unwrap();

            const auto & unresolvedSegName = unresolvedSegIdent.sym;

            if (inaccessible) {
                const auto & defKind = sess->defTable.getDef(unresSeg.unwrap().defId.unwrap()).kindStr();
                // Report "Cannot access" error
                suggestErrorMsg(
                    log::fmt("Cannot access private ", defKind, " '", unresolvedSegName, "' in '", pathStr, "'"),
                    unresolvedSegIdent.span
                );
            } else {
                // Report "Not defined" error
                auto msg = log::fmt("'", unresolvedSegName, "' is not defined");
                if (not pathStr.empty()) {
                    msg += " in '" + pathStr + "'";
                }
                suggestErrorMsg(msg, unresolvedSegIdent.span);
                suggestAltNames(targetNS, unresolvedSegName, altDefs);
            }
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
    bool NameResolver::resolveLocal(const Symbol & name, const ast::Path & path) {
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

    /**
     * @brief Add help messages with alternatives for unresolved name
     * @param target Namespace to exclude from alternatives
     * @param name
     * @param altDefs Alternative definitions found in scope
     */
    void NameResolver::suggestAltNames(
        Namespace target,
        const Symbol & name,
        const PerNS<IntraModuleDef::Opt> & altDefs
    ) {
        altDefs.each([&](IntraModuleDef::Opt intraModuleDef, Namespace nsKind) {
            if (nsKind == target or intraModuleDef.none()) {
                return;
            }
            std::string kind;
            if (intraModuleDef.unwrap().isFuncOverload()) {
                kind = "function";
            } else {
                kind = sess->defTable.getDef(intraModuleDef.unwrap().asDef()).kindStr();
            }
            log.dev(
                "Found alternative for unresolved name '",
                name,
                "' as ",
                kind,
                " in ",
                Module::nsToString(nsKind),
                " namespace"
            );
            suggestHelp(
                log::fmt(
                    "Alternative: '",
                    name,
                    "' ",
                    kind,
                    ", but it cannot be used as ",
                    Def::nsAsUsageStr(target)
                )
            );
        });
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

    void NameResolver::appendModulePath(const Symbol & modName, DefId defId) {
        // Check for dev mode here as getting definition might be an expensive operation
        if (not config.checkDev()) {
            return;
        }
        appendCustomPath(sess->defTable.getDef(defId).kindStr() + " '" + modName.toString() + "'");
    }

    void NameResolver::appendBlockPath(NodeId nodeId) {
        appendCustomPath("block#" + nodeId.toString());
    }

    void NameResolver::appendCustomPath(const std::string & segment) {
        if (not config.checkDev()) {
            return;
        }
        scopePath.emplace_back(segment);
    }

    /// Removes last path segment from `scopePath`
    void NameResolver::removePathSeg() {
        scopePath.pop_back();
    }
}
