#include "ast/AstPrinter.h"

namespace jc::ast {
    AstPrinter::AstPrinter() {
        log.getConfig().printOwner = false;
    }

    void AstPrinter::print(const sess::sess_ptr & sess, const Party & party, AstPrinterMode mode) {
        this->sess = sess;
        this->mode = mode;

        party.getRootModule()->accept(*this);
    }

    void AstPrinter::visit(const ErrorNode&) {
        log.raw("[ERROR]");
    }

    void AstPrinter::visit(const File & file) {
        // We don't use `printBodyLike` to avoid increasing indent on top-level
        for (const auto & item : file.items) {
            item.accept(*this);
            log.nl();
        }
    }

    void AstPrinter::visit(const RootModule & rootModule) {
        rootModule.getRootFile()->accept(*this);
        rootModule.getRootDir()->accept(*this);
    }

    void AstPrinter::visit(const FileModule & fileModule) {
        log.raw("--- file", fileModule.getName()).nl();
        fileModule.getFile()->accept(*this);
    }

    void AstPrinter::visit(const DirModule & dirModule) {
        log.raw("--- dir", dirModule.getName()).nl();
        for (const auto & module : dirModule.getModules()) {
            module->accept(*this);
            log.nl();
        }
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const Enum & enumDecl) {
        enumDecl.name.accept(*this);

        printBodyLike(enumDecl.entries, ",\n");
    }

    void AstPrinter::visit(const EnumEntry & enumEntry) {
        enumEntry.name.accept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                log.raw(" = ");
                std::get<expr_ptr>(enumEntry.body).accept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                printDelim(std::get<named_list>(enumEntry.body), "(", ")");
                break;
            }
            case EnumEntryKind::Struct: {
                printBodyLike(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }
    }

