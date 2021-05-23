#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_ptr> NameResolver::resolve(const ast::item_list & tree) {
        visitItems(tree);

        return {rib, std::move(suggestions)};
    }

    void NameResolver::visit(ast::FuncDecl & funcDecl) {
        uint32_t prevDepth = getDepth();
        visitTypeParams(funcDecl.typeParams);

        enterRib(); // -> (signature rib)
        for (const auto & param : funcDecl.params) {
            param->type->accept(*this);
        }

        if (funcDecl.returnType) {
            funcDecl.returnType.unwrap()->accept(*this);
        }

        enterRib(); // -> (params rib)
        for (const auto & param : funcDecl.params) {
            declare(param->name->unwrapValue(), Name::Kind::Param, param->id);
        }

        if (funcDecl.oneLineBody) {
            funcDecl.oneLineBody.unwrap()->accept(*this);
        } else {
            funcDecl.body.unwrap()->accept(*this);
        }

        liftToDepth(prevDepth); // <- (params rib) <- (all type params ribs)
    }

    void NameResolver::visit(ast::VarStmt & varStmt) {
        enterRib();
        // TODO
    }

    // Statements //
    void NameResolver::visit(ast::ExprStmt & exprStmt) {
        exprStmt.expr->accept(*this);
    }

    void NameResolver::visit(ast::WhileStmt & whileStmt) {
        whileStmt.condition->accept(*this);
        visit(*whileStmt.body);
    }

    // Expressions //
    void NameResolver::visit(ast::Assignment & assign) {
        assign.lhs->accept(*this);
        assign.rhs->accept(*this);
    }

    void NameResolver::visit(ast::Block & block) {
        enterRib(); // -> block rib
        for (const auto & stmt : block.stmts) {
            stmt->accept(*this);
        }
        exitRib(); // <- block rib
    }

    void NameResolver::visit(ast::BorrowExpr & borrowExpr) {
        borrowExpr.expr->accept(*this);
    }

    void NameResolver::visit(ast::BreakExpr & breakExpr) {
        if (breakExpr.expr) {
            breakExpr.expr.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(ast::ContinueExpr & continueExpr) {}

    void NameResolver::visit(ast::DerefExpr & derefExpr) {
        derefExpr.expr->accept(*this);
    }

    void NameResolver::visit(ast::IfExpr & ifExpr) {
        ifExpr.condition->accept(*this);
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap()->accept(*this);
        }
        if (ifExpr.elseBranch) {
            ifExpr.elseBranch.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(ast::Infix & infix) {
        infix.lhs->accept(*this);
        infix.rhs->accept(*this);
    }

    void NameResolver::visit(ast::Invoke & invoke) {
        invoke.lhs->accept(*this);
        visitNamedList(invoke.args);
    }

    void NameResolver::visit(ast::Lambda & lambdaExpr) {
        enterRib(); // -> (lambda params)

        for (const auto & param : lambdaExpr.params) {
            // TODO
//            param.name->accept(*this);
            if (param->type) {
                param->type.unwrap()->accept(*this);
            }
        }

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap()->accept(*this);
        }

        lambdaExpr.body->accept(*this);

        exitRib(); // <- (lambda params)
    }

    void NameResolver::visit(ast::ListExpr & listExpr) {
        for (const auto & el : listExpr.elements) {
            el->accept(*this);
        }
    }

    void NameResolver::visit(ast::LiteralConstant & literalConstant) {}

    void NameResolver::visit(ast::LoopExpr & loopExpr) {
        visit(*loopExpr.body);
    }

    void NameResolver::visit(ast::MemberAccess & memberAccess) {
        memberAccess.lhs->accept(*this);

        // TODO!!!: Resolve field
    }

    void NameResolver::visit(ast::ParenExpr & parenExpr) {
        parenExpr.expr->accept(*this);
    }

    void NameResolver::visit(ast::PathExpr & pathExpr) {
        // TODO!!!
    }

    void NameResolver::visit(ast::Prefix & prefix) {
        prefix.rhs->accept(*this);
    }

    void NameResolver::visit(ast::QuestExpr & questExpr) {
        questExpr.expr->accept(*this);
    }

    void NameResolver::visit(ast::ReturnExpr & returnExpr) {
        if (returnExpr.expr) {
            returnExpr.expr.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(ast::SpreadExpr & spreadExpr) {
        spreadExpr.expr->accept(*this);
    }

    void NameResolver::visit(ast::Subscript & subscript) {
        subscript.lhs->accept(*this);

        for (const auto & index : subscript.indices) {
            index->accept(*this);
        }
    }

    void NameResolver::visit(ast::ThisExpr & thisExpr) {}

    void NameResolver::visit(ast::TupleExpr & tupleExpr) {
        visitNamedList(tupleExpr.elements);
    }

    void NameResolver::visit(ast::UnitExpr & unitExpr) {
        // TODO MEOW?
    }

    void NameResolver::visit(ast::WhenExpr & whenExpr) {
        whenExpr.subject->accept(*this);

        for (const auto & entry : whenExpr.entries) {
            enterRib(); // -> (when entry)
            for (const auto & cond : entry->conditions) {
                cond->accept(*this);
            }
            entry->body->accept(*this);
            exitRib(); // <- (when entry)
        }
    }

    // Types //
    void NameResolver::visit(ast::ParenType & parenType) {
        parenType.type->accept(*this);
    }

    void NameResolver::visit(ast::TupleType & tupleType) {
        for (const auto & el : tupleType.elements) {
            if (el->type) {
                el->type.unwrap()->accept(*this);
            }
        }
    }

    void NameResolver::visit(ast::FuncType & funcType) {
        for (const auto & param : funcType.params) {
            param->accept(*this);
        }
        funcType.returnType->accept(*this);
    }

    void NameResolver::visit(ast::SliceType & listType) {
        listType.type->accept(*this);
    }

    void NameResolver::visit(ast::ArrayType & arrayType) {
        arrayType.type->accept(*this);
        arrayType.sizeExpr->accept(*this);
    }

    void NameResolver::visit(ast::TypePath & typePath) {
        // TODO: !!!
    }

    void NameResolver::visit(ast::UnitType & unitType) {
        // TODO: MEOW?
    }

    // Extended visitors //
    void NameResolver::visitItems(const ast::item_list & members) {
        enterRib(); // -> (members)

        // At first we need to forward all declarations.
        // This is the work for ItemResolver.
        for (const auto & member : members) {
            std::string name;
            Name::Kind kind;
            switch (member->kind) {
                case ast::ItemKind::Func: {
                    name = std::static_pointer_cast<ast::FuncDecl>(member)->name->unwrapValue();
                    kind = Name::Kind::Func;
                } break;
                case ast::ItemKind::Enum: {
                    name = std::static_pointer_cast<ast::Enum>(member)->name->unwrapValue();
                    kind = Name::Kind::Enum;
                } break;
                case ast::ItemKind::Struct: {
                    name = std::static_pointer_cast<ast::Struct>(member)->name->unwrapValue();
                    kind = Name::Kind::Struct;
                } break;
                case ast::ItemKind::TypeAlias: {
                    name = std::static_pointer_cast<ast::TypeAlias>(member)->name->unwrapValue();
                    kind = Name::Kind::TypeAlias;
                } break;
                case ast::ItemKind::Trait: {
                    name = std::static_pointer_cast<ast::Trait>(member)->name->unwrapValue();
                    kind = Name::Kind::Trait;
                } break;
                default: continue;
            }
            declare(name, kind, member->id);
        }

        // Then we resolve the signatures and bodies
        // This is done here -- in NameResolver
        for (const auto & member : members) {
            member->accept(*this);
        }

        exitRib(); // <- (members)
    }

    void NameResolver::visitTypeParams(const ast::opt_type_params & maybeTypeParams) {
        if (!maybeTypeParams) {
            return;
        }
        const auto & typeParams = maybeTypeParams.unwrap();
        enterRib(); // -> (type rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Type) {
                declare(
                    std::static_pointer_cast<ast::GenericType>(typeParam)->name->unwrapValue(),
                    Name::Kind::TypeParam,
                    typeParam->id
                );
            }
        }
        enterRib(); // -> (lifetime rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Lifetime) {
                declare(
                    std::static_pointer_cast<ast::Lifetime>(typeParam)->name->unwrapValue(),
                    Name::Kind::Lifetime,
                    typeParam->id
                );
            }
        }
        enterRib(); // -> (const rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Const) {
                declare(
                    std::static_pointer_cast<ast::ConstParam>(typeParam)->name->unwrapValue(),
                    Name::Kind::ConstParam,
                    typeParam->id
                );
            }
        }
    }

    void NameResolver::visitNamedList(const ast::named_list_ptr & namedList) {
        for (const auto & el : namedList->elements) {
            // Note: We don't visit element `name`, because it is immaterial on name-resolution stage
            if (el->value) {
                el->value.unwrap()->accept(*this);
            }
        }
    }

    // Ribs //
    uint32_t NameResolver::getDepth() const {
        return depth;
    }

    void NameResolver::enterRib() {
        rib = std::make_shared<Rib>(rib);
        depth++;
    }

    void NameResolver::exitRib() {
        auto parent = rib->parent;
        if (!parent) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
        rib = parent.unwrap();
        depth--;
    }

    void NameResolver::liftToDepth(size_t prevDepth) {
        if (prevDepth > depth) {
            common::Logger::devPanic("Called `NameResolver::lifeToDepth` with `prevDepth` > `depth`");
        }

        for (size_t i = prevDepth; i < depth; i++) {
            exitRib();
        }
    }

    // Declarations //
    void NameResolver::declare(const std::string & name, Name::Kind kind, ast::node_id nodeId) {
        const auto & found = rib->names.find(name);
        if (found == rib->names.end()) {
            rib->names.emplace(name, std::make_shared<Name>(kind, nodeId));
            return;
        }
        suggestCannotRedeclare(name, Name::kindStr(kind), found->second->kindStr(), nodeId, found->second->nodeId);
    }

    // Resolution //
    void NameResolver::resolveId(ast::Identifier & id) {
        // TODO: Add allowed resolutions:
        //  When we resolve type name, we should suggest an error if it is a function or something else

        const auto & name = id.unwrapValue();
        opt_node_id resolved;
        dt::Option<rib_ptr> maybeRib = rib;
        while (maybeRib) {
            auto checkRib = maybeRib.unwrap();
            const auto & found = checkRib->names.find(name);
            if (found != checkRib->names.end()) {
                resolved = found->second->nodeId;
                break;
            }
            maybeRib = checkRib->parent;
        }

        if (!resolved) {
            suggestErrorMsg("Cannot find name '" + name + "'", id.id);
            return;
        }

        id.setReference(resolved.unwrap());
    }

    // Suggestions //
    void NameResolver::suggest(sugg::sugg_ptr suggestion) {
        suggestions.emplace_back(std::move(suggestion));
    }

    void NameResolver::suggest(const std::string & msg, ast::node_id nodeId, SuggKind kind, eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, ast::Node::nodeMap.getNodeSpan(nodeId), kind, eid));
    }

    void NameResolver::suggestErrorMsg(const std::string & msg, ast::node_id nodeId, eid_t eid) {
        suggest(msg, nodeId, SuggKind::Error, eid);
    }

    void NameResolver::suggestWarnMsg(const std::string & msg, ast::node_id nodeId, eid_t eid) {
        suggest(msg, nodeId, SuggKind::Warn, eid);
    }

    void NameResolver::suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg) {
        suggest(std::make_unique<sugg::HelpSugg>(helpMsg, std::move(sugg)));
    }

    void NameResolver::suggestCannotRedeclare(
        const std::string & name,
        const std::string & as,
        const std::string & declaredAs,
        ast::node_id nodeId,
        ast::node_id declaredHere
    ) {
        suggest(std::make_unique<sugg::MsgSpanLinkSugg>(
            "Cannot redeclare '"+ name +"' as "+ as,
            ast::Node::nodeMap.getNodeSpan(nodeId),
            "Because it is already declared as "+ declaredAs +" here",
            ast::Node::nodeMap.getNodeSpan(declaredHere),
            SuggKind::Error
        ));
    }
}
