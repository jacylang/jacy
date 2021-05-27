#include "ast/AstPrinter.h"

namespace jc::ast {
    void AstPrinter::print(const Party & party, AstPrinterMode mode) {
        this->mode = mode;

        party.getRootModule()->accept(*this);
    }

    void AstPrinter::visit(const File & file) {
        for (const auto & item : file.items) {
            item->accept(*this);
        }
    }

    void AstPrinter::visit(const ErrorStmt & errorStmt) {
        log.raw("[ERROR STMT]");
    }

    void AstPrinter::visit(const ErrorExpr & errorExpr) {
        log.raw("[ERROR EXPR]");
    }

    void AstPrinter::visit(const ErrorType & errorType) {
        log.raw("[ERROR TYPE]");
    }

    void AstPrinter::visit(const ErrorTypePath & errorTypePath) {
        log.raw("[ERROR TYPEPATH]");
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const Enum & enumDecl) {
        printIndent();

        log.notImplemented("AstPrinter:enumDecl");

//        log.raw("enum ");
//        enumDecl.id->accept(*this);
//
//        for (const auto & enumEntry : enumDecl->entries) {
//            enumEntry.id->accept(*this);
//            if (enumEntry->value) {
//                log.raw(" = ");
//                enumEntry.value->accept(*this);
//            }
//            log.raw(",").nl();
//        }
//
//        print(enumDecl->body);
    }

    void AstPrinter::visit(const ExprStmt & exprStmt) {
        printIndent();

        exprStmt.expr->accept(*this);
    }

    void AstPrinter::visit(const ForStmt & forStmt) {
        printIndent();

        log.raw("for ");
        // TODO: Update when `for` will have patterns
        printId(forStmt.forEntity);
        log.raw(" in ");
        forStmt.inExpr->accept(*this);
        forStmt.body->accept(*this);
    }

    void AstPrinter::visit(const ItemStmt & itemStmt) {
        // TODO: Print attributes
        itemStmt.item->accept(*this);
    }

