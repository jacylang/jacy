#include "resolve/NameResolver.h"

namespace jc::resolve {
    dt::SuggResult<rib_stack> NameResolver::resolve(sess::sess_ptr sess, const ast::item_list & tree) {
        typeResolver = std::make_unique<TypeResolver>(sess);
        itemResolver = std::make_unique<ItemResolver>(sess);

        enterRib();

        for (const auto & item : tree) {
            item->accept(*this);
        }

        return {ribs, moveConcat(
            typeResolver->extractSuggestions(),
            itemResolver->extractSuggestions()
        )};
    }

    // Statements //
    void NameResolver::visit(ast::ExprStmt * exprStmt) {
        exprStmt->expr->accept(*this);
    }

    void NameResolver::visit(ast::FuncDecl * funcDecl) {
        funcDecl->accept(*itemResolver);

        enterRib(); // -> (func params)
        funcDecl->accept(*typeResolver);

        if (funcDecl->oneLineBody) {
            funcDecl->oneLineBody.unwrap()->accept(*this);
        } else {
            visit(funcDecl->body.unwrap().get());
        }

        exitRib(); // <- (func params)
    }

    void NameResolver::visit(ast::Impl * impl) {
        impl->accept(*itemResolver);
    }

    void NameResolver::visit(ast::Item * item) {
        item->stmt->accept(*this);
    }

    void NameResolver::visit(ast::Struct * _struct) {
        _struct->accept(*itemResolver);

        enterRib(); // -> (type rib)
        _struct->accept(*typeResolver);

        visitMembers(_struct->members);

        exitRib(); // <- (type rib)
    }

    void NameResolver::visit(ast::Trait * trait) {
        trait->accept(*itemResolver);

        enterRib(); // -> (type rib)

        auto typeRib = ribs.top();
        trait->accept(*typeResolver);

        for (const auto & superTrait : trait->superTraits) {
            superTrait->accept(*this);
        }

        visitMembers(trait->members);

        exitRib(); // <- (type rib)
    }

    void NameResolver::visit(ast::TypeAlias * typeAlias) {
        typeAlias->accept(*itemResolver);

        // Note: For now this rib for typeAlias type is useless, but in the future, we'll have generic type aliases
        enterRib(); // -> (type rib)
        typeAlias->accept(*typeResolver);
        exitRib(); // <- (type rib)
    }

    void NameResolver::visit(ast::VarDecl * varDecl) {
        // FIXME: Visit by LocalResolver
        varDecl->accept(*typeResolver);
    }

    void NameResolver::visit(ast::WhileStmt * whileStmt) {
        whileStmt->condition->accept(*this);
        visit(whileStmt->body.get());
    }

    // Expressions //
    void NameResolver::visit(ast::Assignment * assign) {
        assign->lhs->accept(*this);
        assign->rhs->accept(*this);
    }

    void NameResolver::visit(ast::Block * block) {
        enterRib(); // -> block rib
        for (const auto & stmt : block->stmts) {
            stmt->accept(*this);
        }
        exitRib(); // <- block rib
    }

    void NameResolver::visit(ast::BorrowExpr * borrowExpr) {
        borrowExpr->expr->accept(*this);
    }

