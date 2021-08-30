#include "resolve/NameResolver.h"

namespace jc::resolve {
    NameResolver::NameResolver() : StubVisitor("NameResolver"), config(common::Config::getInstance()) {}

    dt::SuggResult<dt::none_t> NameResolver::resolve(const sess::Session::Ptr & sess, const ast::Party & party) {
        this->sess = sess;
        printRibsFlag = common::Config::getInstance().checkPrint(common::Config::PrintKind::Ribs);

        log.assertLogic(sess->modTreeRoot.unwrap()->parent.none(), "Root module must not have parent");

        try {
            enterRootRib();
            visitEach(party.items);
        } catch (std::exception & e) {
            log.debug("Module path on name resolution fail:\n", utils::arr::join(scopePath, " -> "));
            throw;
        }

        // Debug call to print generated rib stack output
        dumpRibs();

        sess->resStorage = std::move(_resStorage);
        return {None, extractSuggestions()};
    }

    void NameResolver::visit(const ast::Func & func) {
        enterModule(func.getName().name, Namespace::Value); // -> `func` mod rib

        if (func.sig.returnType.some()) {
            func.sig.returnType.unwrap().autoAccept(*this);
        }

        enterRib(); // -> (params) rib

        for (const auto & param : func.sig.params) {
            param.pat.autoAccept(*this);
            if (param.defaultValue.some()) {
                param.defaultValue.unwrap().autoAccept(*this);
            }
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

        enterBlock(impl.id);
        visitEach(impl.members);
        exitRib();
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterModule(mod.getName().name);
        visitEach(mod.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        for (const auto & field : _struct.fields) {
            field.type.autoAccept(*this);
        }
    }

    void NameResolver::visit(const ast::Trait & trait) {
        enterModule(trait.getName().name);
        visitEach(trait.superTraits);
        visitEach(trait.members);
        exitRib();
    }

    void NameResolver::visit(const ast::Init & init) {
        enterModule(Module::getInitName(init), Namespace::Value); // -> `func` mod rib

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

    void NameResolver::visit(const ast::StructExpr & _struct) {
        resolvePath(Namespace::Type, _struct.path.unwrap()->path);
        visitEach(_struct.fields);
    }

    // Types //
    void NameResolver::visit(const ast::TypePath & typePath) {
        resolvePath(Namespace::Type, typePath.path);
    }

    // Patterns //
    /// All identifiers (not Path) appeared in patterns are bindings, thus we just define them in current rib

    void NameResolver::visit(const ast::BorrowPat & pat) {
        define(pat.name);
    }

    void NameResolver::visit(const ast::PathPat&) {
        // Complex case, as path in pattern can be const, variant or some associated item.
        // All these names are stored in different namespaces, so we need to find both in value and type namespaces
    }

    void NameResolver::visit(const ast::StructPat & pat) {
        // Note: Path in StructPat is always a type
        resolvePath(Namespace::Type, pat.path.unwrap()->path);

        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case ast::StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<ast::StructPatternDestructEl>(el.el);
                    define(dp.name);
                    dp.pat.autoAccept(*this);
                    break;
                }
                case ast::StructPatEl::Kind::Borrow: {
                    const auto & bp = std::get<ast::StructPatBorrowEl>(el.el);
                    define(bp.name);
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
            log::Logger::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds: ", depth);
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
            Logger::devPanic("Maximum ribStack depth limit exceeded");
        }
        ribStack.emplace_back(std::make_unique<Rib>(kind));
    }

    void NameResolver::enterModule(const std::string & name, Namespace ns, Rib::Kind kind) {
        log.dev("Enter module '", name, "' from ", Module::nsToString(ns), " namespace");
        using namespace utils::map;
        currentModule = sess->defStorage
                            .getModule(
                                expectAt(
                                    currentModule->getNS(ns),
                                    name,
                                    "`NameResolver::enterModule` -> namespace: '" + Module::nsToString(ns) + "'"));

        appendModulePath(name, currentModule->getDefId());

        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterBlock(NodeId nodeId, Rib::Kind kind) {
        appendBlockPath(nodeId);

        currentModule = sess->defStorage.getBlock(nodeId);
        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::exitRib() {
        if (getDepth() == 0) {
            Logger::devPanic("NameResolver: Tried to exit from empty rib stack");
        }
        if (curRib()->boundModule.some()) {
            currentModule = currentModule->parent.unwrap("Tried to exit top-level module");
            // Remove last segment if exit from module/block
            removePathSeg();
        }
        printRib();
        ribStack.pop_back();
    }

    void NameResolver::liftToDepth(size_t prevDepth) {
        if (prevDepth > getDepth()) {
            log::Logger::devPanic("Called `NameResolver::lifeToDepth` with `prevDepth` > `depth`");
        }

        // Note: Save depth when we started, because it will be changed in `exitRib`
        const auto depth = getDepth();
        for (size_t i = prevDepth; i < depth; i++) {
            exitRib();
        }
    }

    // Definitions //
    void NameResolver::define(NodeId localNodeId, const ast::Ident::PR & ident) {
        log.dev("Define '", ident.unwrap().name, "' local");

        const auto & redecl = curRib()->define(ident);

        if (redecl.some()) {
            const auto & name = ident.unwrap().name;
            suggestErrorMsg("'" + name + "' has been already declared", ident.span());
        }
    }

    // Resolution //
    void NameResolver::resolveSimplePath(const ast::SimplePath&) {
        // TODO
        // Simple-dimple LOL
    }

    /// Resolves any kind of path
    /// Namespace used for last segment in path, e.g. in `a::b::c` `c` must be in specified namespace
    void NameResolver::resolvePath(Namespace targetNS, const ast::Path & path) {
        // TODO: global

        // TODO: Generic args not allowed in local variables, check for single-seg path with generics

        // Resolve local //
        // If path is one segment long then it can be a local variable
        if (path.segments.size() == 1) {
            const auto & seg = path.segments.at(0).unwrap();
            if (seg.ident.some()) {
                const auto & identStr = seg.ident.unwrap().unwrap().name;
                auto resolved = resolveLocal(targetNS, identStr, path.id);
                if (not resolved) {
                    log.dev("Failed to resolve '", identStr, "' [", path.id, "]");
                    suggestErrorMsg("'" + identStr + "' is not defined", path.span);
                }
            }
            return;
        }

        // TODO!!!: Keyword segments: self, super, etc.

        // Resolve complex path //

        Module::Ptr searchMod = sess->defStorage.getModule(currentModule->nearestModDef.unwrap());
        // Path as string, built iterating through path segments
        // When resolution fails, it contains all segments we dived into
        std::string pathStr;
        bool inaccessible = false;
        Option<UnresSeg> unresSeg{None};
        PerNS<DefId::Opt> altDefs = {None, None, None};

        for (size_t i = 0; i < path.segments.size(); i++) {
            const auto & seg = path.segments.at(i).unwrap();
            const auto & segName = seg.ident.unwrap().unwrap().name;

            // TODO: Resolve segment generics

            // For path prefix `a::b::` we find segments in type namespace,
            // but last segment is resolved in target namespace
            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;
            Namespace ns = isPrefixSeg ? Namespace::Type : targetNS;

            searchMod->find(ns, segName).then([&](const DefId & defId) {
                // Check visibility
                // Note: We check if current segment is not the first one,
                //  because items in a module are visible for other items in it, and we already found the name
                // Note!: Order matters, we need to check visibility before descend to next module
                if (not isFirstSeg and sess->defStorage.getDefVis(defId) != DefVis::Pub) {
                    inaccessible = true;
                    unresSeg = {i, defId};
                    log.dev("Failed to resolve '", segName, "' as it is a private def ", defId);
                    return;
                }

                if (isPrefixSeg) {
                    // Resolve prefix path, `a::b::` (before target)
                    searchMod = sess->defStorage.getModule(defId);
                    log.dev("Enter module by path segment '", pathStr, "' with def id ", defId);
                } else {
                    // Resolve last segment
                    log.dev("Resolved path '", pathStr, "::", segName, "' as def id ", defId);
                    _resStorage.setRes(path.id, Res {defId});
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

            if (isPrefixSeg) {
                if (not isFirstSeg) {
                    pathStr += "::";
                }
                pathStr += segName;
            }
        }

        if (unresSeg.some()) {
            using namespace utils::arr;
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = expectAt(
                path.segments,
                unresSeg.unwrap().segIndex,
                "`unresolvedSegIdent`").unwrap().ident.unwrap().unwrap();

            const auto & unresolvedSegName = unresolvedSegIdent.name;

            if (inaccessible) {
                const auto & defKind = sess->defStorage.getDef(unresSeg.unwrap().defId.unwrap()).kindStr();
                // Report "Cannot access" error
                suggestErrorMsg(
                    "Cannot access private " + defKind + " '" + unresolvedSegName + "' in '" + pathStr + "'",
                    unresolvedSegIdent.span);
            } else {
                // Report "Not defined" error
                auto msg = "'" + unresolvedSegName + "' is not defined";
                if (not pathStr.empty()) {
                    msg += " in '" + pathStr + "'";
                }
                suggestErrorMsg(msg, unresolvedSegIdent.span);
                suggestAltNames(targetNS, unresolvedSegName, altDefs);
            }
        }
    }

    bool NameResolver::resolveLocal(Namespace ns, const std::string & name, NodeId refNodeId) {
        auto depth = getDepth();
        while (true) {
            if (depth == 0) {
                break;
            }
            const auto & rib = ribStack.at(depth - 1);
            if (rib->find(ns, name, refNodeId, _resStorage)) {
                log.dev("Resolved '", name, "'");
                return true;
            }
            depth--;
        }
        log.dev("Failed to resolve local '", name, "'");

        log::Logger::devDebug("Set error resolution for node ", refNodeId);

        // Set error resolution
        _resStorage.setRes(refNodeId, Res{});

        return false;
    }

    void NameResolver::suggestAltNames(Namespace target, const std::string & name, const PerNS<DefId::Opt> & altDefs) {
        altDefs.each([&](DefId::Opt defId, Namespace nsKind) {
            if (nsKind == target or defId.none()) {
                return;
            }
            const auto & def = sess->defStorage.getDef(defId.unwrap());
            log.dev(
                "Found alternative for unresolved name '",
                name,
                "' as def ",
                defId.unwrap(),
                " in ",
                Module::nsToString(nsKind),
                " namespace");
            suggestHelp(
                "Alternative: '" + name + "' " + def.kindStr() + ", but it cannot be used as " +
                Def::nsAsUsageStr(target));
        });
    }

    // Debug //
    void NameResolver::printRib() {
        if (not printRibsFlag or ribStack.empty()) {
            return;
        }
        ribsDebugOutput += log.format("[", getDepth(), "] (locals): ", curRib()->locals, "\n");
    }

    void NameResolver::dumpRibs() {
        if (not printRibsFlag or ribStack.empty()) {
            return;
        }
        log.info("Printing rib (`-print=ribs`)");
        log.raw(ribsDebugOutput).nl();
    }

    void NameResolver::appendModulePath(const std::string & modName, DefId defId) {
        // Check for dev mode here as getting definition might be an expensive operation
        if (not config.checkDev()) {
            return;
        }
        appendCustomPath(sess->defStorage.getDef(defId).kindStr() + " '" + modName + "'");
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
