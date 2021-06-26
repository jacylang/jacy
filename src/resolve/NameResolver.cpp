#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<dt::none_t> NameResolver::resolve(const sess::sess_ptr & sess, const ast::Party & party) {
        this->sess = sess;
        printRibsFlag = common::Config::getInstance().checkPrint(common::Config::PrintKind::Ribs);

        enterRootRib();

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        log.dev("Rib depth after name resolution: ", getDepth());

        sess->resStorage = std::move(resStorage);
        return {dt::None, extractSuggestions()};
    }

    void NameResolver::visit(const ast::Dir & dir) {
        enterNamedMod(dir.name);
        visitEach(dir.modules);
        exitRib();
    }

    void NameResolver::visit(const ast::File & file) {
        enterNamedMod(sess->sourceMap.getSourceFile(file.fileId).filename());
        visitEach(file.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Func & func) {
        enterNamedMod(func.name.unwrap()->getValue()); // -> `func` mod rib

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
            define(param->name.unwrap()->getValue(), Name::Kind::Param, param->name.unwrap()->id);
        }

        if (func.body) {
            func.body.unwrap().accept(*this);
        }

        exitRib(); // <- (params) rib

        exitRib(); // <- `func` mod rib
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterNamedMod(mod.name.unwrap()->getValue());
        visitEach(mod.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        // FIXME: Forward define struct field in `ModuleTreeBuilder` to resolve paths pointing to struct??!!
        for (const auto & field : _struct.fields) {
            field->type.accept(*this);
        }
    }

    void NameResolver::visit(const ast::UseDecl & useDecl) {
        resolveUseTree(useDecl.useTree);
    }

    void NameResolver::resolveUseTree(const ast::use_tree_ptr & maybeUseTree) {
        // TODO: Everything goes harder
    }

    // Statements //
    void NameResolver::visit(const ast::LetStmt & letStmt) {
        enterRib();
        define(letStmt.pat->name.unwrap()->getValue(), Name::Kind::Local, letStmt.id);
    }

    // Expressions //
    void NameResolver::visit(const ast::Block & block) {
        if (block.blockKind == ast::BlockKind::OneLine) {
            // Note: One-line block as it is an expression does not open new scope
            block.oneLine.unwrap().accept(*this);
            return;
        }

        const auto prevDepth = getDepth();
        enterAnonMod(block.id); // -> block rib
        for (const auto & stmt : block.stmts.unwrap()) {
            stmt.accept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Lambda & lambdaExpr) {
        enterRib(); // -> (lambda params)

        for (const auto & param : lambdaExpr.params) {
            // TODO: Param name
            if (param->type) {
                param->type.unwrap().accept(*this);
            }
        }

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        lambdaExpr.body.accept(*this);

        exitRib(); // <- (lambda params)
    }

    void NameResolver::visit(const ast::PathExpr & pathExpr) {
        // Note!!!: PathExpr MUST BE visited only in case of it is a part of an expression.
        //  For example, `StructExpr` must call `resolvePathExpr` itself, but not visit its path
        //  Every Node that uses `PathExpr` not as "always from value namespace" must resolve path itself!
        resolvePathExpr(RibNamespace::Value, pathExpr);
    }

    // Types //
    void NameResolver::visit(const ast::TypePath & typePath) {
        // TODO: !!!
    }

    // Ribs //
    size_t NameResolver::getDepth() const {
        return ribStack.size();
    }

    const rib_ptr & NameResolver::curRib() const {
        const auto depth = getDepth();
        if (depth == 0) {
            common::Logger::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds: ", getDepth());
        }
        return ribStack.at(getDepth() - 1);
    }

    void NameResolver::enterRootRib() {
        ribStack.emplace_back(std::make_unique<Rib>(Rib::Kind::Root));
        currentModule = sess->modTreeRoot.unwrap();
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterRib(Rib::Kind kind) {
        log.dev("Enter rib");
        if (getDepth() == UINT32_MAX) {
            Logger::devPanic("Maximum ribStack depth limit exceeded");
        }
        ribStack.emplace_back(std::make_unique<Rib>(kind));
    }

    void NameResolver::enterNamedMod(const std::string & name, Rib::Kind kind) {
        log.dev("Enter named mod '", name, "'");
        currentModule = currentModule->children.at(name);
        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::enterAnonMod(node_id nodeId, Rib::Kind kind) {
        log.dev("Enter anon mod '", nodeId, "'");
        currentModule = currentModule->anonBlocks.at(nodeId);
        enterRib(kind);
        curRib()->bindMod(currentModule);
    }

    void NameResolver::exitRib() {
        log.dev("Exit rib");
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

    // Declarations //
    void NameResolver::define(const ast::id_ptr & ident) {
        log.dev("Define '", ident.unwrap()->getValue(), "' local");

        const auto & redecl = curRib()->define(ident);

        if (redecl) {
            const auto & name = ident.unwrap()->getValue();
            suggestErrorMsg("'" + name + "' has been already declared", ident.span());
        }
    }

    // Resolution //
    void NameResolver::resolveSimplePath(const ast::simple_path_ptr & simplePath) {
        // TODO
        // Simple-dimple LOL
    }

    void NameResolver::resolvePathExpr(Namespace ns, const ast::PathExpr & pathExpr) {
        // TODO: global

        if (pathExpr.segments.size() == 1) {
            // Simplest case, we just got an identifier

            // TODO!!!: Keyword segments: self, super, etc.
            // FIXME
            const auto & seg = pathExpr.segments.at(0).unwrap();
            if (seg->ident) {
                const auto & identStr = seg->ident.unwrap().unwrap()->getValue();
                auto resolved = resolve(ns, seg->ident.unwrap());
                if (not resolved) {
                    log.dev("Failed to resolve '", identStr, "' [", pathExpr.id, "]");
                    suggestErrorMsg("'" + identStr + "' is not defined", pathExpr.span);
                }
            }
        }
    }

    bool NameResolver::resolve(Namespace ns, const ast::id_ptr & ident) {
        log.dev("Resolve '", ident.unwrap()->getValue(), "'");
        auto depth = getDepth();
        while (true) {
            if (depth == 0) {
                break;
            }
            const auto & rib = ribStack.at(depth - 1);
            if (rib->resolve(ns, ident, resStorage)) {
                return true;
            }
            depth--;
        }
        return false;
    }

    // Suggestions //
    // FIXME: Move to ModuleTreeBuilder
//    void NameResolver::suggestCannotRedeclare(
//        const std::string & name,
//        const std::string & as,
//        const std::string & declaredAs,
//        ast::node_id nodeId,
//        ast::node_id declaredHere
//    ) {
//        suggest(
//            std::make_unique<sugg::MsgSpanLinkSugg>(
//                "Cannot redeclare '" + name + "' as " + as,
//                sess->nodeMap.getNodeSpan(nodeId),
//                "Because it is already declared as " + declaredAs + " here",
//                sess->nodeMap.getNodeSpan(declaredHere),
//                SuggKind::Error
//            )
//        );
//    }

    // Debug //
    void NameResolver::printRib() {
        if (not printRibsFlag or ribStack.empty()) {
            return;
        }
        log.info("Printing rib (`-print=ribs`) at depth [", getDepth(), "]");
        const auto & rib = curRib();
        log.raw(rib->locals);
    }
}
