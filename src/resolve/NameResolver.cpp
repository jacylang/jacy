#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> NameResolver::resolve(const sess::sess_ptr & sess, const ast::Party & party) {
        this->sess = sess;
        printRibsFlag = common::Config::getInstance().checkPrint(common::Config::PrintKind::Ribs);

        enterRootRib();

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        sess->resStorage = std::move(_resStorage);
        return {dt::None, extractSuggestions()};
    }

    void NameResolver::visit(const ast::Dir & dir) {
        enterModule(dir.name);
        visitEach(dir.modules);
        exitRib();
    }

    void NameResolver::visit(const ast::File & file) {
        enterModule(sess->sourceMap.getSourceFile(file.fileId).filename());
        visitEach(file.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Func & func) {
        // Note: Functions stored in value namespace
        enterModule(func.name.unwrap()->getValue(), Namespace::Value); // -> `func` mod rib

        for (const auto & param : func.params) {
            param->type.accept(*this);
            if (param->defaultValue) {
                param->defaultValue.unwrap().accept(*this);
            }
        }

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        // Note: Function parameter names can conflict
        enterRib(); // -> (params) rib

        for (const auto & param : func.params) {
            define(param->name);
        }

        if (func.body) {
            func.body.unwrap().accept(*this);
        }

        exitRib(); // <- (params) rib

        exitRib(); // <- `func` mod rib
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterModule(mod.name.unwrap()->getValue());
        visitEach(mod.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        for (const auto & field : _struct.fields) {
            field->type.accept(*this);
        }
    }

    // Statements //
    void NameResolver::visit(const ast::LetStmt & letStmt) {
        enterRib();
        letStmt.pat.accept(*this);

        if (letStmt.type) {
            letStmt.type.unwrap().accept(*this);
        }

        if (letStmt.assignExpr) {
            letStmt.assignExpr.unwrap().accept(*this);
        }
    }

    // Expressions //
    void NameResolver::visit(const ast::Block & block) {
        if (block.blockKind == ast::BlockKind::OneLine) {
            // Note: One-line block as it is an expression does not open new scope
            block.oneLine.unwrap().accept(*this);
            return;
        }

        const auto prevDepth = getDepth();
        enterBlock(block.id); // -> block rib
        for (const auto & stmt : block.stmts.unwrap()) {
            stmt.accept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Lambda & lambda) {
        enterRib(); // -> (lambda params)

        for (const auto & param : lambda.params) {
            param->pat.accept(*this);
            if (param->type) {
                param->type.unwrap().accept(*this);
            }
        }

        if (lambda.returnType) {
            lambda.returnType.unwrap().accept(*this);
        }

        lambda.body.accept(*this);

        exitRib(); // <- (lambda params)
    }

    void NameResolver::visit(const ast::PathExpr & pathExpr) {
        // Note!!!: PathExpr MUST BE visited only in case of it is a part of an expression.
        //  For example, `StructExpr` must call `resolvePath` itself, but not visit its path
        //  Every Node that uses `PathExpr` not as "always from value namespace" must resolve path itself!
        resolvePath(Namespace::Value, *pathExpr.path);
    }

    void NameResolver::visit(const ast::MatchArm & arm) {
        // Note: Each pattern in arm is separate rib, thus we need separate handler for it here

        const auto prevDepth = getDepth();

        for (const auto & pat : arm.patterns) {
            enterRib();
            pat.accept(*this);
        }

        arm.body.accept(*this);

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::StructExpr & _struct) {
        resolvePath(Namespace::Type, *_struct.path.unwrap()->path);
        visitEach(_struct.fields);
    }

    // Types //
    void NameResolver::visit(const ast::TypePath & typePath) {
        resolvePath(Namespace::Type, *typePath.path);
    }

    // Patterns //
    /// All identifiers (not Path) appeared in patterns are bindings, thus we just define them in current rib

    void NameResolver::visit(const ast::BorrowPat & pat) {
        define(pat.name);
    }

    void NameResolver::visit(const ast::PathPat & pat) {
        // Complex case, as path in pattern can be const, variant or some associated item.
        // All these names are stored in different namespaces, so we need to find both in value and type namespaces
    }

    void NameResolver::visit(const ast::StructPat & pat) {
        // Note: Path in StructPat is always a type
        resolvePath(Namespace::Type, *pat.path.unwrap()->path);

        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case ast::StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<ast::StructPatternDestructEl>(el.el);
                    define(dp.name);
                    dp.pat.accept(*this);
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

    const rib_ptr & NameResolver::curRib() const {
        const auto depth = getDepth();
        if (depth == 0) {
            common::Logger::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds: ", depth);
        }
        return ribStack.at(depth - 1);
    }

    void NameResolver::enterRootRib() {
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
        currentModule = sess->defStorage.getModule(currentModule->getNS(ns).at(name));
        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterBlock(node_id nodeId, Rib::Kind kind) {
        currentModule = sess->defStorage.getBlock(nodeId);
        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::exitRib() {
        if (getDepth() == 0) {
            Logger::devPanic("NameResolver: Tried to exit from empty rib stack");
        }
        if (curRib()->boundModule) {
            currentModule = currentModule->parent.unwrap("Tried to exit top-level module");
        }
        printRib();
        ribStack.pop_back();
    }

    void NameResolver::liftToDepth(size_t prevDepth) {
        if (prevDepth > getDepth()) {
            common::Logger::devPanic("Called `NameResolver::lifeToDepth` with `prevDepth` > `depth`");
        }

        // Note: Save depth when we started, because it will be changed in `exitRib`
        const auto depth = getDepth();
        for (size_t i = prevDepth; i < depth; i++) {
            exitRib();
        }
    }

    // Definitions //
    void NameResolver::define(const ast::id_ptr & ident) {
        log.dev("Define '", ident.unwrap()->getValue(), "' local");

        const auto & redecl = curRib()->define(ident);

        if (redecl) {
            const auto & name = ident.unwrap()->getValue();
            suggestErrorMsg("'" + name + "' has been already declared", ident.span());
        }
    }

    // Resolution //
    void NameResolver::resolveSimplePath(const ast::simple_path_ptr&) {
        // TODO
        // Simple-dimple LOL
    }

    /// Resolves any kind of path
    /// Namespace used for last segment in path, e.g. in `a::b::c` `c` must be in specified namespace
    void NameResolver::resolvePath(Namespace targetNS, const ast::Path & path) {
        // Unresolved segment
        // Has index in path segments vector and optional definition id,
        // if segment is resolved but it is private
        struct UnresSeg {
            size_t segIndex;
            opt_def_id defId{dt::None};
        };

        // TODO: global

        // TODO: Generic args not allowed in local variables, check for single-seg path with generics

        // Resolve local //
        // If path is one segment long then it can be a local variable
        if (path.segments.size() == 1) {
            const auto & seg = path.segments.at(0).unwrap();
            if (seg->ident) {
                const auto & identStr = seg->ident.unwrap().unwrap()->getValue();
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

        module_ptr searchMod = sess->defStorage.getModule(currentModule->nearestModDef.unwrap());
        // Path as string, built iterating through path segments
        // When resolution fails, it contains all segments we dived into
        std::string pathStr;
        bool inaccessible = false;
        dt::Option<UnresSeg> unresSeg{dt::None};
        PerNS<opt_def_id> altDefs = {dt::None, dt::None, dt::None};

        for (size_t i = 0; i < path.segments.size(); i++) {
            const auto & seg = path.segments.at(i).unwrap();
            const auto & segName = seg->ident.unwrap().unwrap()->getValue();

            // TODO: Resolve segment generics

            // For path prefix `a::b::` we find segments in type namespace,
            // but last segment is resolved in target namespace
            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;
            Namespace ns = isPrefixSeg ? Namespace::Type : targetNS;

            searchMod->find(ns, segName).then([&](def_id defId) {
                // Check visibility
                // Note: We check if current segment is not the first one,
                //  because items in a module are visible for other items in it, and we already found the name
                // Note!: Order matters, we need to check visibility before descend to next module
                if (not isFirstSeg and sess->defStorage.getDefVis(defId) != DefVis::Pub) {
                    inaccessible = true;
                    unresSeg = {i, defId};
                    log.dev("Failed to resolve '", segName, "' as it is a private def #", defId);
                    return;
                }

                if (isPrefixSeg) {
                    // Resolve prefix path, `a::b::` (before target)
                    searchMod = sess->defStorage.getModule(defId);
                    log.dev("Enter module by path segment '", pathStr, "' with def id #", defId);
                } else {
                    // Resolve last segment
                    log.dev("Resolved path '", pathStr, "::", segName, "' as def id #", defId);
                    _resStorage.setRes(path.id, Res{defId});
                }
            }).otherwise([&]() {
                // Resolution failed
                log.dev("Failed to resolve '", segName, "' by path '", pathStr, "'");
                unresSeg = {i, dt::None};
                altDefs = searchMod->findAlt(segName);
            });

            if (unresSeg) {
                break;
            }

            if (isPrefixSeg) {
                if (not isFirstSeg) {
                    pathStr += "::";
                }
                pathStr += segName;
            }
        }

        if (unresSeg) {
            // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
            // so give different error message
            const auto & unresolvedSegIdent = path.segments.at(unresSeg.unwrap().segIndex)
                .unwrap()->ident.unwrap().unwrap();
            const auto & unresolvedSegName = unresolvedSegIdent->getValue();

            if (inaccessible) {
                const auto & defKind = sess->defStorage.getDef(unresSeg.unwrap().defId.unwrap()).kindStr();
                // Report "Cannot access" error
                suggestErrorMsg(
                    "Cannot access private " + defKind + " '" + unresolvedSegName + "' in '" + pathStr + "'",
                    unresolvedSegIdent->span);
            } else {
                // Report "Not defined" error
                auto msg = "'" + unresolvedSegName + "' is not defined";
                if (not pathStr.empty()) {
                    msg += " in '" + pathStr + "'";
                }
                suggestErrorMsg(msg, unresolvedSegIdent->span);
                suggestAltNames(targetNS, unresolvedSegName, altDefs);
            }
        }
    }

    bool NameResolver::resolveLocal(Namespace ns, const std::string & name, node_id refNodeId) {
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

        common::Logger::devDebug("Set error resolution for node #", refNodeId);
        // Set error resolution
        _resStorage.setRes(refNodeId, Res{});

        return false;
    }

    void NameResolver::suggestAltNames(Namespace target, const std::string & name, const PerNS<opt_def_id> & altDefs) {
        altDefs.each([&](opt_def_id defId, Namespace nsKind) {
            if (nsKind == target or defId.none()) {
                return;
            }
            const auto & def = sess->defStorage.getDef(defId.unwrap());
            log.dev(
                "Found alternative for unresolved name '",
                name,
                "' as def #",
                defId,
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
        log.info("Printing rib (`-print=ribs`) at depth [", getDepth(), "]");
        const auto & rib = curRib();
        log.raw("[locals]: ", rib->locals).nl();
    }
}
