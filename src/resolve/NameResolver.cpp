#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_stack> NameResolver::resolve(const sess::sess_ptr & sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootModule()->accept(*this);

        return {ribStack, std::move(extractSuggestions())};
    }

    void NameResolver::visit(const ast::FileModule & fileModule) {
        for (const auto & item : fileModule.getFile()->items) {
            item.accept(*this);
        }
    }

    void NameResolver::visit(const ast::DirModule & dirModule) {
        for (const auto & module : dirModule.getModules()) {
            module->accept(*this);
        }
    }

    void NameResolver::visit(const ast::Func & func) {
        uint32_t prevDepth = getDepth();
        visitTypeParams(func.typeParams);

        enterRib(Rib::Kind::Type); // -> (signature rib)
        for (const auto & param : func.params) {
            param->type.accept(*this);
        }

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        enterRib(Rib::Kind::Value); // -> (params rib)
        for (const auto & param : func.params) {
            declare(param->name.unwrap()->getValue(), Name::Kind::Param, param->name.unwrap()->id);
        }

        if (func.oneLineBody) {
            func.oneLineBody.unwrap().accept(*this);
        } else {
            func.body.unwrap()->accept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::Mod & mod) {
        enterRib(Rib::Kind::Value);

        visitItems(mod.items);

        exitRib();
    }

    void NameResolver::visit(const ast::Struct & _struct) {
        uint32_t prevDepth = getDepth();
        visitTypeParams(_struct.typeParams);

        enterRib(Rib::Kind::Value); // -> (item rib)

        for (const auto & field : _struct.fields) {
            field->type.accept(*this);
        }

        liftToDepth(prevDepth);
    }

    void NameResolver::visit(const ast::UseDecl & useDecl) {
        resolveUseTree(useDecl.useTree);
    }

    void NameResolver::resolveUseTree(const ast::use_tree_ptr & maybeUseTree) {
        // TODO: Everything goes harder
    }

    // Statements //
    void NameResolver::visit(const ast::ExprStmt & exprStmt) {
        exprStmt.expr.accept(*this);
    }

    void NameResolver::visit(const ast::VarStmt & varStmt) {
        enterRib(Rib::Kind::Value);
        declare(varStmt.name.unwrap()->getValue(), Name::Kind::Local, varStmt.id);
    }

    void NameResolver::visit(const ast::WhileStmt & whileStmt) {
        whileStmt.condition.accept(*this);
        visit(*whileStmt.body);
    }

    // Expressions //
    void NameResolver::visit(const ast::Assignment & assign) {
        assign.lhs.accept(*this);
        assign.rhs.accept(*this);
    }

    void NameResolver::visit(const ast::Block & block) {
        enterRib(Rib::Kind::Value); // -> block rib
        for (const auto & stmt : block.stmts) {
            stmt.accept(*this);
        }
        exitRib(); // <- block rib
    }

    void NameResolver::visit(const ast::BorrowExpr & borrowExpr) {
        borrowExpr.expr.accept(*this);
    }

    void NameResolver::visit(const ast::BreakExpr & breakExpr) {
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }
    }

    void NameResolver::visit(const ast::ContinueExpr & continueExpr) {
    }

    void NameResolver::visit(const ast::DerefExpr & derefExpr) {
        derefExpr.expr.accept(*this);
    }

    void NameResolver::visit(const ast::IfExpr & ifExpr) {
        ifExpr.condition.accept(*this);
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap()->accept(*this);
        }
        if (ifExpr.elseBranch) {
            ifExpr.elseBranch.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(const ast::Infix & infix) {
        infix.lhs.accept(*this);
        infix.rhs.accept(*this);
    }

    void NameResolver::visit(const ast::Invoke & invoke) {
        invoke.lhs.accept(*this);
        visitNamedList(invoke.args);
    }

    void NameResolver::visit(const ast::Lambda & lambdaExpr) {
        enterRib(Rib::Kind::Value); // -> (lambda params)

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

    void NameResolver::visit(const ast::ListExpr & listExpr) {
        for (const auto & el : listExpr.elements) {
            el.accept(*this);
        }
    }

    void NameResolver::visit(const ast::LiteralConstant & literalConstant) {
    }

    void NameResolver::visit(const ast::LoopExpr & loopExpr) {
        visit(*loopExpr.body);
    }

    void NameResolver::visit(const ast::MemberAccess & memberAccess) {
        memberAccess.lhs.accept(*this);
        // Note: As far as lhs is unknown expression, we cannot check fields on this stage.
        //  At first we need to infer the type of lhs, and then type-check it whereas it is a structure
        //  and has certain field
    }

    void NameResolver::visit(const ast::ParenExpr & parenExpr) {
        parenExpr.expr.accept(*this);
    }

    void NameResolver::visit(const ast::PathExpr & pathExpr) {
        // TODO: global


    }

    void NameResolver::visit(const ast::Prefix & prefix) {
        prefix.rhs.accept(*this);
    }

    void NameResolver::visit(const ast::QuestExpr & questExpr) {
        questExpr.expr.accept(*this);
    }

    void NameResolver::visit(const ast::ReturnExpr & returnExpr) {
        if (returnExpr.expr) {
            returnExpr.expr.unwrap().accept(*this);
        }
    }

    void NameResolver::visit(const ast::SpreadExpr & spreadExpr) {
        spreadExpr.expr.accept(*this);
    }

    void NameResolver::visit(const ast::StructExpr & structExpr) {
        structExpr.path.accept(*this);

        for (const auto & maybeField : structExpr.fields) {
            const auto & field = maybeField.unwrap();
            switch (field->kind) {
                case ast::StructExprField::Kind::Raw: {
                    field->expr->accept(*this);
                    break;
                }
                case ast::StructExprField::Kind::Base: {
                    field->expr->accept(*this);
                    break;
                }
                default:;
            }
        }
    }

    void NameResolver::visit(const ast::Subscript & subscript) {
        subscript.lhs.accept(*this);

        for (const auto & index : subscript.indices) {
            index.accept(*this);
        }
    }

    void NameResolver::visit(const ast::ThisExpr & thisExpr) {
    }

    void NameResolver::visit(const ast::TupleExpr & tupleExpr) {
        visitNamedList(tupleExpr.elements);
    }

    void NameResolver::visit(const ast::UnitExpr & unitExpr) {
        // TODO MEOW?
    }

    void NameResolver::visit(const ast::WhenExpr & whenExpr) {
        whenExpr.subject.accept(*this);

        for (const auto & entry : whenExpr.entries) {
            enterRib(Rib::Kind::Value); // -> (when entry)
            for (const auto & cond : entry->conditions) {
                cond.accept(*this);
            }
            entry->body->accept(*this);
            exitRib(); // <- (when entry)
        }
    }

    // Types //
    void NameResolver::visit(const ast::ParenType & parenType) {
        parenType.type.accept(*this);
    }

    void NameResolver::visit(const ast::TupleType & tupleType) {
        for (const auto & el : tupleType.elements) {
            if (el->type) {
                el->type.unwrap().accept(*this);
            }
        }
    }

    void NameResolver::visit(const ast::FuncType & funcType) {
        for (const auto & param : funcType.params) {
            param.accept(*this);
        }
        funcType.returnType.accept(*this);
    }

    void NameResolver::visit(const ast::SliceType & listType) {
        listType.type.accept(*this);
    }

    void NameResolver::visit(const ast::ArrayType & arrayType) {
        arrayType.type.accept(*this);
        arrayType.sizeExpr.accept(*this);
    }

    void NameResolver::visit(const ast::TypePath & typePath) {
        // TODO: !!!
    }

    void NameResolver::visit(const ast::UnitType & unitType) {
        // TODO: MEOW?
    }

    // Extended visitors //
    void NameResolver::visitItems(const ast::item_list & members) {
        enterRib(Rib::Kind::Value); // -> (members)

        // At first we need to forward all declarations.
        // This is the work for ItemResolver.
        for (const auto & maybeMember : members) {
            const auto & member = maybeMember.unwrap();
            std::string name;
            Name::Kind kind;
            switch (member->kind) {
                case ast::ItemKind::Func: {
                    name = std::static_pointer_cast<ast::Func>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Func;
                    break;
                }
                case ast::ItemKind::Enum: {
                    name = std::static_pointer_cast<ast::Enum>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Enum;
                    break;
                }
                case ast::ItemKind::Struct: {
                    name = std::static_pointer_cast<ast::Struct>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Struct;
                    break;
                }
                case ast::ItemKind::TypeAlias: {
                    name = std::static_pointer_cast<ast::TypeAlias>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::TypeAlias;
                    break;
                }
                case ast::ItemKind::Trait: {
                    name = std::static_pointer_cast<ast::Trait>(member)->name.unwrap()->getValue();
                    kind = Name::Kind::Trait;
                    break;
                }
                default: continue;
            }
            declare(name, kind, member->id);
        }

        // Then we resolve the signatures and bodies
        // This is done here -- in NameResolver
        for (const auto & member : members) {
            member.accept(*this);
        }

        exitRib(); // <- (members)
    }

    void NameResolver::visitTypeParams(const ast::opt_type_params & maybeTypeParams) {
        if (!maybeTypeParams) {
            return;
        }
        const auto & typeParams = maybeTypeParams.unwrap();
        enterRib(Rib::Kind::Type); // -> (type rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Type) {
                declare(
                    std::static_pointer_cast<ast::GenericType>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::TypeParam,
                    typeParam->id
                );
            }
        }
        enterRib(Rib::Kind::Lifetime); // -> (lifetime rib)
        for (const auto & typeParam : typeParams) {
            if (typeParam->kind == ast::TypeParamKind::Lifetime) {
                declare(
                    std::static_pointer_cast<ast::Lifetime>(typeParam)->name.unwrap()->getValue(),
                    Name::Kind::Lifetime,
                    typeParam->id
                );
            }
        }
        enterRib(Rib::Kind::Value); // -> (const rib)
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

    void NameResolver::visitNamedList(const ast::named_list & namedList) {
        for (const auto & el : namedList) {
            // Note: We don't visit element `name`, because it is immaterial on name-resolution stage
            if (el->value) {
                el->value.unwrap().accept(*this);
            }
        }
    }

    // Ribs //
    uint32_t NameResolver::getDepth() const {
        return depth;
    }

    const rib_ptr & NameResolver::curRib() const {
        try {
            return ribStack.at(depth);
        } catch (std::exception & e) {
            common::Logger::devPanic("Called `NameResolver::curRib` with depth out of `ribStack` bounds:", depth);
        }
    }

    opt_rib NameResolver::ribAt(size_t d) const {
        if (d >= ribStack.size()) {
            return dt::None;
        }
        return ribStack.at(d);
    }

    void NameResolver::enterRib(Rib::Kind kind) {
        ribStack.push_back(std::make_unique<Rib>(kind));
        depth = ribStack.size() - 1;
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
    void NameResolver::resolveSimplePath(const ast::simple_path_ptr & simplePath) {
        // TODO
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
}