    void NameResolver::visit(ast::BreakExpr * breakExpr) {
        if (breakExpr->expr) {
            breakExpr->expr.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(ast::ContinueExpr * continueExpr) {}

    void NameResolver::visit(ast::DerefExpr * derefExpr) {
        derefExpr->expr->accept(*this);
    }

    void NameResolver::visit(ast::Identifier * identifier) {
        // Note: If we have standalone identifier like that,
        //  we 100% sure that it is inside of an expression.
        //  Just because TypeResolver and ItemResolver handles identifiers that're parts
        //  of types or items already.
        //  So, if we have identifier here...
        //  It MUST be any kind of variable or item (!)

        resolveId(identifier->getValue());
    }

    void NameResolver::visit(ast::IfExpr * ifExpr) {
        ifExpr->condition->accept(*this);
        if (ifExpr->ifBranch) {
            ifExpr->ifBranch.unwrap()->accept(*this);
        }
        if (ifExpr->elseBranch) {
            ifExpr->elseBranch.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(ast::Infix * infix) {
        infix->lhs->accept(*this);
        infix->rhs->accept(*this);
    }

    void NameResolver::visit(ast::Invoke * invoke) {
        invoke->lhs->accept(*this);
        visitNamedList(invoke->args);
    }

    void NameResolver::visit(ast::Lambda * lambdaExpr) {
        enterRib(); // -> (lambda params)

        for (const auto & param : lambdaExpr->params) {
            param->name->accept(*this);
            if (param->type) {
                param->type.unwrap()->accept(*this);
            }
        }

        if (lambdaExpr->returnType) {
            lambdaExpr->returnType.unwrap()->accept(*this);
        }

        lambdaExpr->body->accept(*this);

        exitRib(); // <- (lambda params)
    }

    void NameResolver::visit(ast::ListExpr * listExpr) {
        for (const auto & el : listExpr->elements) {
            el->accept(*this);
        }
    }

    void NameResolver::visit(ast::LiteralConstant * literalConstant) {}

    void NameResolver::visit(ast::LoopExpr * loopExpr) {
        visit(loopExpr->body.get());
    }

    void NameResolver::visit(ast::MemberAccess * memberAccess) {
        memberAccess->lhs->accept(*this);

        // TODO!!!: Resolve field
    }

    void NameResolver::visit(ast::ParenExpr * parenExpr) {
        parenExpr->expr->accept(*this);
    }

    void NameResolver::visit(ast::PathExpr * pathExpr) {
        // TODO!!!
    }

    void NameResolver::visit(ast::Prefix * prefix) {
        prefix->rhs->accept(*this);
    }

    void NameResolver::visit(ast::QuestExpr * questExpr) {
        questExpr->expr->accept(*this);
    }

    void NameResolver::visit(ast::ReturnExpr * returnExpr) {
        if (returnExpr->expr) {
            returnExpr->expr.unwrap()->accept(*this);
        }
    }

    void NameResolver::visit(ast::SpreadExpr * spreadExpr) {
        spreadExpr->expr->accept(*this);
    }

    void NameResolver::visit(ast::Subscript * subscript) {
        subscript->lhs->accept(*this);

        for (const auto & index : subscript->indices) {
            index->accept(*this);
        }
    }

    void NameResolver::visit(ast::ThisExpr * thisExpr) {}

    void NameResolver::visit(ast::TupleExpr * tupleExpr) {
        visitNamedList(tupleExpr->elements);
    }

    void NameResolver::visit(ast::UnitExpr * unitExpr) {
        // TODO MEOW?
    }

    void NameResolver::visit(ast::WhenExpr * whenExpr) {
        whenExpr->subject->accept(*this);

        for (const auto & entry : whenExpr->entries) {
            enterRib(); // -> (when entry)
            for (const auto & cond : entry->conditions) {
                cond->accept(*this);
            }
            entry->body->accept(*this);
            exitRib(); // <- (when entry)
        }
    }

    // Types //
    void NameResolver::visit(ast::ParenType * parenType) {
        parenType->type->accept(*this);
    }

    void NameResolver::visit(ast::TupleType * tupleType) {
        for (const auto & el : tupleType->elements) {
            if (el->name) {
                el->name.unwrap()->accept(*this);
            }
            if (el->type) {
                el->type.unwrap()->accept(*this);
            }
        }
    }

    void NameResolver::visit(ast::FuncType * funcType) {
        for (const auto & param : funcType->params) {
            param->accept(*this);
        }
        funcType->returnType->accept(*this);
    }

    void NameResolver::visit(ast::SliceType * listType) {
        listType->type->accept(*this);
    }

    void NameResolver::visit(ast::ArrayType * arrayType) {
        arrayType->type->accept(*this);
        arrayType->sizeExpr->accept(*this);
    }

    void NameResolver::visit(ast::TypePath * typePath) {
        // TODO: !!!
    }

    void NameResolver::visit(ast::UnitType * unitType) {
        // TODO: MEOW?
    }

    // Extended visitors //
    void NameResolver::visitMembers(const ast::item_list & members) {
        enterRib(); // -> (members)
        for (const auto & member : members) {
            member->accept(*this);
        }
        exitRib(); // <- (members)
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
    void NameResolver::enterRib() {
        auto newRib = std::make_shared<Rib>();
        ribs.push(newRib);
        enterSpecificRib(newRib);
    }

    void NameResolver::exitRib() {
        if (ribs.empty()) {
            Logger::devPanic("NameResolver: Tried to exit top-level rib");
        }
        enterSpecificRib(ribs.top());
    }

    /**
     * enterSpecificRib
     * @brief Does not modify ribStack and just sets current rib to specific.
     *  Very useful when we need to go to some rib we saved and resolve names there.
     * @param rib
     */
    void NameResolver::enterSpecificRib(const rib_ptr & rib) {
        typeResolver->acceptRib(rib);
        itemResolver->acceptRib(rib);
    }

    // Resolution //
    opt_node_id NameResolver::resolveId(const std::string & name) {
        dt::Option<rib_ptr> maybeRib = ribs.top();
        while (maybeRib) {
            const auto & checkRib = maybeRib.unwrap();
            const auto & local = checkRib->locals.find(name);
            if (local != checkRib->locals.end()) {
                return local->second->nodeId;
            }
            const auto & item = checkRib->items.find(name);
            if (item != checkRib->items.end()) {
                return item->second->nodeId;
            }
            maybeRib = ribs.top();
        }
        return dt::None;
    }
}
