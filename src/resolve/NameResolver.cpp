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
    }

    void NameResolver::visit(const ast::File & file) {

    }


    void NameResolver::visit(const ast::Func & func) {
        enterRib(); // -> (type params) rib
        declareTypeParams(func.typeParams);

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
            declare(param->name.unwrap()->getValue(), Name::Kind::Param, param->name.unwrap()->id);
        }

        if (func.body) {
            func.body.unwrap().accept(*this);
        }

        exitRib(); // <- (params) rib

        exitRib(); // <- (type params) rib
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterModuleRib(mod.id);
        visitEach(mod.items);
        exitRib();
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        declareTypeParams(_struct.typeParams);

        // FIXME: Forward declare struct field in `ModuleTreeBuilder` to resolve paths pointing to struct??!!
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
        declare(letStmt.pat->name.unwrap()->getValue(), Name::Kind::Local, letStmt.id);
    }

    // Expressions //
    void NameResolver::visit(const ast::Block & block) {
        if (block.blockKind == ast::BlockKind::OneLine) {
            // Note: One-line block as it is an expression does not open new scope
            block.oneLine.unwrap().accept(*this);
            return;
        }

        const auto prevDepth = getDepth();
        enterModuleRib(block.id); // -> block rib
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

    void NameResolver::declareTypeParams(const ast::opt_type_params & maybeTypeParams) {
        if (!maybeTypeParams) {
            return;
        }
        // FIXME: USE ONE LOOP!!!
        const auto & typeParams = maybeTypeParams.unwrap();
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Type) {
                declare(
                    std::static_pointer_cast<ast::GenericType>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::TypeParam,
                    typeParam->id
                );
            }
        }
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Lifetime) {
                declare(
                    std::static_pointer_cast<ast::Lifetime>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::Lifetime,
                    typeParam->id
                );
            }
        }
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Const) {
                declare(
                    std::static_pointer_cast<ast::ConstParam>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::ConstParam,
                    typeParam->id
                );
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
            common::Logger::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds: ", getDepth());
        }
        return ribStack.at(getDepth() - 1);
    }

    void NameResolver::enterRootRib() {
        ribStack.emplace_back(std::make_unique<Rib>(Rib::Kind::Root));
    }

    void NameResolver::enterRib(Rib::Kind kind) {
        if (getDepth() == UINT32_MAX) {
            Logger::devPanic("Maximum ribStack depth limit exceeded");
        }
        ribStack.emplace_back(std::make_unique<Rib>(kind));
    }

    void NameResolver::enterModuleRib(node_id nodeId, Rib::Kind kind) {
        currentModule = currentModule->children.at(nodeId);
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

    // Declarations //
    void NameResolver::declare(const std::string & name, Name::Kind kind, ast::node_id nodeId) {
        log.dev("Declare '", name, "' as ", Name::kindStr(kind));

        const auto & redecl = curRib()->declare(name, kind, nodeId);

        if (redecl) {
            suggestCannotRedeclare(
                name,
                Name::kindStr(kind),
                redecl.unwrap()->kindStr(),
                nodeId,
                redecl.unwrap()->nodeId
            );
        }
    }

    // Resolution //
    void NameResolver::resolveSimplePath(const ast::simple_path_ptr & simplePath) {
        // TODO
        // Simple-dimple LOL
    }

    void NameResolver::resolvePathExpr(RibNamespace ns, const ast::PathExpr & pathExpr) {
        // TODO: global

        if (pathExpr.segments.size() == 1) {
            // Simplest case, we just got an identifier

            // TODO!!!: Keyword segments: self, super, etc.
            // FIXME
            const auto & seg = pathExpr.segments.at(0).unwrap();
            if (seg->ident) {
                const auto & identStr = seg->ident.unwrap().unwrap()->getValue();
                const auto & resolved = resolve(ns, identStr);
                if (not resolved) {
                    log.dev("Failed to resolve '", identStr, "' [", pathExpr.id, "]");
                    suggestErrorMsg("'" + identStr + "' is not defined", pathExpr.span);
                } else {
                    log.dev("Resolved '", identStr, "' [", pathExpr.id, "] as node with id [", resolved, "]");
                    // Set resolution
                    resStorage.setRes(pathExpr.id, resolved.unwrap());
                }
            }
        }
    }

    opt_node_id NameResolver::resolve(RibNamespace ns, const std::string & name) {
        auto depth = getDepth();
        while (true) {
            const auto & rib = ribStack.at(depth);
            auto resolved = rib->resolve(name, ns);
            if (resolved) {
                return resolved.unwrap()->nodeId;
            }
            if (depth == 0) {
                break;
            }
            depth--;
        }
        return dt::None;
    }

    // Suggestions //
    void NameResolver::suggestCannotRedeclare(
        const std::string & name,
        const std::string & as,
        const std::string & declaredAs,
        ast::node_id nodeId,
        ast::node_id declaredHere
    ) {
        suggest(
            std::make_unique<sugg::MsgSpanLinkSugg>(
                "Cannot redeclare '" + name + "' as " + as,
                sess->nodeMap.getNodeSpan(nodeId),
                "Because it is already declared as " + declaredAs + " here",
                sess->nodeMap.getNodeSpan(declaredHere),
                SuggKind::Error
            )
        );
    }

    // Debug //
    void NameResolver::printRib() {
        if (not printRibsFlag or ribStack.empty()) {
            return;
        }
        log.info("Printing rib (`-print=ribs`) at depth [", getDepth(), "]");
        const auto & rib = curRib();
        log.raw("{").nl();
        log.raw("types: ", rib->typeNS).nl();
        log.raw("values: ", rib->valueNS).nl();
        log.raw("lifetimes: ", rib->lifetimeNS).nl();
        log.raw("}").nl();
    }
}