    void AstPrinter::visit(const Func & funcDecl) {
        printIndent();

        printModifiers(funcDecl.modifiers);
        log.raw("func");
        print(funcDecl.typeParams);
        log.raw(" ");
        printId(funcDecl.name);

        log.raw("(");
        for (size_t i = 0; i < funcDecl.params.size(); ++i) {
            const auto & param = funcDecl.params.at(i);
            log.raw(param->name);

            if (param->type) {
                log.raw(": ");
                param->type->accept(*this);
            }
            if (param->defaultValue) {
                log.raw(" = ");
                param->defaultValue.unwrap()->accept(*this);
            }
            if (i < funcDecl.params.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(")");

        if (funcDecl.returnType) {
            log.raw(": ");
            funcDecl.returnType.unwrap()->accept(*this);
        }

        if (funcDecl.oneLineBody) {
            log.raw(" = ");
            // For one-line block increment indent to make it prettier
            incIndent();
            funcDecl.oneLineBody.unwrap()->accept(*this);
            decIndent();
        } else {
            log.raw(" ");
            funcDecl.body.unwrap()->accept(*this);
        }
    }

    void AstPrinter::visit(const Impl & impl) {
        printIndent();

        log.raw("impl");
        print(impl.typeParams);
        log.raw(" ");
        impl.traitTypePath->accept(*this);
        log.raw(" for ");
        impl.forType->accept(*this);
        printMembers(impl.members);
    }

    void AstPrinter::visit(const Mod & mod) {
        printIndent();

        log.raw("mod ");
        printId(mod.name);
        printMembers(mod.items);
    }

    void AstPrinter::visit(const Struct & _struct) {
        printIndent();

        log.raw("struct ");
        printId(_struct.name);
        log.raw(" ");

        for (size_t i = 0; i < _struct.fields.size(); i++) {
            const auto & field = _struct.fields.at(i);
            printId(field->name);
            log.raw(": ");
            field->type->accept(*this);

            if (i < _struct.fields.size() - 1) {
                log.raw(", ");
            }
        }
    }

    void AstPrinter::visit(const Trait & trait) {
        printIndent();

        log.raw("trait ");
        printId(trait.name);
        print(trait.typeParams);

        if (!trait.superTraits.empty()) {
            log.raw(" : ");
        }

        for (size_t i = 0; i < trait.superTraits.size(); i++) {
            trait.superTraits.at(i)->accept(*this);
            if (i < trait.superTraits.size() - 1) {
                log.raw(", ");
            }
        }
        printMembers(trait.members);
    }

    void AstPrinter::visit(const TypeAlias & typeAlias) {
        printIndent();

        log.raw("type ");
        printId(typeAlias.name);
        log.raw(" = ");
        typeAlias.type->accept(*this);
    }

    void AstPrinter::visit(const VarStmt & varDecl) {
        printIndent();

        log.raw(varDecl.kind.kindToString(), "");
        printId(varDecl.name);
        if (varDecl.type) {
            log.raw(": ");
            varDecl.type->accept(*this);
        }
    }

    void AstPrinter::visit(const WhileStmt & whileStmt) {
        printIndent();

        log.raw("while ");
        whileStmt.condition->accept(*this);
        log.raw(" ");
        whileStmt.body->accept(*this);
    }

    /////////////////
    // Expressions //
    /////////////////
    void AstPrinter::visit(const Assignment & assignment) {
        printIndent();

        assignment.lhs->accept(*this);
        log.raw(" = ");
        assignment.rhs->accept(*this);
    }

    void AstPrinter::visit(const Block & block) {
        if (block.stmts.empty()) {
            log.raw("{}");
            return;
        }

        log.raw("{").nl();
        incIndent();
        for (const auto & stmt : block.stmts) {
            stmt->accept(*this);
            log.nl();
        }
        decIndent();
        // Closing brace must go on the "super"-indent
        printIndent();
        log.raw("}");
    }

    void AstPrinter::visit(const BorrowExpr & borrowExpr) {
        if (borrowExpr.twin) {
            log.raw("&&");
        } else {
            log.raw("&");
        }
        if (borrowExpr.mut) {
            log.raw("mut");
        }
        log.raw(" ");
        borrowExpr.expr->accept(*this);
    }

    void AstPrinter::visit(const BreakExpr & breakExpr) {
        log.raw("break ");
        if (breakExpr.expr) {
            breakExpr.expr.unwrap()->accept(*this);
        }
    }

    void AstPrinter::visit(const ContinueExpr & continueExpr) {
        log.raw("continue");
    }

    void AstPrinter::visit(const DerefExpr & derefExpr) {
        log.raw("&");
        derefExpr.expr->accept(*this);
    }

    void AstPrinter::visit(const IfExpr & ifExpr) {
        log.raw("if ");
        ifExpr.condition->accept(*this);
        log.raw(" ");
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap()->accept(*this);
        }
        if (ifExpr.elseBranch) {
            log.raw(" else ");
            ifExpr.elseBranch.unwrap()->accept(*this);
        }
    }

    void AstPrinter::visit(const Infix & infix) {
        if (precedenceDebug) {
            log.raw("(");
        }

        infix.lhs->accept(*this);
        log.raw(" ");
        if (infix.op.kind == parser::TokenKind::Id) {
            log.raw(infix.op.val);
        } else {
            log.raw(infix.op.kindToString());
        }
        log.raw(" ");
        infix.rhs->accept(*this);

        if (precedenceDebug) {
            log.raw(")");
        }
    }

    void AstPrinter::visit(const Invoke & invoke) {
        invoke.lhs->accept(*this);
        log.raw("(");
        print(*invoke.args);
        log.raw(")");
    }

    void AstPrinter::visit(const Lambda & lambdaExpr) {
        log.raw("|");
        for (size_t i = 0; i < lambdaExpr.params.size(); i++) {
            const auto & param = lambdaExpr.params.at(i);
            printId(param->name);
            if (param->type) {
                log.raw(": ");
                param->type.unwrap()->accept(*this);
            }
            if (i < lambdaExpr.params.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw("| ");

        if (lambdaExpr.returnType) {
            log.raw(" -> ");
            lambdaExpr.returnType.unwrap()->accept(*this);
            log.raw(" ");
        }

        lambdaExpr.body->accept(*this);
    }

    void AstPrinter::visit(const ListExpr & listExpr) {
        log.raw("[");
        for (const auto & el : listExpr.elements) {
            el->accept(*this);
        }
        log.raw("]");
    }

    void AstPrinter::visit(const LiteralConstant & literalConstant) {
        log.raw(literalConstant.token.val);
    }

    void AstPrinter::visit(const LoopExpr & loopExpr) {
        log.raw("loop ");
        loopExpr.body->accept(*this);
    }

    void AstPrinter::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs->accept(*this);
        log.raw(".");
        printId(memberAccess.field);
    }

    void AstPrinter::visit(const ParenExpr & parenExpr) {
        log.raw("(");
        parenExpr.expr->accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const PathExpr & pathExpr) {
        // TODO: `Names` mode

        if (pathExpr.global) {
            log.raw("::");
        }
        for (size_t i = 0; i < pathExpr.segments.size(); i++) {
            const auto & segment = pathExpr.segments.at(i);
            printId(segment->name);
            print(segment->typeParams, true);
            if (i < pathExpr.segments.size() - 1) {
                log.raw("::");
            }
        }
    }

    void AstPrinter::visit(const Prefix & prefix) {
        log.raw(prefix.op.kindToString());
        prefix.rhs->accept(*this);
    }

    void AstPrinter::visit(const QuestExpr & questExpr) {
        questExpr.expr->accept(*this);
        log.raw("?");
    }

    void AstPrinter::visit(const ReturnExpr & returnExpr) {
        log.raw("return ");
        if (returnExpr.expr) {
            returnExpr.expr.unwrap()->accept(*this);
        }
    }

    void AstPrinter::visit(const SpreadExpr & spreadExpr) {
        log.raw(spreadExpr.token.kindToString());
        spreadExpr.expr->accept(*this);
    }

    void AstPrinter::visit(const Subscript & subscript) {
        subscript.lhs->accept(*this);
        for (size_t i = 0; i < subscript.indices.size(); ++i) {
            subscript.indices.at(i)->accept(*this);
            if (i < subscript.indices.size() - 1) {
                log.raw(", ");
            }
        }
    }

    void AstPrinter::visit(const ThisExpr & thisExpr) {
        log.raw("this");
    }

    void AstPrinter::visit(const TupleExpr & tupleExpr) {
        log.raw("(");
        print(*tupleExpr.elements);
        log.raw(")");
    }

    void AstPrinter::visit(const UnitExpr & unitExpr) {
        log.raw("()");
    }

    void AstPrinter::visit(const WhenExpr & whenExpr) {
        log.raw("when ");
        whenExpr.subject->accept(*this);

        log.raw("{");
        incIndent();

        for (const auto & entry : whenExpr.entries) {
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

    void AstPrinter::visit(const ParenType & parenType) {
        log.raw("(");
        parenType.type->accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const TupleType & tupleType) {
        log.raw("(");
        for (size_t i = 0; i < tupleType.elements.size(); i++) {
            const auto & el = tupleType.elements.at(i);
            if (el->name) {
                printId(el->name.unwrap());
            }
            if (el->type) {
                if (el->name) {
                    log.raw(": ");
                }
                el->type.unwrap()->accept(*this);
            }
            if (i < tupleType.elements.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(")");
    }

    void AstPrinter::visit(const FuncType & funcType) {
        log.raw("(");
        print(funcType.params);
        log.raw(") -> ");
        funcType.returnType->accept(*this);
    }

    void AstPrinter::visit(const SliceType & listType) {
        log.raw("[");
        listType.type->accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(const ArrayType & arrayType) {
        log.raw("[");
        arrayType.type->accept(*this);
        log.raw("; ");
        arrayType.sizeExpr->accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(const TypePath & typePath) {
        if (typePath.global) {
            log.raw("::");
        }
        for (size_t i = 0; i < typePath.segments.size(); i++) {
            print(*typePath.segments.at(i));
            if (i < typePath.segments.size() - 1) {
                log.raw("::");
            }
        }
    }

    void AstPrinter::visit(const UnitType & unitType) {
        log.raw("()");
    }

    void AstPrinter::visit(const GenericType & genericType) {
        printId(genericType.name);
        if (genericType.type) {
            log.raw(": ");
            genericType.type.unwrap()->accept(*this);
        }
    }

    void AstPrinter::visit(const ConstParam & constParam) {
        log.raw("const");
        printId(constParam.name);
        log.raw(": ");
        constParam.type->accept(*this);
        if (constParam.defaultValue) {
            log.raw(" = ");
            constParam.defaultValue.unwrap()->accept(*this);
        }
    }

    void AstPrinter::visit(const Lifetime & lifetime) {
        log.raw("`");
        printId(lifetime.name);
    }

    void AstPrinter::printIndent() const {
        for (int i = 0; i < indent; i++) {
            log.raw(indentChar);
        }
    }

    void AstPrinter::print(const ast::attr_list & attributes) {
        for (const auto & attr : attributes) {
            log.raw("@");
            printId(attr->name);
            log.raw("(");
            print(*attr->params);
            log.raw(")").nl();
        }
    }

    void AstPrinter::printModifiers(const parser::token_list & modifiers) {
        for (const auto & mod : modifiers) {
            log.raw(mod.kindToString());
        }
    }

    void AstPrinter::print(const ast::opt_type_params & optTypeParams, bool pathPrefix) {
        if (!optTypeParams) {
            return;
        }

        auto typeParams = optTypeParams.unwrap("AstPrinter -> print typeParams -> optTypeParams");
        if (typeParams.empty()) {
            return;
        } else if (pathPrefix) {
            log.raw("::");
        }

        log.raw("<");
        for (size_t i = 0; i < typeParams.size(); i++) {
            typeParams.at(i)->accept(*this);
            if (i < typeParams.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(">");
    }

    void AstPrinter::print(ast::NamedList & namedList) {
        for (size_t i = 0; i < namedList.elements.size(); i++) {
            const auto & namedEl = namedList.elements.at(i);
            if (namedEl->name) {
                printId(namedEl->name.unwrap());
                if (namedEl->value) {
                    log.raw(": ");
                }
            }
            if (namedEl->value) {
                namedEl->value.unwrap()->accept(*this);
            }
            if (i < namedList.elements.size() - 1) {
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

    void AstPrinter::print(TypePathSegment & idType) {
        printId(idType.name);
        print(idType.typeParams);
    }

    void AstPrinter::printMembers(const item_list & members) {
        log.raw(" {");
        incIndent();
        for (const auto & item : members) {
            item->accept(*this);
            log.nl();
        }
        log.raw("}");
    }

    void AstPrinter::printId(const id_ptr & id) {
        const auto & value = id->getValue();
        if (value) {
            log.raw(value.unwrap());
        } else {
            log.raw("[ERROR ID]");
        }
    }

    void AstPrinter::incIndent() {
        ++indent;
    }

    void AstPrinter::decIndent() {
        --indent;
    }
}
