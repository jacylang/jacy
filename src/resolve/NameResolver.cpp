#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_stack> NameResolver::resolve(const ast::item_list & tree) {
        visitItems(tree);

        return {ribStack, std::move(suggestions)};
    }

    void NameResolver::visit(ast::Func & funcDecl) {
        uint32_t prevDepth = getDepth();
        visitTypeParams(funcDecl.typeParams);

        enterNormalRib(); // -> (signature rib)
        for (const auto & param : funcDecl.params) {
            param->type->accept(*this);
        }

        if (funcDecl.returnType) {
            funcDecl.returnType.unwrap()->accept(*this);
        }

        enterNormalRib(); // -> (params rib)
        for (const auto & param : funcDecl.params) {
            declare(param->name->unwrapValue(), Name::Kind::Param, param->id);
        }

        if (funcDecl.oneLineBody) {
            funcDecl.oneLineBody.unwrap()->accept(*this);
        } else {
            funcDecl.body.unwrap()->accept(*this);
        }

        liftToDepth(prevDepth);
    }

//    void NameResolver::visit(ast::Impl & impl) {}

    void NameResolver::visit(ast::Struct & _struct) {
        uint32_t prevDepth = getDepth();
        visitTypeParams(_struct.typeParams);

        auto structRib = std::make_shared<StructRib>(_struct.name->id);
        enterRib(structRib); // -> (item rib)

        for (const auto & field : _struct.fields) {
            structRib->fields.emplace(field->name->unwrapValue(), field->id);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(ast::VarStmt & varStmt) {
        enterNormalRib();
        declare(varStmt.name->unwrapValue(), Name::Kind::Local, varStmt.id);
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
        enterNormalRib(); // -> block rib
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

    void NameResolver::visit(ast::ContinueExpr & continueExpr) {
    }

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
        enterNormalRib(); // -> (lambda params)

        for (const auto & param : lambdaExpr.params) {
            // TODO: Param name
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

    void NameResolver::visit(ast::LiteralConstant & literalConstant) {
    }

    void NameResolver::visit(ast::LoopExpr & loopExpr) {
        visit(*loopExpr.body);
    }

    void NameResolver::visit(ast::MemberAccess & memberAccess) {
        memberAccess.lhs->accept(*this);
        // Note: As far as lhs is unknown expression, we cannot check fields on this stage.
        //  At first we need to infer the type of lhs, and then type-check it whereas it is a structure
        //  and has certain field
    }

    void NameResolver::visit(ast::ParenExpr & parenExpr) {
        parenExpr.expr->accept(*this);
    }

    void NameResolver::visit(ast::PathExpr & pathExpr) {
        // TODO: global
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

    void NameResolver::visit(ast::ThisExpr & thisExpr) {
    }

    void NameResolver::visit(ast::TupleExpr & tupleExpr) {
        visitNamedList(tupleExpr.elements);
    }

    void NameResolver::visit(ast::UnitExpr & unitExpr) {
        // TODO MEOW?
    }

    void NameResolver::visit(ast::WhenExpr & whenExpr) {
        whenExpr.subject->accept(*this);

        for (const auto & entry : whenExpr.entries) {
            enterNormalRib(); // -> (when entry)
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
        enterNormalRib(); // -> (members)

        // At first we need to forward all declarations.
        // This is the work for ItemResolver.
        for (const auto & member : members) {
            std::string name;
            Name::Kind kind;
            switch (member->kind) {
                case ast::ItemKind::Func: {
                    name = std::static_pointer_cast<ast::Func>(member)->name->unwrapValue();
                    kind = Name::Kind::Func;
                    break;
                }
                case ast::ItemKind::Enum: {
                    name = std::static_pointer_cast<ast::Enum>(member)->name->unwrapValue();
                    kind = Name::Kind::Enum;
                    break;
                }
                case ast::ItemKind::Struct: {
                    name = std::static_pointer_cast<ast::Struct>(member)->name->unwrapValue();
                    kind = Name::Kind::Struct;
                    break;
                }
                case ast::ItemKind::TypeAlias: {
                    name = std::static_pointer_cast<ast::TypeAlias>(member)->name->unwrapValue();
                    kind = Name::Kind::TypeAlias;
                    break;
                }
                case ast::ItemKind::Trait: {
                    name = std::static_pointer_cast<ast::Trait>(member)->name->unwrapValue();
                    kind = Name::Kind::Trait;
                    break;
                }
                default:
                    continue;
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
        enterNormalRib(); // -> (type rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Type) {
                declare(
                    std::static_pointer_cast<ast::GenericType>(typeParam)->name->unwrapValue(),
                    Name::Kind::TypeParam,
                    typeParam->id
                );
            }
        }
        enterNormalRib(); // -> (lifetime rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Lifetime) {
                declare(
                    std::static_pointer_cast<ast::Lifetime>(typeParam)->name->unwrapValue(),
                    Name::Kind::Lifetime,
                    typeParam->id
                );
            }
        }
        enterNormalRib(); // -> (const rib)
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

    rib_ptr NameResolver::curRib() const {
        return ribStack.at(depth);
    }

    opt_rib NameResolver::ribAt(size_t d) const {
        if (d >= ribStack.size()) {
            return dt::None;
        }
        return ribStack.at(d);
    }

    void NameResolver::enterNormalRib() {
        enterRib(std::make_shared<Rib>(Rib::Kind::Normal));
    }

    void NameResolver::enterItemRib(node_id nameNodeId) {
        enterRib(std::make_shared<ItemRib>(nameNodeId));
    }

    void NameResolver::enterRib(const rib_ptr & nestedRib) {
        ribStack.push_back(nestedRib);
        depth++;
    }

    void NameResolver::exitRib() {
        if (depth == 0) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
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
        const auto & found = curRib()->names.find(name);
        if (found == curRib()->names.end()) {
            curRib()->names.emplace(name, std::make_shared<Name>(kind, nodeId));
            return;
        }
        suggestCannotRedeclare(name, Name::kindStr(kind), found->second->kindStr(), nodeId, found->second->nodeId);
    }

    // Resolution //
    void NameResolver::resolveId(ast::Identifier & id, Name::Usage usage) {
        // TODO: Add allowed resolutions:
        //  When we resolve type name, we should suggest an error if it is a function or something else

        const auto & name = id.unwrapValue();

        dt::Option<name_ptr> nearestResolution;
        opt_node_id resolved;

        auto curDepth = depth;
        dt::Option<rib_ptr> maybeRib = curRib();
        while (maybeRib) {
            auto checkRib = maybeRib.unwrap();
            const auto & found = checkRib->names.find(name);
            if (found != checkRib->names.end()) {
                // Save nearest name we found
                nearestResolution = found->second;
                if (found->second->isUsableAs(usage)) {
                    // If name, we found is of allowed kind we use it
                    resolved = found->second->nodeId;
                    break;
                }
            }
            curDepth--;
            maybeRib = ribAt(curDepth);
        }

        if (resolved) {
            id.setReference(resolved.unwrap());
            return;
        }

        if (nearestResolution) {
            suggestErrorMsg(
                "Cannot find suitable item for '" + name + "', nearest item is a " +
                nearestResolution.unwrap()->kindStr() + " that cannot be used as " + Name::usageToString(usage),
                id.id
            );
        } else {
            suggestErrorMsg("Cannot find name '" + name + "'", id.id);
        }
    }

    void NameResolver::resolvePath(bool global, const ast::id_t_list & segments) {
        if (global) {
            common::Logger::notImplemented("Global path resolution");
        }

        if (segments.empty()) {
            common::Logger::devPanic("Called `resolvePath` with 0 size segments");
        }

        auto curDepth = getDepth();
        size_t segmentIndex = 0;
        opt_rib maybeRib = ribAt(curDepth);
        while (maybeRib) {
            const auto & segment = segments.at(segmentIndex);
            const auto & checkRib = maybeRib.unwrap();

        }
    }

//    std::string NameResolver::getNameByNodeId(node_id nameNodeId) {
//        const auto & idNode = std::dynamic_pointer_cast<ast::Identifier>(ast::Node::nodeMap.getNodePtr(nameNodeId));
//        if (!idNode) {
//            common::Logger::devPanic("Called `getNameByNodeId` with non-Identifier nameNodeId");
//        }
//        return idNode->unwrapValue();
//    }

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
        suggest(
            std::make_unique<sugg::MsgSpanLinkSugg>(
                "Cannot redeclare '" + name + "' as " + as,
                ast::Node::nodeMap.getNodeSpan(nodeId),
                "Because it is already declared as " + declaredAs + " here",
                ast::Node::nodeMap.getNodeSpan(declaredHere),
                SuggKind::Error
            )
        );
    }
}
