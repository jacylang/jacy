#include "ast/AstPrinter.h"

namespace jc::ast {
    AstPrinter::AstPrinter() {
        log.getConfig().printOwner = false;
        lastColor = static_cast<uint8_t>(allowedNamesColors.size());
        printAstNodeMap = common::Config::getInstance().checkPrint(common::Config::PrintKind::AstNodeMap);
    }

    void AstPrinter::print(const sess::sess_ptr & sess, const Party & party, AstPrinterMode mode) {
        this->sess = sess;
        this->mode = mode;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);
    }

    void AstPrinter::visit(const ErrorNode&) {
        log.raw("[ERROR]");
    }

    void AstPrinter::visit(const File & file) {
        // We don't use `printBodyLike` to avoid increasing indent on top-level
        log.raw("--- file ", sess->sourceMap.getSourceFile(file.fileId).filename()).nl();
        for (const auto & item : file.items) {
            item.accept(*this);
            log.nl();
        }
    }

    void AstPrinter::visit(const Dir & dir) {
        log.raw("--- dir ", dir.name).nl();
        for (const auto & module : dir.modules) {
            module->accept(*this);
            log.nl();
        }
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const ExprStmt & exprStmt) {
        printNodeId(exprStmt);

        exprStmt.expr.accept(*this);
        log.raw(";");
    }

    void AstPrinter::visit(const ForStmt & forStmt) {
        printNodeId(forStmt);

        log.raw("for ");
        // TODO: Update when `for` will have patterns
        forStmt.forEntity.accept(*this);
        log.raw(" in ");
        forStmt.inExpr.accept(*this);
        forStmt.body.accept(*this);
    }

    void AstPrinter::visit(const ItemStmt & itemStmt) {
        printNodeId(itemStmt);

        if (itemStmt.item) {
            printAttributes(itemStmt.item.unwrap()->attributes);
        }
        itemStmt.item.accept(*this);
    }

    ///////////
    // Items //
    ///////////
    void AstPrinter::visit(const Enum & enumDecl) {
        printNodeId(enumDecl);

        log.raw("enum ");
        colorizeDef(enumDecl.name);

        printBodyLike(enumDecl.entries, ",\n");
    }

    void AstPrinter::visit(const EnumEntry & enumEntry) {
        printNodeId(enumEntry);

        enumEntry.name.accept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                log.raw(" = ");
                std::get<expr_ptr>(enumEntry.body).accept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                printDelim(std::get<tuple_t_el_list>(enumEntry.body), "(", ")");
                break;
            }
            case EnumEntryKind::Struct: {
                printBodyLike(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }
    }

    void AstPrinter::visit(const Func & func) {
        printNodeId(func);

        printModifiers(func.modifiers);
        log.raw("func");
        printGenerics(func.generics);
        log.raw(" ");

        colorizeDef(func.name);

        printDelim(func.params, "(", ")");

        if (func.returnType) {
            log.raw(": ");
            func.returnType.unwrap().accept(*this);
        }

        if (func.body) {
            if (func.body.unwrap() and func.body.unwrap().unwrap()->blockKind == BlockKind::OneLine) {
                log.raw(" = ");
            }
            func.body.unwrap().accept(*this);
        } else {
            log.raw(";");
        }
    }

    void AstPrinter::visit(const FuncParam & funcParam) {
        printNodeId(funcParam);

        colorizeName(funcParam.name);
        funcParam.name.accept(*this);
        resetNameColor();

        log.raw(": ");
        funcParam.type.accept(*this);

        if (funcParam.defaultValue) {
            log.raw(" = ");
            funcParam.defaultValue.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const Impl & impl) {
        printNodeId(impl);

        log.raw("impl");
        printGenerics(impl.generics);
        log.raw(" ");
        impl.traitTypePath.accept(*this);
        if (impl.forType) {
            log.raw(" for ");
            impl.forType.unwrap().accept(*this);
        }
        printBodyLike(impl.members, "\n");
    }

    void AstPrinter::visit(const Mod & mod) {
        printNodeId(mod);

        log.raw("mod ");
        colorizeDef(mod.name);
        printBodyLike(mod.items, "\n");
    }

    void AstPrinter::visit(const Struct & _struct) {
        printNodeId(_struct);

        log.raw("struct ");
        colorizeDef(_struct.name);
        log.raw(" ");

        printDelim(_struct.fields, "{", "}", ",\n");
    }

    void AstPrinter::visit(const StructField & field) {
        printNodeId(field);

        field.name.accept(*this);
        log.raw(": ");
        field.type.accept(*this);
    }

    void AstPrinter::visit(const Trait & trait) {
        printNodeId(trait);

        log.raw("trait ");

        colorizeDef(trait.name);

        printGenerics(trait.generics);

        if (!trait.superTraits.empty()) {
            log.raw(" : ");
        }

        printDelim(trait.superTraits);
        printBodyLike(trait.members, "\n");
    }

    void AstPrinter::visit(const TypeAlias & typeAlias) {
        printNodeId(typeAlias);

        log.raw("type ");
        colorizeDef(typeAlias.name);
        log.raw(" = ");
        typeAlias.type.accept(*this);
        log.raw(";");
    }

    void AstPrinter::visit(const UseDecl & useDecl) {
        printNodeId(useDecl);

        log.raw("use ");
        useDecl.useTree.accept(*this);
    }

    void AstPrinter::visit(const UseTreeRaw & useTree) {
        printNodeId(useTree);

        useTree.path->accept(*this);
    }

    void AstPrinter::visit(const UseTreeSpecific & useTree) {
        printNodeId(useTree);

        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        printDelim(useTree.specifics, "{", "}");
    }

    void AstPrinter::visit(const UseTreeRebind & useTree) {
        printNodeId(useTree);

        useTree.path->accept(*this);
        log.raw(" as ");
        useTree.as.accept(*this);
    }

    void AstPrinter::visit(const UseTreeAll & useTree) {
        printNodeId(useTree);

        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        log.raw("*");
    }

    void AstPrinter::visit(const LetStmt & letStmt) {
        printNodeId(letStmt);

        log.raw("let ");

        colorizeDef(letStmt.pat->name);

        if (letStmt.type) {
            log.raw(": ");
            letStmt.type.unwrap().accept(*this);
        }
        if (letStmt.assignExpr) {
            log.raw(" = ");
            letStmt.assignExpr.unwrap().accept(*this);
        }
        log.raw(";");
    }

    void AstPrinter::visit(const WhileStmt & whileStmt) {
        printNodeId(whileStmt);

        log.raw("while ");
        whileStmt.condition.accept(*this);
        log.raw(" ");
        whileStmt.body.accept(*this);
    }

    /////////////////
    // Expressions //
    /////////////////
    void AstPrinter::visit(const Assignment & assignment) {
        printNodeId(assignment);

        assignment.lhs.accept(*this);
        log.raw(" = ");
        assignment.rhs.accept(*this);
    }

    void AstPrinter::visit(const Block & block) {
        printNodeId(block);

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
        printNodeId(borrowExpr);

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
        printNodeId(breakExpr);

        log.raw("break ");
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const ContinueExpr & continueExpr) {
        printNodeId(continueExpr);

        log.raw("continue");
    }

    void AstPrinter::visit(const DerefExpr & derefExpr) {
        printNodeId(derefExpr);

        log.raw("&");
        derefExpr.expr.accept(*this);
    }

    void AstPrinter::visit(const IfExpr & ifExpr) {
        printNodeId(ifExpr);

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
        printNodeId(infix);

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
        printNodeId(invoke);

        invoke.lhs.accept(*this);
        printDelim(invoke.args, "(", ")");
    }

    void AstPrinter::visit(const Lambda & lambdaExpr) {
        printNodeId(lambdaExpr);

        printDelim(lambdaExpr.params, "|", "|");

        if (lambdaExpr.returnType) {
            log.raw(" -> ");
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        log.raw(" ");
        lambdaExpr.body.accept(*this);
    }

    void AstPrinter::visit(const LambdaParam & param) {
        printNodeId(param);

        param.name.accept(*this);
        if (param.type) {
            log.raw(": ");
            param.type.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const ListExpr & listExpr) {
        printNodeId(listExpr);

        log.raw("[");
        for (const auto & el : listExpr.elements) {
            el.accept(*this);
        }
        log.raw("]");
    }

    void AstPrinter::visit(const LiteralConstant & literalConstant) {
        printNodeId(literalConstant);

        log.raw(literalConstant.token.val);
    }

    void AstPrinter::visit(const LoopExpr & loopExpr) {
        printNodeId(loopExpr);

        log.raw("loop ");
        loopExpr.body.accept(*this);
    }

    void AstPrinter::visit(const MemberAccess & memberAccess) {
        printNodeId(memberAccess);

        memberAccess.lhs.accept(*this);
        log.raw(".");
        memberAccess.field.accept(*this);
    }

    void AstPrinter::visit(const ParenExpr & parenExpr) {
        printNodeId(parenExpr);

        log.raw("(");
        parenExpr.expr.accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const PathExpr & pathExpr) {
        printNodeId(pathExpr);

        colorizeName(pathExpr.id);

        if (pathExpr.global) {
            log.raw("::");
        }
        printDelim(pathExpr.segments, "", "", "::");

        resetNameColor();
    }

    void AstPrinter::visit(const PathExprSeg & seg) {
        printNodeId(seg);

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
        printGenerics(seg.generics, true);
    }

    void AstPrinter::visit(const Prefix & prefix) {
        printNodeId(prefix);

        log.raw(prefix.op.kindToString());
        prefix.rhs.accept(*this);
    }

    void AstPrinter::visit(const QuestExpr & questExpr) {
        printNodeId(questExpr);

        questExpr.expr.accept(*this);
        log.raw("?");
    }

    void AstPrinter::visit(const ReturnExpr & returnExpr) {
        printNodeId(returnExpr);

        log.raw("return ");
        if (returnExpr.expr) {
            returnExpr.expr.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const SpreadExpr & spreadExpr) {
        printNodeId(spreadExpr);

        log.raw(spreadExpr.token.kindToString());
        spreadExpr.expr.accept(*this);
    }

    void AstPrinter::visit(const StructExpr & structExpr) {
        printNodeId(structExpr);

        structExpr.path.accept(*this);
        printBodyLike(structExpr.fields, ",\n");
    }

    void AstPrinter::visit(const StructExprField & field) {
        printNodeId(field);

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
        printNodeId(subscript);

        subscript.lhs.accept(*this);
        log.raw("[");
        printDelim(subscript.indices);
        log.raw("]");
    }

    void AstPrinter::visit(const SelfExpr & selfExpr) {
        printNodeId(selfExpr);

        log.raw("this");
    }

    void AstPrinter::visit(const TupleExpr & tupleExpr) {
        printNodeId(tupleExpr);

        printDelim(tupleExpr.elements, "(", ")");
    }

    void AstPrinter::visit(const UnitExpr & unit) {
        printNodeId(unit);

        log.raw("()");
    }

    void AstPrinter::visit(const MatchExpr & matchExpr) {
        printNodeId(matchExpr);

        log.raw("match ");
        matchExpr.subject.accept(*this);
        printBodyLike(matchExpr.entries, ",\n");
    }

    void AstPrinter::visit(const MatchArm & matchArm) {
        printNodeId(matchArm);

        printDelim(matchArm.conditions);
        log.raw(" => ");
        matchArm.body.accept(*this);
    }

    // Types //
    void AstPrinter::visit(const ParenType & parenType) {
        printNodeId(parenType);

        log.raw("(");
        parenType.type.accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const TupleType & tupleType) {
        printNodeId(tupleType);

        printDelim(tupleType.elements, "(", ")");
    }

    void AstPrinter::visit(const TupleTypeEl & el) {
        printNodeId(el);

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
        printNodeId(funcType);

        printDelim(funcType.params, "(", ")");
        log.raw(" -> ");
        funcType.returnType.accept(*this);
    }

    void AstPrinter::visit(const SliceType & listType) {
        printNodeId(listType);

        log.raw("[");
        listType.type.accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(const ArrayType & arrayType) {
        printNodeId(arrayType);

        log.raw("[");
        arrayType.type.accept(*this);
        log.raw("; ");
        arrayType.sizeExpr.accept(*this);
        log.raw("]");
    }

    void AstPrinter::visit(const TypePath & typePath) {
        colorizeName(typePath.id);

        printNodeId(typePath);

        if (typePath.global) {
            log.raw("::");
        }
        printDelim(typePath.segments, "", "", "::");

        resetNameColor();
    }

    void AstPrinter::visit(const TypePathSeg & seg) {
        printNodeId(seg);

        seg.name.accept(*this);
        printGenerics(seg.generics);
    }

    void AstPrinter::visit(const UnitType & unitType) {
        printNodeId(unitType);

        log.raw("()");
    }

    // Generics //
    void AstPrinter::visit(const TypeParam & typeParam) {
        printNodeId(typeParam);

        typeParam.name.accept(*this);
        if (typeParam.boundType) {
            log.raw(": ");
            typeParam.boundType.unwrap().accept(*this);
        }
    }

    void AstPrinter::visit(const Lifetime & lifetime) {
        printNodeId(lifetime);

        log.raw("`");
        lifetime.name.accept(*this);
    }

    void AstPrinter::visit(const ConstParam & constParam) {
        printNodeId(constParam);

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
        printNodeId(attr);

        log.raw("@");
        attr.name.accept(*this);
        printDelim(attr.params, "(", ")");
        log.nl();
    }

    void AstPrinter::visit(const Identifier & id) {
        printNodeId(id);

        log.raw(id.getValue());
    }

    void AstPrinter::visit(const Arg & el) {
        printNodeId(el);

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
        printNodeId(path);

        if (path.global) {
            log.raw("::");
        }

        printDelim(path.segments, "", "", "::");
    }

    void AstPrinter::visit(const SimplePathSeg & seg) {
        printNodeId(seg);

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

    // Patterns //
    void AstPrinter::visit(const ParenPat & pat) {
        log.raw("(");
        pat.pat.accept(*this);
        log.raw(")");
    }

    void AstPrinter::visit(const LitPat & pat) {
        printNodeId(pat);

        if (pat.neg) {
            log.raw("-");
        }

        log.raw(pat.literal.val);
    }

    void AstPrinter::visit(const BorrowPat & pat) {
        if (pat.ref) {
            log.raw("ref ");
        }

        if (pat.mut) {
            log.raw("mut ");
        }

        pat.name.accept(*this);
    }

    void AstPrinter::visit(const SpreadPat&) {
        log.raw("...");
    }

    // Helpers //
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

    void AstPrinter::printGenerics(const ast::opt_gen_params & generics, bool pathPrefix) {
        generics.then([&](const auto & generics) {
            if (generics.empty()) {
                return;
            } else if (pathPrefix) {
                log.raw("::");
            }

            printDelim(generics, "<", ">");
        });
    }

    // Indentation //
    void AstPrinter::printIndent() const {
        log.raw(common::Indent<4>(indent));
    }

    void AstPrinter::incIndent() {
        ++indent;
    }

    void AstPrinter::decIndent() {
        --indent;
    }

    // NodeMap mode //
    void AstPrinter::printNodeId(const Node & node) const {
        if (not printAstNodeMap or mode != AstPrinterMode::Parsing) {
            return;
        }
        log.raw(Color::LightGray, "[", node.id, "]", Color::Reset);
    }

    // Names mode //
    void AstPrinter::colorizeDef(const id_ptr & ident) {
        if (mode != AstPrinterMode::Names) {
            ident.unwrap()->accept(*this);
            return;
        }
        log.raw(getNameColor(ident.unwrap()->id));
        ident.unwrap()->accept(*this);
        resetNameColor();
    }

    void AstPrinter::colorizeName(node_id nodeId) {
        if (mode != AstPrinterMode::Names) {
            return;
        }
        const auto & resolved = sess->resStorage.getRes(nodeId);
        switch (resolved.kind) {
            case resolve::ResKind::Error: {
                log.raw(Color::LightGray, "[[Unresolved]]", Color::Reset);
                break;
            }
            case resolve::ResKind::Local: {
                log.raw(getNameColor(resolved.asLocal()));
                break;
            }
            case resolve::ResKind::Def: {
                // Get definition and use its name node_id as resolution color
                log.raw(getNameColor(sess->defStorage.getDef(resolved.asDef()).nameNodeId.unwrap()));
                break;
            }
            case resolve::ResKind::PrimType: {
                log.raw(resolve::primTypeToString(resolved.asPrimType()));
                break;
            }
        }
    }

    void AstPrinter::resetNameColor() {
        if (mode != AstPrinterMode::Names) {
            return;
        }
        log.raw(Color::Reset);
    }

    common::Color AstPrinter::getNameColor(node_id defId) {
        // Note: Functionality is common for name declaration and name usage,
        //  because AstPrinter does not do forward declarations

        const auto & found = namesColors.find(defId);
        if (found == namesColors.end()) {
            // Assign color for name if not found

            if (lastColor >= allowedNamesColors.size() - 1) {
                // If we used last allowed color then repeat from the beginning
                lastColor = 0;
            } else {
                // Use next allowed color
                lastColor++;
            }
            namesColors[defId] = allowedNamesColors.at(lastColor);
            return namesColors[defId];
        }
        return found->second;
    }
}
