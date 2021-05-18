#include "ast/AstPrinter.h"

namespace jc::ast {
    void AstPrinter::print(const ast::stmt_list & tree) {
        if (tree.empty()) {
            log.debug("Tree is empty");
        }
        for (const auto & stmt : tree) {
            stmt->accept(*this);
            log.nl();
        }
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(EnumDecl * enumDecl) {
        printIndent();

        log.raw("enum ");
        enumDecl->id->accept(*this);

        for (const auto & enumEntry : enumDecl->entries) {
            enumEntry->id->accept(*this);
            if (enumEntry->value) {
                log.raw(" = ");
                enumEntry->value->accept(*this);
            }
            log.raw(",").nl();
        }

        print(enumDecl->body);
    }

    void AstPrinter::visit(ExprStmt * exprStmt) {
        printIndent();

        exprStmt->expr->accept(*this);
    }

    void AstPrinter::visit(ForStmt * forStmt) {
        printIndent();

        log.raw("for ");
        forStmt->forEntity->accept(*this);
        log.raw(" in ");
        forStmt->inExpr->accept(*this);;
        print(forStmt->body);
    }

    void AstPrinter::visit(FuncDecl * funcDecl) {
        printIndent();

        printModifiers(funcDecl->modifiers);
        log.raw("func");
        print(funcDecl->typeParams);
        log.raw(" ");
        funcDecl->id->accept(*this);

        log.raw("(");
        for (size_t i = 0; i < funcDecl->params.size(); ++i) {
            const auto & param = funcDecl->params.at(i);
            param->id.unwrap()->accept(*this);

            if (param->type) {
                log.raw(": ");
                param->type.unwrap()->accept(*this);
            }
            if (param->defaultValue) {
                log.raw(" = ");
                param->defaultValue.unwrap()->accept(*this);
            }
            if (i < funcDecl->params.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(")");

        if (funcDecl->returnType) {
            log.raw(": ");
            funcDecl->returnType.unwrap()->accept(*this);
        }

        if (funcDecl->oneLineBody) {
            log.raw(" = ");
            // For one-line block increment indent to make it prettier
            incIndent();
            funcDecl->oneLineBody.unwrap()->accept(*this);
            decIndent();
        } else {
            log.raw(" ");
            print(funcDecl->body.unwrap());
        }
    }

    void AstPrinter::visit(Impl * impl) {
        printIndent();

        log.raw("impl");
        print(impl->typeParams);
        log.raw(" ");
        impl->traitTypePath->accept(*this);
        log.raw(" for ");
        impl->forType->accept(*this);
        printMembers(impl->members);
    }

    void AstPrinter::visit(Item * item) {
        printIndent();

        print(item->attributes);
        item->stmt->accept(*this);
    }

    void AstPrinter::visit(Struct * _struct) {
        printIndent();

        log.raw("struct ");
        _struct->id->accept(*this);
        printMembers(_struct->members);
    }

    void AstPrinter::visit(Trait * trait) {
        printIndent();

        log.raw("trait ");
        trait->id->accept(*this);
        print(trait->typeParams);

        if (!trait->superTraits.empty()) {
            log.raw(" : ");
        }

        for (size_t i = 0; i < trait->superTraits.size(); i++) {
            trait->superTraits.at(i)->accept(*this);
            if (i < trait->superTraits.size() - 1) {
                log.raw(", ");
            }
        }
        printMembers(trait->members);
    }

    void AstPrinter::visit(TypeAlias * typeAlias) {
        printIndent();

        log.raw("type ");
        typeAlias->id->accept(*this);
        log.raw(" = ");
        typeAlias->type->accept(*this);
    }

    void AstPrinter::visit(VarDecl * varDecl) {
        printIndent();

        log.raw(varDecl->kind.typeToString());
        varDecl->id->accept(*this);
        if (varDecl->type) {
            log.raw(": ");
            varDecl->type->accept(*this);
        }
    }

    void AstPrinter::visit(WhileStmt * whileStmt) {
        printIndent();

        log.raw("while ");
        whileStmt->condition->accept(*this);
        log.raw(" ");
        print(whileStmt->body);
    }

    /////////////////
    // Expressions //
    /////////////////
    void AstPrinter::visit(Assignment * assignment) {
        printIndent();

        assignment->lhs->accept(*this);
        log.raw(" = ");
        assignment->rhs->accept(*this);
    }

    void AstPrinter::visit(BreakExpr * breakExpr) {
        log.raw("break");
    }

    void AstPrinter::visit(ContinueExpr * continueExpr) {
        log.raw("continue");
    }

    void AstPrinter::visit(Identifier * identifier) {
        if (identifier->token.type == parser::TokenType::Id) {
            log.raw(identifier->token.val);
        } else {
            // TODO: Soft keywords
        }
    }

    void AstPrinter::visit(IfExpr * ifExpr) {
        log.raw("if ");
        ifExpr->condition.unwrap()->accept(*this);
        log.raw(" ");
        if (ifExpr->ifBranch) {
            print(ifExpr->ifBranch.unwrap());
        }
        if (ifExpr->elseBranch) {
            log.raw(" else ");
            print(ifExpr->elseBranch.unwrap());
        }
    }

    void AstPrinter::visit(Infix * infix) {
        infix->lhs->accept(*this);
        if (infix->token.type == parser::TokenType::Id) {
            log.raw(infix->token.val);
        } else {
            log.raw(infix->token.typeToString());
        }
        infix->rhs->accept(*this);
    }

    void AstPrinter::visit(Invoke * invoke) {
        invoke->lhs->accept(*this);
        log.raw("(");
        print(invoke->args.get());
        log.raw(")");
    }

    void AstPrinter::visit(ListExpr * listExpr) {
        log.raw("[");
        for (const auto & el : listExpr->elements) {
            el->accept(*this);
        }
        log.raw("]");
    }

    void AstPrinter::visit(LiteralConstant * literalConstant) {
        log.raw(literalConstant->token.val);
    }

    void AstPrinter::visit(LoopExpr * loopExpr) {
        log.raw("loop ");
        print(loopExpr->body);
    }

    void AstPrinter::visit(ParenExpr * parenExpr) {
        log.raw("(");
        parenExpr->expr->accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(PathExpr * pathExpr) {
        if (pathExpr->global) {
            log.raw("::");
        }
        for (const auto & segment : pathExpr->segments) {
            segment->id->accept(*this);
            if (segment->typeParams.size()) {
                log.raw("::");
                print(segment->typeParams);
            }
        }
    }

    void AstPrinter::visit(Postfix * postfix) {
        postfix->lhs->accept(*this);
        log.raw(postfix->token.typeToString());
    }

    void AstPrinter::visit(Prefix * prefix) {
        log.raw(prefix->token.typeToString());
        prefix->rhs->accept(*this);
    }

    void AstPrinter::visit(ReturnExpr * returnExpr) {
        log.raw("return ");
        returnExpr->expr->accept(*this);
    }

    void AstPrinter::visit(SpreadExpr * spreadExpr) {
        log.raw(spreadExpr->token.typeToString());
        spreadExpr->expr->accept(*this);
    }

    void AstPrinter::visit(Subscript * subscript) {
        subscript->lhs->accept(*this);
        for (size_t i = 0; i < subscript->indices.size(); ++i) {
            subscript->indices.at(i)->accept(*this);
            if (i < subscript->indices.size() - 1) {
                log.raw(", ");
            }
        }
    }

    void AstPrinter::visit(SuperExpr * superExpr) {
        log.raw("super");
    }

    void AstPrinter::visit(ThisExpr * thisExpr) {
        log.raw("this");
    }

    void AstPrinter::visit(TupleExpr * tupleExpr) {
        log.raw("(");
        print(tupleExpr->elements.get());
        log.raw(")");
    }

    void AstPrinter::visit(UnitExpr * unitExpr) {
        log.raw("()");
    }

    void AstPrinter::visit(WhenExpr * whenExpr) {
        log.raw("when ");
        whenExpr->subject->accept(*this);

        log.raw("{");
        incIndent();

        for (const auto & entry : whenExpr->entries) {
            for (size_t i = 0; i < entry->conditions.size(); ++i) {
                const auto & cond = entry->conditions.at(i);
                cond->accept(*this);
                if (i < entry->conditions.size() - 1) {
                    log.raw(", ");
                }
            }
            log.raw(" => ");
        }

        decIndent();
        log.raw("}");
    }

    void AstPrinter::visit(ParenType * parenType) {
        log.raw("(");
        parenType->type->accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(TupleType * tupleType) {
        log.raw("(");
        for (size_t i = 0; i < tupleType->elements.size(); i++) {
            const auto & el = tupleType->elements.at(i);
            if (el->id) {
                el->id.unwrap()->accept(*this);
            }
            if (el->type) {
                if (el->id) {
                    log.raw(": ");
                }
                el->type.unwrap()->accept(*this);
            }
            if (i < tupleType->elements.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(")");
    }

    void AstPrinter::visit(FuncType * funcType) {
        log.raw("(");
        print(funcType->params);
        log.raw(") -> ");
        funcType->returnType->accept(*this);
    }

    void AstPrinter::visit(ArrayType * listType) {
        log.raw("[");
        listType->type->accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(TypePath * typePath) {
        if (typePath->global) {
            log.raw("::");
        }
        for (size_t i = 0; i < typePath->ids.size(); i++) {
            print(typePath->ids.at(i).get());
            if (i < typePath->ids.size() - 1) {
                log.raw("::");
            }
        }
    }

    void AstPrinter::visit(UnitType * unitType) {
        log.raw("()");
    }

    void AstPrinter::printIndent() const {
        for (int i = 0; i < indent; i++) {
            log.raw(indentChar);
        }
    }

    void AstPrinter::print(const ast::attr_list & attributes) {
        for (const auto & attr : attributes) {
            log.raw("@");
            attr->id->accept(*this);
            log.raw("(");
            print(attr->params.get());
            log.raw(")").nl();
        }
    }

    void AstPrinter::printModifiers(const parser::token_list & modifiers) {
        for (const auto & mod : modifiers) {
            log.raw(mod.typeToString());
        }
    }

    void AstPrinter::print(const ast::type_param_list & typeParams) {
        if (typeParams.empty()) {
            return;
        }

        log.raw("<");
        for (const auto & typeParam : typeParams) {
            typeParam->id->accept(*this);
            if (typeParam->type) {
                log.raw(": ");
                typeParam->type->accept(*this);
            }
        }
        log.raw(">");
    }

    void AstPrinter::print(const ast::delegation_list & delegations) {
        if (delegations.empty()) {
            return;
        }

        log.raw(" : ");
        for (const auto & del : delegations) {
            del->id->accept(*this);
        }
    }

    void AstPrinter::print(const ast::block_ptr & block) {
        if (block->stmts.empty()) {
            log.raw("{}");
            return;
        }

        log.raw("{").nl();
        incIndent();
        print(block->stmts);
        decIndent();
        // Closing brace must go on the "super"-indent
        printIndent();
        log.raw("}");
    }

    void AstPrinter::print(ast::NamedList * namedList) {
        for (size_t i = 0; i < namedList->elements.size(); i++) {
            const auto & namedEl = namedList->elements.at(i);
            if (namedEl->id) {
                namedEl->id.unwrap()->accept(*this);
                if (namedEl->value) {
                    log.raw(" = ");
                }
            }
            if (namedEl->value) {
                namedEl->value.unwrap()->accept(*this);
            }
            if (i < namedList->elements.size() - 1) {
                log.raw(", ");
            }
        }
    }

    void AstPrinter::print(const ast::type_list & typeList) {
        for (size_t i = 0; i < typeList.size(); i++) {
            typeList.at(i)->accept(*this);
            if (i < typeList.size() - 1) {
                log.raw(", ");
            }
        }
    }

    void AstPrinter::print(IdType * idType) {
        idType->id->accept(*this);
        print(idType->typeParams);
    }

    void AstPrinter::printMembers(const stmt_list & members) {
        log.raw(" {");
        incIndent();
        for (const auto & stmt : members) {
            stmt->accept(*this);
            log.nl();
        }
        log.raw("}");
    }

    void AstPrinter::incIndent() {
        ++indent;
    }

    void AstPrinter::decIndent() {
        --indent;
    }
}