    void AstPrinter::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.accept(*this);
    }

    void AstPrinter::visit(const ForStmt & forStmt) {
        log.raw("for ");
        // TODO: Update when `for` will have patterns
        forStmt.forEntity.accept(*this);
        log.raw(" in ");
        forStmt.inExpr.accept(*this);
        forStmt.body.accept(*this);
    }

    void AstPrinter::visit(const ItemStmt & itemStmt) {
        if (itemStmt.item) {
            printAttributes(itemStmt.item.unwrap()->attributes);
        }
        itemStmt.item.accept(*this);
    }

    void AstPrinter::visit(const Func & func) {
        printModifiers(func.modifiers);
        log.raw("func");
        printTypeParams(func.typeParams);
        log.raw(" ");
        func.name.accept(*this);

        printDelim(func.params, "(", ")");

        if (func.returnType) {
            log.raw(": ");
            func.returnType.unwrap().accept(*this);
        }

        if (func.oneLineBody) {
            log.raw(" = ");
            // For one-line block increment indent to make it prettier
            func.oneLineBody.unwrap().accept(*this);
        } else {
            log.raw(" ");
            func.body.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const FuncParam & funcParam) {
        funcParam.name.accept(*this);

        log.raw(": ");
        funcParam.type.accept(*this);

        if (funcParam.defaultValue) {
            log.raw(" = ");
            funcParam.defaultValue.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const Impl & impl) {
        log.raw("impl");
        printTypeParams(impl.typeParams);
        log.raw(" ");
        impl.traitTypePath.accept(*this);
        log.raw(" for ");
        impl.forType.accept(*this);
        printBodyLike(impl.members, "\n");
    }

    void AstPrinter::visit(const Mod & mod) {
        log.raw("mod ");
        mod.name.accept(*this);
        printBodyLike(mod.items, "\n");
    }

    void AstPrinter::visit(const Struct & _struct) {
        log.raw("struct ");
        _struct.name.accept(*this);
        log.raw(" ");

        printDelim(_struct.fields, "{", "}", ",\n");
    }

    void AstPrinter::visit(const StructField & field) {
        field.name.accept(*this);
        log.raw(": ");
        field.type.accept(*this);
    }

    void AstPrinter::visit(const Trait & trait) {
        log.raw("trait ");
        trait.name.accept(*this);
        printTypeParams(trait.typeParams);

        if (!trait.superTraits.empty()) {
            log.raw(" : ");
        }

        printDelim(trait.superTraits);
        printBodyLike(trait.members, "\n");
    }

    void AstPrinter::visit(const TypeAlias & typeAlias) {
        log.raw("type ");
        typeAlias.name.accept(*this);
        log.raw(" = ");
        typeAlias.type.accept(*this);
    }

    void AstPrinter::visit(const UseDecl & useDecl) {
        log.raw("use ");
        useDecl.useTree.accept(*this);
    }

    void AstPrinter::visit(const UseTreeRaw & useTree) {
        useTree.path->accept(*this);
    }

    void AstPrinter::visit(const UseTreeSpecific & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        printDelim(useTree.specifics, "{", "}");
    }

    void AstPrinter::visit(const UseTreeRebind & useTree) {
        useTree.path->accept(*this);
        log.raw(" as ");
        useTree.as.accept(*this);
    }

    void AstPrinter::visit(const UseTreeAll & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        log.raw("*");
    }

    void AstPrinter::visit(const VarStmt & varStmt) {
        log.raw(varStmt.kind.kindToString(), "");
        varStmt.name.accept(*this);
        if (varStmt.type) {
            log.raw(": ");
            varStmt.type.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const WhileStmt & whileStmt) {
        log.raw("while ");
        whileStmt.condition.accept(*this);
        log.raw(" ");
        whileStmt.body.accept(*this);
    }

    /////////////////
    // Expressions //
    /////////////////
    void AstPrinter::visit(const Assignment & assignment) {
        assignment.lhs.accept(*this);
        log.raw(" = ");
        assignment.rhs.accept(*this);
    }

    void AstPrinter::visit(const Block & block) {
        if (block.blockKind == BlockKind::OneLine) {
            block.oneLine.unwrap().accept(*this);
            return;
        }
        if (block.stmts.unwrap().empty()) {
            log.raw("{}");
            return;
        }
        printBodyLike(block.stmts.unwrap(), "\n");
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
        borrowExpr.expr.accept(*this);
    }

    void AstPrinter::visit(const BreakExpr & breakExpr) {
        log.raw("break ");
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const ContinueExpr&) {
        log.raw("continue");
    }

    void AstPrinter::visit(const DerefExpr & derefExpr) {
        log.raw("&");
        derefExpr.expr.accept(*this);
    }

    void AstPrinter::visit(const IfExpr & ifExpr) {
        log.raw("if ");
        ifExpr.condition.accept(*this);
        log.raw(" ");
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap().accept(*this);
        }
        if (ifExpr.elseBranch) {
            log.raw(" else ");
            ifExpr.elseBranch.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const Infix & infix) {
        if (precedenceDebug) {
            log.raw("(");
        }

        infix.lhs.accept(*this);
        log.raw(" ");
        if (infix.op.kind == parser::TokenKind::Id) {
            log.raw(infix.op.val);
        } else {
            log.raw(infix.op.kindToString());
        }
        log.raw(" ");
        infix.rhs.accept(*this);

        if (precedenceDebug) {
            log.raw(")");
        }
    }

    void AstPrinter::visit(const Invoke & invoke) {
        invoke.lhs.accept(*this);
        printDelim(invoke.args, "(", ")");
    }

    void AstPrinter::visit(const Lambda & lambdaExpr) {
        printDelim(lambdaExpr.params, "|", "|");

        if (lambdaExpr.returnType) {
            log.raw(" -> ");
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        log.raw(" ");
        lambdaExpr.body.accept(*this);
    }

    void AstPrinter::visit(const LambdaParam & param) {
        param.name.accept(*this);
        if (param.type) {
            log.raw(": ");
            param.type.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const ListExpr & listExpr) {
        log.raw("[");
        for (const auto & el : listExpr.elements) {
            el.accept(*this);
        }
        log.raw("]");
    }

    void AstPrinter::visit(const LiteralConstant & literalConstant) {
        log.raw(literalConstant.token.val);
    }

    void AstPrinter::visit(const LoopExpr & loopExpr) {
        log.raw("loop ");
        loopExpr.body.accept(*this);
    }

    void AstPrinter::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.accept(*this);
        log.raw(".");
        memberAccess.field.accept(*this);
    }

    void AstPrinter::visit(const ParenExpr & parenExpr) {
        log.raw("(");
        parenExpr.expr.accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const PathExpr & pathExpr) {
        // TODO: `Names` mode

        if (pathExpr.global) {
            log.raw("::");
        }
        printDelim(pathExpr.segments, "", "", "::");
    }

    void AstPrinter::visit(const PathExprSeg & seg) {
        switch (seg.kind) {
            case PathExprSeg::Kind::Super: {
                log.raw("super");
                break;
            }
            case PathExprSeg::Kind::Self: {
                log.raw("self");
                break;
            }
            case PathExprSeg::Kind::Party: {
                log.raw("party");
                break;
            }
            case PathExprSeg::Kind::Ident: {
                seg.ident.unwrap().accept(*this);
                break;
            }
            default: {
                log.devPanic("Unexpected `PathExprSeg::Kind` in `AstPrinter`");
            }
        }
        printTypeParams(seg.typeParams, true);
    }

    void AstPrinter::visit(const Prefix & prefix) {
        log.raw(prefix.op.kindToString());
        prefix.rhs.accept(*this);
    }

    void AstPrinter::visit(const QuestExpr & questExpr) {
        questExpr.expr.accept(*this);
        log.raw("?");
    }

    void AstPrinter::visit(const ReturnExpr & returnExpr) {
        log.raw("return ");
        if (returnExpr.expr) {
            returnExpr.expr.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const SpreadExpr & spreadExpr) {
        log.raw(spreadExpr.token.kindToString());
        spreadExpr.expr.accept(*this);
    }

    void AstPrinter::visit(const StructExpr & structExpr) {
        structExpr.path.accept(*this);
        printBodyLike(structExpr.fields, ",\n");
    }

    void AstPrinter::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().accept(*this);
                log.raw(": ");
                field.expr->accept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().accept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                log.raw("...");
                field.expr->accept(*this);
                break;
            }
        }
    }

    void AstPrinter::visit(const Subscript & subscript) {
        subscript.lhs.accept(*this);
        log.raw("[");
        printDelim(subscript.indices);
        log.raw("]");
    }

    void AstPrinter::visit(const ThisExpr&) {
        log.raw("this");
    }

    void AstPrinter::visit(const TupleExpr & tupleExpr) {
        printDelim(tupleExpr.elements, "(", ")");
    }

    void AstPrinter::visit(const UnitExpr&) {
        log.raw("()");
    }

    void AstPrinter::visit(const WhenExpr & whenExpr) {
        log.raw("when ");
        whenExpr.subject.accept(*this);
        printBodyLike(whenExpr.entries, ",\n");
    }

    void AstPrinter::visit(const WhenEntry & entry) {
        printDelim(entry.conditions);
        log.raw(" => ");
        entry.body.accept(*this);
    }

    // Types //
    void AstPrinter::visit(const ParenType & parenType) {
        log.raw("(");
        parenType.type.accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const TupleType & tupleType) {
        printDelim(tupleType.elements, "(", ")");
    }

    void AstPrinter::visit(const TupleTypeEl & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.type) {
            if (el.name) {
                log.raw(": ");
            }
            el.type.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const FuncType & funcType) {
        printDelim(funcType.params, "(", ")");
        log.raw(" -> ");
        funcType.returnType.accept(*this);
    }

    void AstPrinter::visit(const SliceType & listType) {
        log.raw("[");
        listType.type.accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(const ArrayType & arrayType) {
        log.raw("[");
        arrayType.type.accept(*this);
        log.raw("; ");
        arrayType.sizeExpr.accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(const TypePath & typePath) {
        if (typePath.global) {
            log.raw("::");
        }
        printDelim(typePath.segments, "", "", "::");
    }

    void AstPrinter::visit(const TypePathSeg & seg) {
        seg.name.accept(*this);
        printTypeParams(seg.typeParams);
    }

    void AstPrinter::visit(const UnitType&) {
        log.raw("()");
    }

    // Generics //
    void AstPrinter::visit(const GenericType & genericType) {
        genericType.name.accept(*this);
        if (genericType.boundType) {
            log.raw(": ");
            genericType.boundType.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const Lifetime & lifetime) {
        log.raw("`");
        lifetime.name.accept(*this);
    }

    void AstPrinter::visit(const ConstParam & constParam) {
        log.raw("const");
        constParam.name.accept(*this);
        log.raw(": ");
        constParam.type.accept(*this);
        if (constParam.defaultValue) {
            log.raw(" = ");
            constParam.defaultValue.unwrap().accept(*this);
        }
    }

    // Fragments //
    void AstPrinter::visit(const Attribute & attr) {
        log.raw("@");
        attr.name.accept(*this);
        printDelim(attr.params, "(", ")");
        log.nl();
    }

    void AstPrinter::visit(const Identifier & id) {
        log.raw(id.getValue());
    }

    void AstPrinter::visit(const NamedElement & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
            if (el.value) {
                log.raw(": ");
            }
        }
        if (el.value) {
            el.value.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const SimplePath & path) {
        if (path.global) {
            log.raw("::");
        }

        printDelim(path.segments, "", "", "::");
    }

    void AstPrinter::visit(const SimplePathSeg & seg) {
        switch (seg.kind) {
            case SimplePathSeg::Kind::Super: {
                log.raw("super");
                break;
            }
            case SimplePathSeg::Kind::Self: {
                log.raw("self");
                break;
            }
            case SimplePathSeg::Kind::Party: {
                log.raw("party");
                break;
            }
            case SimplePathSeg::Kind::Ident: {
                seg.ident.unwrap().accept(*this);
                break;
            }
        }
    }

    void AstPrinter::printAttributes(const ast::attr_list & attributes) {
        for (const auto & attr : attributes) {
            attr->accept(*this);
        }
    }

    void AstPrinter::printModifiers(const parser::token_list & modifiers) {
        for (const auto & mod : modifiers) {
            log.raw(mod.kindToString());
        }
    }

    void AstPrinter::printTypeParams(const ast::opt_type_params & optTypeParams, bool pathPrefix) {
        if (!optTypeParams) {
            return;
        }

        auto typeParams = optTypeParams.unwrap("AstPrinter -> print typeParams -> optTypeParams");
        if (typeParams.empty()) {
            return;
        } else if (pathPrefix) {
            log.raw("::");
        }

        printDelim(typeParams, "<", ">");
    }

    // Indentation //
    void AstPrinter::printIndent() const {
        log.raw(common::Indent(indent));
    }

    void AstPrinter::incIndent() {
        ++indent;
    }

    void AstPrinter::decIndent() {
        --indent;
    }

    // Names mode //
    void AstPrinter::printName(node_id nodeId) {
        const auto & resolved = sess->resStorage.getRes(nodeId);
        if (not resolved) {
            log.raw("[[Unresolved]]");
        } else {
            log.raw("[[", sess->nodeMap.getNode(resolved).span.toString(), "]]");
        }
    }
}
